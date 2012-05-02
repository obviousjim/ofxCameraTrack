/*
 *  ofxTLCameraTrack.h
 *  RGBDVisualize
 *
 *  Created by James George on 2/7/12.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "ofMain.h"
#include "ofxCameraTrack.h"
#include "ofxTimeline.h"

class ofxTLCameraTrack : public ofxTLElement {
  public:
	ofxTLCameraTrack();
	~ofxTLCameraTrack();
	
	virtual void setup();
	virtual void draw();
	
	virtual void mousePressed(ofMouseEventArgs& args);
	virtual void mouseMoved(ofMouseEventArgs& args);
	virtual void mouseDragged(ofMouseEventArgs& args, bool snapped);
	virtual void mouseReleased(ofMouseEventArgs& args);
	
	virtual void keyPressed(ofKeyEventArgs& args);
	
	virtual void nudgeBy(ofVec2f nudgePercent);
	
	virtual void save();
	virtual void load();
	
	virtual void clear();
	
	virtual void selectAll();
	virtual void unselectAll();
	
	void sample();
	
	void setTimelineInOutToTrack();
	bool lockCameraToTrack;
	void setCamera(ofCamera& cam);
	ofxCameraTrack& getCameraTrack();
	
	
  protected:
	vector<int> selectedTrackPoints;
	vector<float> dragOffsets;
	
	int hoverIndex;
	bool canDrag;
	bool isPointSelected(int index);
	
	int trackIndexForScreenX(float screenX);
	void updateDragOffsets(float screenX);
    
	int mostRecentlySelected; //for ease selection
    bool easeInSelected;
	
	ofxCameraTrack track;	
	void update(ofEventArgs& args);
    
    //convenient drawing functions
    void draweEase(CameraTrackEase ease, ofPoint screenPoint, bool easeIn);

};
