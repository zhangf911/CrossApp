

#ifndef __CC_TOUCH_H__
#define __CC_TOUCH_H__

#include "basics/CAObject.h"
#include "basics/CAGeometry.h"

NS_CC_BEGIN


class CC_DLL CATouch : public CAObject
{
public:

    CATouch()
        : m_nId(0)
        ,m_startPointCaptured(false)
        ,m_bDelta(false)
        ,m_startPoint(DPoint(0xffffffff, 0xffffffff))
        ,m_point(DPoint(0xffffffff, 0xffffffff))
        ,m_prevPoint(DPoint(0xffffffff, 0xffffffff))
    {}


    DPoint getLocation() const;

    DPoint getPreviousLocation() const;

    DPoint getStartLocation() const;

    DPoint getDelta() const;
    
    DPoint getDeltaFromAToZ() const;
    
    bool isDelta() const;
    
    void setTouchInfo(int id, float x, float y);

    int getID() const
    {
        return m_nId;
    }

private:
    int m_nId;
    bool m_startPointCaptured;
    bool m_bDelta;
    DPoint m_startPoint;
    DPoint m_point;
    DPoint m_prevPoint;
};

enum EventType
{
    keyboardEvent = 0,
    leftMouseEvent,
    rightMouseEvent,
    middleMouseEvent,
    movedMouseEvent,
    iosEvent,
    androidEvent,
};

class CC_DLL CAEvent : public CAObject
{


public:
    CAEvent()
    : m_eType(keyboardEvent)
    {}
    
    EventType getEventType() { return m_eType;}
    
    void setEventType(EventType type) { m_eType = type;}
protected:
    EventType m_eType;

};

// end of input group
/// @}

NS_CC_END

#endif  // __PLATFORM_TOUCH_H__
