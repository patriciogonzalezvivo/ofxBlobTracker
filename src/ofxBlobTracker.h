/*
 *  ofxBlobTracking.h
 *
 *  Created by Ramsin Khoshabeh on 5/4/08.
 *  Copyright 2008 risenparadigm. All rights reserved.
 *
 * Changelog:
 * 08/15/08 -- Fixed a major bug in the track algorithm
 */

#ifndef OFXBLOBTRACKER
#define OFXBLOBTRACKER

#include "ofxBlob.h"
#include "ofxContourFinder.h"

class ofxBlobTracker{
public:
	ofxBlobTracker();
    
    void    update( ofxCvGrayscaleImage& input, int _threshold = -1, 
                   int minArea = 20 ,int maxArea = (340*240)/3, int nConsidered = 10,
                   double hullPress = 20, bool bFindHoles = false, bool bUseApproximation = true);
    void    draw( float _x = 0, float _y = 0, float _width = 0, float _height = 0);
    
    int     size(){return trackedBlobs.size(); };
    
    ofxBlob operator[](unsigned int _n){ if ( (_n >= 0U) && (_n < trackedBlobs.size()) ) return trackedBlobs[_n]; };
    
    ofEvent<ofxBlob>    blobAdded;
    ofEvent<ofxBlob>    blobMoved;
    ofEvent<ofxBlob>    blobDeleted;
    
    int     movementFiltering;
    bool    bUpdateBackground;

private:
	void    track(ofxContourFinder* newBlobs);
	int     trackKnn(ofxContourFinder *newBlobs, ofxBlob *track, int k, double thresh);
    
    ofxContourFinder    contourFinder;
    ofxCvGrayscaleImage backgroundImage;
    
	vector<ofxBlob> trackedBlobs;		//tracked blobs
    
	int             IDCounter, numEnter, numLeave, nSize;
    int             width, height;
};

#endif
