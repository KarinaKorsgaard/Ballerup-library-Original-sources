//
//  boadingPassGenerator.h
//  boardingPassGenerator
//
//  Created by Karina Korsgaard Jensen on 21/03/2018.
//

#ifndef speechGenerator_h
#define speechGenerator_h


#include "ofApp.h"
#include "commonS.h"

class SpeechGenerator{
public:
    ofTrueTypeFont tidSted_f,mat_f;
    ofTrueTypeFont matb_f, number_f;
    
    ofFile printNumberFile;
    int printNumber;
    
    void setup(){
       // ofxSVG svg;
       // svg.load("bp_generator/boardingtemplate.svg");
        tidSted_f.load("fonts/bourton/Bourton-Base.ttf",25);
        mat_f.load("fonts/bourton/Bourton-Base-Drop.ttf",15);
        matb_f.load("fonts/bourton/Bourton-Base-Drop.ttf",14);
        number_f.load("fonts/bourton/Bourton-Base-Drop.ttf",20);
        
       // rects = getPolyline(svg);
	   /*
        shader.load("bp_generator/sharpen");
        
        layout["dest"] = rects[1];
        layout["time"] = rects[2];
        
        layout["mat1"] = rects[3];
        layout["mat2"] = rects[4];
        layout["mat3"] = rects[5];
        
        
        layout["matb1"] = rects[6];
        layout["matb2"] = rects[7];
        layout["matb3"] = rects[8];
        */
        string newLine = "\r\n";
#ifdef __APPLE__
        newLine = "\n";
#endif
        
        if(ofFile::doesFileExist(ofToDataPath("printNumberFile.txt"))) {
            cout<< "print number exists exists"<< endl;
            auto lines = ofSplitString(ofBufferFromFile("printNumberFile.txt").getText(), newLine);
            printNumber = ofToInt(ofSplitString(lines.back()," ")[0]);
            cout<<"lines: "<< printNumber<<" num lines "<< lines.size()<<endl;
        }else {
            ofFile newFile(ofToDataPath("printNumberFile.txt"),ofFile::WriteOnly); //file doesn't exist yet
            newFile.create(); // now file exists
            newFile.open(ofToDataPath("printNumberFile.txt"),ofFile::WriteOnly);
            newFile << "0 beginning";
            newFile.close();
        }
    }
    
    string generate(Speech d, int current){
        ofEnableAntiAliasing();
        
        ofImage bg;
        bg.load("other/background-01.png");
        int w = bg.getWidth();
        int h = bg.getHeight();
        
        float scale = 1.;
        
        ofFbo fbo; // for composing
        ofDisableArbTex();
        fbo.allocate(w,h, GL_RGBA);
        ofEnableArbTex();
        
        ofPixels pix;
        pix.allocate(w,h,GL_RGBA);
        
        ofFbo fbores; // for final output
        fbores.allocate(w,h,GL_RGBA);
        
       // shader.load("other/sharpen");
        
 
 
        ofPushMatrix();
        fbo.begin();
        ofClear(255);
        ofBackground(255);
        
        bg.draw(0,0,w,h);
        
        
        ofTranslate(0, 18);
        int numL = 1;
		d.face.draw(25, 10);
		ofTranslate(0,d.face.getHeight());
        
        ofSetColor(0);
        string strupper = ofToUpper(d.name);
        tidSted_f.drawString(d.speaker, 25*scale, 50*scale);
        drawCollumn(transformToCollumn(strupper, fbo.getWidth()-50*scale, tidSted_f), 25*scale, 90*scale, tidSted_f, 50);
        

		string theSpeech = "";
        for(int u = 0; u<d.quotes.size();u++){
			theSpeech.append(d.quotes[u].str);
        }
		drawCollumn(transformToCollumn(theSpeech, fbo.getWidth()-50*scale, matb_f), 25 * scale, 210 * scale, matb_f, 50);

        
        ofSetColor(255);
        string number = ofToString(writeToFile(),6,'0');
        
        number_f.drawString(number,fbo.getWidth()-115,27);
        
        ofPopMatrix();
        fbo.end();
        
        fbores.begin();
        ofClear(255);
        shader.begin();
        
        fbo.draw(0,0);
        shader.end();
        fbores.end();
        
        fbores.readToPixels(pix);
        ofSaveImage(pix, "generated/"+ofToString(current)+".png", OF_IMAGE_QUALITY_BEST);
        return "latest.png";
    }
        /*
    
    // helper functions
    vector<ofRectangle> getPolyline(ofxSVG svg){
        vector<ofRectangle>polys;
        for(int j = svg.getNumPath()-1; j>-1;j--){
            ofPath p = svg.getPathAt(j);
            p.setPolyWindingMode(OF_POLY_WINDING_ODD);
            vector<ofPolyline>& lines = const_cast<vector<ofPolyline>&>(p.getOutline());
            polys.push_back(lines[0].getBoundingBox());
        }
        return polys;
    }
    */
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
    
    void drawCollumn(vector<string> s, int x, int y, ofTrueTypeFont f,int maxLine = 10 ){
        
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
    
    void drawLatest(){
        if(ofFile::doesFileExist(ofToDataPath(latest))){
            ofImage img;
            img.load(latest);
            img.draw(0,0);
        }
    }
    
    int writeToFile(){
        
        string newLine = "\r\n";
#ifdef __APPLE__
        newLine = "\n";
#endif
        
        printNumber++;
        ofFile newFile;
        newFile.open(ofToDataPath("printNumberFile.txt"),ofFile::Append);
        newFile <<newLine + ofToString(printNumber)+" "+ofGetTimestampString("%Y-%m-%d-%H");
        newFile.close();
        return printNumber;
    }

    
private:
    map<string,ofRectangle> layout;
    ofShader shader;
    vector<ofRectangle>rects;
    string latest;

};

#endif /* boadingPassGenerator_h */
