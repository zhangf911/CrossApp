#include "CAVideoPlayerView.h"
#include "basics/CAScheduler.h"
#include "basics/CAApplication.h"
#include "view/CADrawingPrimitives.h"


NS_CC_BEGIN

#define LOCAL_MIN_BUFFERED_DURATION   0.2
#define LOCAL_MAX_BUFFERED_DURATION   0.4
#define NETWORK_MIN_BUFFERED_DURATION 2.0
#define NETWORK_MAX_BUFFERED_DURATION 5.0

#define ThreadMsgType_SetPosition 1
#define ThreadMsgType_DecodeFrame 2

typedef struct DecodeFramesMsg
{
	CAVideoPlayerView* pAVGLView;
	float param1;
	float param2;
}DecodeFramesMsg;

CAVideoPlayerView::CAVideoPlayerView()
: m_pRenderer(NULL)
, m_pDecoder(NULL)
, m_isPlaying(false)
, m_isBuffered(false)
, m_fMinBufferedDuration(0)
, m_fMaxBufferedDuration(0)
, m_fBufferedDuration(0)
, m_fMoviePosition(0)
, m_tickCorrectionPosition(0)
, m_pCurVideoFrame(NULL)
, m_pCurAudioFrame(NULL)
, m_uCurAudioFramePos(0)
, m_pLoadingView(NULL)
{
}

CAVideoPlayerView::~CAVideoPlayerView()
{
	CAScheduler::unschedule(schedule_selector(CAVideoPlayerView::tick), this);

	CAThread::clear(true);
	setPosition(0);
	CAThread::close();

	CC_SAFE_DELETE(m_pCurVideoFrame);
	CC_SAFE_DELETE(m_pCurAudioFrame);

	CC_SAFE_DELETE(m_pRenderer);
	CC_SAFE_DELETE(m_pDecoder);
}

CAVideoPlayerView* CAVideoPlayerView::create()
{
    CAVideoPlayerView* view = new CAVideoPlayerView();
    if (view && view->init()) 
	{
        view->autorelease();
        return view;
    }
    CC_SAFE_DELETE(view);
    return NULL;
}

CAVideoPlayerView* CAVideoPlayerView::createWithCenter(const DRect &rect)
{
    CAVideoPlayerView* view = new CAVideoPlayerView;
    if (view && view->initWithCenter(rect))
	{
		view->m_viewRect = rect;
        view->autorelease();
        return view;
    }
    CC_SAFE_DELETE(view);
    
    return NULL;
}

CAVideoPlayerView* CAVideoPlayerView::createWithFrame(const DRect &rect)
{
    CAVideoPlayerView* view = new CAVideoPlayerView;
    if (view && view->initWithFrame(rect)) 
	{
		view->m_viewRect = rect;
        view->autorelease();
        return view;
    }
    CC_SAFE_DELETE(view);
    
    return NULL;
}

bool CAVideoPlayerView::init()	
{
    if (!CAView::init()) {
        return false;
    }
    
	setColor(ccc4(0, 0, 0, 255));

    return true;
}

void CAVideoPlayerView::initWithPath(const std::string& szPath)
{
	m_fMinBufferedDuration = LOCAL_MIN_BUFFERED_DURATION;
	m_fMaxBufferedDuration = LOCAL_MAX_BUFFERED_DURATION;
	m_cszPath = szPath;
}

void CAVideoPlayerView::initWithUrl(const std::string& szUrl)
{
	m_fMinBufferedDuration = NETWORK_MIN_BUFFERED_DURATION;
	m_fMaxBufferedDuration = NETWORK_MAX_BUFFERED_DURATION;
	m_cszPath = szUrl;
}

void CAVideoPlayerView::setContentSize(const DSize& size)
{
    CAView::setContentSize(size);
    
	if (m_pRenderer)
	{
		m_viewRect = m_pRenderer->updateVertices(
			m_pDecoder->getFrameWidth(), m_pDecoder->getFrameHeight(), getFrame().size.width, getFrame().size.height);
	}
	setImageRect(m_viewRect);
}

void CAVideoPlayerView::setImageCoords(DRect rect)
{
    float left = 0, right = 1, top = 0, bottom = 1;
    
    if (m_bRectRotated)
    {
        if (m_bFlipX)
        {
            CC_SWAP(top, bottom, float);
        }
        
        if (m_bFlipY)
        {
            CC_SWAP(left, right, float);
        }
        
        m_sQuad.bl.texCoords.u = left;
        m_sQuad.bl.texCoords.v = top;
        m_sQuad.br.texCoords.u = left;
        m_sQuad.br.texCoords.v = bottom;
        m_sQuad.tl.texCoords.u = right;
        m_sQuad.tl.texCoords.v = top;
        m_sQuad.tr.texCoords.u = right;
        m_sQuad.tr.texCoords.v = bottom;
    }
    else
    {
        if(m_bFlipX)
        {
            CC_SWAP(left,right,float);
        }
        
        if(m_bFlipY)
        {
            CC_SWAP(top,bottom,float);
        }
        
        m_sQuad.bl.texCoords.u = left;
        m_sQuad.bl.texCoords.v = bottom;
        m_sQuad.br.texCoords.u = right;
        m_sQuad.br.texCoords.v = bottom;
        m_sQuad.tl.texCoords.u = left;
        m_sQuad.tl.texCoords.v = top;
        m_sQuad.tr.texCoords.u = right;
        m_sQuad.tr.texCoords.v = top;
    }
}

void CAVideoPlayerView::updateImageRect()
{
    // Don't update Z.
    float m_fLeft, m_fTop, m_fRight, m_fBottom;
    
	m_fLeft = m_viewRect.origin.x;
	m_fRight = m_viewRect.origin.x + m_viewRect.size.width;
	m_fTop = m_viewRect.origin.y;
	m_fBottom = m_viewRect.origin.y + m_viewRect.size.height;
    
    m_sQuad.bl.vertices = vertex3(m_fLeft, m_fTop, 0);
    m_sQuad.br.vertices = vertex3(m_fRight, m_fTop, 0);
    m_sQuad.tl.vertices = vertex3(m_fLeft, m_fBottom, 0);
    m_sQuad.tr.vertices = vertex3(m_fRight, m_fBottom, 0);
}

void CAVideoPlayerView::visit()
{
    CAView::visit();
    
    updateDraw();
}


void CAVideoPlayerView::draw()
{
    long offset = (long)&m_sQuad;
    
	if (m_pCurVideoFrame && m_pRenderer) 
	{
		m_pRenderer->draw(m_pCurVideoFrame, offset);
	}
}

void CAVideoPlayerView::setCurrentFrame(VPVideoFrame *frame)
{
	CC_SAFE_DELETE(m_pCurVideoFrame);
	m_pCurVideoFrame = frame;
	showLoadingView(m_pCurVideoFrame == NULL);
}

void CAVideoPlayerView::play()
{
	if (isPlaying())
		return;

	showLoadingView(true);

	m_tickCorrectionTime.tv_sec = 0;
	m_tickCorrectionTime.tv_usec = 0;

	this->enableAudio(true);
	asyncDecodeFrames();
	CAScheduler::schedule(schedule_selector(CAVideoPlayerView::tick), this, 0);
	m_isPlaying = true;
}

void CAVideoPlayerView::pause()
{
	if (!this->isPlaying())
		return;
	
	showLoadingView(false);
	m_isPlaying = false;
	this->enableAudio(false);
}

bool CAVideoPlayerView::isPlaying()
{
	return m_isPlaying;
}

void CAVideoPlayerView::enableAudio(bool on)
{
	if (m_pDecoder && m_pDecoder->isValidAudio())
	{
		m_pDecoder->enableAudio(on);
	}
}

float CAVideoPlayerView::getDuration()
{
	if (m_pDecoder)
	{
		return m_pDecoder->getDuration();
	}
	return 0;
}

float CAVideoPlayerView::getPosition()
{
	if (m_pDecoder)
	{
		return m_fMoviePosition - m_pDecoder->getStartTime();
	}
	return 0;
}

void CAVideoPlayerView::setPosition(float position)
{
	if (m_pDecoder == NULL)
		return;
	
	m_isBuffered = true;
	pause();
	CAThread::clear(true);
	setDecodePosition(position);
}

void CAVideoPlayerView::showLoadingView(bool on)
{
	if (m_pLoadingView == NULL)
	{
		m_pLoadingView = CAActivityIndicatorView::create();
		m_pLoadingView->setStyle(CAActivityIndicatorViewStyleWhite);
		float x = m_viewRect.origin.x + m_viewRect.size.width / 2;
		float y = m_viewRect.origin.y + m_viewRect.size.height / 2;
		m_pLoadingView->setCenterOrigin(DPoint(x, y));
		m_pLoadingView->resignFirstResponder();
		this->addSubview(m_pLoadingView);
	}
	m_pLoadingView->setVisible(on);
}

bool CAVideoPlayerView::createDecoder()
{
	if (m_pDecoder)
		return true;
	
	m_pDecoder = new VPDecoder();
	if (m_pDecoder == NULL)
	{
		return false;
	}

	if (!m_pDecoder->openFile(m_cszPath))
	{
		CC_SAFE_DELETE(m_pDecoder);
		return false;
	}

	if (!m_pDecoder->isValidVideo())
		m_fMinBufferedDuration *= 10.0; // increase for audio

	m_pDecoder->setAudioCallback(this, decoder_audio_selector(CAVideoPlayerView::audioCallback));

	if (m_pDecoder->setupVideoFrameFormat(kVideoFrameFormatYUV))
	{
		m_pRenderer = new VPFrameRenderYUV();
	}
	else
	{
		m_pRenderer = new VPFrameRenderRGB();
	}

	if (!m_pRenderer->loadShaders())
	{
		CC_SAFE_DELETE(m_pRenderer);
		CC_SAFE_DELETE(m_pDecoder);
		return false;
	}
	setFrame(getFrame());

	CAThread::setMaxMsgCount(8);
	CAThread::startAndWait(decodeProcessThread);
	return true;
}

void CAVideoPlayerView::setVPPosition(float p)
{
	VPFrame* frame = NULL;
	while (m_vVideoFrames.PopElement(frame)) CC_SAFE_DELETE(frame);
	while (m_vAudioFrames.PopElement(frame)) CC_SAFE_DELETE(frame);
	
	float position = MIN(m_pDecoder->getDuration(), MAX(0, p));
	m_pDecoder->setPosition(position);

	m_aLock.Lock();
	m_fMoviePosition = position + m_pDecoder->getStartTime();
	m_fBufferedDuration = 0;
	CC_SAFE_DELETE(m_pCurAudioFrame);
	m_uCurAudioFramePos = 0;
	m_aLock.UnLock();
}

void CAVideoPlayerView::decodeProcess()
{
	bool good = true;
	
	while (good)
	{
		good = false;
		if (m_pDecoder && (m_pDecoder->isValidVideo() || m_pDecoder->isValidAudio())) 
		{
			std::vector<VPFrame*> frames = m_pDecoder->decodeFrames(0.1f);
			if (!frames.empty()) 
			{
				good = addFrames(frames);
			}
		}
	}
}

bool CAVideoPlayerView::decodeProcessThread(void* param)
{
	DecodeFramesMsg* pMsg = (DecodeFramesMsg*)param;
	if (pMsg)
	{
		if (pMsg->param1 == ThreadMsgType_SetPosition)
		{
			pMsg->pAVGLView->setVPPosition(pMsg->param2);
		}
		if (pMsg->param1 == ThreadMsgType_DecodeFrame)
		{
			pMsg->pAVGLView->decodeProcess();
		}
	}
	CC_SAFE_FREE(pMsg);
	return true;
}

void CAVideoPlayerView::asyncDecodeFrames()
{
	DecodeFramesMsg* pMsg = (DecodeFramesMsg*)malloc(sizeof(DecodeFramesMsg));
	if (pMsg)
	{
		pMsg->pAVGLView = this;
		pMsg->param1 = ThreadMsgType_DecodeFrame;
	}
	CAThread::notifyRun(pMsg);
}

void CAVideoPlayerView::setDecodePosition(float pos)
{
	DecodeFramesMsg* pMsg = (DecodeFramesMsg*)malloc(sizeof(DecodeFramesMsg));
	if (pMsg)
	{
		pMsg->pAVGLView = this;
		pMsg->param1 = ThreadMsgType_SetPosition;
		pMsg->param2 = pos;
	}
	CAThread::notifyRun(pMsg);
}

bool CAVideoPlayerView::addFrames(const std::vector<VPFrame*>& frames)
{
	bool isValidVideo = m_pDecoder->isValidVideo();
	bool isValidAudio = m_pDecoder->isValidAudio();

	for (int i = 0; i < frames.size(); i++)
	{
		VPFrame* frame = frames[i];
		if (frame == NULL)
			continue;

		if (frame->getType() == kFrameTypeVideo)
		{
			if (isValidVideo)
			{
				m_vVideoFrames.AddElement(frame);

				m_aLock.Lock();
				m_fBufferedDuration += frame->getDuration();
				m_aLock.UnLock();
			}
			else
			{
				CC_SAFE_DELETE(frame);
			}
		}

		if (frame->getType() == kFrameTypeAudio)
		{
			if (isValidAudio)
			{
				m_vAudioFrames.AddElement(frame);

				if (!isValidVideo)
				{
					m_aLock.Lock();
					m_fBufferedDuration += frame->getDuration();
					m_aLock.UnLock();
				}
			}
			else
			{
				CC_SAFE_DELETE(frame);
			}
		}
	}
	return m_isPlaying && m_fBufferedDuration < m_fMaxBufferedDuration;
}

float CAVideoPlayerView::presentFrame()
{
	float interval = 0;

	if (m_pDecoder->isValidVideo()) 
	{
		VPFrame *frame = NULL;

		m_isBuffered = (m_fBufferedDuration<m_fMaxBufferedDuration / 2);
		if (m_pDecoder->isEOF())
		{
			m_isBuffered = false;
		}

		while (!m_isBuffered && m_vVideoFrames.PopElement(frame))
		{
			m_fBufferedDuration -= frame->getDuration();

			float fCurPos = frame->getPosition();
			if (fCurPos >= m_fMoviePosition)
			{
				interval = frame->getDuration();
				m_fMoviePosition = fCurPos;
				break;
			}
			else
			{
				interval = 0;
				CC_SAFE_DELETE(frame);
			}
		}
		setCurrentFrame((VPVideoFrame*)frame);
	}
	return interval;
}

void CAVideoPlayerView::tick(float dt)
{
	if (!createDecoder())
		return;

	if (m_pDecoder == NULL || m_pRenderer == NULL)
		return;

	if (m_isBuffered && ((m_fBufferedDuration > m_fMinBufferedDuration) || m_pDecoder->isEOF())) {

		m_tickCorrectionTime.tv_sec = 0;
		m_tickCorrectionTime.tv_usec = 0;

		m_isBuffered = false;
	}

	if (!m_isPlaying)
		return;

	float interval = 0;
	if (!m_isBuffered)
	{
		m_aLock.Lock();
		interval = presentFrame();
		m_aLock.UnLock();
	}
		

	if (m_pDecoder->isEOF()) 
	{
		unsigned int leftFrames =
			(m_pDecoder->isValidVideo() ? m_vVideoFrames.GetCount() : 0) +
			(m_pDecoder->isValidAudio() ? m_vAudioFrames.GetCount() : 0);
		if (0 == leftFrames)
		{
			pause();
			setPosition(0);
			return;
		}
	}

	if (m_fBufferedDuration < m_fMaxBufferedDuration)
	{
		asyncDecodeFrames();
	}

	float correction = tickCorrection();
	float time = MAX(interval + correction, 0.01f);
	CAScheduler::schedule(schedule_selector(CAVideoPlayerView::tick), this, time);
}

float CAVideoPlayerView::tickCorrection()
{
	if (m_isBuffered)
		return 0;

	struct timeval now;
	gettimeofday(&now, 0);

	if (!m_tickCorrectionTime.tv_sec) {

		m_tickCorrectionTime = now;
		m_tickCorrectionPosition = m_fMoviePosition;
		return 0;
	}

	float dPosition = m_fMoviePosition - m_tickCorrectionPosition;
	float dTime = (now.tv_sec - m_tickCorrectionTime.tv_sec) + (now.tv_usec - m_tickCorrectionTime.tv_usec) / 1000000.0f;
	float correction = dPosition - dTime;

	if (correction > 1.f || correction < -1.f) 
	{
		correction = 0;
		m_tickCorrectionTime.tv_sec = 0;
		m_tickCorrectionTime.tv_usec = 0;
	}
	return correction;
}

void CAVideoPlayerView::audioCallback(unsigned char *stream, int len, int channels)
{
	memset(stream, 0, len);

    while (len > 0)
    {
		if (!isPlaying())
			return;

		if (m_isBuffered)
			return;
    
        
        if (m_pCurAudioFrame == NULL)
        {
			VPFrame* frame = NULL;
			if (m_vAudioFrames.PopElement(frame))
			{
				m_pCurAudioFrame = (VPAudioFrame*)frame;
			}
			m_uCurAudioFramePos = 0;
        }
        
        if (m_pCurAudioFrame)
        {
			if (!m_pDecoder->isValidVideo())
            {
				m_aLock.Lock();
				m_fMoviePosition = m_pCurAudioFrame->getPosition();
				m_fBufferedDuration -= m_pCurAudioFrame->getDuration();
				m_aLock.UnLock();
			}

            unsigned char* bytes = (unsigned char*)(m_pCurAudioFrame->getData() + m_uCurAudioFramePos);
            const unsigned int bytesLeft = m_pCurAudioFrame->getDataLength() - m_uCurAudioFramePos;
            const unsigned int bytesToCopy = MIN(len, bytesLeft);
            
            if (bytesToCopy > 0)
            {
                memcpy(stream, bytes, bytesToCopy);
                stream += bytesToCopy;
                len -= bytesToCopy;
                m_uCurAudioFramePos += bytesToCopy;
            }
            else
            {
                if (bytesLeft == 0)
                {
                    CC_SAFE_DELETE(m_pCurAudioFrame);
                }
            }
        }
    }
}

NS_CC_END