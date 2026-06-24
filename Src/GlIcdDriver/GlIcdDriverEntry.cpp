#include "GlIcdDriver/GlIcdDriver.hpp"

#ifdef CCT_PLATFORM_WINDOWS

extern "C" CCT_EXPORT gl::GlDriver* CCT_CALL glDriverCreate(const char* icdPath)
{
	auto* driver = new(std::nothrow) gl::GlIcdDriver();
	if (driver == nullptr)
	{
		return nullptr;
	}
	if (!driver->Load(icdPath))
	{
		delete driver;
		return nullptr;
	}
	return driver;
}

extern "C" CCT_EXPORT void CCT_CALL glDriverDestroy(gl::GlDriver* driver)
{
	delete driver;
}

#endif // CCT_PLATFORM_WINDOWS
