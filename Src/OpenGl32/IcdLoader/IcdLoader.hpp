//
// Created by arthur on 23/04/2025.
//

#pragma once

#include <mutex>
#include <vector>
#include <Concerto/Core/DynLib.hpp>

#include "OpenGl32/Defines.hpp"
#include "OpenGl32/GLGpuSelect.h"

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

		cct::DynLib& GetIcd();
		void SetSelectedPixelFormatIndex(cct::Int32 pixelFormatIndex);
		cct::Int32 GetSelectedPixelFormatIndex() const;

		void SetCurrentDeviceContextForCurrentThread(IcdDeviceContextWrapper& currentDeviceContext);
		IcdDeviceContextWrapper* GetCurrentDeviceContextForCurrentThread();
		void ResetCurrentDeviceContextForCurrentThread();

		const std::vector<AdapterInfo>& GetAdapterInfos() const;
	private:
		static std::unique_ptr<IcdLoader> s_instance;
		std::vector<AdapterInfo> m_adapterInfos;
		cct::DynLib m_icd;
		cct::Int32 m_selectedPixelFormatIndex;

		std::mutex m_deviceContextByThreadMutex;
		std::unordered_map<std::thread::id, IcdDeviceContextWrapper*> m_deviceContextByThread;
	};
}


GLGPUS_EXPORT cct::UInt32 CCT_CALL EnumerateDevices(cct::UInt32* pPhysicalDeviceCount, AdapterInfo* pDevices);
GLGPUS_EXPORT cct::UInt32 CCT_CALL ChooseDevice(cct::UInt64 pDeviceUuid);


