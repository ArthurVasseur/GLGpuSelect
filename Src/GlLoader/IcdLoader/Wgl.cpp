//
// Created by arthur on 23/04/2025.
//

#include "GlLoader/IcdLoader/Wgl.hpp"

#include <array>
#include <cstring>

#include <Concerto/Core/Assert.hpp>
#include <Concerto/Core/Error/Error.hpp>
#include <Concerto/Core/Types/Types.hpp>

#include "GlLoader/DeviceContext/DeviceContext.hpp"
#include "GlLoader/DeviceContext/Wgl/WglDeviceContext.hpp"
#include "GlLoader/IcdLoader/IcdLoader.hpp"

#ifdef CCT_PLATFORM_WINDOWS
#include <d3dkmthk.h>

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
			GLGPUS_ASSERT_FALSE("glgpusInstance is null");
			return nullptr;
		}

		auto* currentDevice = glgpusInstance->GetCurrentDeviceContextForCurrentThread();
		if (!currentDevice)
		{
			GLGPUS_ASSERT_FALSE("No current device context found for current thread");
			SetLastError(ERROR_INVALID_HANDLE);
			return nullptr;
		}

		if (currentDevice->DeviceContext->IsActiveOnCurrentThread() == false)
		{
			GLGPUS_ASSERT_FALSE("wglSwapBuffers called on a non-active context");
			SetLastError(ERROR_INVALID_HANDLE_STATE);
			return nullptr;
		}

		if (currentDevice->DeviceContext->GetPlatformDeviceContext() != hdc)
		{
			GLGPUS_ASSERT_FALSE("wglSwapBuffers called on a different device context than the one it was created with");
			SetLastError(ERROR_INVALID_HANDLE_STATE);
			return nullptr;
		}

		return currentDevice;
	}

	gl::GlDriver& GetDriver()
	{
		auto* instance = glgpus::IcdLoader::Instance();
		GLGPUS_ASSERT(instance != nullptr, "IcdLoader instance is null");
		return instance->GetDriver();
	}
} // namespace

static thread_local void* s_currentValue = nullptr;

extern "C" GLGPUS_API void CCT_CALL wglSetCurrentValue(void* value)
{
	s_currentValue = value;
}

extern "C" GLGPUS_API void* CCT_CALL wglGetCurrentValue()
{
	return s_currentValue;
}

extern "C" GLGPUS_API DHGLRC CCT_CALL wglGetDHGLRC(DHGLRC dhglrc)
{
	return dhglrc;
}

int wglChoosePixelFormat(HDC hdc, const PIXELFORMATDESCRIPTOR* ppfd)
{
	GLGPUS_AUTO_PROFILER_SCOPE();

	auto* glgpusInstance = glgpus::IcdLoader::Instance();
	if (glgpusInstance == nullptr)
	{
		GLGPUS_ASSERT_FALSE("glgpusInstance is null");
		return -1;
	}

	glgpusInstance->EnsureInitialized();
	if (!glgpusInstance->IsInitialized())
		return -1;

	// atio6axx.dll stores callbacks[2] and [4] unconditionally, and callbacks[5]-[8] when
	// count >= 7/8/9 respectively. With count=9, +0xc4a stays 1 (flip/HW-queue path) and
	// AMD calls through +0xc98 (callbacks[8]) without a null-check — crashing if null.
	static std::once_flag s_callbacksFlag;
	std::call_once(s_callbacksFlag, [&]()
				   {
		std::array<FARPROC, 9> callbacks = {
			reinterpret_cast<FARPROC>(wglSetCurrentValue),
			reinterpret_cast<FARPROC>(wglGetCurrentValue),
			reinterpret_cast<FARPROC>(wglGetDHGLRC),
			nullptr, // [3]: AMD never reads this slot
			nullptr, // [4]: DWM present (stub → fall back)
			nullptr, // [5]: context getter stub
			nullptr, // [6]: display config stub
			nullptr, // [7]: unknown stub
			nullptr, // [8]: D3DKMTSubmitPresentToHwQueue
		};
		GetDriver().SetCallbackProcs(static_cast<int>(callbacks.size()), callbacks.data()); });

	if (ppfd == nullptr)
		return 0;

	auto count = GetDriver().DescribePixelFormat(hdc, 0, 0, nullptr);
	int bestIndex = 0;
	int bestScore = std::numeric_limits<int>::max();

	for (long i = 1; i <= count; ++i)
	{
		PIXELFORMATDESCRIPTOR pixelFormatDescriptor;
		GetDriver().DescribePixelFormat(hdc, i, sizeof(PIXELFORMATDESCRIPTOR), &pixelFormatDescriptor);
		if (int score = ScorePFD(ppfd, &pixelFormatDescriptor); score < bestScore)
		{
			bestScore = score;
			bestIndex = i;
			if (score == 0)
				break; // perfect match
		}
	}

	glgpusInstance->SetSelectedPixelFormatIndex(hdc, bestIndex);
	return bestIndex;
}

int wglSetPixelFormat(HDC hdc, int format, [[maybe_unused]] const PIXELFORMATDESCRIPTOR* ppfd)
{
	GLGPUS_AUTO_PROFILER_SCOPE();

	auto* glgpusInstance = glgpus::IcdLoader::Instance();
	if (glgpusInstance == nullptr)
		return 0;
	glgpusInstance->EnsureInitialized();
	if (!glgpusInstance->IsInitialized())
		return 0;

	glgpusInstance->SetSelectedPixelFormatIndex(hdc, format);
	return GetDriver().SetPixelFormat(hdc, format);
}

int wglGetPixelFormat(HDC hdc)
{
	GLGPUS_AUTO_PROFILER_SCOPE();

	auto* instance = glgpus::IcdLoader::Instance();
	if (instance == nullptr)
		return 0;
	return instance->GetSelectedPixelFormatIndex(hdc);
}

int wglDescribePixelFormat(HDC hdc, int iPixelFormat, UINT nBytes, PIXELFORMATDESCRIPTOR* ppfd)
{
	GLGPUS_AUTO_PROFILER_SCOPE();

	return GetDriver().DescribePixelFormat(hdc, iPixelFormat, nBytes, ppfd);
}

HGLRC wglCreateContext(HDC hdc)
{
	GLGPUS_AUTO_PROFILER_SCOPE();

	// If SetPixelFormat (Win32) was called before our ICD was initialized,
	// DrvSetPixelFormat may not have been called yet. Catch up here.
	if (int fmt = wglGetPixelFormat(hdc); fmt != 0)
		GetDriver().SetPixelFormat(hdc, fmt);

	auto icdDeviceContext = GetDriver().CreateContext(hdc);

	if (icdDeviceContext == nullptr)
	{
		GLGPUS_ASSERT_FALSE("DrvCreateContext returned a null context");
		return nullptr;
	}

	auto* icdDeviceContextWrapper = new glgpus::IcdDeviceContextWrapper;
	icdDeviceContextWrapper->DeviceContext = new glgpus::WglDeviceContext(hdc, icdDeviceContext);

#ifdef GLGPUS_LOG_CONTEXT_MANIPULATION
	GLGPUS_LOG_WARN("wglCreateContext(hdc {}) -> HGLRC: {}", static_cast<void*>(hdc), static_cast<void*>(icdDeviceContext));
#endif

	return reinterpret_cast<HGLRC>(icdDeviceContextWrapper);
}

HGLRC wglCreateLayerContext(HDC hdc, int layerPlane)
{
	GLGPUS_AUTO_PROFILER_SCOPE();

	auto icdDeviceContext = GetDriver().CreateLayerContext(hdc, layerPlane);

	if (icdDeviceContext == nullptr)
	{
		GLGPUS_ASSERT_FALSE("DrvCreateLayerContext returned a null context");
		return nullptr;
	}

	auto* icdDeviceContextWrapper = new glgpus::IcdDeviceContextWrapper;
	icdDeviceContextWrapper->DeviceContext = new glgpus::WglDeviceContext(hdc, icdDeviceContext);

#ifdef GLGPUS_LOG_CONTEXT_MANIPULATION
	GLGPUS_LOG_WARN("wglCreateLayerContext(hdc {}, layerPlane {}) -> HGLRC: {}", static_cast<void*>(hdc), layerPlane, static_cast<void*>(icdDeviceContext));
#endif

	return reinterpret_cast<HGLRC>(icdDeviceContextWrapper);
}

BOOL wglDeleteContext(HGLRC hglrc)
{
	GLGPUS_AUTO_PROFILER_SCOPE();

	if (hglrc == nullptr)
	{
		GLGPUS_ASSERT_FALSE("Null context passed to wglDeleteContext");
		SetLastError(ERROR_INVALID_HANDLE);
		return false;
	}

	auto* icdDeviceContextWrapper = reinterpret_cast<glgpus::IcdDeviceContextWrapper*>(hglrc);

	if (icdDeviceContextWrapper->DeviceContext->IsActive())
	{
		if (!icdDeviceContextWrapper->DeviceContext->IsActiveOnCurrentThread())
		{
			GLGPUS_ASSERT_FALSE("Attempted to delete a context that is active on another thread");
			SetLastError(ERROR_BUSY);
			return false;
		}
		GLGPUS_LOG_WARN("Attempted to delete an active HGLRC ({}); automatically unbinding the context. "
						"Please ensure you call wglMakeCurrent(NULL, NULL) before deleting the context in your code.",
						reinterpret_cast<void*>(hglrc));
		if (!wglMakeCurrent(nullptr, nullptr))
			return false;
	}

#ifdef GLGPUS_LOG_CONTEXT_MANIPULATION
	GLGPUS_LOG_WARN("wglDeleteContext(hglrc {}) associated hdc: {}",
					icdDeviceContextWrapper->DeviceContext->GetIcdContext(),
					icdDeviceContextWrapper->DeviceContext->GetPlatformDeviceContext());
#endif

	bool result = icdDeviceContextWrapper->DeviceContext->DeleteContext();
	delete icdDeviceContextWrapper; // destructor handles DeviceContext

	return result;
}

void SetProcTable(const glgpus::GlProcTable*)
{
	// The dispatch table is retrieved from DrvSetContext's return value.
	// If an ICD calls this callback instead, we silently ignore it.
}

BOOL wglMakeCurrent(HDC hdc, HGLRC hglrc)
{
	GLGPUS_AUTO_PROFILER_SCOPE();

	auto* icdDeviceContextWrapper = reinterpret_cast<glgpus::IcdDeviceContextWrapper*>(hglrc);

	auto* glgpuInstance = glgpus::IcdLoader::Instance();
	if (glgpuInstance == nullptr)
	{
		GLGPUS_ASSERT_FALSE("glgpusInstance is null");
		return false;
	}

#ifdef GLGPUS_LOG_CONTEXT_MANIPULATION
	GLGPUS_LOG_WARN("wglMakeCurrent(hdc {}, hglrc: {})", static_cast<void*>(hdc),
					icdDeviceContextWrapper ? icdDeviceContextWrapper->DeviceContext->GetIcdContext() : nullptr);
#endif

	if (icdDeviceContextWrapper == nullptr)
	{
		if (glgpus::IcdDeviceContextWrapper* currentContext = glgpuInstance->GetCurrentDeviceContextForCurrentThread())
		{
			GetDriver().ReleaseContext(currentContext->DeviceContext->GetIcdContext());
		}
		wglSetCurrentValue(nullptr);
		glgpuInstance->ResetCurrentDeviceContextForCurrentThread();
		return true;
	}

	if (icdDeviceContextWrapper->DeviceContext->IsActiveOnCurrentThread())
		return true;

	if (icdDeviceContextWrapper->DeviceContext->IsActive())
	{
		GLGPUS_ASSERT_FALSE("wglMakeCurrent called on an already active context");
		return false;
	}

	auto icdCtx = static_cast<HGLRC>(icdDeviceContextWrapper->DeviceContext->GetIcdContext());
	GetDriver().Activate(hdc, icdCtx, SetProcTable);

	glgpuInstance->SetCurrentDeviceContextForCurrentThread(*icdDeviceContextWrapper);

	glgpus::OpenGlDispatchTable table = GetDriver().BuildDispatchTable({});
	for (auto* layer : glgpuInstance->GetLayers())
		table = layer->BuildDispatchTable(table);
	icdDeviceContextWrapper->DeviceContext->SetGlDispatchTable(table);

	return true;
}

HGLRC wglGetCurrentContext()
{
	GLGPUS_AUTO_PROFILER_SCOPE();

	auto* instance = glgpus::IcdLoader::Instance();
	if (instance == nullptr)
		return nullptr;
	auto* deviceContext = instance->GetCurrentDeviceContextForCurrentThread();
	if (deviceContext == nullptr)
		return nullptr;
	return reinterpret_cast<HGLRC>(deviceContext);
}

HDC wglGetCurrentDC()
{
	GLGPUS_AUTO_PROFILER_SCOPE();

	auto* instance = glgpus::IcdLoader::Instance();
	if (instance == nullptr)
		return nullptr;
	auto* deviceContext = instance->GetCurrentDeviceContextForCurrentThread();
	if (deviceContext == nullptr || deviceContext->DeviceContext == nullptr)
		return nullptr;
	return static_cast<HDC>(deviceContext->DeviceContext->GetPlatformDeviceContext());
}

BOOL wglShareLists(HGLRC hglrc1, HGLRC hglrc2)
{
	GLGPUS_AUTO_PROFILER_SCOPE();

	auto* icdDeviceContextWrapper1 = reinterpret_cast<glgpus::IcdDeviceContextWrapper*>(hglrc1);
	auto* icdDeviceContextWrapper2 = reinterpret_cast<glgpus::IcdDeviceContextWrapper*>(hglrc2);

	if (icdDeviceContextWrapper1 == nullptr || icdDeviceContextWrapper2 == nullptr)
	{
		GLGPUS_ASSERT_FALSE("Invalid HGLRC handle passed to wglShareLists");
		SetLastError(ERROR_INVALID_HANDLE);
		return false;
	}

	return icdDeviceContextWrapper1->DeviceContext->ShareLists(
		icdDeviceContextWrapper2->DeviceContext->GetIcdContext());
}

BOOL wglCopyContext(HGLRC hglrcSrc, HGLRC hglrcDst, UINT mask)
{
	GLGPUS_AUTO_PROFILER_SCOPE();

	auto* src = reinterpret_cast<glgpus::IcdDeviceContextWrapper*>(hglrcSrc);
	auto* dst = reinterpret_cast<glgpus::IcdDeviceContextWrapper*>(hglrcDst);

	if (src == nullptr || dst == nullptr)
	{
		GLGPUS_ASSERT_FALSE("Invalid HGLRC handle passed to wglCopyContext");
		SetLastError(ERROR_INVALID_HANDLE);
		return false;
	}

	return dst->DeviceContext->CopyContext(src->DeviceContext->GetIcdContext(), mask);
}

int wglDescribeLayerPlane(HDC hdc, int pixelFormat, int layerPlane, UINT nBytes, void* plpd)
{
	GLGPUS_AUTO_PROFILER_SCOPE();

	return GetDriver().DescribeLayerPlane(hdc, pixelFormat, layerPlane, nBytes, plpd);
}

BOOL wglRealizeLayerPalette(HDC hdc, int layerPlane, BOOL bRealize)
{
	GLGPUS_AUTO_PROFILER_SCOPE();

	return GetDriver().RealizeLayerPalette(hdc, layerPlane, bRealize);
}

int wglSetLayerPaletteEntries(HDC hdc, int layerPlane, int start, int numEntries, const void* pe)
{
	GLGPUS_AUTO_PROFILER_SCOPE();

	return GetDriver().SetLayerPaletteEntries(hdc, layerPlane, start, numEntries, pe);
}

int wglGetLayerPaletteEntries(HDC hdc, int layerPlane, int start, int numEntries, int* pcr)
{
	GLGPUS_AUTO_PROFILER_SCOPE();

	return GetDriver().GetLayerPaletteEntries(hdc, layerPlane, start, numEntries, pcr);
}

BOOL wglSwapBuffers(HDC hdc)
{
	GLGPUS_AUTO_PROFILER_SCOPE();

	if (SwapBuffersRuntimeCheck(hdc) == nullptr)
		return false;

	return GetDriver().SwapBuffers(hdc);
}

BOOL wglSwapLayerBuffers(HDC hdc, UINT fuPlanes)
{
	GLGPUS_AUTO_PROFILER_SCOPE();

	if (SwapBuffersRuntimeCheck(hdc) == nullptr)
		return false;

	return GetDriver().SwapLayerBuffers(hdc, fuPlanes);
}

void* wglGetProcAddress(LPCSTR lpszProc)
{
	GLGPUS_AUTO_PROFILER_SCOPE();

	if (const auto func = glgpus::GetFromDispatchTable(lpszProc))
		return func;

	if (const auto func = GetProcAddress(GetThisDllHandle(), lpszProc))
		return reinterpret_cast<void*>(func);

	if (const auto func = GetDriver().GetProcAddress(lpszProc))
		return func;

	return nullptr;
}

BOOL wglUseFontBitmaps(HDC hdc, DWORD first, DWORD count, DWORD listBase)
{
	GLGPUS_AUTO_PROFILER_SCOPE();

	GLGPUS_ASSERT_FALSE("Not implemented");
	return false;
}

BOOL wglUseFontBitmapsA(HDC hdc, DWORD first, DWORD count, DWORD listBase)
{
	return wglUseFontBitmaps(hdc, first, count, listBase);
}

BOOL wglUseFontBitmapsW(HDC hdc, DWORD first, DWORD count, DWORD listBase)
{
	GLGPUS_AUTO_PROFILER_SCOPE();
	GLGPUS_ASSERT_FALSE("Not implemented");

	return false;
}

BOOL wglUseFontOutlines(HDC hdc, DWORD first, DWORD count, DWORD listBase, FLOAT deviation, FLOAT extrusion, int format, void* lpgmf)
{
	GLGPUS_AUTO_PROFILER_SCOPE();

	GLGPUS_ASSERT_FALSE("Not implemented");
	return false;
}

BOOL wglUseFontOutlinesA(HDC hdc, DWORD first, DWORD count, DWORD listBase, FLOAT deviation, FLOAT extrusion, int format, void* lpgmf)
{
	return wglUseFontOutlines(hdc, first, count, listBase, deviation, extrusion, format, lpgmf);
}

BOOL wglUseFontOutlinesW(HDC hdc, DWORD first, DWORD count, DWORD listBase, FLOAT deviation, FLOAT extrusion, int format, void* lpgmf)
{
	GLGPUS_AUTO_PROFILER_SCOPE();
	GLGPUS_ASSERT_FALSE("Not implemented");

	return false;
}

PROC wglGetDefaultProcAddress(LPCSTR lpszProc)
{
	GLGPUS_AUTO_PROFILER_SCOPE();

	SetLastError(ERROR_PROC_NOT_FOUND);
	return nullptr;
}

DWORD wglSwapMultipleBuffers(UINT n, const WGLSWAP* ps)
{
	GLGPUS_AUTO_PROFILER_SCOPE();

	if (n > 16)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return 0;
	}

	DWORD failed = 0;
	for (UINT i = 0; i < n; ++i)
	{
		if (!wglSwapBuffers(ps[i].hdc))
			failed |= (1u << i);
	}
	return failed;
}

#endif
