#include "CameraObject.h"

class IRCameraObject : public CameraObject{
public:
	void ImageAcquireSetup();
	void ImageAcquire();
};