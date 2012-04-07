/*
 *  ofxTLCameraTrack.cpp
 *  RGBDVisualize
 *
 *  Created by James George on 2/7/12.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#include "ofxTLCameraTrack.h"
#include "ofxTimeline.h"
#include "ofxTLUtils.h"

ofxTLCameraTrack::ofxTLCameraTrack(){
	lockCameraToTrack = false;
}

ofxTLCameraTrack::~ofxTLCameraTrack(){

}

void ofxTLCameraTrack::setup(){
	enable();
	load();
	
	ofAddListener(ofEvents().update, this, &ofxTLCameraTrack::update);
}

void ofxTLCameraTrack::draw(){
	
	ofPushStyle();
	ofSetColor(timeline->getColors().keyColor);
	ofNoFill();

	for(int i = 0; i < track.getSamples().size(); i++){
		float screenX = screenXForIndex(track.getSamples()[i].frame);
		float screenY = bounds.y+bounds.height/2;

		ofCircle(screenX, screenY, 8);
	}

	ofFill();
	ofSetColor(timeline->getColors().highlightColor);
	for(int i = 0; i < selectedTrackPoints.size(); i++){
		float screenX = screenXForIndex(track.getSamples()[ selectedTrackPoints[i] ].frame);
		float screenY = bounds.y+bounds.height/2;
		ofCircle(screenX, screenY, 4);
	}

	ofPopStyle();
}

void ofxTLCameraTrack::update(ofEventArgs& args){
	if(lockCameraToTrack){
		//TODO: timebased camera tracking
		track.moveCameraToFrame(timeline->getCurrentFrame());
	}
}

void ofxTLCameraTrack::setTimelineInOutToTrack(){
	//TODO: timebased camera tracking
	if(track.getSamples().size() > 0){
		timeline->setInPointAtFrame(track.getFirstFrame());
		timeline->setOutPointAtFrame(track.getLastFrame());
	}
	else{
		timeline->setInOutRange(ofRange(0,1.0));
	}	
}

ofxCameraTrack& ofxTLCameraTrack::getCameraTrack(){
	return track;
}

void ofxTLCameraTrack::setCamera(ofCamera& cam){
	track.camera = &cam;
}

void ofxTLCameraTrack::sample(){
	unselectAll();
	track.sample(timeline->getCurrentFrame());	
	save();
}

void ofxTLCameraTrack::mousePressed(ofMouseEventArgs& args){
	if(bounds.inside(args.x, args.y)){
		int selectedTrack = trackIndexForScreenX(args.x);
		if(selectedTrack == -1){
			timeline->unselectAll();
			return;
		}

		bool alreadySelected = isPointSelected(selectedTrack);
		if(!alreadySelected){
			if(!ofGetModifierKeyShift()){
				timeline->unselectAll();
			}
			selectedTrackPoints.push_back( selectedTrack );
		}
		
		canDrag = !ofGetModifierKeyShift();
		updateDragOffsets(args.x);
	}
}

void ofxTLCameraTrack::mouseMoved(ofMouseEventArgs& args){
}

void ofxTLCameraTrack::mouseDragged(ofMouseEventArgs& args, bool snapped){
	if(canDrag){
		for(int i = 0; i < selectedTrackPoints.size(); i++){
			track.getSamples()[ selectedTrackPoints[i] ].frame = indexForScreenX(args.x - dragOffsets[i]);
		}
	}
	else if(selectedTrackPoints.size() == 0){
		if(bounds.inside(args.x, args.y)){

			float index = indexForScreenX(args.x);
			track.moveCameraToFrame(index);
			if(timeline->getMovePlayheadOnDrag()){
				timeline->setCurrentFrame(index);
			}
		}
	}
}

void ofxTLCameraTrack::mouseReleased(ofMouseEventArgs& args){
	canDrag = false;
	track.updateSortOrder();
	save();
}

void ofxTLCameraTrack::keyPressed(ofKeyEventArgs& args){
		
	if(args.key == OF_KEY_DEL|| args.key == OF_KEY_BACKSPACE){
		for(int i = selectedTrackPoints.size()-1; i >= 0; i--){
			track.getSamples().erase( track.getSamples().begin() + selectedTrackPoints[i]);
		}
		selectedTrackPoints.clear();
		track.updateSortOrder();
		save();
	}
}

void ofxTLCameraTrack::nudgeBy(ofVec2f nudgePercent){

}

void ofxTLCameraTrack::save(){
//	cout << " saving camera track to " << xmlFileName << endl;
	track.writeToFile(xmlFileName);	
}

void ofxTLCameraTrack::load(){
//	cout << " loading camera track from " << xmlFileName << endl;
	track.loadFromFile(xmlFileName);
}

void ofxTLCameraTrack::clear(){
}

void ofxTLCameraTrack::selectAll(){
	selectedTrackPoints.clear();
	for(int i = 0; i < track.getSamples().size(); i++){
		selectedTrackPoints.push_back(i);
	}
}

void ofxTLCameraTrack::unselectAll(){
	selectedTrackPoints.clear();	
}

int ofxTLCameraTrack::trackIndexForScreenX(float screenX){

	for(int i = 0; i < track.getSamples().size(); i++){
		float camScreenX = screenXForIndex(track.getSamples()[i].frame);
//		cout << "cam point " << i << " screen index is " << camScreenX << " screen index is " << screenX << endl;
		if(fabs(camScreenX - screenX) < 8){
//			cout << " SELECTED TRACK INDEX " << i << endl;
			return i;
		}
	}
	return -1;
}

bool ofxTLCameraTrack::isPointSelected(int index){	
	for(int i = 0; i < selectedTrackPoints.size(); i++){
		if(selectedTrackPoints[i] == index){
			return true;
		}
	}
	return false;
}

void ofxTLCameraTrack::updateDragOffsets(float screenX){
	dragOffsets.clear();
	for(int i = 0; i < selectedTrackPoints.size(); i++){
		dragOffsets.push_back( screenX - screenXForIndex(track.getSamples()[ selectedTrackPoints[i] ].frame) );
	}
}
