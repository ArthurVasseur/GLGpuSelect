//
// Created by arthur on 23/04/2025.
//

#include "OpenGl32/DeviceContext/Wgl/WglDeviceContext.hpp"

#ifdef CCT_PLATFORM_WINDOWS
#include "OpenGl32/IcdLoader/IcdLoader.hpp"
#include "OpenGl32/IcdLibrary/Wgl/WglIcdLibrary.hpp"

namespace glgpus
{
	WglDeviceContext::WglDeviceContext(HDC hdc, void* icdContext) :
		DeviceContext(static_cast<void*>(hdc)),
		m_icdContext(icdContext)
	{
	}

	bool WglDeviceContext::DeleteContext()
	{
		return IcdLoader::Instance()->GetPlatformIcd<WglIcdLibrary>().DrvDeleteContext(static_cast<HGLRC>(m_icdContext));
	}

	bool WglDeviceContext::ShareLists(void* otherIcdContext)
	{
		return IcdLoader::Instance()->GetPlatformIcd<WglIcdLibrary>().DrvShareLists(
			static_cast<HGLRC>(m_icdContext),
			static_cast<HGLRC>(otherIcdContext));
	}

	bool WglDeviceContext::CopyContext(void* srcIcdContext, unsigned int mask)
	{
		return IcdLoader::Instance()->GetPlatformIcd<WglIcdLibrary>().DrvCopyContext(
			static_cast<HGLRC>(srcIcdContext),
			static_cast<HGLRC>(m_icdContext),
			static_cast<UINT>(mask));
	}
} // namespace glgpus
#endif // CCT_PLATFORM_WINDOWS
