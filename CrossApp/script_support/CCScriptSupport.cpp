

#include "CCScriptSupport.h"

bool CC_DLL cc_assert_script_compatible(const char *msg)
{
    CrossApp::CCScriptEngineProtocol* pEngine = CrossApp::CCScriptEngineManager::sharedManager()->getScriptEngine();
    if (pEngine && pEngine->handleAssert(msg))
    {
        return true;
    }
    return false;
}

NS_CC_BEGIN

 #if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS) || (CC_TARGET_PLATFORM == CC_PLATFORM_MAC)
// #pragma mark -
// #pragma mark CCScriptHandlerEntry
#endif

CCScriptHandlerEntry* CCScriptHandlerEntry::create(int nHandler)
{
    CCScriptHandlerEntry* entry = new CCScriptHandlerEntry(nHandler);
    entry->autorelease();
    return entry;
}

CCScriptHandlerEntry::~CCScriptHandlerEntry(void)
{
	if (m_nHandler != 0)
	{
        CCScriptEngineManager::sharedManager()->getScriptEngine()->removeScriptHandler(m_nHandler);
        m_nHandler = 0;
    }
}

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS) || (CC_TARGET_PLATFORM == CC_PLATFORM_MAC)
// #pragma mark -
// #pragma mark CCSchedulerScriptHandlerEntry
#endif

CCSchedulerScriptHandlerEntry* CCSchedulerScriptHandlerEntry::create(int nHandler, float fInterval, bool bPaused)
{
    CCSchedulerScriptHandlerEntry* pEntry = new CCSchedulerScriptHandlerEntry(nHandler);
    pEntry->init(fInterval, bPaused);
    pEntry->autorelease();
    return pEntry;
}

bool CCSchedulerScriptHandlerEntry::init(float fInterval, bool bPaused)
{
//    m_pTimer = new CrossApp::CATimer();
//    m_pTimer->initWithScriptHandler(m_nHandler, fInterval);
//    m_pTimer->autorelease();
//    m_pTimer->retain();
//    m_bPaused = bPaused;
    return true;
}

CCSchedulerScriptHandlerEntry::~CCSchedulerScriptHandlerEntry(void)
{
//    m_pTimer->release();
}

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS) || (CC_TARGET_PLATFORM == CC_PLATFORM_MAC)
// #pragma mark -
// #pragma mark CCTouchScriptHandlerEntry
#endif

CCTouchScriptHandlerEntry* CCTouchScriptHandlerEntry::create(int nHandler,
                                                             bool bIsMultiTouches,
                                                             int nPriority,
                                                             bool bSwallowsTouches)
{
    CCTouchScriptHandlerEntry* pEntry = new CCTouchScriptHandlerEntry(nHandler);
    pEntry->init(bIsMultiTouches, nPriority, bSwallowsTouches);
    pEntry->autorelease();
    return pEntry;
}

CCTouchScriptHandlerEntry::~CCTouchScriptHandlerEntry(void)
{
    if (m_nHandler != 0)
    {
        CCScriptEngineManager::sharedManager()->getScriptEngine()->removeScriptHandler(m_nHandler);
        m_nHandler = 0;
    }
}

bool CCTouchScriptHandlerEntry::init(bool bIsMultiTouches, int nPriority, bool bSwallowsTouches)
{
    m_bIsMultiTouches = bIsMultiTouches;
    m_nPriority = nPriority;
    m_bSwallowsTouches = bSwallowsTouches;
    
    return true;
}

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS) || (CC_TARGET_PLATFORM == CC_PLATFORM_MAC)
// #pragma mark -
// #pragma mark CCScriptEngineManager
#endif

static CCScriptEngineManager* s_pSharedScriptEngineManager = NULL;


CCScriptEngineManager::~CCScriptEngineManager(void)
{
    removeScriptEngine();
}

void CCScriptEngineManager::setScriptEngine(CCScriptEngineProtocol *pScriptEngine)
{
    removeScriptEngine();
    CCLog("setScriptEngine");
    m_pScriptEngine = pScriptEngine;
}

void CCScriptEngineManager::removeScriptEngine(void)
{
    if (m_pScriptEngine)
    {
        delete m_pScriptEngine;
        m_pScriptEngine = NULL;
    }
}

CCScriptEngineManager* CCScriptEngineManager::sharedManager(void)
{
    if (!s_pSharedScriptEngineManager)
    {
        s_pSharedScriptEngineManager = new CCScriptEngineManager();
    }
    return s_pSharedScriptEngineManager;
}

void CCScriptEngineManager::purgeSharedManager(void)
{
    if (s_pSharedScriptEngineManager)
    {
        delete s_pSharedScriptEngineManager;
        s_pSharedScriptEngineManager = NULL;
    }
}

NS_CC_END
