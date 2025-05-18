//
// Created by arthur on 18/05/2025.
//

#include "OpenGl32/IcdLibrary/IcdLibrary.hpp"

namespace glgpus
{
	bool IcdLibrary::IsLoaded() const
	{
		return m_icd.IsLoaded();
	}
} // namespace glgpus
