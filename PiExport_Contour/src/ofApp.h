#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "ofxXmlSettings.h"

class ofApp : public ofBaseApp {

    public:	
	void setup();
	void update();
	void draw();
	
	string url;
	int width, height, camWidth, camHeight, appFramerate;
	int alphaVal;
	float lineWidth;
	bool drawWireframe;
	
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

	cv::Mat frame, frameProcessed;
	ofImage gray;
	int syncVideoQuality; // 5 best to 1 worst, default 3 medium
	bool videoColor;
	bool debug;

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
