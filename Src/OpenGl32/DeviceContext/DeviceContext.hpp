//
// Created by arthur on 23/04/2025.
//

#pragma once

#include <thread>

#include "OpenGl32/OpenGlFunctions.hpp"

namespace glgpus
{
	class DeviceContext
	{
	public:
		DeviceContext(void* platformDeviceContext);

		void SetGlDispatchTable(const OpenGlDispatchTable& dispatchTable);
		const OpenGlDispatchTable& GetGlDispatchTable() const;

		bool IsActiveOnCurrentThread() const;
		void SetActiveOnCurrentThread();
		bool IsActive() const;
		void SetInactive();

		void* GetPlatformDeviceContext() const;
		void SetCurrentValue(void* currentValue);
		void* GetCurrentValue() const;

	private:
		OpenGlDispatchTable m_dispatchTable;
		std::thread::id m_activeOnThread;
		void* m_platformDeviceContext;
		void* m_currentValue;
	};

	struct IcdDeviceContextWrapper
	{
		DeviceContext* DeviceContext = nullptr;
		void* IcdDeviceContext = nullptr;
	};
} // namespace glgpus
