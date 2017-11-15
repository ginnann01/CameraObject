#include "IRCameraObject.h"
#include "RGBCameraObject.h"
#include <thread>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/optional.hpp>
#include <unistd.h>

PV_INIT_SIGNAL_HANDLER();

//
// Main function
//
int main(){
    boost::property_tree::ptree pt;
    boost::property_tree::read_ini("Settings.ini", pt);

    bool OutputAVI;
    bool PreviewWindow;
    int VideoFPS;

    //カメラオブジェクトの作成
	IRCameraObject IRCamera;
	RGBCameraObject RGBCamera;

	//----------
	//オプションファイルの読み込み
    //----------
    if(boost::optional<bool> value = pt.get_optional<bool>("Flags.outputAVI")){
        OutputAVI = *value;
    }
    if(boost::optional<bool> value = pt.get_optional<bool>("Flags.PreviewWindow")){
        PreviewWindow = *value;
    }
    if(boost::optional<int> value = pt.get_optional<int>("VideoParam.FPS")){
        VideoFPS = *value;
        IRCamera.FPS = *value;
        RGBCamera.FPS = *value;
    }
    if(boost::optional<bool> value = pt.get_optional<bool>("Flags.outputXML")){
        IRCamera.XMLOutput = *value;
    }
    if(boost::optional<bool> value = pt.get_optional<bool>("Flags.outputXML")){
        RGBCamera.XMLOutput = *value;
    }

    //----------
    //IRカメラのデバイス選択
    //----------
    std::cout << "----------" << std::endl;
	std::cout << "IRCamera Setup" << std::endl;
	std::cout << "----------" << std::endl;
    IRCamera.getDeviceID();
    if(!IRCamera.DeviceHealth){
        return 0;
    }

    //----------
	//RGBカメラのデバイス選択
	//----------
	std::cout << "----------" << std::endl;
	std::cout << "RGBCamera Setup" << std::endl;
	std::cout << "----------" << std::endl;
    RGBCamera.getDeviceID();
    if(!RGBCamera.DeviceHealth){
        return 0;
    }


    //----------
    //各カメラの接続処理
    //----------

    //IRカメラの接続
    IRCamera.Connect2Device();
    if(!IRCamera.DeviceHealth){
        return 0;
    }
    IRCamera.ImageAcquireSetup();

    std::cout << "IRCamera ImageSize : " << IRCamera.iHeight << "x" << IRCamera.iWidth << std::endl;
    
    //RGBカメラの接続
    RGBCamera.Connect2Device();
    if(!RGBCamera.DeviceHealth){
        return 0;
    }
    RGBCamera.ImageAcquireSetup();

    std::cout << "RGBCamera ImageSize : " << RGBCamera.iHeight << "x" << RGBCamera.iWidth << std::endl;

    
    //----------
    //動画保存
    //----------
    cv::VideoWriter IRWriter;
    cv::VideoWriter RGBWriter;
    if(OutputAVI){
        IRWriter.open("./IRoutput.avi", 0/*非圧縮AVI*/, VideoFPS, cv::Size(IRCamera.iWidth, IRCamera.iHeight), true );
        RGBWriter.open("./RGBoutput.avi", 0/*非圧縮AVI*/, VideoFPS, cv::Size(RGBCamera.iWidth, RGBCamera.iHeight), true );
    }

    //FPS固定用の処理時間測定
    cv::TickMeter meter;
    double elapsedSec;
    std::cout << "-----Start ImageAcquire-----" << std::endl;
    cv::namedWindow("IRwindow",1);
    cv::namedWindow("RGBwindow",1);

    //double *f;
    //IRCamera.pParameterArray->GetFloatValue("AcquisitionFrameRate", &f);
    //std::cout << "IR FrameRate : " << f << std::endl;
    //RGBCamera.pParameterArray->GetFloatValue("AcquisitionFrameRate", &f);
    //std::cout << "RGB FrameRate : " << f << std::endl;
    //std::cout << "IR Framerate : " << IRCamera.pParameterArray->Get("AcquisitionFrameRate")->ToString.GetAscii << std::endl;
    
    //----------
    //撮影処理
    //----------
    while(1){
    	meter.start();

    	//画像の取り込みを2スレッドに分けて並列実行
    	std::thread th_CapIR(&IRCameraObject::ImageAcquire, &IRCamera);
    	std::thread th_CapRGB(&RGBCameraObject::ImageAcquire, &RGBCamera);
    	th_CapIR.join();
    	th_CapRGB.join();

        if(IRCamera.CaptureFlag == 1 && RGBCamera.CaptureFlag == 1){
            if(PreviewWindow){
                cv::imshow("IRwindow", IRCamera.PreviewImage);
                cv::imshow("RGBwindow", RGBCamera.PreviewImage);
            }
            if(OutputAVI){
     	      IRWriter << IRCamera.Image;
     	      RGBWriter << RGBCamera.Image;
            }
     	  if(cv::waitKey(1) != -1){
    	       break;
    	   }
        }else{
            std::cout << "Capture Failed" << std::endl;
            if(cv::waitKey(1) != -1){
               break;
           }
        }

    	meter.stop();
    	elapsedSec = meter.getTimeSec();
    	std::cout << elapsedSec << "Sec" << std::endl;
    	meter.reset();
    	if(elapsedSec < 1.0/(double)VideoFPS){
    		//std::cout << "wait " << 1.0/(double)VideoFPS-elapsedSec << std::endl;
    		usleep( (1.0/(double)VideoFPS-elapsedSec) * 1000000 );
    	}else{
    		std::cout << "ProcessTime exseed the FPS limit" << std::endl;
    	}

    }
    
    std::cout << "-----Finish ImageAcquire-----" << std::endl;
    std::cout << "Frame Time : " << elapsedSec << " [Sec]" << std::endl;
    if(OutputAVI){
        IRWriter.release();
        RGBWriter.release();
    }

    IRCamera.ImageAcquireFinish();
    RGBCamera.ImageAcquireFinish();

    return 0;
}