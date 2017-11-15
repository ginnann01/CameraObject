#include "IRCameraObject.h"

void IRCameraObject::ImageAcquireSetup(){
	PvResult lResult;

    cout << "Starting pipeline" << endl;
    lPipeline->Start();

    //デバイスパラメータの取得
    pParameterArray = lDevice->GetParameters();
        
    /*
    * カメラの動作設定をすりゅ
    */

    //ソフトウェアトリガモードの設定
    lResult = pParameterArray->SetEnumValue("AcquisitionMode", "SingleFrame");
    if(!lResult)
        std::cout << "# Failed to set AcquisitionMode to SingleFrame" << std::endl;


    /*
    * カメラから情報を取得する
    */
    //必要であればデバイスパラメータ一覧(XML)を出力できる
    if(XMLOutput){
	    lDevice->DumpGenICamXML("./IRCameraParam.zip");
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
    //pParameterArray->ExecuteCommand("AcquisitionStart");
}

void IRCameraObject::ImageAcquire(){
        CaptureFlag = 0;
        PvResult lResult;
        lResult = pParameterArray->ExecuteCommand("AcquisitionStart");

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
