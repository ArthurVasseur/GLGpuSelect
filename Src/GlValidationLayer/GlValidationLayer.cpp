#include "GlValidationLayer/GlValidationLayer.hpp"

namespace gl
{

	glgpus::OpenGlDispatchTable GlValidationLayer::BuildDispatchTable(const glgpus::OpenGlDispatchTable& next)
	{
		return next;
	}

} // namespace gl
