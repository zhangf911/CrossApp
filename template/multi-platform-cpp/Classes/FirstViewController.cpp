
#include "FirstViewController.h"

FirstViewController::FirstViewController()
{

}

FirstViewController::~FirstViewController()
{

}

void FirstViewController::viewDidLoad()
{
    // Do any additional setup after loading the view from its nib.
	CCRect winRect = this->getView()->getBounds();
    CAImageView* imageView = CAImageView::createWithImage(CAImage::create("r/HelloWorld.png"));
    imageView->setImageViewScaleType(CAImageViewScaleTypeFitImageCrop);
    imageView->setFrame(winRect);
    this->getView()->addSubview(imageView);

    CALabel* label = CALabel::createWithCenter(CCRect(winRect.size.width*0.5, winRect.size.height*0.5-270, winRect.size.width, 200));
    label->setTextAlignment(CATextAlignmentCenter);
    label->setVerticalTextAlignmet(CAVerticalTextAlignmentCenter);
    label->setFontSize(_px(72));
    label->setText("Hello World!");
    label->setColor(CAColor_white);
    this->getView()->insertSubview(label, 1);
    
    CCLog("%f", CAApplication::getApplication()->getWinSize().width);
}

void FirstViewController::viewDidUnload()
{
    // Release any retained subviews of the main view.
    // e.g. self.myOutlet = nil;
}