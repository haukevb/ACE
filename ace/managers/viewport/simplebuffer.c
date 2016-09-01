#include <ace/managers/viewport/simplebuffer.h>

tSimpleBufferManager *simpleBufferCreate(tVPort *pVPort, UWORD uwBoundWidth, UWORD uwBoundHeight) {
	tCopList *pCopList;
	tSimpleBufferManager *pManager;

	logBlockBegin(
		"simpleBufferManagerCreate(pVPort: %p, uwBoundWidth: %u, uwBoundHeight: %u)",
		pVPort, uwBoundWidth, uwBoundHeight
	);
	
	pManager = memAllocFast(sizeof(tSimpleBufferManager));
	logWrite("Addr: %p\n", pManager);
	logWrite("Bounds: %ux%u\n", uwBoundWidth, uwBoundHeight);
	
	// Struct init
	pManager->sCommon.pNext = 0;
	pManager->sCommon.process = (tVpManagerFn)simpleBufferProcess;
	pManager->sCommon.destroy = (tVpManagerFn)simpleBufferDestroy;
	pManager->sCommon.pVPort = pVPort;
	pManager->sCommon.ubId = VPM_SCROLL;
		
	// Buffer bitmap
	tBitMap *pBuffer = bitmapCreate(
		uwBoundWidth, uwBoundHeight,
		pVPort->ubBPP, BMF_CLEAR
	);
	if(!pBuffer) {
		logWrite("ERR: Can't alloc buffer bitmap!\n");
		logBlockEnd("simpleBufferManagerCreate()");
		return 0;
	}
	
	// Find camera manager, create if not exists
	pManager->pCameraManager = (tCameraManager*)vPortGetManager(pVPort, VPM_CAMERA);
	if(!pManager->pCameraManager)
		pManager->pCameraManager = cameraCreate(pVPort, 0, 0, uwBoundWidth, uwBoundHeight);
	
	pCopList = pVPort->pView->pCopList;
	// CopBlock contains: bitplanes + shiftX
	pManager->pCopBlock = copBlockCreate(
		pCopList,
		2*pVPort->ubBPP + 5, // Shift + 2 ddf + 2 modulos + 2*bpp*bpladdr
		0,
		pVPort->uwOffsY
	);
	
	simpleBufferSetBitmap(pManager, pBuffer);
	
	// Add manager to VPort
	vPortAddManager(pVPort, (tVpManager*)pManager);
	logBlockEnd("simpleBufferManagerCreate()");
	return pManager;
}

void simpleBufferSetBitmap(tSimpleBufferManager *pManager, tBitMap *pBitMap) {
	UWORD uwModulo;
	tCopList *pCopList;
	tCopBlock *pBlock;
	UBYTE i;
	ULONG ulPlaneAddr;
	
	logBlockBegin(
		"simpleBufferSetBitmap(pManager: %p, pBitMap: %p)",
		pManager, pBitMap
	);
	
	if(pManager->pBuffer && pManager->pBuffer->Depth != pBitMap->Depth)
		logWrite("ERR: buffer bitmaps differ in BPP!\n");
	
	pManager->uBfrBounds.sUwCoord.uwX = pBitMap->BytesPerRow << 3;
	pManager->uBfrBounds.sUwCoord.uwY = pBitMap->Rows;
	pManager->pBuffer = pBitMap;
	uwModulo = pBitMap->BytesPerRow - (pManager->sCommon.pVPort->uwWidth >> 3);
	
	// Copperlist - regen bitplane ptrs, update shift
	pBlock = pManager->pCopBlock;
	pCopList = pManager->sCommon.pVPort->pView->pCopList;
	pManager->pCopBlock->uwCurrCount = 0; // Rewind to beginning
	copMove(pCopList, pBlock, &custom.ddfstop, 0x00D0);     // Data fetch
	copMove(pCopList, pBlock, &custom.ddfstrt, 0x0030);
	copMove(pCopList, pBlock, &custom.bpl1mod, uwModulo-1); // Bitplane modulo
	copMove(pCopList, pBlock, &custom.bpl2mod, uwModulo-1);
	copMove(pCopList, pBlock, &custom.bplcon1, 0);          // Shift: 0
	for (i = 0; i != pManager->sCommon.pVPort->ubBPP; ++i) {
		ulPlaneAddr = (ULONG)pManager->pBuffer->Planes[i];
		copMove(pCopList, pBlock, &pBplPtrs[i].uwHi, ulPlaneAddr >> 16);
		copMove(pCopList, pBlock, &pBplPtrs[i].uwLo, ulPlaneAddr & 0xFFFF);
	}
	
	logBlockEnd("simplebufferSetBitmap()");
}

void simpleBufferDestroy(tSimpleBufferManager *pManager) {
	logWrite("Destroying bitmap...\n");
	bitmapDestroy(pManager->pBuffer);
	logWrite("Freeing mem...\n");
	memFree(pManager, sizeof(tSimpleBufferManager));
	logWrite("Done\n");
}

void simpleBufferProcess(tSimpleBufferManager *pManager) {
	UBYTE i;
	UWORD uwShift;
	ULONG ulBplAdd;
	ULONG ulPlaneAddr;
	tCameraManager *pCameraManager;
	tCopList *pCopList;
	
	pCameraManager = pManager->pCameraManager;
	pCopList = pManager->sCommon.pVPort->pView->pCopList;
	
	// Calculate X movement
	uwShift = 15-(pCameraManager->uPos.sUwCoord.uwX & 0xF);
	uwShift = (uwShift << 4) | uwShift;
	ulBplAdd = (pCameraManager->uPos.sUwCoord.uwX >> 4) << 1;
	
	// Calculate Y movement
	ulBplAdd += pManager->pBuffer->BytesPerRow*pCameraManager->uPos.sUwCoord.uwY;
	
	// Update (rewrite) copperlist
	pManager->pCopBlock->uwCurrCount = 4; // Rewind to shift instruction pos
	copMove(pCopList, pManager->pCopBlock, &custom.bplcon1, uwShift);
	for(i = 0; i != pManager->pBuffer->Depth; ++i) {
		ulPlaneAddr = ((ULONG)pManager->pBuffer->Planes[i]) + ulBplAdd;
		copMove(pCopList, pManager->pCopBlock, &pBplPtrs[i].uwHi, ulPlaneAddr >> 16);
		copMove(pCopList, pManager->pCopBlock, &pBplPtrs[i].uwLo, ulPlaneAddr & 0xFFFF);
	}
	
}
