#include "GlValidationLayer/GlValidationLayer.hpp"

extern "C" __declspec(dllexport) gl::GlLayer* CCT_CALL glLayerCreate()
{
	return new gl::GlValidationLayer();
}

extern "C" __declspec(dllexport) void CCT_CALL glLayerDestroy(gl::GlLayer* layer)
{
	delete layer;
}
