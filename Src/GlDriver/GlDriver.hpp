#pragma once
#include "GlLayer/GlLayer.hpp"

#ifdef CCT_PLATFORM_WINDOWS
#include <string_view>

#include "GlLoader/IcdLoader/Wgl.hpp"

namespace gl
{

	class GlDriver : public GlLayer
	{
	public:
		virtual ~GlDriver() = default;
		virtual bool Load(std::string_view icdPath) = 0;
		virtual HGLRC CreateContext(HDC hdc) = 0;
		virtual BOOL DeleteContext(HGLRC hglrc) = 0;
		virtual void Activate(HDC hdc, HGLRC hglrc, glgpus::PFN_SetProcTable pfn) = 0;
		virtual BOOL ReleaseContext(DHGLRC dhglrc) = 0;
		virtual BOOL ShareLists(HGLRC h1, HGLRC h2) = 0;
		virtual BOOL CopyContext(HGLRC src, HGLRC dst, UINT mask) = 0;
		virtual HGLRC CreateLayerContext(HDC hdc, int layerPlane) = 0;
		virtual BOOL SetPixelFormat(HDC hdc, int fmt) = 0;
		virtual int DescribePixelFormat(HDC hdc, int fmt, UINT bytes, PIXELFORMATDESCRIPTOR* ppfd) = 0;
		virtual BOOL SwapBuffers(HDC hdc) = 0;
		virtual BOOL SwapLayerBuffers(HDC hdc, UINT planes) = 0;
		virtual void* GetProcAddress(LPCSTR func) = 0;
		virtual BOOL ValidateVersion(ULONG version) = 0;
		virtual void SetCallbackProcs(int count, PROC* procs) = 0;
		virtual BOOL RealizeLayerPalette(HDC hdc, int plane, BOOL bRealize) = 0;
		virtual BOOL PresentBuffers(HDC hdc, WGLPRESENTBUFFERSDATA* data) = 0;
		virtual int DescribeLayerPlane(HDC hdc, int pixelFormat, int layerPlane, UINT bytes, void* plpd) = 0;
		virtual BOOL SetLayerPaletteEntries(HDC hdc, int pixelFormat, int layerPlane, int numEntries, const void* pe) = 0;
		virtual BOOL GetLayerPaletteEntries(HDC hdc, int pixelFormat, int layerPlane, int numEntries, int* pe) = 0;
		glgpus::OpenGlDispatchTable BuildDispatchTable(const glgpus::OpenGlDispatchTable& next) override = 0;
	};

} // namespace gl
#endif // CCT_PLATFORM_WINDOWS
