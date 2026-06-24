#include "GlIcdDriver/GlIcdDriver.hpp"

#include "GlLoader/Defines.hpp"

namespace gl
{

	bool GlIcdDriver::Load(std::string_view icdPath)
	{
		GLGPUS_AUTO_PROFILER_SCOPE();

		if (!m_icd.Load(icdPath))
			return false;

		bool loaded = true;

		m_drvProcTable.DrvSetPixelFormat = m_icd.GetFunction<BOOL, HDC, int>("DrvSetPixelFormat");
		if (!m_drvProcTable.DrvSetPixelFormat)
		{
			GLGPUS_ASSERT_FALSE("Could not find 'DrvSetPixelFormat' in ICD");
			SetLastError(ERROR_PROC_NOT_FOUND);
			loaded = false;
		}

		m_drvProcTable.DrvDescribePixelFormat = m_icd.GetFunction<int, HDC, int, UINT, PIXELFORMATDESCRIPTOR*>("DrvDescribePixelFormat");
		if (!m_drvProcTable.DrvDescribePixelFormat)
		{
			GLGPUS_ASSERT_FALSE("Could not find 'DrvDescribePixelFormat' in ICD");
			SetLastError(ERROR_PROC_NOT_FOUND);
			loaded = false;
		}

		m_drvProcTable.DrvCreateContext = m_icd.GetFunction<HGLRC, HDC>("DrvCreateContext");
		if (!m_drvProcTable.DrvCreateContext)
		{
			GLGPUS_ASSERT_FALSE("Could not find 'DrvCreateContext' in ICD");
			SetLastError(ERROR_PROC_NOT_FOUND);
			loaded = false;
		}

		m_drvProcTable.DrvCreateLayerContext = m_icd.GetFunction<HGLRC, HDC, int>("DrvCreateLayerContext");
		if (!m_drvProcTable.DrvCreateLayerContext)
		{
			GLGPUS_ASSERT_FALSE("Could not find 'DrvCreateLayerContext' in ICD");
			SetLastError(ERROR_PROC_NOT_FOUND);
			loaded = false;
		}

		m_drvProcTable.DrvDeleteContext = m_icd.GetFunction<BOOL, HGLRC>("DrvDeleteContext");
		if (!m_drvProcTable.DrvDeleteContext)
		{
			GLGPUS_ASSERT_FALSE("Could not find 'DrvDeleteContext' in ICD");
			SetLastError(ERROR_PROC_NOT_FOUND);
			loaded = false;
		}

		m_drvProcTable.DrvReleaseContext = m_icd.GetFunction<BOOL, DHGLRC>("DrvReleaseContext");
		if (!m_drvProcTable.DrvReleaseContext)
		{
			GLGPUS_ASSERT_FALSE("Could not find 'DrvReleaseContext' in ICD");
			SetLastError(ERROR_PROC_NOT_FOUND);
			loaded = false;
		}

		m_drvProcTable.DrvSetContext = m_icd.GetFunction<const glgpus::GlProcTable*, HDC, HGLRC, glgpus::PFN_SetProcTable>("DrvSetContext");
		if (!m_drvProcTable.DrvSetContext)
		{
			GLGPUS_ASSERT_FALSE("Could not find 'DrvSetContext' in ICD");
			SetLastError(ERROR_PROC_NOT_FOUND);
			loaded = false;
		}

		m_drvProcTable.DrvShareLists = m_icd.GetFunction<BOOL, HGLRC, HGLRC>("DrvShareLists");
		if (!m_drvProcTable.DrvShareLists)
		{
			GLGPUS_ASSERT_FALSE("Could not find 'DrvShareLists' in ICD");
			SetLastError(ERROR_PROC_NOT_FOUND);
			loaded = false;
		}

		m_drvProcTable.DrvCopyContext = m_icd.GetFunction<BOOL, HGLRC, HGLRC, UINT>("DrvCopyContext");
		if (!m_drvProcTable.DrvCopyContext)
		{
			GLGPUS_ASSERT_FALSE("Could not find 'DrvCopyContext' in ICD");
			SetLastError(ERROR_PROC_NOT_FOUND);
			loaded = false;
		}

		m_drvProcTable.DrvDescribeLayerPlane = m_icd.GetFunction<int, HDC, int, int, UINT, void*>("DrvDescribeLayerPlane");
		if (!m_drvProcTable.DrvDescribeLayerPlane)
		{
			GLGPUS_ASSERT_FALSE("Could not find 'DrvDescribeLayerPlane' in ICD");
			SetLastError(ERROR_PROC_NOT_FOUND);
			loaded = false;
		}

		m_drvProcTable.DrvSetLayerPaletteEntries = m_icd.GetFunction<BOOL, HDC, int, int, int, const void*>("DrvSetLayerPaletteEntries");
		if (!m_drvProcTable.DrvSetLayerPaletteEntries)
		{
			GLGPUS_ASSERT_FALSE("Could not find 'DrvSetLayerPaletteEntries' in ICD");
			SetLastError(ERROR_PROC_NOT_FOUND);
			loaded = false;
		}

		m_drvProcTable.DrvGetLayerPaletteEntries = m_icd.GetFunction<BOOL, HDC, int, int, int, int*>("DrvGetLayerPaletteEntries");
		if (!m_drvProcTable.DrvGetLayerPaletteEntries)
		{
			GLGPUS_ASSERT_FALSE("Could not find 'DrvGetLayerPaletteEntries' in ICD");
			SetLastError(ERROR_PROC_NOT_FOUND);
			loaded = false;
		}

		m_drvProcTable.DrvSwapBuffers = m_icd.GetFunction<BOOL, HDC>("DrvSwapBuffers");
		if (!m_drvProcTable.DrvSwapBuffers)
		{
			GLGPUS_ASSERT_FALSE("Could not find 'DrvSwapBuffers' in ICD");
			SetLastError(ERROR_PROC_NOT_FOUND);
			loaded = false;
		}

		m_drvProcTable.DrvSwapLayerBuffers = m_icd.GetFunction<BOOL, HDC, UINT>("DrvSwapLayerBuffers");
		if (!m_drvProcTable.DrvSwapLayerBuffers)
		{
			GLGPUS_ASSERT_FALSE("Could not find 'DrvSwapLayerBuffers' in ICD");
			SetLastError(ERROR_PROC_NOT_FOUND);
			loaded = false;
		}

		m_getDrvProcRaw = reinterpret_cast<void* (*)(const char*)>(m_icd.GetSymbol("DrvGetProcAddress"));
		if (m_getDrvProcRaw == nullptr)
		{
			GLGPUS_ASSERT_FALSE("Could not find 'DrvGetProcAddress' in ICD");
			SetLastError(ERROR_PROC_NOT_FOUND);
			loaded = false;
		}

		m_drvProcTable.DrvValidateVersion = m_icd.GetFunction<BOOL, ULONG>("DrvValidateVersion");
		if (!m_drvProcTable.DrvValidateVersion)
		{
			GLGPUS_ASSERT_FALSE("Could not find 'DrvValidateVersion' in ICD");
			SetLastError(ERROR_PROC_NOT_FOUND);
			loaded = false;
		}

		m_drvProcTable.DrvSetCallbackProcs = m_icd.GetFunction<void, int, PROC*>("DrvSetCallbackProcs");
		if (!m_drvProcTable.DrvSetCallbackProcs)
		{
			GLGPUS_ASSERT_FALSE("Could not find 'DrvSetCallbackProcs' in ICD");
			SetLastError(ERROR_PROC_NOT_FOUND);
			loaded = false;
		}

		m_drvProcTable.DrvRealizeLayerPalette = m_icd.GetFunction<BOOL, HDC, int, BOOL>("DrvRealizeLayerPalette");
		if (!m_drvProcTable.DrvRealizeLayerPalette)
		{
			GLGPUS_ASSERT_FALSE("Could not find 'DrvRealizeLayerPalette' in ICD");
			SetLastError(ERROR_PROC_NOT_FOUND);
			loaded = false;
		}

		// DrvPresentBuffers is optional — not all ICDs export it.
		m_drvProcTable.DrvPresentBuffers = m_icd.GetFunction<BOOL, HDC, WGLPRESENTBUFFERSDATA*>("DrvPresentBuffers");

		return loaded;
	}

	void GlIcdDriver::Activate(HDC hdc, HGLRC hglrc, glgpus::PFN_SetProcTable pfn)
	{
		GLGPUS_AUTO_PROFILER_SCOPE();
		GLGPUS_ASSERT(m_drvProcTable.DrvSetContext, "DrvSetContext not loaded");
		m_lastProcTable = m_drvProcTable.DrvSetContext(hdc, hglrc, pfn);
	}

	glgpus::OpenGlDispatchTable GlIcdDriver::BuildDispatchTable(const glgpus::OpenGlDispatchTable& /*next*/)
	{
		GLGPUS_ASSERT(m_lastProcTable != nullptr, "Activate must be called before BuildDispatchTable");
		return glgpus::BuildDispatchTableFromIcd(m_lastProcTable->WglDispatchTable, m_getDrvProcRaw);
	}

	HGLRC GlIcdDriver::CreateContext(HDC hdc)
	{
		GLGPUS_AUTO_PROFILER_SCOPE();
		if (!m_drvProcTable.DrvCreateContext)
		{
			GLGPUS_ASSERT_FALSE("DrvCreateContext not loaded");
			return nullptr;
		}
		return m_drvProcTable.DrvCreateContext(hdc);
	}

	BOOL GlIcdDriver::DeleteContext(HGLRC hglrc)
	{
		GLGPUS_AUTO_PROFILER_SCOPE();
		if (!m_drvProcTable.DrvDeleteContext)
		{
			GLGPUS_ASSERT_FALSE("DrvDeleteContext not loaded");
			return FALSE;
		}
		return m_drvProcTable.DrvDeleteContext(hglrc);
	}

	BOOL GlIcdDriver::ReleaseContext(DHGLRC dhglrc)
	{
		GLGPUS_AUTO_PROFILER_SCOPE();
		if (!m_drvProcTable.DrvReleaseContext)
		{
			GLGPUS_ASSERT_FALSE("DrvReleaseContext not loaded");
			return FALSE;
		}
		return m_drvProcTable.DrvReleaseContext(dhglrc);
	}

	BOOL GlIcdDriver::ShareLists(HGLRC h1, HGLRC h2)
	{
		GLGPUS_AUTO_PROFILER_SCOPE();
		if (!m_drvProcTable.DrvShareLists)
		{
			GLGPUS_ASSERT_FALSE("DrvShareLists not loaded");
			return FALSE;
		}
		return m_drvProcTable.DrvShareLists(h1, h2);
	}

	BOOL GlIcdDriver::CopyContext(HGLRC src, HGLRC dst, UINT mask)
	{
		GLGPUS_AUTO_PROFILER_SCOPE();
		if (!m_drvProcTable.DrvCopyContext)
		{
			GLGPUS_ASSERT_FALSE("DrvCopyContext not loaded");
			return FALSE;
		}
		return m_drvProcTable.DrvCopyContext(src, dst, mask);
	}

	HGLRC GlIcdDriver::CreateLayerContext(HDC hdc, int layerPlane)
	{
		GLGPUS_AUTO_PROFILER_SCOPE();
		if (!m_drvProcTable.DrvCreateLayerContext)
		{
			GLGPUS_ASSERT_FALSE("DrvCreateLayerContext not loaded");
			return nullptr;
		}
		return m_drvProcTable.DrvCreateLayerContext(hdc, layerPlane);
	}

	BOOL GlIcdDriver::SetPixelFormat(HDC hdc, int fmt)
	{
		GLGPUS_AUTO_PROFILER_SCOPE();
		if (!m_drvProcTable.DrvSetPixelFormat)
		{
			GLGPUS_ASSERT_FALSE("DrvSetPixelFormat not loaded");
			return FALSE;
		}
		return m_drvProcTable.DrvSetPixelFormat(hdc, fmt);
	}

	int GlIcdDriver::DescribePixelFormat(HDC hdc, int fmt, UINT bytes, PIXELFORMATDESCRIPTOR* ppfd)
	{
		GLGPUS_AUTO_PROFILER_SCOPE();
		if (!m_drvProcTable.DrvDescribePixelFormat)
		{
			GLGPUS_ASSERT_FALSE("DrvDescribePixelFormat not loaded");
			return 0;
		}
		return m_drvProcTable.DrvDescribePixelFormat(hdc, fmt, bytes, ppfd);
	}

	BOOL GlIcdDriver::SwapBuffers(HDC hdc)
	{
		GLGPUS_AUTO_PROFILER_SCOPE();
		if (!m_drvProcTable.DrvSwapBuffers)
		{
			GLGPUS_ASSERT_FALSE("DrvSwapBuffers not loaded");
			return FALSE;
		}
		return m_drvProcTable.DrvSwapBuffers(hdc);
	}

	BOOL GlIcdDriver::SwapLayerBuffers(HDC hdc, UINT planes)
	{
		GLGPUS_AUTO_PROFILER_SCOPE();
		if (!m_drvProcTable.DrvSwapLayerBuffers)
		{
			GLGPUS_ASSERT_FALSE("DrvSwapLayerBuffers not loaded");
			return FALSE;
		}
		return m_drvProcTable.DrvSwapLayerBuffers(hdc, planes);
	}

	void* GlIcdDriver::GetProcAddress(LPCSTR func)
	{
		GLGPUS_AUTO_PROFILER_SCOPE();
		if (!m_getDrvProcRaw)
		{
			GLGPUS_ASSERT_FALSE("DrvGetProcAddress not loaded");
			return nullptr;
		}
		return m_getDrvProcRaw(func);
	}

	BOOL GlIcdDriver::ValidateVersion(ULONG version)
	{
		GLGPUS_AUTO_PROFILER_SCOPE();
		if (!m_drvProcTable.DrvValidateVersion)
		{
			GLGPUS_ASSERT_FALSE("DrvValidateVersion not loaded");
			return FALSE;
		}
		return m_drvProcTable.DrvValidateVersion(version);
	}

	void GlIcdDriver::SetCallbackProcs(int count, PROC* procs)
	{
		GLGPUS_AUTO_PROFILER_SCOPE();
		if (!m_drvProcTable.DrvSetCallbackProcs)
		{
			GLGPUS_ASSERT_FALSE("DrvSetCallbackProcs not loaded");
			return;
		}
		m_drvProcTable.DrvSetCallbackProcs(count, procs);
	}

	BOOL GlIcdDriver::RealizeLayerPalette(HDC hdc, int plane, BOOL bRealize)
	{
		GLGPUS_AUTO_PROFILER_SCOPE();
		if (!m_drvProcTable.DrvRealizeLayerPalette)
		{
			GLGPUS_ASSERT_FALSE("DrvRealizeLayerPalette not loaded");
			return FALSE;
		}
		return m_drvProcTable.DrvRealizeLayerPalette(hdc, plane, bRealize);
	}

	BOOL GlIcdDriver::PresentBuffers(HDC hdc, WGLPRESENTBUFFERSDATA* data)
	{
		GLGPUS_AUTO_PROFILER_SCOPE();
		if (!m_drvProcTable.DrvPresentBuffers)
			return FALSE;
		return m_drvProcTable.DrvPresentBuffers(hdc, data);
	}

	int GlIcdDriver::DescribeLayerPlane(HDC hdc, int pixelFormat, int layerPlane, UINT bytes, void* plpd)
	{
		GLGPUS_AUTO_PROFILER_SCOPE();
		if (!m_drvProcTable.DrvDescribeLayerPlane)
		{
			GLGPUS_ASSERT_FALSE("DrvDescribeLayerPlane not loaded");
			return -1;
		}
		return m_drvProcTable.DrvDescribeLayerPlane(hdc, pixelFormat, layerPlane, bytes, plpd);
	}

	BOOL GlIcdDriver::SetLayerPaletteEntries(HDC hdc, int pixelFormat, int layerPlane, int numEntries, const void* pe)
	{
		GLGPUS_AUTO_PROFILER_SCOPE();
		if (!m_drvProcTable.DrvSetLayerPaletteEntries)
		{
			GLGPUS_ASSERT_FALSE("DrvSetLayerPaletteEntries not loaded");
			return FALSE;
		}
		return m_drvProcTable.DrvSetLayerPaletteEntries(hdc, pixelFormat, layerPlane, numEntries, pe);
	}

	BOOL GlIcdDriver::GetLayerPaletteEntries(HDC hdc, int pixelFormat, int layerPlane, int numEntries, int* pe)
	{
		GLGPUS_AUTO_PROFILER_SCOPE();
		if (!m_drvProcTable.DrvGetLayerPaletteEntries)
		{
			GLGPUS_ASSERT_FALSE("DrvGetLayerPaletteEntries not loaded");
			return FALSE;
		}
		return m_drvProcTable.DrvGetLayerPaletteEntries(hdc, pixelFormat, layerPlane, numEntries, pe);
	}

} // namespace gl
