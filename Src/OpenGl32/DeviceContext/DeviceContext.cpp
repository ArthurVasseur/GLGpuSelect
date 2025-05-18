//
// Created by arthur on 23/04/2025.
//

#include "OpenGl32/DeviceContext/DeviceContext.hpp"

namespace glgpus
{
	DeviceContext::DeviceContext(void* platformDeviceContext) :
		m_dispatchTable(),
		m_platformDeviceContext(platformDeviceContext)
	{
		CCT_ASSERT(platformDeviceContext != nullptr, "Invalid device context");
	}

	void DeviceContext::SetGlDispatchTable(const GlDispatchTable& dispatchTable)
	{
		m_dispatchTable = dispatchTable;
	}

	const GlDispatchTable& DeviceContext::GetGlDispatchTable() const
	{
		return m_dispatchTable;
	}

	bool DeviceContext::IsActiveOnCurrentThread() const
	{
		return m_activeOnThread == std::this_thread::get_id();
	}

	void DeviceContext::SetActiveOnCurrentThread()
	{
		m_activeOnThread = std::this_thread::get_id();
	}

	bool DeviceContext::IsActive() const
	{
		return m_activeOnThread != std::thread::id {};
	}

	void DeviceContext::SetInactive()
	{
		m_activeOnThread = {};
	}

	void* DeviceContext::GetPlatformDeviceContext() const
	{
		return m_platformDeviceContext;
	}
} // namespace glgpus
