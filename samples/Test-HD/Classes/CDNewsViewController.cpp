//
//  CDNewsViewController.cpp
//  Test
//
//  Created by renhongguang on 15/4/16.
//
//

#include "CDNewsViewController.h"
#include "CDWebViewController.h"
#include "CDShowNewsImage.h"

extern int page_index;
float temp_time = 0;

CDNewsViewController::CDNewsViewController(int index,string newsType)
:p_pLoading(NULL)
,p_Conllection(NULL)
,p_section(1)
{
    urlID = index;
    m_pNewsType = newsType;
    m_msg.clear();
}

CDNewsViewController::~CDNewsViewController(){
    
}

string CDNewsViewController::getSign(std::map<std::string,std::string> key_value)
{
    string appsecret = "c174cb1fda3491285be953998bb867a0";
    string tempStr = "";
    std::map<std::string,std::string>::iterator itr;
    for (itr=key_value.begin(); itr!=key_value.end(); itr++) {
        tempStr = tempStr+itr->first+itr->second;
    }
    tempStr = appsecret+tempStr+appsecret;
    CCLog("tempStr===%s",tempStr.c_str());
    string sign = MD5(tempStr).md5();
    for(int i=0;i<sign.length();i++)
    {
        if(sign[i]>='a'&&sign[i]<='z')
            sign[i]-=32;
        else if
            (sign[i]>='A'&&sign[i]<='Z')sign[i]+=32;
    }
    return sign;
}

void CDNewsViewController::viewDidLoad()
{
    winSize = this->getView()->getBounds().size;
    if (m_msg.empty())
    {
        std::map<std::string,
        std::string> key_value;
        key_value["tag"] = menuTag[urlID];
        key_value["page"]= "1";
        key_value["limit"]= "20";
        key_value["appid"]="10000";
        key_value["sign_method"]="md5";
        string tempSign = getSign(key_value);
        key_value["sign"] = tempSign;
        string tempUrl = crossapp_format_string("http://api.doubi.so/%s/",m_pNewsType.c_str());
        CommonHttpManager::getInstance()->send_get(tempUrl, key_value, this,
                                                   CommonHttpJson_selector(CDNewsViewController::onRequestFinished));
        
        
        p_pLoading = CAActivityIndicatorView::createWithCenter(DRect(winSize.width/2,winSize.height/2,50,50));
        this->getView()->insertSubview(p_pLoading, CAWindowZOderTop);
        p_pLoading->setLoadingMinTime(0.5f);
        p_pLoading->setTargetOnCancel(this, callfunc_selector(CDNewsViewController::initNewsView));
    }
    else
    {
        this->initNewsView();
    }
}

void CDNewsViewController::showAlert()
{
    if (p_alertView) {
        this->getView()->removeSubview(p_alertView);
        p_alertView= NULL;
    }

    p_alertView = CAView::createWithFrame(this->getView()->getBounds());
    this->getView()->addSubview(p_alertView);
    
    CAImageView* bg = CAImageView::createWithFrame(DRect(0,0,winSize.width,winSize.height));
    bg->setImageViewScaleType(CAImageViewScaleTypeFitImageCrop);
    bg->setImage(CAImage::create("image/HelloWorld.png"));
    
    CAButton* btn5 = CAButton::create(CAButtonTypeSquareRect);
    btn5->setTag(100);
    btn5->setCenter(DRect(winSize.width/2, winSize.height/2, winSize.width, winSize.height));
    btn5->setTitleColorForState(CAControlStateNormal,CAColor_white);
    btn5->setBackGroundViewForState(CAControlStateNormal, bg);
    btn5->setBackGroundViewForState(CAControlStateHighlighted, bg);
    btn5->addTarget(this, CAControl_selector(CDNewsViewController::buttonCallBack), CAControlEventTouchUpInSide);
    p_alertView->addSubview(btn5);
    
    CALabel* test = CALabel::createWithCenter(DRect(winSize.width/2,
                                                        winSize.height-100,
                                                        winSize.width,
                                                        40));
    test->setColor(CAColor_gray);
    test->setTextAlignment(CATextAlignmentCenter);
    test->setVerticalTextAlignmet(CAVerticalTextAlignmentTop);
    test->setFontSize(_px(24));
    test->setText("网络不给力，请点击屏幕重新加载～");
    p_alertView->addSubview(test);
    
}

void CDNewsViewController::buttonCallBack(CAControl* btn,CCPoint point)
{
    this->getView()->removeSubview(p_alertView);
    p_alertView = NULL;
    std::map<std::string,
    std::string> key_value;
    key_value["tag"] = menuTag[urlID];
    key_value["page"]= "1";
    key_value["limit"]= "20";
    key_value["appid"]="10000";
    key_value["sign_method"]="md5";
    string tempSign = getSign(key_value);
    key_value["sign"] = tempSign;
    string tempUrl = crossapp_format_string("http://api.doubi.so/%s/",m_pNewsType.c_str());
    CommonHttpManager::getInstance()->send_get(tempUrl, key_value, this,
                                               CommonHttpJson_selector(CDNewsViewController::onRequestFinished));
    {
        p_pLoading = CAActivityIndicatorView::createWithCenter(DRect(winSize.width/2,winSize.height/2,50,50));
        this->getView()->insertSubview(p_pLoading, CAWindowZOderTop);
        p_pLoading->setLoadingMinTime(0.5f);
        p_pLoading->setTargetOnCancel(this, callfunc_selector(CDNewsViewController::initNewsView));
    }
}

void CDNewsViewController::onRequestFinished(const HttpResponseStatus& status, const CSJson::Value& json)
{
    if (status == HttpResponseSucceed)
    {
        if (m_pNewsType == "news") {
            const CSJson::Value& value = json["result"];
            int length = value.size();
            m_msg.clear();
            for (int index = 0; index < length; index++)
            {
                newsMsg temp_msg;
                temp_msg.m_title = value[index]["title"].asString();
                temp_msg.m_desc = value[index]["desc"].asString();
                temp_msg.m_url = value[index]["url"].asString();
                temp_msg.m_imageUrl = value[index]["image"].asString();
                m_msg.push_back(temp_msg);
            }
        }else{
            const CSJson::Value& value = json["result"];
            int length = value.size();
            
            m_ImageMsg.clear();
            for (int index = 0; index < length; index++)
            {
                newsImage temp_msg;
                temp_msg.m_title = value[index]["title"].asString();
                for (int i=0; i<value[index]["picon"].size(); i++) {
                    string temp_pic = value[index]["picon"][i]["pic"].asString();
                    string temp_dsc = value[index]["picon"][i]["desc"].asString();
                    temp_msg.m_imageUrl.push_back(temp_pic);
                    temp_msg.m_imageDesc.push_back(temp_dsc);
                }
                
                m_ImageMsg.push_back(temp_msg);
            }
        }

    }
    p_pLoading->stopAnimating();
    
    if (p_Conllection)
    {
        p_Conllection->reloadData();
        p_section=1;
    }
}

void CDNewsViewController::onRefreshRequestFinished(const HttpResponseStatus& status, const CSJson::Value& json)
{
    if (status == HttpResponseSucceed)
    {
        if (m_pNewsType == "news") {
            const CSJson::Value& value = json["result"];
            int length = value.size();
            for (int index = 0; index < length; index++)
            {
                newsMsg temp_msg;
                temp_msg.m_title = value[index]["title"].asString();
                temp_msg.m_desc = value[index]["desc"].asString();
                temp_msg.m_url = value[index]["url"].asString();
                temp_msg.m_imageUrl = value[index]["image"].asString();
                m_msg.push_back(temp_msg);
            }
        }else{
            const CSJson::Value& value = json["result"];
            int length = value.size();
            for (int index = 0; index < length; index++)
            {
                newsImage temp_msg;
                temp_msg.m_title = value[index]["title"].asString();
                for (int i=0; i<value[index]["picon"].size(); i++) {
                    string temp_pic = value[index]["picon"][i]["pic"].asString();
                    string temp_dsc = value[index]["picon"][i]["desc"].asString();
                    temp_msg.m_imageUrl.push_back(temp_pic);
                    temp_msg.m_imageDesc.push_back(temp_dsc);
                }
                
                m_ImageMsg.push_back(temp_msg);
            }
        }

    }
    
    do
    {
        CC_BREAK_IF(p_pLoading == NULL);
        if (p_pLoading->isAnimating())
        {
            p_pLoading->stopAnimating();
        }
        else
        {
            p_Conllection->reloadData();
        }
    }
    while (0);
}

void CDNewsViewController::initNewsView()
{
    if (m_pNewsType=="news"&&m_msg.empty()) {
        showAlert();
        return;
    }else if(m_pNewsType=="newsgirlpic"&&m_ImageMsg.empty())
    {
        showAlert();
        return;
    }
    p_Conllection = CAAutoCollectionView::createWithFrame(CCRect(0,0,this->getView()->getBounds().size.width,this->getView()->getBounds().size.height));
    p_Conllection->setAllowsSelection(true);
    p_Conllection->setCollectionViewDelegate(this);
    p_Conllection->setCollectionViewDataSource(this);
    p_Conllection->setScrollViewDelegate(this);
    p_Conllection->setHoriCellInterval(_px(2));
    p_Conllection->setVertCellInterval(_px(2));
    
    p_Conllection->setHoriMargins((winSize.width-1506)/4);
    p_Conllection->setBackGroundColor(ccc4(234,234,234,255));
    
    CAPullToRefreshView *refreshDiscount = CAPullToRefreshView::create(CAPullToRefreshView::CAPullToRefreshTypeFooter);
    refreshDiscount->setLabelColor(CAColor_black);
    CAPullToRefreshView *refreshDiscount1 = CAPullToRefreshView::create(CAPullToRefreshView::CAPullToRefreshTypeHeader);
    refreshDiscount1->setLabelColor(CAColor_black);
    p_Conllection->setFooterRefreshView(refreshDiscount);
    p_Conllection->setHeaderRefreshView(refreshDiscount1);
    
    this->getView()->addSubview(p_Conllection);
    p_Conllection->reloadData();
}

void CDNewsViewController::viewDidUnload()
{
    
}

void CDNewsViewController::tempCallBack()
{
    temp_time-=0.02f;
}

void CDNewsViewController::scrollViewHeaderBeginRefreshing(CrossApp::CAScrollView *view)
{
    std::map<std::string,
    std::string> key_value;
    key_value["tag"] = menuTag[urlID];
    key_value["page"]= "1";
    key_value["limit"]= "20";
    key_value["appid"]="10000";
    key_value["sign_method"]="md5";
    string tempSign = getSign(key_value);
    key_value["sign"] = tempSign;
    string tempUrl = crossapp_format_string("http://api.doubi.so/%s/",m_pNewsType.c_str());
    CommonHttpManager::getInstance()->send_get(tempUrl, key_value, this,
                                               CommonHttpJson_selector(CDNewsViewController::onRequestFinished));
}

void CDNewsViewController::scrollViewFooterBeginRefreshing(CAScrollView* view)
{
    p_section++;
    std::map<std::string,
    std::string> key_value;
    key_value["tag"] = menuTag[urlID];
    key_value["page"]= crossapp_format_string("%d",p_section);
    key_value["limit"]= "20";
    key_value["appid"]="10000";
    key_value["sign_method"]="md5";
    string tempSign = getSign(key_value);
    key_value["sign"] = tempSign;
    string tempUrl = crossapp_format_string("http://api.doubi.so/%s/",m_pNewsType.c_str());
    CommonHttpManager::getInstance()->send_get(tempUrl, key_value, this,
                                               CommonHttpJson_selector(CDNewsViewController::onRefreshRequestFinished));
}

void CDNewsViewController::collectionViewDidSelectCellAtIndexPath(CAAutoCollectionView *collectionView, unsigned int section, unsigned int item)
{
    if (m_pNewsType == "news") {
        CDWebViewController* _webController = new CDWebViewController();
        _webController->init();
        _webController->setTitle(" ");
        
        _webController->autorelease();
        RootWindow::getInstance()->getSplitNavigationController()->pushViewController(_webController, true);
        _webController->initWebView(m_msg[item].m_url);
    }else{
        CDShowNewsImage* _controller = new CDShowNewsImage();
        _controller->init();
        _controller->setTitle(" ");
        
        _controller->autorelease();
        RootWindow::getInstance()->getSplitNavigationController()->pushViewController(_controller, true);
        _controller->initNewsImageView(m_ImageMsg[item]);
    }
}

void CDNewsViewController::collectionViewDidDeselectCellAtIndexPath(CAAutoCollectionView *collectionView, unsigned int section, unsigned int item)
{
    
}

CACollectionViewCell* CDNewsViewController::collectionCellAtIndex(CAAutoCollectionView *collectionView, const CCSize& cellSize, unsigned int section, unsigned int item)
{
    DSize _size = cellSize;
    CACollectionViewCell* p_Cell = collectionView->dequeueReusableCellWithIdentifier("CrossApp");
    if (p_Cell == NULL)
    {
        p_Cell = CACollectionViewCell::create("CrossApp");
        p_Cell->setAllowsSelected(false);
        
        CAImageView* itemImage = CAImageView::createWithFrame(DRect(0, 0, _size.width, _size.height));
        itemImage->setTag(99);
        p_Cell->addSubview(itemImage);
        
        CommonUrlImageView* theImage = CommonUrlImageView::createWithFrame(DRect(30, 30, 440, 300));
        theImage->setTag(101);
        theImage->setImageViewScaleType(CAImageViewScaleTypeFitImageCrop);
        theImage->setImage(CAImage::create("image/HelloWorld.png"));
        p_Cell->addSubview(theImage);
        
        DSize itemSize = itemImage->getBounds().size;
        CALabel* itemText = CALabel::createWithFrame(DRect(30, 360, itemSize.width-60,80));
        itemText->setTag(100);
        itemText->setFontSize(_px(34));
        itemText->setTextAlignment(CATextAlignmentLeft);
        itemText->setVerticalTextAlignmet(CAVerticalTextAlignmentTop);
        p_Cell->addSubview(itemText);
        
        CAImageView* commentImage = CAImageView::createWithFrame(DRect(itemSize.width-100, itemSize.height-60, 48, 48));
        commentImage->setTag(200);
        commentImage->setImage(CAImage::create("image/comment.png"));
        p_Cell->addSubview(commentImage);
    }
    
    CAImageView* itemImageView = (CAImageView*)p_Cell->getSubviewByTag(99);
    itemImageView->setImage(CAImage::create("image/collection_bg.png"));
    
    CommonUrlImageView* theImage = (CommonUrlImageView*)p_Cell->getSubviewByTag(101);
    theImage->setImage(CAImage::create("image/HelloWorld.png"));
    CALabel* itemText = (CALabel*)p_Cell->getSubviewByTag(100);
    
    
    if (m_pNewsType == "news") {
        theImage->setUrl(m_msg[item].m_imageUrl);
        itemText->setText(m_msg[item].m_title);
        itemText->setColor(ccc4(20,20,20,255));
    }else{
        theImage->setUrl(m_ImageMsg[item].m_imageUrl[1]);
        itemText->setText(m_ImageMsg[item].m_title);
        itemText->setColor(ccc4(20,20,20,255));
    }
    
    return p_Cell;
}

unsigned int CDNewsViewController::numberOfSections(CAAutoCollectionView *collectionView)
{
    return 1;
}


unsigned int CDNewsViewController::numberOfItemsInSection(CAAutoCollectionView *collectionView, unsigned int section)
{
    int tempNum = 0;
    if (m_pNewsType=="news") {
        tempNum = (int)m_msg.size();
    }else{
        tempNum = (int)m_ImageMsg.size();
    }
    return tempNum;
}

CCSize CDNewsViewController::collectionViewSizeForItemAtIndexPath(CAAutoCollectionView* collectionView, unsigned int section, unsigned int item)
{
    return DSize(500, 600);
}