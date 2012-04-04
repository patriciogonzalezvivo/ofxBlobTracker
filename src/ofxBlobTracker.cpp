//
//  ofxBlobTracker.cpp
//  museoDelJuguete
//
//  Created by Patricio González Vivo on 3/26/12.
//  Copyright (c) 2012 PatricioGonzalezVivo.com. All rights reserved.
//

#include "ofxBlobTracker.h"

ofxBlobTracker::ofxBlobTracker(){
	IDCounter = 0;
	numEnter = 0;
	numLeave = 0;
	nSize = 0; 
    
    width = 0;
    height = 0;
    
    movementFiltering = 0;
    
    bUpdateBackground = true;
}

void ofxBlobTracker::update( ofxCvGrayscaleImage& input, int _threshold, int _minArea,int _maxArea , int _nConsidered , double _hullPress , bool _bFindHoles , bool _bUseApproximation ){
    
    if (( width != input.getWidth()) || 
        ( height != input.getHeight()) ){
        width = input.getWidth();
        height = input.getHeight();
        backgroundImage.allocate(width,height);
        bUpdateBackground = true;
    }
    
    if (bUpdateBackground){
        backgroundImage = input;
        bUpdateBackground = false;
    }
    
    input.absDiff(backgroundImage);
    
    if (_threshold != -1)
        input.threshold(_threshold);
    
    input.updateTexture();
    
    contourFinder.findContours(input, _minArea, _maxArea, _nConsidered, _hullPress, _bFindHoles, _bUseApproximation);
    track(&contourFinder);    
}

//assigns IDs to each blob in the contourFinder
void ofxBlobTracker::track(ofxContourFinder* newBlobs){	
	numEnter = 0;
	numLeave = 0;
	
    /****************************************************************************
	 //Blob tracking
     ****************************************************************************/
	//initialize ID's of all blobs
	for(int i=0; i<newBlobs->nBlobs; i++)
        newBlobs->blobs[i].id=-1;
	
	nSize = trackedBlobs.size();
    
	// STEP 1: Blob matching 
	//
	//go through all tracked blobs to compute nearest new point
	for(int i = 0; i < trackedBlobs.size(); i++) {
		/******************************************************************
		 * *****************TRACKING FUNCTION TO BE USED*******************
		 * Replace 'trackKnn(...)' with any function that will take the
		 * current track and find the corresponding track in the newBlobs
		 * 'winner' should contain the index of the found blob or '-1' if
		 * there was no corresponding blob
		 *****************************************************************/
		int winner = trackKnn(newBlobs, &(trackedBlobs[i]), 3, 0);
        
		if(winner == -1) { //track has died, mark it for deletion
			trackedBlobs[i].id = -1;
		} else { //still alive, have to update
			//if winning new blob was labeled winner by another track\
			//then compare with this track to see which is closer
			if(newBlobs->blobs[winner].id!=-1){
				//find the currently assigned blob
				int j; //j will be the index of it
				for(j=0; j<trackedBlobs.size(); j++){
					if(trackedBlobs[j].id==newBlobs->blobs[winner].id)
						break;
				}
                
				if(j==trackedBlobs.size()){//got to end without finding it
					newBlobs->blobs[winner].id = trackedBlobs[i].id;
					newBlobs->blobs[winner].age = trackedBlobs[i].age;
					newBlobs->blobs[winner].sitting = trackedBlobs[i].sitting;
					newBlobs->blobs[winner].downTime = trackedBlobs[i].downTime;
					newBlobs->blobs[winner].lastTimeTimeWasChecked = trackedBlobs[i].lastTimeTimeWasChecked;
					trackedBlobs[i] = newBlobs->blobs[winner];
				} else { //found it, compare with current blob
					double x = newBlobs->blobs[winner].centroid.x;
					double y = newBlobs->blobs[winner].centroid.y;
					double xOld = trackedBlobs[j].centroid.x;
					double yOld = trackedBlobs[j].centroid.y;
					double xNew = trackedBlobs[i].centroid.x;
					double yNew = trackedBlobs[i].centroid.y;
					double distOld = (x-xOld)*(x-xOld)+(y-yOld)*(y-yOld);
					double distNew = (x-xNew)*(x-xNew)+(y-yNew)*(y-yNew);
                    
					//if this track is closer, update the ID of the blob
					//otherwise delete this track.. it's dead
					if( distNew < distOld){ //update
						newBlobs->blobs[winner].id = trackedBlobs[i].id;
						newBlobs->blobs[winner].age = trackedBlobs[i].age;
						newBlobs->blobs[winner].sitting = trackedBlobs[i].sitting;
						newBlobs->blobs[winner].downTime = trackedBlobs[i].downTime;
						newBlobs->blobs[winner].lastTimeTimeWasChecked = trackedBlobs[i].lastTimeTimeWasChecked;
						//TODO-----------------------------------------------
						//now the old winning blob has lost the win.
						//I should also probably go through all the newBlobs
						//at the end of this loop and if there are ones without
						//any winning matches, check if they are close to this
						//one. Right now I'm not doing that to prevent a
						//recursive mess. It'll just be a new track.
                        
						//erase calibrated blob from map
						//calibratedBlobs.erase(trackedBlobs[j].id);
						//mark the blob for deletion
						trackedBlobs[j].id = -1;
						//-----------------------------------------------------
					} else {	//delete
						//erase calibrated blob from map
						//calibratedBlobs.erase(trackedBlobs[i].id);
						//mark the blob for deletion
						trackedBlobs[i].id = -1;
					}
				}
			} else {//no conflicts, so simply update
				newBlobs->blobs[winner].id = trackedBlobs[i].id;
				newBlobs->blobs[winner].age = trackedBlobs[i].age;
				newBlobs->blobs[winner].sitting = trackedBlobs[i].sitting;
				newBlobs->blobs[winner].downTime = trackedBlobs[i].downTime;
				newBlobs->blobs[winner].lastTimeTimeWasChecked = trackedBlobs[i].lastTimeTimeWasChecked;
			}
		}
	}
    
	// AlexP
	// save the current time since we will be using it a lot
	int now = ofGetElapsedTimeMillis();
    
	// STEP 2: Blob update
	//
	//--Update All Current Tracks
	//remove every track labeled as dead (ID='-1')
	//find every track that's alive and copy it's data from newBlobs
	for(int i = 0; i < trackedBlobs.size(); i++) {
		if(trackedBlobs[i].id == -1) { //dead
			numLeave++;
			//erase track
            ofNotifyEvent(blobDeleted, trackedBlobs[i]);
			trackedBlobs.erase(trackedBlobs.begin()+i, trackedBlobs.begin()+i+1);
			i--; //decrement one since we removed an element
		} else {//living, so update it's data
			for(int j = 0; j < newBlobs->nBlobs; j++) {
				if(trackedBlobs[i].id == newBlobs->blobs[j].id) {
					//update track
					ofPoint tempLastCentroid = trackedBlobs[i].centroid; // assign the new centroid to the old
                    
                    if (trackedBlobs[i].gotFingers)
                        newBlobs->blobs[j].gotFingers = true;
                    
					trackedBlobs[i] = newBlobs->blobs[j];
					trackedBlobs[i].lastCentroid = tempLastCentroid;
                    
					ofPoint tD;
					//get the Differences in position
					tD.set(trackedBlobs[i].centroid.x - trackedBlobs[i].lastCentroid.x, 
                           trackedBlobs[i].centroid.y - trackedBlobs[i].lastCentroid.y);
                    
					//calculate the acceleration
					float posDelta = sqrtf((tD.x*tD.x)+(tD.y*tD.y));
                    if (posDelta > 0.001){
                        if (trackedBlobs[i].gotFingers){
                            if (trackedBlobs[i].nFingers <= 1){
                                trackedBlobs[i].palm += trackedBlobs[i].D;
                            }
                        }
                        
                        ofNotifyEvent(blobMoved, trackedBlobs[i]);
                    }
                    
					// AlexP
					// now, filter the blob position based on MOVEMENT_FILTERING value
					// the MOVEMENT_FILTERING ranges [0,15] so we will have that many filtering steps
					// Here we have a weighted low-pass filter
					// adaptively adjust the blob position filtering strength based on blob movement
					// http://www.wolframalpha.com/input/?i=plot+1/exp(x/15)+and+1/exp(x/10)+and+1/exp(x/5)+from+0+to+100
					float a = 1.0f - 1.0f / expf(posDelta / (1.0f + (float) movementFiltering*10));
					trackedBlobs[i].centroid.x = a * trackedBlobs[i].centroid.x + (1-a) * trackedBlobs[i].lastCentroid.x;
					trackedBlobs[i].centroid.y = a * trackedBlobs[i].centroid.y + (1-a) * trackedBlobs[i].lastCentroid.y;
                    
					//get the Differences in position
					trackedBlobs[i].D.set(trackedBlobs[i].centroid.x - trackedBlobs[i].lastCentroid.x, 
                                          trackedBlobs[i].centroid.y - trackedBlobs[i].lastCentroid.y);
                    
					//if( abs((int)trackedBlobs[i].D.x) > 1 || abs((int)trackedBlobs[i].D.y) > 1) {
                    //						printf("\nUNUSUAL BLOB @ %f\n-----------------------\ntrackedBlobs[%i]\nD = (%f, %f)\nXY= (%f, %f)\nlastTimeTimeWasChecked = %f\nsitting = %f\n",
                    //							   ofGetElapsedTimeMillis(),
                    //							   i,
                    //							   trackedBlobs[i].D.x,  trackedBlobs[i].D.y,
                    //							   trackedBlobs[i].centroid.x, trackedBlobs[i].centroid.y,
                    //							   trackedBlobs[i].lastTimeTimeWasChecked,
                    //							   trackedBlobs[i].downTime,
                    //							   trackedBlobs[i].sitting
                    //						);
                    //					}
                    
					//calculate the acceleration again
					tD = trackedBlobs[i].D;
					trackedBlobs[i].maccel = sqrtf((tD.x* tD.x)+(tD.y*tD.y)) / (now - trackedBlobs[i].lastTimeTimeWasChecked);
                    
					//calculate the age
					trackedBlobs[i].age = ofGetElapsedTimef() - trackedBlobs[i].downTime;
                    
					//set sitting (held length)
                    if(trackedBlobs[i].maccel < 7) {	//1 more frame of sitting
						if(trackedBlobs[i].sitting != -1)
							trackedBlobs[i].sitting = ofGetElapsedTimef() - trackedBlobs[i].downTime; 
					} else
						trackedBlobs[i].sitting = -1;
                    
					//if blob has been 'holding/sitting' for 1 second send a held event
					if(trackedBlobs[i].sitting > 1.0f){
                        //held event only happens once so set to -1
                        trackedBlobs[i].sitting = -1;
					}
					
					// AlexP
					// The last lastTimeTimeWasChecked is updated at the end after all acceleration values are calculated
					trackedBlobs[i].lastTimeTimeWasChecked = now;
				}
			}
		}
	}
    
	// STEP 3: add tracked blobs to TouchEvents
	//--Add New Living Tracks
	//now every new blob should be either labeled with a tracked ID or\
	//have ID of -1... if the ID is -1... we need to make a new track
	for(int i = 0; i < newBlobs->nBlobs; i++){
		if(newBlobs->blobs[i].id==-1){
			//add new track
			newBlobs->blobs[i].id = IDCounter++;
			IDCounter++;
			newBlobs->blobs[i].downTime = ofGetElapsedTimef();
			//newBlobs->blobs[i].lastTimeTimeWasChecked = ofGetElapsedTimeMillis();
            
			trackedBlobs.push_back(newBlobs->blobs[i]);
            ofNotifyEvent(blobAdded, trackedBlobs.back());
			numEnter++;
            
			if (numEnter > 20){
                ofLog(OF_LOG_ERROR, "ofxTracking: Track: something wrong!\n");
			}
		}
	}
}

/*************************************************************************
 * Finds the blob in 'newBlobs' that is closest to the trackedBlob with index
 * 'ind' according to the KNN algorithm and returns the index of the winner
 * newBlobs	= list of blobs detected in the latest frame
 * track		= current tracked blob being tested
 * k			= number of nearest neighbors to consider\
 *			  1,3,or 5 are common numbers..\
 *			  must always be an odd number to avoid tying
 * thresh	= threshold for optimization
 **************************************************************************/
int ofxBlobTracker::trackKnn(ofxContourFinder *newBlobs, ofxBlob *track, int k, double thresh){    
	int winner = -1; //initially label track as '-1'=dead
	if((k%2)==0) k++; //if k is not an odd number, add 1 to it
    
	//if it exists, square the threshold to use as square distance
	if(thresh>0)
		thresh *= thresh;
    
	//list of neighbor point index and respective distances
	std::list<std::pair<int,double> > nbors;
	std::list<std::pair<int,double> >::iterator iter;
    
	//find 'k' closest neighbors of testpoint
	double x, y, xT, yT, dist;
	
    // SEARCHING FOR BLOBS
    for(int i=0; i < newBlobs->nBlobs; i++){
        x = newBlobs->blobs[i].centroid.x;
        y = newBlobs->blobs[i].centroid.y;
        
        xT = track->centroid.x;
        yT = track->centroid.y;
        dist = (x-xT)*(x-xT)+(y-yT)*(y-yT);
        
        if(dist<=thresh) { //it's good, apply label if no label yet and return
            winner = i;
            return winner;
        }
        
        /****************************************************************
         * check if this blob is closer to the point than what we've seen
         *so far and add it to the index/distance list if positive
         ****************************************************************/
        
        //search the list for the first point with a longer distance
        for(iter=nbors.begin(); iter!=nbors.end()
            && dist>=iter->second; iter++);
        
        if((iter!=nbors.end())||(nbors.size()<k)) { //it's valid, insert it
            nbors.insert(iter, 1, std::pair<int, double>(i, dist));
            //too many items in list, get rid of farthest neighbor
            if(nbors.size()>k)
                nbors.pop_back();
        }
    }
	
    
	/********************************************************************
     * we now have k nearest neighbors who cast a vote, and the majority
     * wins. we use each class average distance to the target to break any
     * possible ties.
     *********************************************************************/
    
	// a mapping from labels (IDs) to count/distance
	std::map<int, std::pair<int, double> > votes;
    
	//remember:
	//iter->first = index of newBlob
	//iter->second = distance of newBlob to current tracked blob
	for(iter=nbors.begin(); iter!=nbors.end(); iter++){
		//add up how many counts each neighbor got
		int count = ++(votes[iter->first].first);
		double dist = (votes[iter->first].second+=iter->second);
        
		/* check for a possible tie and break with distance */
		if(count>votes[winner].first || (count==votes[winner].first)
           && (dist<votes[winner].second) ) {
			winner = iter->first;
		}
	}
	return winner;
}

void ofxBlobTracker::draw( float _x, float _y, float _width, float _height ) {
	
    float scalex = 0.0f;
    float scaley = 0.0f;
	
    if( width != 0 ) { scalex = _width/width; } else { scalex = width;};//1.0f; }
    if( height != 0 ) { scaley = _height/height; } else { scaley = height;};//1.0f; }
	
    ofPushStyle();
    ofPushMatrix();
    ofTranslate( _x, _y);
    ofScale( scalex, scaley);
	
    // ---------------------------- draw the bounding rectangle
    ofNoFill();
    for( int i=0; i<(int)trackedBlobs.size(); i++ ) {
        ofSetColor(221, 0, 204, 200);
        trackedBlobs[i].drawBox();
        ofSetColor(0,255,255);
        trackedBlobs[i].drawContours();
        ofSetColor(0,153,255,100);
        trackedBlobs[i].drawCenter();
        ofSetColor(255,255);
        ofDrawBitmapString(ofToString(trackedBlobs[i].id), trackedBlobs[i].centroid );
        
        if (trackedBlobs[i].nFingers > 0){
            ofDrawBitmapString("Hand with " + ofToString(trackedBlobs[i].nFingers) + " fingers", trackedBlobs[i].centroid.x , trackedBlobs[i].centroid.y + 15);
            for (int j = 0; j < trackedBlobs[i].nFingers ; j++){
                ofFill();
                ofSetColor(255,0,0);
                ofCircle(trackedBlobs[i].fingers[j],4);
            }
        }
    }
    
	ofPopMatrix();
	ofPopStyle();
}