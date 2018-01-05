#pragma once

#include "ofMain.h"
#include "ofEvents.h"

#define NUM_ENEMY 5

class enemy {
public:
	ofVec2f pos;
	float rad;
	float speed;
};


class ofApp : public ofBaseApp {

public:
	static const int DIST_PIN_1 = 0;
	static const int DIST_PIN_2 = 1;

	void setup();
	void update();
	void draw();


	//Screen States represent the different Menu and Game screens
	int m_screenState;
	// 0 = no one ready
	// 1 = p1 ready
	// 2 = p2 ready
	// 3 = both ready
	// 4 = game screen
	// 5 = p1 win
	// 6 = p2 win

	//Image variables
	ofImage	m_background[2];
	ofImage	m_menu[6];
	ofImage m_player[2];
	ofImage m_enemyImg;


	int bgstate;	//state of background for twinkling stars
	int counter;	//counter to slow the twinkling of stars

	ofTrueTypeFont  m_font;
	ofArduino		m_arduino;

	float			m_distVal1;				//distance values
	float			m_distVal2;

	float			m_distValRounded1;		//eased distance values
	float			m_distValRounded2;

	bool			m_win1;					//bools to check for player
	bool			m_win2;

	ofVec2f			m_playerPos1;			//player position
	ofVec2f			m_playerPos2;
	
	float			m_playerDisFromBottom;	//player y position
	float			m_playerWidth;			//used to detect collision and constrain edges

	enemy			m_enemy[2][NUM_ENEMY];	//initialize enemies

	float			m_width;				//ease of access variables for window size
	float			m_height;

	bool			m_bSetup;

	float TEMPRGB;


	void setupArduino(const int & _version);
	void updateArduino();
	bool collision(ofVec2f pos1, ofVec2f pos2, float rad1, float rad2);
	void setValues();

	float getIRDistance(int _value);
};

