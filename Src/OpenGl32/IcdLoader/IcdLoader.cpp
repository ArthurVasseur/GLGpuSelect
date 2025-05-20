//
// Created by arthur on 23/04/2025.
//

#include <vector>
#include <array>
#include <Concerto/Core/Result.hpp>

#include "OpenGl32/IcdLoader/IcdLoader.hpp"

#include "OpenGl32/DeviceContext/DeviceContext.hpp"

#ifdef CCT_PLATFORM_WINDOWS
#include <Windows.h>
#include <d3dkmthk.h>
#undef min
#undef max
#define NT_SUCCESS(v) (v >= 0)
#include "OpenGl32/IcdLibrary/Wgl/WglIcdLibrary.hpp"
#endif
namespace glgpus
{
#ifdef CCT_PLATFORM_WINDOWS
	namespace wddm
	{
		static NTSTATUS QueryAdapterInfo(D3DKMT_HANDLE adapter_h, KMTQUERYADAPTERINFOTYPE info_type, void* info, size_t info_size)
		{
			D3DKMT_QUERYADAPTERINFO adapterInfo = {
				.hAdapter = adapter_h,
				.Type = info_type,
				.pPrivateDriverData = info,
				.PrivateDriverDataSize = static_cast<UINT>(info_size),
			};
			return D3DKMTQueryAdapterInfo(&adapterInfo);
		}

		static cct::Result<std::vector<AdapterInfo>, glgpusResult> EnumerateDevices()
		{
			std::vector<AdapterInfo> adapters;
			std::array<D3DKMT_ADAPTERINFO, MAX_ENUM_ADAPTERS> adapterInfos;

			D3DKMT_ENUMADAPTERS2 enumAdapters = {
			  .NumAdapters = adapterInfos.size(),
			  .pAdapters = adapterInfos.data(),
			};

			NTSTATUS status = D3DKMTEnumAdapters2(&enumAdapters);
			if (!NT_SUCCESS(status))
				return glgpusResult::Unknown;

			if (enumAdapters.NumAdapters == 0)
				return glgpusResult::Success;

			cct::DeferredExit _([&]()
				{
					for (uint32_t i = 0; i < enumAdapters.NumAdapters; i++)
					{
						D3DKMT_CLOSEADAPTER close_adapter = {
						   .hAdapter = adapterInfos[i].hAdapter,
						};
						status = D3DKMTCloseAdapter(&close_adapter);
						CCT_ASSERT(NT_SUCCESS(status), "D3DKMTCloseAdapter failed");
					}
				});

			for (uint32_t i = 0; i < enumAdapters.NumAdapters; i++)
			{
				D3DKMT_PHYSICAL_ADAPTER_COUNT adapterCount = {};
				status = QueryAdapterInfo(adapterInfos[i].hAdapter, KMTQAITYPE_PHYSICALADAPTERCOUNT, &adapterCount, sizeof(adapterCount));
				if (!NT_SUCCESS(status))
					return glgpusResult::Unknown;

				for (uint32_t j = 0; j < adapterCount.Count; j++)
				{
					D3DKMT_QUERY_DEVICE_IDS queryIds = {};
					queryIds.PhysicalAdapterIndex = j;

					status = QueryAdapterInfo(adapterInfos[i].hAdapter, KMTQAITYPE_PHYSICALADAPTERDEVICEIDS, &queryIds, sizeof(queryIds));
					if (!NT_SUCCESS(status))
					{
						return glgpusResult::Unknown;
					}

					if (queryIds.DeviceIds.VendorID == static_cast<std::underlying_type_t<VendorId>>(VendorId::Microsoft))
						continue;

					CCT_ASSERT(queryIds.DeviceIds.DeviceID <= std::numeric_limits<cct::UInt16>::max(), "Invalid size");
					CCT_ASSERT(queryIds.DeviceIds.VendorID <= std::numeric_limits<cct::UInt16>::max(), "Invalid size");
					CCT_ASSERT(queryIds.DeviceIds.SubVendorID <= std::numeric_limits<cct::UInt16>::max(), "Invalid size");
					CCT_ASSERT(queryIds.DeviceIds.SubSystemID <= std::numeric_limits<cct::UInt16>::max(), "Invalid size");
					CCT_ASSERT(queryIds.DeviceIds.RevisionID <= std::numeric_limits<cct::UInt8>::max(), "Invalid size");

					D3DKMT_ADAPTERADDRESS address = {};
					status = QueryAdapterInfo(adapterInfos[i].hAdapter, KMTQAITYPE_ADAPTERADDRESS, &address, sizeof(address));
					if (!NT_SUCCESS(status))
					{
						return glgpusResult::Unknown;
					}

					CCT_ASSERT(address.BusNumber <= std::numeric_limits<cct::UInt8>::max(), "Invalid size");
					CCT_ASSERT(address.DeviceNumber <= std::numeric_limits<cct::UInt8>::max(), "Invalid size");
					CCT_ASSERT(address.FunctionNumber <= std::numeric_limits<cct::UInt8>::max(), "Invalid size");


					D3DKMT_OPENGLINFO oglInfo = {};
					status = QueryAdapterInfo(adapterInfos[i].hAdapter, KMTQAITYPE_UMOPENGLINFO, &oglInfo, sizeof(oglInfo));
					if (!NT_SUCCESS(status))
					{
						return glgpusResult::Unknown;
					}
					AdapterInfo adapterInfo = {};
					std::memcpy(&adapterInfo.Uuid, &adapterInfos[i].AdapterLuid, sizeof(cct::UInt64));
					adapterInfo.VendorId = static_cast<cct::UInt16>(queryIds.DeviceIds.VendorID);
					adapterInfo.Index = queryIds.PhysicalAdapterIndex;
					adapterInfo.DeviceId = static_cast<cct::UInt16>(queryIds.DeviceIds.DeviceID);
					adapterInfo.openGlVersion = oglInfo.Version;

					D3DKMT_OPENADAPTERFROMLUID openAdapter = {
						.AdapterLuid = adapterInfos[i].AdapterLuid,
						.hAdapter = 0
					};

					status = D3DKMTOpenAdapterFromLuid(&openAdapter);
					if (!NT_SUCCESS(status))
						return glgpusResult::Unknown;

					D3DKMT_CLOSEADAPTER closeAdapter = {
						.hAdapter = openAdapter.hAdapter,
					};

					cct::DeferredExit __([&]()
					{
						status = D3DKMTCloseAdapter(&closeAdapter);
						CCT_ASSERT(NT_SUCCESS(status), "D3DKMTCloseAdapter failed");
					});
					D3DKMT_ADAPTERREGISTRYINFO registry = {};
					status = QueryAdapterInfo(openAdapter.hAdapter, KMTQAITYPE_ADAPTERREGISTRYINFO_RENDER, &registry, sizeof(registry));
					if (!NT_SUCCESS(status))
					{
						//if KMTQAITYPE_ADAPTERREGISTRYINFO_RENDER is not available, try get the driver description renderer
						D3DKMT_DRIVER_DESCRIPTION registry = {};
						status = QueryAdapterInfo(openAdapter.hAdapter, KMTQAITYPE_DRIVER_DESCRIPTION_RENDER, &registry, sizeof(registry));
						if (!NT_SUCCESS(status))
							return glgpusResult::Unknown;
					}
					auto name = ToUtf8(registry.AdapterString);
					std::memcpy(&adapterInfo.Name, name.data(), name.size());
					adapterInfo.Index = i;
					adapters.emplace_back(adapterInfo);
				}
			}
			return adapters;
		}

		static cct::Result<std::string, glgpusResult> ChooseDevice(cct::UInt64 pDeviceUuid)
		{
			LUID luid;
			std::memcpy(&luid, &pDeviceUuid, sizeof(cct::UInt64));
			D3DKMT_OPENADAPTERFROMLUID openAdapter = {
				.AdapterLuid = luid,
				.hAdapter = 0
			};

			auto status = D3DKMTOpenAdapterFromLuid(&openAdapter);
			if (!NT_SUCCESS(status))
				return glgpusResult::Unknown;

			D3DKMT_OPENGLINFO oglInfo = {};
			status = QueryAdapterInfo(openAdapter.hAdapter, KMTQAITYPE_UMOPENGLINFO, &oglInfo, sizeof(oglInfo));
			if (!NT_SUCCESS(status))
				return glgpusResult::Unknown;

			return ToUtf8(oglInfo.UmdOpenGlIcdFileName);
		}
	}
#endif
	std::unique_ptr<IcdLoader> IcdLoader::s_instance = nullptr;

	IcdLoader::IcdLoader() :
#ifdef CCT_PLATFORM_WINDOWS
		m_icdLibrary(std::make_unique<WglIcdLibrary>()),
#endif
		m_selectedPixelFormatIndex(0),
		m_currentValue(nullptr)
	{
	}

	IcdLoader* IcdLoader::Instance()
	{
		if (s_instance == nullptr)
		{
			auto* instance = new(std::nothrow) IcdLoader();
			if (instance == nullptr)
				return nullptr;
			s_instance.reset(instance);
		}
		return s_instance.get();
	}

	cct::UInt32 IcdLoader::EnumerateAdapters(cct::UInt32* pPhysicalDeviceCount, AdapterInfo* pDevices)
	{
		auto deviceInfoResult =
#ifdef CCT_PLATFORM_WINDOWS
		wddm::EnumerateDevices();
#else
		std::vector<AdapterInfo>();
#endif

		if (deviceInfoResult.IsError())
			return MakeResult(deviceInfoResult.GetError(), "Failed to enumerate devices");

		m_adapterInfos = std::move(deviceInfoResult).GetValue();

		if (pPhysicalDeviceCount && !pDevices)
			*pPhysicalDeviceCount = static_cast<cct::UInt32>(m_adapterInfos.size());
		else if (pDevices)
			std::memcpy(pDevices, m_adapterInfos.data(), m_adapterInfos.size() * sizeof(AdapterInfo));

		return MakeResult(glgpusResult::Success);
	}

	cct::UInt32 IcdLoader::ChooseDevice(cct::UInt64 pDeviceUuid)
	{
#ifdef CCT_PLATFORM_WINDOWS
		auto icdPathResult = wddm::ChooseDevice(pDeviceUuid);
#else
			std::vector<AdapterInfo>();
#endif
		if (icdPathResult.IsError())
			return MakeResult(icdPathResult.GetError(), "Failed to choose device");

		m_icdLibrary->Load(icdPathResult.GetValue());
		if (!m_icdLibrary->IsLoaded())
			return MakeResult(glgpusResult::Unknown, "Could not open icd file for the given device");

		AdapterInfo* adapterInfo = nullptr;
		for (auto& info : m_adapterInfos)
		{
			if (info.Uuid == pDeviceUuid)
			{
				adapterInfo = &info;
				break;
			}
		}

		if (adapterInfo == nullptr)
			return MakeResult(glgpusResult::InvalidArg, "Invalid UUID");

		cct::Logger::Info("Using {}\nICD path: '{}'", adapterInfo->Name, icdPathResult.GetValue());

		return 0;
	}

	IcdLibrary& IcdLoader::GetIcd() const
	{
		CCT_ASSERT(m_icdLibrary != nullptr, "ICD library is not loaded");
		return *m_icdLibrary;
	}

	void IcdLoader::SetSelectedPixelFormatIndex(cct::Int32 pixelFormatIndex)
	{
		m_selectedPixelFormatIndex = pixelFormatIndex;
	}

	cct::Int32 IcdLoader::GetSelectedPixelFormatIndex() const
	{
		return m_selectedPixelFormatIndex;
	}

	void IcdLoader::SetCurrentDeviceContextForCurrentThread(IcdDeviceContextWrapper& currentDeviceContext)
	{
		currentDeviceContext.DeviceContext->SetActiveOnCurrentThread();

		std::scoped_lock _(m_deviceContextByThreadMutex);
		m_deviceContextByThread[std::this_thread::get_id()] = &currentDeviceContext;
	}

	IcdDeviceContextWrapper* IcdLoader::GetCurrentDeviceContextForCurrentThread() const
	{
		std::scoped_lock _(m_deviceContextByThreadMutex);

		auto it = m_deviceContextByThread.find(std::this_thread::get_id());
		if (it == m_deviceContextByThread.end())
			return nullptr;

		return it->second;
	}

	void IcdLoader::ResetCurrentDeviceContextForCurrentThread()
	{
		std::scoped_lock _(m_deviceContextByThreadMutex);

		auto it = m_deviceContextByThread.find(std::this_thread::get_id());
		if (it == m_deviceContextByThread.end())
			return;
		it->second->DeviceContext->SetInactive();
		m_deviceContextByThread.erase(it);
	}

	const std::vector<AdapterInfo>& IcdLoader::GetAdapterInfos() const
	{
		return m_adapterInfos;
	}

	void IcdLoader::SetCurrentValue(void* currentValue)
	{
		auto* currentDeviceContext = GetCurrentDeviceContextForCurrentThread();
		if (currentDeviceContext && currentDeviceContext->DeviceContext)
			currentDeviceContext->DeviceContext->SetCurrentValue(currentValue);
		else m_currentValue = currentValue;
	}

	void* IcdLoader::GetCurrentValue() const
	{
		auto* currentDeviceContext = GetCurrentDeviceContextForCurrentThread();
		if (currentDeviceContext && currentDeviceContext->DeviceContext)
			return currentDeviceContext->DeviceContext->GetCurrentValue();
		return m_currentValue;
	}
}

cct::UInt32 glgpusEnumerateDevices(cct::UInt32* pPhysicalDeviceCount, AdapterInfo* pDevices)
{
	auto* instance = glgpus::IcdLoader::Instance();
	if (!instance)
	{
		CCT_ASSERT_FALSE("glgpus::IcdLoader::Instance() returned nullptr");
		return glgpus::MakeResult(glgpus::glgpusResult::OutOfHostMemory);
	}

	return instance->EnumerateAdapters(pPhysicalDeviceCount, pDevices);
}

cct::UInt32 glgpusChooseDevice(cct::UInt64 pDeviceUuid)
{
	auto* instance = glgpus::IcdLoader::Instance();
	if (!instance)
	{
		CCT_ASSERT_FALSE("glgpus::IcdLoader::Instance() returned nullptr");
		return glgpus::MakeResult(glgpus::glgpusResult::OutOfHostMemory);
	}

	return instance->ChooseDevice(pDeviceUuid);
}
