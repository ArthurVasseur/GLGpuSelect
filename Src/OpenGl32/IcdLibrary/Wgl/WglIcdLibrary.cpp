//
// Created by arthur on 18/05/2025.
//

#pragma once

#include "OpenGl32/IcdLibrary/Wgl/WglIcdLibrary.hpp"

namespace glgpus
{
	bool WglIcdLibrary::Load(std::string_view icdPath)
	{
		if (!m_icd.Load(icdPath))
			return false;

		m_drvProcTable.DrvSetPixelFormat = m_icd.GetFunction<BOOL, HDC, int>("DrvSetPixelFormat");
		if (!m_drvProcTable.DrvSetPixelFormat)
		{
			CCT_ASSERT_FALSE("Could not find 'DrvSetPixelFormat' in ICD");
			SetLastError(ERROR_PROC_NOT_FOUND);
			//return false;
		}
		m_drvProcTable.DrvDescribePixelFormat = m_icd.GetFunction<BOOL, HDC, int, UINT, PIXELFORMATDESCRIPTOR*>("DrvDescribePixelFormat");
		if (!m_drvProcTable.DrvDescribePixelFormat)
		{
			CCT_ASSERT_FALSE("Could not find 'DrvDescribePixelFormat' in ICD");
			SetLastError(ERROR_PROC_NOT_FOUND);
			//return false;
		}
		m_drvProcTable.DrvCreateContext = m_icd.GetFunction<HGLRC, HDC>("DrvCreateContext");
		if (!m_drvProcTable.DrvCreateContext)
		{
			CCT_ASSERT_FALSE("Could not find 'DrvCreateContext' in ICD");
			SetLastError(ERROR_PROC_NOT_FOUND);
			//return false;
		}
		m_drvProcTable.DrvCreateLayerContext = m_icd.GetFunction<HGLRC, HDC, int>("DrvCreateLayerContext");
		if (!m_drvProcTable.DrvCreateLayerContext)
		{
			CCT_ASSERT_FALSE("Could not find 'DrvCreateLayerContext' in ICD");
			SetLastError(ERROR_PROC_NOT_FOUND);
			//return false;
		}
		m_drvProcTable.DrvDeleteContext = m_icd.GetFunction<BOOL, HGLRC>("DrvDeleteContext");
		if (!m_drvProcTable.DrvDeleteContext)
		{
			CCT_ASSERT_FALSE("Could not find 'DrvDeleteContext' in ICD");
			SetLastError(ERROR_PROC_NOT_FOUND);
			//return false;
		}
		m_drvProcTable.DrvReleaseContext = m_icd.GetFunction<BOOL, DHGLRC>("DrvReleaseContext");
		if (!m_drvProcTable.DrvReleaseContext)
		{
			CCT_ASSERT_FALSE("Could not find 'DrvReleaseContext' in ICD");
			SetLastError(ERROR_PROC_NOT_FOUND);
			//return false;
		}
		m_drvProcTable.DrvSetContext = m_icd.GetFunction<const GlProcTable*, HDC, HGLRC, PFN_SetProcTable>("DrvSetContext");
		if (!m_drvProcTable.DrvSetContext)
		{
			CCT_ASSERT_FALSE("Could not find 'DrvSetContext' in ICD");
			SetLastError(ERROR_PROC_NOT_FOUND);
			//return false;
		}
		m_drvProcTable.DrvShareLists = m_icd.GetFunction<BOOL, HGLRC, HGLRC>("DrvShareLists");
		if (!m_drvProcTable.DrvShareLists)
		{
			CCT_ASSERT_FALSE("Could not find 'DrvShareLists' in ICD");
			SetLastError(ERROR_PROC_NOT_FOUND);
			//return false;
		}
		m_drvProcTable.DrvCopyContext = m_icd.GetFunction<BOOL, HGLRC, HGLRC, UINT>("DrvCopyContext");
		if (!m_drvProcTable.DrvCopyContext)
		{
			CCT_ASSERT_FALSE("Could not find 'DrvCopyContext' in ICD");
			SetLastError(ERROR_PROC_NOT_FOUND);
			//return false;
		}
		m_drvProcTable.DrvDescribeLayerPlane = m_icd.GetFunction<int, HDC, int, int, UINT, void*>("DrvDescribeLayerPlane");
		if (!m_drvProcTable.DrvDescribeLayerPlane)
		{
			CCT_ASSERT_FALSE("Could not find 'DrvDescribeLayerPlane' in ICD");
			SetLastError(ERROR_PROC_NOT_FOUND);
			//return false;
		}
		m_drvProcTable.DrvSetLayerPaletteEntries = m_icd.GetFunction<BOOL, HDC, int, int, int, const void*>("DrvSetLayerPaletteEntries");
		if (!m_drvProcTable.DrvSetLayerPaletteEntries)
		{
			CCT_ASSERT_FALSE("Could not find 'DrvSetLayerPaletteEntries' in ICD");
			SetLastError(ERROR_PROC_NOT_FOUND);
			//return false;
		}
		m_drvProcTable.DrvGetLayerPaletteEntries = m_icd.GetFunction<BOOL, HDC, int, int, int, int* >("DrvGetLayerPaletteEntries");
		if (!m_drvProcTable.DrvGetLayerPaletteEntries)
		{
			CCT_ASSERT_FALSE("Could not find 'DrvGetLayerPaletteEntries' in ICD");
			SetLastError(ERROR_PROC_NOT_FOUND);
			//return false;
		}
		m_drvProcTable.DrvSwapBuffers = m_icd.GetFunction<BOOL, HDC>("DrvSwapBuffers");
		if (!m_drvProcTable.DrvSwapBuffers)
		{
			CCT_ASSERT_FALSE("Could not find 'DrvSwapBuffers' in ICD");
			SetLastError(ERROR_PROC_NOT_FOUND);
			//return false;
		}
		m_drvProcTable.DrvSwapLayerBuffers = m_icd.GetFunction<BOOL, HDC, UINT>("DrvSwapLayerBuffers");
		if (!m_drvProcTable.DrvSwapLayerBuffers)
		{
			CCT_ASSERT_FALSE("Could not find 'DrvSwapLayerBuffers' in ICD");
			SetLastError(ERROR_PROC_NOT_FOUND);
			//return false;
		}
		m_drvProcTable.DrvGetProcAddress = m_icd.GetFunction<void*, LPCSTR>("DrvGetProcAddress");
		if (!m_drvProcTable.DrvGetProcAddress)
		{
			CCT_ASSERT_FALSE("Could not find 'DrvGetProcAddress' in ICD");
			SetLastError(ERROR_PROC_NOT_FOUND);
			//return false;
		}


		m_drvProcTable.DrvValidateVersion = m_icd.GetFunction<BOOL, ULONG>("DrvValidateVersion");
		if (!m_drvProcTable.DrvValidateVersion)
		{
			CCT_ASSERT_FALSE("Could not find 'DrvValidateVersion' in ICD");
			SetLastError(ERROR_PROC_NOT_FOUND);
			//return false;
		}
		m_drvProcTable.DrvSetCallbackProcs = m_icd.GetFunction<void, int, PROC*>("DrvSetCallbackProcs");
		if (!m_drvProcTable.DrvSetCallbackProcs)
		{
			CCT_ASSERT_FALSE("Could not find 'DrvSetCallbackProcs' in ICD");
			SetLastError(ERROR_PROC_NOT_FOUND);
			//return false;
		}
		m_drvProcTable.DrvRealizeLayerPalette = m_icd.GetFunction<BOOL, HDC, int, BOOL>("DrvRealizeLayerPalette");
		if (!m_drvProcTable.DrvRealizeLayerPalette)
		{
			CCT_ASSERT_FALSE("Could not find 'DrvRealizeLayerPalette' in ICD");
			SetLastError(ERROR_PROC_NOT_FOUND);
			//return false;
		}

		return true;
	}

	BOOL WglIcdLibrary::DrvSetPixelFormat(HDC hdc, int pixelFormat) const
	{
		if (!m_drvProcTable.DrvSetPixelFormat)
		{
			CCT_ASSERT_FALSE("Invalid function 'DrvSetPixelFormat' in IcdLibrary");
			return false;
		}
		return m_drvProcTable.DrvSetPixelFormat(hdc, pixelFormat);
	}

	BOOL WglIcdLibrary::DrvDescribePixelFormat(HDC hdc, int pixelFormat, UINT nBytes, PIXELFORMATDESCRIPTOR* ppfd) const
	{
		if (!m_drvProcTable.DrvDescribePixelFormat)
		{
			CCT_ASSERT_FALSE("Invalid function 'DrvDescribePixelFormat' in IcdLibrary");
			return false;
		}
		return m_drvProcTable.DrvDescribePixelFormat(hdc, pixelFormat, nBytes, ppfd);
	}

	HGLRC WglIcdLibrary::DrvCreateContext(HDC hdc) const
	{
		if (!m_drvProcTable.DrvCreateContext)
		{
			CCT_ASSERT_FALSE("Invalid function 'DrvCreateContext' in IcdLibrary");
			return nullptr;
		}
		return m_drvProcTable.DrvCreateContext(hdc);
	}

	HGLRC WglIcdLibrary::DrvCreateLayerContext(HDC hdc, int layerPlane) const
	{
		if (!m_drvProcTable.DrvCreateLayerContext)
		{
			CCT_ASSERT_FALSE("Invalid function 'DrvCreateLayerContext' in IcdLibrary");
			return nullptr;
		}
		return m_drvProcTable.DrvCreateLayerContext(hdc, layerPlane);
	}

	BOOL WglIcdLibrary::DrvDeleteContext(HGLRC hglrc) const
	{
		if (!m_drvProcTable.DrvDeleteContext)
		{
			CCT_ASSERT_FALSE("Invalid function 'DrvDeleteContext' in IcdLibrary");
			return false;
		}
		return m_drvProcTable.DrvDeleteContext(hglrc);
	}

	BOOL WglIcdLibrary::DrvReleaseContext(DHGLRC dhglrc) const
	{
		if (!m_drvProcTable.DrvReleaseContext)
		{
			CCT_ASSERT_FALSE("Invalid function 'DrvReleaseContext' in IcdLibrary");
			return false;
		}
		return m_drvProcTable.DrvReleaseContext(dhglrc);
	}

	const GlProcTable* WglIcdLibrary::DrvSetContext(HDC hdc, HGLRC hglrc, PFN_SetProcTable procTable) const
	{
		if (!m_drvProcTable.DrvSetContext)
		{
			CCT_ASSERT_FALSE("Invalid function 'DrvSetContext' in IcdLibrary");
			return nullptr;
		}
		return m_drvProcTable.DrvSetContext(hdc, hglrc, procTable);
	}

	BOOL WglIcdLibrary::DrvShareLists(HGLRC hglrc1, HGLRC hglrc2) const
	{
		if (!m_drvProcTable.DrvShareLists)
		{
			CCT_ASSERT_FALSE("Invalid function 'DrvShareLists' in IcdLibrary");
			return false;
		}
		return m_drvProcTable.DrvShareLists(hglrc1, hglrc2);
	}

	BOOL WglIcdLibrary::DrvCopyContext(HGLRC hglrcSrc, HGLRC hglrcDst, UINT mask) const
	{
		if (!m_drvProcTable.DrvCopyContext)
		{
			CCT_ASSERT_FALSE("Invalid function 'DrvCopyContext' in IcdLibrary");
			return false;
		}
		return m_drvProcTable.DrvCopyContext(hglrcSrc, hglrcDst, mask);
	}

	int WglIcdLibrary::DrvDescribeLayerPlane(HDC hdc, int pixelFormat, int layerPlane, UINT nBytes, void* plpd) const
	{
		if (!m_drvProcTable.DrvDescribeLayerPlane)
		{
			CCT_ASSERT_FALSE("Invalid function 'DrvDescribeLayerPlane' in IcdLibrary");
			return -1;
		}
		return m_drvProcTable.DrvDescribeLayerPlane(hdc, pixelFormat, layerPlane, nBytes, plpd);
	}

	BOOL WglIcdLibrary::DrvSetLayerPaletteEntries(HDC hdc, int layerPlane, int start, int entriesCount, const void* pcr) const
	{
		if (!m_drvProcTable.DrvSetLayerPaletteEntries)
		{
			CCT_ASSERT_FALSE("Invalid function 'DrvSetLayerPaletteEntries' in IcdLibrary");
			return false;
		}
		return m_drvProcTable.DrvSetLayerPaletteEntries(hdc, layerPlane, start, entriesCount, pcr);
	}

	BOOL WglIcdLibrary::DrvGetLayerPaletteEntries(HDC hdc, int layerPlane, int start, int entriesCount, int* pcr) const
	{
		if (!m_drvProcTable.DrvGetLayerPaletteEntries)
		{
			CCT_ASSERT_FALSE("Invalid function 'DrvGetLayerPaletteEntries' in IcdLibrary");
			return false;
		}
		return m_drvProcTable.DrvGetLayerPaletteEntries(hdc, layerPlane, start, entriesCount, pcr);
	}

	BOOL WglIcdLibrary::DrvSwapBuffers(HDC hdc) const
	{
		if (!m_drvProcTable.DrvSwapBuffers)
		{
			CCT_ASSERT_FALSE("Invalid function 'DrvSwapBuffers' in IcdLibrary");
			return false;
		}
		return m_drvProcTable.DrvSwapBuffers(hdc);
	}

	BOOL WglIcdLibrary::DrvSwapLayerBuffers(HDC hdc, UINT fuPlanes) const
	{
		if (!m_drvProcTable.DrvSwapLayerBuffers)
		{
			CCT_ASSERT_FALSE("Invalid function 'DrvSwapLayerBuffers' in IcdLibrary");
			return false;
		}
		return m_drvProcTable.DrvSwapLayerBuffers(hdc, fuPlanes);
	}

	void* WglIcdLibrary::DrvGetProcAddress(LPCSTR func) const
	{
		if (!m_drvProcTable.DrvGetProcAddress)
		{
			CCT_ASSERT_FALSE("Invalid function 'DrvGetProcAddress' in IcdLibrary");
			return nullptr;
		}
		return m_drvProcTable.DrvGetProcAddress(func);
	}

	BOOL WglIcdLibrary::DrvValidateVersion(ULONG version) const
	{
		if (!m_drvProcTable.DrvValidateVersion)
		{
			CCT_ASSERT_FALSE("Invalid function 'DrvValidateVersion' in IcdLibrary");
			return false;
		}
		return m_drvProcTable.DrvValidateVersion(version);
	}

	void WglIcdLibrary::DrvSetCallbackProcs(int size, PROC* procs) const
	{
		if (!m_drvProcTable.DrvSetCallbackProcs)
		{
			CCT_ASSERT_FALSE("Invalid function 'DrvSetCallbackProcs' in IcdLibrary");
			return;
		}
		return m_drvProcTable.DrvSetCallbackProcs(size, procs);
	}

	BOOL WglIcdLibrary::DrvRealizeLayerPalette(HDC hdc, int layerPlane, BOOL bRealize) const
	{
		if (!m_drvProcTable.DrvRealizeLayerPalette)
		{
			CCT_ASSERT_FALSE("Invalid function 'DrvRealizeLayerPalette' in IcdLibrary");
			return false;
		}
		return m_drvProcTable.DrvRealizeLayerPalette(hdc, layerPlane, bRealize);
	}
} // namespace glgpus
