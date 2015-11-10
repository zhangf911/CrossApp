//
//  CAScrollView.cpp
//  CrossApp
//
//  Created by Li Yuanfeng on 14-4-23.
//  Copyright (c) 2014 http://9miao.com All rights reserved.
//

#include "CAScrollView.h"
#include "CAScale9ImageView.h"
#include "basics/CAApplication.h"
#include "dispatcher/CATouchDispatcher.h"
#include "basics/CAScheduler.h"
#include "dispatcher/CATouch.h"
#include "support/CAPointExtension.h"
#include "kazmath/GL/matrix.h"
#include "CCEGLView.h"
#include "animation/CAViewAnimation.h"
#include "animation/CAAnimation.h"
#include "shaders/CAShaderCache.h"

NS_CC_BEGIN

#pragma CAScrollView

CAScrollView::CAScrollView()
:m_pContainer(NULL)
,m_obViewSize(DSizeZero)
,m_bTouchEnabledAtSubviews(true)
,m_pScrollViewDelegate(NULL)
,m_bBounces(true)
,m_bBounceHorizontal(true)
,m_bBounceVertical(true)
,m_bTracking(false)
,m_bDecelerating(false)
,m_bZooming(false)
,m_fMaximumZoomScale(1.0f)
,m_fMinimumZoomScale(1.0f)
,m_fZoomScale(1.0f)
,m_fTouchLength(0.0f)
,m_tInertia(DPointZero)
,m_tCloseToPoint(DPointZero)
,m_tInitialPoint(DPointZero)
,m_pIndicatorHorizontal(NULL)
,m_pIndicatorVertical(NULL)
,m_bShowsHorizontalScrollIndicator(true)
,m_bShowsVerticalScrollIndicator(true)
,m_pHeaderRefreshView(NULL)
,m_pFooterRefreshView(NULL)
,m_bPCMode(false)
{
    this->setPriorityScroll(true);
    this->setReachBoundaryHandOverToSuperview(true);
    this->setHaveNextResponder(false);
}

CAScrollView::~CAScrollView()
{
    CC_SAFE_RELEASE(m_pHeaderRefreshView);
    CC_SAFE_RELEASE(m_pFooterRefreshView);
    m_vChildInThis.clear();
    m_pScrollViewDelegate = NULL;
}

void CAScrollView::onEnterTransitionDidFinish()
{
    CAView::onEnterTransitionDidFinish();
}

void CAScrollView::onExitTransitionDidStart()
{
    CAView::onExitTransitionDidStart();
    
    m_tPointOffset.clear();
    m_vTouches.clear();
    m_tInertia = DPointZero;
}

CAScrollView* CAScrollView::createWithFrame(const DRect& rect)
{
    CAScrollView* scrollView = new CAScrollView();
    if (scrollView && scrollView->initWithFrame(rect))
    {
        scrollView->autorelease();
        return scrollView;
    }
    CC_SAFE_DELETE(scrollView);
    return NULL;
}

CAScrollView* CAScrollView::createWithCenter(const DRect& rect)
{
    CAScrollView* scrollView = new CAScrollView();
    if (scrollView && scrollView->initWithCenter(rect))
    {
        scrollView->autorelease();
        return scrollView;
    }
    CC_SAFE_DELETE(scrollView);
    return NULL;
}

bool CAScrollView::init()
{
    if (!CAView::init())
    {
        return false;
    }
    this->setDisplayRange(false);
    
    m_pContainer = new CAView();
    this->setContainerFrame(DPointZero, this->getBounds().size);
    m_vChildInThis.pushBack(m_pContainer);
    this->addSubview(m_pContainer);
    m_pContainer->release();
    
    this->updateIndicator();
    return true;
}

void CAScrollView::addSubview(CAView* subview)
{
    do
    {
        CC_BREAK_IF(m_vChildInThis.contains(subview));
        m_pContainer->addSubview(subview);
        return;
    }
    while (0);
    
    CAView::addSubview(subview);
}

void CAScrollView::insertSubview(CAView* subview, int z)
{
    do
    {
        CC_BREAK_IF(m_vChildInThis.contains(subview));
        m_pContainer->insertSubview(subview, z);
        return;
    }
    while (0);
    
    CAView::insertSubview(subview, z);
}

void CAScrollView::removeAllSubviews()
{
    m_pContainer->removeAllSubviews();
}

void CAScrollView::removeSubview(CAView* subview)
{
    m_pContainer->removeSubview(subview);
}

void CAScrollView::removeSubviewByTag(int tag)
{
    m_pContainer->removeSubviewByTag(tag);
}

CAView* CAScrollView::getSubviewByTag(int aTag)
{
    return m_pContainer->getSubviewByTag(aTag);
}

const CAVector<CAView*>& CAScrollView::getSubviews()
{
    return m_pContainer->getSubviews();
}

void CAScrollView::setViewSize(const DSize& var)
{
    CC_RETURN_IF(m_obViewSize.equals(var));
    
    m_obViewSize = var;
    m_obViewSize.width = MAX(m_obViewSize.width, m_obContentSize.width);
    m_obViewSize.height = MAX(m_obViewSize.height, m_obContentSize.height);
    
    CC_RETURN_IF(m_pContainer == NULL);
    
    DPoint point = m_pContainer->getFrameOrigin();
    this->setContainerFrame(point, m_obViewSize);
    
    if (!CAScheduler::isScheduled(schedule_selector(CAScrollView::deaccelerateScrolling), this))
    {
        CAScheduler::schedule(schedule_selector(CAScrollView::deaccelerateScrolling), this, 1/60.0f);
    }
}

const DSize& CAScrollView::getViewSize()
{
    return m_obViewSize;
}

bool CAScrollView::isReachBoundaryLeft()
{
    do
    {
        CC_BREAK_IF(m_fMaximumZoomScale - m_fMinimumZoomScale < FLT_EPSILON);
        CC_BREAK_IF(CAApplication::getApplication()->getTouchDispatcher()->getTouchCount() < 2);
        return false;
    }
    while (0);

    return m_pContainer->getFrame().getMinX() >= 0.0f;
}

bool CAScrollView::isReachBoundaryRight()
{
    do
    {
        CC_BREAK_IF(m_fMaximumZoomScale - m_fMinimumZoomScale < FLT_EPSILON);
        CC_BREAK_IF(CAApplication::getApplication()->getTouchDispatcher()->getTouchCount() < 2);
        return false;
    }
    while (0);

    return m_pContainer->getFrame().getMaxX() <= m_obContentSize.width;
}

bool CAScrollView::isReachBoundaryUp()
{
    do
    {
        CC_BREAK_IF(m_fMaximumZoomScale - m_fMinimumZoomScale < FLT_EPSILON);
        CC_BREAK_IF(CAApplication::getApplication()->getTouchDispatcher()->getTouchCount() < 2);
        return false;
    }
    while (0);

    return m_pContainer->getFrame().getMinY() >= 0.0f;
}

bool CAScrollView::isReachBoundaryDown()
{
    do
    {
        CC_BREAK_IF(m_fMaximumZoomScale - m_fMinimumZoomScale < FLT_EPSILON);
        CC_BREAK_IF(CAApplication::getApplication()->getTouchDispatcher()->getTouchCount() < 2);
        return false;
    }
    while (0);

    return m_pContainer->getFrame().getMaxX() <= m_obContentSize.height;
}

void CAScrollView::setTouchEnabledAtSubviews(bool var)
{
    m_bTouchEnabledAtSubviews = var;
    m_pContainer->setTouchEnabled(var);
}

bool CAScrollView::isTouchEnabledAtSubviews()
{
    return m_bTouchEnabledAtSubviews;
}

void CAScrollView::setBounces(bool var)
{
    m_bBounces = var;
    if (var == false)
    {
        m_bBounceHorizontal = false;
        m_bBounceVertical = false;
    }
}

bool CAScrollView::isBounces()
{
    return m_bBounces;
}

void CAScrollView::setShowsScrollIndicators(bool var)
{
    m_bShowsScrollIndicators = var;
    if (!var)
    {
        m_pIndicatorHorizontal->setVisible(var);
        m_pIndicatorVertical->setVisible(var);
    }
    else
    {
        m_pIndicatorHorizontal->setVisible(var && m_bShowsHorizontalScrollIndicator);
        m_pIndicatorVertical->setVisible(var && m_bShowsVerticalScrollIndicator);
    }
}

bool CAScrollView::isShowsScrollIndicators()
{
    return m_bShowsScrollIndicators;
}

void CAScrollView::setShowsHorizontalScrollIndicator(bool var)
{
    m_bShowsHorizontalScrollIndicator = var;
    m_pIndicatorHorizontal->setVisible(var);
}

bool CAScrollView::isShowsHorizontalScrollIndicator()
{
    return m_bShowsHorizontalScrollIndicator;
}

void CAScrollView::setShowsVerticalScrollIndicator(bool var)
{
    m_bShowsVerticalScrollIndicator = var;
    m_pIndicatorVertical->setVisible(var);
}

bool CAScrollView::isShowsVerticalScrollIndicator()
{
    return m_bShowsVerticalScrollIndicator;
}

void CAScrollView::setContentOffset(const DPoint& offset, bool animated)
{
    if (animated)
    {
        m_tInertia = DPointZero;
        m_tCloseToPoint = ccpMult(offset, -1);
        m_tInitialPoint = m_pContainer->getFrameOrigin();
        CAAnimation::schedule(CAAnimation_selector(CAScrollView::closeToPoint), this, 0.25f);
    }
    else
    {
        this->setContainerFrame(ccpMult(offset, -1));
        this->update(1/60.0f);
        if (m_pScrollViewDelegate)
        {
            m_pScrollViewDelegate->scrollViewDidMoved(this);
        }
        CAScheduler::schedule(schedule_selector(CAScrollView::contentOffsetFinish), this, 0, 0, 1/60.0f);
    }
}

void CAScrollView::closeToPoint(float dt, float now, float total)
{
    if (now < total)
    {
        DPoint offSet = ccpSub(m_tCloseToPoint, m_tInitialPoint);
        offSet = ccpMult(offSet, now / total);
        this->setContainerFrame(ccpAdd(m_tInitialPoint, offSet));
        
        if (m_pScrollViewDelegate)
        {
            m_pScrollViewDelegate->scrollViewDidMoved(this);
        }
        this->changedFromPullToRefreshView();
    }
    else
    {
        this->setContainerFrame(m_tCloseToPoint);
        this->update(1/60.0f);
        if (m_pScrollViewDelegate)
        {
            m_pScrollViewDelegate->scrollViewStopMoved(this);
        }
        CAScheduler::schedule(schedule_selector(CAScrollView::contentOffsetFinish), this, 0, 0, 1/60.0f);
        
        this->hideIndicator();
        m_tCloseToPoint = this->getViewSize();
        m_tInitialPoint = m_tCloseToPoint;
        this->changedFromPullToRefreshView();
        this->detectionFromPullToRefreshView();
    }
}

DPoint CAScrollView::getContentOffset()
{
    return ccpMult(m_pContainer->getFrameOrigin(), -1);
}

void CAScrollView::setBackGroundImage(CAImage* image)
{
    CAView::setImage(image);
    
    DRect rect = DRectZero;
    rect.size = image->getContentSize();
    CAView::setImageRect(rect);
}

void CAScrollView::setBackGroundColor(const CAColor4B &color)
{
    CAView::setImage(CAImage::CC_WHITE_IMAGE());
    CAView::setColor(color);
}

void CAScrollView::setZoomScale(float zoom)
{
    zoom = MAX(zoom, m_fMinimumZoomScale);
    zoom = MIN(zoom, m_fMaximumZoomScale);
    m_fZoomScale = zoom;
    m_pContainer->setScale(m_fZoomScale);
}

void CAScrollView::setContentSize(const CrossApp::DSize &var)
{
    CAView::setContentSize(var);
    
    DSize viewSize = this->getViewSize();
    viewSize.width = MAX(m_obContentSize.width, viewSize.width);
    viewSize.height = MAX(m_obContentSize.height, viewSize.height);
    this->setViewSize(viewSize);
    
    CC_RETURN_IF(m_pContainer == NULL);
    
    DPoint point = m_pContainer->getFrameOrigin();
    this->getScrollWindowNotOutPoint(point);
    this->setContainerFrame(point);
    
    this->updateIndicator();
    this->update(0);
}

void CAScrollView::setContainerFrame(const DPoint& point, const DSize& size)
{
    if (!size.equals(DSizeZero))
    {
        DRect rect;
        rect.origin = point;
        rect.size = size;
        m_pContainer->setFrame(rect);
    }
    else
    {
        m_pContainer->setFrameOrigin(point);
    }
    
    DRect rect = m_pContainer->getFrame();
    
}

bool CAScrollView::ccTouchBegan(CATouch *pTouch, CAEvent *pEvent)
{
    do
    {
        CC_BREAK_IF(m_vTouches.size() > 2);
        
        if (!m_vTouches.contains(pTouch))
        {
            m_vTouches.pushBack(pTouch);
        }
        
        if (m_vTouches.size() == 1)
        {
            CAAnimation::unschedule(CAAnimation_selector(CAScrollView::closeToPoint), this);
            this->stopDeaccelerateScroll();
            m_tCloseToPoint = this->getViewSize();
            m_tInitialPoint = m_tCloseToPoint;
            m_pContainer->setAnchorPoint(DPoint(0.5f, 0.5f));
            CAScheduler::schedule(schedule_selector(CAScrollView::updatePointOffset), this, 0);
        }
        else if (m_vTouches.size() == 2)
        {
            CATouch* touch0 = dynamic_cast<CATouch*>(m_vTouches.at(0));
            CATouch* touch1 = dynamic_cast<CATouch*>(m_vTouches.at(1));

            m_fTouchLength = ccpDistance(this->convertToNodeSpace(touch0->getLocation()) ,
                                         this->convertToNodeSpace(touch1->getLocation()));
            
            DPoint mid_point = ccpMidpoint(m_pContainer->convertToNodeSpace(touch0->getLocation()),
                                            m_pContainer->convertToNodeSpace(touch1->getLocation()));
            
            m_pContainer->setAnchorPointInPoints(mid_point);

            if (m_pScrollViewDelegate)
            {
                m_pScrollViewDelegate->scrollViewDidZoom(this);
            }
            m_bZooming = true;
            m_tPointOffset.clear();
            CAScheduler::unschedule(schedule_selector(CAScrollView::updatePointOffset), this);
        }
        
        return true;
    }
    while (0);

    return false;
}

void CAScrollView::ccTouchMoved(CATouch *pTouch, CAEvent *pEvent)
{
    CC_RETURN_IF(m_bPCMode);
    CC_RETURN_IF(m_vTouches.contains(pTouch) == false);
    DPoint p_container = m_pContainer->getFrameOrigin();
    DPoint p_off = DPointZero;
    
    if (m_vTouches.size() == 1)
    {
        p_off = ccpSub(this->convertToNodeSpace(pTouch->getLocation()),
                       this->convertToNodeSpace(pTouch->getPreviousLocation()));
        
        DPoint off = p_off;
        if (off.getLength() <= _px(5))
        {
            off = DPointZero;
        }
        
        m_tPointOffset.push_back(off);
    }
    else if (m_vTouches.size() == 2)
    {
        CATouch* touch0 = dynamic_cast<CATouch*>(m_vTouches.at(0));
        CATouch* touch1 = dynamic_cast<CATouch*>(m_vTouches.at(1));
        DPoint mid_point = ccpMidpoint(this->convertToNodeSpace(touch0->getLocation()),
                                        this->convertToNodeSpace(touch1->getLocation()));
        p_off = ccpSub(mid_point, ccpAdd(p_container, m_pContainer->getAnchorPointInPoints() * m_fZoomScale));
        
        if (m_fMinimumZoomScale < m_fMaximumZoomScale)
        {
            float touch_lenght = ccpDistance(this->convertToNodeSpace(touch0->getLocation()) ,
                                             this->convertToNodeSpace(touch1->getLocation()));
            float scale_off = _dip(touch_lenght - m_fTouchLength) * 0.0020f;
            
            m_fZoomScale = m_pContainer->getScale();
            m_fZoomScale += m_fZoomScale * scale_off;
            
            m_fZoomScale = MIN(m_fZoomScale, m_fMaximumZoomScale);
            m_fZoomScale = MAX(m_fZoomScale, m_fMinimumZoomScale);
            
            m_pContainer->setScale(m_fZoomScale);
            m_fTouchLength = touch_lenght;
        }
    }

    if (m_bBounces)
    {
        DSize size = this->getBounds().size;
        DPoint curr_point = m_pContainer->getFrameOrigin();
        DPoint relust_point = curr_point;
        this->getScrollWindowNotOutPoint(relust_point);
        
        float lenght_x = fabsf(curr_point.x - relust_point.x);
        float lenght_y = fabsf(curr_point.y - relust_point.y);
        
        DPoint scale = DPoint(1.0f, 1.0f);
        
        if (!(lenght_x < FLT_EPSILON))
        {
            scale.x = (0.5f - MIN(lenght_x / size.width, 0.5f));
            p_off.x *= scale.x;
        }
        
        if (!(lenght_y < FLT_EPSILON))
        {
            scale.y = (0.5f - MIN(lenght_y / size.height, 0.5f));
            p_off.y *= scale.y;
        }
    }
    
    p_container = ccpAdd(p_container, p_off);

    if (m_bBounces == false)
    {
        this->getScrollWindowNotOutPoint(p_container);
    }
    else
    {
        if (m_bBounceHorizontal == false)
        {
            p_container.x = this->getScrollWindowNotOutHorizontal(p_container.x);
        }
        
        if (m_bBounceVertical == false)
        {
            p_container.y = this->getScrollWindowNotOutVertical(p_container.y);
        }
    }
    
    if (p_container.equals(m_pContainer->getFrameOrigin()) == false)
    {
        this->setContainerFrame(p_container);
        this->showIndicator();
        
        if (m_bTracking == false)
        {
            if (m_pScrollViewDelegate)
            {
                m_pScrollViewDelegate->scrollViewWillBeginDragging(this);
            }
            m_bTracking = true;
        }
        
        if (m_pScrollViewDelegate)
        {
            m_pScrollViewDelegate->scrollViewDidScroll(this);
            m_pScrollViewDelegate->scrollViewDragging(this);
            m_pScrollViewDelegate->scrollViewDidMoved(this);
        }
    }
    
    this->changedFromPullToRefreshView();
}

void CAScrollView::ccTouchEnded(CATouch *pTouch, CAEvent *pEvent)
{
    CC_RETURN_IF(m_vTouches.contains(pTouch) == false);
    if (m_vTouches.size() == 1)
    {
        DPoint p = DPointZero;
        CAScheduler::unschedule(schedule_selector(CAScrollView::updatePointOffset), this);
        if (m_tPointOffset.size() > 0)
        {
            for (unsigned int i=0; i<m_tPointOffset.size(); i++)
            {
                p = ccpAdd(p, m_tPointOffset.at(i));
            }
            p = p/m_tPointOffset.size();
        }
        m_tInertia = p;
        m_tPointOffset.clear();
        
        if (!m_tInertia.equals(DPointZero))
        {
            if (m_pScrollViewDelegate)
            {
                m_pScrollViewDelegate->scrollViewDidScroll(this);
            }
        }
        
        this->startDeaccelerateScroll();
        
        if (m_pScrollViewDelegate && m_bTracking == false)
        {
            m_pScrollViewDelegate->scrollViewTouchUpWithoutMoved(this, this->convertToNodeSpace(pTouch->getLocation()));
        }
        
        if (m_pScrollViewDelegate)
        {
            m_pScrollViewDelegate->scrollViewDidEndDragging(this);
        }
        m_bTracking = false;
        
        this->detectionFromPullToRefreshView();
    }
    else if (m_vTouches.size() == 2)
    {
        m_bZooming = false;
    }
    m_vTouches.eraseObject(pTouch);
}

void CAScrollView::ccTouchCancelled(CATouch *pTouch, CAEvent *pEvent)
{
    CC_RETURN_IF(m_vTouches.contains(pTouch) == false);
    if (m_vTouches.size() == 1)
    {
        m_tPointOffset.clear();
        
        if (m_pScrollViewDelegate)
        {
            m_pScrollViewDelegate->scrollViewDidEndDragging(this);
        }
        m_bTracking = false;
    }
    else if (m_vTouches.size() == 2)
    {
        m_bZooming = false;
    }
    m_vTouches.eraseObject(pTouch);
    this->stopDeaccelerateScroll();
    this->hideIndicator();
}

void CAScrollView::mouseScrollWheel(CATouch* pTouch, float off_x, float off_y, CAEvent* pEvent)
{
    DPoint p_container = m_pContainer->getFrameOrigin();
    p_container = ccpAdd(p_container, DPoint(off_x, off_y) * 10);
    this->getScrollWindowNotOutPoint(p_container);
    this->setContainerFrame(p_container);
    this->showIndicator();
    
    if (m_pScrollViewDelegate)
    {
        m_pScrollViewDelegate->scrollViewDidScroll(this);
        m_pScrollViewDelegate->scrollViewDidMoved(this);
    }
}

void CAScrollView::switchPCMode(bool var)
{
    m_bPCMode = var;
    this->setMouseScrollWheelEnabled(m_bPCMode);
    this->setPriorityScroll(!m_bPCMode);
    if (m_pIndicatorHorizontal)
    {
        m_pIndicatorHorizontal->switchPCMode(m_bPCMode);
    }
    if (m_pIndicatorVertical)
    {
        m_pIndicatorVertical->switchPCMode(m_bPCMode);
    }
}

void CAScrollView::updatePointOffset(float dt)
{
    CC_RETURN_IF(m_vTouches.size() == 0);
    CC_RETURN_IF(m_bScrollEnabled == false);
    CATouch* pTouch = dynamic_cast<CATouch*>(m_vTouches.at(0));
    DPoint p_off = ccpSub(this->convertToNodeSpace(pTouch->getLocation()),
                   this->convertToNodeSpace(pTouch->getPreviousLocation()));

    if (p_off.getLength() <= _px(5))
    {
        p_off = DPointZero;
    }
    
    m_tPointOffset.push_back(p_off);
    
    while (m_tPointOffset.size() > 3)
    {
        m_tPointOffset.pop_front();
    }
}

void CAScrollView::stopDeaccelerateScroll()
{
    CAScheduler::unschedule(schedule_selector(CAScrollView::deaccelerateScrolling), this);
    m_tInertia = DPointZero;
    m_bDecelerating = false;
    
    if (m_bTouchEnabledAtSubviews)
    {
        m_pContainer->setTouchEnabled(true);
    }
}

void CAScrollView::startDeaccelerateScroll()
{
    CAAnimation::unschedule(CAAnimation_selector(CAScrollView::closeToPoint), this);
    CAScheduler::unschedule(schedule_selector(CAScrollView::deaccelerateScrolling), this);
    CAScheduler::unschedule(schedule_selector(CAScrollView::update), this);
    CAScheduler::schedule(schedule_selector(CAScrollView::deaccelerateScrolling), this, 1/60.0f);
    CAScheduler::schedule(schedule_selector(CAScrollView::update), this, 1/60.0f);
    m_bDecelerating = true;
    if (m_bTouchEnabledAtSubviews)
    {
        m_pContainer->setTouchEnabled(false);
    }
}

void CAScrollView::deaccelerateScrolling(float dt)
{
    dt = MIN(dt, 1/30.0f);
    dt = MAX(dt, 1/100.0f);

    if (m_tInertia.getLength() > maxSpeedCache(dt))
    {
        m_tInertia = ccpMult(m_tInertia, maxSpeedCache(dt) / m_tInertia.getLength());
    }
    
    DPoint speed = DPointZero;
    if (m_tInertia.getLength() > maxSpeed(dt))
    {
        speed = ccpMult(m_tInertia, maxSpeed(dt) / m_tInertia.getLength());
    }
    else
    {
        speed = m_tInertia;
    }
    
    DPoint point = m_pContainer->getFrameOrigin();

    if (m_bBounces)
    {
        DPoint resilience = DPointZero;
        DSize size = this->getBounds().size;
        DPoint curr_point = m_pContainer->getFrameOrigin();
        DPoint relust_point = curr_point;
        this->getScrollWindowNotOutPoint(relust_point);
        float off_x = relust_point.x - curr_point.x;
        float off_y = relust_point.y - curr_point.y;
        
        if (fabsf(off_x) > FLT_EPSILON)
        {
            resilience.x = off_x / size.width;
        }
        
        if (fabsf(off_y) > FLT_EPSILON)
        {
            resilience.y = off_y / size.height;
        }
        
        resilience = ccpMult(resilience, maxSpeed(dt));
        
        if (speed.getLength() < resilience.getLength())
        {
            speed = resilience;
            m_tInertia = DPointZero;
        }
    }
    
    if (speed.getLength() < 0.25f)
    {
        m_tInertia = DPointZero;
        this->getScrollWindowNotOutPoint(point);
        this->setContainerFrame(point);
        this->update(0);
        this->hideIndicator();
        this->stopDeaccelerateScroll();
        
        if (m_pScrollViewDelegate)
        {
            m_pScrollViewDelegate->scrollViewStopMoved(this);
        }
        
    }
    else
    {
        point = ccpAdd(point, speed);

        if (this->isScrollWindowNotMaxOutSide(point))
        {
            m_tInertia = DPointZero;
        }
        
        if (m_bBounces == false)
        {
            this->getScrollWindowNotOutPoint(point);
        }
        else
        {
            if (m_bBounceHorizontal == false)
            {
                point.x = this->getScrollWindowNotOutHorizontal(point.x);
            }
            
            if (m_bBounceVertical == false)
            {
                point.y = this->getScrollWindowNotOutVertical(point.y);
            }
        }

        if (point.equals(m_pContainer->getFrameOrigin()))
        {
            m_tInertia = DPointZero;
        }
        else
        {
            this->showIndicator();
            this->setContainerFrame(point);
        }
        
        if (fabsf(m_tInertia.x) > _px(16))
        {
            m_tInertia.x = m_tInertia.x * (1 - decelerationRatio(dt));
        }
        else if (fabsf(m_tInertia.x) > FLT_EPSILON)
        {
            m_tInertia.x = MAX((fabsf(m_tInertia.x) - _px(0.5f)), 0) * fabsf(m_tInertia.x) / m_tInertia.x;
        }
        
        if (fabsf(m_tInertia.y) > _px(16))
        {
            m_tInertia.y = m_tInertia.y * (1 - decelerationRatio(dt));
        }
        else if (fabsf(m_tInertia.y) > FLT_EPSILON)
        {
            m_tInertia.y = MAX((fabsf(m_tInertia.y) - _px(0.5f)), 0) * fabsf(m_tInertia.y) / m_tInertia.y;
        }
        
        if (m_pScrollViewDelegate)
        {
            m_pScrollViewDelegate->scrollViewDidMoved(this);
        }
    }
}

void CAScrollView::updateIndicator()
{
    if (m_pIndicatorHorizontal == NULL)
    {
        m_pIndicatorHorizontal = CAIndicator::create(CAIndicator::CAIndicatorTypeHorizontal, this);
        m_vChildInThis.pushBack(m_pIndicatorHorizontal);
        this->insertSubview(m_pIndicatorHorizontal, 1);
    }
    
    if (m_pIndicatorVertical == NULL)
    {
        m_pIndicatorVertical = CAIndicator::create(CAIndicator::CAIndicatorTypeVertical, this);
        m_vChildInThis.pushBack(m_pIndicatorVertical);
        this->insertSubview(m_pIndicatorVertical, 1);
    }
    
    const char indicatorSize = _px(6);

    const DRect indicatorHorizontalFrame = DRect(indicatorSize * 2,
                                                   this->getBounds().size.height - indicatorSize * 2,
                                                   this->getBounds().size.width - indicatorSize * 4,
                                                   indicatorSize);
    m_pIndicatorHorizontal->setFrame(indicatorHorizontalFrame);
    
    const DRect indicatorVerticalFrame = DRect(this->getBounds().size.width - indicatorSize * 2,
                                                 indicatorSize * 2,
                                                 indicatorSize,
                                                 this->getBounds().size.height - indicatorSize * 4);
    m_pIndicatorVertical->setFrame(indicatorVerticalFrame);
}

void CAScrollView::showIndicator()
{
    if (!CAScheduler::isScheduled(schedule_selector(CAScrollView::update), this))
    {
        CAScheduler::schedule(schedule_selector(CAScrollView::update), this, 1/60.0f);
    }
    if (m_pIndicatorHorizontal)
    {
        m_pIndicatorHorizontal->setHide(false);
    }
    if (m_pIndicatorVertical)
    {
        m_pIndicatorVertical->setHide(false);
    }
    CAScrollView::update(0);
}

void CAScrollView::hideIndicator()
{
    CAScheduler::unschedule(schedule_selector(CAScrollView::update), this);
    
    if (!m_bPCMode)
    {
        m_pIndicatorHorizontal->setHide(true);
        m_pIndicatorVertical->setHide(true);
    }
}

void CAScrollView::update(float dt)
{
    if (m_pIndicatorHorizontal)
    {
        m_pIndicatorHorizontal->setIndicator(m_obContentSize, m_pContainer->getFrame());
    }
    if (m_pIndicatorVertical)
    {
        m_pIndicatorVertical->setIndicator(m_obContentSize, m_pContainer->getFrame());
    }
}

void CAScrollView::getScrollWindowNotOutPoint(DPoint& point)
{
    point.x = this->getScrollWindowNotOutHorizontal(point.x);
    point.y = this->getScrollWindowNotOutVertical(point.y);
}

float CAScrollView::getScrollWindowNotOutHorizontal(float x)
{
    DSize size = this->getBounds().size;
    DSize cSize = m_pContainer->getFrame().size;
    
    if (cSize.width >= size.width)
    {
        x = MIN(x, 0) ;
        x = MAX(x, size.width - cSize.width);
    }
    else
    {
        x = size.width/2 - cSize.width/2;
    }
    
    return x;
}

float CAScrollView::getScrollWindowNotOutVertical(float y)
{
    DSize size = this->getBounds().size;
    DSize cSize = m_pContainer->getFrame().size;
    
    if (cSize.height >= size.height)
    {
        float y_max = this->isHeaderRefreshing() ? _px(128) : 0.0f;
        float y_min = this->isFooterRefreshing() ? size.height - cSize.height - _px(128) : size.height - cSize.height;
        
        y = MIN(y, y_max);
        y = MAX(y, y_min);
    }
    else
    {
        y = size.height/2 - cSize.height/2;
    }

    return y;
}

bool CAScrollView::isScrollWindowNotOutSide()
{
    DPoint point = m_pContainer->getFrameOrigin();
    this->getScrollWindowNotOutPoint(point);
    
    return !point.equals(m_pContainer->getFrameOrigin());
}


bool CAScrollView::isScrollWindowNotMaxOutSide(const DPoint& point)
{
    DPoint p = point;
    this->getScrollWindowNotOutPoint(p);
    
    return (fabsf(p.x - point.x) > maxBouncesLenght()
            ||
            fabsf(p.y - point.y) > maxBouncesLenght());
}

void CAScrollView::setHeaderRefreshView(CrossApp::CAPullToRefreshView *var)
{
    this->removeSubview(m_pHeaderRefreshView);
    CC_SAFE_RETAIN(var);
    CC_SAFE_RELEASE(m_pHeaderRefreshView);
    m_pHeaderRefreshView = var;
}

CAPullToRefreshView* CAScrollView::getHeaderRefreshView()
{
    return m_pHeaderRefreshView;
}

void CAScrollView::setFooterRefreshView(CrossApp::CAPullToRefreshView *var)
{
    this->removeSubview(m_pFooterRefreshView);
    CC_SAFE_RETAIN(var);
    CC_SAFE_RELEASE(m_pFooterRefreshView);
    m_pFooterRefreshView = var;
}

CAPullToRefreshView* CAScrollView::getFooterRefreshView()
{
    return m_pFooterRefreshView;
}

void CAScrollView::endHeaderRefresh()
{
    if (m_pHeaderRefreshView)
    {
        m_pHeaderRefreshView->setPullToRefreshStateType(CAPullToRefreshView::CAPullToRefreshStateNone);
    }
}

void CAScrollView::endFooterRefresh()
{
    if (m_pFooterRefreshView)
    {
        m_pFooterRefreshView->setPullToRefreshStateType(CAPullToRefreshView::CAPullToRefreshStateNone);
    }
}

void CAScrollView::layoutPullToRefreshView()
{
    DSize viewSize = this->getViewSize();
    if (m_pHeaderRefreshView)
    {
        m_pHeaderRefreshView->setFrame(DRect(0, -128.0f, viewSize.width, 128.0f));
        if (m_pHeaderRefreshView->getSuperview() == NULL)
        {
            m_pContainer->addSubview(m_pHeaderRefreshView);
        }
        if (m_pHeaderRefreshView->isLayoutFinish() == false)
        {
            m_pHeaderRefreshView->startLayout();
        }
        this->endHeaderRefresh();
    }

    if (m_pFooterRefreshView)
    {
        m_pFooterRefreshView->setFrame(DRect(0, viewSize.height, viewSize.width, 128.0f));
        if (m_pFooterRefreshView->getSuperview() == NULL)
        {
            m_pContainer->addSubview(m_pFooterRefreshView);
        }
        if (m_pFooterRefreshView->isLayoutFinish() == false)
        {
            m_pFooterRefreshView->startLayout();
        }
        this->endFooterRefresh();
    }
}

void CAScrollView::changedFromPullToRefreshView()
{
    DRect rect = this->getBounds();
	rect.origin = this->getContentOffset();
    
    if (m_pHeaderRefreshView)
    {
        if (m_pHeaderRefreshView->getFrame().origin.y < rect.origin.y)
        {
            m_pHeaderRefreshView->setPullToRefreshStateType(CAPullToRefreshView::CAPullToRefreshStateNormal);
        }
        else
        {
            m_pHeaderRefreshView->setPullToRefreshStateType(CAPullToRefreshView::CAPullToRefreshStatePulling);
        }
    }
    if (m_pFooterRefreshView)
    {
        if (m_pFooterRefreshView->getFrame().origin.y + m_pFooterRefreshView->getFrame().size.height > rect.origin.y + rect.size.height)
        {
            m_pFooterRefreshView->setPullToRefreshStateType(CAPullToRefreshView::CAPullToRefreshStateNormal);
        }
        else
        {
            m_pFooterRefreshView->setPullToRefreshStateType(CAPullToRefreshView::CAPullToRefreshStatePulling);
        }
    } 
}

void CAScrollView::detectionFromPullToRefreshView()
{
    if (m_pHeaderRefreshView && m_pHeaderRefreshView->isCanRefresh())
    {
        this->stopDeaccelerateScroll();
        this->setContentOffset(DPoint(0, -_px(128)), true);
        m_pHeaderRefreshView->setPullToRefreshStateType(CAPullToRefreshView::CAPullToRefreshStateRefreshing);
        if (m_pScrollViewDelegate)
        {
            m_pScrollViewDelegate->scrollViewHeaderBeginRefreshing(this);
        }
    }
    
    if (m_pFooterRefreshView && m_pFooterRefreshView->isCanRefresh())
    {
        this->stopDeaccelerateScroll();
        this->setContentOffset(DPoint(0, this->getViewSize().height - this->getBounds().size.height + _px(128)), true);
        m_pFooterRefreshView->setPullToRefreshStateType(CAPullToRefreshView::CAPullToRefreshStateRefreshing);
        
        if (m_pScrollViewDelegate)
        {
            m_pScrollViewDelegate->scrollViewFooterBeginRefreshing(this);
        }
    }
}

void CAScrollView::startPullToHeaderRefreshView()
{
    this->setContentOffset(DPoint(0, -128.0f), true);
}

bool CAScrollView::isHeaderRefreshing()
{
    return m_pHeaderRefreshView && m_pHeaderRefreshView->isRefreshing();
}

bool CAScrollView::isFooterRefreshing()
{
    return m_pFooterRefreshView && m_pFooterRefreshView->isRefreshing();
}

#pragma CAIndicator

CAIndicator::CAIndicator(const CAIndicatorType& type, CAScrollView* var)
:m_pIndicator(NULL)
,m_eType(type)
,m_bPCMode(false)
,m_pMyScrollView(var)
,m_bTouch(false)
{
    this->setHaveNextResponder(false);
}

CAIndicator::~CAIndicator()
{

}

void CAIndicator::onEnterTransitionDidFinish()
{
    CAView::onEnterTransitionDidFinish();
}

void CAIndicator::onExitTransitionDidStart()
{
    CAView::onExitTransitionDidStart();
    this->setAlpha(0.0f);
}

CAIndicator* CAIndicator::create(const CAIndicatorType& type, CAScrollView* var)
{
    CAIndicator* indicator = new CAIndicator(type, var);
    if (indicator && indicator->init())
    {
        indicator->autorelease();
        return indicator;
    }
    CC_SAFE_DELETE(indicator);
    return NULL;
}

bool CAIndicator::init()
{
    if (!CAView::init())
    {
        return false;
    }
    this->setColor(CAColor_clear);
    CAImage* image = CAImage::create("source_material/indicator.png");
    
    m_pIndicator = CAScale9ImageView::createWithImage(image);
    this->addSubview(m_pIndicator);
    this->setAlpha(0.0f);
    
    return true;
}

void CAIndicator::setIndicator(const DSize& parentSize, const DRect& childrenFrame)
{
    CC_RETURN_IF(!this->isVisible());
    CC_RETURN_IF(m_bTouch);
    
    CAScale9ImageView* indicator = dynamic_cast<CAScale9ImageView*>(m_pIndicator);
    
    if (m_eType == CAIndicatorTypeHorizontal)
    {
        float size_scale_x = parentSize.width / childrenFrame.size.width;
        size_scale_x = MIN(size_scale_x, 1.0f);
        
        float lenght_scale_x = parentSize.width / 2 - childrenFrame.origin.x;
        lenght_scale_x /= childrenFrame.size.width;
        
        DRect rect;
        rect.size = m_obContentSize;
        rect.size.width *= size_scale_x;
        
        if (lenght_scale_x < size_scale_x / 2)
        {
            rect.size.width *= lenght_scale_x / (size_scale_x / 2);
        }
        if (1 - lenght_scale_x < size_scale_x / 2)
        {
            rect.size.width *= (1 - lenght_scale_x) / (size_scale_x / 2);
        }
        rect.size.width = MAX(rect.size.height, rect.size.width);
        
        rect.origin = m_obContentSize;
        rect.origin.y *= 0.5f;
        rect.origin.x *= lenght_scale_x;
        rect.origin.x = MAX(rect.origin.x, rect.size.width/2);
        rect.origin.x = MIN(rect.origin.x, m_obContentSize.width - rect.size.width/2);
        
        indicator->setCenter(rect);
    }
    else if (m_eType == CAIndicatorTypeVertical)
    {
        float size_scale_y = parentSize.height / childrenFrame.size.height;
        size_scale_y = MIN(size_scale_y, 1.0f);
        
        float lenght_scale_y = parentSize.height / 2 - childrenFrame.origin.y;
        lenght_scale_y /= childrenFrame.size.height;
        
        DRect rect;
        rect.size = m_obContentSize;
        rect.size.height *= size_scale_y;
        
        if (lenght_scale_y < size_scale_y / 2)
        {
            rect.size.height *= lenght_scale_y / (size_scale_y / 2);
        }
        if (1 - lenght_scale_y < size_scale_y / 2)
        {
            rect.size.height *= (1 - lenght_scale_y) / (size_scale_y / 2);
        }
        rect.size.height = MAX(rect.size.height, rect.size.width);

        rect.origin = m_obContentSize;
        rect.origin.x *= 0.5f;
        rect.origin.y *= lenght_scale_y;
        rect.origin.y = MAX(rect.origin.y, rect.size.height/2);
        rect.origin.y = MIN(rect.origin.y, m_obContentSize.height - rect.size.height/2);
        
        indicator->setCenter(rect);
    }
}

void CAIndicator::setHide(bool var)
{
    if (var == false)
    {
        CC_RETURN_IF(fabs(1.0f-this->getAlpha()) < FLT_EPSILON);
        
        this->setAlpha(1.0f);
    }
    else
    {
        CAViewAnimation::areBeginAnimationsWithID(m_s__StrID);
        
        CC_RETURN_IF(1.0f-this->getAlpha() > FLT_EPSILON);
        CAViewAnimation::beginAnimations(m_s__StrID, NULL);
        CAViewAnimation::setAnimationDuration(0.3f);
        CAViewAnimation::setAnimationDelay(0.2f);
        this->setAlpha(0.0f);
        CAViewAnimation::commitAnimations();
    }
}

bool CAIndicator::ccTouchBegan(CATouch *pTouch, CAEvent *pEvent)
{
    m_bTouch = (m_pIndicator->getBounds().containsPoint(m_pIndicator->convertTouchToNodeSpace(pTouch)));
    return m_bTouch;
}

void CAIndicator::ccTouchMoved(CATouch *pTouch, CAEvent *pEvent)
{
    DPoint point = m_pIndicator->getCenterOrigin();
    DPoint p_off = ccpSub(this->convertToNodeSpace(pTouch->getLocation()),
                           this->convertToNodeSpace(pTouch->getPreviousLocation()));
    
    DSize size = m_pIndicator->getFrame().size;
    
    if (m_eType == CAIndicatorTypeHorizontal)
    {
        point.x += p_off.x;
        point.x = MAX(point.x, size.width/2);
        point.x = MIN(point.x, m_obContentSize.width - size.width/2);
    }
    else
    {
        point.y += p_off.y;
        point.y = MAX(point.y, size.height/2);
        point.y = MIN(point.y, m_obContentSize.height - size.height/2);
    }
    m_pIndicator->setCenterOrigin(point);
    
    DSize indictor_size = ccpSub(m_obContentSize, size);
    DPoint indictor_point = ccpSub(point, ccpMult(size, 0.5f));
    DSize view_size = ccpSub(m_pMyScrollView->getViewSize(), m_pMyScrollView->getBounds().size);
    DPoint view_offset = m_pMyScrollView->getContentOffset();
    if (m_eType == CAIndicatorTypeHorizontal)
    {
        view_offset.x = indictor_point.x / indictor_size.width * view_size.width;
    }
    else
    {
        view_offset.y = indictor_point.y / indictor_size.height * view_size.height;
    }
    m_pMyScrollView->setContentOffset(view_offset, false);
}

void CAIndicator::ccTouchEnded(CATouch *pTouch, CAEvent *pEvent)
{
    m_bTouch = false;
}

void CAIndicator::ccTouchCancelled(CATouch *pTouch, CAEvent *pEvent)
{
    m_bTouch = false;
}

void CAIndicator::mouseMoved(CATouch* pTouch, CAEvent* pEvent)
{
    if (m_pIndicator->getBounds().containsPoint(m_pIndicator->convertTouchToNodeSpace(pTouch)))
    {
        m_pIndicator->setColor(CAColor_gray);
    }
    else
    {
        m_pIndicator->setColor(CAColor_white);
    }
}

void CAIndicator::mouseMovedOutSide(CATouch* pTouch, CAEvent* pEvent)
{
    m_pIndicator->setColor(CAColor_white);
}

void CAIndicator::switchPCMode(bool var)
{
    m_bPCMode = var;
    this->setPriorityScroll(m_bPCMode);
    this->setHaveNextResponder(!m_bPCMode);
    this->setMouseMovedEnabled(m_bPCMode);
    this->setHide(false);
}

NS_CC_END