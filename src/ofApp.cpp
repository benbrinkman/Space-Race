#include "ofApp.h"


//--------------------------------------------------------------
void ofApp::setup()
{
	ofSetFrameRate(60);
	m_width = ofGetWindowWidth();
	m_height = ofGetWindowHeight();
	m_font.load("franklinGothic.otf", 16);
		
	setValues();	//all variables that need to be reset for the game to start over are in this function

	m_playerWidth = 50;
	counter = 0;
	bgstate = 0;

	m_playerDisFromBottom = 50;
	// replace the string below with the port for your Arduino board
	// you can get this from the Arduino application (Tools Menu -> Port) 
	m_arduino.connect("COM3", 57600);

	m_distVal1 = m_distVal2 = m_distValRounded1 = m_distValRounded2 = 0;

	// Listen for EInitialized notification. this indicates that the arduino is ready to receive commands and it is safe to call setupArduino()
	ofAddListener(m_arduino.EInitialized, this, &ofApp::setupArduino);

	m_bSetup = false;

	//load art assets
	m_background[0].load("bg.jpg");
	m_background[1].load("bg2.jpg");
	m_menu[0].load("menu1.png");
	m_menu[1].load("menu2.png");
	m_menu[2].load("menu3.png");
	m_menu[3].load("menu4.png");
	m_menu[4].load("p1win.png");
	m_menu[5].load("p2win.png");
	m_player[0].load("ship.png");
	m_player[1].load("ship2.png");
	m_enemyImg.load("enemy.png");
}

//--------------------------------------------------------------
void ofApp::update()
{
	ofSetFrameRate(60);

	//make easy access to screen dimensions
	m_width = ofGetWindowWidth();
	m_height = ofGetWindowHeight();

	//counter for the twinkling of stars effect
	counter++;
	if (counter > 60)
		counter = 0;

	if (counter > 30)
		bgstate = 0;
	if (counter <= 30)
		bgstate = 1;

	// Sometimes the EInitialized event does not fire reliably for Arduino.
	// As a failsafe, if the arduino is not set up after 5 seconds, force it
	// to be set up.
	if (false == m_bSetup && ofGetElapsedTimeMillis() > 5000)
	{
		setupArduino(0);
	}

	if (true == m_bSetup) {
		updateArduino();

		//get distance values for each sensor
		m_distVal1 = getIRDistance(m_arduino.getAnalog(DIST_PIN_1));
		m_distVal2 = getIRDistance(m_arduino.getAnalog(DIST_PIN_2));
		//ofLogNotice() << "distance: " << m_distVal1 << "cm";
		
		
		//screen state variable represents the different menu screens. This controls when the players are ready 
		if (m_screenState < 4) {	//screenState 4 is the game screen, so anything before is menu states
			if (m_distVal1 < 15 && m_distVal2 < 15)
				m_screenState = 3;
			else if (m_distVal1 < 15)
				m_screenState = 1;
			else if (m_distVal2 < 15)
				m_screenState = 2;
			else
				m_screenState = 0;
		}
		if (m_screenState == 3 && m_distVal1 < 5.0f && m_distVal2 < 5.0f && ofGetElapsedTimeMillis() > 7000) {	//since we initialize distance to 0, wait a bit longer to let the game start so it does not prematurly begin
			m_screenState = 4;
		}

		//gameplay
		if (m_screenState == 4) {
			if (m_distVal1 > 30)m_distVal1 = 30;			//Ensure we don't get values out of range
			if (m_distVal2 > 30)m_distVal2 = 30;

			float dif1 = (m_distValRounded1 - m_distVal1);	//get distance between for easing
			float dif2 = (m_distValRounded2 - m_distVal2);

			m_distValRounded1 -= dif1 / 15;					//move 1/15 of the difference every frame
			m_distValRounded2 -= dif2 / 15;

			//map distance values to on game dimensions
			//both will be drawn from the center of the screen, so player one is inverted
			m_playerPos1 = ofVec2f(ofMap(m_distValRounded1, 4.5f, 20.0f, m_playerWidth, m_width / 2 - m_playerWidth, true) * -1.0f, m_height - m_playerDisFromBottom);
			m_playerPos2 = ofVec2f(ofMap(m_distValRounded2, 4.5f, 20.0f, m_playerWidth, m_width / 2 - m_playerWidth, true), m_height - m_playerDisFromBottom);
			
			//enemy controls
			for (int j = 0; j < 2; j++) {						//run once for each side
				for (int i = 0; i < NUM_ENEMY; i++) {
					m_enemy[j][i].pos.y += m_enemy[j][i].speed;	//move enemies
					
					if (j == 0 && m_enemy[j][i].pos.x > 0) {	//invert enemies on player one side
						m_enemy[j][i].pos.x *= -1;
					}
					if (m_enemy[j][i].pos.y > m_height + m_enemy[j][i].rad) {	//reset enemies when they go out of sight
						m_enemy[j][i].rad = ofRandom(20, 40);					//give random radius to make the user feel like it's not the same enemies over and over
						m_enemy[j][i].pos.y = -(m_enemy[j][i].rad);				//place just above the screen
						m_enemy[j][i].pos.x = ofRandom(m_enemy[j][i].rad, m_width / 2 - m_enemy[j][i].rad);	//place randomly on the X plane
					}

					//collision detection for each player to determine who wins
					if (j == 0) {
						if (collision(m_enemy[j][i].pos, m_playerPos1, m_enemy[j][i].rad, m_playerWidth / 2)) {
							m_screenState = 6;	//player 2 wins screen
						}
					}
					if (j == 1) {
						if (collision(m_enemy[j][i].pos, m_playerPos2, m_enemy[j][i].rad, m_playerWidth / 2)) {
							m_screenState = 5;	//player 1 wins screen
						}
					}
				}
			}
		}
		else if (m_screenState > 4 && m_distVal1 < 5 && m_distVal2 < 5) {	//reset game to play again
			m_screenState = 4;
			setValues();
		}
	}
}

//--------------------------------------------------------------
void ofApp::setupArduino(const int & _version)
{
	// remove listener because we don't need it anymore
	ofRemoveListener(m_arduino.EInitialized, this, &ofApp::setupArduino);

	// print firmware name and version to the console
	ofLogNotice() << m_arduino.getFirmwareName();
	ofLogNotice() << "firmata v" << m_arduino.getMajorFirmwareVersion() << "." << m_arduino.getMinorFirmwareVersion();

	
	m_arduino.sendAnalogPinReporting(DIST_PIN_1, ARD_ANALOG);	//A0
	m_arduino.sendAnalogPinReporting(DIST_PIN_2, ARD_ANALOG);	//A1

	m_bSetup = true;
}

//--------------------------------------------------------------
void ofApp::updateArduino() {

	// update the arduino, get any data or messages.
	// the call to m_arduino.update() is required
	m_arduino.update();

}

//--------------------------------------------------------------
void ofApp::draw()
{
	//these run regardless of screen state
	ofBackground(0, 0, 0);
	m_background[bgstate].draw(0, 0, 1280, 720);
	ofRectMode(OF_RECTMODE_CENTER);
	

	//draw the different screen based on current state
	if (m_screenState == 0) {
		m_menu[0].draw(0, 0, 1280, 720);
	}
	else if (m_screenState == 1) {
		m_menu[1].draw(0, 0, 1280, 720);
	}
	else if (m_screenState == 2) {
		m_menu[2].draw(0, 0, 1280, 720);
	}
	else if (m_screenState == 3) {
		m_menu[3].draw(0, 0, 1280, 720);
	}
	else if (m_screenState == 4) {

		ofSetColor(255, 255, 255);
		ofDrawRectangle(m_width / 2 - 5, 0, 10, m_height);		//draw divide line

		for (int j = 0; j < 2; j++) {
			for (int i = 0; i < NUM_ENEMY; i++) {
				ofPushMatrix();
				ofTranslate(m_width / 2, 0);			//initialize at halfway point
				ofTranslate(m_enemy[j][i].pos);			//place in correct position
				//ofDrawCircle(0, 0, m_enemy[j][i].rad);
				m_enemyImg.draw(-m_enemy[j][i].rad, -m_enemy[j][i].rad, m_enemy[j][i].rad * 2, m_enemy[j][i].rad * 2);	//subtract radius so it draws on center of the position
				ofPopMatrix();
			}
		}
		//draw player 1
		ofPushMatrix();
			ofTranslate(m_playerPos1);		
			ofTranslate(m_width / 2, 0);
			//ofDrawCircle(0, 0, m_playerWidth / 2);
			m_player[1].draw(-m_playerWidth / 2, -m_playerWidth * 1.46f / 2, m_playerWidth, m_playerWidth * 1.46f);
		ofPopMatrix();

		//draw player 2
		ofPushMatrix();
			ofTranslate(m_width / 2, 0);
			ofTranslate(m_playerPos2);
			//ofDrawCircle(0, 0, m_playerWidth/2);
			m_player[0].draw(-m_playerWidth / 2, -m_playerWidth * 1.46f / 2, m_playerWidth, m_playerWidth * 1.46f);
		ofPopMatrix();
	}
	else if (m_screenState == 5) {
		m_menu[4].draw(0, 0, 1280, 720);	//draw player 1 win screen
	}
	else if (m_screenState == 6) {
		m_menu[5].draw(0, 0, 1280, 720);	//draw player 2 win screen
	}






}

//convert coltage signal into a readable result distance in cm
float ofApp::getIRDistance(int _value)
{
	if (_value < 16)
	{
		_value = 16;
	}

	return 2076.0f / (_value - 11.0f);
}

bool ofApp::collision(ofVec2f pos1, ofVec2f pos2, float rad1, float rad2) {	//collision function to detect if player gets hit
	float dis = ofDist(pos1.x, pos1.y, pos2.x, pos2.y);
	if (dis < (rad1 + rad2))return true;
	else return false;
}

//these are all the values that need to be reinitialized for the game to reset
void ofApp::setValues() {
	for (int j = 0; j < 2; j++) {
		for (int i = 0; i < NUM_ENEMY; i++) {
			m_enemy[j][i].speed = 5;
			m_enemy[j][i].rad = ofRandom(20, 40);
			m_enemy[j][i].pos.y = -(m_enemy[j][i].rad) - (m_height / NUM_ENEMY) * i;	//randomize the y by the screen size so they fall at relativly regular intervals
			m_enemy[j][i].pos.x = ofRandom(m_enemy[j][i].rad, m_width / 2 - m_enemy[j][i].rad);
		}
	}
	m_win1 = m_win2 = false;
}