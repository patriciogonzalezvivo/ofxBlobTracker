/*
* ofBlob.h
*
*
* A blob is a homogenous patch represented by a polygonal contour.
* Typically a blob tracker uses the contour to figure out the blob's
* persistence and "upgrades" it with ids and other temporal
* information.
*
*/

#ifndef OFXBLOB
#define OFXBLOB

#include <vector>
#include "ofMain.h"
#include "ofxCvBlob.h"

class ofxBlob : public ofxCvBlob {
public: 
    int                 nFingers;
    vector<ofPoint>     fingers;

    ofRectangle         angleBoundingRect;
    ofPoint             lastCentroid, D;
    float				lastTimeTimeWasChecked;
    float               angle, downTime;
    float				maccel;  //distance traveled since last frame
    float				age;     //how long the blob has been at war
    float				sitting; //how long hes been sitting in the same place
    int					id;
    		
    //----------------------------------------
    ofxBlob() {
        // ofxCvBlob
        area 		= 0.0f;
        length 		= 0.0f;
        hole 		= false;
        nPts        = 0;
        
        age			= 0.0f;
        sitting		= 0.0f;
        lastTimeTimeWasChecked = ofGetElapsedTimeMillis(); //get current time as of creation
    }

    //----------------------------------------
    void drawContours(float x = 0, float y = 0, float inputWidth = ofGetWidth(), float inputHeight = ofGetHeight(), float outputWidth =ofGetWidth(), float outputHeight = ofGetHeight()) {
        //draw contours
        ofPushStyle();
        ofNoFill();
        ofBeginShape();
        for (int i = 0; i < pts.size(); i++)
            ofVertex(x + pts[i].x/inputWidth * outputWidth, y + pts[i].y/(inputHeight) * outputHeight);
        ofEndShape(true);
        ofPopStyle();
    }
	
    void drawCenter(float x = 0, float y = 0, float inputWidth = ofGetWidth(), float inputHeight = ofGetHeight(), float outputWidth = ofGetWidth(), float outputHeight = ofGetHeight()) {
        ofPushMatrix();
        
        ofTranslate(x + angleBoundingRect.x/inputWidth * outputWidth, y + angleBoundingRect.y/inputHeight * outputHeight);
        ofRotate(angle+90, 0.0f, 0.0f, 1.0f);
        ofTranslate(-(x + angleBoundingRect.x/inputWidth * outputWidth), -(y + angleBoundingRect.y/inputHeight * outputHeight)); 
    
        ofPushStyle();
        ofNoFill();
        ofRect(x + (angleBoundingRect.x/inputWidth) * outputWidth, y + ((angleBoundingRect.y - angleBoundingRect.height/2)/inputHeight) * outputHeight, 1, (angleBoundingRect.height)/inputHeight * outputHeight); //Vertical Plus
        ofRect(x + ((angleBoundingRect.x - angleBoundingRect.width/2)/inputWidth) * outputWidth, y + (angleBoundingRect.y/inputHeight) * outputHeight, (angleBoundingRect.width)/inputWidth * outputWidth, 1); //Horizontal Plus
        
        ofPopStyle();
        ofPopMatrix();
    }
	
    void drawBox(float x = 0, float y = 0, float inputWidth = ofGetWidth(), float inputHeight = ofGetHeight(), float outputWidth = ofGetWidth(), float outputHeight = ofGetHeight()){		
        ofPushMatrix();
        ofTranslate(x + angleBoundingRect.x/inputWidth * outputWidth, y + angleBoundingRect.y/inputHeight * outputHeight);
        ofRotate(angle+90, 0.0f, 0.0f, 1.0f);
        ofTranslate(-(x + angleBoundingRect.x/inputWidth * outputWidth), -(y + angleBoundingRect.y/inputHeight * outputHeight));                
        ofNoFill();
		 
        ofPushStyle();
        ofNoFill();
        ofRect(x + (angleBoundingRect.x - angleBoundingRect.width/2)/inputWidth * outputWidth, y + (angleBoundingRect.y - angleBoundingRect.height/2)/inputHeight * outputHeight, angleBoundingRect.width/inputWidth * outputWidth, angleBoundingRect.height/inputHeight * outputHeight);

        ofPopStyle();
        ofPopMatrix();
    }
};
#endif


