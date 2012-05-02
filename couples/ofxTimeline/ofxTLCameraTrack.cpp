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
		float screenY = bounds.y;
        ofPoint screenPoint = ofPoint(screenX,screenY);

        if(i == mostRecentlySelected){
            if(easeInSelected){
                ofSetColor(timeline->getColors().highlightColor);
                draweEase(track.getSamples()[i].easeIn,  screenPoint, true);
	            ofSetColor(timeline->getColors().keyColor);
                draweEase(track.getSamples()[i].easeOut, screenPoint, false);
            }
            else {
                ofSetColor(timeline->getColors().keyColor);
                draweEase(track.getSamples()[i].easeIn,  screenPoint, true);
	            ofSetColor(timeline->getColors().highlightColor);
                draweEase(track.getSamples()[i].easeOut, screenPoint, false);
            }
        }
        else{
            ofSetColor(timeline->getColors().keyColor);
            draweEase(track.getSamples()[i].easeIn,  screenPoint, true);
            draweEase(track.getSamples()[i].easeOut, screenPoint, false);
        }
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

void ofxTLCameraTrack::draweEase(CameraTrackEase ease, ofPoint screenPoint, bool easeIn){
    switch (ease) {
        case CAMERA_EASE_LINEAR:
            if(easeIn){
                ofTriangle(screenPoint.x-bounds.height, screenPoint.y,
                           screenPoint.x, screenPoint.y, 
                           screenPoint.x, screenPoint.y+bounds.height);
            }
            else{
                ofTriangle(screenPoint.x, screenPoint.y,
                           screenPoint.x, screenPoint.y+bounds.height, 
                           screenPoint.x+bounds.height, screenPoint.y+bounds.height);                    
            }
            break;
        case CAMERA_EASE_SMOOTH:
            if(easeIn){
                ofBezier(screenPoint.x-bounds.height, screenPoint.y, 
                         screenPoint.x-bounds.height/2, screenPoint.y, 
                         screenPoint.x, screenPoint.y+bounds.height/2, 
                         screenPoint.x, screenPoint.y+bounds.height);
                ofLine(screenPoint.x-bounds.height, screenPoint.y,
                       screenPoint.x, screenPoint.y);
                ofLine(screenPoint.x, screenPoint.y,
                       screenPoint.x, screenPoint.y+bounds.height);
            }
            else {
                ofBezier(screenPoint.x, screenPoint.y, 
                         screenPoint.x, screenPoint.y+bounds.height/2, 
                         screenPoint.x+bounds.height/2, screenPoint.y+bounds.height, 
                         screenPoint.x+bounds.height, screenPoint.y+bounds.height);  
                ofLine(screenPoint.x, screenPoint.y, 
                       screenPoint.x, screenPoint.y+bounds.height);
                ofLine(screenPoint.x, screenPoint.y+bounds.height, 
                       screenPoint.x+bounds.height, screenPoint.y+bounds.height);
            }
            break;
        case CAMERA_EASE_CUT:
            if(easeIn){
	            ofRect(screenPoint.x-bounds.height/2, screenPoint.y, 
                       bounds.height/2, bounds.height/2);
            }
            else{
                ofRect(screenPoint.x, screenPoint.y+bounds.height/2, 
                       bounds.height/2, bounds.height/2);
            }
            break;
        default:
            ofLogError("ofxTLCameraTrack::draweEase -- invalid ease");
            break;
    }
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
        
		mostRecentlySelected = selectedTrack;
        easeInSelected = args.x < screenXForIndex(track.getSamples()[selectedTrack].frame);
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
    
    //TODO cycling through eases may be better as a function of camera track...
    if(args.key == OF_KEY_UP){
        if(mostRecentlySelected != -1){
            CameraSample& samp = track.getSamples()[ mostRecentlySelected ];
            if(easeInSelected){
	            samp.easeIn = ofxCameraTrack::getPreviousEase(samp.easeIn);
            }
            else {
                samp.easeOut = ofxCameraTrack::getPreviousEase(samp.easeOut);
            }
            save();
        }
    }
    else if(args.key == OF_KEY_DOWN){
        if(mostRecentlySelected != -1){
            CameraSample& samp = track.getSamples()[ mostRecentlySelected ];
            if(easeInSelected){
	            samp.easeIn = ofxCameraTrack::getNextEase(samp.easeIn);
            }
            else {
                samp.easeOut = ofxCameraTrack::getNextEase(samp.easeOut);
            }
            save();
        }
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
    mostRecentlySelected = -1;
}

int ofxTLCameraTrack::trackIndexForScreenX(float screenX){

	for(int i = 0; i < track.getSamples().size(); i++){
		float camScreenX = screenXForIndex(track.getSamples()[i].frame);
//		cout << "cam point " << i << " screen index is " << camScreenX << " screen index is " << screenX << endl;
		if(fabs(camScreenX - screenX) < bounds.height/2){
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
