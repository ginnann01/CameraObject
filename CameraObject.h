#pragma once

#include <PvSampleUtils.h>
#include <PvDevice.h>
#include <PvDeviceGEV.h>
#include <PvDeviceU3V.h>
#include <PvStream.h>
#include <PvStreamGEV.h>
#include <PvStreamU3V.h>
#include <PvPipeline.h>
#include <PvBuffer.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>

#include <unistd.h>

#define BUFFER_COUNT ( 45 )

class CameraObject{

public:

	//デバイス接続関連
	//必要な場合に適宜外からアクセスできるよう公開
    PvDevice *lDevice = NULL;
    PvStream *lStream = NULL;
    PvString lConnectionID;
    PvPipeline *lPipeline = NULL;
    PvGenParameterArray *pParameterArray;
    bool DeviceHealth = true;

    double FPS;


    //画像取得処理関連
    cv::Mat imgBuffer;
    cv::Mat Image;
    cv::Mat PreviewImage;
    //取得画像の高さと横幅
    int iHeight, iWidth;

    int CaptureFlag = 0;

    bool XMLOutput = false;

    CameraObject();
    ~CameraObject();

    //デバイス一覧からデバイスを選択してIDを取得
    void getDeviceID();

    //デバイスへの接続
    void Connect2Device();

    virtual void ImageAcquireSetup() = 0;
    virtual void ImageAcquire() = 0;
    void ImageAcquireFinish();

protected:
    bool cv2colorFlag = false;
    uint32_t colorCvtType;
    
    /*
    指定デバイスへの接続
    */
    PvDevice *ConnectToDevice( const PvString &aConnectionID );

    /*
    ストリームを開く
    */
    PvStream *OpenStream( const PvString &aConnectionID );

    void ConfigureStream( PvDevice *aDevice, PvStream *aStream );

    PvPipeline *CreatePipeline( PvDevice *aDevice, PvStream *aStream );

    void InitMatBuffer();

    void ImageConvert2OpenCV(PvBuffer *lBuffer);

    bool setCameraColorType(PvGenEnum * type);

};



