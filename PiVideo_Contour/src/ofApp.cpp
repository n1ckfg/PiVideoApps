#include "ofApp.h"

using namespace cv;
using namespace ofxCv;

//--------------------------------------------------------------
void ofApp::setup() {
    settings.loadFile("settings.xml");
    
    ofSetVerticalSync(false);
    ofHideCursor();

    appFramerate = settings.getValue("settings:app_framerate", 60);
    camFramerate = settings.getValue("settings:cam_framerate", 30);
    ofSetFrameRate(appFramerate);

    videoColor = (bool) settings.getValue("settings:video_color", 0); 
    
    width = settings.getValue("settings:width", 720);
    height = settings.getValue("settings:height", 480);
    ofSetWindowShape(width, height);

    debug = (bool) settings.getValue("settings:debug", 1);
    
    contourSlices = settings.getValue("settings:contour_slices", 10); 


    // camera
    if (videoColor) {
        gray.allocate(width, height, OF_IMAGE_COLOR);
    } else {
        gray.allocate(width, height, OF_IMAGE_GRAYSCALE);        
    }
    
    cam.setup(width, height, camFramerate, videoColor); // color/gray;

    camRotation = settings.getValue("settings:cam_rotation", 0); 
    camSharpness = settings.getValue("settings:sharpness", 0); 
    camContrast = settings.getValue("settings:contrast", 0); 
    camBrightness = settings.getValue("settings:brightness", 50); 
    camIso = settings.getValue("settings:iso", 300); 
    camExposureMode = settings.getValue("settings:exposure_mode", 0); 
    camExposureCompensation = settings.getValue("settings:exposure_compensation", 0); 
    camShutterSpeed = settings.getValue("settings:shutter_speed", 0);

    cam.setRotation(camRotation);
    cam.setSharpness(camSharpness);
    cam.setContrast(camContrast);
    cam.setBrightness(camBrightness);
    cam.setISO(camIso);
    cam.setExposureMode((MMAL_PARAM_EXPOSUREMODE_T) camExposureMode);
    cam.setExposureCompensation(camExposureCompensation);
    cam.setShutterSpeed(camShutterSpeed);
    //cam.setFrameRate // not implemented in ofxCvPiCam 
    
    fbo.allocate(width, height, GL_RGBA);
    pixels.allocate(width, height, OF_IMAGE_COLOR);
        
    thresholdValue = settings.getValue("settings:threshold", 127); 
    contourThreshold = 2.0;
    contourMinAreaRadius = 1.0;
    contourMaxAreaRadius = 250.0;   
    contourFinder.setMinAreaRadius(contourMinAreaRadius);
    contourFinder.setMaxAreaRadius(contourMaxAreaRadius);
    //contourFinder.setInvert(true); // find black instead of white
    trackingColorMode = TRACK_COLOR_RGB;
}

//--------------------------------------------------------------
void ofApp::update() {
    timestamp = (int) ofGetSystemTimeMillis();
    
    frame = cam.grab();

    if (!frame.empty()) {
        toOf(frame, gray.getPixelsRef());
    }
}

//--------------------------------------------------------------
void ofApp::draw() {
    ofBackground(255,0,0);

    if(!frame.empty()) {
        if (debug) {
            drawMat(frameProcessed, 0, 0);
            ofSetLineWidth(2);
            ofNoFill();
        }

        int contourCounter = 0;
        unsigned char * pixels = gray.getPixels().getData();
        int gw = gray.getWidth();

        for (int h=0; h<255; h += int(255/contourSlices)) {
            contourFinder.setThreshold(h);
            contourFinder.findContours(frame);
            contourFinder.draw();            

            int n = contourFinder.size();
            for (int i = 0; i < n; i++) {
                ofPolyline line = contourFinder.getPolyline(i);
                vector<glm::vec3> cvPoints = line.getVertices();

                int x = int(cvPoints[0].x);
                int y = int(cvPoints[0].y);
                ofColor col = pixels[x + y * gw];
                float colorData[3]; 
                colorData[0] = col.r;
                colorData[1] = col.g;
                colorData[2] = col.b;
                char const * pColor = reinterpret_cast<char const *>(colorData);
                std::string colorString(pColor, pColor + sizeof colorData);
                contourColorBuffer.set(colorString); 

                float pointsData[cvPoints.size() * 2]; 
                for (int j=0; j<cvPoints.size(); j++) {
                    int index = j * 2;
                    pointsData[index] = cvPoints[j].x;
                    pointsData[index+1] = cvPoints[j].y;
                }
                char const * pPoints = reinterpret_cast<char const *>(pointsData);
                std::string pointsString(pPoints, pPoints + sizeof pointsData);
                contourPointsBuffer.set(pointsString); 

                contourCounter++;
            }        
        }
    }

    if (debug) {
        stringstream info;
        info << cam.width << "x" << cam.height << " @ "<< ofGetFrameRate() <<"fps"<< "\n";
        ofDrawBitmapStringHighlight(info.str(), 10, 10, ofColor::black, ofColor::yellow);
    }
}
