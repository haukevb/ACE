#include "test/blitsmalldest.h"

#include <ace/utils/extview.h>
#include <ace/utils/palette.h>
#include <ace/managers/game.h>
#include <ace/managers/blit.h>
#include <ace/managers/key.h>
#include <ace/managers/joy.h>
#include <ace/managers/viewport/simplebuffer.h>

#include "menu/menu.h"

static tView *s_pTestBlitView;
static tVPort *s_pTestBlitVPort;
static tSimpleBufferManager *s_pTestBlitBfr;

static tBitMap *s_pRefBitmap;
static tBitMap *s_pDstBitmap;
UBYTE ubFrameIdx;

void prepareRefBitmap() {
	// s_pRefBitmap = bitmapCreateFromFile("data/blitToSmall.bm");
	UBYTE ubBlockWidth = 32;
	UBYTE ubBlockHeight = 32;
	UBYTE ubImageCount = 16;
	UBYTE i;
	
	s_pRefBitmap = bitmapCreate(ubBlockWidth+ubImageCount, ubBlockHeight*ubImageCount, WINDOW_SCREEN_BPP, 0);
	
	for(i = 0; i != ubImageCount; ++i) {
		blitRect(s_pRefBitmap, 0+i, 0+ubBlockHeight*i, ubBlockWidth, ubBlockHeight, 2);   // fill
		blitRect(s_pRefBitmap, 0+i, 0+ubBlockHeight*i, ubBlockWidth, 1, 1);               // top
		blitRect(s_pRefBitmap, 0+i, ubBlockHeight+ubBlockHeight*i-1, ubBlockWidth, 1, 1); // bottom
		blitRect(s_pRefBitmap, 0+i, 0+ubBlockHeight*i, 1, ubBlockHeight, 1);              // left
		blitRect(s_pRefBitmap, ubBlockWidth+i, 0+ubBlockHeight*i, 1, ubBlockHeight, 1);   // right
	}

}

void gsTestBlitSmallDestCreate(void) {
	// Prepare view & viewport
	s_pTestBlitView = viewCreate(V_GLOBAL_CLUT);
	s_pTestBlitVPort = vPortCreate(s_pTestBlitView, WINDOW_SCREEN_WIDTH, WINDOW_SCREEN_HEIGHT, WINDOW_SCREEN_BPP, 0);
	s_pTestBlitBfr = simpleBufferCreate(s_pTestBlitVPort, WINDOW_SCREEN_WIDTH, WINDOW_SCREEN_HEIGHT, BMF_CLEAR);
	paletteLoad("data/blitToSmall.plt", s_pTestBlitVPort->pPalette, 1 << WINDOW_SCREEN_BPP);
	
	s_pDstBitmap = bitmapCreate(32, 32, WINDOW_SCREEN_BPP, 0);
	prepareRefBitmap();
	ubFrameIdx = 0;
	
	// Display view with its viewports
	viewLoad(s_pTestBlitView);
}

void gsTestBlitSmallDestLoop(void) {
	static BYTE bSpeedX = 0, bSpeedY = 0;
	BYTE bUpdate = 0;
	
	if (keyUse(KEY_ESCAPE)) {
		gameChangeState(gsMenuCreate, gsMenuLoop, gsMenuDestroy);
		return;
	}
	if(keyUse(KEY_RIGHT))
		bUpdate += 1;
	if(keyUse(KEY_LEFT))
		bUpdate -=1;
	
	if(bUpdate && ubFrameIdx + bUpdate > -1 && ubFrameIdx + bUpdate < 16) {
		ubFrameIdx += bUpdate;
		blitRect(s_pTestBlitBfr->pBuffer, 0, 0, WINDOW_SCREEN_WIDTH, WINDOW_SCREEN_HEIGHT, 0);
		blitCopy(s_pRefBitmap, ubFrameIdx, ubFrameIdx*32, s_pDstBitmap, 0, 0, 32, 32, 0xCA, 0xFF);
		blitCopyAligned(s_pDstBitmap, 0, 0, s_pTestBlitBfr->pBuffer, 16, 16, 32, 32);
	}
	
	WaitTOF();
	
}

void gsTestBlitSmallDestDestroy(void) {
	// Destroy buffer, view & viewport
	viewDestroy(s_pTestBlitView);
	
	bitmapDestroy(s_pDstBitmap);
	bitmapDestroy(s_pRefBitmap);
}