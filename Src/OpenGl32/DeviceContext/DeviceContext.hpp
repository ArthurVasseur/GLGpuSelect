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
		virtual ~DeviceContext() = default;

		void SetGlDispatchTable(const OpenGlDispatchTable& dispatchTable);
		const OpenGlDispatchTable& GetGlDispatchTable() const;

		bool IsActiveOnCurrentThread() const;
		void SetActiveOnCurrentThread();
		bool IsActive() const;
		void SetInactive();

		void* GetPlatformDeviceContext() const;
		void SetCurrentValue(void* currentValue);
		void* GetCurrentValue() const;

		virtual void* GetIcdContext() const { return nullptr; }
		virtual bool DeleteContext() { return false; }
		virtual bool ShareLists(void* otherIcdContext) { return false; }
		virtual bool CopyContext(void* srcIcdContext, unsigned int mask) { return false; }

	private:
		OpenGlDispatchTable m_dispatchTable;
		std::thread::id m_activeOnThread;
		void* m_platformDeviceContext;
		void* m_currentValue;
	};

	struct IcdDeviceContextWrapper
	{
		DeviceContext* DeviceContext = nullptr;
		~IcdDeviceContextWrapper() { delete DeviceContext; }
	};
} // namespace glgpus
