//
// Created by arthur on 23/04/2025.
//

#ifndef OPENGL32_GLGPUSELECT_H
#define OPENGL32_GLGPUSELECT_H

#ifdef __cplusplus
#include <Concerto/Core/Types.hpp>
#else
#include <Concerto/Core/Types.h>
#endif

#if defined(CCT_PLATFORM_WINDOWS) && defined(CCT_ARCH_X86) // FIXME
#undef CCT_CALL
#define CCT_CALL __cdecl
#endif

#ifdef GLGPUS_BUILD
#define GLGPUS_API CCT_EXPORT
#else
#define GLGPUS_API CCT_IMPORT
#endif

struct
{
	uint64_t Uuid;
	uint32_t Index;
	uint16_t VendorId;
	uint16_t DeviceId;
	uint64_t openGlVersion;
	char Name[255];
} typedef AdapterInfo;

#ifdef CCT_LANGUAGE_CPP
extern "C"
{
#endif

GLGPUS_API uint32_t CCT_CALL glgpusEnumerateDevices(uint32_t* pPhysicalDeviceCount, AdapterInfo* pDevices);
GLGPUS_API uint32_t CCT_CALL glgpusChooseDevice(uint64_t pDeviceUuid);

#ifdef CCT_LANGUAGE_CPP
}
#endif

#endif // OPENGL32_GLGPUSELECT_H