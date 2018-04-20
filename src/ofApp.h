#pragma once

#include "ofMain.h"
#include "ofxJSON.h"
#include "ofxPanel.h"
#include "speechGenerator.h"

#define SHUFFLE_BUTTON 114
#define SLOT_W 1920-300
#define SLOT_H 150

#include "commonS.h"

typedef enum {
    WINNING_HALT,
    WINNING_ANIMATION,
    TRANSITION_OUT,
    SHOW_MOVIE, // these can have any name you want, eg STATE_CLAP etc
    EMOJIS_IN,
    PAUSE,
    EMOJIS_OUT,
    TEXT_IN,
    TRANSITION_IN,
    RESET,
    EMPTY
} State;

class ChainEvent{
public:
    vector<double> timers;
    vector<double> durations;
    vector<State> eventName;
    bool done = true;
    int eventNumber = 0;

    void addEvent(double duration, State name){
        timers.push_back(0.0);
        durations.push_back(duration);
        eventName.push_back(name);
    }
    void update(){
		
        if(!done){	
			timers[eventNumber]+=ofGetLastFrameTime();
            if(timers[eventNumber]>=durations[eventNumber]+.3){
                timers[eventNumber] = 0.0;
                eventNumber++;
				
				if (eventNumber > timers.size() - 1) {
					eventNumber = 0;
					done = true;
					setToEmpty();
				}
            }    
        }
    }
	
    void beginEvents(){
		cout << "begin events" << endl;
        done = false;
        eventNumber = 0;
        for(int i = 0; i<timers.size();i++){
            timers[i] = 0.0;
        }
    }
    
    void setToEmpty(int i = 0) {
        if(i==0) {
			done = true;
			eventNumber = 0;
		}
        else {
            eventNumber = i;
            done = false;
        }
    }
    
    int getEvent(){
        return eventNumber;
    }
    double getTime(){
        return timers[eventNumber];
    }
    double getDuration(){
        return durations[eventNumber];
    }
    State getName(){
        if(done)return EMPTY;
        else return eventName[eventNumber];
    }
};

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		bool printDone = false;
    bool debug = true;
	bool showGui = false;
    
    ofTrueTypeFont font;
    ofTrueTypeFont fontSmall;

    ofTexture roundedRect;
    ofTexture roundCircle;
    
    vector<slot>slots;
    vector<Speech>speeches;
    vector<string> finalSpeech;
    vector<ofColor>colors;

    void printSpeech(int s);
    void randomizeSlots(bool different = false);
    void animateSlots(slot * s, int i);
    
    int printNumber;
    int writeToFile();
    string slash;
    string newLine;

    ofxPanel gui;
    ofParameter<float>animationTime;
    ofParameter<float>easiness;
    ofParameter<float>alignHeight;
    ofParameter<float>alignSpace;
    ofParameter<int>jump;
    ofParameter<float>alphaTransitionTime;
    
    ChainEvent chainEvent;
    float transitionAlpha;
    
    int jumpDistance;
    void drawSlots();
    bool        bSendSerialMessage;            // a flag for sending serial
    char        bytesRead[3];                // data from serial, we will be trying to read 3
    char        bytesReadString[4];            // a string needs a null terminator, so we need 3 + 1 bytes
    int            nBytesRead;                    // how much did we read?
    int            nTimesRead;                    // how many times did we read?
    float        readTime;                    // when did we last read?
	bool speechCreated = false;
    void readArduino();
    void echoArduino();
    bool echo = false;
    double echoTimer = 0.0;
    int deviceCount = 0;
    
    int input = 0;
    int p_input = 0; 
    ofSerial    serial;
SpeechGenerator sp;
    
    ofVideoPlayer video;

    float ease(float time, float begin, float end, float duration) {
        end -=begin;
        
		if (time > duration) return end+begin;
		time /= duration;
		return end*time*time + begin;
        
    }
    
    void createSpeech(int s);
    string latestFile;
    ofTexture getResizedTexture(string path, int w, int h, ofImage mask, bool m = false, bool bw = true){
        ofDisableArbTex();
        
        ofImage img;

        int nw,nh;
        
        img.load(path);

        if(img.getWidth()>img.getHeight()){
            nw = h;
            nh = h*(img.getWidth()/img.getHeight());
        }
        else {
            nw = h*(img.getHeight()/img.getWidth());
            nh = h;
        }
        // img.resize(nw,nh);
        
        ofTexture t;
        t.allocate(nw,nh,GL_RGBA);
        
        t = img.getTexture();
        
        if(m){
            mask.resize(h,h);
           // t.setAlphaMask(mask.getTexture());
        }
        if(!bw)return t;
        
        ofEnableArbTex();
        
        ofFbo f;
        
        ofSetColor(255);
        ofFill();
        f.allocate(h,h, GL_RGBA);
        ofShader s;
        s.load("shader");
        f.begin();
        ofClear(0);
        
        s.begin();
        s.setUniformTexture("tex", t, 0);
        s.setUniformTexture("mask", mask.getTexture(), 1);
        s.setUniform2f("res",h,h);
		s.setUniform2f("res2", nh, nw);
        ofDrawRectangle(0,0,h,h);
        s.end();
        
        f.end();
        return f.getTexture();
        
    }
    
    ofTexture getStringAsTexture(ofColor c, int w, int h, vector<string> l, ofTrueTypeFont f, int maxLines = 20){
        ofFbo fbo;
        fbo.allocate(w, h, GL_RGBA);
        
        fbo.begin();
        ofClear(0);
        ofSetColor(20);
       
        
        ofSetColor(c);

        int y = h/2 - (MIN(l.size(),maxLines)*f.getLineHeight())/2;
        
        drawCollumn(l, h/2, y*1.4, f, maxLines);
        fbo.end();
        
        ofBufferObject b;
        b.allocate(w * h * 4, GL_STATIC_DRAW);
        fbo.getTexture().copyTo(b);
        
        ofTexture t;
        t.allocate(w,h,GL_RGBA);
        t.loadData(b, GL_RGBA, GL_UNSIGNED_BYTE);
        
        return t;
    }

    vector<string> transformToCollumn(string str, int w, ofTrueTypeFont f){

        vector<string> result;
        string appending="";
        for(int i = 0; i<str.length();i++){
            string c = str.substr(i, 1);
            if (c == "\303") {
                c = str.substr(i, 2);
                i++;
            }
            appending.append(c);
            
            if(f.stringWidth(appending) > w){
                // find last space
                string thisLine;
                string toNextLine;
                for(int s = 0; s<appending.length(); s++){
                    if(isSpace(appending[s])){
                        thisLine = appending.substr(0,s+1);
                        toNextLine = appending.substr(s+1,appending.length()-s);
                    }
                }
                result.push_back(thisLine);
                appending = toNextLine;
            }
        }
        result.push_back(appending);
        return result;
    }
    
    void drawCollumn(vector<string> s, int x, int y, ofTrueTypeFont f, int maxLine = 20 ){
        
        if(maxLine<s.size()){
            s[maxLine-1].pop_back();
            s[maxLine-1].append("...");
        }
        for(int i = 0; i < MIN(s.size(),maxLine); i++){
            f.drawString(s[i], x, y+i*f.getLineHeight() );
        }
    }
    bool isSpace(unsigned int c){
        //http://www.fileformat.info/info/unicode/category/Zs/list.htm
        return     c == 0x20 || c == 0xA0 || c == 0x1680 || c == 0x2000 || c == 0x2001
        || c == 0x2002
        || c == 0x2003 || c == 0x2004 || c == 0x2005 || c == 0x2006 || c == 0x2007 || c == 0x2028
        || c == 0x2029
        || c == 0x2008 || c == 0x2009 || c == 0x200A || c == 0x202F || c == 0x205F || c == 0x3000
        //    //https://en.wikipedia.org/wiki/Whitespace_character
        //    || c == 0x0009 //tab
        //    || c == 0x000A //line feed
        //    || c == 0x000B //line tab
        //    || c == 0x000C //form feed
        //    || c == 0x000F //carriage return
        //    || c == 0x0085 //next line
        ;
    }
    
    int getColor(slot * s){
        int col = 5;
        int thisSpeech = s->new_quote->speechID;
        Quote thisQ = *s->new_quote;
        //ned new color for this slot
        bool exists = false;
     
        vector<bool>occupiedColors;
        vector<int>occupiedColorInts;
        for(int i = 0; i<slots.size(); i++) occupiedColors.push_back(false);

        //does the speech of the slot new speech exist?
        for(int i = 0; i<slots.size(); i++) {
            if(slots[i].colorIsSet)occupiedColors[slots[i].currentColor] = true;
        
            if(thisSpeech == slots[i].new_quote->speechID && s->slotId!=i && slots[i].colorIsSet) {
                exists = true;
                col = slots[i].currentColor;
            }
        }
        if(!exists) {
            for(int i = 0; i<occupiedColors.size(); i++)
                if(!occupiedColors[i])
                    occupiedColorInts.push_back(i);
            int randomColor = ofRandom(occupiedColorInts.size());
            col = occupiedColorInts[randomColor];
            
        }  
        return col;
    }


};
