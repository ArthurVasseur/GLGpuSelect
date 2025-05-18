//
// Created by arthur on 23/04/2025.
//

#pragma once

#include "OpenGl32/DeviceContext/DeviceContext.hpp"
#include "OpenGl32/IcdLibrary/Wgl/WglIcdLibrary.hpp"

namespace glgpus
{
	class WglDeviceContext : public DeviceContext
	{
	public:
		WglDeviceContext(HDC hdc);

		bool DeleteContext(HGLRC hglrc);
		bool ShareLists(HGLRC hglrc);
		bool CopyContext(HGLRC hglrc, UINT mask);
		bool SwapBuffersRuntimeCheck(HDC hdc);

	private:

	};
} // namespace glgpus
