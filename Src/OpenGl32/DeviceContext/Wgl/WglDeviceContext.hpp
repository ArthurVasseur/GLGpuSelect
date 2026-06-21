//
// Created by arthur on 23/04/2025.
//

#pragma once

#include "OpenGl32/DeviceContext/DeviceContext.hpp"

#ifdef CCT_PLATFORM_WINDOWS
#include "OpenGl32/IcdLoader/Wgl.hpp"

namespace glgpus
{
	class WglDeviceContext : public DeviceContext
	{
	public:
		WglDeviceContext(HDC hdc, void* icdContext);

		void* GetIcdContext() const override
		{
			return m_icdContext;
		}
		bool DeleteContext() override;
		bool ShareLists(void* otherIcdContext) override;
		bool CopyContext(void* srcIcdContext, unsigned int mask) override;

	private:
		void* m_icdContext;
	};
} // namespace glgpus
#endif // CCT_PLATFORM_WINDOWS
