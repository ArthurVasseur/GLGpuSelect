#pragma once
#include "GlDriver/GlDriver.hpp"

#ifdef CCT_PLATFORM_WINDOWS
#include <Concerto/Core/DynLib/DynLib.hpp>

namespace gl {

class GlIcdDriver final : public GlDriver {
public:
    GlIcdDriver() = default;

    bool Load(std::string_view icdPath) override;
    HGLRC CreateContext(HDC hdc) override;
    BOOL DeleteContext(HGLRC hglrc) override;
    void Activate(HDC hdc, HGLRC hglrc, glgpus::PFN_SetProcTable pfn) override;
    BOOL ReleaseContext(DHGLRC dhglrc) override;
    BOOL ShareLists(HGLRC h1, HGLRC h2) override;
    BOOL CopyContext(HGLRC src, HGLRC dst, UINT mask) override;
    HGLRC CreateLayerContext(HDC hdc, int layerPlane) override;
    BOOL SetPixelFormat(HDC hdc, int fmt) override;
    int DescribePixelFormat(HDC hdc, int fmt, UINT bytes, PIXELFORMATDESCRIPTOR* ppfd) override;
    BOOL SwapBuffers(HDC hdc) override;
    BOOL SwapLayerBuffers(HDC hdc, UINT planes) override;
    void* GetProcAddress(LPCSTR func) override;
    BOOL ValidateVersion(ULONG version) override;
    void SetCallbackProcs(int count, PROC* procs) override;
    BOOL RealizeLayerPalette(HDC hdc, int plane, BOOL bRealize) override;
    BOOL PresentBuffers(HDC hdc, WGLPRESENTBUFFERSDATA* data) override;
    int DescribeLayerPlane(HDC hdc, int pixelFormat, int layerPlane, UINT bytes, void* plpd) override;
    BOOL SetLayerPaletteEntries(HDC hdc, int pixelFormat, int layerPlane, int numEntries, const void* pe) override;
    BOOL GetLayerPaletteEntries(HDC hdc, int pixelFormat, int layerPlane, int numEntries, int* pe) override;
    glgpus::OpenGlDispatchTable BuildDispatchTable(const glgpus::OpenGlDispatchTable& next) override;

private:
    struct DrvProcTable {
        cct::FunctionRef<BOOL(HDC, int)> DrvSetPixelFormat;
        cct::FunctionRef<int(HDC, int, UINT, PIXELFORMATDESCRIPTOR*)> DrvDescribePixelFormat;
        cct::FunctionRef<HGLRC(HDC)> DrvCreateContext;
        cct::FunctionRef<HGLRC(HDC, int)> DrvCreateLayerContext;
        cct::FunctionRef<BOOL(HGLRC)> DrvDeleteContext;
        cct::FunctionRef<BOOL(DHGLRC)> DrvReleaseContext;
        cct::FunctionRef<const glgpus::GlProcTable*(HDC, HGLRC, glgpus::PFN_SetProcTable)> DrvSetContext;
        cct::FunctionRef<BOOL(HGLRC, HGLRC)> DrvShareLists;
        cct::FunctionRef<BOOL(HGLRC, HGLRC, UINT)> DrvCopyContext;
        cct::FunctionRef<int(HDC, int, int, UINT, void*)> DrvDescribeLayerPlane;
        cct::FunctionRef<BOOL(HDC, int, int, int, const void*)> DrvSetLayerPaletteEntries;
        cct::FunctionRef<BOOL(HDC, int, int, int, int*)> DrvGetLayerPaletteEntries;
        cct::FunctionRef<BOOL(HDC)> DrvSwapBuffers;
        cct::FunctionRef<BOOL(HDC, UINT)> DrvSwapLayerBuffers;
        cct::FunctionRef<BOOL(ULONG)> DrvValidateVersion;
        cct::FunctionRef<void(int, PROC*)> DrvSetCallbackProcs;
        cct::FunctionRef<BOOL(HDC, int, BOOL)> DrvRealizeLayerPalette;
        cct::FunctionRef<BOOL(HDC, WGLPRESENTBUFFERSDATA*)> DrvPresentBuffers;
    } m_drvProcTable;

    cct::DynLib m_icd;
    const glgpus::GlProcTable* m_lastProcTable = nullptr;
    void* (*m_getDrvProcRaw)(const char*) = nullptr;
};

} // namespace gl
#endif // CCT_PLATFORM_WINDOWS
