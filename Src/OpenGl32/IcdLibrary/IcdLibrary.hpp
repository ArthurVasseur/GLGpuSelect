//
// Created by arthur on 18/05/2025.
//

#pragma once

#include <Concerto/Core/DynLib/DynLib.hpp>

namespace glgpus
{
	class IcdLibrary
	{
	public:
		IcdLibrary() = default;
		virtual ~IcdLibrary() = default;

		virtual bool Load(std::string_view icdPath) = 0;
		bool IsLoaded() const;

		cct::DynLib& GetIcd()
		{
			return m_icd;
		}
	protected:
		cct::DynLib m_icd;
	};
} // namespace glgpus
