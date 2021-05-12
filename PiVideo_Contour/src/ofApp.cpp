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
    camWidth = settings.getValue("settings:cam_width", 640);
    camHeight = settings.getValue("settings:cam_height", 480);
    ofSetWindowShape(width, height);

    debug = (bool) settings.getValue("settings:debug", 1);
    camAutoSettings = (bool) settings.getValue("settings:cam_auto_settings", 1);
    
    lineWidth = settings.getValue("settings:line_width", 10); 
    alphaVal = settings.getValue("settings:alpha_val", 255); 
    contourSlices = settings.getValue("settings:contour_slices", 10); 
    drawWireframe = (bool) settings.getValue("settings:draw_wireframe", 0); 

    // camera
    if (videoColor) {
        gray.allocate(width, height, OF_IMAGE_COLOR);
    } else {
        gray.allocate(width, height, OF_IMAGE_GRAYSCALE);        
    }
    
    cam.setup(camWidth, camHeight, camFramerate, videoColor); // color/gray;

    camRotation = settings.getValue("settings:cam_rotation", 0); 
    camSharpness = settings.getValue("settings:sharpness", 0); 
    camContrast = settings.getValue("settings:contrast", 0); 
    camBrightness = settings.getValue("settings:brightness", 50); 
    camIso = settings.getValue("settings:iso", 300); 
    camExposureMode = settings.getValue("settings:exposure_mode", 0); 
    camExposureCompensation = settings.getValue("settings:exposure_compensation", 0); 
    camShutterSpeed = settings.getValue("settings:shutter_speed", 0);

    cam.setRotation(camRotation);
    
    if (!camAutoSettings) {
        cam.setSharpness(camSharpness);
        cam.setContrast(camContrast);
        cam.setBrightness(camBrightness);
        cam.setISO(camIso);
        cam.setExposureMode((MMAL_PARAM_EXPOSUREMODE_T) camExposureMode);
        cam.setExposureCompensation(camExposureCompensation);
        cam.setShutterSpeed(camShutterSpeed);   
        //cam.setFrameRate // not implemented in ofxCvPiCam 
    }

    fbo.allocate(camWidth, camHeight, GL_RGBA);
    pixels.allocate(camWidth, camHeight, OF_IMAGE_COLOR);
        
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
    ofBackground(0);
    
    if(!frame.empty()) {
        fbo.begin();
        ofClear(0,0,0,0);
        
        if (debug) {
            drawMat(frame, 0, 0);
        }
                   
        int contourCounter = 0;
        unsigned char * pixels = gray.getPixels().getData();
        int gw = gray.getWidth();

        for (int h=0; h<255; h += int(255/contourSlices)) {
            contourFinder.setThreshold(h);
            contourFinder.findContours(frame);
            //contourFinder.draw();            

            int n = contourFinder.size();
            for (int h = 0; h < n; h++) {
                ofPolyline line = contourFinder.getPolyline(h);
                vector<glm::vec3> cvPoints = line.getVertices();
                
                int index = cvPoints.size() / 2;
                int x = int(cvPoints[index].x);
                int y = int(cvPoints[index].y);
                ofColor col = pixels[x + y * gw];
                
                /*
                ofSetColor(col);
                ofSetLineWidth(8);
                ofNoFill();
                ofBeginShape();
                for (int j=0; j<cvPoints.size(); j++) {
                    ofVertex(cvPoints[j].x, cvPoints[j].y);
                }   
                ofEndShape();
                */
                
                ofMesh mesh;
                mesh.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);    
                
                float widthSmooth = 10;
                float angleSmooth;
                
                for (int i = 0; i < cvPoints.size(); i++) {
                    int me_m_one = i - 1;
                    int me_p_one = i + 1;
                    if (me_m_one < 0) me_m_one = 0;
                    if (me_p_one ==  cvPoints.size()) me_p_one = cvPoints.size() - 1;
                    
                    ofPoint diff = cvPoints[me_p_one] - cvPoints[me_m_one];
                    float angle = atan2(diff.y, diff.x);
                    
                    if (i == 0) {
                        angleSmooth = angle;
                    } else {
                        angleSmooth = ofLerpDegrees(angleSmooth, angle, 1.0);
                    }
                    
                    float dist = diff.length();
                    
                    float w = ofMap(dist, 0, 20, lineWidth, 2, true); //40, 2, true);
                    
                    widthSmooth = 0.9f * widthSmooth + 0.1f * w;
                    
                    ofPoint offset;
                    offset.x = cos(angleSmooth + PI/2) * widthSmooth;
                    offset.y = sin(angleSmooth + PI/2) * widthSmooth;

                    mesh.addVertex(cvPoints[i] + offset);
                    mesh.addVertex(cvPoints[i] - offset);
                }
                
                ofSetColor(col, alphaVal);
                mesh.draw();
                if (drawWireframe) {
                    ofSetColor(col);
                    mesh.drawWireframe();
                }
                        
                /*
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
                */             
                   
                contourCounter++;
            }        
        }

        fbo.end();       
    }

    fbo.draw(0,0,width,height);

    if (debug) {
        stringstream info;
        info << cam.width << "x" << cam.height << " @ "<< ofGetFrameRate() <<"fps"<< "\n";
        ofDrawBitmapStringHighlight(info.str(), 10, 10, ofColor::black, ofColor::yellow);
    }
}
