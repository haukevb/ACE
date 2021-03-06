#include <ace/managers/viewport/camera.h>
#include <ace/macros.h>

tCameraManager *cameraCreate(tVPort *pVPort, UWORD uwPosX, UWORD uwPosY, UWORD uwMaxX, UWORD uwMaxY) {
	logBlockBegin("cameraCreate(pVPort: %p, uwPosX: %u, uwPosY: %u, uwMaxX: %u, uwMaxY: %u)", pVPort, uwPosX, uwPosY, uwMaxX, uwMaxY);
	tCameraManager *pManager;
	
	pManager = memAllocFast(sizeof(tCameraManager));
	logWrite("Addr: %p\n", pManager);
	pManager->sCommon.pNext = 0;
	pManager->sCommon.process = (tVpManagerFn)cameraProcess;
	pManager->sCommon.destroy = (tVpManagerFn)cameraDestroy;
	pManager->sCommon.pVPort = pVPort;
	pManager->sCommon.ubId = VPM_CAMERA;
	
	cameraReset(pManager, uwPosX, uwPosY, uwMaxX, uwMaxY);
	
	vPortAddManager(pVPort, (tVpManager*)pManager);
	logBlockEnd("cameraCreate()");
	return pManager;
}

void cameraDestroy(tCameraManager *pManager) {
	logWrite("cameraManagerDestroy...");
	memFree(pManager, sizeof(tCameraManager));
	logWrite("OK! \n");
}

inline void cameraProcess(tCameraManager *pManager) {
	pManager->uLastPos.ulYX = pManager->uPos.ulYX;
}

void cameraReset(tCameraManager *pManager, UWORD uwStartX, UWORD uwStartY, UWORD uwWidth, UWORD uwHeight) {
	logBlockBegin("cameraReset(pManager: %p, uwStartX: %u, uwStartY: %u, uwWidth: %u, uwHeight: %u)", pManager, uwStartX, uwStartY, uwWidth, uwHeight);
	
	pManager->uPos.sUwCoord.uwX = uwStartX;
	pManager->uPos.sUwCoord.uwY = uwStartY;
	pManager->uLastPos.sUwCoord.uwX = uwStartX;
	pManager->uLastPos.sUwCoord.uwY = uwStartY;
	
	// Max camera coords based on viewport size
	pManager->uMaxPos.sUwCoord.uwX = uwWidth - pManager->sCommon.pVPort->uwWidth;
	pManager->uMaxPos.sUwCoord.uwY = uwHeight - pManager->sCommon.pVPort->uwHeight;	
	logWrite("Camera max coord: %u,%u\n", pManager->uMaxPos.sUwCoord.uwX, pManager->uMaxPos.sUwCoord.uwY);
	
	logBlockEnd("cameraReset()");
}

inline void cameraSetCoord(tCameraManager *pManager, UWORD uwX, UWORD uwY) {
	pManager->uPos.sUwCoord.uwX = uwX;
	pManager->uPos.sUwCoord.uwY = uwY;
	// logWrite("New camera pos: %u,%u\n", uwX, uwY);
}

void cameraMove(tCameraManager *pManager, WORD wX, WORD wY) {
	WORD wTmp;
	
	pManager->uPos.sUwCoord.uwX = clamp(pManager->uPos.sUwCoord.uwX+wX, 0, pManager->uMaxPos.sUwCoord.uwX);
	pManager->uPos.sUwCoord.uwY = clamp(pManager->uPos.sUwCoord.uwY+wY, 0, pManager->uMaxPos.sUwCoord.uwY);
}

void cameraCenterAt(tCameraManager *pManager, UWORD uwAvgX, UWORD uwAvgY) {
	tVPort *pVPort;
	
	pVPort = pManager->sCommon.pVPort;
	pManager->uPos.sUwCoord.uwX = clamp(uwAvgX - (pVPort->uwWidth>>1), 0, pManager->uMaxPos.sUwCoord.uwX);
	pManager->uPos.sUwCoord.uwY = clamp(uwAvgY - (pVPort->uwHeight>>1), 0, pManager->uMaxPos.sUwCoord.uwY);
}

inline UBYTE cameraIsMoved(tCameraManager *pManager) {
	return pManager->uPos.ulYX != pManager->uLastPos.ulYX;
}

inline UWORD cameraGetXDiff(tCameraManager *pManager) {
	return abs(cameraGetDeltaX(pManager));
}

inline UWORD cameraGetYDiff(tCameraManager *pManager) {
	return abs(cameraGetDeltaX(pManager));
}

inline WORD cameraGetDeltaX(tCameraManager *pManager) {
	return (pManager->uPos.sUwCoord.uwX - pManager->uLastPos.sUwCoord.uwX);
}

inline WORD cameraGetDeltaY(tCameraManager *pManager) {
	return (pManager->uPos.sUwCoord.uwY - pManager->uLastPos.sUwCoord.uwY);
}