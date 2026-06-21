//
// Created by arthur on 23/04/2025.
//

#pragma once

#include <mutex>
#include <unordered_map>
#include <vector>

#include <Concerto/Core/DynLib/DynLib.hpp>

#include "OpenGl32/Defines.hpp"
#include "OpenGl32/GLGpuSelect.h"
#include "OpenGl32/IcdLibrary/IcdLibrary.hpp"

#ifdef CCT_PLATFORM_WINDOWS
#include <Windows.h>
#undef min
#undef max
#endif

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

		void EnsureInitialized();
		bool IsInitialized() const
		{
			return m_initialized;
		}

#ifdef CCT_PLATFORM_WINDOWS
		void SetSelectedPixelFormatIndex(HDC hdc, int index);
		int GetSelectedPixelFormatIndex(HDC hdc) const;
#endif

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
		bool m_initialized;

		mutable std::mutex m_deviceContextByThreadMutex;
		std::unordered_map<std::thread::id, IcdDeviceContextWrapper*> m_deviceContextByThread;
		std::unordered_map<void*, int> m_pixelFormatByHdc;

		void* m_currentValue;
	};
} // namespace glgpus
