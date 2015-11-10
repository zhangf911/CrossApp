//
//  CABar.cpp
//  CrossApp
//
//  Created by Li Yuanfeng on 14-4-14.
//  Copyright (c) 2014 http://9miao.com All rights reserved.
//

#include "CABar.h"
#include "view/CAScale9ImageView.h"
#include "view/CALabel.h"
#include "view/CAScrollView.h"
#include "basics/CAApplication.h"
#include "support/CAPointExtension.h"
#include "dispatcher/CATouch.h"
#include "animation/CAViewAnimation.h"

NS_CC_BEGIN

#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS

#define NAVBAR_HEIGHT _px(128)
#define TABBAR_HEIGHT _px(138)

#else

#define NAVBAR_HEIGHT _px(88)
#define TABBAR_HEIGHT _px(98)

#endif

#define NAVBAR_CONTENT_HEIGHT _px(88)
#define TABBAR_CONTENT_HEIGHT _px(98)


#pragma CANavigationBar

CANavigationBar::CANavigationBar()
:m_pContentView(NULL)
,m_pTitle(NULL)
,m_pDelegate(NULL)
,m_pBackGroundView(NULL)
,m_cTitleColor(CAColor_white)
,m_cButtonColor(CAColor_white)
,m_pItem(NULL)
{

}

CANavigationBar::~CANavigationBar()
{
    CC_SAFE_RELEASE_NULL(m_pItem);
    CC_SAFE_RELEASE(m_pBackGroundView);
}

bool CANavigationBar::init(const DSize& size)
{
    this->setColor(CAColor_clear);
    DSize winSize = CAApplication::getApplication()->getWinSize();
    DSize contentSize;
    contentSize.width = size.width > FLT_EPSILON ? MIN(winSize.width, size.width) : winSize.width;
    contentSize.height = size.height > FLT_EPSILON ? size.height : NAVBAR_HEIGHT;
    this->setFrame(DRect(0, 0, contentSize.width, contentSize.height));
    
    DRect rect = this->getBounds();
    rect.size.height = NAVBAR_CONTENT_HEIGHT;
    rect.origin.y = NAVBAR_HEIGHT - NAVBAR_CONTENT_HEIGHT;
    m_pContentView = new CAView();
    m_pContentView->setFrame(rect);
    this->addSubview(m_pContentView);
    
    return true;
}

CANavigationBar* CANavigationBar::create(const DSize& size)
{
    CANavigationBar* nav = new CANavigationBar();
    if (nav && nav->init(size))
    {
        nav->autorelease();
        return nav;
    }
    CC_SAFE_DELETE(nav);
    return NULL;
}

void CANavigationBar::onEnterTransitionDidFinish()
{
    CAView::onEnterTransitionDidFinish();

    if (m_pBackGroundView == NULL || m_pBackGroundView->getSuperview() == NULL)
    {
        this->showBackGround();
    }
    this->updateNavigationBar();
}

void CANavigationBar::onExitTransitionDidStart()
{
    CAView::onExitTransitionDidStart();
}

void CANavigationBar::setItem(CANavigationBarItem* item)
{
    if (item == NULL)
    {
        item = CANavigationBarItem::create("The Title");
    }
    CC_SAFE_RETAIN(item);
    CC_SAFE_RELEASE_NULL(m_pItem);
    m_pItem = item;
    this->updateNavigationBar();
}

void CANavigationBar::setBackGroundView(CAView* var)
{
    var->setTouchEnabled(false);
    this->removeSubview(m_pBackGroundView);
    CC_SAFE_RETAIN(var);
    CC_SAFE_RELEASE(m_pBackGroundView);
    m_pBackGroundView = var;
    CC_RETURN_IF(!m_bRunning);
    this->showBackGround();
}

void CANavigationBar::setTitleColor(const CAColor4B& color)
{
    m_cTitleColor = color;
    CC_RETURN_IF(!m_bRunning);
    this->showTitle();
}

void CANavigationBar::setButtonColor(const CAColor4B& color)
{
    m_cButtonColor = color;
    CC_RETURN_IF(!m_bRunning);
    this->showLeftButton();
    this->showRightButton();
}

void CANavigationBar::updateNavigationBar()
{
    CC_RETURN_IF(m_pItem == NULL);
    this->showTitle();
    this->showLeftButton();
    this->showRightButton();
}

void CANavigationBar::showBackGround()
{
    if (m_pBackGroundView == NULL)
    {
        m_pBackGroundView = CAScale9ImageView::createWithImage(CAImage::create("source_material/navigation_bg.png"));
        CC_SAFE_RETAIN(m_pBackGroundView);
    }
    m_pBackGroundView->setFrame(this->getBounds());
    this->insertSubview(m_pBackGroundView, -1);
}

void CANavigationBar::showTitle()
{
    DRect rect;
    rect.size = m_pContentView->getBounds().size;
    rect.origin = rect.size/2;
    rect.size.width = rect.size.width - rect.size.height * 4;

    if (m_pTitle)
    {
        m_pContentView->removeSubview(m_pTitle);
        m_pTitle = NULL;
    }
    
    if (CAView* titleView = m_pItem->getTitleView())
    {
        float aspectRatio = 0;
        if (!titleView->getFrame().size.equals(DSizeZero))
        {
            aspectRatio = titleView->getFrame().size.width / titleView->getFrame().size.height;
        }
        
        rect.size.height *= 2/3.0f;
        rect.size.width = aspectRatio < FLT_EPSILON ? rect.size.width : aspectRatio * rect.size.height;
        titleView->setCenter(rect);
        m_pContentView->addSubview(titleView);
        m_pTitle = titleView;
    }
    else if (CAImage* image = m_pItem->getTitleViewImage())
    {
        float height = MIN(image->getContentSize().height, rect.size.height * 0.75f);
        float width =  height * image->getContentSize().width / image->getContentSize().height;
        width = MIN(rect.size.width, width);
        rect.size = DSize(width, height);
        m_pTitle = CAImageView::createWithImage(image);
        m_pTitle->setCenter(rect);
        m_pContentView->addSubview(m_pTitle);
    }
    else
    {
        CALabel* title = CALabel::createWithCenter(rect);
        title->setTextAlignment(CATextAlignmentCenter);
        title->setVerticalTextAlignmet(CAVerticalTextAlignmentCenter);
        title->setNumberOfLine(1);
		title->setColor(m_cTitleColor);
        title->setFontSize(_px(32));
        m_pContentView->addSubview(title);
        m_pTitle = title;
        
        if (m_pItem)
        {
            std::string str = m_pItem->getTitle();
            ((CALabel*)m_pTitle)->setText(str.c_str());
        }
    }
}

void CANavigationBar::showLeftButton()
{
    std::vector<CAButton*>::iterator itr;
    for (itr = m_pLeftButtons.begin(); itr != m_pLeftButtons.end(); itr++)
    {
        (*itr)->removeFromSuperview();
    }
    m_pLeftButtons.clear();
    
    const CAVector<CAObject*>& buttonItems = m_pItem->getLeftButtonItems();

    DRect rect;
    rect.size.width = _px(80);
    rect.size.height = m_pContentView->getBounds().size.height * 0.8f;
    rect.origin.x = rect.size.width * 0.7f;
    rect.origin.y = m_pContentView->getBounds().size.height * 0.5f;

    for (size_t i=0; i<buttonItems.size(); i++)
    {
        CABarButtonItem* item = dynamic_cast<CABarButtonItem*>(buttonItems.at(i));
        
        rect.size.width = item ? item->getItemWidth() : _px(80);
        rect.origin.x += i * rect.size.width;
        
        CAButton* button = CAButton::createWithCenter(rect, CAButtonTypeCustom);
        m_pContentView->addSubview(button);
        
        if (item == NULL && m_pItem)
        {
            button->setImageForState(CAControlStateNormal, CAImage::create("source_material/btn_left_white.png"));
            button->setImageColorForState(CAControlStateHighlighted, ccc4(255, 255, 200, 255));
            button->addTarget(this, CAControl_selector(CANavigationBar::goBack), CAControlEventTouchUpInSide);
        }
        else if (item)
        {
            if (item->getImage())
            {
                button->setImageForState(CAControlStateNormal, item->getImage());
                if (item->getHighlightedImage())
                {
                    button->setImageForState(CAControlStateHighlighted, item->getHighlightedImage());
                }
                else
                {
                    button->setImageColorForState(CAControlStateHighlighted, ccc4(127, 127, 127, 255));
                }
            }
            else
            {
                button->setTitleForState(CAControlStateNormal, item->getTitle());
                button->setTitleColorForState(CAControlStateNormal, m_cButtonColor);
                button->setTitleForState(CAControlStateHighlighted, item->getTitle());
                button->setTitleColorForState(CAControlStateHighlighted, ccc4(m_cButtonColor.r/2, m_cButtonColor.g/2, m_cButtonColor.b/2, 255));
            }
            
            button->addTarget(item->getTarget(), item->getSel(), CAControlEventTouchUpInSide);
        }
        m_pLeftButtons.push_back(button);
    }
}

void CANavigationBar::showRightButton()
{
    std::vector<CAButton*>::iterator itr;
    for (itr = m_pRightButtons.begin(); itr != m_pRightButtons.end(); itr++)
    {
        (*itr)->removeFromSuperview();
    }
    m_pRightButtons.clear();
    
    const CAVector<CAObject*>& buttonItems = m_pItem->getRightButtonItems();
    
    DRect rect;
    rect.size.width = _px(80);
    rect.size.height = m_pContentView->getBounds().size.height * 0.8f;
    rect.origin.x = m_pContentView->getBounds().size.width - rect.size.width * 0.7f;
    rect.origin.y = m_pContentView->getBounds().size.height * 0.5f;

    for (size_t i=0; i<buttonItems.size(); i++)
    {
        CABarButtonItem* item = dynamic_cast<CABarButtonItem*>(buttonItems.at(i));
        
        rect.size.width = item ? item->getItemWidth() : _px(80);
        rect.origin.x -= i * rect.size.width;
        
        CAButton* button = CAButton::createWithCenter(rect, CAButtonTypeCustom);
        m_pContentView->addSubview(button);

        if (item)
        {
            if (item->getImage())
            {
                button->setImageForState(CAControlStateNormal, item->getImage());
                if (item->getHighlightedImage())
                {
                    button->setImageForState(CAControlStateHighlighted, item->getHighlightedImage());
                }
                else
                {
                    button->setImageColorForState(CAControlStateHighlighted, ccc4(127, 127, 127, 255));
                }
            }
            else
            {
                button->setTitleForState(CAControlStateNormal, item->getTitle());
                button->setTitleColorForState(CAControlStateNormal, m_cButtonColor);
                button->setTitleForState(CAControlStateHighlighted, item->getTitle());
                button->setTitleColorForState(CAControlStateHighlighted, ccc4(m_cButtonColor.r/2, m_cButtonColor.g/2, m_cButtonColor.b/2, 255));
            }
            
            button->addTarget(item->getTarget(), item->getSel(), CAControlEventTouchUpInSide);
        }
        m_pRightButtons.push_back(button);
    }
}

void CANavigationBar::goBack(CAControl* btn, DPoint point)
{
    if (m_pDelegate)
    {
        m_pDelegate->navigationPopViewController(this, true);
    }
}

#pragma CABadgeView

CABadgeView::CABadgeView()
:m_pBackground(NULL)
,m_pTextView(NULL)
{

}

CABadgeView::~CABadgeView()
{
    
}

bool CABadgeView::init()
{
    m_pBackground = CAScale9ImageView::createWithCenter(DRect(0, 0, 46, 46));
    m_pBackground->setCapInsets(DRect(22.5, 22.5, 1, 1));
    m_pBackground->setImage(CAImage::create("source_material/bg_badge.png"));
    this->addSubview(m_pBackground);
    
    m_pTextView = CALabel::createWithCenter(DRect(0, 0, 180, 46));
    m_pTextView->setTextAlignment(CATextAlignmentCenter);
    m_pTextView->setVerticalTextAlignmet(CAVerticalTextAlignmentCenter);
    m_pTextView->setFontSize(30);
    m_pTextView->setColor(CAColor_white);
    m_pTextView->setBold(true);
    this->addSubview(m_pTextView);
    
    this->setScale(_px(1) /1.23f);
    
    return true;
}

void CABadgeView::setBadgeText(const std::string& text)
{
    this->setVisible(!text.empty());
    
    m_pTextView->setCenter(DRect(0, 0, 180, 46));
    m_pTextView->setText(text);
    
    DRect rect = m_pBackground->getCenter();
    rect.size.width = CAImage::getStringWidth("", 30, text);
    if (rect.size.width > 30)
    {
        rect.size.width += 16;
    }
    rect.size.width = MIN(rect.size.width, 196);
    rect.size.width = MAX(rect.size.width, 46);

    m_pBackground->setCenter(rect);
}

void CABadgeView::setContentSize(const DSize& contentSize)
{

}

#pragma CATabBar

CATabBar::CATabBar()
:m_pContentView(NULL)
,m_pBackGroundView(NULL)
,m_pSelectedIndicatorView(NULL)
,m_pBackGroundImage(NULL)
,m_sBackGroundColor(CAColor_white)
,m_pSelectedBackGroundImage(NULL)
,m_sSelectedBackGroundColor(CAColor_white)
,m_pSelectedIndicatorImage(NULL)
,m_sSelectedIndicatorColor(CAColor_white)
,m_pSelectedItem(NULL)
,m_cItemSize(DSizeZero)
,m_nSelectedIndex(-1)
,m_sTitleColor(CAColor_white)
,m_sSelectedTitleColor(ccc4(50, 193, 255, 255))
,m_bShowIndicator(false)
,m_pDelegate(NULL)
,m_eVerticalAlignment(CABarVerticalAlignmentBottom)
{
    
}

CATabBar::~CATabBar()
{
    std::vector<CATabBarItem*>::iterator itr;
    for (itr=m_pItems.begin(); itr!=m_pItems.end(); itr++)
    {
        (*itr)->autorelease();
    }
    m_pItems.clear();
    m_pButtons.clear();
    m_pBadgeViews.clear();
    CC_SAFE_RELEASE_NULL(m_pBackGroundImage);
    CC_SAFE_RELEASE_NULL(m_pSelectedBackGroundImage);
    CC_SAFE_RELEASE_NULL(m_pSelectedIndicatorImage);
    CC_SAFE_RELEASE_NULL(m_pSelectedBackGroundImage);
    CC_SAFE_RELEASE_NULL(m_pSelectedIndicatorImage);
}

bool CATabBar::init(const CAVector<CATabBarItem*>& items, const DSize& size, const CABarVerticalAlignment& var)
{
    if (!CAView::init())
    {
        return false;
    }
    m_eVerticalAlignment = var;
    this->setColor(CAColor_clear);
    this->setItems(items);

    float height;
    switch (m_eVerticalAlignment)
    {
        case CABarVerticalAlignmentBottom:
            height = TABBAR_CONTENT_HEIGHT;
            break;
        case CABarVerticalAlignmentTop:
            height = TABBAR_HEIGHT;
            break;
        default:
            break;
    }
    
    DSize winSize = CAApplication::getApplication()->getWinSize();
    DSize contentSize;
    contentSize.width = size.width > FLT_EPSILON ? MIN(winSize.width, size.width) : winSize.width;
    contentSize.height = size.height > FLT_EPSILON ? size.height : height;
    this->setFrame(DRect(0, 0, contentSize.width, contentSize.height));

    DRect rect = this->getBounds();
    rect.size.width = MIN(rect.size.width, _px(1024));
    rect.size.height = TABBAR_CONTENT_HEIGHT;
    rect.origin.x = (this->getBounds().size.width - rect.size.width) / 2;
    rect.origin.y = height - rect.size.height;
    
    m_pContentView = new CAView();
    m_pContentView->setFrame(rect);
    this->addSubview(m_pContentView);
    m_pContentView->release();
    
    unsigned int count = (unsigned int)m_pItems.size();
    m_cItemSize = m_pContentView->getBounds().size;
    m_cItemSize.width = rect.size.width/count;

    if (m_pButtons.empty())
    {
        for (unsigned int i=0; i<count; i++)
        {
            DRect rect = DRectZero;
            rect.size = m_cItemSize;
            rect.origin.x = m_cItemSize.width * i;
            
            CAButton* btn = CAButton::createWithFrame(rect, CAButtonTypeCustom);
            m_pContentView->addSubview(btn);
            btn->setTag(i);
            btn->addTarget(this, CAControl_selector(CATabBar::setTouchSelected), CAControlEventTouchUpInSide);
            m_pButtons.pushBack(btn);
            
            CABadgeView* badgeView = new CABadgeView();
            badgeView->init();
            badgeView->setCenter(DRect(rect.size.width, _px(25), 0, 0));
            btn->insertSubview(badgeView, 10);
            m_pBadgeViews.pushBack(badgeView);
            badgeView->release();
        }
    }
    if (m_pBackGroundImage == NULL)
    {
        this->setBackGroundImage(CAImage::create("source_material/tabBar_bg.png"));
    }
    if (m_pSelectedBackGroundImage == NULL)
    {
        this->setSelectedBackGroundImage(CAImage::create("source_material/tabBar_selected_bg.png"));
    }
    if (m_pSelectedIndicatorImage == NULL)
    {
        this->setSelectedIndicatorImage(CAImage::create("source_material/tabBar_selected_indicator.png"));
    }

    return true;
}

CATabBar* CATabBar::create(const CAVector<CATabBarItem*>& items, const DSize& size, const CABarVerticalAlignment& var)
{
    CATabBar* tabBar = new CATabBar();
    if (tabBar && tabBar->init(items, size, var))
    {
        tabBar->autorelease();
        return tabBar;
    }
    CC_SAFE_DELETE(tabBar);
    return NULL;
}

void CATabBar::setItems(const CAVector<CATabBarItem*>& items)
{
    CC_RETURN_IF(items.empty());
	m_pItems = items;
}

void CATabBar::replaceItemAtIndex(size_t index, CATabBarItem* item)
{
    if (index < m_pItems.size())
    {
		m_pItems.replace(index, item);
        if (!m_pButtons.empty())
        {
            CAButton* btn = m_pButtons.at(index);
            btn->setTitleForState(CAControlStateAll, item->getTitle());
            btn->setImageForState(CAControlStateNormal, item->getImage());
            CAImage* selectedImage = item->getSelectedImage()
            ? item->getSelectedImage()
            : item->getImage();
            btn->setImageForState(CAControlStateHighlighted, selectedImage);
            btn->setImageForState(CAControlStateSelected, selectedImage);

            CABadgeView* badgeView = m_pBadgeViews.at(index);
            badgeView->setBadgeText(item->getBadgeValue());
        }
    }
}

const DRect& CATabBar::getContentViewFrame()
{
    return m_pContentView->getFrame();
}

void CATabBar::setBackGroundImage(CrossApp::CAImage *var)
{
    CC_SAFE_RETAIN(var);
    CC_SAFE_RELEASE_NULL(m_pBackGroundImage);
    m_pBackGroundImage = var;
    m_sBackGroundColor = CAColor_white;
    this->showBackGround();
}

CAImage* CATabBar::getBackGroundImage()
{
    return m_pBackGroundImage;
}

void CATabBar::setBackGroundColor(const CAColor4B &var)
{
    m_sBackGroundColor = var;
    CC_SAFE_RELEASE_NULL(m_pBackGroundImage);
    this->showBackGround();
}

const CAColor4B& CATabBar::getBackGroundColor()
{
    return m_sBackGroundColor;
}

void CATabBar::setSelectedBackGroundImage(CrossApp::CAImage *var)
{
    CC_SAFE_RETAIN(var);
    CC_SAFE_RELEASE_NULL(m_pSelectedBackGroundImage);
    m_pSelectedBackGroundImage = var;
    m_sSelectedBackGroundColor = CAColor_white;
    
    this->showSelectedBackGround();
}

CAImage* CATabBar::getSelectedBackGroundImage()
{
    return m_pSelectedBackGroundImage;
}

void CATabBar::setSelectedBackGroundColor(const CAColor4B &var)
{
    m_sSelectedBackGroundColor = var;
    CC_SAFE_RELEASE_NULL(m_pSelectedBackGroundImage);
    this->showSelectedBackGround();
}

const CAColor4B& CATabBar::getSelectedBackGroundColor()
{
    return m_sSelectedBackGroundColor;
};

void CATabBar::setSelectedIndicatorImage(CrossApp::CAImage *var)
{
    CC_SAFE_RETAIN(var);
    CC_SAFE_RELEASE_NULL(m_pSelectedIndicatorImage);
    m_pSelectedIndicatorImage = var;
    m_sSelectedIndicatorColor = CAColor_white;
    CC_RETURN_IF(var == NULL);
    
    this->showSelectedIndicatorView();
}

CAImage* CATabBar::getSelectedIndicatorImage()
{
    return m_pSelectedIndicatorImage;
}


void CATabBar::setSelectedIndicatorColor(const CAColor4B &var)
{
    m_sSelectedIndicatorColor = var;
    CC_SAFE_RELEASE_NULL(m_pSelectedIndicatorImage);
    
    this->showSelectedIndicatorView();
}

const CAColor4B& CATabBar::getSelectedIndicatorColor()
{
    return m_sSelectedIndicatorColor;
}

void CATabBar::setTitleColorForNormal(const CAColor4B &var)
{
    m_sTitleColor = var;
    if (!m_pButtons.empty())
    {
        for (size_t i=0; i<m_pButtons.size(); i++)
        {
            CAButton* btn = m_pButtons.at(i);
            btn->setTitleColorForState(CAControlStateNormal, m_sTitleColor);
        }
    }
}

const CAColor4B& CATabBar::getTitleColorForNormal()
{
    return m_sTitleColor;
}

void CATabBar::setTitleColorForSelected(const CAColor4B &var)
{
    m_sSelectedTitleColor = var;
    if (!m_pButtons.empty())
    {
        for (size_t i=0; i<m_pButtons.size(); i++)
        {
            CAButton* btn = m_pButtons.at(i);
            btn->setTitleColorForState(CAControlStateHighlighted, m_sSelectedTitleColor);
            btn->setTitleColorForState(CAControlStateSelected, m_sSelectedTitleColor);
        }
    }
}

const CAColor4B& CATabBar::getTitleColorForSelected()
{
    return m_sSelectedTitleColor;
}

void CATabBar::showBackGround()
{
    this->removeSubview(m_pBackGroundView);
    
    if (m_pBackGroundImage)
    {
        m_pBackGroundView = CAScale9ImageView::createWithImage(m_pBackGroundImage);
    }
    else
    {
        m_pBackGroundView = CAView::createWithColor(m_sBackGroundColor);
    }
    m_pBackGroundView->setFrame(this->getBounds());
    this->insertSubview(m_pBackGroundView, -1);
}

void CATabBar::showSelectedBackGround()
{
    for (size_t i=0; i<m_pButtons.size(); i++)
    {
        CAButton* btn = m_pButtons.at(i);
        btn->setTitleForState(CAControlStateAll, m_pItems.at(i)->getTitle());
        btn->setTitleColorForState(CAControlStateAll, m_sTitleColor);
        btn->setTitleColorForState(CAControlStateHighlighted, m_sSelectedTitleColor);
        btn->setTitleColorForState(CAControlStateSelected, m_sSelectedTitleColor);
        btn->setImageForState(CAControlStateNormal, m_pItems.at(i)->getImage());
        CAImage* selectedImage = m_pItems.at(i)->getSelectedImage()
        ? m_pItems.at(i)->getSelectedImage()
        : m_pItems.at(i)->getImage();
        btn->setImageForState(CAControlStateHighlighted, selectedImage);
        btn->setImageForState(CAControlStateSelected, selectedImage);
        btn->setBackGroundViewForState(CAControlStateNormal, CAView::createWithColor(CAColor_clear));
        if (m_pSelectedBackGroundImage)
        {
            btn->setBackGroundViewForState(CAControlStateHighlighted,
                                           CAScale9ImageView::createWithImage(m_pSelectedBackGroundImage));
            btn->setBackGroundViewForState(CAControlStateSelected,
                                           CAScale9ImageView::createWithImage(m_pSelectedBackGroundImage));
        }
        else
        {
            btn->setBackGroundViewForState(CAControlStateHighlighted,
                                           CAView::createWithColor(m_sSelectedBackGroundColor));
            btn->setBackGroundViewForState(CAControlStateSelected,
                                           CAView::createWithColor(m_sSelectedBackGroundColor));
        }
        btn->setAllowsSelected(true);
        
        CABadgeView* badgeView = m_pBadgeViews.at(i);
        badgeView->setBadgeText(m_pItems.at(i)->getBadgeValue());
    }
}

void CATabBar::showSelectedIndicatorView()
{
    m_pContentView->removeSubview(m_pSelectedIndicatorView);
    if (m_pSelectedIndicatorImage)
    {
        CAScale9ImageView* imageView = CAScale9ImageView::createWithImage(m_pSelectedIndicatorImage);
        DRect insetRect;
        insetRect.origin = m_pSelectedIndicatorImage->getContentSize() / 2;
        insetRect.origin = ccpSub(insetRect.origin, DPoint(1, 1));
        insetRect.size = DPoint(2, 2);
        imageView->setCapInsets(insetRect);
        m_pSelectedIndicatorView = imageView;
    }
    else
    {
        m_pSelectedIndicatorView = CAView::createWithColor(m_sSelectedIndicatorColor);
    }
    
    DRect rect;
    rect.size.width = m_cItemSize.width;
    rect.size.height = 8;
    rect.origin.x = m_nSelectedIndex * m_cItemSize.width;
    rect.origin.y = m_cItemSize.height - rect.size.height;
    m_pSelectedIndicatorView->setFrame(rect);
    m_pContentView->insertSubview(m_pSelectedIndicatorView, 1);
    m_pSelectedIndicatorView->setVisible(m_bShowIndicator);
}

void CATabBar::showSelectedIndicator()
{
    m_bShowIndicator = true;
    if (m_pSelectedIndicatorView)
    {
        m_pSelectedIndicatorView->setVisible(true);
    }
}

void CATabBar::setSelectedAtIndex(int index)
{
    CC_RETURN_IF(index < -1);
    CC_RETURN_IF((size_t)index >= m_pItems.size());
    
    if (m_nSelectedIndex != -1)
    {
        m_pButtons.at(m_nSelectedIndex)->setControlStateNormal();
    }
    
    if (index != -1)
    {
        m_pButtons.at(index)->setControlStateSelected();
        m_nSelectedIndex = index;
        m_pSelectedItem = m_pItems.at(m_nSelectedIndex);
        
        if (m_pSelectedIndicatorView)
        {
            m_pSelectedIndicatorView->setVisible(m_bShowIndicator);
            DPoint p = m_pSelectedIndicatorView->getFrameOrigin();
            p.x = m_nSelectedIndex * m_cItemSize.width;
            
            CAViewAnimation::beginAnimations("", NULL);
            CAViewAnimation::setAnimationDuration(0.3f);
            CAViewAnimation::setAnimationCurve(CAViewAnimationCurveEaseOut);
            m_pSelectedIndicatorView->setFrameOrigin(p);
            CAViewAnimation::commitAnimations();
        }
    }
    else
    {
        m_pSelectedIndicatorView->setVisible(false);
    }
}

void CATabBar::addForbidSelectedAtIndex(int index)
{
    m_sForbidSelectedIndexs.insert(index);
}

void CATabBar::setTouchSelected(CrossApp::CAControl *control, CrossApp::DPoint point)
{
    int index = control->getTag();
    if (!m_sForbidSelectedIndexs.count(index))
    {
        this->setSelectedAtIndex(control->getTag());
        
        if (m_pDelegate)
        {
            m_pDelegate->tabBarSelectedItem(this, m_pSelectedItem, m_nSelectedIndex);
        }
    }
    else
    {
        if (m_pDelegate)
        {
            m_pDelegate->tabBarClickToForbidSelectedItem(this, m_pSelectedItem, m_nSelectedIndex);
        }
    }
    
}


NS_CC_END