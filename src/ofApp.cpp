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
    } else {
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
    
    
    
    
    font.load("fonts"+slash+"bourton"+slash+"Bourton-Base.ttf", 30);
    fontSmall.load("fonts"+slash+"bourton"+slash+"Bourton-Base-Drop.ttf", 30);
    
    
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
    //colors.push_back(ofColor(255));
    
    speeches.resize(result.size());
    
    ofDisableArbTex();
    ofImage mask;
    mask.load("other"+slash+"alphamask.png");
    mask.setImageType(OF_IMAGE_COLOR_ALPHA);
    mask.getPixels().setChannel(3, mask.getPixels().getChannel(0));
    ofEnableArbTex();
    
	sp.setup();
    for(int i = 0; i<speeches.size();i++){
        
        Speech* s = &speeches[i];
        
        const Json::Value& json = result[i];
        
        s->name = json["speech"].asString();
        
        ofDirectory dir;
        dir.allowExt("png");
        dir.listDir("emojis");
        string emoji = "emojis"+slash+json["emoji"].asString();
        if(!ofFile::doesFileExist(emoji))emoji.append(".png");
        if(!ofFile::doesFileExist(emoji))emoji = dir.getPath(i%dir.size());
        cout << emoji<< endl;
        s->emoji = getResizedTexture(emoji,SLOT_H,SLOT_H, mask, false, false);
        
     
        
        string face = "faces"+slash+json["imageName"].asString();
        string facejpg = face;
        if(!ofFile::doesFileExist(face))face.append(".png");
        if(!ofFile::doesFileExist(face))face = facejpg + ".jpg";
		
        if(!ofFile::doesFileExist(face))face = "blank.png";
		if (face == "blank.png")cout <<"MISSING!!!:  faces" + slash + json["imageName"].asString() << endl;
        s->face = getResizedTexture(face,SLOT_H,SLOT_H,mask, true);
        s->source = json["source"].asString();
        s->description = json["description"].asString();
        s->speechId = i;
        //s->speaker = json["speaker"].asString();
        
        s->quotes.resize(5);
        for(int u = 0; u<5; u++){
            string str = json["quote"+ofToString(u+1)].asString();
            s->quotes[u].str = str;
            
            s->quotes[u].quoteID = u;
            s->quotes[u].speechID = i;
            s->quotes[u].collumn = transformToCollumn((str), SLOT_W-SLOT_H*2, fontSmall);
            s->quotes[u].strTex = getStringAsTexture(ofColor(255), SLOT_W-SLOT_H, SLOT_H, s->quotes[u].collumn, fontSmall, 2);
        }
		
    }
    
    for(int i=0; i<5; i++){
        slot s;
        slots.push_back(s);
        
        int sp = MIN(i,speeches.size()-1);
        
        slots.back().quote = &speeches[sp].quotes[i];
        slots.back().new_quote =  &speeches[sp].quotes[i];
        
        slots.back().currentColor = sp;
        //slots.back().currentSpeech = sp;
        slots.back().slotId = i;
        slots.back().isAnimated = false;
        slots.back().ypos = 20;
		slots.back().xpos = 0;
        slots.back().fbo.allocate(SLOT_W, SLOT_H, GL_RGBA);
    }
    
    
    int baud = 9600;
#ifdef __APPLE__
    system("cancel -a"); // clears print que
    cout << "on mac"<< endl;
#else
    // run bat file to clear print q.
    string currentWork = ofFilePath::getCurrentWorkingDirectory();
    string com = currentWork + "\\data\\clearShortCut.lnk";
    system(com.c_str());
    cout << "on windows "<< com << endl;
#endif
    /*
     typedef enum {
     HALT
     WINNING_ANIMATION,
     TRANSITION_OUT,
     SHOW_MOVIE, // these can have any name you want, eg STATE_CLAP etc
     EMOJIS_IN,
     TEXT_IN,
     TRANSITION_IN,
     RESET,
     } State;
     */
    chainEvent.addEvent(1.0, WINNING_HALT);
    chainEvent.addEvent(.1, WINNING_ANIMATION);
    chainEvent.addEvent(.5, TRANSITION_OUT);
    //chainEvent.addEvent(1.0, SHOW_MOVIE);
    chainEvent.addEvent(.5, EMOJIS_IN);
    
    chainEvent.addEvent(0.5, EMOJIS_OUT);
    chainEvent.addEvent(1.0, TEXT_IN);
    chainEvent.addEvent(3.0, PAUSE);
    chainEvent.addEvent(2.0, TRANSITION_IN);
    chainEvent.addEvent(3.0, RESET);
    
    chainEvent.setToEmpty(8);

	debug = false;
	ofSetFullscreen(true);
    
    video.load("Jackpot_plate_2.mov");
    
    
    
    gui.setup();
    gui.add(animationTime.set("AnimationTime",1.,.01,2.));
    // gui.add(alphaTransitionTime.set("alphaTransitionTime",1.,100.,500.));
    gui.add(easiness.set("easiness",.2,0.,1.1));
    gui.add(jump.set("jump",1,1,speeches.size()));
    gui.loadFromFile("settings.xml");
}



//--------------------------------------------------------------
void ofApp::update(){
    ofSetWindowTitle(ofToString(ofGetFrameRate()));
    
    
  // if(!debug) {
		isInitialized = serial.isInitialized();
		if(!isInitialized)initialiseArdiono();
		else readArduino();
  // }
    //while play
    if(input==SHUFFLE_BUTTON && chainEvent.getName()==EMPTY){
        randomizeSlots();
    }
    if(input == 'a')slots[0].isLocked = !slots[0].isLocked;
    if(input == 'b')slots[1].isLocked = !slots[1].isLocked;
    if(input == 'c')slots[2].isLocked = !slots[2].isLocked;
    if(input == 'd')slots[3].isLocked = !slots[3].isLocked;
    if(input == 'e')slots[4].isLocked = !slots[4].isLocked;
    
   
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
        speeches[s].emoji.draw(0,SLOT_H/2-MIN(h,w)*emojiScale/2,MIN(h,w)*emojiScale,MIN(h,w)*emojiScale*emojiAspect);

        ofTranslate(SLOT_H, 0);
        //s->ypos = ease(s->animateTime, 0, SLOT_H, animationTime);
		ofSetColor(colors[slots[i].colorNumber]);
        ofDrawRectangle(0, slots[i].ypos, SLOT_W-SLOT_H, SLOT_H);
        ofSetColor(0);
        slots[i].quote->strTex.draw(20, slots[i].ypos + SLOT_H*.1);
        
        if(slots[i].isAnimated){

            ofSetColor(colors[slots[i].p_colorNumber]);
            ofDrawRectangle(0, slots[i].ypos-SLOT_H, SLOT_W-SLOT_H, SLOT_H);
            
            ofSetColor(0);
            slots[i].p_quote->strTex.draw(20, slots[i].ypos-SLOT_H+SLOT_H*.1);
        }
        
        ofSetColor(0);
        roundedRect.draw(-1, -1, SLOT_W-SLOT_H+2, SLOT_H+2);
        ofTranslate(-SLOT_H*0.4, 0);
        ofSetColor(255);
        speeches[s].face.draw(SLOT_H/2-SLOT_H*imageScale/2, SLOT_H/2-MIN(w,h)*imageScale/2, w*imageScale, h*imageScale);
        ofSetColor(255);
        
        roundCircle.draw(SLOT_H/2-SLOT_H*imageScale/2, SLOT_H/2-SLOT_H*imageScale/2, SLOT_H*imageScale, SLOT_H*imageScale);
        ofColor c = slots[i].isLocked ? ofColor::black : colors[slots[i].colorNumber];
        ofSetColor(c);
        roundCircle.draw(SLOT_H/2-SLOT_H*imageScale/2, SLOT_H/2-SLOT_H*imageScale/2, SLOT_H*imageScale, SLOT_H*imageScale);
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
        if(chainEvent.getName()==EMPTY){
            chainEvent.beginEvents();
            serial.writeByte('b');
            cout << "send b"<< endl;
        }
    }
    chainEvent.update();
    
    /*
     chainEvent.addEvent(1.0, WINNING_HALT);
     chainEvent.addEvent(.1, WINNING_ANIMATION);
     chainEvent.addEvent(1.0, TRANSITION_OUT);
     chainEvent.addEvent(.5, EMOJIS_IN);
     
     chainEvent.addEvent(0.5, EMOJIS_OUT);
     chainEvent.addEvent(1.0, TEXT_IN);
     chainEvent.addEvent(3.0, PAUSE);
     chainEvent.addEvent(2.0, TRANSITION_IN);
     chainEvent.addEvent(3.0, RESET);
     */
    switch (chainEvent.getName()) {
        case WINNING_HALT: {
            if(same == -1){
                chainEvent.setToEmpty();
                serial.writeByte('s');
                cout << "send stop"<< endl;
            }
			//cout << "winning halt" << endl;
            if(!debug)
                createSpeech(slots[0].quote->speechID);
            break;
        }
        case WINNING_ANIMATION: { // something something
			printDone = false;
            finalSpeech = transformToCollumn(speeches[slots[0].quote->speechID].source,1920/2.7 ,font);
            finalSpeech.push_back("");
            finalSpeech.push_back("Din gevinst bliver printet nu");
            break;
        }
        case TRANSITION_OUT: {
			//cout << "out " << endl;
            for(int i = 0; i<slots.size();i++) {
                slots[i].xpos = ease(chainEvent.getTime(), 0.f, -SLOT_W*4.f, chainEvent.getDuration());
            }
            if(!video.isPlaying()){
                video.play();
            }
            video.update();
            break;
        }
       
        case EMOJIS_IN:{
            video.update();
            break;
        }
        case EMOJIS_OUT:{
            video.update();
            break;
        }
        case TEXT_IN:{
            if(!debug)
                printSpeech(slots[0].quote->speechID);
            video.update();
            break;
        }
        case TRANSITION_IN: {
            for(int i = 0; i<slots.size();i++) {
                slots[i].xpos = ease(chainEvent.getTime(), -SLOT_W*4.f, 0.f, chainEvent.getDuration());
            }
            video.update();
            break;
        }
        case PAUSE:{
            video.update();
            break;
        }
        
        case RESET: {
            if(video.isPlaying())video.stop();
			//cout << "reset" << endl;
            for(int i = 0; i<slots.size();i++){
                slots[i].isLocked = false;
            }
            randomizeSlots(true);
            break;
        }
        default:
            break;
    }
    input = -1;
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    ofBackground(0);
    
    ofPushMatrix();
	if (debug)ofScale(0.5, 0.5);
	int w = speeches[0].face.getWidth();
	int h = slots[0].fbo.getHeight();
	int spacingTop = 100;
	int w_total = slots[0].fbo.getWidth();
	float spacingH =(1080-spacingTop*2)/5;
    ofPushMatrix();
	ofTranslate((1920/2)-(w_total/2), spacingTop);

    for(int i= 0; i<slots.size(); i++){
        int s = slots[i].quote->speechID;
        ofSetColor(255);
        slots[i].fbo.draw(slots[i].xpos, 0);
		//cout << slots[i].xpos << endl;
        ofTranslate(0, spacingH);
    }
    ofPopMatrix();
    /*
     chainEvent.addEvent(2.0, WINNING_HALT);
     chainEvent.addEvent(.5, WINNING_ANIMATION);
     chainEvent.addEvent(2.0, TRANSITION_OUT);
     chainEvent.addEvent(1.0, SHOW_MOVIE);
     chainEvent.addEvent(.5, EMOJIS_IN);
     PAUSE
     chainEvent.addEvent(.5, EMOJIS_OUT);
     chainEvent.addEvent(1.0, TEXT_IN);
     chainEvent.addEvent(1.0, PAUSE);
     chainEvent.addEvent(2.0, TRANSITION_IN);
     chainEvent.addEvent(3.0, RESET);*/
    switch (chainEvent.getName()) {
        case WINNING_ANIMATION: {
            ofSetColor(colors[slots[0].currentColor]);
            ofDrawRectangle(0, ofGetHeight()-10, ease(chainEvent.getTime(), 0, ofGetWidth(), chainEvent.getDuration()), 10);
            break;
        }
        case TRANSITION_OUT: {
           // ofSetColor(colors[slots[0].currentColor]);
           // ofDrawRectangle(0, ofGetHeight()-10, ease(chainEvent.getTime(), ofGetWidth(), 0, chainEvent.getDuration()), 10);
            float alpha = ease(chainEvent.getTime()*5.,0,255,chainEvent.getDuration());
            ofSetColor(255,alpha);
            video.draw(0, 0, ofGetWidth(), ofGetHeight());
            break;
        }
        case SHOW_MOVIE: {
          //  float alpha = ease(chainEvent.getTime()*5.,0,255,chainEvent.getDuration());
            ofSetColor(255);
            video.draw(0, 0, ofGetWidth(), ofGetHeight());
            break;
        }
        case EMOJIS_IN: {
            ofSetColor(255);
            video.draw(0, 0, ofGetWidth(), ofGetHeight());
            Speech s = speeches[slots[0].quote->speechID];
            int h = 100;
            int w = h* s.emoji.getWidth()/s.emoji.getHeight();
            for(int i = 0; i<5;i++){
                float alpha = ease(chainEvent.getTime()*(5-i),0,255,chainEvent.getDuration());
                alpha = MIN(alpha,255);
                float y = ease(chainEvent.getTime()*(5-i),ofGetHeight()/2,ofGetHeight()/2-h/2,chainEvent.getDuration());
                ofSetColor(255,alpha);
                int x = ofGetWidth()/2-(5*w*1.5)/2 + i*w*1.5;
                s.emoji.draw(x,y,w,h);
            }
            break;
        }
            
        case EMOJIS_OUT: {
            ofSetColor(255);
            video.draw(0, 0, ofGetWidth(), ofGetHeight());
            Speech s = speeches[slots[0].quote->speechID];
            int h = 100;
            int w = h* s.emoji.getWidth()/s.emoji.getHeight();
            for(int i = 0; i<5;i++){
                float scale = ease(chainEvent.getTime(),1,0,chainEvent.getDuration());
                int x = ofGetWidth()/2-(5*w*1.5)/2 + i*w*1.5 + (1-scale)*w/2;
                s.emoji.draw(x,ofGetHeight()/2-(h*scale)/2,w*scale,h*scale);
                
            }
            break;
        }
        
        case TEXT_IN: {
            ofSetColor(255);
            video.draw(0, 0, ofGetWidth(), ofGetHeight());
            ofSetColor(0);
            float y = ease(chainEvent.getTime(),ofGetHeight(),ofGetHeight()-font.getLineHeight()*5,chainEvent.getDuration()/5.);
            sp.drawCollumn(finalSpeech, ofGetWidth()/2, y, font);
            break;
        }
        case PAUSE:{
            ofSetColor(255);
            video.draw(0, 0, ofGetWidth(), ofGetHeight());
            ofSetColor(0);
            sp.drawCollumn(finalSpeech, ofGetWidth()/2, ofGetHeight()-font.getLineHeight()*5, font);
            break;
        }
        case TRANSITION_IN: {
            
            float alpha = ease(chainEvent.getTime(),255,0,chainEvent.getDuration());
            ofSetColor(255,alpha);
            video.draw(0, 0, ofGetWidth(), ofGetHeight());
            ofSetColor(0,alpha);
            sp.drawCollumn(finalSpeech, ofGetWidth()/2, ofGetHeight()-font.getLineHeight()*5, font);
            break;
        }
       
        default:
            break;
    }
    
    ofPopMatrix();
    

    
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
            if(ofRandom(1)>easiness && !different)random = i==0 ? slots.back().quote->speechID : slots[(i-1)%slots.size()].quote->speechID;
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
void ofApp::createSpeech(int s){
    latestFile = sp.generate(speeches[s], s);
}
//--------------------------------------------------------------
void ofApp::printSpeech(int s){
    // print..
    if(!printDone) {
        string file = latestFile;
        
        string cwd = ofFilePath::getCurrentWorkingDirectory();
        
#ifdef __APPLE__
        string command = "lp "+ ofSplitString(cwd,"/bin")[0] + "/bin/data/generated/"+ file;
        if(!debug)system(command.c_str());
        
#else
        string command = cwd+"\\SumatraPDF.exe -print-to-default -print-settings \"fit\" "+ cwd +"\\data\\generated\\"+ file;
        if(debug)cout << command <<" "<< cwd << endl;
        if(!debug)system(command.c_str());
        
#endif
        printDone = true;
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

    for(int i = 0; i<deviceList.size();i++) {
        cout<<i<<": "<<deviceList[i].getDeviceName()<<endl;
        isInitialized = serial.isInitialized();
        if(!isInitialized)serial.setup(deviceList[i].getDeviceName(), baud);
    }



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
    char fa = fromArduino[0];
    tempInput = fa;
    
    if(tempInput>-1 && tempInput!=0) {
        input = tempInput;
        cout <<"From Arduino "<< input << endl;
        /*if(p_input!=tempInput || tempInput == SHUFFLE_BUTTON){
            p_input=tempInput;
            input = tempInput;
            cout <<"From Arduino "<< input << endl;
        }
         */
    }
    
    /*if(debug){
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
    }*/
}
//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    
    if(key == 'w'){
        for(int i = 0; i<slots.size();i++){
            slots[i].new_quote = &speeches[0].quotes[i];
            slots[i].colorIsSet = false;
            // slots[i].currentSpeech = random;
        }

        for(int i = 0; i<slots.size();i++){
            if(!slots[i].colorIsSet) {
                slots[i].currentColor = getColor(&slots[i]);
                slots[i].colorIsSet = true;
            }
        }
    }
    else if(debug)input = SHUFFLE_BUTTON;
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    if(debug && key == 'p'){printSpeech(0);printDone=false;}
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
    float scale = debug ? 0.5 :1.;
    int h = speeches[0].face.getHeight();
    for(int i = 0; i<slots.size(); i++){
        if(y-100*scale>SLOT_H*scale*1.2*i && y-100*scale<SLOT_H*scale*1.2*(i+1)){
            slots[i].isLocked = !slots[i].isLocked;
            break;
        }
    }
}
//--------------------------------------------------------------
void ofApp::animateSlots(slot * s, int i){
    int currentSpeech = s->quote->speechID;
    int newSpeech = s->new_quote->speechID;
    
    if(currentSpeech!=newSpeech) {
        
        s->animateTime+=ofGetLastFrameTime();
        s->ypos = ease(s->animateTime, 0, SLOT_H, animationTime);
        s->isAnimated = true;
        
        if(s->animateTime>animationTime){
            
            s->animateTime = 0.0;
            int dist_to_next;
            if(currentSpeech <= newSpeech) {
                dist_to_next = newSpeech - currentSpeech;
            }
            else {
                dist_to_next = (speeches.size() - currentSpeech) + newSpeech;
            }
            jumpDistance = MIN(jump.get(), dist_to_next);
            
            s->quote = &speeches[(currentSpeech + jumpDistance)%speeches.size()].quotes[s->slotId];
            s->colorNumber = (dist_to_next-jumpDistance + s->currentColor)%colors.size();
            
            currentSpeech = s->quote->speechID;
            
            if(currentSpeech <= newSpeech) {
                dist_to_next = newSpeech - currentSpeech;
            }
            else {
                dist_to_next = (speeches.size() - currentSpeech) + newSpeech;
            }
            jumpDistance = MIN(jump.get(), dist_to_next);

            s->p_quote = &speeches[(currentSpeech + jumpDistance)%speeches.size()].quotes[s->slotId];
            s->p_colorNumber = (dist_to_next-jumpDistance + s->currentColor)%colors.size();
            
            s->ypos = ease(s->animateTime, 0, SLOT_H, animationTime);
            //s->ypos = ease(s->animateTime, 0, SLOT_H, animationTime);
            //s->ypos = ease(s->animateTime, SLOT_H, 0, animationTime);
        }
    
    } else {
        s->isAnimated = false;
        //s->ypos = SLOT_H;
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

