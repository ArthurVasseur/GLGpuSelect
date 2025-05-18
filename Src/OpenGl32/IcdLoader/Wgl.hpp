//
// Created by arthur on 23/04/2025.
//

#pragma once
#include "OpenGl32/OpenGlFunctions.hpp"
#ifdef CCT_PLATFORM_WINDOWS

#include <Windows.h>
#undef min
#undef max

using DHGLRC = void*;
/* PIXELFORMATDESCRIPTOR flags */
#define PFD_DOUBLEBUFFER            0x00000001
#define PFD_STEREO                  0x00000002
#define PFD_DRAW_TO_WINDOW          0x00000004
#define PFD_DRAW_TO_BITMAP          0x00000008
#define PFD_SUPPORT_GDI             0x00000010
#define PFD_SUPPORT_OPENGL          0x00000020
#define PFD_GENERIC_FORMAT          0x00000040
#define PFD_NEED_PALETTE            0x00000080
#define PFD_NEED_SYSTEM_PALETTE     0x00000100
#define PFD_SWAP_EXCHANGE           0x00000200
#define PFD_SWAP_COPY               0x00000400
#define PFD_SWAP_LAYER_BUFFERS      0x00000800
#define PFD_GENERIC_ACCELERATED     0x00001000
#define PFD_SUPPORT_DIRECTDRAW      0x00002000
#define PFD_DIRECT3D_ACCELERATED    0x00004000
#define PFD_SUPPORT_COMPOSITION     0x00008000

struct PIXELFORMATDESCRIPTOR
{
	WORD  nSize;
	WORD  nVersion;
	DWORD dwFlags;
	BYTE  iPixelType;
	BYTE  cColorBits;
	BYTE  cRedBits;
	BYTE  cRedShift;
	BYTE  cGreenBits;
	BYTE  cGreenShift;
	BYTE  cBlueBits;
	BYTE  cBlueShift;
	BYTE  cAlphaBits;
	BYTE  cAlphaShift;
	BYTE  cAccumBits;
	BYTE  cAccumRedBits;
	BYTE  cAccumGreenBits;
	BYTE  cAccumBlueBits;
	BYTE  cAccumAlphaBits;
	BYTE  cDepthBits;
	BYTE  cStencilBits;
	BYTE  cAuxBuffers;
	BYTE  iLayerType;
	BYTE  bReserved;
	DWORD dwLayerMask;
	DWORD dwVisibleMask;
	DWORD dwDamageMask;
};


GLGPUS_EXPORT int CCT_CALL wglSetPixelFormat(HDC hdc, int format, [[maybe_unused]] const PIXELFORMATDESCRIPTOR* ppfd);
GLGPUS_EXPORT int CCT_CALL wglGetPixelFormat(HDC hdc);
GLGPUS_EXPORT int CCT_CALL wglDescribePixelFormat(HDC hdc, int iPixelFormat, UINT nBytes, PIXELFORMATDESCRIPTOR* ppfd);
GLGPUS_EXPORT HGLRC CCT_CALL wglCreateContext(HDC hdc);
GLGPUS_EXPORT HGLRC CCT_CALL wglCreateLayerContext(HDC hdc, int layerPlane);
GLGPUS_EXPORT BOOL CCT_CALL wglDeleteContext(HGLRC hglrc);
GLGPUS_EXPORT BOOL CCT_CALL wglMakeCurrent(HDC hdc, HGLRC hglrc);
GLGPUS_EXPORT HGLRC CCT_CALL wglGetCurrentContext();
GLGPUS_EXPORT HDC  CCT_CALL wglGetCurrentDC();
GLGPUS_EXPORT BOOL CCT_CALL wglShareLists(HGLRC hglrc1, HGLRC hglrc2);
GLGPUS_EXPORT BOOL CCT_CALL wglCopyContext(HGLRC hglrcSrc, HGLRC hglrcDst, UINT mask);
GLGPUS_EXPORT int  CCT_CALL wglDescribeLayerPlane(HDC hdc, int pixelFormat, int layerPlane, UINT nBytes, void* plpd);
GLGPUS_EXPORT BOOL CCT_CALL wglRealizeLayerPalette(HDC hdc, int layerPlane, BOOL bRealize);
GLGPUS_EXPORT int  CCT_CALL wglSetLayerPaletteEntries(HDC hdc, int pixelFormat, int layerPlane, int numEntries, const void* pe);
GLGPUS_EXPORT int  CCT_CALL wglGetLayerPaletteEntries(HDC hdc, int pixelFormat, int layerPlane, int ne, int* pe);
GLGPUS_EXPORT BOOL CCT_CALL wglSwapBuffers(HDC hdc);
GLGPUS_EXPORT BOOL CCT_CALL wglSwapLayerBuffers(HDC hdc, UINT fuPlanes);
GLGPUS_EXPORT void* CCT_CALL wglGetProcAddress(LPCSTR lpszProc);
GLGPUS_EXPORT BOOL CCT_CALL wglUseFontBitmaps(HDC hdc, DWORD first, DWORD count, DWORD listBase);
GLGPUS_EXPORT BOOL CCT_CALL wglUseFontOutlines(HDC hdc, DWORD first, DWORD count, DWORD listBase, FLOAT deviation, FLOAT extrusion, int format, void* lpgmf);

#endif // CCT_PLATFORM_WINDOWS