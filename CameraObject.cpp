#include "CameraObject.h"

CameraObject::CameraObject(){
    PV_SAMPLE_INIT();
}

CameraObject::~CameraObject(){

    if(lStream != NULL){
        cout << "Closing stream" << endl;
        lStream->Close();
        PvStream::Free( lStream );
    }

    if(lDevice != NULL){
        cout << "Disconnecting device" << endl;
        lDevice->Disconnect();
        PvDevice::Free( lDevice );
    }

    PV_SAMPLE_TERMINATE();
    
    std::cout << "Device terminated" << std::endl;

}

void CameraObject::getDeviceID(){
    if(!PvSelectDevice(&lConnectionID)){
        std::cout << "Device select error" << std::endl;
        DeviceHealth = false;
        return;
    }
    std::cout << "DeviceID is get" << std::endl;
}

//デバイスへの接続
void CameraObject::Connect2Device(){
    if(DeviceHealth){
        lDevice = ConnectToDevice(lConnectionID);
        if(lDevice == NULL){
            std::cout << "Device connect error" << std::endl;
            DeviceHealth = false;
            return;
        }
        std::cout << "Device connected" << std::endl;
        lStream = OpenStream(lConnectionID);
        
        if(lStream == NULL){
            std::cout << "Stream open error" << std::endl;
            DeviceHealth = false;
            return;
        }
        std::cout << "Stream opened" << std::endl;
        ConfigureStream( lDevice, lStream );
        lPipeline = CreatePipeline( lDevice, lStream );
        if(lPipeline == NULL){
            std::cout << "Pipeline create error" << std::endl;
            DeviceHealth = false;
            return;
        }
        std::cout << "Pipeline created" << std::endl;
    
    }else{
        std::cout << "Device Health error" << std::endl;
        return;
    }
}

    /*
    指定デバイスへの接続
    */
    PvDevice *CameraObject::ConnectToDevice( const PvString &aConnectionID )
    {
        PvDevice *lDevice;
        PvResult lResult;

        // Connect to the GigE Vision or USB3 Vision device
        cout << "Connecting to device" << endl;
        lDevice = PvDevice::CreateAndConnect( aConnectionID, &lResult );
        if ( lDevice == NULL )
        {
            cout << "Unable to connect to device" << endl;
        }

        return lDevice;
    }

    /*
    ストリームを開く
    */
    PvStream *CameraObject::OpenStream( const PvString &aConnectionID )
    {
        PvStream *lStream;
        PvResult lResult;

        // Open stream to the GigE Vision or USB3 Vision device
        cout << "Opening stream from device" << endl;
        lStream = PvStream::CreateAndOpen( aConnectionID, &lResult );
        if ( lStream == NULL )
        {
            cout << "Unable to stream from device" << endl;
        }

        return lStream;
    }

    void CameraObject::ConfigureStream( PvDevice *aDevice, PvStream *aStream )
    {
        // If this is a GigE Vision device, configure GigE Vision specific streaming parameters
        PvDeviceGEV* lDeviceGEV = dynamic_cast<PvDeviceGEV *>( aDevice );
        if ( lDeviceGEV != NULL )
        {
            PvStreamGEV *lStreamGEV = static_cast<PvStreamGEV *>( aStream );

            // Negotiate packet size
            lDeviceGEV->NegotiatePacketSize();

            // Configure device streaming destination
            lDeviceGEV->SetStreamDestination( lStreamGEV->GetLocalIPAddress(), lStreamGEV->GetLocalPort() );
        }
    }

    PvPipeline *CameraObject::CreatePipeline( PvDevice *aDevice, PvStream *aStream ){
        //Pipelineオブジェクトの作成(newで作るのでDelete必須)
        PvPipeline* lPipeline = new PvPipeline( aStream );

        if ( lPipeline != NULL ){        
            // Reading payload size from device
            uint32_t lSize = aDevice->GetPayloadSize();
            printf("Payload Size :%d\n", lSize);
        
            // Set the Buffer count and the Buffer size
            lPipeline->SetBufferCount( BUFFER_COUNT );
            lPipeline->SetBufferSize( lSize );
        }
        
        return lPipeline;
    }

    /*
    画像の取り込み
    */
    /*
    void CameraObject::ImageAcquire(){
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
    */
    void CameraObject::ImageAcquireFinish(){
        pParameterArray->ExecuteCommand("AcquisitionStop");

        // Disable streaming on the device
        cout << "Disable streaming on the controller." << endl;
        lDevice->StreamDisable();

        // Stop the pipeline
        cout << "Stop pipeline" << endl;
        lPipeline->Stop();
        delete lPipeline;

    }

    void CameraObject::InitMatBuffer(){
    	imgBuffer.create(iHeight, iWidth, CV_8UC1);
    	imgBuffer.setTo(0);
    }

    void CameraObject::ImageConvert2OpenCV(PvBuffer *lBuffer){
    	PvImage *lImage = lBuffer->GetImage();
    	uint32_t lWidth = lImage->GetWidth();
        uint32_t lHeight = lImage->GetHeight();

    	//cv::Mat im;
		memcpy(imgBuffer.data, lImage->GetDataPointer(), lImage->GetEffectiveImageSize());

    	if(cv2colorFlag){
	    	cv::cvtColor(imgBuffer, Image, colorCvtType);
	    }
    }

bool CameraObject::setCameraColorType(PvGenEnum * type){
	// Check type and set inner type parameter to correct one
	PvString enumTypeString;
	type->GetValue(enumTypeString);

	if(enumTypeString == "BayerRG8")
	{
	    printf("BayerRG8 format confirmed.\n");
	    colorCvtType = CV_BayerBG2BGR;
	    return true;
	} 
	else if(enumTypeString == "BayerGB8") 
	{
	    printf("BayerGB8 format confirmed.\n");
	    colorCvtType = CV_BayerGR2BGR;
	    return true;
	} 
	else if(enumTypeString == "BayerBG8")
	{
		printf("BayerBG8 format confirmed\n");
		colorCvtType = CV_BayerBG2RGB;
		return true;
	}
    else if (enumTypeString == "Mono8")
    {
        printf("Mono8 format confirmed\n");
        colorCvtType = CV_GRAY2BGR;
        return true;
    }
	else 
	{
	    printf("%s format confirmed. Not support / need no conversion.\n", enumTypeString.GetAscii());
	    return false; 
	}
}