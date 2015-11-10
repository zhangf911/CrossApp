//
//  CADevice.h
//  PublishPhoto
//
//  Created by lihui on 14-10-21.
//
//

#ifndef __PublishPhoto__CADevice__
#define __PublishPhoto__CADevice__

#include <iostream>
#include "basics/CAObject.h"
#include "images/CAImage.h"

NS_CC_BEGIN

struct CAWifiInfo
{
    std::string ssid;
    int level;
    std::string mac;
};

typedef enum
{
    CABLUETOOTHOEPNSUCCESS=0,
    CABLUETOOTHISOEPN,
    CABLUETOOTHOPENING,
    CABLUETOOTHCLOSESUCCESS,
    CABLUETOOTHCLOSED,
    CABLUETOOTHCLOSEFAILD
}CABlueToothState;

typedef enum
{
    CABLUETOOTHOPEN = 0,
    CABLUETOOTHCLOSE,
    CABLUETOOTHDISCOVERY = 3 ,
    CABLUETOOTHCANCELDISCOVERY
    
}CABlueToothType;

struct CABlueToothUnit
{
    std::string address;
    std::string name;
};

class CC_DLL CAMediaDelegate
{
public:
    virtual ~CAMediaDelegate(){};
    
    virtual void getSelectedImage(CAImage *image) = 0;
};

class CC_DLL CABlueToothDelegate
{
public:
    virtual ~CABlueToothDelegate(){};
    
    virtual void getBlueToothState(CABlueToothState state) {};
    
    virtual void getSearchBlueToothDevice(CABlueToothUnit unit){};
    
    virtual void startDiscoveryBlueToothDevice(){};
    
    virtual void finishedDiscoveryBlueToothDevice(){};
};

class CC_DLL CAWifiDelegate
{
public:
    virtual ~CAWifiDelegate(){};
    
    virtual void getWifiListFunc(std::vector<CAWifiInfo> _wifiInfoList) = 0;
};


struct CC_DLL CAAddressBookRecord
{
    std::string firstName;
    std::string lastName;
    std::string middleName;
    std::string prefix;
    std::string suffix;
    std::string nickname;
    std::string firstNamePhonetic;
    std::string lastNamePhonetic;
    std::string middleNamePhonetic;
    std::string organization;
    std::string jobtitle;
    std::string department;
    std::string birthday;
    std::string note;
    std::string lastEdit;
    std::string email;
    std::string country;
    std::string city;
    std::string province;
    std::string street;
    std::string zip;
    std::string countrycode;
    std::string phoneNumber;
    std::string fullname;
};

typedef enum
{
    CANetWorkTypeWifi=0,
    CANetWorkType3G,
    CANetWorkTypeNone
    
}CANetWorkType;

namespace CADevice
{
    CC_DLL const char* getSystemVersionWithIOS();
    
    CC_DLL const char* getAppVersion();
    
    CC_DLL void openCamera(CAMediaDelegate* target);
    
    CC_DLL void openAlbum(CAMediaDelegate* target);
    
    CC_DLL float getScreenBrightness();
    
    CC_DLL void setScreenBrightness(float brightness);
    
    CC_DLL void writeToSavedPhotosAlbum(const std::string &s);
    
    CC_DLL std::vector<CAAddressBookRecord> getAddressBook();
    
    CC_DLL void updateVersion(const std::string &url
                              ,unsigned int versionNumber
                              ,const std::string &appId);
    
    CC_DLL CANetWorkType getNetWorkType();
    
    CC_DLL void getWifiList(CAWifiDelegate *target);
    
    CC_DLL void setVolume(float sender, int type);
    
    CC_DLL float getVolume(int type);
    
    CC_DLL void OpenURL(const std::string &url);
    
    CC_DLL float getBatteryLevel();
    
    CC_DLL bool isNetWorkAvailble();
    
    CC_DLL void sendLocalNotification(const char* title, const char* content, unsigned long time);
    
    CC_DLL CAWifiInfo getWifiConnectionInfo();
    
    CC_DLL void initBlueTooth(CABlueToothDelegate *target);
    
    CC_DLL void setBlueToothType(CABlueToothType type);
};

NS_CC_END

#endif /* defined(__PublishPhoto__CADevice__) */

