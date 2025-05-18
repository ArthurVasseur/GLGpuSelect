//
// Created by arthur on 23/04/2025.
//

#include <array>
#include <Concerto/Core/Assert.hpp>
#include <Concerto/Core/Types.hpp>
#include <Concerto/Core/Error.hpp>

#include "OpenGl32/IcdLoader/Wgl.hpp"
#include "OpenGl32/DeviceContext/DeviceContext.hpp"
#include "OpenGl32/IcdLoader/IcdLoader.hpp"

#ifdef CCT_PLATFORM_WINDOWS

namespace
{
	int ScorePFD(const PIXELFORMATDESCRIPTOR* want, const PIXELFORMATDESCRIPTOR* have)
	{
		int score = 0;

		score += abs(want->cDepthBits - have->cDepthBits) * 10;
		score += abs(want->cRedBits - have->cRedBits) * 1;
		score += abs(want->cGreenBits - have->cGreenBits) * 1;
		score += abs(want->cBlueBits - have->cBlueBits) * 1;

		if (!!(want->dwFlags & PFD_DOUBLEBUFFER) != !!(have->dwFlags & PFD_DOUBLEBUFFER))
			score += 1000;
		if (!!(want->dwFlags & PFD_SUPPORT_OPENGL) != !!(have->dwFlags & PFD_SUPPORT_OPENGL))
			score += 1000;
		return score;
	}

	HMODULE GetThisDllHandle()
	{
		MEMORY_BASIC_INFORMATION info;
		std::size_t len = VirtualQueryEx(GetCurrentProcess(), (void*)GetThisDllHandle, &info, sizeof(info));
		return len ? static_cast<HMODULE>(info.AllocationBase) : nullptr;
	}

	glgpus::IcdDeviceContextWrapper* SwapBuffersRuntimeCheck(HDC hdc)
	{
		auto* glgpusInstance = glgpus::IcdLoader::Instance();
		if (glgpusInstance == nullptr)
		{
			CCT_ASSERT_FALSE("glgpusInstance is null");
			return nullptr;
		}

		auto* currentDevice = glgpusInstance->GetCurrentDeviceContextForCurrentThread();
		if (!currentDevice)
		{
			CCT_ASSERT_FALSE("No current device context found for current thread");
			SetLastError(ERROR_INVALID_HANDLE);
			return nullptr;
		}

		if (currentDevice->DeviceContext->IsActiveOnCurrentThread() == false)
		{
			CCT_ASSERT_FALSE("wglSwapBuffers called on a non-active context");
			SetLastError(ERROR_INVALID_HANDLE_STATE);
			return nullptr;
		}

		if (currentDevice->DeviceContext->GetPlatformDeviceContext() != hdc)
		{
			CCT_ASSERT_FALSE("wglSwapBuffers called on a different device context than the one it was created with");
			SetLastError(ERROR_INVALID_HANDLE_STATE);
			return nullptr;
		}

		return currentDevice;
	}
}

GLGPUS_EXPORT void CCT_CALL wglSetCurrentValue(void* value)
{
	CCT_ASSERT_FALSE("Not implemented");
	return;
}

GLGPUS_EXPORT void* CCT_CALL wglGetCurrentValue()
{
	auto wglGetCurrentValuePFN = glgpus::IcdLoader::Instance()->GetIcd().GetFunction<void*>("DrvGetCurrentValue");
	if (!wglGetCurrentValuePFN)
	{
		CCT_ASSERT_FALSE("Could not find wglGetCurrentValuePFN in ICD");
		SetLastError(ERROR_PROC_NOT_FOUND);
		return nullptr;
	}
	return wglGetCurrentValuePFN();
}

GLGPUS_EXPORT DHGLRC CCT_CALL wglGetDHGLRC(glgpus::IcdDeviceContextWrapper* context)
{
	if (context)
		return context->IcdDeviceContext;

	auto wglGetDHGLRCPFN = glgpus::IcdLoader::Instance()->GetIcd().GetFunction<DHGLRC, void*>("DrvGetDHGLRC");
	if (wglGetDHGLRCPFN)
		return wglGetDHGLRCPFN(context);

	CCT_ASSERT_FALSE("Could not find DHGLRC");

	return nullptr;
}

GLGPUS_EXPORT int CCT_CALL wglChoosePixelFormat(HDC hdc, const PIXELFORMATDESCRIPTOR* ppfd)
{
	auto* glgpusInstance = glgpus::IcdLoader::Instance();
	if (glgpusInstance == nullptr)
	{
		CCT_ASSERT_FALSE("glgpusInstance is null");
		return -1;
	}

	if (glgpusInstance->GetIcd().IsLoaded() == false)
	{
		glgpusInstance->EnumerateAdapters(nullptr, nullptr); // force enumeration of adapters
		const auto& adapters = glgpusInstance->GetAdapterInfos();
		if (adapters.empty())
		{
			CCT_ASSERT_FALSE("No adapters found");
			return -1;
		}

		std::size_t adapterIndex = 0;
		if (const auto* value = std::getenv("GLGPUS_ADAPTER_OS_INDEX"))
		{
			adapterIndex = std::stoi(value);
		}
		auto& selectedAdapter = adapters[adapterIndex];
		if (glgpusInstance->ChooseDevice(selectedAdapter.Uuid) != 0)
		{
			CCT_ASSERT_FALSE("Failed to choose device");
			return -1;
		}

		auto& icd = glgpusInstance->GetIcd();
		auto DrvValidateVersionFunc = icd.GetFunction<BOOL, ULONG>("DrvValidateVersion");

		if (!DrvValidateVersionFunc)
			return glgpus::MakeResult(glgpus::glgpusResult::Unknown, "Invalid icd implementation");

		if (DrvValidateVersionFunc(selectedAdapter.openGlVersion) == 0)
			return glgpus::MakeResult(glgpus::glgpusResult::Unknown, "Invalid version");

		auto DrvSetCallbackProcsFunc = icd.GetFunction<void, int, PROC*>("DrvSetCallbackProcs");

		if (!DrvSetCallbackProcsFunc)
			return glgpus::MakeResult(glgpus::glgpusResult::Unknown, "Invalid icd implementation");

		// Must follow WGLCALLBACKS in gldrv.h
		// On an ATI ICD, other values seems to not be supported, it makes the driver crash
		std::array callbacks = {
			reinterpret_cast<FARPROC>(wglSetCurrentValue),
			reinterpret_cast<FARPROC>(wglGetCurrentValue),
			reinterpret_cast<FARPROC>(wglGetDHGLRC),
		};

		DrvSetCallbackProcsFunc(callbacks.size(), callbacks.data());
	}

	auto drvDescribePixelFormatPFN = glgpus::IcdLoader::Instance()->GetIcd().GetFunction<LONG, HDC, INT, ULONG, PIXELFORMATDESCRIPTOR*>("DrvDescribePixelFormat");
	if (!drvDescribePixelFormatPFN)
	{
		CCT_ASSERT_FALSE("Could not find drvDescribePixelFormatPFN in ICD");
		SetLastError(ERROR_PROC_NOT_FOUND);
		return -1;
	}

	auto count = drvDescribePixelFormatPFN(hdc, 0, 0, nullptr);
	int bestIndex = 0;
	int bestScore = std::numeric_limits<int>::max();

	for (long i = 1; i <= count; ++i)
	{
		PIXELFORMATDESCRIPTOR pixelFormatDescriptor;
		drvDescribePixelFormatPFN(hdc, i, sizeof(PIXELFORMATDESCRIPTOR), &pixelFormatDescriptor);
		if (int score = ScorePFD(ppfd, &pixelFormatDescriptor); score < bestScore)
		{
			bestScore = score;
			bestIndex = i;
			if (score == 0)
				break;  // perfect match
		}
	}

	glgpus::IcdLoader::Instance()->SetSelectedPixelFormatIndex(bestIndex);

	return bestIndex;
}

int wglSetPixelFormat(HDC hdc, int format, [[maybe_unused]] const PIXELFORMATDESCRIPTOR* ppfd)
{
	auto DrvSetPixelFormatPFN = glgpus::IcdLoader::Instance()->GetIcd().GetFunction<BOOL, HDC, int>("DrvSetPixelFormat");
	if (!DrvSetPixelFormatPFN)
	{
		CCT_ASSERT_FALSE("Could not find DrvSetPixelFormatPFN in ICD");
		SetLastError(ERROR_PROC_NOT_FOUND);
		return -1;
	}
	return DrvSetPixelFormatPFN(hdc, format);
}

int wglGetPixelFormat(HDC hdc)
{
	return glgpus::IcdLoader::Instance()->GetSelectedPixelFormatIndex();
}

int wglDescribePixelFormat(HDC hdc, int iPixelFormat, UINT nBytes, PIXELFORMATDESCRIPTOR* ppfd)
{
	auto DrvDescribePixelFormatPFN = glgpus::IcdLoader::Instance()->GetIcd().GetFunction<int, HDC, int, UINT, PIXELFORMATDESCRIPTOR*>("DrvDescribePixelFormat");
	if (!DrvDescribePixelFormatPFN)
	{
		CCT_ASSERT_FALSE("Could not find DrvDescribePixelFormatPFN in ICD");
		SetLastError(ERROR_PROC_NOT_FOUND);
		return -1;
	}
	return DrvDescribePixelFormatPFN(hdc, iPixelFormat, nBytes, ppfd);
}

HGLRC wglCreateContext(HDC hdc)
{
	auto wglCreateContextPFN = glgpus::IcdLoader::Instance()->GetIcd().GetFunction<HGLRC, HDC>("DrvCreateContext");
	if (!wglCreateContextPFN)
	{
		CCT_ASSERT_FALSE("Could not find wglCreateContextPFN in ICD");
		SetLastError(ERROR_PROC_NOT_FOUND);
		return nullptr;
	}

	auto icdDeviceContext = wglCreateContextPFN(hdc);

	if (icdDeviceContext == nullptr)
	{
		CCT_ASSERT_FALSE("DrvCreateContext returned a null context");
		return nullptr;
	}

	auto* icdDeviceContextWrapper = new glgpus::IcdDeviceContextWrapper;
	icdDeviceContextWrapper->DeviceContext = new glgpus::DeviceContext(hdc);
	icdDeviceContextWrapper->IcdDeviceContext = icdDeviceContext;

#ifdef GLGPUS_LOG_CONTEXT_MANIPULATION
	cct::Logger::Warning("wglCreateContext(hdc {}) -> HGLRC: {}", static_cast<void*>(hdc), static_cast<void*>(icdDeviceContext));
#endif

	return reinterpret_cast<HGLRC>(icdDeviceContextWrapper);
}

HGLRC wglCreateLayerContext(HDC hdc, int layerPlane)
{
	auto wglCreateLayerContextPFN = glgpus::IcdLoader::Instance()->GetIcd().GetFunction<HGLRC, HDC, int>("DrvCreateLayerContext");
	if (!wglCreateLayerContextPFN)
	{
		CCT_ASSERT_FALSE("Could not find wglCreateLayerContextPFN in ICD");
		SetLastError(ERROR_PROC_NOT_FOUND);
		return nullptr;
	}

	auto icdDeviceContext = wglCreateLayerContextPFN(hdc, layerPlane);

	if (icdDeviceContext == nullptr)
	{
		CCT_ASSERT_FALSE("DrvCreateLayerContext returned a null context");
		return nullptr;
	}

	auto* icdDeviceContextWrapper = new glgpus::IcdDeviceContextWrapper;
	icdDeviceContextWrapper->DeviceContext = new glgpus::DeviceContext(hdc);
	icdDeviceContextWrapper->IcdDeviceContext = icdDeviceContext;


#ifdef GLGPUS_LOG_CONTEXT_MANIPULATION
	cct::Logger::Warning("wglCreateLayerContext(hdc {}, layerPlane {}) -> HGLRC: {}", static_cast<void*>(hdc), layerPlane, static_cast<void*>(icdDeviceContext));
#endif

	return reinterpret_cast<HGLRC>(icdDeviceContextWrapper);
}

BOOL wglDeleteContext(HGLRC hglrc)
{
	if (hglrc == nullptr)
	{
		CCT_ASSERT_FALSE("Null context passed to wglDeleteContext");
		SetLastError(ERROR_INVALID_HANDLE);
		return false;
	}

	auto* icdDeviceContextWrapper = reinterpret_cast<glgpus::IcdDeviceContextWrapper*>(hglrc);
	if (icdDeviceContextWrapper == nullptr)
	{
		CCT_ASSERT_FALSE("Invalid HGLRC handle passed to wglDeleteContext");
		SetLastError(ERROR_INVALID_HANDLE);
		return false;
	}

	if (icdDeviceContextWrapper->DeviceContext->IsActive())
	{
		cct::Logger::Warning("Attempted to delete an active HGLRC; automatically unbinding the context. "
			"Please ensure you call wglMakeCurrent(NULL, NULL) before deleting the context in your code.");
		if (!wglMakeCurrent(nullptr, nullptr))
			return false;
	}

	auto wglDeleteContextPFN = glgpus::IcdLoader::Instance()->GetIcd().GetFunction<BOOL, HGLRC>("DrvDeleteContext");
	if (!wglDeleteContextPFN)
	{
		CCT_ASSERT_FALSE("Could not find DrvDeleteContext in ICD");
		SetLastError(ERROR_PROC_NOT_FOUND);
		return false;
	}

#ifdef GLGPUS_LOG_CONTEXT_MANIPULATION
	cct::Logger::Warning("wglDeleteContext(hglrc {}) associated hdc: {}", icdDeviceContextWrapper->IcdDeviceContext, icdDeviceContextWrapper->DeviceContext->GetPlatformDeviceContext());
#endif

	bool result = wglDeleteContextPFN(static_cast<HGLRC>(icdDeviceContextWrapper->IcdDeviceContext));
	delete icdDeviceContextWrapper->DeviceContext;
	delete icdDeviceContextWrapper;

	return result;
}

void SetProcTable(const glgpus::GlProcTable*)
{
	CCT_BREAK_IN_DEBUGGER;
}

BOOL wglMakeCurrent(HDC hdc, HGLRC hglrc)
{
	auto* icdDeviceContextWrapper = reinterpret_cast<glgpus::IcdDeviceContextWrapper*>(hglrc);

	auto* glgpuInstance = glgpus::IcdLoader::Instance();
	if (glgpuInstance == nullptr)
	{
		CCT_ASSERT_FALSE("glgpusInstance is null");
		return false;
	}

#ifdef GLGPUS_LOG_CONTEXT_MANIPULATION
	cct::Logger::Warning("wglMakeCurrent(hdc {}, hglrc: {})", static_cast<void*>(hdc), icdDeviceContextWrapper ? icdDeviceContextWrapper->IcdDeviceContext : nullptr);
#endif

	if (icdDeviceContextWrapper == nullptr)
	{
		if (glgpus::IcdDeviceContextWrapper* currentContext = glgpuInstance->GetCurrentDeviceContextForCurrentThread())
		{
			auto DrvReleaseContextPFN = glgpuInstance->GetIcd().GetFunction<BOOL, DHGLRC>("DrvReleaseContext");
			if (!DrvReleaseContextPFN)
			{
				CCT_ASSERT_FALSE("Could not find DrvSetContextPFN in ICD");
				SetLastError(ERROR_PROC_NOT_FOUND);
				return false;
			}
			DrvReleaseContextPFN(currentContext->IcdDeviceContext);
		}
		glgpuInstance->ResetCurrentDeviceContextForCurrentThread();
		return true;
	}

	if (icdDeviceContextWrapper->DeviceContext->IsActiveOnCurrentThread())
		return true;

	if (icdDeviceContextWrapper->DeviceContext->IsActive())
	{
		CCT_ASSERT_FALSE("wglMakeCurrent called on an already active context");
		return false;
	}

	auto DrvSetContextPFN = glgpuInstance->GetIcd().GetFunction<const glgpus::GlProcTable*, HDC, HGLRC, glgpus::PFN_SetProcTable>("DrvSetContext");
	if (!DrvSetContextPFN)
	{
		CCT_ASSERT_FALSE("Could not find DrvSetContextPFN in ICD");
		SetLastError(ERROR_PROC_NOT_FOUND);
		return false;
	}

	const auto* dispatchTable = DrvSetContextPFN(hdc, static_cast<HGLRC>(icdDeviceContextWrapper->IcdDeviceContext), SetProcTable);

	if (!dispatchTable)
	{
		CCT_ASSERT_FALSE("DrvSetContext failed");
		return false;
	}

	glgpuInstance->SetCurrentDeviceContextForCurrentThread(*icdDeviceContextWrapper);
	if (icdDeviceContextWrapper->DeviceContext)
		icdDeviceContextWrapper->DeviceContext->SetGlDispatchTable(dispatchTable->glDispatchTable);

	return true;
}

HGLRC wglGetCurrentContext()
{
	auto* deviceContext = glgpus::IcdLoader::Instance()->GetCurrentDeviceContextForCurrentThread();
	if (deviceContext == nullptr)
		return nullptr;
	return reinterpret_cast<HGLRC>(deviceContext);
}

HDC  wglGetCurrentDC()
{
	auto* deviceContext = glgpus::IcdLoader::Instance()->GetCurrentDeviceContextForCurrentThread();
	if (deviceContext == nullptr || deviceContext->DeviceContext == nullptr)
		return nullptr;
	return static_cast<HDC>(deviceContext->DeviceContext->GetPlatformDeviceContext());
}

BOOL wglShareLists(HGLRC hglrc1, HGLRC hglrc2)
{
	auto* icdDeviceContextWrapper1 = reinterpret_cast<glgpus::IcdDeviceContextWrapper*>(hglrc1);
	auto* icdDeviceContextWrapper2 = reinterpret_cast<glgpus::IcdDeviceContextWrapper*>(hglrc2);

	if (icdDeviceContextWrapper1 == nullptr || icdDeviceContextWrapper2 == nullptr)
	{
		CCT_ASSERT_FALSE("Invalid HGLRC handle passed to wglShareLists");
		SetLastError(ERROR_INVALID_HANDLE);
		return false;
	}

	auto wglShareListsPFN = glgpus::IcdLoader::Instance()->GetIcd().GetFunction<BOOL, HGLRC, HGLRC>("DrvShareLists");
	if (!wglShareListsPFN)
	{
		CCT_ASSERT_FALSE("Could not find wglShareListsPFN in ICD");
		SetLastError(ERROR_PROC_NOT_FOUND);
		return 0;
	}

	return wglShareListsPFN(static_cast<HGLRC>(icdDeviceContextWrapper1->IcdDeviceContext), static_cast<HGLRC>(icdDeviceContextWrapper2->IcdDeviceContext));
}

BOOL wglCopyContext(HGLRC hglrcSrc, HGLRC hglrcDst, UINT mask)
{
	auto wglCopyContextPFN = glgpus::IcdLoader::Instance()->GetIcd().GetFunction<BOOL, HGLRC, HGLRC, UINT>("DrvCopyContext");
	if (!wglCopyContextPFN)
	{
		CCT_ASSERT_FALSE("Could not find wglCopyContextPFN in ICD");
		SetLastError(ERROR_PROC_NOT_FOUND);
		return 0;
	}
	return wglCopyContextPFN(hglrcSrc, hglrcDst, mask);
}

int  wglDescribeLayerPlane(HDC hdc, int pixelFormat, int layerPlane, UINT nBytes, void* plpd)
{
	auto wglDescribeLayerPlanePFN = glgpus::IcdLoader::Instance()->GetIcd().GetFunction<int, HDC, int, int, UINT, void*>("DrvDescribeLayerPlane");
	if (!wglDescribeLayerPlanePFN)
	{
		CCT_ASSERT_FALSE("Could not find wglDescribeLayerPlanePFN in ICD");
		SetLastError(ERROR_PROC_NOT_FOUND);
		return 0;
	}
	return wglDescribeLayerPlanePFN(hdc, pixelFormat, layerPlane, nBytes, plpd);
}

BOOL wglRealizeLayerPalette(HDC hdc, int layerPlane, BOOL bRealize)
{
	auto wglRealizeLayerPalettePFN = glgpus::IcdLoader::Instance()->GetIcd().GetFunction<BOOL, HDC, int, BOOL>("DrvRealizeLayerPalette");
	if (!wglRealizeLayerPalettePFN)
	{
		CCT_ASSERT_FALSE("Could not find wglRealizeLayerPalettePFN in ICD");
		SetLastError(ERROR_PROC_NOT_FOUND);
		return 0;
	}
	return wglRealizeLayerPalettePFN(hdc, layerPlane, bRealize);
}

int  wglSetLayerPaletteEntries(HDC hdc, int pixelFormat, int layerPlane, int numEntries, const void* pe)
{
	auto wglSetLayerPaletteEntriesPFN = glgpus::IcdLoader::Instance()->GetIcd().GetFunction<int, HDC, int, int, int, const void*>("DrvSetLayerPaletteEntries");
	if (!wglSetLayerPaletteEntriesPFN)
	{
		CCT_ASSERT_FALSE("Could not find wglSetLayerPaletteEntriesPFN in ICD");
		SetLastError(ERROR_PROC_NOT_FOUND);
		return 0;
	}
	return wglSetLayerPaletteEntriesPFN(hdc, pixelFormat, layerPlane, numEntries, pe);
}

int  wglGetLayerPaletteEntries(HDC hdc, int pixelFormat, int layerPlane, int ne, int* pe)
{
	auto wglGetLayerPaletteEntriesPFN = glgpus::IcdLoader::Instance()->GetIcd().GetFunction<int, HDC, int, int, int, int*>("DrvGetLayerPaletteEntries");
	if (!wglGetLayerPaletteEntriesPFN)
	{
		CCT_ASSERT_FALSE("Could not find wglGetLayerPaletteEntriesPFN in ICD");
		SetLastError(ERROR_PROC_NOT_FOUND);
		return 0;
	}
	return wglGetLayerPaletteEntriesPFN(hdc, pixelFormat, layerPlane, ne, pe);
}

BOOL wglSwapBuffers(HDC hdc)
{
	if (SwapBuffersRuntimeCheck(hdc) == nullptr)
		return false;

	auto wglSwapBuffersPFN = glgpus::IcdLoader::Instance()->GetIcd().GetFunction<BOOL, HDC>("DrvSwapBuffers");
	if (!wglSwapBuffersPFN)
	{
		CCT_ASSERT_FALSE("Could not find wglSwapBuffersPFN in ICD");
		SetLastError(ERROR_PROC_NOT_FOUND);
		return 0;
	}

	return wglSwapBuffersPFN(hdc);
}

BOOL wglSwapLayerBuffers(HDC hdc, UINT fuPlanes)
{
	if (SwapBuffersRuntimeCheck(hdc) == nullptr)
		return false;

	auto wglSwapLayerBuffersPFN = glgpus::IcdLoader::Instance()->GetIcd().GetFunction<BOOL, HDC, UINT>("DrvSwapLayerBuffers");
	if (!wglSwapLayerBuffersPFN)
	{
		CCT_ASSERT_FALSE("Could not find wglSwapLayerBuffersPFN in ICD");
		SetLastError(ERROR_PROC_NOT_FOUND);
		return 0;
	}
	return wglSwapLayerBuffersPFN(hdc, fuPlanes);
}

void* wglGetProcAddress(LPCSTR lpszProc)
{
	if (const auto func = glgpus::GetFromDispatchTable(lpszProc))
		return func;

	if (const auto func = GetProcAddress(GetThisDllHandle(), lpszProc))
		return reinterpret_cast<void*>(func);

	auto wglGetProcAddressPFN = glgpus::IcdLoader::Instance()->GetIcd().GetFunction<void*, LPCSTR>("DrvGetProcAddress");
	if (!wglGetProcAddressPFN)
	{
		CCT_ASSERT_FALSE("Could not find wglGetProcAddressPFN in ICD");
		SetLastError(ERROR_PROC_NOT_FOUND);
		return nullptr;
	}

	if (const auto func = wglGetProcAddressPFN(lpszProc))
		return func;

	return nullptr;
}

BOOL wglUseFontBitmaps(HDC hdc, DWORD first, DWORD count, DWORD listBase)
{
	auto wglUseFontBitmapsPFN = glgpus::IcdLoader::Instance()->GetIcd().GetFunction<BOOL, HDC, DWORD, DWORD, DWORD>("DrvUseFontBitmaps");
	if (!wglUseFontBitmapsPFN)
	{
		CCT_ASSERT_FALSE("Could not find wglUseFontBitmapsPFN in ICD");
		SetLastError(ERROR_PROC_NOT_FOUND);
		return 0;
	}
	return wglUseFontBitmapsPFN(hdc, first, count, listBase);
}

BOOL wglUseFontOutlines(HDC hdc, DWORD first, DWORD count, DWORD listBase, FLOAT deviation, FLOAT extrusion, int format, void* lpgmf)
{
	auto wglUseFontOutlinesPFN = glgpus::IcdLoader::Instance()->GetIcd().GetFunction<BOOL, HDC, DWORD, DWORD, DWORD, FLOAT, FLOAT, int, void*>("DrvUseFontOutlines");
	if (!wglUseFontOutlinesPFN)
	{
		CCT_ASSERT_FALSE("Could not find wglUseFontOutlinesPFN in ICD");
		SetLastError(ERROR_PROC_NOT_FOUND);
		return 0;
	}
	return wglUseFontOutlinesPFN(hdc, first, count, listBase, deviation, extrusion, format, lpgmf);
}

#endif
