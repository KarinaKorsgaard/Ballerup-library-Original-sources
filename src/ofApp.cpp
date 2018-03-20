#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    gui.setup();
    gui.add(animationTime.set("AnimationTime",1.,1.,5000.));
    gui.add(alphaTransitionTime.set("alphaTransitionTime",1.,100.,500.));
    gui.add(scaleFace.set("scaleFace",0.1,0.,2.));
    gui.add(facePos.set("facePos",ofVec2f(0,0),ofVec2f(0,0),ofVec2f(ofGetWidth(),ofGetHeight())));
    
    gui.loadFromFile("settings.xml");

    font.load("circular.ttf", 20);
    fontSmall.load("circular.ttf", 15);
    
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
    ofLoadImage(tex, "other/alphamask.png");
    
    vector<string>assets;
    assets.push_back("nose");
    
    assets.push_back("eyeLeft");
    assets.push_back("eyeRight");
    assets.push_back("mouthLower");
    assets.push_back("mouthUpper");
    
    faceCoords.push_back(ofRectangle(0,344-278,124,200));
    faceCoords.push_back(ofRectangle(faceCoords[0].x-100,0,184,120));
    faceCoords.push_back(ofRectangle(faceCoords[0].x+100,0,184,120));
    faceCoords.push_back(ofRectangle(0-20,549-278,276,133));
    faceCoords.push_back(ofRectangle(0-20,449-278,276,153));
    
    
    
    
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
        
        quotes[i].str = str;
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
        
        quotes[i].face.resize(assets.size());
        for(int u = 0; u<assets.size();u++){
            string name =ofSplitString(q["imageName"].asString(), ".")[0];
            string path = ofToDataPath("cutouts/"+name+"-"+assets[u]+".png");
            if(ofFile::doesFileExist(path))
               quotes[i].face[u].load(path);
            else
                quotes[i].face[u].load("cutouts/blank.png");
        }    
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
    
    
    ofSetWindowTitle(ofToString(ofGetFrameRate()));
    
    isInitialized = serial.isInitialized();
    if(!isInitialized)initialiseArdiono();
    else readArduino();
    
    if(input!=-1){
        if(input == 6){
            doOutput();
        }
        else if (input<slots.size()){
            
            isDone = false;
            if(!slots[input].isAnimated){
                int temp = slots[input].quote->quoteID;
                while(temp == slots[input].new_quote->quoteID)
                    slots[input].new_quote = &quotes[int(ofRandom(quotes.size()))];
            }
        }
    }
    
    
    for(int i = 0; i<slots.size();i++){
        animateSlots(&slots[i]);
    }
    
    if(!isDone){
        if(transitionAlpha>0)
            transitionAlpha-=ofGetLastFrameTime()*alphaTransitionTime;
    }
    else if (isDone && slots[0].xpos<-ofGetWidth()){
        if(transitionAlpha<254)
            transitionAlpha+=ofGetLastFrameTime()*alphaTransitionTime;
    }
    
}
void ofApp::doOutput(){
    if(!isDone){
        finalSpeech.clear();
        string temp = "";
        for(int i = 0; i<slots.size();i++){
            string str = slots[i].quote->str;
            for(int u = 0; u<str.length();u++){
                temp.append(str.substr(u,1));
                
                ofRectangle r = font.getStringBoundingBox(temp, 0, 0);
                if(r.width>collumWidth && str.substr(u,1)==" "){
                    finalSpeech.push_back(temp);
                    cout<< finalSpeech.back()<<endl;
                    temp = "";
                }
            }
            if(i!=slots.size()-1)
                temp.append(", ");
            
            //finalSpeech.push_back(temp);
            //cout<< finalSpeech.back()<<endl;
        }
        finalSpeech.push_back(temp);
        finalSpeech.back().append(".");
        
        
    }
    
    isDone = true;
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
        slots[i].quote->img.draw(slots[i].xpos + 0, 0, w, tempH);
        
        roundedRect(slots[i].xpos + xpos_rect , 0, 500, charHeight*2, charHeight);
        
        ofSetColor(0);
        
        slots[i].quote->tex.draw(slots[i].xpos + xpos_rect, slots[i].ypos);
        if(slots[i].isAnimated){
            Quote q = quotes[slots[i].quote->quoteID % quotes.size()];
            q.tex.draw(slots[i].xpos + xpos_rect, q.tex.getHeight() + slots[i].ypos);
        }
        ofTranslate(0, charHeight*4);
    }
    ofPopMatrix();
    
    if(isDone){

        ofSetColor(transitionAlpha);

        int lineH = font.getLineHeight();
        if(slots.back().xpos<-ofGetWidth()){
            for(int i = 0; i<finalSpeech.size();i++)
            font.drawString(finalSpeech[i], ofGetWidth()/2, (ofGetHeight()/2-finalSpeech.size()*lineH/2) + i*lineH);
            drawFace();
        }

        ofRectangle r = fontSmall.getStringBoundingBox("press print to print your speech,", 0, 0);
        fontSmall.drawString("press print to print your speech,",ofGetWidth()/2-r.width/2,ofGetHeight()-100);
        r = fontSmall.getStringBoundingBox("or press a button to restart!", 0, 0);
        fontSmall.drawString("or press a button to restart!",ofGetWidth()/2-r.width/2,ofGetHeight()-70);
        
        
    }
    
    gui.draw();
}
//--------------------------------------------------------------
void ofApp::drawFace(){
    ofPushMatrix();
    ofTranslate(facePos->x, facePos->y);
    ofScale(scaleFace, scaleFace);
    
    bool blink = ofRandom(2)<0.1;
    if(blink)lowerMouth.y = sin(ofGetElapsedTimef()*10)*50;
    lowerMouth.y = lowerMouth.y>0 ? 50:0;

    for(int i = slots.size()-1; i>-1; i--){
        ofImage img = slots[i].quote->face[i];
        float a = img.getWidth()/img.getHeight();
        
        float h = faceCoords[i].height;
        float w = h*a;
        //if((i==1 || i==2)&&blink)h=0;
        
        float x = faceCoords[i].x;
        float y = faceCoords[i].y;
        if(i==3)y+=lowerMouth.y;
        
        img.draw(x,y,w,h);
    }
    ofPopMatrix();
}
//--------------------------------------------------------------
void ofApp::animateSlots(slot * s){
    if (!isDone){
        //animate quote pos
        if(s->quote->quoteID!=s->new_quote->quoteID) {
            s->ypos -= ofGetLastFrameTime()*animationTime;
            s->isAnimated = true;
            if(s->ypos < -s->quote->tex.getHeight()){
                s->ypos = 20;
                s->quote=&quotes[(s->quote->quoteID+1) % quotes.size()];
            }
        } else {
            s->isAnimated = false;
        }
        //animate back in
        if(s->xpos<0) {
            float t = ofGetElapsedTimef()-s->animateTime;
            s->xpos = easeInOut_s(t, -ofGetWidth(), ofGetWidth(), 2, 1);
        }else{
            s->animateTime = ofGetElapsedTimef();
        }
        if(transitionAlpha>0)
            transitionAlpha-=ofGetLastFrameTime();
    }
    else {
        if(s->xpos>-ofGetWidth()) {
            float t = ofGetElapsedTimef()-s->animateTime;
            s->xpos = easeInOut_s(t, 0 , -ofGetWidth(), 2, 1);
        }else{
            s->animateTime = ofGetElapsedTimef();
        }
    }
}
//--------------------------------------------------------------
void ofApp::initialiseArdiono(){
    serial.listDevices();
    vector <ofSerialDeviceInfo> deviceList = serial.getDeviceList();
    
    // this should be set to whatever com port your serial device is connected to.
    // (ie, COM4 on a pc, /dev/tty.... on linux, /dev/tty... on a mac)
    // arduino users check in arduino app....
    int baud = 9600;
#ifdef __APPLE__
    serial.setup(0, baud); //open the first device
    cout << "on mac"<< endl;
#elif
    serial.setup("COM4", baud); // windows example
    cout << "on windows"<< endl;
#endif
    //serial.setup("COM4", baud); // windows example
    //serial.setup("/dev/tty.usbserial-1421", baud); // mac osx example
    //serial.setup("/dev/ttyUSB0", baud); //linux example
    
    nTimesRead = 0;
    nBytesRead = 0;
    readTime = 0;
    memset(bytesReadString, 0, 4);
    
    isInitialized = serial.isInitialized();
    if(isInitialized)cout << "arduino is on"<<endl;
}
void ofApp::readArduino(){
    input = -1;
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
    
    //if(nBytesRead>0){
        memcpy(bytesReadString, bytesReturned, 3);
        
        bSendSerialMessage = false;
        readTime = ofGetElapsedTimef();
        
        string fromArduino = string(bytesReadString);
        char fa = fromArduino[0];
        if(int(fa>15)){
            input = ofToInt(fromArduino);
            //cout << fa<<" "<<ofToInt(fromArduino)<<" "<<int(fa) << " bytes "<< nBytesRead << " time " << nTimesRead << endl;
        }
   // }
    if(fromArduino!="")cout << fromArduino << endl;
}
//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if(key == 'd')doOutput();
    
    if(key-'0'<slots.size()){
       slots[key-'0'].quote = &quotes[int(ofRandom(quotes.size()))];
        
    }

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

