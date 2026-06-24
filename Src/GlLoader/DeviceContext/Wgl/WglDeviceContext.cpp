//
// Created by arthur on 23/04/2025.
//

#include "GlLoader/DeviceContext/Wgl/WglDeviceContext.hpp"

#ifdef CCT_PLATFORM_WINDOWS
#include "GlLoader/IcdLoader/IcdLoader.hpp"

namespace glgpus
{
	WglDeviceContext::WglDeviceContext(HDC hdc, void* icdContext) :
		DeviceContext(static_cast<void*>(hdc)),
		m_icdContext(icdContext)
	{
	}

	bool WglDeviceContext::DeleteContext()
	{
		return IcdLoader::Instance()->GetDriver().DeleteContext(static_cast<HGLRC>(m_icdContext));
	}

	bool WglDeviceContext::ShareLists(void* otherIcdContext)
	{
		return IcdLoader::Instance()->GetDriver().ShareLists(
			static_cast<HGLRC>(m_icdContext),
			static_cast<HGLRC>(otherIcdContext));
	}

	bool WglDeviceContext::CopyContext(void* srcIcdContext, unsigned int mask)
	{
		return IcdLoader::Instance()->GetDriver().CopyContext(
			static_cast<HGLRC>(srcIcdContext),
			static_cast<HGLRC>(m_icdContext),
			static_cast<UINT>(mask));
	}
} // namespace glgpus
#endif // CCT_PLATFORM_WINDOWS
