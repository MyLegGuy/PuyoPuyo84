/*
	--------------------------------------
	 Program Name: Puyo Puyo 84
	 Author: MyLegGuy
	 License: MIT (See LICENSE)
	 Description: Puyo Puyo clone for Ti-84 Plus CE
	--------------------------------------
*/
//////////////////////////////////////////////////////////
// Headers
//////////////////////////////////////////////////////////

/* Keep these headers */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Standard headers */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Good graphics
#include <graphx.h>
// Standard stuff
#include <tice.h>
// Keyboard
#include <keypadc.h>
// AppVar storage
#include <fileioc.h>

// Sprite data
#include "gfx/logo_gfx.h"

// main.h
	void savePlayerData();
	uint8_t genericMenu(char* _menuName, char** _menuOptions, uint8_t _totalMenuOptions, uint8_t _startingChoice);

//////////////////////////////////////////////////////////
// Define
//////////////////////////////////////////////////////////

// Set to 1 before compiling
#define IS_NORMAL_CONTROLS 1

// X pixels between the label and its value
#define INFO_INDENTATION 5
// Distance between the board and the info
#define INFO_OFFSET 3

#define FONT_HEIGHT 8

#define PUYO_DEATH_X PUYO_SPAWN_X
#define PUYO_DEATH_Y 0

#define PUYO_SPAWN_X 2
#define PUYO_SPAWN_Y 0

#define BACKGROUND_COLOR COLOR_BLACK

#define POSSIBLE_NUMBER_WIDTH 9

#define HOLD_DOWN_DELAY 75

#define SINGLE_POP_DELAY 500

#define MINPUYOMATCH 4

#define USEGRAPHICSCLIPPING 1

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

#define MAXDEFINEDATTACKPOWER 24

#define PAUSEDSTRING "Pause"
#define NEWHIMESSAGE "New High Score"
#define SAVEOPTIONSNAME "NPUYOSAV"
#define SAVEGAMENAME "NPUYOGAM"

/*
	1 - Initial release
*/
#define SAVEFILEVERSION 1
//
#define TEACHERKEYVERSION 1

#define BOARD_WIDTH 6
#define BOARD_HEIGHT 12

#define MAXPUYOFORECAST 2

#define PUYO_RADIUS 9
// For next next puyo
#define PUYO_RADIUS_SMALL 4

#define KEYBOARD_POLL_WAIT 10
// < wanted time > / KEYBOARD_POLL_WAIT
// Right now I'm doing 500
#define PUYO_DROP_SPEED 50

#if PUYO_RADIUS==9
	#define BOARD_Y_OFFSET 14
#elif PUYO_RADIUS==8
	#define BOARD_Y_OFFSET 34
#else
	#define BOARD_Y_OFFSET (240 - BOARD_X_OFFSET - PUYO_RADIUS*BOARD_HEIGHT*2)
#endif
#define BOARD_X_OFFSET 10
#define BOARD_BOARDER_COLOR COLOR_PINK
#define BOARD_BACKGROUND_COLOR COLOR_GRAY

#define ENABLE_PURPLE_PUYO 0

#define PUYO_NONE 0
#define PUYO_RED 1
#define PUYO_BLUE 2
#define PUYO_GREEN 3
#define PUYO_YELLOW 4
#define PUYO_PURPLE 5

// If I add another color, increase this number
#if ENABLE_PURPLE_PUYO
	#define MAXPUYOINDEX (PUYO_PURPLE+1) // Does include purple puyo
#else
	#define MAXPUYOINDEX PUYO_PURPLE // Does not include purple puyo
#endif

// Palette indexes for custom colors
#define COLOR_RED 166
#define COLOR_BLUE 167
#define COLOR_GREEN 168
#define COLOR_YELLOW 169
#define COLOR_PURPLE 170
#define COLOR_PINK 171
#define COLOR_WHITE 172
#define COLOR_BLACK 173
#define COLOR_GRAY 174

#define COLOR_POPPING_PUYO COLOR_BLACK // Color of a puyo before it pops

// Orientation of secondary puyo
// Default is this, up
#define PUYO_ORIENTATION_UP 1
#define PUYO_ORIENTATION_RIGHT 2
#define PUYO_ORIENTATION_DOWN 3
#define PUYO_ORIENTATION_LEFT 4
#define PUYO_ORIENTATION_UNDEFINED 0

// Keyboard data:
//http://ce-programming.github.io/toolchain/keypadc_8h.html#a8cea914fc7256292713f6cd915e111ac
#if IS_NORMAL_CONTROLS // Normal controls
	#define CONDITION_CLOCKWISE_BUTTON wasJustPressed(1,kb_Mode)
	#define CONDITION_COUNTERCLOCKWISE_BUTTON wasJustPressed(1,kb_2nd)
#else // Emulator controls
	#define CONDITION_CLOCKWISE_BUTTON wasJustPressed(2,kb_Store)
	#define CONDITION_COUNTERCLOCKWISE_BUTTON wasJustPressed(4,kb_2)
#endif
#define CONDITION_PAUSE_BUTTON wasJustPressed(1,kb_Yequ)
#define CONDITION_TEACHER_KEY wasJustPressed(1,kb_Window)

//////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////

uint8_t doDrawWaifu = 1;

uint8_t puyoBoard[BOARD_WIDTH][BOARD_HEIGHT];

uint8_t myPuyoX=3;
uint8_t myPuyoY=1;

uint8_t currentColor=255;

uint8_t primaryPuyoColor=PUYO_RED;
uint8_t secondaryPuyoColor=PUYO_BLUE;

uint8_t nextPrimaryPuyoColor[MAXPUYOFORECAST];
uint8_t nextSecondaryPuyoColor[MAXPUYOFORECAST];

uint8_t puyoColors[MAXPUYOINDEX];

uint8_t secondaryPuyoOrientation=PUYO_ORIENTATION_UP;

uint8_t gameIsRunning=1;

kb_key_t lastKeyboardData[8];

uint32_t currentScore=0;

uint32_t highscore=0;
uint8_t highestEverCombo=0;

// Puyo Puyo Tsu values
// https://puyonexus.com/wiki/List_of_attack_powers
const uint16_t attackPowers[MAXDEFINEDATTACKPOWER] = {0,8,16,32,64,96,128,160,192,224,256,288,320,352,384,416,448,480,512,544,576,608,640,672}; // Multiplayer values
//uint16_t attackPowers[MAXDEFINEDATTACKPOWER] = {4,20,24,32,48,96,160,240,320,480,600,700,800,900,999,999,999,999,999,999,999,999,999,999}; // Singleplayer values

// Has value for purple no matter what
// https://puyonexus.com/wiki/Scoring#Color_Bonus
const uint8_t colorBonus[PUYO_PURPLE+1] = {0,3,6,12,24};

// https://puyonexus.com/wiki/Scoring#Group_Bonus
const uint8_t groupBonus[8] = {0,2,3,4,5,6,7,10};

#define PUYO_WIDTH (PUYO_RADIUS*2+1)
#define PUYO_HEIGHT (PUYO_RADIUS*2+1)

//////////////////////////////////////////////////////////
void goodChangeColor(uint8_t _color){
	if (_color!=currentColor){
		gfx_SetColor(_color);
		currentColor = _color;
	}
}
uint16_t boardToRealCoords(int16_t _value){
	return _value*PUYO_RADIUS*2;
}
void drawX(uint8_t _x, uint8_t _y){
	goodChangeColor(COLOR_POPPING_PUYO);
	gfx_FillRectangle(BOARD_X_OFFSET+boardToRealCoords(_x),BOARD_Y_OFFSET+boardToRealCoords(_y),PUYO_WIDTH,PUYO_HEIGHT);
}
// Pass number of puyo popped in this group
uint8_t getGroupBonus(uint8_t _numberPoppedInGroup){
	if (_numberPoppedInGroup-4>11){
		return groupBonus[7]; // Return max if more than 11 popped
	}
	if (_numberPoppedInGroup<4){ // If we allow 3 puyo to pop
		return groupBonus[0];
	}
	return groupBonus[_numberPoppedInGroup-4];
}

// _groupBonus needs to be calculated manually
uint16_t scoreFormula(uint16_t _puyosClearedInChain, uint8_t _numberOfChains, uint8_t _numberOfDifferentColors, uint8_t _groupBonus){
	uint16_t _partOne = (10*_puyosClearedInChain);
	uint16_t _partTwo = (attackPowers[_numberOfChains-1]+colorBonus[_numberOfDifferentColors-1]+_groupBonus);
	if (_partTwo<1){
		_partTwo=1;
	}else if (_partTwo>999){
		_partTwo=999;
	}
	return _partOne*_partTwo;
}
void _drawScoreString(uint16_t _x, uint16_t _y, char* _scoreString){
	// Hide old score
	goodChangeColor(BACKGROUND_COLOR);
	gfx_FillRectangle(_x,_y,POSSIBLE_NUMBER_WIDTH*9,8);
	// Print new string
	gfx_PrintStringXY(_scoreString, _x, _y); 
}
void drawScore(uint32_t _scoreToDraw, uint16_t _x, uint16_t _y){
	//gfx_FillRectangle(BOARD_X_OFFSET,SCREEN_HEIGHT-8,9*9,8);
	char _numberStringBuffer[10]; // Buffer will be used completely
	// Make new string
	sprintf(&(_numberStringBuffer[0]),"%09d",_scoreToDraw);
	_drawScoreString(_x,_y,_numberStringBuffer);
}
uint16_t getBoardPixelWidth(){
	return BOARD_WIDTH*PUYO_RADIUS*2;
}
uint16_t centerGeneric(uint16_t _size,uint16_t _areaSize){
	return (_areaSize-_size)/(double)2;
}
uint16_t centerStringWidth(char* _stringToCenter, uint16_t _maxSize){
	return centerGeneric(gfx_GetStringWidth(_stringToCenter),_maxSize);
}

// Need a lot of code because string needs to be centered.
void redrawScore(uint32_t _scoreToDraw){
	char _numberStringBuffer[10];
	sprintf(&(_numberStringBuffer[0]),"%09d",_scoreToDraw);
	_drawScoreString(BOARD_X_OFFSET+centerStringWidth(_numberStringBuffer,getBoardPixelWidth()),SCREEN_HEIGHT-FONT_HEIGHT,_numberStringBuffer); // 8 is default font height?
}
#define INFO_X_START (BOARD_X_OFFSET+getBoardPixelWidth()+1+INFO_OFFSET)
#define INFO_Y_START (BOARD_Y_OFFSET+PUYO_RADIUS*MAXPUYOFORECAST*2*2+PUYO_RADIUS)
#define SINGLE_INFO_HEIGHT (2*+FONT_HEIGHT)
void redrawInfoLabel(const char* _labelText, uint8_t _index){
	gfx_PrintStringXY(_labelText,INFO_X_START,INFO_Y_START+SINGLE_INFO_HEIGHT*_index);
}
void redrawInfoLabelValue(uint32_t _value, uint8_t _index){
	char _numberStringBuffer[10];
	sprintf(&(_numberStringBuffer[0]),"%d",_value);
	goodChangeColor(BACKGROUND_COLOR);
	gfx_FillRectangle(INFO_X_START+INFO_INDENTATION,INFO_Y_START+SINGLE_INFO_HEIGHT*_index+FONT_HEIGHT,POSSIBLE_NUMBER_WIDTH*9,FONT_HEIGHT);
	gfx_PrintStringXY(_numberStringBuffer,INFO_X_START+INFO_INDENTATION,INFO_Y_START+SINGLE_INFO_HEIGHT*_index+FONT_HEIGHT);
}
void redrawHighscore(uint32_t _scoreToDraw, uint8_t _index){
	drawScore(_scoreToDraw,INFO_X_START+INFO_INDENTATION,INFO_Y_START+SINGLE_INFO_HEIGHT*_index+FONT_HEIGHT);
}
void controlsStart(){
	uint8_t i;
	for (i=1;i<8;i++){
		lastKeyboardData[i] = kb_Data[i];
	}
	kb_Scan();
}
uint8_t isDown(uint8_t _index,kb_lkey_t _testKey){
	return kb_Data[_index] & _testKey;
}
uint8_t wasJustPressed(uint8_t _index,kb_lkey_t _testKey){
	return (!(lastKeyboardData[_index] & _testKey) && (kb_Data[_index] & _testKey));
}
uint8_t teacherKeyPressed(){
	return CONDITION_TEACHER_KEY;
}
uint8_t pauseButtonPressed(){
	return CONDITION_PAUSE_BUTTON;
}
uint8_t clockwiseButtonPressed(){
	return CONDITION_CLOCKWISE_BUTTON;
}
uint8_t counterclockwiseButtonPressed(){
	return CONDITION_COUNTERCLOCKWISE_BUTTON;
}
// I guess I could use the up button for quick drop one day?
uint8_t upButtonPressed(){
	return wasJustPressed(7,kb_Up);
}
uint8_t downButtonPressed(){
	return wasJustPressed(7,kb_Down);
}
uint8_t downButtonHeld(){
	return isDown(7,kb_Down);
}
uint8_t leftButtonPressed(){
	return wasJustPressed(7,kb_Left);
}
uint8_t rightButtonPressed(){
	return wasJustPressed(7,kb_Right);
}
uint8_t quitButtonPressed(){
	return wasJustPressed(6,kb_Clear);
}
uint8_t getSecondaryPuyoX(uint8_t _passedFirstPuyoX, uint8_t _passedPuyoOrientation){
	switch (_passedPuyoOrientation){
		case PUYO_ORIENTATION_LEFT:
			return _passedFirstPuyoX-1;
		case PUYO_ORIENTATION_RIGHT:
			return _passedFirstPuyoX+1;
		default:
			return _passedFirstPuyoX;
	}
}
uint8_t getSecondaryPuyoY(uint8_t _passedFirstPuyoY, uint8_t _passedPuyoOrientation){
	switch (_passedPuyoOrientation){
		case PUYO_ORIENTATION_UP:
			return _passedFirstPuyoY-1;
		case PUYO_ORIENTATION_DOWN:
			return _passedFirstPuyoY+1;
		default:
			return _passedFirstPuyoY;
	}
}
void goodDrawCircle(uint16_t _x, uint16_t _y, uint8_t _radius){
	#if USEGRAPHICSCLIPPING
		gfx_FillCircle(_x+_radius,_y+_radius,_radius);
	#else
		gfx_FillCircle_NoClip(_x+_radius,_y+_radius,_radius);
	#endif
}
void drawPuyo(uint16_t _x, uint16_t _y, uint8_t _color){
	goodChangeColor(_color);
	goodDrawCircle(BOARD_X_OFFSET+boardToRealCoords(_x),BOARD_Y_OFFSET+boardToRealCoords(_y),PUYO_RADIUS);
}

void clearPuyoBoard(){
	uint8_t i;
	uint8_t j;
	for (i=0;i<BOARD_WIDTH;++i){
		for (j=0;j<BOARD_HEIGHT;++j){
			puyoBoard[i][j]=PUYO_NONE;
		}
	}
}

// Draw a useless error circle to let me know an error occurred
void drawErrorCircle(){
	goodChangeColor(COLOR_PINK);
	goodDrawCircle(0,0,50);
}

void clearPuyoSpot(uint8_t _x, uint8_t _y){
	if (_x==PUYO_DEATH_X && _y==PUYO_DEATH_Y){
		drawX(PUYO_DEATH_X,PUYO_DEATH_Y);
	}else{
		goodChangeColor(BOARD_BACKGROUND_COLOR);
		#if USEGRAPHICSCLIPPING
			gfx_FillRectangle(BOARD_X_OFFSET+boardToRealCoords(_x),BOARD_Y_OFFSET+boardToRealCoords(_y),PUYO_RADIUS*2+1,PUYO_RADIUS*2+1);
		#else
			gfx_FillRectangle_NoClip(BOARD_X_OFFSET+boardToRealCoords(_x),BOARD_Y_OFFSET+boardToRealCoords(_y),PUYO_RADIUS*2+1,PUYO_RADIUS*2+1);
		#endif
	}
}

void drawBoardBackground(){
	goodChangeColor(BOARD_BACKGROUND_COLOR);
	gfx_FillRectangle(BOARD_X_OFFSET,BOARD_Y_OFFSET,getBoardPixelWidth()+1,PUYO_RADIUS*BOARD_HEIGHT*2+1);
}
// < number of puyos drawn > / 2 = amount to add ?
void drawBoardBoarder(){
	goodChangeColor(BOARD_BOARDER_COLOR);
	gfx_Rectangle(BOARD_X_OFFSET-1,BOARD_Y_OFFSET-1,getBoardPixelWidth()+3,BOARD_HEIGHT*PUYO_RADIUS*2+3);
}
void redrawPuyoBoard(){
	uint8_t i;
	uint8_t j;
	drawBoardBackground();
	for (i=0;i<BOARD_WIDTH;++i){
		for (j=0;j<BOARD_HEIGHT;++j){
			if (puyoBoard[i][j]!=PUYO_NONE){
				drawPuyo(i,j,puyoColors[puyoBoard[i][j]]);
			}
		}
	}
	drawBoardBoarder();
}
void redrawPuyoForecast(){
	uint8_t i;
	drawPuyo(BOARD_WIDTH+1,0,puyoColors[nextPrimaryPuyoColor[0]]);
	drawPuyo(BOARD_WIDTH+1,1,puyoColors[nextSecondaryPuyoColor[0]]);
	for (i=1;i<MAXPUYOFORECAST;i++){
		if (i%2==0){
			drawPuyo(BOARD_WIDTH+1,i*2,puyoColors[nextPrimaryPuyoColor[i]]);
			drawPuyo(BOARD_WIDTH+1,i*2+1,puyoColors[nextSecondaryPuyoColor[i]]);
		}else{
			drawPuyo(BOARD_WIDTH+2,i*2,puyoColors[nextPrimaryPuyoColor[i]]);
			drawPuyo(BOARD_WIDTH+2,i*2+1,puyoColors[nextSecondaryPuyoColor[i]]);
		}
	}
}
void redrawWaifu(){
	if (doDrawWaifu){
		gfx_Sprite(AmitieSmall, SCREEN_WIDTH-AmitieSmall_width, SCREEN_HEIGHT-AmitieSmall_height);
	}
}
void redrawAllInfo(){
	redrawInfoLabel("Highscore",0);
	redrawHighscore(highscore,0);
	redrawInfoLabel("Highest Chain",1);
	redrawInfoLabelValue(highestEverCombo,1);
	redrawScore(currentScore);
}
void drawBothPuyos(){
	drawPuyo(myPuyoX,myPuyoY,puyoColors[primaryPuyoColor]);
	drawPuyo(getSecondaryPuyoX(myPuyoX,secondaryPuyoOrientation),getSecondaryPuyoY(myPuyoY,secondaryPuyoOrientation),puyoColors[secondaryPuyoColor]);
}
void redrawEverything(){
	gfx_FillScreen(BACKGROUND_COLOR);
	redrawWaifu();
	redrawPuyoBoard();
	drawX(PUYO_DEATH_X,PUYO_DEATH_Y);
	redrawPuyoForecast();
	redrawAllInfo();
	drawBothPuyos();
}
void hideBothPuyos(){
	clearPuyoSpot(myPuyoX,myPuyoY);
	clearPuyoSpot(getSecondaryPuyoX(myPuyoX,secondaryPuyoOrientation),getSecondaryPuyoY(myPuyoY,secondaryPuyoOrientation));
}
// Can take negative values
uint8_t isOnTopOfAnything(int8_t _x, int8_t _y){
	// Collision
	if (_x<0 || _x==BOARD_WIDTH){
		return 1;
	}
	if (_y<-1 || _y==BOARD_HEIGHT){
		return 1;
	}
	if (puyoBoard[_x][_y]!=PUYO_NONE){
		return 1;
	}
	return 0;
}

void spinPuyo(int8_t _direction){
	uint8_t _possibleNewOrientation;
	if (_direction==-1){ // Counterclockwise
		if (secondaryPuyoOrientation==PUYO_ORIENTATION_UP){
			_possibleNewOrientation = PUYO_ORIENTATION_LEFT;
		}else{
			_possibleNewOrientation=secondaryPuyoOrientation-1;
		}
	}else{ // Clockwise
		if (secondaryPuyoOrientation==PUYO_ORIENTATION_LEFT){
			_possibleNewOrientation = PUYO_ORIENTATION_UP;
		}else{
			_possibleNewOrientation=secondaryPuyoOrientation+1;
		}
	}
	// If spinning the secondary puyo will cause problems
	if (isOnTopOfAnything(getSecondaryPuyoX(myPuyoX,_possibleNewOrientation),getSecondaryPuyoY(myPuyoY,_possibleNewOrientation))){
		if (_possibleNewOrientation==PUYO_ORIENTATION_RIGHT){
			if (!isOnTopOfAnything(myPuyoX-1,myPuyoY)){ // Try to shift main puyo left
				secondaryPuyoOrientation = _possibleNewOrientation;
				myPuyoX--;
				_possibleNewOrientation = PUYO_ORIENTATION_UNDEFINED;
			}
		}else if (_possibleNewOrientation==PUYO_ORIENTATION_DOWN){ // Will always be able to shift up
			myPuyoY--;
			secondaryPuyoOrientation = _possibleNewOrientation;
			_possibleNewOrientation = PUYO_ORIENTATION_UNDEFINED;
		}else if (_possibleNewOrientation==PUYO_ORIENTATION_LEFT){
			if (!isOnTopOfAnything(myPuyoX+1,myPuyoY)){ // Try to shift main puyo right
				secondaryPuyoOrientation = _possibleNewOrientation;
				myPuyoX++;
				_possibleNewOrientation = PUYO_ORIENTATION_UNDEFINED;
			}
		}else{
			// This should not happen.
			drawErrorCircle();
		}

		// If primary puyo shifting didn't work
		if (_possibleNewOrientation!=PUYO_ORIENTATION_UNDEFINED){ // This variable is set to PUYO_ORIENTATION_UNDEFINED if we got primary puyo shifting to work. This code happens if primary puyo shifting doesn't work.
			if (secondaryPuyoOrientation==PUYO_ORIENTATION_UP){
				secondaryPuyoOrientation = PUYO_ORIENTATION_DOWN;
				if (isOnTopOfAnything(getSecondaryPuyoX(myPuyoX,secondaryPuyoOrientation),getSecondaryPuyoY(myPuyoY,secondaryPuyoOrientation))){
					myPuyoY--;
				}
			}else{
				secondaryPuyoOrientation = PUYO_ORIENTATION_UP;
			}
		}
	}else{
		secondaryPuyoOrientation = _possibleNewOrientation;
	}
}

// Don't call directly
void _generateNewPuyoColor(uint8_t* _color){
	*_color = randInt(1,MAXPUYOINDEX-1);
}
void generateNewPuyos(){
	uint8_t i;
	
	// Assign current puyo colors
	primaryPuyoColor = nextPrimaryPuyoColor[0];
	secondaryPuyoColor = nextSecondaryPuyoColor[0];
	// Shift forecast array
	for (i=0;i<MAXPUYOFORECAST-1;i++){
		nextPrimaryPuyoColor[i] = nextPrimaryPuyoColor[i+1];
		nextSecondaryPuyoColor[i] = nextSecondaryPuyoColor[i+1];
	}
	// Add to end of forecast array
	_generateNewPuyoColor(&(nextPrimaryPuyoColor[MAXPUYOFORECAST-1]));
	_generateNewPuyoColor(&(nextSecondaryPuyoColor[MAXPUYOFORECAST-1]));

	// Reset current puyo position
	myPuyoX=PUYO_SPAWN_X;
	myPuyoY=PUYO_SPAWN_Y;
	secondaryPuyoOrientation = PUYO_ORIENTATION_UP;
}
// Don't call directly
// Locks a single puyo at a specific position
void _fallAndWritePuyoData(int8_t _x, int8_t _y, uint8_t _color){
	uint8_t i;
	clearPuyoSpot(_x,_y);
	for (i=_y+1;;i++){
		if (i==BOARD_HEIGHT || puyoBoard[_x][i]!=PUYO_NONE){
			puyoBoard[_x][i-1]=_color;
			drawPuyo(_x,i-1,puyoColors[_color]);
			return;
		}
	}
}

// So I was really happy. I thought that there was a guy who wrote "he fell puyo," but I just remembered it wrong. Now I am sad because he's not a real moron, the real moron is I.
void fellPuyos(){
	int8_t i;
	int8_t j;
	int8_t k;

	for (j=BOARD_HEIGHT-2;j>=0;j--){
		for (i=0;i<BOARD_WIDTH;i++){
			int8_t _cachedPuyoColor=puyoBoard[i][j];
			puyoBoard[i][j] = PUYO_NONE;
			for (k=j;;k++){
				if (puyoBoard[i][k+1]!=PUYO_NONE || k+1==BOARD_HEIGHT){
					puyoBoard[i][k]=_cachedPuyoColor;
					break;
				}
			}
		}
	}
}

// Returns 1 you need to call this function again
int8_t popPuyos(uint8_t* _currentCombo){
	uint8_t _specificPuyoIDs[BOARD_WIDTH][BOARD_HEIGHT]={0};
	uint8_t _comboIDLengths[BOARD_WIDTH*BOARD_HEIGHT+1]; // Don't zero this array. Also don't make this array bigger than a signed byte can hold
	uint8_t _nextComboID = 1;
	uint8_t i;
	uint8_t j;
	int8_t k;
	uint8_t _puyoWasPopped=0;

	// Scoring stuff
	uint16_t _totalPuyosClearedInChain=0;
	uint8_t _whichPuyoColorsPopped[MAXPUYOINDEX]={0};
	uint16_t _calculatedGroupBonus=0;
	_comboIDLengths[0]=0; // Unused slot.

	// Read row by row, starting from the left
	for (j=0;j<BOARD_HEIGHT;j++){
		for (i=0;i<BOARD_WIDTH;i++){
			if (puyoBoard[i][j]!=PUYO_NONE){ // Only do stuff if we're on a puyo
				if (i!=BOARD_WIDTH-1 && puyoBoard[i+1][j]==puyoBoard[i][j]){ // If we can check the puyo to the right and its the same color
					if (_specificPuyoIDs[i+1][j]==0){ // If this other puyo doesn't have a combo ID
						if (_specificPuyoIDs[i][j]==0){// If we don't have our own Puyo ID either, make one.
							_specificPuyoIDs[i][j] = _nextComboID++; // Assign our brand new combo ID. Incremented value is not returned.
							_comboIDLengths[_specificPuyoIDs[i][j]]=1; // New combo ID, so default length is 1
						}
						_specificPuyoIDs[i+1][j] = _specificPuyoIDs[i][j];
						++_comboIDLengths[_specificPuyoIDs[i][j]];
					}else{ // The other puyo already has an ID
						if (_specificPuyoIDs[i+1][j]==_specificPuyoIDs[i][j]){ // They're both the same combo ID, we can do nothing.
							// Do nothing with the puyo to the right
						}else{ // Puyo IDs are different
							if (_specificPuyoIDs[i][j]==0){ // We have no combo ID, we can just be added to the other puyo's combo ID
								_specificPuyoIDs[i][j] = _specificPuyoIDs[i+1][j];
								++_comboIDLengths[_specificPuyoIDs[i][j]];
							}else{ // Combine the two combo IDs
								int8_t l;
								int8_t _cachedIDToConvert=_specificPuyoIDs[i+1][j];
								_comboIDLengths[_cachedIDToConvert]=0; // Other chain ID won't be used anymore, remove it.
								for (l=j;l>0;l--){
									for (k=0;k<BOARD_WIDTH;k++){
										if (_specificPuyoIDs[k][l]==_cachedIDToConvert){ // Find puyo with the same ID as the puyo to the right
											_specificPuyoIDs[k][l]=_specificPuyoIDs[i][j]; // Make those puyos have the same ID as the current puyo
											++_comboIDLengths[_specificPuyoIDs[i][j]]; // Add to the new, combined puyo chain length
										}
									}
								}
							}
						}
					}
				}
				if (j!=BOARD_HEIGHT-1 && puyoBoard[i][j+1]==puyoBoard[i][j]){ // If we can check the puyo under and its the same color
					if (_specificPuyoIDs[i][j]==0){// If we don't have our own Puyo ID either, make one.
						_specificPuyoIDs[i][j] = _nextComboID++;
						_comboIDLengths[_specificPuyoIDs[i][j]]=1;
					}
					++_comboIDLengths[_specificPuyoIDs[i][j]];
					_specificPuyoIDs[i][j+1]=_specificPuyoIDs[i][j];
				}
			}
		}
	}

	
	for (k=0;k<_nextComboID;k++){
		if (_comboIDLengths[k]>=MINPUYOMATCH){ // If we need to pop this combo ID
			uint8_t _totalPuyosInGroup=0;
			if (_puyoWasPopped==0 && *_currentCombo!=0){
				delay(SINGLE_POP_DELAY); // If this is the second combo, delay beore popping more because we want the player to see the board after their first chain.
			}
			_puyoWasPopped=1;
			// Search board for puyos with matching combo IDs
			for (j=0;j<BOARD_HEIGHT;j++){
				for (i=0;i<BOARD_WIDTH;i++){
					if (_specificPuyoIDs[i][j]==k){ // If we found one
						drawPuyo(i,j,COLOR_POPPING_PUYO); // Make it pink
						puyoBoard[i][j]=PUYO_NONE; // Pop it

						// Scoring stuff
						_whichPuyoColorsPopped[puyoBoard[i][j]]=1;
						++_totalPuyosClearedInChain;
						++_totalPuyosInGroup;
					}
				}
			}
			_calculatedGroupBonus+=getGroupBonus(_totalPuyosInGroup);
		}
	}

	if (_puyoWasPopped){
		uint8_t _numberOfDifferentColors=0;
		for (k=0;k<MAXPUYOINDEX;++k){
			if (_whichPuyoColorsPopped[k]!=0){
				++_numberOfDifferentColors;
			}
		}
		++*_currentCombo;
		currentScore+=scoreFormula(_totalPuyosClearedInChain,*_currentCombo,_numberOfDifferentColors,_calculatedGroupBonus);
		redrawScore(currentScore);

		delay(SINGLE_POP_DELAY);
		fellPuyos();
		redrawPuyoBoard();
		return 1;
	}
	return 0;
}

// Lock puyo into place and do combos
void lockPuyos(){
	uint8_t _currentCombo=0;
	if (secondaryPuyoOrientation==PUYO_ORIENTATION_DOWN){ // If the secondary puyo is down, we need it to fall first.
		_fallAndWritePuyoData(getSecondaryPuyoX(myPuyoX,secondaryPuyoOrientation),getSecondaryPuyoY(myPuyoY,secondaryPuyoOrientation),secondaryPuyoColor);
		_fallAndWritePuyoData(myPuyoX,myPuyoY,primaryPuyoColor);
	}else{ // Otherwise, have the first puyo fall first.
		_fallAndWritePuyoData(myPuyoX,myPuyoY,primaryPuyoColor);
		_fallAndWritePuyoData(getSecondaryPuyoX(myPuyoX,secondaryPuyoOrientation),getSecondaryPuyoY(myPuyoY,secondaryPuyoOrientation),secondaryPuyoColor);
	}	
	while (popPuyos(&_currentCombo)==1);
	if (_currentCombo>highestEverCombo){
		highestEverCombo=_currentCombo;
		savePlayerData();
		redrawAllInfo();
	}
}

char playerIsDead(){
	if (puyoBoard[PUYO_DEATH_X][PUYO_DEATH_Y]!=PUYO_NONE){
		return 1;
	}
	return 0;
}

uint8_t yesOrNo(char* _prompt, uint8_t _defaultToYes){
	char* _menuOptions[] = {"Yes","Not Yes"};
	return !genericMenu(_prompt,&(_menuOptions[0]),2,!_defaultToYes);
}

/////////////////////////////////////////////////

// For options
void savePlayerData(){
	ti_var_t myAppVar;
	uint8_t _tempFileFormatVersion;
	myAppVar = ti_Open(SAVEOPTIONSNAME, "w");
	if (myAppVar==0){
		drawErrorCircle();
		delay(300);
		return;
	}
	_tempFileFormatVersion = SAVEFILEVERSION;
	ti_Write(&_tempFileFormatVersion,sizeof(uint8_t),1,myAppVar);
	ti_Write(&doDrawWaifu,sizeof(uint8_t),1,myAppVar);
	ti_Write(&highscore,sizeof(uint32_t),1,myAppVar);
	ti_Write(&highestEverCombo,sizeof(uint8_t),1,myAppVar);
	ti_CloseAll();
}
void loadPlayerData(){
	ti_var_t myAppVar;
	uint8_t _tempReadFileFormatVersion;
	myAppVar = ti_Open(SAVEOPTIONSNAME, "r");
	if (myAppVar==0){
		highscore=0;
		highestEverCombo=0;
		return;
	}
	ti_Read(&_tempReadFileFormatVersion,sizeof(uint8_t),1,myAppVar);
	if (_tempReadFileFormatVersion>SAVEFILEVERSION){
		drawErrorCircle();
	}else{ // Savefile may be valid
		ti_Read(&doDrawWaifu,sizeof(uint8_t),1,myAppVar);
		ti_Read(&highscore,sizeof(uint32_t),1,myAppVar);
		ti_Read(&highestEverCombo,sizeof(uint8_t),1,myAppVar);
	}
	ti_CloseAll();
}

// For teacher key
void loadGame(){
	ti_var_t myAppVar;
	uint8_t _tempReadFileFormatVersion;
	uint8_t _tempMaxPuyoForecastHold = MAXPUYOFORECAST;
	uint8_t i;
	uint8_t j;
	myAppVar = ti_Open(SAVEGAMENAME, "r");
	if (myAppVar==0){
		return;
	}
	ti_Read(&_tempReadFileFormatVersion,sizeof(uint8_t),1,myAppVar);
	if (_tempReadFileFormatVersion>TEACHERKEYVERSION){
		drawErrorCircle();
	}else{
		for (i=0;i<BOARD_WIDTH;++i){
			for (j=0;j<BOARD_HEIGHT;++j){
				ti_Read(&(puyoBoard[i][j]),sizeof(uint8_t),1,myAppVar);
			}
		}
		ti_Read(&currentScore,sizeof(uint32_t),1,myAppVar);
		ti_Read(&primaryPuyoColor,sizeof(uint8_t),1,myAppVar);
		ti_Read(&secondaryPuyoColor,sizeof(uint8_t),1,myAppVar);
		ti_Read(&_tempMaxPuyoForecastHold,sizeof(uint8_t),1,myAppVar);
		for (i=0;i<_tempMaxPuyoForecastHold;++i){
			ti_Read(&(nextPrimaryPuyoColor[i]),sizeof(uint8_t),1,myAppVar);
			ti_Read(&(nextSecondaryPuyoColor[i]),sizeof(uint8_t),1,myAppVar);
		}
	}
	ti_CloseAll();
	ti_Delete(SAVEGAMENAME);
	ti_CloseAll();
	
	// Tell player savegame was loaded.
	gfx_FillScreen(COLOR_RED);
	delay(100);
	gfx_FillScreen(COLOR_BLACK);
}
void saveGame(){
	ti_var_t myAppVar;
	uint8_t _tempFileFormatVersion=TEACHERKEYVERSION;
	uint8_t _tempMaxPuyoForecastHold = MAXPUYOFORECAST;
	uint8_t i;
	uint8_t j;
	myAppVar = ti_Open(SAVEGAMENAME, "w");
	if (myAppVar==0){
		drawErrorCircle();
		delay(300);
		return;
	}
	ti_Write(&_tempFileFormatVersion,sizeof(uint8_t),1,myAppVar);
	for (i=0;i<BOARD_WIDTH;++i){
		for (j=0;j<BOARD_HEIGHT;++j){
			ti_Write(&(puyoBoard[i][j]),sizeof(uint8_t),1,myAppVar);
		}
	}
	ti_Write(&currentScore,sizeof(uint32_t),1,myAppVar);
	ti_Write(&primaryPuyoColor,sizeof(uint8_t),1,myAppVar);
	ti_Write(&secondaryPuyoColor,sizeof(uint8_t),1,myAppVar);
	ti_Write(&_tempMaxPuyoForecastHold,sizeof(uint8_t),1,myAppVar);
	for (i=0;i<MAXPUYOFORECAST;++i){
		ti_Write(&(nextPrimaryPuyoColor[i]),sizeof(uint8_t),1,myAppVar);
		ti_Write(&(nextSecondaryPuyoColor[i]),sizeof(uint8_t),1,myAppVar);
	}
	ti_CloseAll();
}

/////////////////////////////////////////////////

uint8_t genericMenu(char* _menuName, char** _menuOptions, uint8_t _totalMenuOptions, uint8_t _startingChoice){
	uint16_t _drawStart = centerGeneric(8*(_totalMenuOptions+2),SCREEN_HEIGHT);
	uint8_t i;
	uint8_t _choice=_startingChoice;
	gfx_FillScreen(BACKGROUND_COLOR);
	gfx_SetTextFGColor(COLOR_WHITE);
	gfx_PrintStringXY(_menuName,centerStringWidth(_menuName,SCREEN_WIDTH),_drawStart);
	for (i=0;i<_totalMenuOptions;i++){
		gfx_PrintStringXY(_menuOptions[i],centerStringWidth(_menuOptions[i],SCREEN_WIDTH),_drawStart+FONT_HEIGHT*(i+2));
	}
	gfx_SetTextFGColor(COLOR_GREEN);
	gfx_PrintStringXY(_menuOptions[_choice],centerStringWidth(_menuOptions[_choice],SCREEN_WIDTH),_drawStart+FONT_HEIGHT*(_choice+2));
	while(1){
		controlsStart();
		if (upButtonPressed() || downButtonPressed()){
			gfx_SetTextFGColor(COLOR_WHITE);
			gfx_PrintStringXY(_menuOptions[_choice],centerStringWidth(_menuOptions[_choice],SCREEN_WIDTH),_drawStart+FONT_HEIGHT*(_choice+2));
			if (downButtonPressed()){
				if (_choice==_totalMenuOptions-1){
					_choice=0;
				}else{
					_choice+=1;
				}
			}else if (upButtonPressed()){
				if (_choice==0){
					_choice = _totalMenuOptions-1;
				}else{
					_choice-=1;
				}
			}
			gfx_SetTextFGColor(COLOR_GREEN);
			gfx_PrintStringXY(_menuOptions[_choice],centerStringWidth(_menuOptions[_choice],SCREEN_WIDTH),_drawStart+FONT_HEIGHT*(_choice+2));
		}
		if (clockwiseButtonPressed()){
			return _choice;
		}
	}
	return 0;
}

uint8_t optionsMenu(){
	uint8_t _chosenMenuOption=0;
	while (1){
		char* _menuOptions[]={"Back",NULL,"Delete High","Delete Temp Save"};
		_menuOptions[1] = malloc(strlen("NOT AMITIE")+1);
		if (doDrawWaifu){
			strcpy(_menuOptions[1],"Amitie");
		}else{
			strcpy(_menuOptions[1],"NOT Amitie");
		}
		_chosenMenuOption = genericMenu("Options",&(_menuOptions[0]),4,_chosenMenuOption);
		free(_menuOptions[1]);
		if (_chosenMenuOption==0){
			return 0;
		}else if (_chosenMenuOption==1){
			doDrawWaifu=!doDrawWaifu;
		}else if (_chosenMenuOption==2){
			if (yesOrNo("Really delete high scores and settings?",0)){
				ti_Delete(SAVEOPTIONSNAME);
				ti_CloseAll();
				return 1;
			}
		}else if (_chosenMenuOption==3){
			if (yesOrNo("Really delete temp savefile?",0)){
				ti_Delete(SAVEGAMENAME);
				ti_CloseAll();
				return 1;
			}
		}
	}
}

// Return 1 to quit
char titleScreen(){
	uint8_t _chosenMenuOption=0;
	while (1){
		char* _menuOptions[] = {"Play","Options","Not Play"};
		_chosenMenuOption = genericMenu("Puyo Puyo 84",&(_menuOptions[0]),3,_chosenMenuOption);
		if (_chosenMenuOption==0){
			loadGame();
			return 0;
		}else if (_chosenMenuOption==1){
			if (optionsMenu()==1){
				return 1;
			}
			savePlayerData();
			continue;
		}else if (_chosenMenuOption==2){
			return 1;
		}
	}
}

//////////////////////////////////////////////////////////
// Because init() is already taken.
void initPuyo84(){
	uint8_t i;

	// Init board memory
	clearPuyoBoard();

	// Init color indexes
	puyoColors[PUYO_NONE] = COLOR_WHITE;
	puyoColors[PUYO_RED] = COLOR_RED;
	puyoColors[PUYO_GREEN] = COLOR_GREEN;
	puyoColors[PUYO_BLUE] = COLOR_BLUE;
	puyoColors[PUYO_YELLOW] = COLOR_PINK;
	if (MAXPUYOINDEX>=PUYO_PURPLE+1){
		puyoColors[PUYO_PURPLE] = COLOR_YELLOW;
	}

	// Init graphics
	gfx_Begin();

	// Modify palette
	logo_gfx_pal[COLOR_RED] = gfx_RGBTo1555(255,0,0); // Red
	logo_gfx_pal[COLOR_BLUE] = gfx_RGBTo1555(0,0,255); // Blue
	logo_gfx_pal[COLOR_GREEN] = gfx_RGBTo1555(0,190,0); // Green
	logo_gfx_pal[COLOR_YELLOW] = gfx_RGBTo1555(174,174,0); // Yellow
	logo_gfx_pal[COLOR_PURPLE] = gfx_RGBTo1555(150,0,255); // Purple
	logo_gfx_pal[COLOR_PINK] = gfx_RGBTo1555(220,0,170); // Pink
	logo_gfx_pal[COLOR_WHITE] = gfx_RGBTo1555(255,255,255); // White
	logo_gfx_pal[COLOR_GRAY] = gfx_RGBTo1555(212,208,200); // Gray
	logo_gfx_pal[COLOR_BLACK] = gfx_RGBTo1555(0,0,0); // Black

	// Set custom palette
	gfx_SetPalette(logo_gfx_pal, sizeof_logo_gfx_pal, 0);

	// Reset color variable
	gfx_SetColor(COLOR_WHITE);
	currentColor = COLOR_WHITE;

	// Text color
	gfx_SetTextFGColor(COLOR_GREEN);

	// Init random numbers
	srand(rtc_Time());

	// Init puyo forecast
	for (i=0;i<MAXPUYOFORECAST;i++){
		_generateNewPuyoColor(&(nextPrimaryPuyoColor[i]));
		_generateNewPuyoColor(&(nextSecondaryPuyoColor[i]));
	}

	// Init currnet puyos
	generateNewPuyos();

	// Init controls
	kb_Scan();
	controlsStart();

	// Close files that were already open.
	ti_CloseAll();
	// Should probably be after the file closing
	loadPlayerData();

	//////////////////////////////////////////////////////////////////

	// TEST PATTERNS
	/*
	puyoBoard[0][BOARD_HEIGHT-3]=PUYO_RED;
	puyoBoard[0][BOARD_HEIGHT-2]=PUYO_RED;
	puyoBoard[1][BOARD_HEIGHT-2]=PUYO_RED;
	puyoBoard[1][BOARD_HEIGHT-1]=PUYO_RED;
	*/
	/*
	puyoBoard[0][BOARD_HEIGHT-1]=PUYO_RED;
	puyoBoard[0][BOARD_HEIGHT-2]=PUYO_RED;

	puyoBoard[1][BOARD_HEIGHT-1]=PUYO_RED;

	puyoBoard[2][BOARD_HEIGHT-1]=PUYO_RED;
	puyoBoard[2][BOARD_HEIGHT-2]=PUYO_RED;
	*/
	/*
	puyoBoard[0][BOARD_HEIGHT-1]=PUYO_RED;
	puyoBoard[1][BOARD_HEIGHT-1]=PUYO_RED;
	puyoBoard[1][BOARD_HEIGHT-2]=PUYO_RED;
	puyoBoard[2][BOARD_HEIGHT-1]=PUYO_RED;
	*/
	/*
	puyoBoard[BOARD_WIDTH-1][BOARD_HEIGHT-1]=PUYO_RED;
	puyoBoard[BOARD_WIDTH-1][BOARD_HEIGHT-2]=PUYO_RED;
	puyoBoard[BOARD_WIDTH-1][BOARD_HEIGHT-3]=PUYO_RED;
	puyoBoard[BOARD_WIDTH-1][BOARD_HEIGHT-4]=PUYO_RED;
	*/
	/*
	puyoBoard[BOARD_WIDTH-3][BOARD_HEIGHT-1]=PUYO_RED;
	puyoBoard[BOARD_WIDTH-2][BOARD_HEIGHT-1]=PUYO_RED;
	puyoBoard[BOARD_WIDTH-1][BOARD_HEIGHT-1]=PUYO_RED;
	puyoBoard[BOARD_WIDTH-1][BOARD_HEIGHT-2]=PUYO_RED;
	puyoBoard[BOARD_WIDTH-1][BOARD_HEIGHT-3]=PUYO_RED;
	*/
}
void main(void) {
	int8_t i;
	uint8_t j;

	initPuyo84();

	if (titleScreen()){
		gfx_End();
		return;
	}
	gfx_SetTextFGColor(COLOR_GREEN);

	// Start main game
	redrawEverything();
	while (gameIsRunning){
		for (i=0;i<PUYO_DROP_SPEED;++i){
			controlsStart();
			if (quitButtonPressed()){
				gameIsRunning=0;
				break;
			}
			if (teacherKeyPressed()){
				saveGame();
				gameIsRunning=0;
				break;
			}
			if (clockwiseButtonPressed()){
				hideBothPuyos();
				spinPuyo(1);
				drawBothPuyos();
			}
			if (counterclockwiseButtonPressed()){
				hideBothPuyos();
				spinPuyo(-1);
				drawBothPuyos();
			}
			if (upButtonPressed()){
				hideBothPuyos();
				myPuyoY=1;
				drawBothPuyos();
			}
			if (rightButtonPressed()){
				if (!(isOnTopOfAnything(myPuyoX+1,myPuyoY) || isOnTopOfAnything(getSecondaryPuyoX(myPuyoX+1,secondaryPuyoOrientation),getSecondaryPuyoY(myPuyoY,secondaryPuyoOrientation)))){
					hideBothPuyos();
					myPuyoX++;
					drawBothPuyos();
				}
			}
			if (leftButtonPressed()){
				if (!(isOnTopOfAnything(myPuyoX-1,myPuyoY) || isOnTopOfAnything(getSecondaryPuyoX(myPuyoX-1,secondaryPuyoOrientation),getSecondaryPuyoY(myPuyoY,secondaryPuyoOrientation)))){
					hideBothPuyos();
					myPuyoX--;
					drawBothPuyos();
				}
			}
			if (pauseButtonPressed()){
				goodChangeColor(COLOR_BLACK);
				gfx_FillRectangle(0,0,SCREEN_WIDTH,SCREEN_HEIGHT);
				
				gfx_PrintStringXY(PAUSEDSTRING, centerStringWidth(PAUSEDSTRING,SCREEN_WIDTH), centerGeneric(FONT_HEIGHT,SCREEN_HEIGHT));
				while(1){
					controlsStart();
					if (pauseButtonPressed()){
						break;
					}
					if (quitButtonPressed()){
						gameIsRunning=0;
						break;
					}
					delay(KEYBOARD_POLL_WAIT);
				}
				redrawEverything();
			}
			if (downButtonHeld()){
				delay(HOLD_DOWN_DELAY);
				break;
			}
			delay(KEYBOARD_POLL_WAIT);
		}

		if (gameIsRunning){
			// Will the puyos hit something if we go down one more?
			if (isOnTopOfAnything(myPuyoX,myPuyoY+1) || isOnTopOfAnything(getSecondaryPuyoX(myPuyoX,secondaryPuyoOrientation),getSecondaryPuyoY(myPuyoY,secondaryPuyoOrientation)+1)){
				lockPuyos();
				if (playerIsDead()){
					if (currentScore>highscore){
						controlsStart();
						highscore=currentScore;
						savePlayerData();
						for (;!quitButtonPressed();i=!i){
							controlsStart();
							goodChangeColor(BACKGROUND_COLOR);
							gfx_FillRectangle((SCREEN_WIDTH-gfx_GetStringWidth(NEWHIMESSAGE))/(double)2-3,(SCREEN_HEIGHT-FONT_HEIGHT)/(double)2-3,gfx_GetStringWidth(NEWHIMESSAGE)+6,14);
							if (i%2==0){
								gfx_PrintStringXY(NEWHIMESSAGE,(SCREEN_WIDTH-gfx_GetStringWidth(NEWHIMESSAGE))/(double)2,(SCREEN_HEIGHT-FONT_HEIGHT)/(double)2);
							}
							delay(200);
						}
						
					}
					// TODO - Draw and kill
					controlsStart();
					gfx_PrintStringXY("is kill",0,0);
					while (!quitButtonPressed()){
						controlsStart();
						gameIsRunning=0;
					}
				}
				generateNewPuyos();
				redrawPuyoForecast();
				drawBothPuyos();
			}else{ // Won't hit anything, fall as normal.
				hideBothPuyos();
				myPuyoY++;
				drawBothPuyos();
			}
		}

	}

	gfx_End();
}

