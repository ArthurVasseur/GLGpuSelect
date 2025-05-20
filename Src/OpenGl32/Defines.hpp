//
// Created by arthur on 23/04/2025.
//

#pragma once

#include <Concerto/Core/Types.hpp>
#include <Concerto/Core/Assert.hpp>
#include <Concerto/Core/DeferredExit.hpp>
#include "OpenGl32/GLGpuSelect.h"


#define GLGPUS_FROM_HANDLE(type, name, handle)	\
	type* name = type::FromHandle(handle)

#define GLGPUS_TO_HANDLE(type, handle)	\
	(type)(handle)

#define GLGPUS_DISPATCHABLE_HANDLE(type)															\
		static inline type* FromHandle(Vk##type instance)										\
		{																						\
			auto* dispatchable = reinterpret_cast<DispatchableObject<type>*>(instance);			\
			if (!dispatchable)																	\
				return nullptr;																	\
			if (dispatchable->Object.GetObjectType() != type::ObjectType)						\
			{																					\
				CCT_ASSERT_FALSE("Invalid Object Type for: " #type);							\
				return nullptr;																	\
			}																					\
			return &dispatchable->Object;														\
		}

#define GLGPUS_LOG_CONTEXT_MANIPULATION

using GLenum = cct::Int32;
using GLfloat = cct::Float32;
using GLclampf = cct::Float32;
using GLsizei = cct::Int32;
using GLuint = cct::UInt32;
using GLint = cct::Int32;
using GLvoid = void;
using GLbitfield = cct::UInt32;
using GLclampd = cct::Float64;
using GLdouble = cct::Float64;
using GLbyte = cct::Byte;
using GLubyte = cct::UByte;
using GLshort = cct::Int16;
using GLushort = cct::UInt16;
using GLboolean = cct::UInt8;

namespace glgpus
{
	enum class glgpusResult : cct::UInt32
	{
		Success = 0,
		OutOfHostMemory,
		InvalidArg,
		Unknown
	};

	inline std::underlying_type_t<glgpusResult> MakeResult(glgpusResult result, std::string_view message = {})
	{
		if (result != glgpusResult::Success)
		{
			cct::Logger::Error("Invalid result: {}, message: '{}'", static_cast<std::underlying_type_t<glgpusResult>>(result), message);
		}
		return static_cast<std::underlying_type_t<glgpusResult>>(result);
	}

	enum class VendorId : cct::UInt32
	{
		Microsoft = 0x1414,
		Amd = 0x1022,
		Nvidia = 0x10DE,
		Qualcomm = 0x17CB,
		Intel = 0x8086
	};


	inline std::string ToUtf8(const wchar_t* wstr)
	{
		char* oldLocale = std::setlocale(LC_CTYPE, "en_US.utf8");
		cct::DeferredExit _([&]()
		{
			std::setlocale(LC_CTYPE, oldLocale);
		});

		std::mbstate_t state = {};
		std::size_t len = std::wcsrtombs(nullptr, &wstr, 0, &state);
		if (len == std::numeric_limits<std::size_t>::max())
		{
			CCT_ASSERT_FALSE("Invalid size");
			return {};
		}

		std::string name;
		name.resize(len);
		std::wcstombs(name.data(), wstr, name.size());

		return name;
	}


}