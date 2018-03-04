/*
	--------------------------------------
	 Program Name: Puyo Puyo 84
	 Author: MyLegGuy
	 License: MIT (See LICENSE)
	 Description: Puyo Puyo clone for Ti-84 Plus CE
	--------------------------------------

	The license is stupid because these calculator games get reuploaded a lot, I think. Maybe I'm just stupid.
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

// Sprite data
#include "gfx/logo_gfx.h"

//////////////////////////////////////////////////////////
// Define
//////////////////////////////////////////////////////////

#define HOLD_DOWN_DELAY 75

#define SINGLE_POP_DELAY 500

#define MINPUYOMATCH 4

#define USEGRAPHICSCLIPPING 1

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

#define PAUSEDSTRING "Pause"

#define BOARD_WIDTH 6
#define BOARD_HEIGHT 12

#define PUYO_RADIUS 9

#define KEYBOARD_POLL_WAIT 10
// < wanted time > / KEYBOARD_POLL_WAIT
// Right now I'm doing 500
#define PUYO_DROP_SPEED 50

// 240 - BOARD_X_OFFSET - PUYO_RADIUS*BOARD_HEIGHT*2
#if PUYO_RADIUS==9
	#define BOARD_Y_OFFSET 14
#elif PUYO_RADIUS==8
	#define BOARD_Y_OFFSET 34
#else
	#define BOARD_Y_OFFSET 0
	#warning Please caclulate BOARD_Y_OFFSET
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
#if 1 // Normal controls
	#define CONDITION_CLOCKWISE_BUTTON wasJustPressed(1,kb_Mode)
	#define CONDITION_COUNTERCLOCKWISE_BUTTON wasJustPressed(1,kb_2nd)
#else // Emulator controls
	#define CONDITION_CLOCKWISE_BUTTON wasJustPressed(2,kb_Store)
	#define CONDITION_COUNTERCLOCKWISE_BUTTON wasJustPressed(4,kb_2)
#endif
#define CONDITION_PAUSE_BUTTON wasJustPressed(1,kb_Del)

//////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////

uint8_t puyoBoard[BOARD_WIDTH][BOARD_HEIGHT];

uint8_t myPuyoX=3;
uint8_t myPuyoY=1;

uint8_t currentColor=255;

uint8_t primaryPuyoColor=PUYO_RED;
uint8_t secondaryPuyoColor=PUYO_BLUE;

uint8_t puyoColors[MAXPUYOINDEX];

uint8_t secondaryPuyoOrientation=PUYO_ORIENTATION_UP;

uint8_t gameIsRunning=1;

kb_key_t lastKeyboardData[8];

//////////////////////////////////////////////////////////

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

void goodChangeColor(uint8_t _color){
	if (_color!=currentColor){
		gfx_SetColor(_color);
		currentColor = _color;
	}
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
	goodDrawCircle(BOARD_X_OFFSET+_x*PUYO_RADIUS*2,BOARD_Y_OFFSET+_y*PUYO_RADIUS*2,PUYO_RADIUS);
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
	goodChangeColor(BOARD_BACKGROUND_COLOR);
	#if USEGRAPHICSCLIPPING
		gfx_FillRectangle(BOARD_X_OFFSET+_x*PUYO_RADIUS*2,BOARD_Y_OFFSET+_y*PUYO_RADIUS*2,PUYO_RADIUS*2+1,PUYO_RADIUS*2+1);
	#else
		gfx_FillRectangle_NoClip(BOARD_X_OFFSET+_x*PUYO_RADIUS*2,BOARD_Y_OFFSET+_y*PUYO_RADIUS*2,PUYO_RADIUS*2+1,PUYO_RADIUS*2+1);
	#endif
}

void drawBoardBackground(){
	goodChangeColor(BOARD_BACKGROUND_COLOR);
	gfx_FillRectangle(BOARD_X_OFFSET,BOARD_Y_OFFSET,PUYO_RADIUS*BOARD_WIDTH*2+1,PUYO_RADIUS*BOARD_HEIGHT*2+1);
}
// < number of puyos drawn > / 2 = amount to add ?
void drawBoardBoarder(){
	goodChangeColor(BOARD_BOARDER_COLOR);
	gfx_Rectangle(BOARD_X_OFFSET-1,BOARD_Y_OFFSET-1,BOARD_WIDTH*PUYO_RADIUS*2+3,BOARD_HEIGHT*PUYO_RADIUS*2+3);
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
void redrawEverything(){
	gfx_FillScreen(COLOR_BLACK);
	gfx_Sprite(AmitieSmall, SCREEN_WIDTH-AmitieSmall_width, SCREEN_HEIGHT-AmitieSmall_height);
	redrawPuyoBoard();
}
void hideBothPuyos(){
	clearPuyoSpot(myPuyoX,myPuyoY);
	clearPuyoSpot(getSecondaryPuyoX(myPuyoX,secondaryPuyoOrientation),getSecondaryPuyoY(myPuyoY,secondaryPuyoOrientation));
}
void drawBothPuyos(){
	drawPuyo(myPuyoX,myPuyoY,puyoColors[primaryPuyoColor]);
	drawPuyo(getSecondaryPuyoX(myPuyoX,secondaryPuyoOrientation),getSecondaryPuyoY(myPuyoY,secondaryPuyoOrientation),puyoColors[secondaryPuyoColor]);
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
		if (_possibleNewOrientation!=PUYO_ORIENTATION_UNDEFINED){ // This variable is set to PUYO_ORIENTATION_UNDEFINED if we got primary puyo shifting to work. This code happens if primary puyo shifting doesn't work.
			if (_direction==1){
				if (isOnTopOfAnything(getSecondaryPuyoX(myPuyoX,_possibleNewOrientation),getSecondaryPuyoY(myPuyoY,_possibleNewOrientation))){
					myPuyoY--;
				}
				secondaryPuyoOrientation = PUYO_ORIENTATION_DOWN;
			}else if (_direction==-1){
				secondaryPuyoOrientation = PUYO_ORIENTATION_UP;
			}
		}
	}else{
		secondaryPuyoOrientation = _possibleNewOrientation;
	}
}


// Don't call directly
void _generateNewPuyoColors(){
	primaryPuyoColor = randInt(1,MAXPUYOINDEX-1);
	secondaryPuyoColor = randInt(1,MAXPUYOINDEX-1);
}
void generateNewPuyos(){
	myPuyoX=3;
	myPuyoY=1;
	_generateNewPuyoColors();
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
int8_t popPuyos(uint8_t _isFirstTime){
	uint8_t _specificPuyoIDs[BOARD_WIDTH][BOARD_HEIGHT]={0};
	uint8_t _comboIDLengths[BOARD_WIDTH*BOARD_HEIGHT+1]; // Don't zero this array. Also don't make this array bigger than a signed byte can hold
	uint8_t _nextComboID = 1;
	uint8_t i;
	uint8_t j;
	int8_t k;
	uint8_t _puyoWasPopped=0;

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
			if (_puyoWasPopped==0 && _isFirstTime==0){
				delay(SINGLE_POP_DELAY); // If this is the second combo, delay beore popping more because we want the player to see the board after their first chain.
			}
			_puyoWasPopped=1;
			// Search board for puyos with matching combo IDs
			for (j=0;j<BOARD_HEIGHT;j++){
				for (i=0;i<BOARD_WIDTH;i++){
					if (_specificPuyoIDs[i][j]==k){ // If we found one
						drawPuyo(i,j,COLOR_POPPING_PUYO); // Make it pink
						puyoBoard[i][j]=PUYO_NONE; // Pop it
					}
				}
			}
		}
	}

	if (_puyoWasPopped){
		delay(SINGLE_POP_DELAY);
		fellPuyos();
		redrawPuyoBoard();
		return 1;
	}
	return 0;
}

// Lock puyo into place and do combos
void lockPuyos(){
	if (secondaryPuyoOrientation==PUYO_ORIENTATION_DOWN){ // If the secondary puyo is down, we need it to fall first.
		_fallAndWritePuyoData(getSecondaryPuyoX(myPuyoX,secondaryPuyoOrientation),getSecondaryPuyoY(myPuyoY,secondaryPuyoOrientation),secondaryPuyoColor);
		_fallAndWritePuyoData(myPuyoX,myPuyoY,primaryPuyoColor);
	}else{ // Otherwise, have the first puyo fall first.
		_fallAndWritePuyoData(myPuyoX,myPuyoY,primaryPuyoColor);
		_fallAndWritePuyoData(getSecondaryPuyoX(myPuyoX,secondaryPuyoOrientation),getSecondaryPuyoY(myPuyoY,secondaryPuyoOrientation),secondaryPuyoColor);
	}
	if (popPuyos(1)==1){
		while (popPuyos(0)==1);
	}
}

//////////////////////////////////////////////////////////
// Because init() is already taken.
void initPuyo84(){
	// Init board memory
	clearPuyoBoard();

	// Init colors
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

	// Reset color variable
	gfx_SetColor(COLOR_WHITE);
	currentColor = COLOR_WHITE;

	srand(rtc_Time());

	generateNewPuyos();

	// Modify palette
	logo_gfx_pal[COLOR_RED] = gfx_RGBTo1555(255,0,0); // Red
	logo_gfx_pal[COLOR_BLUE] = gfx_RGBTo1555(0,0,255); // Blue
	logo_gfx_pal[COLOR_GREEN] = gfx_RGBTo1555(0,190,0); // Green
	logo_gfx_pal[COLOR_YELLOW] = gfx_RGBTo1555(174,174,0); // Yellow TODO - I don't like this.
	logo_gfx_pal[COLOR_PURPLE] = gfx_RGBTo1555(150,0,255); // Purple
	logo_gfx_pal[COLOR_PINK] = gfx_RGBTo1555(220,0,170); // Pink
	logo_gfx_pal[COLOR_WHITE] = gfx_RGBTo1555(255,255,255); // White
	logo_gfx_pal[COLOR_GRAY] = gfx_RGBTo1555(212,208,200); // Gray
	logo_gfx_pal[COLOR_BLACK] = gfx_RGBTo1555(0,0,0); // Black
	// Set palette
	gfx_SetPalette(logo_gfx_pal, sizeof_logo_gfx_pal, 0);

	kb_Scan();
	controlsStart();

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

	puyoBoard[BOARD_WIDTH-1][BOARD_HEIGHT-1]=PUYO_RED;
	puyoBoard[BOARD_WIDTH-1][BOARD_HEIGHT-2]=PUYO_RED;
	puyoBoard[BOARD_WIDTH-1][BOARD_HEIGHT-3]=PUYO_RED;
	puyoBoard[BOARD_WIDTH-1][BOARD_HEIGHT-4]=PUYO_RED;
	*/

	puyoBoard[BOARD_WIDTH-3][BOARD_HEIGHT-1]=PUYO_RED;
	puyoBoard[BOARD_WIDTH-2][BOARD_HEIGHT-1]=PUYO_RED;
	puyoBoard[BOARD_WIDTH-1][BOARD_HEIGHT-1]=PUYO_RED;
	puyoBoard[BOARD_WIDTH-1][BOARD_HEIGHT-2]=PUYO_RED;
	puyoBoard[BOARD_WIDTH-1][BOARD_HEIGHT-3]=PUYO_RED;
}

void main(void) {
	uint8_t i;
	uint8_t j;

	initPuyo84();

	redrawEverything();

	//for (i=0;i<6;++i){
	//	for (j=0;j<12;++j){
	//		goodDrawCircle(BOARD_X_OFFSET+i*PUYO_RADIUS*2,BOARD_Y_OFFSET+j*PUYO_RADIUS*2,PUYO_RADIUS);
	//	}
	//}
	//goodChangeColor(gfx_red);
	//gfx_FillRectangle(0,0,PUYO_RADIUS*2+1,PUYO_RADIUS*2+1);
	//goodChangeColor(gfx_blue);
	//gfx_FillCircle(9,9,PUYO_RADIUS);

	drawBothPuyos();
	while (gameIsRunning){
		for (i=0;i<PUYO_DROP_SPEED;++i){
			controlsStart();
			if (quitButtonPressed()){
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
				gfx_SetTextFGColor(COLOR_GREEN);
				gfx_PrintStringXY(PAUSEDSTRING, (SCREEN_WIDTH-gfx_GetStringWidth(PAUSEDSTRING))/2, (SCREEN_HEIGHT-8)/2); // 8 is default font height?
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
			if (downButtonPressed()){
				delay(HOLD_DOWN_DELAY);
				break;
			}
			delay(KEYBOARD_POLL_WAIT);
		}

		if (gameIsRunning){
			// Will the puyos hit something if we go down one more?
			if (isOnTopOfAnything(myPuyoX,myPuyoY+1) || isOnTopOfAnything(getSecondaryPuyoX(myPuyoX,secondaryPuyoOrientation),getSecondaryPuyoY(myPuyoY,secondaryPuyoOrientation)+1)){
				// TODO - Fell puyos
				lockPuyos();
				generateNewPuyos();
			}else{ // Won't hit anything, fall as normal.
				hideBothPuyos();
				myPuyoY++;
				drawBothPuyos();
			}
		}

	}

	//drawBothPuyos();
	///* Wait for a key press */
	//while (!os_GetCSC()){
	//	// Increment here
	//	hideBothPuyos();
	//	myPuyoY++;
	//	drawBothPuyos();
	//	while (!os_GetCSC());
	//	delay(2000);
	//}
	gfx_End();
}

