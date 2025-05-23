//
// Created by arthur on 18/05/2025.
//

#pragma once

#include "OpenGl32/IcdLibrary/Wgl/WglIcdLibrary.hpp"

namespace glgpus
{
	bool WglIcdLibrary::Load(std::string_view icdPath)
	{
		GLGPUS_AUTO_PROFILER_SCOPE();

		{
			GLGPUS_PROFILER_SCOPE("cct::DynLib::Load");
			if (!m_icd.Load(icdPath))
				return false;
		}
		{
			GLGPUS_PROFILER_SCOPE("cct::DynLib::GetFunction(DrvSetPixelFormat)");
			m_drvProcTable.DrvSetPixelFormat = m_icd.GetFunction<BOOL, HDC, int>("DrvSetPixelFormat");
			if (!m_drvProcTable.DrvSetPixelFormat)
			{
				GLGPUS_ASSERT_FALSE("Could not find 'DrvSetPixelFormat' in ICD");
				SetLastError(ERROR_PROC_NOT_FOUND);
			}
		}
		{
			GLGPUS_PROFILER_SCOPE("cct::DynLib::GetFunction(DrvDescribePixelFormat)");
			m_drvProcTable.DrvDescribePixelFormat = m_icd.GetFunction<BOOL, HDC, int, UINT, PIXELFORMATDESCRIPTOR*>("DrvDescribePixelFormat");
			if (!m_drvProcTable.DrvDescribePixelFormat)
			{
				GLGPUS_ASSERT_FALSE("Could not find 'DrvDescribePixelFormat' in ICD");
				SetLastError(ERROR_PROC_NOT_FOUND);
			}
		}
		{
			GLGPUS_PROFILER_SCOPE("cct::DynLib::GetFunction(DrvCreateContext)");
			m_drvProcTable.DrvCreateContext = m_icd.GetFunction<HGLRC, HDC>("DrvCreateContext");
			if (!m_drvProcTable.DrvCreateContext)
			{
				GLGPUS_ASSERT_FALSE("Could not find 'DrvCreateContext' in ICD");
				SetLastError(ERROR_PROC_NOT_FOUND);
			}
		}
		{
			GLGPUS_PROFILER_SCOPE("cct::DynLib::GetFunction(DrvCreateLayerContext)");
			m_drvProcTable.DrvCreateLayerContext = m_icd.GetFunction<HGLRC, HDC, int>("DrvCreateLayerContext");
			if (!m_drvProcTable.DrvCreateLayerContext)
			{
				GLGPUS_ASSERT_FALSE("Could not find 'DrvCreateLayerContext' in ICD");
				SetLastError(ERROR_PROC_NOT_FOUND);
			}
		}
		{
			GLGPUS_PROFILER_SCOPE("cct::DynLib::GetFunction(DrvDeleteContext)");
			m_drvProcTable.DrvDeleteContext = m_icd.GetFunction<BOOL, HGLRC>("DrvDeleteContext");
			if (!m_drvProcTable.DrvDeleteContext)
			{
				GLGPUS_ASSERT_FALSE("Could not find 'DrvDeleteContext' in ICD");
				SetLastError(ERROR_PROC_NOT_FOUND);
			}
		}
		{
			GLGPUS_PROFILER_SCOPE("cct::DynLib::GetFunction(DrvReleaseContext)");
			m_drvProcTable.DrvReleaseContext = m_icd.GetFunction<BOOL, DHGLRC>("DrvReleaseContext");
			if (!m_drvProcTable.DrvReleaseContext)
			{
				GLGPUS_ASSERT_FALSE("Could not find 'DrvReleaseContext' in ICD");
				SetLastError(ERROR_PROC_NOT_FOUND);
			}
		}
		{
			GLGPUS_PROFILER_SCOPE("cct::DynLib::GetFunction(DrvSetContext)");
			m_drvProcTable.DrvSetContext = m_icd.GetFunction<const GlProcTable*, HDC, HGLRC, PFN_SetProcTable>("DrvSetContext");
			if (!m_drvProcTable.DrvSetContext)
			{
				GLGPUS_ASSERT_FALSE("Could not find 'DrvSetContext' in ICD");
				SetLastError(ERROR_PROC_NOT_FOUND);
			}
		}
		{
			GLGPUS_PROFILER_SCOPE("cct::DynLib::GetFunction(DrvShareLists)");
			m_drvProcTable.DrvShareLists = m_icd.GetFunction<BOOL, HGLRC, HGLRC>("DrvShareLists");
			if (!m_drvProcTable.DrvShareLists)
			{
				GLGPUS_ASSERT_FALSE("Could not find 'DrvShareLists' in ICD");
				SetLastError(ERROR_PROC_NOT_FOUND);
			}
		}
		{
			GLGPUS_PROFILER_SCOPE("cct::DynLib::GetFunction(DrvCopyContext)");
			m_drvProcTable.DrvCopyContext = m_icd.GetFunction<BOOL, HGLRC, HGLRC, UINT>("DrvCopyContext");
			if (!m_drvProcTable.DrvCopyContext)
			{
				GLGPUS_ASSERT_FALSE("Could not find 'DrvCopyContext' in ICD");
				SetLastError(ERROR_PROC_NOT_FOUND);
			}
		}
		{
			GLGPUS_PROFILER_SCOPE("cct::DynLib::GetFunction(DrvDescribeLayerPlane)");
			m_drvProcTable.DrvDescribeLayerPlane = m_icd.GetFunction<int, HDC, int, int, UINT, void*>("DrvDescribeLayerPlane");
			if (!m_drvProcTable.DrvDescribeLayerPlane)
			{
				GLGPUS_ASSERT_FALSE("Could not find 'DrvDescribeLayerPlane' in ICD");
				SetLastError(ERROR_PROC_NOT_FOUND);
			}
		}
		{
			GLGPUS_PROFILER_SCOPE("cct::DynLib::GetFunction(DrvSetLayerPaletteEntries)");
			m_drvProcTable.DrvSetLayerPaletteEntries = m_icd.GetFunction<BOOL, HDC, int, int, int, const void*>("DrvSetLayerPaletteEntries");
			if (!m_drvProcTable.DrvSetLayerPaletteEntries)
			{
				GLGPUS_ASSERT_FALSE("Could not find 'DrvSetLayerPaletteEntries' in ICD");
				SetLastError(ERROR_PROC_NOT_FOUND);
			}
		}
		{
			GLGPUS_PROFILER_SCOPE("cct::DynLib::GetFunction(DrvGetLayerPaletteEntries)");
			m_drvProcTable.DrvGetLayerPaletteEntries = m_icd.GetFunction<BOOL, HDC, int, int, int, int* >("DrvGetLayerPaletteEntries");
			if (!m_drvProcTable.DrvGetLayerPaletteEntries)
			{
				GLGPUS_ASSERT_FALSE("Could not find 'DrvGetLayerPaletteEntries' in ICD");
				SetLastError(ERROR_PROC_NOT_FOUND);
			}
		}
		{
			GLGPUS_PROFILER_SCOPE("cct::DynLib::GetFunction(DrvSwapBuffers)");
			m_drvProcTable.DrvSwapBuffers = m_icd.GetFunction<BOOL, HDC>("DrvSwapBuffers");
			if (!m_drvProcTable.DrvSwapBuffers)
			{
				GLGPUS_ASSERT_FALSE("Could not find 'DrvSwapBuffers' in ICD");
				SetLastError(ERROR_PROC_NOT_FOUND);
			}
		}
		{
			GLGPUS_PROFILER_SCOPE("cct::DynLib::GetFunction(DrvSwapLayerBuffers)");
			m_drvProcTable.DrvSwapLayerBuffers = m_icd.GetFunction<BOOL, HDC, UINT>("DrvSwapLayerBuffers");
			if (!m_drvProcTable.DrvSwapLayerBuffers)
			{
				GLGPUS_ASSERT_FALSE("Could not find 'DrvSwapLayerBuffers' in ICD");
				SetLastError(ERROR_PROC_NOT_FOUND);
			}
		}
		{
			GLGPUS_PROFILER_SCOPE("cct::DynLib::GetFunction(DrvGetProcAddress)");
			m_drvProcTable.DrvGetProcAddress = m_icd.GetFunction<void*, LPCSTR>("DrvGetProcAddress");
			if (!m_drvProcTable.DrvGetProcAddress)
			{
				GLGPUS_ASSERT_FALSE("Could not find 'DrvGetProcAddress' in ICD");
				SetLastError(ERROR_PROC_NOT_FOUND);
			}
		}


		{
			GLGPUS_PROFILER_SCOPE("cct::DynLib::GetFunction(DrvValidateVersion)");
			m_drvProcTable.DrvValidateVersion = m_icd.GetFunction<BOOL, ULONG>("DrvValidateVersion");
			if (!m_drvProcTable.DrvValidateVersion)
			{
				GLGPUS_ASSERT_FALSE("Could not find 'DrvValidateVersion' in ICD");
				SetLastError(ERROR_PROC_NOT_FOUND);
			}
		}
		{
			GLGPUS_PROFILER_SCOPE("cct::DynLib::GetFunction(DrvSetCallbackProcs)");
			m_drvProcTable.DrvSetCallbackProcs = m_icd.GetFunction<void, int, PROC*>("DrvSetCallbackProcs");
			if (!m_drvProcTable.DrvSetCallbackProcs)
			{
				GLGPUS_ASSERT_FALSE("Could not find 'DrvSetCallbackProcs' in ICD");
				SetLastError(ERROR_PROC_NOT_FOUND);
			}
		}
		{
			GLGPUS_PROFILER_SCOPE("cct::DynLib::GetFunction(DrvRealizeLayerPalette)");
			m_drvProcTable.DrvRealizeLayerPalette = m_icd.GetFunction<BOOL, HDC, int, BOOL>("DrvRealizeLayerPalette");
			if (!m_drvProcTable.DrvRealizeLayerPalette)
			{
				GLGPUS_ASSERT_FALSE("Could not find 'DrvRealizeLayerPalette' in ICD");
				SetLastError(ERROR_PROC_NOT_FOUND);
			}
		}

		return true;
	}

	BOOL WglIcdLibrary::DrvSetPixelFormat(HDC hdc, int pixelFormat) const
	{
		GLGPUS_AUTO_PROFILER_SCOPE();

		if (!m_drvProcTable.DrvSetPixelFormat)
		{
			GLGPUS_ASSERT_FALSE("Invalid function 'DrvSetPixelFormat' in IcdLibrary");
			return false;
		}
		return m_drvProcTable.DrvSetPixelFormat(hdc, pixelFormat);
	}

	BOOL WglIcdLibrary::DrvDescribePixelFormat(HDC hdc, int pixelFormat, UINT nBytes, PIXELFORMATDESCRIPTOR* ppfd) const
	{
		GLGPUS_AUTO_PROFILER_SCOPE();

		if (!m_drvProcTable.DrvDescribePixelFormat)
		{
			GLGPUS_ASSERT_FALSE("Invalid function 'DrvDescribePixelFormat' in IcdLibrary");
			return false;
		}
		return m_drvProcTable.DrvDescribePixelFormat(hdc, pixelFormat, nBytes, ppfd);
	}

	HGLRC WglIcdLibrary::DrvCreateContext(HDC hdc) const
	{
		GLGPUS_AUTO_PROFILER_SCOPE();

		if (!m_drvProcTable.DrvCreateContext)
		{
			GLGPUS_ASSERT_FALSE("Invalid function 'DrvCreateContext' in IcdLibrary");
			return nullptr;
		}
		return m_drvProcTable.DrvCreateContext(hdc);
	}

	HGLRC WglIcdLibrary::DrvCreateLayerContext(HDC hdc, int layerPlane) const
	{
		GLGPUS_AUTO_PROFILER_SCOPE();

		if (!m_drvProcTable.DrvCreateLayerContext)
		{
			GLGPUS_ASSERT_FALSE("Invalid function 'DrvCreateLayerContext' in IcdLibrary");
			return nullptr;
		}
		return m_drvProcTable.DrvCreateLayerContext(hdc, layerPlane);
	}

	BOOL WglIcdLibrary::DrvDeleteContext(HGLRC hglrc) const
	{
		GLGPUS_AUTO_PROFILER_SCOPE();

		if (!m_drvProcTable.DrvDeleteContext)
		{
			GLGPUS_ASSERT_FALSE("Invalid function 'DrvDeleteContext' in IcdLibrary");
			return false;
		}
		return m_drvProcTable.DrvDeleteContext(hglrc);
	}

	BOOL WglIcdLibrary::DrvReleaseContext(DHGLRC dhglrc) const
	{
		GLGPUS_AUTO_PROFILER_SCOPE();

		if (!m_drvProcTable.DrvReleaseContext)
		{
			GLGPUS_ASSERT_FALSE("Invalid function 'DrvReleaseContext' in IcdLibrary");
			return false;
		}
		return m_drvProcTable.DrvReleaseContext(dhglrc);
	}

	const GlProcTable* WglIcdLibrary::DrvSetContext(HDC hdc, HGLRC hglrc, PFN_SetProcTable procTable) const
	{
		GLGPUS_AUTO_PROFILER_SCOPE();

		if (!m_drvProcTable.DrvSetContext)
		{
			GLGPUS_ASSERT_FALSE("Invalid function 'DrvSetContext' in IcdLibrary");
			return nullptr;
		}
		return m_drvProcTable.DrvSetContext(hdc, hglrc, procTable);
	}

	BOOL WglIcdLibrary::DrvShareLists(HGLRC hglrc1, HGLRC hglrc2) const
	{
		GLGPUS_AUTO_PROFILER_SCOPE();

		if (!m_drvProcTable.DrvShareLists)
		{
			GLGPUS_ASSERT_FALSE("Invalid function 'DrvShareLists' in IcdLibrary");
			return false;
		}
		return m_drvProcTable.DrvShareLists(hglrc1, hglrc2);
	}

	BOOL WglIcdLibrary::DrvCopyContext(HGLRC hglrcSrc, HGLRC hglrcDst, UINT mask) const
	{
		GLGPUS_AUTO_PROFILER_SCOPE();

		if (!m_drvProcTable.DrvCopyContext)
		{
			GLGPUS_ASSERT_FALSE("Invalid function 'DrvCopyContext' in IcdLibrary");
			return false;
		}
		return m_drvProcTable.DrvCopyContext(hglrcSrc, hglrcDst, mask);
	}

	int WglIcdLibrary::DrvDescribeLayerPlane(HDC hdc, int pixelFormat, int layerPlane, UINT nBytes, void* plpd) const
	{
		GLGPUS_AUTO_PROFILER_SCOPE();

		if (!m_drvProcTable.DrvDescribeLayerPlane)
		{
			GLGPUS_ASSERT_FALSE("Invalid function 'DrvDescribeLayerPlane' in IcdLibrary");
			return -1;
		}
		return m_drvProcTable.DrvDescribeLayerPlane(hdc, pixelFormat, layerPlane, nBytes, plpd);
	}

	BOOL WglIcdLibrary::DrvSetLayerPaletteEntries(HDC hdc, int layerPlane, int start, int entriesCount, const void* pcr) const
	{
		GLGPUS_AUTO_PROFILER_SCOPE();

		if (!m_drvProcTable.DrvSetLayerPaletteEntries)
		{
			GLGPUS_ASSERT_FALSE("Invalid function 'DrvSetLayerPaletteEntries' in IcdLibrary");
			return false;
		}
		return m_drvProcTable.DrvSetLayerPaletteEntries(hdc, layerPlane, start, entriesCount, pcr);
	}

	BOOL WglIcdLibrary::DrvGetLayerPaletteEntries(HDC hdc, int layerPlane, int start, int entriesCount, int* pcr) const
	{
		GLGPUS_AUTO_PROFILER_SCOPE();

		if (!m_drvProcTable.DrvGetLayerPaletteEntries)
		{
			GLGPUS_ASSERT_FALSE("Invalid function 'DrvGetLayerPaletteEntries' in IcdLibrary");
			return false;
		}
		return m_drvProcTable.DrvGetLayerPaletteEntries(hdc, layerPlane, start, entriesCount, pcr);
	}

	BOOL WglIcdLibrary::DrvSwapBuffers(HDC hdc) const
	{
		GLGPUS_AUTO_PROFILER_SCOPE();

		if (!m_drvProcTable.DrvSwapBuffers)
		{
			GLGPUS_ASSERT_FALSE("Invalid function 'DrvSwapBuffers' in IcdLibrary");
			return false;
		}
		return m_drvProcTable.DrvSwapBuffers(hdc);
	}

	BOOL WglIcdLibrary::DrvSwapLayerBuffers(HDC hdc, UINT fuPlanes) const
	{
		GLGPUS_AUTO_PROFILER_SCOPE();

		if (!m_drvProcTable.DrvSwapLayerBuffers)
		{
			GLGPUS_ASSERT_FALSE("Invalid function 'DrvSwapLayerBuffers' in IcdLibrary");
			return false;
		}
		return m_drvProcTable.DrvSwapLayerBuffers(hdc, fuPlanes);
	}

	void* WglIcdLibrary::DrvGetProcAddress(LPCSTR func) const
	{
		GLGPUS_AUTO_PROFILER_SCOPE();

		if (!m_drvProcTable.DrvGetProcAddress)
		{
			GLGPUS_ASSERT_FALSE("Invalid function 'DrvGetProcAddress' in IcdLibrary");
			return nullptr;
		}
		return m_drvProcTable.DrvGetProcAddress(func);
	}

	BOOL WglIcdLibrary::DrvValidateVersion(ULONG version) const
	{
		GLGPUS_AUTO_PROFILER_SCOPE();

		if (!m_drvProcTable.DrvValidateVersion)
		{
			GLGPUS_ASSERT_FALSE("Invalid function 'DrvValidateVersion' in IcdLibrary");
			return false;
		}
		return m_drvProcTable.DrvValidateVersion(version);
	}

	void WglIcdLibrary::DrvSetCallbackProcs(int size, PROC* procs) const
	{
		GLGPUS_AUTO_PROFILER_SCOPE();

		if (!m_drvProcTable.DrvSetCallbackProcs)
		{
			GLGPUS_ASSERT_FALSE("Invalid function 'DrvSetCallbackProcs' in IcdLibrary");
			return;
		}
		return m_drvProcTable.DrvSetCallbackProcs(size, procs);
	}

	BOOL WglIcdLibrary::DrvRealizeLayerPalette(HDC hdc, int layerPlane, BOOL bRealize) const
	{
		GLGPUS_AUTO_PROFILER_SCOPE();

		if (!m_drvProcTable.DrvRealizeLayerPalette)
		{
			GLGPUS_ASSERT_FALSE("Invalid function 'DrvRealizeLayerPalette' in IcdLibrary");
			return false;
		}
		return m_drvProcTable.DrvRealizeLayerPalette(hdc, layerPlane, bRealize);
	}
} // namespace glgpus
