/*
 *  ofxCameraRecorder.h
 *  RGBDVisualize
 *
 *  Created by Jim on 1/18/12.
 *  Copyright 2012 FlightPhase. All rights reserved.
 *
 */

#pragma once
#include "ofMain.h"

typedef struct{
	int frame;
	ofVec3f position;
	ofQuaternion orientation;
} CameraSample;

class ofxCameraTrack {
  public:
	ofxCameraTrack();
	
	vector<CameraSample> & getSamples();
	void sample(int frame);
    void addSample();
	void writeToFile(string fileName);
	void loadFromFile(string fileName);
 	void reset();
	void moveCameraToFrame(int frame);
	int getFirstFrame();
	int getLastFrame();
	string getXMLRep();
	void loadFromXMLRep(string rep);
	
	void updateSortOrder();
	
	ofCamera* camera;
	
	//TODO: //draw in 3d
  protected:
	vector<CameraSample> samples;
	
	CameraSample interpolateBetween(CameraSample sample1, CameraSample sample2, int frame); 
};