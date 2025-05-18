//
// Created by arthur on 23/04/2025.
//

#pragma once

struct AdapterInfo
{
	uint64_t Uuid;
	uint32_t Index;
	uint16_t VendorId;
	uint16_t DeviceId;
	uint64_t openGlVersion;
	char Name[255];
};