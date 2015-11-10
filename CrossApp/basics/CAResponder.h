//
//  CAResponder.h
//  CrossApp
//
//  Created by Li Yuanfeng on 14-6-15.
//  Copyright (c) 2014 http://9miao.com All rights reserved.
//

#ifndef __CrossApp__CAResponder__
#define __CrossApp__CAResponder__

#include <iostream>
#include "CAObject.h"
#include "dispatcher/CATouch.h"

NS_CC_BEGIN;

class CATouch;

class CC_DLL CAResponder: public CAObject
{
    
public:

    CAResponder();
    
    virtual ~CAResponder();
    
    bool isFirstResponder();
    
    virtual bool resignFirstResponder();

    virtual bool becomeFirstResponder();
    
    virtual CAResponder* nextResponder();

    CC_SYNTHESIZE_IS(bool, m_bHaveNextResponder, HaveNextResponder);// default is true
    
    CC_SYNTHESIZE_IS(bool, m_bTouchEnabled, TouchEnabled);// default is true

    CC_SYNTHESIZE_IS(bool, m_bScrollEnabled, ScrollEnabled);// default is true
    
    CC_SYNTHESIZE_IS(bool, m_bHorizontalScrollEnabled, HorizontalScrollEnabled);// default is true
    
    CC_SYNTHESIZE_IS(bool, m_bVerticalScrollEnabled, VerticalScrollEnabled);// default is true
    
    CC_SYNTHESIZE_IS(bool, m_bPriorityScroll, PriorityScroll);// default is false, CAScrollView as well as his derived type defaults to true
    
    CC_SYNTHESIZE_IS(bool, m_bReachBoundaryHandOverToSuperview, ReachBoundaryHandOverToSuperview);// default is true
    
    CC_SYNTHESIZE_IS(bool, m_bTouchEventScrollHandOverToSuperview, TouchEventScrollHandOverToSuperview);// default is true
    
    CC_PROPERTY_IS(bool, m_bMouseMovedEnabled, MouseMovedEnabled);// default is false
    
    CC_PROPERTY_IS(bool, m_bMouseScrollWheelEnabled, MouseScrollWheelEnabled);// default is false
    
    CC_SYNTHESIZE_READONLY(unsigned long, m_uZLevel, ZLevel);
    
    virtual bool isReachBoundaryLeft() {return false;}
    
    virtual bool isReachBoundaryRight() {return false;}
    
    virtual bool isReachBoundaryUp() {return false;}
    
    virtual bool isReachBoundaryDown() {return false;}
    
    /**
     * @lua NA
     */
    virtual bool ccTouchBegan(CATouch *pTouch, CAEvent *pEvent);
    /**
     * @lua NA
     */
    virtual void ccTouchMoved(CATouch *pTouch, CAEvent *pEvent);
    /**
     * @lua NA
     */
    virtual void ccTouchEnded(CATouch *pTouch, CAEvent *pEvent);
    /**
     * @lua NA
     */
    virtual void ccTouchCancelled(CATouch *pTouch, CAEvent *pEvent);
    /**
     * @lua NA
     */
    virtual void mouseMoved(CATouch* pTouch, CAEvent* pEvent);
    /**
     * @lua NA
     */
    virtual void mouseMovedOutSide(CATouch* pTouch, CAEvent* pEvent);
    /**
     * @lua NA
     */
    virtual void mouseScrollWheel(CATouch* pTouch, float off_x, float off_y, CAEvent* pEvent);
};

NS_CC_END;

#endif /* defined(__CrossApp__CAResponder__) */
