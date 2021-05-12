#include "ofApp.h"

using namespace cv;
using namespace ofxCv;

//--------------------------------------------------------------
void ofApp::setup() {
    settings.loadFile("settings.xml");
    
    ofSetVerticalSync(false);
    ofHideCursor();
    
    inputUrl = "input";
    outputUrl = "output";
    ofDirectory inputDir(inputUrl);
    inputDir.allowExt(inputFileType);
    inputDir.listDir();
    inputDir.sort();
    for (int i=0; i<inputDir.size(); i++) {
        string url = inputDir.getPath(i);
        cout << "Found " << url << endl;
        inputUrls.push_back(url);
    }
    
    appFramerate = settings.getValue("settings:app_framerate", 60);
    ofSetFrameRate(appFramerate);

    videoColor = (bool) settings.getValue("settings:video_color", 0); 
    
    width = settings.getValue("settings:width", 720);
    height = settings.getValue("settings:height", 480);
    camWidth = settings.getValue("settings:cam_width", 640);
    camHeight = settings.getValue("settings:cam_height", 480);
    ofSetWindowShape(width, height);

    debug = (bool) settings.getValue("settings:debug", 1);
    
    lineWidth = settings.getValue("settings:line_width", 10); 
    alphaVal = settings.getValue("settings:alpha_val", 255); 
    contourSlices = settings.getValue("settings:contour_slices", 10); 
    drawWireframe = (bool) settings.getValue("settings:draw_wireframe", 0); 
        
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
    //timestamp = (int) ofGetSystemTimeMillis();
    
    if (counter < inputUrls.size()) {
        gray.load(inputUrls[counter]);
        pixels.allocate(gray.getWidth(), gray.getHeight(), OF_IMAGE_COLOR);
        frame = toCv(gray.getPixels());
        
        fbo.allocate(gray.getWidth(), gray.getHeight(), GL_RGBA);
        
        counter++;
    }
}

//--------------------------------------------------------------
void ofApp::draw() {
    ofBackground(0);
    
    fbo.begin();
    ofClear(0,0,0,0);
               
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
               
            contourCounter++;
        }
    }

    fbo.end();

    gray.draw(0, 0, width/2, height);
    fbo.draw(width/2, 0, width, height);
    
    ofSaveScreen(ofFilePath::join(outputUrl, ofToString(counter)) + "." + outputFileType);
}
