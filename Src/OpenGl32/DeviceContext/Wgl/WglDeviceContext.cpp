//
// Created by arthur on 23/04/2025.
//

#include "OpenGl32/DeviceContext/Wgl/WglDeviceContext.hpp"

namespace glgpus
{
	WglDeviceContext::WglDeviceContext(HDC hdc) :
		DeviceContext(static_cast<void*>(hdc))
	{
	}

	bool WglDeviceContext::DeleteContext(HGLRC hglrc)
	{
		return false;
	}

	bool WglDeviceContext::ShareLists(HGLRC hglrc)
	{
		return false;
	}

	bool WglDeviceContext::CopyContext(HGLRC hglrc, UINT mask)
	{
		return false;
	}
} // namespace glgpus
