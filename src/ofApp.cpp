#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    slash = "\\";
    newLine = "\r\n";
#ifdef __APPLE__
    slash = "/";
    newLine = "\n";
#endif
    
    if(ofFile::doesFileExist(ofToDataPath("printNumberFile.txt"))) {
        auto lines = ofSplitString(ofBufferFromFile("printNumberFile.txt").getText(), newLine);
        printNumber = ofToInt(ofSplitString(lines.back()," ")[0]);
    }else {
        ofFile newFile(ofToDataPath("printNumberFile.txt"),ofFile::WriteOnly); //file doesn't exist yet
        newFile.create(); // now file exists
        newFile.open(ofToDataPath("printNumberFile.txt"),ofFile::WriteOnly);
        newFile << "0 beginning" + newLine;
        newFile.close();
        printNumber = 0;
    }
    
    ofLoadImage(roundedRect, "other"+slash+"roundRect.png");
    ofLoadImage(roundCircle, "other"+slash+"circleOutline.png");
    
    ofSetVerticalSync(true);
    ofEnableAntiAliasing();
    ofEnableAlphaBlending();
    
    
    gui.setup();
    gui.add(animationTime.set("AnimationTime",1.,1.,5000.));
    gui.add(alphaTransitionTime.set("alphaTransitionTime",1.,100.,500.));
    gui.loadFromFile("settings.xml");
    
    font.load("fonts"+slash+"bourton"+slash+"Bourton-Base-Drop.ttf", 30);
    fontSmall.load("fonts"+slash+"bourton"+slash+"Bourton-Base-Drop.ttf", 20);
    
    
    std::string file = "speeches.json";
    ofxJSONElement result;
    bool parsingSuccessful = result.open(file);
    if (parsingSuccessful) {
        ofLogNotice("ofApp::setup") << "example_output_pretty.json written successfully.";
    }
    else ofLogError("ofApp::setup")  << "Failed to parse JSON" << endl;
    
    colors.push_back(ofColor(255,206,0));
    colors.push_back(ofColor(255,115,0));
    colors.push_back(ofColor(255,0,170));
    colors.push_back(ofColor(0,255,157));
    colors.push_back(ofColor(118,0,255));
    colors.push_back(ofColor(255));
    
    speeches.resize(result.size());
    ofImage mask;
    mask.load("other"+slash+"alphamask.png");
    mask.setImageType(OF_IMAGE_COLOR_ALPHA);
    mask.getPixels().setChannel(3, mask.getPixels().getChannel(0));
    
    for(int i = 0; i<speeches.size();i++){
        
        Speech* s = &speeches[i];
        
        const Json::Value& json = result[i];
        
        s->name = json["speech"].asString();
        s->emoji = getResizedTexture("emojis"+slash+json["emoji"].asString(),SLOT_H,SLOT_H, mask);
        s->face = getResizedTexture("faces"+slash+json["imageName"].asString(),SLOT_H,SLOT_H,mask, true);
        s->speechId = i;
        
        s->quotes.resize(5);
        for(int u = 0; u<5; u++){
            string str = json["quote"+ofToString(u+1)].asString();
            s->quotes[u].str = str;
            s->quotes[u].quoteID = u;
            s->quotes[u].speechID = i;
            s->quotes[u].collumn = transformToCollumn((str), SLOT_W, fontSmall);
            s->quotes[u].strTex = getStringAsTexture(ofColor(255), SLOT_W+SLOT_H, SLOT_H, s->quotes[u].collumn, fontSmall, 2);
        }
    }
    
    for(int i=0; i<5; i++){
        slot s;
        slots.push_back(s);
        
        int sp = MIN(i,speeches.size()-1);
        
        slots.back().quote = &speeches[sp].quotes[i];
        slots.back().new_quote =  &speeches[sp].quotes[i];
        
        slots.back().currentColor = sp;
        //    slots.back().currentSpeech = sp;
        slots.back().slotId = i;
        slots.back().isAnimated = false;
        slots.back().ypos = 20;
        slots.back().fbo.allocate(SLOT_W+SLOT_H*3, SLOT_H, GL_RGBA);
    }
    
    
    int baud = 9600;
#ifdef __APPLE__
    system("cancel -a"); // clears print que
    cout << "on mac"<< endl;
#else
    // run bat file to clear print q.
    string cwd = ofFilePath::getCurrentWorkingDirectory();
    string command = cwd + "\\data\\clearPrinter.bat";
    system(command.c_str());
    cout << "on windows"<< endl;
#endif
    /*
     typedef enum {
     HALT
     WINNING_ANIMATION,
     TRANSITION_OUT,
     PRINT_AND_SHOW, // these can have any name you want, eg STATE_CLAP etc
     TRANSITION_IN,
     RESET,
     } State;
     */
    chainEvent.addEvent(2.0, WINNING_HALT);
    chainEvent.addEvent(3.0, WINNING_ANIMATION);
    chainEvent.addEvent(2.0, TRANSITION_OUT);
    chainEvent.addEvent(6.0, PRINT_AND_SHOW);
    chainEvent.addEvent(2.0, TRANSITION_IN);
    chainEvent.addEvent(3.0, RESET);
    
    chainEvent.setToEmpty(4);
    
}

int ofApp::writeToFile(){
    printNumber++;
    ofFile newFile;
    newFile.open(ofToDataPath("printNumberFile.txt"),ofFile::Append);
    newFile <<newLine + ofToString(printNumber)+" "+ofGetTimestampString("%Y-%m-%d-%H");
    newFile.close();
    return printNumber;
}

//--------------------------------------------------------------
void ofApp::update(){
    ofSetWindowTitle(ofToString(ofGetFrameRate()));
    
    
   if(!debug) {
		isInitialized = serial.isInitialized();
		if(!isInitialized)initialiseArdiono();
		else readArduino();
   }
    //while play
    if(input==SHUFFLE_BUTTON && chainEvent.getName()==EMPTY){
        randomizeSlots();
        input = -1;
    }
    
   
    for(int i = 0; i<slots.size();i++){
        animateSlots(&slots[i], i);
        
        ofPushMatrix();
        int s = slots[i].quote->speechID;
        int w = speeches[s].face.getWidth();
        int h = speeches[s].face.getHeight();
        int alpha = slots[i].isLocked ? 100: 255;
        
        float imageScale = 0.8;
        float emojiScale = 0.45;
        slots[i].fbo.begin();
        ofClear(0);
        ofSetColor(255);
        float emojiAspect = speeches[s].emoji.getHeight()/speeches[s].emoji.getWidth();
        speeches[s].emoji.draw(0,h/2-h*emojiScale/2,h*emojiScale,(h*emojiScale)*emojiAspect);
        ofTranslate(h, 0);
        ofSetColor(colors[slots[i].currentColor]);
        roundedRect.draw(0, 0, SLOT_W+SLOT_H, SLOT_H);
        ofSetColor(0);
        slots[i].quote->strTex.draw(0, slots[i].ypos);
        
        if(slots[i].isAnimated){
            Quote q = speeches[(s+1)%speeches.size()].quotes[i];
            q.strTex.draw(0, q.strTex.getHeight() + slots[i].ypos);
        }
        
        ofTranslate(-h*0.4, 0);
        ofSetColor(255);
        speeches[s].face.draw(0, h/2-h*imageScale/2, w*imageScale, h*imageScale);
        ofSetColor(colors[slots[i].currentColor]);
        roundCircle.draw(0, h/2-h*imageScale/2, h*imageScale, h*imageScale);
        slots[i].fbo.end();
        ofPopMatrix();
    }

    
    int same = slots[0].quote->speechID;
    for(int i = 0; i<slots.size();i++) {
        int s = slots[i].quote->speechID;
        if(same!=s) {
            same = -1;
        }
    }
    if(same!=-1) {
        if(chainEvent.getName()==EMPTY)
            chainEvent.beginEvents();
    }
    chainEvent.update();
    
    /*
     typedef enum {
     WINNING_HALT
     WINNING_ANIMATION,
     TRANSITION_OUT,
     PRINT_AND_SHOW, // these can have any name you want, eg STATE_CLAP etc
     TRANSITION_IN,
     RESET,
     } State;
     */
    switch (chainEvent.getName()) {
        case WINNING_HALT: {
            if(same == -1){
                chainEvent.setToEmpty();
            }
            break;
        }
        case WINNING_ANIMATION: { // something something
            finalSpeech.clear();
            string temp = "";
            for(int i = 0; i<slots.size();i++){
                string str = slots[i].quote->str;
                temp.append(str+" ");
            }
            finalSpeech = transformToCollumn(temp, 800, fontSmall);
            serial.writeByte(9);
            
            break;
        }
        case TRANSITION_OUT: {
            for(int i = 0; i<slots.size();i++) {
                slots[i].xpos = ease(chainEvent.getTime(), 0.f, -SLOT_W*4.f, chainEvent.getDuration());
            }
            transitionAlpha = ease(chainEvent.getTime(), 0.f, 255.f, chainEvent.getDuration());
            break;
            
        }
        case PRINT_AND_SHOW: {
            if(!debug)
                printSpeech(slots[0].quote->speechID);
            break;
        }
        case TRANSITION_IN: {
            for(int i = 0; i<slots.size();i++) {
                slots[i].xpos = ease(chainEvent.getTime(), -SLOT_W*4.f, 0.f, chainEvent.getDuration());
            }
            transitionAlpha = ease(chainEvent.getTime(), 255.f, 0.f, chainEvent.getDuration());
            break;
            
        }
        case RESET: {
            for(int i = 0; i<slots.size();i++){
                slots[i].isLocked = false;
            }
            randomizeSlots();
            break;
        }
        default:
            break;
    }
    
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    ofBackground(0);
    
    ofPushMatrix();
    ofTranslate(40, 100);
    for(int i= 0; i<slots.size(); i++){
        int s = slots[i].quote->speechID;
        int w = speeches[s].face.getWidth();
        int h = speeches[s].face.getHeight();
        int alpha = slots[i].isLocked ? 100: 255;
        ofSetColor(255, alpha);
        slots[i].fbo.draw(h * 1.2 + slots[i].xpos, 0);
        ofTranslate(0,slots[i].fbo.getHeight()*1.2);
    }
    ofPopMatrix();
    
    switch (chainEvent.getName()) {
        case WINNING_ANIMATION: {
            ofSetColor(colors[slots[0].currentColor]);
            ofDrawRectangle(0, ofGetHeight()-100, ease(chainEvent.getTime(), 0, ofGetWidth(), chainEvent.getDuration()), 100);
            break;
        }
        case TRANSITION_OUT: {
            ofSetColor(colors[slots[0].currentColor]);
            ofDrawRectangle(0, ofGetHeight()-100, ease(chainEvent.getTime(), ofGetWidth(), 0, chainEvent.getDuration()), 100);
            break;
        }
        default:
            break;
    }
    

    
    if(transitionAlpha>0){
        ofSetColor(255,CLAMP(transitionAlpha,0,255));
        drawCollumn(finalSpeech, 200, 200, fontSmall, 100);
    }
    
    if(debug)gui.draw();
}

//--------------------------------------------------------------
void ofApp::randomizeSlots(bool different){
    for(int i = 0; i<slots.size();i++){
        if(!slots[i].isAnimated && !slots[i].isLocked){
            int temp = slots[i].quote->speechID;
            if(different)temp = slots[MAX(i-1,0)].quote->speechID;
            int random = temp;
            while(temp == random){
                random = ofRandom(speeches.size());
            }
            slots[i].new_quote = &speeches[random].quotes[i];
            slots[i].colorIsSet = false;
            // slots[i].currentSpeech = random;
        }
    }
    
    for(int i = 0; i<slots.size();i++){
        if(!slots[i].colorIsSet) {
            slots[i].currentColor = getColor(&slots[i]);
            slots[i].colorIsSet = true;
        }
    }
}

//--------------------------------------------------------------
void ofApp::printSpeech(int s){
    // print..
    string cwd = ofFilePath::getCurrentWorkingDirectory();
    
#ifdef __APPLE__
    string command = "lp "+ ofSplitString(cwd,"/bin")[0] + "/bin/data/speeches/"+ speeches[s].name+".png";
    if(!debug)system(command.c_str());
    
#else
    string command = "SumatraPDF.exe -print-to-default "+ cwd +"\\data\\speeches\\"+ speeches[s].name+".png";
    if(debug)cout << command <<" "<< cwd << endl;
    if(!debug)system(command.c_str());
    
#endif
    
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
#else
    serial.setup(0, baud); // windows example
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
    
    int tempInput = -1;
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
    char fa = fromArduino[0]-'0';
    tempInput = fa;
    
    if(tempInput>-1) {
        
        if(p_input!=tempInput){
            p_input=tempInput;
            input = tempInput;
            cout <<"From Arduino "<< input << endl;
        }
    }
    
    if(debug){
        if(ofGetMouseX()>ofGetWidth()-300 && ofGetMouseY()<300){
            int tempInput = SHUFFLE_BUTTON;
            if(p_input!=tempInput){
                p_input=tempInput;
                input = tempInput;
                cout <<"From Arduino "<< input << endl;
            }
        }
        else{
            int tempInput = 0;
            if(p_input!=tempInput){
                p_input=tempInput;
                input = tempInput;
                cout <<"From Arduino "<< input << endl;
            }
        }
    }
}
//--------------------------------------------------------------
void ofApp::keyPressed(int key){

    
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
	if (key == 'd')debug = !debug;
	ofSetFullscreen(!debug);
    
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
    
    int h = speeches[0].face.getHeight();
    for(int i = 0; i<slots.size(); i++){
        if(y-100>SLOT_H*1.2*i && y-100<SLOT_H*1.2*(i+1)){
            slots[i].isLocked = !slots[i].isLocked;
            break;
        }
    }
}
//--------------------------------------------------------------
void ofApp::animateSlots(slot * s, int i){
    if(s->quote->speechID!=s->new_quote->speechID) {
        s->ypos -= ofGetLastFrameTime()*animationTime;
        s->isAnimated = true;
        if(s->ypos < - s->quote->strTex.getHeight()){
            s->ypos = 20;
            s->quote = &speeches[(s->quote->speechID + 1)%speeches.size()].quotes[s->slotId];
        }
    } else {
        s->isAnimated = false;
    }
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

