#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "ofxCvPiCam.h"
#include "ofxXmlSettings.h"

class ofApp : public ofBaseApp {

    public:	
	void setup();
	void update();
	void draw();
		
	int width, height, appFramerate, camFramerate;
	
	ofFile file;
	ofxXmlSettings settings;

	ofFbo fbo;
	ofPixels pixels;
	int rpiCamVersion; // 0 for not an RPi cam, 1, 2, or 3
	string lastPhotoTakenName;
	int stillCompression;
	int timestamp;
	   
	// ~ ~ ~ ~ ~ ~ ~     

	ofBuffer videoBuffer;
	ofBuffer photoBuffer;
	ofBuffer contourColorBuffer;
	ofBuffer contourPointsBuffer;

	ofxCvPiCam cam;
	cv::Mat frame, frameProcessed;
	ofImage gray;
	int syncVideoQuality; // 5 best to 1 worst, default 3 medium
	bool videoColor;
	bool debug;

	// for more camera settings, see:
	// https://github.com/orgicus/ofxCvPiCam/blob/master/example-ofxCvPiCam-allSettings/src/testApp.cpp

    int camRotation;        
    int camShutterSpeed; // 0 to 330000 in microseconds, default 0
    int camSharpness; // -100 to 100, default 0
    int camContrast; // -100 to 100, default 0
    int camBrightness; // 0 to 100, default 50
	int camIso; // 100 to 800, default 300
	int camExposureCompensation; // -10 to 10, default 0;

	// 0 off, 1 auto, 2 night, 3 night preview, 4 backlight, 5 spotlight, 6 sports, 7, snow, 8 beach, 9 very long, 10 fixed fps, 11 antishake, 12 fireworks, 13 max
	int camExposureMode; // 0 to 13, default 0

	//string oscAddress;
	int thresholdValue; // default 127
	int thresholdKeyCounter;
	bool thresholdKeyFast;

	ofxCv::ContourFinder contourFinder;
	float contourThreshold;  // default 127
	float contourMinAreaRadius; // default 10
	float contourMaxAreaRadius; // default 150
	int contourSlices; // default 20
	ofxCv::TrackingColorMode trackingColorMode; // RGB, HSV, H, HS; default RGB

};
