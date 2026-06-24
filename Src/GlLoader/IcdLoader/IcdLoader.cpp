//
// Created by arthur on 23/04/2025.
//

#include "GlLoader/IcdLoader/IcdLoader.hpp"

#include <array>
#include <vector>

#include <Concerto/Core/Result/Result.hpp>

#include "GlLoader/DeviceContext/DeviceContext.hpp"

#ifdef CCT_PLATFORM_WINDOWS
#include <d3dkmthk.h>
#include <Windows.h>
#undef min
#undef max
#define NT_SUCCESS(v) (v >= 0)
#endif
namespace glgpus
{
#ifdef CCT_PLATFORM_WINDOWS
	namespace wddm
	{
		static NTSTATUS QueryAdapterInfo(D3DKMT_HANDLE adapter_h, KMTQUERYADAPTERINFOTYPE info_type, void* info, size_t info_size)
		{
			GLGPUS_AUTO_PROFILER_SCOPE();

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
			GLGPUS_AUTO_PROFILER_SCOPE();

			std::vector<AdapterInfo> adapters;
			std::array<D3DKMT_ADAPTERINFO, MAX_ENUM_ADAPTERS> adapterInfos;

			D3DKMT_ENUMADAPTERS2 enumAdapters = {
				.NumAdapters = adapterInfos.size(),
				.pAdapters = adapterInfos.data(),
			};

			NTSTATUS status = {};
			{
				GLGPUS_PROFILER_SCOPE("D3DKMTEnumAdapters2");
				status = D3DKMTEnumAdapters2(&enumAdapters);
				if (!NT_SUCCESS(status))
					return glgpusResult::Unknown;
			}

			if (enumAdapters.NumAdapters == 0)
				return glgpusResult::Success;

			cct::DeferredExit _([&]()
								{
					for (uint32_t i = 0; i < enumAdapters.NumAdapters; i++)
					{
						D3DKMT_CLOSEADAPTER close_adapter = {
						   .hAdapter = adapterInfos[i].hAdapter,
						};
						GLGPUS_PROFILER_SCOPE("D3DKMTCloseAdapter");
						status = D3DKMTCloseAdapter(&close_adapter);
						GLGPUS_ASSERT(NT_SUCCESS(status), "D3DKMTCloseAdapter failed");
					} });

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

					GLGPUS_ASSERT(queryIds.DeviceIds.DeviceID <= std::numeric_limits<cct::UInt16>::max(), "Invalid size");
					GLGPUS_ASSERT(queryIds.DeviceIds.VendorID <= std::numeric_limits<cct::UInt16>::max(), "Invalid size");
					GLGPUS_ASSERT(queryIds.DeviceIds.SubVendorID <= std::numeric_limits<cct::UInt16>::max(), "Invalid size");
					GLGPUS_ASSERT(queryIds.DeviceIds.SubSystemID <= std::numeric_limits<cct::UInt16>::max(), "Invalid size");
					GLGPUS_ASSERT(queryIds.DeviceIds.RevisionID <= std::numeric_limits<cct::UInt8>::max(), "Invalid size");

					D3DKMT_ADAPTERADDRESS address = {};
					status = QueryAdapterInfo(adapterInfos[i].hAdapter, KMTQAITYPE_ADAPTERADDRESS, &address, sizeof(address));
					if (!NT_SUCCESS(status))
					{
						return glgpusResult::Unknown;
					}

					GLGPUS_ASSERT(address.BusNumber <= std::numeric_limits<cct::UInt8>::max(), "Invalid size");
					GLGPUS_ASSERT(address.DeviceNumber <= std::numeric_limits<cct::UInt8>::max(), "Invalid size");
					GLGPUS_ASSERT(address.FunctionNumber <= std::numeric_limits<cct::UInt8>::max(), "Invalid size");

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
						.hAdapter = 0};

					{
						GLGPUS_PROFILER_SCOPE("D3DKMTOpenAdapterFromLuid");
						status = D3DKMTOpenAdapterFromLuid(&openAdapter);
						if (!NT_SUCCESS(status))
							return glgpusResult::Unknown;
					}

					D3DKMT_CLOSEADAPTER closeAdapter = {
						.hAdapter = openAdapter.hAdapter,
					};

					cct::DeferredExit __([&]()
										 {
							GLGPUS_PROFILER_SCOPE("D3DKMTCloseAdapter");
							status = D3DKMTCloseAdapter(&closeAdapter);
							GLGPUS_ASSERT(NT_SUCCESS(status), "D3DKMTCloseAdapter failed"); });
					D3DKMT_ADAPTERREGISTRYINFO registry = {};
					status = QueryAdapterInfo(openAdapter.hAdapter, KMTQAITYPE_ADAPTERREGISTRYINFO_RENDER, &registry, sizeof(registry));
					if (!NT_SUCCESS(status))
					{
						// if KMTQAITYPE_ADAPTERREGISTRYINFO_RENDER is not available, try get the driver description renderer
						D3DKMT_DRIVER_DESCRIPTION registry = {};
						status = QueryAdapterInfo(openAdapter.hAdapter, KMTQAITYPE_DRIVER_DESCRIPTION_RENDER, &registry, sizeof(registry));
						if (!NT_SUCCESS(status))
							return glgpusResult::Unknown;
					}
					std::string name;
					{
						GLGPUS_PROFILER_SCOPE("ToUtf8");
						name = ToUtf8(registry.AdapterString);
					}
					const std::size_t copyLen = std::min(name.size(), sizeof(adapterInfo.Name) - 1);
					std::memcpy(&adapterInfo.Name, name.data(), copyLen);
					adapterInfo.Name[copyLen] = '\0';
					adapterInfo.Index = i;
					adapters.emplace_back(adapterInfo);
				}
			}
			return adapters;
		}

		static cct::Result<std::string, glgpusResult> ChooseDevice(cct::UInt64 pDeviceUuid)
		{
			GLGPUS_AUTO_PROFILER_SCOPE();
			LUID luid;
			std::memcpy(&luid, &pDeviceUuid, sizeof(cct::UInt64));
			D3DKMT_OPENADAPTERFROMLUID openAdapter = {
				.AdapterLuid = luid,
				.hAdapter = 0};

			auto status = D3DKMTOpenAdapterFromLuid(&openAdapter);
			if (!NT_SUCCESS(status))
				return glgpusResult::Unknown;

			D3DKMT_OPENGLINFO oglInfo = {};
			status = QueryAdapterInfo(openAdapter.hAdapter, KMTQAITYPE_UMOPENGLINFO, &oglInfo, sizeof(oglInfo));
			if (!NT_SUCCESS(status))
				return glgpusResult::Unknown;

			return ToUtf8(oglInfo.UmdOpenGlIcdFileName);
		}
	} // namespace wddm
#endif
	std::unique_ptr<IcdLoader> IcdLoader::s_instance = nullptr;

	IcdLoader::IcdLoader() :
		m_initialized(false),
		m_currentValue(nullptr)
	{
	}

	IcdLoader::~IcdLoader()
	{
		for (std::size_t i = 0; i < m_layers.size(); ++i)
		{
			if (m_layerDestroyFns[i] != nullptr)
				m_layerDestroyFns[i](m_layers[i]);
		}
		m_layers.clear();
		m_layerDestroyFns.clear();

		if (m_driver != nullptr && m_driverDestroy != nullptr)
		{
			m_driverDestroy(m_driver);
			m_driver = nullptr;
		}
	}

	IcdLoader* IcdLoader::Instance()
	{
		static std::once_flag s_initFlag;
		std::call_once(s_initFlag, []()
					   {
			auto* instance = new(std::nothrow) IcdLoader();
			if (instance)
				s_instance.reset(instance); });
		return s_instance.get();
	}

	cct::UInt32 IcdLoader::EnumerateAdapters(cct::UInt32* pPhysicalDeviceCount, AdapterInfo* pDevices)
	{
		GLGPUS_AUTO_PROFILER_SCOPE();

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
		GLGPUS_AUTO_PROFILER_SCOPE();

#ifdef CCT_PLATFORM_WINDOWS
		auto icdPathResult = wddm::ChooseDevice(pDeviceUuid);
#else
		std::vector<AdapterInfo>();
#endif
		if (icdPathResult.IsError())
			return MakeResult(icdPathResult.GetError(), "Failed to choose device");

		BuildChain(icdPathResult.GetValue());
		if (!m_driver)
			return MakeResult(glgpusResult::Unknown, "Could not load driver for the given device");

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

		GLGPUS_LOG_INFO("Using {}\nICD path: '{}'", adapterInfo->Name, icdPathResult.GetValue());

		return 0;
	}

	gl::GlDriver& IcdLoader::GetDriver() const
	{
		GLGPUS_ASSERT(m_driver != nullptr, "Driver is not loaded");
		return *m_driver;
	}

	const std::vector<gl::GlLayer*>& IcdLoader::GetLayers() const
	{
		return m_layers;
	}

	void IcdLoader::BuildChain(std::string_view icdPath)
	{
		if (!m_driverLib.Load("gl-icd-driver.dll"))
		{
			GLGPUS_ASSERT_FALSE("Could not load gl-icd-driver.dll");
			return;
		}

		auto driverCreate = m_driverLib.GetFunction<gl::GlDriver*, const char*>("glDriverCreate");
		if (!driverCreate)
		{
			GLGPUS_ASSERT_FALSE("Could not find 'glDriverCreate' in gl-icd-driver.dll");
			return;
		}

		m_driverDestroy = reinterpret_cast<void (*)(gl::GlDriver*)>(m_driverLib.GetSymbol("glDriverDestroy"));
		if (m_driverDestroy == nullptr)
			GLGPUS_ASSERT_FALSE("Could not find 'glDriverDestroy' in gl-icd-driver.dll");

		m_driver = driverCreate(std::string(icdPath).c_str());
		if (m_driver == nullptr)
		{
			GLGPUS_ASSERT_FALSE("glDriverCreate failed");
			return;
		}

		const char* layerEnvVar = std::getenv("GL_LAYERS");
		if (layerEnvVar == nullptr)
			return;

		std::string_view layerList(layerEnvVar);
		std::size_t start = 0;
		while (start < layerList.size())
		{
			const std::size_t end = layerList.find(';', start);
			const std::size_t len = (end == std::string_view::npos ? layerList.size() : end) - start;
			const std::string_view dllPath = layerList.substr(start, len);
			start += len + 1;

			if (dllPath.empty())
			{
				continue;
			}

			auto& lib = m_layerLibs.emplace_back();
			if (!lib.Load(std::string(dllPath)))
			{
				GLGPUS_ASSERT_FALSE("Could not load layer DLL");
				continue;
			}

			auto layerCreate = lib.GetFunction<gl::GlLayer*>("glLayerCreate");
			if (!layerCreate)
			{
				GLGPUS_ASSERT_FALSE("Could not find 'glLayerCreate' in layer DLL");
				continue;
			}

			auto* layerDestroy = reinterpret_cast<void (*)(gl::GlLayer*)>(lib.GetSymbol("glLayerDestroy"));
			if (layerDestroy == nullptr)
				GLGPUS_ASSERT_FALSE("Could not find 'glLayerDestroy' in layer DLL");

			if (auto* layer = layerCreate())
			{
				m_layers.push_back(layer);
				m_layerDestroyFns.push_back(layerDestroy);
			}
		}
	}

	void IcdLoader::EnsureInitialized()
	{
#ifdef CCT_PLATFORM_WINDOWS
		static std::once_flag s_icdInitFlag;
		std::call_once(s_icdInitFlag, [this]()
					   {
			EnumerateAdapters(nullptr, nullptr);
			const auto& adapters = GetAdapterInfos();
			if (adapters.empty())
			{
				GLGPUS_ASSERT_FALSE("No adapters found");
				return;
			}

			std::size_t adapterIndex = 0;
			if (const auto* value = std::getenv("GLGPUS_ADAPTER_OS_INDEX"))
				adapterIndex = std::stoi(value);

			if (adapterIndex >= adapters.size())
			{
				GLGPUS_ASSERT_FALSE("GLGPUS_ADAPTER_OS_INDEX is out of range");
				return;
			}

			const auto& selectedAdapter = adapters[adapterIndex];
			if (ChooseDevice(selectedAdapter.Uuid) != 0)
			{
				GLGPUS_ASSERT_FALSE("Failed to choose device");
				return;
			}

			if (m_driver->ValidateVersion(selectedAdapter.openGlVersion) == 0)
			{
				GLGPUS_ASSERT_FALSE("Invalid ICD version");
				return;
			}

			m_initialized = true; });
#endif
	}

#ifdef CCT_PLATFORM_WINDOWS
	void IcdLoader::SetSelectedPixelFormatIndex(HDC hdc, int index)
	{
		std::scoped_lock _(m_deviceContextByThreadMutex);
		m_pixelFormatByHdc[static_cast<void*>(hdc)] = index;
	}

	int IcdLoader::GetSelectedPixelFormatIndex(HDC hdc) const
	{
		std::scoped_lock _(m_deviceContextByThreadMutex);
		auto it = m_pixelFormatByHdc.find(static_cast<void*>(hdc));
		return it != m_pixelFormatByHdc.end() ? it->second : 0;
	}
#endif

	void IcdLoader::SetCurrentDeviceContextForCurrentThread(IcdDeviceContextWrapper& currentDeviceContext)
	{
		std::scoped_lock _(m_deviceContextByThreadMutex);
		currentDeviceContext.DeviceContext->SetActiveOnCurrentThread();
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
		else
			m_currentValue = currentValue;
	}

	void* IcdLoader::GetCurrentValue() const
	{
		auto* currentDeviceContext = GetCurrentDeviceContextForCurrentThread();
		if (currentDeviceContext && currentDeviceContext->DeviceContext)
			return currentDeviceContext->DeviceContext->GetCurrentValue();
		return m_currentValue;
	}
} // namespace glgpus

cct::UInt32 glgpusEnumerateDevices(cct::UInt32* pPhysicalDeviceCount, AdapterInfo* pDevices)
{
	auto* instance = glgpus::IcdLoader::Instance();
	if (!instance)
	{
		GLGPUS_ASSERT_FALSE("glgpus::IcdLoader::Instance() returned nullptr");
		return glgpus::MakeResult(glgpus::glgpusResult::OutOfHostMemory);
	}

	return instance->EnumerateAdapters(pPhysicalDeviceCount, pDevices);
}

cct::UInt32 glgpusChooseDevice(cct::UInt64 pDeviceUuid)
{
	auto* instance = glgpus::IcdLoader::Instance();
	if (!instance)
	{
		GLGPUS_ASSERT_FALSE("glgpus::IcdLoader::Instance() returned nullptr");
		return glgpus::MakeResult(glgpus::glgpusResult::OutOfHostMemory);
	}

	return instance->ChooseDevice(pDeviceUuid);
}
