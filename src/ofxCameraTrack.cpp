/*
 *  ofxCameraTrack.cpp
 *  RGBDVisualize
 *
 *  Created by Jim on 1/18/12.
 *  Copyright 2012 FlightPhase. All rights reserved.
 *
 */

#include "ofxCameraTrack.h"
#include "ofxXmlSettings.h"
#include "ofxTween.h"

bool trackpointsort(CameraSample a, CameraSample b){
	return a.frame < b.frame;
}

ofxCameraTrack::ofxCameraTrack(){
	camera = NULL;
}

void ofxCameraTrack::reset(){
	samples.clear();

}

void ofxCameraTrack::sample(int frame){
	if(camera == NULL){
		ofLogError("ofxCameraTrack -- can't sample a null camera");
	}

    for(int i = 0; i < samples.size(); i++){
        if(samples[i].frame == frame){
            samples[i].position = camera->getPosition();
            samples[i].orientation = camera->getOrientationQuat();
            return;
        }
    }
    
	CameraSample c;
	c.frame = frame;
	c.position = camera->getPosition();
	c.orientation = camera->getOrientationQuat();
    c.easeIn  = CAMERA_EASE_LINEAR;
    c.easeOut = CAMERA_EASE_LINEAR;
	samples.push_back(c);
	
	sort(samples.begin(), samples.end(), trackpointsort);
	
	cout << "Sampled. First frame is  " << samples[0].frame << " " << samples[samples.size()-1].frame << endl;
}

string ofxCameraTrack::getXMLRep(){
	ofxXmlSettings settings;
	settings.addTag("camera");
	settings.pushTag("camera");
	for(int i = 0; i < samples.size(); i++){
		settings.addTag("sample");
		settings.pushTag("sample", i);
		
		settings.addValue("frame", samples[i].frame);
		
		settings.addValue("px", samples[i].position.x);
		settings.addValue("py", samples[i].position.y);
		settings.addValue("pz", samples[i].position.z);
		
		settings.addValue("ox", samples[i].orientation._v.x);
		settings.addValue("oy", samples[i].orientation._v.y);
		settings.addValue("oz", samples[i].orientation._v.z);
		settings.addValue("ow", samples[i].orientation._v.w);
		
        settings.addValue("easein",  (int)samples[i].easeIn);
		settings.addValue("easeout", (int)samples[i].easeOut);

		settings.popTag();
	}
	settings.popTag();
	string rep;
	settings.copyXmlToString(rep);
	return rep;
}

void ofxCameraTrack::loadFromXMLRep(string rep){
	samples.clear();
	ofxXmlSettings settings;
	settings.loadFromBuffer(rep);
	settings.pushTag("camera");
	int numSamples = settings.getNumTags("sample");
	for(int i = 0; i < numSamples; i++){
		CameraSample c;
		settings.pushTag("sample", i);
		c.frame = settings.getValue("frame", 0);
		c.position = ofVec3f(settings.getValue("px", 0.),
							 settings.getValue("py", 0.),
							 settings.getValue("pz", 0.));		
		c.orientation.set(settings.getValue("ox", 0.),
									 settings.getValue("oy", 0.),
									 settings.getValue("oz", 0.),
									 settings.getValue("ow", 1.));
        c.easeIn  = (CameraTrackEase)settings.getValue("easein", (int)CAMERA_EASE_LINEAR);
        c.easeOut = (CameraTrackEase)settings.getValue("easeout", (int)CAMERA_EASE_LINEAR);
		settings.popTag();
		samples.push_back(c);
	}
	settings.popTag();	
}


void ofxCameraTrack::writeToFile(string fileName){
	ofxXmlSettings settings;
	string xmlRep = getXMLRep();
	settings.loadFromBuffer(xmlRep);
	settings.saveFile(fileName);
}

void ofxCameraTrack::loadFromFile(string fileName){
	ofxXmlSettings settings;
	if(settings.loadFile(fileName)){
		string rep;
		settings.copyXmlToString(rep);
		loadFromXMLRep(rep);
	}
	else{
		samples.clear();
		ofLogError("ofxCameraTrack -- couldn't load camera file " + fileName);
	}
}

void ofxCameraTrack::updateSortOrder(){
	sort(samples.begin(), samples.end(), trackpointsort);
}

int ofxCameraTrack::getFirstFrame(){
	return samples.size() == 0 ? 0 : samples[0].frame;
}

int ofxCameraTrack::getLastFrame(){
	return samples.size() == 0 ? 0 : samples[samples.size()-1].frame;
}

void ofxCameraTrack::moveCameraToFrame(int frame){
	if(samples.size() == 0){
		ofLogError("ofxCameraTrack -- no samples to move camera to!");
		return;
	}

	if(camera == NULL){
		ofLogError("ofxCameraTrack -- can't modify a null camera!");
		return;
	}
	
	//cout << "Sampling at frame " << frame << " with frames ranging between " << samples[0].frame << " - " << samples[samples.size()-1].frame << endl;
	
	CameraSample interp = samples[samples.size()-1];
	for(int i = 0; i < samples.size(); i++){
		if(samples[i].frame == frame){
			interp = samples[i];
		}
		
		if(samples[i].frame > frame){
			if(i != 0){
				interp = interpolateBetween(samples[i-1], samples[i], frame);
			}
			else {
				interp = samples[i];
			}
			break;
		}
	}
	
	camera->setPosition(interp.position);
	camera->setOrientation(interp.orientation);
	
//	cout << "set position to " << camera->getPosition() << endl;
}
				
CameraSample ofxCameraTrack::interpolateBetween(CameraSample sample1, CameraSample sample2, int frame){
	CameraSample interp;
    float alpha;
    //CUT
    if(sample1.easeOut == CAMERA_EASE_CUT || sample2.easeIn == CAMERA_EASE_CUT ){
        alpha = 0;
    }
    //LINEAR
    else if(sample1.easeOut == CAMERA_EASE_LINEAR && sample2.easeIn == CAMERA_EASE_LINEAR){
        alpha = ofMap(frame, sample1.frame, sample2.frame, 0, 1.0, false);
    }
    //EASE IN
    else if(sample1.easeOut == CAMERA_EASE_SMOOTH && sample2.easeIn == CAMERA_EASE_LINEAR){
        ofxEasingQuad ease;
        alpha = ofxTween::map(frame, sample1.frame, sample2.frame, 0, 1.0, false, ease, ofxTween::easeIn);                
    }
    //EASE OUT
    else if(sample1.easeOut == CAMERA_EASE_LINEAR && sample2.easeIn == CAMERA_EASE_SMOOTH){
        ofxEasingQuad ease;
        alpha = ofxTween::map(frame, sample1.frame, sample2.frame, 0, 1.0, false, ease, ofxTween::easeOut);    
    }
    //EASE IN OUT
    else if(sample1.easeOut == CAMERA_EASE_SMOOTH && sample2.easeIn == CAMERA_EASE_SMOOTH){
        ofxEasingQuad ease;
        alpha = ofxTween::map(frame, sample1.frame, sample2.frame, 0, 1.0, false, ease, ofxTween::easeInOut);            
    }
    
	//float alpha = ofMap(frame, sample1.frame, sample2.frame, 0, 1.0, false);
    
	interp.frame = frame;
	interp.position = sample1.position.getInterpolated(sample2.position, alpha);
	interp.orientation.slerp(alpha, sample1.orientation, sample2.orientation);
	
	//cout << "interpolating between " << sample1.position << " and " << sample2.position << " with alpha " << alpha << endl; 
	return interp;
}

vector<CameraSample>& ofxCameraTrack::getSamples(){
	return samples;
}

CameraTrackEase ofxCameraTrack::getNextEase(CameraTrackEase ease){
    switch(ease){
        case CAMERA_EASE_CUT:
            return CAMERA_EASE_LINEAR;
        case CAMERA_EASE_LINEAR:
            return CAMERA_EASE_SMOOTH;
        case CAMERA_EASE_SMOOTH:
            return CAMERA_EASE_CUT;
    }
}

CameraTrackEase ofxCameraTrack::getPreviousEase(CameraTrackEase ease){
    switch(ease){
        case CAMERA_EASE_CUT:
            return CAMERA_EASE_SMOOTH;
        case CAMERA_EASE_LINEAR:
            return CAMERA_EASE_CUT;
        case CAMERA_EASE_SMOOTH:
            return CAMERA_EASE_LINEAR;
    }
}
