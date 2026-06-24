#pragma once
#include "GlLayer/GlLayer.hpp"

namespace gl {

class GlValidationLayer final : public GlLayerBase {
public:
    GlValidationLayer() = default;
    glgpus::OpenGlDispatchTable BuildDispatchTable(const glgpus::OpenGlDispatchTable& next) override;
};

} // namespace gl
