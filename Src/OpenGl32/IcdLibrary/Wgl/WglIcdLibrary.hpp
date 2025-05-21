//
// Created by arthur on 18/05/2025.
//

#pragma once

#include <Concerto/Core/DynLib.hpp>

#include "OpenGl32/WglDispatchTable.hpp"
#include "OpenGl32/IcdLoader/IcdLoader.hpp"
#include "OpenGl32/IcdLoader/Wgl.hpp"

namespace glgpus
{
	class WglIcdLibrary : public IcdLibrary
	{
	public:
		WglIcdLibrary() = default;
		bool Load(std::string_view icdPath) override;

		BOOL DrvSetPixelFormat(HDC hdc, int pixelFormat) const;
		BOOL DrvDescribePixelFormat(HDC hdc, int pixelFormat, UINT nBytes, PIXELFORMATDESCRIPTOR* ppfd) const;
		HGLRC DrvCreateContext(HDC hdc) const;
		HGLRC DrvCreateLayerContext(HDC hdc, int layerPlane) const;
		BOOL DrvDeleteContext(HGLRC hglrc) const;
		BOOL DrvReleaseContext(DHGLRC dhglrc) const;
		const GlProcTable* DrvSetContext(HDC hdc, HGLRC hglrc, PFN_SetProcTable procTable) const;
		BOOL DrvShareLists(HGLRC hglrc1, HGLRC hglrc2) const;
		BOOL DrvCopyContext(HGLRC hglrcSrc, HGLRC hglrcDst, UINT mask) const;
		int DrvDescribeLayerPlane(HDC hdc, int pixelFormat, int layerPlane, UINT nBytes, void* plpd) const;
		BOOL DrvSetLayerPaletteEntries(HDC hdc, int layerPlane, int start, int entriesCount, const void* pcr) const;
		BOOL DrvGetLayerPaletteEntries(HDC hdc, int layerPlane, int start, int entriesCount, int* pcr) const;
		BOOL DrvSwapBuffers(HDC hdc) const;
		BOOL DrvSwapLayerBuffers(HDC hdc, UINT fuPlanes) const;
		void* DrvGetProcAddress(LPCSTR func) const;

		BOOL DrvValidateVersion(ULONG version) const;
		void DrvSetCallbackProcs(int size, PROC* procs) const;
		BOOL DrvRealizeLayerPalette(HDC hdc, int layerPlane, BOOL bRealize) const;
	private:
		struct DrvProcTable
		{
			// correspond topublic wgl functions
			cct::FunctionRef<BOOL(HDC, int)> DrvSetPixelFormat;
			cct::FunctionRef<BOOL(HDC, int, UINT, PIXELFORMATDESCRIPTOR*)> DrvDescribePixelFormat;
			cct::FunctionRef<HGLRC(HDC)> DrvCreateContext;
			cct::FunctionRef<HGLRC(HDC, int)> DrvCreateLayerContext;
			cct::FunctionRef<BOOL(HGLRC)> DrvDeleteContext;
			cct::FunctionRef<BOOL(DHGLRC)> DrvReleaseContext;
			cct::FunctionRef<const GlProcTable* (HDC, HGLRC, PFN_SetProcTable)> DrvSetContext;
			cct::FunctionRef<BOOL(HGLRC, HGLRC)> DrvShareLists;
			cct::FunctionRef<BOOL(HGLRC, HGLRC, UINT)> DrvCopyContext;
			cct::FunctionRef<int(HDC, int, int, UINT, void*)> DrvDescribeLayerPlane;
			cct::FunctionRef<BOOL(HDC, int, int, int, const void*)> DrvSetLayerPaletteEntries;
			cct::FunctionRef<BOOL(HDC, int, int, int, int*)> DrvGetLayerPaletteEntries;
			cct::FunctionRef<BOOL(HDC)> DrvSwapBuffers;
			cct::FunctionRef<BOOL(HDC, UINT)> DrvSwapLayerBuffers;
			cct::FunctionRef<void* (LPCSTR)> DrvGetProcAddress;

			// Internal use
			cct::FunctionRef<BOOL(ULONG)> DrvValidateVersion;
			cct::FunctionRef<void(int, PROC*)> DrvSetCallbackProcs;
			cct::FunctionRef<BOOL(HDC, int, BOOL)> DrvRealizeLayerPalette;
		} m_drvProcTable;
	};
} // namespace glgpus
