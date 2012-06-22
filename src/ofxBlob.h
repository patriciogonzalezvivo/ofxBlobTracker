/*
* ofBlob.h
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
    
    //  Finger/Hand detection
    //
    int                 nFingers;       //  Active fingers
    vector<ofPoint>     fingers;        //  Position of active fingers
    bool                gotFingers;     //  If in some point got fingers we can supose that it's a hand
    ofPoint             palm;
    ofColor             color;

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
        area 		= 0.0f;
        length 		= 0.0f;
        hole 		= false;
        nPts        = 0;
        
        gotFingers  = false;
        
        age			= 0.0f;
        sitting		= 0.0f;
        lastTimeTimeWasChecked = ofGetElapsedTimeMillis(); //get current time as of creation
    }

    bool isCircular(){
        bool rta = false;
        
        cout << "Width-Height: " << abs(boundingRect.width-boundingRect.height) << endl;
        
        if ( abs(boundingRect.width-boundingRect.height) < 0.02 ){
            
            float maxDistance = 0;
            float minDistance = 10000;
            
            for(unsigned int i = 0; i < pts.size(); i++){
                float dist = centroid.distance( pts[i] );
                
                if (dist > maxDistance)
                    maxDistance = dist;
                
                if (dist < minDistance)
                    minDistance = dist;
            }
            
            cout << "Min-Max: " << abs(maxDistance - minDistance) << endl;
            if ( abs(maxDistance - minDistance) < 0.015 )
                rta = true;
        }
        
        return  rta;
    }
    
    //----------------------------------------
    void drawContours(float x = 0, float y = 0, float outputWidth = ofGetWidth(), float outputHeight = ofGetHeight()) {
        ofPushMatrix();
        ofPushStyle();
        
        ofNoFill();
        ofBeginShape();
        for (unsigned int i = 0; i < pts.size(); i++)
            ofVertex(x + pts[i].x * outputWidth, y + pts[i].y * outputHeight);
        ofEndShape(true);
        
        ofPopStyle();
        ofPopMatrix();
    }
	
    void drawCenter(float x = 0, float y = 0, float outputWidth = ofGetWidth(), float outputHeight = ofGetHeight()) {
        ofPushMatrix();
        
        ofTranslate(x + angleBoundingRect.x * outputWidth, y + angleBoundingRect.y * outputHeight);
        ofRotate(angle+90, 0.0f, 0.0f, 1.0f);
        ofTranslate(-(x + angleBoundingRect.x * outputWidth), -(y + angleBoundingRect.y * outputHeight)); 
    
        ofPushStyle();
        ofNoFill();
        ofRect(x + (angleBoundingRect.x) * outputWidth, y + ((angleBoundingRect.y - angleBoundingRect.height/2)) * outputHeight, 1, (angleBoundingRect.height) * outputHeight); //Vertical Plus
        ofRect(x + ((angleBoundingRect.x - angleBoundingRect.width/2)) * outputWidth, y + (angleBoundingRect.y) * outputHeight, (angleBoundingRect.width) * outputWidth, 1); //Horizontal Plus
        
        ofPopStyle();
        ofPopMatrix();
    }
	
    void drawBox(float x = 0, float y = 0, float outputWidth = ofGetWidth(), float outputHeight = ofGetHeight()){		
        ofPushMatrix();
        ofTranslate(x + angleBoundingRect.x * outputWidth, y + angleBoundingRect.y * outputHeight);
        ofRotate(angle+90, 0.0f, 0.0f, 1.0f);
        ofTranslate(-(x + angleBoundingRect.x * outputWidth), -(y + angleBoundingRect.y * outputHeight));                
        ofNoFill();
		 
        ofPushStyle();
        ofNoFill();
        ofRect(x + (angleBoundingRect.x - angleBoundingRect.width/2) * outputWidth, y + (angleBoundingRect.y - angleBoundingRect.height/2) * outputHeight, angleBoundingRect.width * outputWidth, angleBoundingRect.height * outputHeight);

        ofPopStyle();
        ofPopMatrix();
    }
};
#endif


