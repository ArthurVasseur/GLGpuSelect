#pragma once
#include "GlLoader/OpenGlFunctions.hpp"

namespace gl
{

	class GlLayer
	{
	public:
		virtual ~GlLayer() = default;
		virtual glgpus::OpenGlDispatchTable BuildDispatchTable(const glgpus::OpenGlDispatchTable& next) = 0;
	};

	class GlLayerBase : public GlLayer
	{
	public:
		glgpus::OpenGlDispatchTable BuildDispatchTable(const glgpus::OpenGlDispatchTable& next) override
		{
			return next;
		}
	};

} // namespace gl
