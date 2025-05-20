#ifdef __cplusplus
#error Not a C compiler
#endif

#include <stdlib.h>
#include <stdio.h>
#include <OpenGl32/GLGpuSelect.h>

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