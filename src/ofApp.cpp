#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    gui.setup();
    gui.add(animationTime.set("AnimationTime",1.,1.,5000.));
    gui.loadFromFile("settings.xml");
    
    // serial
    serial.listDevices();
    vector <ofSerialDeviceInfo> deviceList = serial.getDeviceList();
    
    // this should be set to whatever com port your serial device is connected to.
    // (ie, COM4 on a pc, /dev/tty.... on linux, /dev/tty... on a mac)
    // arduino users check in arduino app....
    int baud = 9600;
    serial.setup(0, baud); //open the first device
    //serial.setup("COM4", baud); // windows example
    //serial.setup("/dev/tty.usbserial-A4001JEC", baud); // mac osx example
    //serial.setup("/dev/ttyUSB0", baud); //linux example
    
    nTimesRead = 0;
    nBytesRead = 0;
    readTime = 0;
    memset(bytesReadString, 0, 4);
    //    //serial
    
    font.load("circular.ttf", 20);
    charHeight = font.getStringBoundingBox("Åg", 0, 0).height;
    // json crap
    std::string file = "example.json";
    // Now parse the JSON
    ofxJSONElement result;
    bool parsingSuccessful = result.open(file);
    if (parsingSuccessful) {
        if (!result.save("example_output_pretty.json", true))
            ofLogNotice("ofApp::setup") << "example_output_pretty.json written unsuccessfully.";
        else
            ofLogNotice("ofApp::setup") << "example_output_pretty.json written successfully.";
        
        if (!result.save("example_output_fast.json", false))
            ofLogNotice("ofApp::setup") << "example_output_pretty.json written unsuccessfully.";
        else ofLogNotice("ofApp::setup") << "example_output_pretty.json written successfully.";
    }
    else ofLogError("ofApp::setup")  << "Failed to parse JSON" << endl;
    
    quotes.resize(result.size());
    cout << "quotes "<< quotes.size()<<endl;
    ofTexture tex;
    ofLoadImage(tex, "img/alphamask.png");
    
    for(int i = 0; i<quotes.size();i++){
        
        const Json::Value& q = result[i];
        string str = q["quote"].asString();
        int posy = charHeight/2 + font.getLineHeight()/2 - 5;
        ofFbo fbo;
        fbo.allocate(400, charHeight*2, GL_RGBA);
        
        ofRectangle r = font.getStringBoundingBox(str, 0, 0);
        
        int y = charHeight/2 - r.height/2 + font.getLineHeight()/2 + 3;
        
        fbo.begin();
        ofClear(0);
        
        ofSetColor(255);
        font.drawString(str, 30, y);
        
        ofNoFill();
        ofSetLineWidth(0.5);
        
        fbo.end();
        
        ofBufferObject buffer;
        buffer.allocate(fbo.getWidth() * fbo.getHeight() * 4, GL_STATIC_DRAW);
        fbo.getTexture().copyTo(buffer);
        
        quotes[i].tex.allocate(fbo.getWidth(), fbo.getHeight(), GL_RGBA);
        quotes[i].tex.loadData(buffer, GL_RGBA, GL_UNSIGNED_BYTE);

        quotes[i].img.load("img/"+q["imageName"].asString());
        
        int w = quotes[i].img.getWidth();
        int h = quotes[i].img.getHeight();
        int tempH = tex.getHeight();
        w = w * tempH/h;
        quotes[i].img.resize(w,tempH);
        quotes[i].img.getTexture().setAlphaMask(tex);
        
        quotes[i].quoteID = i;
    }
    
    
    for(int i=0; i<5;i++){
        slot s;
        slots.push_back(s);
        
        slots.back().quote = &quotes[int(ofRandom(quotes.size()))];
        slots.back().new_quote = &quotes[int(ofRandom(quotes.size()))];
        slots.back().isAnimated = false;
        slots.back().ypos = 20;
        
    }
}

//--------------------------------------------------------------
void ofApp::update(){

    
    int input = -1;
    nTimesRead = 0;
    nBytesRead = 0;
    int nRead  = 0;  // a temp variable to keep count per read
    
    unsigned char bytesReturned[3];
    
    memset(bytesReadString, 0, 4);
    memset(bytesReturned, 0, 3);
    
    while( (nRead = serial.readBytes( bytesReturned, 3)) > 0){
        nTimesRead++;
        nBytesRead = nRead;
    };
    
    if(nBytesRead>0){
        memcpy(bytesReadString, bytesReturned, 3);
        
        bSendSerialMessage = false;
        readTime = ofGetElapsedTimef();
        
        string fromArduino = string(bytesReadString);
        char fa = fromArduino[0];
        if(int(fa>15)){
            input = ofToInt(fromArduino);
           // cout << fa<<" "<<ofToInt(fromArduino)<<" "<<int(fa) << " bytes "<< nBytesRead << " time " << nTimesRead << endl;
        }
    }
    
    if(input!=-1){
        if(input == 6){
            // do output
        }
        else {
            input = MIN(input , slots.size()-1);
            if(!slots[input].isAnimated){
                int temp = slots[input].quote->quoteID;
                while(temp == slots[input].new_quote->quoteID)
                    slots[input].new_quote = &quotes[int(ofRandom(quotes.size()))];
            }
        }
    }
    
    for(int i = 0; i<slots.size();i++){
        if(slots[i].quote->quoteID!=slots[i].new_quote->quoteID){
            animateSlots(&slots[i]);
        }
        else {
            slots[i].isAnimated = false;
        }
    }
}

void ofApp::animateSlots(slot * s){
    s->ypos -= ofGetLastFrameTime()*animationTime;
    s->isAnimated = true;
   
    if(s->ypos < -s->quote->tex.getHeight()){
        s->ypos = 20;
        s->quote=&quotes[(s->quote->quoteID+1) % quotes.size()];
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    ofBackground(0);
    
    ofPushMatrix();
    
    ofTranslate(40, 100);
    int xpos_rect = charHeight * 3;
    for(int i=0; i<slots.size();i++){

        ofSetColor(255);
        int w = slots[i].quote->img.getWidth();
        int h = slots[i].quote->img.getHeight();
        int tempH = charHeight*2;
        w = w * tempH/h;
        slots[i].quote->img.draw(0, 0, w, tempH);
        
        roundedRect(xpos_rect , 0, 500, charHeight*2, charHeight);
        
        ofSetColor(0);
        
        slots[i].quote->tex.draw(xpos_rect, slots[i].ypos);
        if(slots[i].isAnimated){
            Quote q = quotes[slots[i].quote->quoteID % quotes.size()];
            q.tex.draw(xpos_rect, q.tex.getHeight() + slots[i].ypos);
        }
        ofTranslate(0, charHeight*4);
    }
    ofPopMatrix();
    
    gui.draw();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
    
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){
    
}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){
    
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
    
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){
    
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){
    
}

