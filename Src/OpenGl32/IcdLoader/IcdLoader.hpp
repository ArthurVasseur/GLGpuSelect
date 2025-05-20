//
// Created by arthur on 23/04/2025.
//

#pragma once

#include <mutex>
#include <vector>
#include <Concerto/Core/DynLib.hpp>

#include "OpenGl32/Defines.hpp"
#include "OpenGl32/GLGpuSelect.h"
#include "OpenGl32/IcdLibrary/IcdLibrary.hpp"

namespace glgpus
{
	struct IcdDeviceContextWrapper;

	class IcdLoader
	{
	public:
		IcdLoader();
		static IcdLoader* Instance();
		cct::UInt32 EnumerateAdapters(cct::UInt32* pPhysicalDeviceCount, AdapterInfo* pDevices);
		cct::UInt32 ChooseDevice(cct::UInt64 pDeviceUuid);

		IcdLibrary& GetIcd() const;
		template<typename T>
		T& GetPlatformIcd()
		{
		#ifdef CCT_DEBUG
			CCT_ASSERT(m_icdLibrary && dynamic_cast<T*>(m_icdLibrary.get()), "T must inherits from IcdLibrary");
		#endif
			return static_cast<T&>(*m_icdLibrary);
		}

		void SetSelectedPixelFormatIndex(cct::Int32 pixelFormatIndex);
		cct::Int32 GetSelectedPixelFormatIndex() const;

		void SetCurrentDeviceContextForCurrentThread(IcdDeviceContextWrapper& currentDeviceContext);
		IcdDeviceContextWrapper* GetCurrentDeviceContextForCurrentThread() const;
		void ResetCurrentDeviceContextForCurrentThread();

		const std::vector<AdapterInfo>& GetAdapterInfos() const;

		void SetCurrentValue(void* currentValue);
		void* GetCurrentValue() const;
	private:
		static std::unique_ptr<IcdLoader> s_instance;
		std::unique_ptr<IcdLibrary> m_icdLibrary;
		std::vector<AdapterInfo> m_adapterInfos;
		cct::Int32 m_selectedPixelFormatIndex;

		mutable std::mutex m_deviceContextByThreadMutex;
		std::unordered_map<std::thread::id, IcdDeviceContextWrapper*> m_deviceContextByThread;

		void* m_currentValue;
	};
}
