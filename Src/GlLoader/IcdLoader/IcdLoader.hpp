//
// Created by arthur on 23/04/2025.
//

#pragma once

#include <mutex>
#include <string_view>
#include <unordered_map>
#include <vector>

#include <Concerto/Core/DynLib/DynLib.hpp>

#include "GlDriver/GlDriver.hpp"
#include "GlLayer/GlLayer.hpp"
#include "GlLoader/Defines.hpp"
#include "GlLoader/GLGpuSelect.h"

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
		~IcdLoader();
		static IcdLoader* Instance();
		cct::UInt32 EnumerateAdapters(cct::UInt32* pPhysicalDeviceCount, AdapterInfo* pDevices);
		cct::UInt32 ChooseDevice(cct::UInt64 pDeviceUuid);

		gl::GlDriver& GetDriver() const;
		const std::vector<gl::GlLayer*>& GetLayers() const;

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
		void BuildChain(std::string_view icdPath);

		static std::unique_ptr<IcdLoader> s_instance;
		gl::GlDriver* m_driver = nullptr;
		void (*m_driverDestroy)(gl::GlDriver*) = nullptr;
		std::vector<gl::GlLayer*> m_layers;
		std::vector<void (*)(gl::GlLayer*)> m_layerDestroyFns;
		cct::DynLib m_driverLib;
		std::vector<cct::DynLib> m_layerLibs;
		std::vector<AdapterInfo> m_adapterInfos;
		bool m_initialized;

		mutable std::mutex m_deviceContextByThreadMutex;
		std::unordered_map<std::thread::id, IcdDeviceContextWrapper*> m_deviceContextByThread;
		std::unordered_map<void*, int> m_pixelFormatByHdc;

		void* m_currentValue;
	};
} // namespace glgpus
