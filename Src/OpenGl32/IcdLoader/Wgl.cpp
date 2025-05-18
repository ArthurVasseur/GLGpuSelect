//
// Created by arthur on 23/04/2025.
//

#include <array>
#include <Concerto/Core/Assert.hpp>
#include <Concerto/Core/Types.hpp>
#include <Concerto/Core/Error.hpp>

#include "OpenGl32/IcdLoader/Wgl.hpp"
#include "OpenGl32/DeviceContext/DeviceContext.hpp"
#include "OpenGl32/IcdLibrary/Wgl/WglIcdLibrary.hpp"
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
	glgpus::WglIcdLibrary& GetWglIcdLibrary()
	{
		return glgpus::IcdLoader::Instance()->GetPlatformIcd<glgpus::WglIcdLibrary>();
	}
}

GLGPUS_EXPORT void CCT_CALL wglSetCurrentValue(void* value)
{
	glgpus::IcdLoader::Instance()->SetCurrentValue(value);
}

GLGPUS_EXPORT void* CCT_CALL wglGetCurrentValue()
{
	return glgpus::IcdLoader::Instance()->GetCurrentValue();
}

GLGPUS_EXPORT DHGLRC CCT_CALL wglGetDHGLRC(glgpus::IcdDeviceContextWrapper* context)
{
	if (context)
		return context->IcdDeviceContext;
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

		if (GetWglIcdLibrary().DrvValidateVersion(selectedAdapter.openGlVersion) == 0)
			return glgpus::MakeResult(glgpus::glgpusResult::Unknown, "Invalid version");

		// Must follow WGLCALLBACKS in gldrv.h
		// On an ATI ICD, other values seems to not be supported, it makes the driver crash
		std::array callbacks = {
			reinterpret_cast<FARPROC>(wglSetCurrentValue),
			reinterpret_cast<FARPROC>(wglGetCurrentValue),
			reinterpret_cast<FARPROC>(wglGetDHGLRC),
		};

		GetWglIcdLibrary().DrvSetCallbackProcs(callbacks.size(), callbacks.data());
	}

	auto count = GetWglIcdLibrary().DrvDescribePixelFormat(hdc, 0, 0, nullptr);
	int bestIndex = 0;
	int bestScore = std::numeric_limits<int>::max();

	for (long i = 1; i <= count; ++i)
	{
		PIXELFORMATDESCRIPTOR pixelFormatDescriptor;
		GetWglIcdLibrary().DrvDescribePixelFormat(hdc, i, sizeof(PIXELFORMATDESCRIPTOR), &pixelFormatDescriptor);
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
	return GetWglIcdLibrary().DrvSetPixelFormat(hdc, format);
}

int wglGetPixelFormat(HDC hdc)
{
	return glgpus::IcdLoader::Instance()->GetSelectedPixelFormatIndex();
}

int wglDescribePixelFormat(HDC hdc, int iPixelFormat, UINT nBytes, PIXELFORMATDESCRIPTOR* ppfd)
{
	return GetWglIcdLibrary().DrvDescribePixelFormat(hdc, iPixelFormat, nBytes, ppfd);
}

HGLRC wglCreateContext(HDC hdc)
{
	auto icdDeviceContext = GetWglIcdLibrary().DrvCreateContext(hdc);

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
	auto icdDeviceContext = GetWglIcdLibrary().DrvCreateLayerContext(hdc, layerPlane);

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

#ifdef GLGPUS_LOG_CONTEXT_MANIPULATION
	cct::Logger::Warning("wglDeleteContext(hglrc {}) associated hdc: {}", icdDeviceContextWrapper->IcdDeviceContext, icdDeviceContextWrapper->DeviceContext->GetPlatformDeviceContext());
#endif

	bool result = GetWglIcdLibrary().DrvDeleteContext(static_cast<HGLRC>(icdDeviceContextWrapper->IcdDeviceContext));
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
			GetWglIcdLibrary().DrvReleaseContext(currentContext->IcdDeviceContext);
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

	const auto* dispatchTable = GetWglIcdLibrary().DrvSetContext(hdc, static_cast<HGLRC>(icdDeviceContextWrapper->IcdDeviceContext), SetProcTable);

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

	return GetWglIcdLibrary().DrvShareLists(static_cast<HGLRC>(icdDeviceContextWrapper1->IcdDeviceContext), static_cast<HGLRC>(icdDeviceContextWrapper2->IcdDeviceContext));
}

BOOL wglCopyContext(HGLRC hglrcSrc, HGLRC hglrcDst, UINT mask)
{
	return GetWglIcdLibrary().DrvCopyContext(hglrcSrc, hglrcDst, mask);
}

int  wglDescribeLayerPlane(HDC hdc, int pixelFormat, int layerPlane, UINT nBytes, void* plpd)
{
	return GetWglIcdLibrary().DrvDescribeLayerPlane(hdc, pixelFormat, layerPlane, nBytes, plpd);
}

BOOL wglRealizeLayerPalette(HDC hdc, int layerPlane, BOOL bRealize)
{
	return GetWglIcdLibrary().DrvRealizeLayerPalette(hdc, layerPlane, bRealize);
}

int  wglSetLayerPaletteEntries(HDC hdc, int pixelFormat, int layerPlane, int numEntries, const void* pe)
{
	return GetWglIcdLibrary().DrvSetLayerPaletteEntries(hdc, pixelFormat, layerPlane, numEntries, pe);
}

int  wglGetLayerPaletteEntries(HDC hdc, int pixelFormat, int layerPlane, int ne, int* pe)
{
	return GetWglIcdLibrary().DrvGetLayerPaletteEntries(hdc, pixelFormat, layerPlane, ne, pe);
}

BOOL wglSwapBuffers(HDC hdc)
{
	if (SwapBuffersRuntimeCheck(hdc) == nullptr)
		return false;

	return GetWglIcdLibrary().DrvSwapBuffers(hdc);
}

BOOL wglSwapLayerBuffers(HDC hdc, UINT fuPlanes)
{
	if (SwapBuffersRuntimeCheck(hdc) == nullptr)
		return false;

	return GetWglIcdLibrary().DrvSwapLayerBuffers(hdc, fuPlanes);
}

void* wglGetProcAddress(LPCSTR lpszProc)
{
	if (const auto func = glgpus::GetFromDispatchTable(lpszProc))
		return func;

	if (const auto func = GetProcAddress(GetThisDllHandle(), lpszProc))
		return reinterpret_cast<void*>(func);

	if (const auto func = GetWglIcdLibrary().DrvGetProcAddress(lpszProc))
		return func;

	return nullptr;
}

BOOL wglUseFontBitmaps(HDC hdc, DWORD first, DWORD count, DWORD listBase)
{
	CCT_ASSERT_FALSE("Not implemented");
	return false;
}

BOOL wglUseFontOutlines(HDC hdc, DWORD first, DWORD count, DWORD listBase, FLOAT deviation, FLOAT extrusion, int format, void* lpgmf)
{
	CCT_ASSERT_FALSE("Not implemented");
	return false;
}

#endif
