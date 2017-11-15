#include "RGBCameraObject.h"

void RGBCameraObject::ImageAcquireSetup(){
	PvResult lResult;

    cout << "Starting pipeline" << endl;
    lPipeline->Start();

    //デバイスパラメータの取得
    pParameterArray = lDevice->GetParameters();
        
    /*
    * カメラの動作設定をすりゅ
    */
    /*
    std::cout << "---------RGB FPS Set to " << FPS << std::endl;
    lResult = pParameterArray->SetFloatValue("AcquisitionFrameRate", (double)FPS);
    if(!lResult.IsOK())
        std::cout << "RGB FPS set failue" << std::endl;
    double f = 0;
    lResult = pParameterArray->GetFloatValue("AcquisitionFrameRate", f);
    if(lResult.IsOK()){
        std::cout << "----------RGB FPS : " << f << std::endl;
    }
    */

    //ソフトウェアトリガモードの設定
    pParameterArray->SetEnumValue("TriggerMode", "On");
    lResult = pParameterArray->SetEnumValue("ExposureMode", "Timed");
    if(!lResult.IsOK())
        std::cout << "----------ExMode set error" << std::endl;
    double extime = 10000;
    lResult = pParameterArray->SetFloatValue("ExposureTime", extime);
    if(!lResult.IsOK())
        std::cout << "----------ExTime set error" << std::endl;
    pParameterArray->SetEnumValue("TriggerSource", "Software");
    lResult = pParameterArray->SetEnumValue("TriggerSoftwareSource", "Command");
    if(!lResult.IsOK())
        std::cout << "----------Command set error" << std::endl;
    //lResult = pParameterArray->SetFloatValue("TriggerDelay", 100000);
    //if(!lResult.IsOK())
    //    std::cout << "----------Delay set error" << std::endl;

    /*
    * カメラから情報を取得する
    */
    //必要であればデバイスパラメータ一覧(XML)を出力できる
    if(XMLOutput){
	    lDevice->DumpGenICamXML("./RGBCameraParam.zip");
	}

    //画像のカラーフォーマット
    PvGenEnum * pixFormat = dynamic_cast<PvGenEnum *>( pParameterArray->Get( "PixelFormat" ) );
    cv2colorFlag = setCameraColorType(pixFormat);
        
    //画像の高さと横幅
    PvGenInteger * imgHeight = dynamic_cast<PvGenInteger *>( pParameterArray->Get( "Height" ) );
    PvGenInteger * imgWidth = dynamic_cast<PvGenInteger *>( pParameterArray->Get( "Width" ) );
    int64_t h, w;
    imgHeight->GetValue(h);
    imgWidth->GetValue(w);
    iHeight  = h;
    iWidth = w;

    //Mat型のbufferを確保
    InitMatBuffer();

    lDevice->StreamEnable();
    pParameterArray->ExecuteCommand("AcquisitionStart");
}

void RGBCameraObject::ImageAcquire(){
        CaptureFlag = 0;
        PvResult lResult;
        lResult = pParameterArray->ExecuteCommand("TriggerSoftware");

        PvBuffer *lBuffer = NULL;
        PvResult lOperationResult;
        lResult = lPipeline->RetrieveNextBuffer(&lBuffer, 1000, &lOperationResult);
            
        if(lResult.IsOK()){
            if(lOperationResult.IsOK()){
                ImageConvert2OpenCV(lBuffer);
                cv::resize(Image, PreviewImage, cv::Size(640, 480));
                CaptureFlag = 1;
            }
            //bufferのクリア
            lPipeline->ReleaseBuffer(lBuffer);
        }else{
            //画像取得に失敗(タイムアウト)
            std::cout << "failed to aquire (Connection Timeout)" << endl;
            CaptureFlag = 0;
        }
    }