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
    ofTrueTypeFont general_f,des_f;
    ofTrueTypeFont speech_f, number_f,headline_f;
    
    ofFile printNumberFile;
    int printNumber;
    
    void setup(){

        speech_f.load("fonts/bourton/Bourton-Base.ttf",26*2);
        headline_f.load("fonts/bourton/Bourton-Base.ttf",34*2);
        des_f.load("fonts/bourton/Bourton-Base-Drop.ttf",23*2);
        general_f.load("fonts/bourton/Bourton-Base.ttf",20*2);
        number_f.load("fonts/bourton/Bourton-Base.ttf",20*2);
        
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
        bg.load("other/top.png");
        int w = bg.getWidth();
        int h = bg.getHeight();
        
        string theSpeech = "";
        for(int u = 0; u<d.quotes.size();u++){
            theSpeech.append(d.quotes[u].str + " ");
        }
        vector<string>speech = transformToCollumn(theSpeech, w*0.87, speech_f);
        vector<string>name = transformToCollumn(d.name, w*0.87, headline_f);
        vector<string>description = transformToCollumn(d.description, w*0.87, des_f);
        vector<string>source = transformToCollumn(d.source, w*0.87, des_f);
        vector<string>general = transformToCollumn("Med biblioteket har du adgang til primærkilder til din opgave", w*0.87, general_f);
        string src = "www.bib.ballerup.dk/e-materialer/britannica-original-sources";
       // cout << src<< endl;
        vector<string>general2 = transformToCollumn("Find dem på Britannica Original Sources: "+src, w*0.87, general_f);
        vector<int>align;
        float scale = 3.;
        align.push_back(61*scale+h); // 0 picture + space
        align.push_back(24*scale+name.size()*headline_f.getLineHeight()); //1 speech + space
        align.push_back(24*scale+speech.size()*speech_f.getLineHeight()); //1 speech + space
        align.push_back(32*scale+source.size()*des_f.getLineHeight()); // 2 source + space
        align.push_back(31*scale+description.size()*des_f.getLineHeight()); //3 des + space
        align.push_back(31*scale); // space , line
        align.push_back(31*scale+general.size()*general_f.getLineHeight()); // general + spcae
        align.push_back(40*scale+general2.size()*general_f.getLineHeight()); // general + space
        align.push_back(61*scale); // bottom

        int actualH = std::accumulate(align.rbegin(), align.rend(), 0);
        
        ofFbo fbo; // for composing
        ofDisableArbTex();
        fbo.allocate(w,actualH, GL_RGBA);
        
        ofEnableArbTex();
        
        ofPixels pix;
        pix.allocate(w,h,GL_RGBA);
        

        ofPushMatrix();
        fbo.begin();
        ofClear(255);
        ofBackground(255);
        
        bg.draw(0,0,w,h);
        ofTranslate(0, align[0]);
        ofSetColor(0);
        drawCollumn(name, w/2, 0, headline_f);
        ofTranslate(0, align[1]);
        drawCollumn(speech, w/2, 0, speech_f);
        ofTranslate(0, align[2]);
        drawCollumn(source, w/2, 0, des_f);
        ofTranslate(0, align[3]);
        drawCollumn(description, w/2, 0, des_f);
        ofTranslate(0, align[4]);
        ofDrawLine(w/2-95*scale/2,0,w/2+95*scale/2,0);
        ofTranslate(0, align[5]);
        drawCollumn(general, w/2, 0, general_f);
        ofTranslate(0, align[6]);
        drawCollumn(general2, w/2, 0, general_f);
        ofTranslate(0, align[7]);
        vector<string> number ={"#"+ofToString(writeToFile(),6,'0')};
        drawCollumn(number, w/2, 0, number_f);
        
        ofPopMatrix();
        fbo.end();

        
        fbo.readToPixels(pix);
        ofImage img;
        img.setFromPixels(pix);
        img.mirror(true,true);
        ofSaveImage(img, "generated/"+ofToString(current)+".png", OF_IMAGE_QUALITY_BEST);
        return ofToString(current)+".png";
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
    
    void drawCollumn(vector<string> s, int x, int y, ofTrueTypeFont f,int maxLine = 10 , bool center = true){
        
        if(s.size()>maxLine){
    //        s[maxLine-1].pop_back();
            s[MAX(maxLine-1,0)].append("...");
        }
        for(int i = 0; i < MIN(s.size(),maxLine); i++){
            int posx = x;
            if(center){
                posx = x-f.stringWidth(s[i])/2;
            }
            f.drawString(s[i], posx, y+i*f.getLineHeight() );
        }
    }
    bool isSpace(unsigned int c){
        //http://www.fileformat.info/info/unicode/category/Zs/list.htm
        return     c == 0x20 || c == 0xA0 || c == 0x1680 || c == 0x2000 || c == 0x2001
        || c == 0x2002
        || c == 0x2003 || c == 0x2004 || c == 0x2005 || c == 0x2006 || c == 0x2007 || c == 0x2028
        || c == 0x2029
        || c == 0x2008 || c == 0x2009 || c == 0x200A || c == 0x202F || c == 0x205F || c == 0x3000 || c == '/'
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
