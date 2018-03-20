#pragma once

#include "ofMain.h"
#include "ofxJSON.h"
#include "ofxPanel.h"

struct Quote{
    ofTexture tex;
    string str;
    ofImage img;
    int quoteID;
    
    vector<ofImage>face;

};

struct slot{
    Quote * quote;
    Quote * new_quote;
    bool isAnimated;
    double ypos = 0.0;
    double xpos = 0.0;
    double animateTime = 0.0;
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
		
    ofTrueTypeFont font;
    ofTrueTypeFont fontSmall;
    vector<Quote>quotes;
    vector<slot>slots;
    vector<ofRectangle>faceCoords;
    
    void doOutput();
    void drawFace();

    
    vector<string> finalSpeech;
    int collumWidth = 400;

    int charHeight;
    void animateSlots(slot * s);
    
    ofxPanel gui;
    ofParameter<float>animationTime;
    ofParameter<float>alphaTransitionTime;
    ofParameter<float>scaleFace;
    ofParameter<ofVec2f>facePos;
    ofVec2f lowerMouth = ofVec2f(0,0);
    
    bool isDone = false;
    float transitionAlpha;
    
    bool        bSendSerialMessage;            // a flag for sending serial
    char        bytesRead[3];                // data from serial, we will be trying to read 3
    char        bytesReadString[4];            // a string needs a null terminator, so we need 3 + 1 bytes
    int            nBytesRead;                    // how much did we read?
    int            nTimesRead;                    // how many times did we read?
    float        readTime;                    // when did we last read?
    bool isInitialized = false;
    void initialiseArdiono();
    void readArduino();
    int input = -1; 
    ofSerial    serial;
    

    
    Quote getRandomQuote(){
        return quotes[int(ofRandom(quotes.size()))];
    }
    
    void roundedRect(float x, float y, float w, float h, float r) {
        ofBeginShape();
        ofVertex(x+r, y);
        ofVertex(x+w-r, y);
        quadraticBezierVertex(x+w, y, x+w, y+r, x+w-r, y);
        ofVertex(x+w, y+h-r);
        quadraticBezierVertex(x+w, y+h, x+w-r, y+h, x+w, y+h-r);
        ofVertex(x+r, y+h);
        quadraticBezierVertex(x, y+h, x, y+h-r, x+r, y+h);
        ofVertex(x, y+r);
        quadraticBezierVertex(x, y, x+r, y, x, y+r);
        ofEndShape();
    }
    
    void quadraticBezierVertex(float cpx, float cpy, float x, float y, float prevX, float prevY) {
        float cp1x = prevX + 2.0/3.0*(cpx - prevX);
        float cp1y = prevY + 2.0/3.0*(cpy - prevY);
        float cp2x = cp1x + (x - prevX)/3.0;
        float cp2y = cp1y + (y - prevY)/3.0;
        
        // finally call cubic Bezier curve function
        ofBezierVertex(cp1x, cp1y, cp2x, cp2y, x, y);
    };
    
    inline static float easeInOut_s(float t, float b , float c, float d, float s) {
        s*=(1.525f);
        if ((t/=d/2) < 1){
            return c/2*(t*t*((s+1)*t - s)) + b;
        }
        float postFix = t-=2;
        return c/2*((postFix)*t*((s+1)*t + s) + 2) + b;
    }
};
