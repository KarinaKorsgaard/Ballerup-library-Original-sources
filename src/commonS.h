#ifndef common_h
#define common_h

#define SLOT_W 1920-300
#define SLOT_H 150



struct Quote {
	ofTexture strTex;
	string str;
	vector<string>collumn;
	int quoteID;
	int speechID;
};

struct Speech {
    vector<Quote>quotes;
    ofTexture emoji;
    ofTexture face;
    int speechId;
    string name;
    string speaker;
    string source;
    string description;
};

struct slot {
	Quote * quote;
	Quote * new_quote;
    Quote * p_quote;
	bool isAnimated;
	float ypos = 0.0;
	float xpos = -SLOT_W*4.f;
	double animateTime = 0.0;
	bool isLocked = false;
	ofFbo fbo;
	int currentColor;
	int slotId;
	bool colorIsSet = true;
    int colorNumber;
    int p_colorNumber;

};


#endif
