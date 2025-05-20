# Overview

GLGpuSelect is a cross-platform, drop-in replacement for opengl32.dll on Windows and libGL.so on Linux, allowing you to select which GPU an OpenGL application uses—per-application—without modifying the application itself. It intercepts OpenGL calls at the library level and reroutes them to the GPU driver of your choice, making it ideal for systems with both integrated and discrete GPUs.
Features

- Per-application GPU selection
    Route individual OpenGL applications to either your integrated or discrete GPU without system-wide settings.

- Zero-code changes required
    Acts as a drop-in replacement: simply replace your existing OpenGL library.

- Cross-platform support
    Works on Windows, Linux will come in the future.

- Customizable
    Provides environment variables, or a simple `C` API to select the GPU.

# Usage

# Environment Variables
`GLGPUS_ADAPTER_OS_INDEX`: Set the GPU index for the current process.

# C API

```c
#include <GLGpuSelect.h>

int main(void)
{

	uint32_t deviceCount = 0;
	uint32_t result = glgpusEnumerateDevices(&deviceCount, NULL);
	if (result != 0)
		return EXIT_FAILURE;

	AdapterInfo* adapters = malloc(deviceCount * sizeof(AdapterInfo));

	result = glgpusEnumerateDevices(&deviceCount, adapters);
	if (result != 0)
		return EXIT_FAILURE;

	printf("Available devices:\n");
	for (uint32_t i = 0; i < deviceCount; ++i)
	{
		printf("\t%s\n", adapters[i].Name);
	}

	glgpusChooseDevice(adapters[0].Uuid);

	free(adapters);

	//Then call your OpenGL init code here ChoosePixelFormat, on windows, eglChooseConfig on Linux, etc.

	return EXIT_SUCCESS;
}
```

# License

This project is licensed under the GPL-3.0 License. See the LICENSE file for details.
A separate COMMERCIAL_LICENSE.txt is provided for commercial usage scenarios.