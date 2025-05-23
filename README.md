![Build Status](https://img.shields.io/github/actions/workflow/status/ArthurVasseur/GLGpuSelect/ci.yml)
![Coverage](https://img.shields.io/codecov/c/gh/ArthurVasseur/GLGpuSelect)
![License](https://img.shields.io/badge/License-GPLv3-blue)

# 🚀 GLGpuSelect

**OpenGL GPU Selection • Cross-Platform • Drop‑In Shim • Zero Code Changes**

GLGpuSelect is a **drop‑in replacement** for `opengl32.dll` (Windows) and — coming soon — `libGL.so` (Linux), enabling **per‑application GPU routing** for any OpenGL app. No modifications, no global OS tweaks: just swap in the shim and pick your integrated or discrete GPU.

<div align="center">
✨ **Key Benefits:** ✨

* **gpu-selection**: Per-application GPU choice without system‑wide settings
* **zero-code-changes**: Seamless library swap, no app code edits
* **hardware-acceleration**: Full performance on your chosen GPU
* **cross-platform**: Windows today, Linux support soon
* **gpu-affinity**: Control via environment variables or C-API

</div>

---

## 📋 Table of Contents

1. [Installation](#-installation)
2. [Quick Start](#-quick-start)
3. [Usage](#-usage)
4. [Examples & Documentation](#-examples--documentation)
5. [Repository Structure](#-repository-structure)
6. [Contributing](#-contributing)
7. [Releases & Changelog](#-releases--changelog)
8. [License](#-license)
9. [Topics](#-topics)

---

## 🛠️ Installation

### Prerequisites

* **Xmake**: [https://xmake.io/#/](https://xmake.io/#/)

### Clone & Configure

```bash
git clone https://github.com/ArthurVasseur/GLGpuSelect.git
cd GLGpuSelect
```

### Configure Build Options

| Flag          | Description                | Default |
| ------------- | -------------------------- | :-----: |
| `--examples`  | Build example applications |    no   |
| `--profiling` | Enable profiling hooks     |    no   |
| `--tests`     | Build unit tests           |    no   |
| `--asserts`   | Enable runtime assertions  |    no   |
| `--logging`   | Enable verbose logging     |    no   |

```bash
xmake config -m debug --examples=y --profiling=y --tests=y --asserts=y --logging=y
```

### Build & Install

```bash
xmake         # compile
xmake install -o /usr/local   # or your chosen path
```

Then copy the `libGLGpuSelect.so` or `opengl32.dll` to your OpenGL app directory.

---

## 🚀 Quick Start

### Environment Variable

```bash
# Select GPU by OS adapter index (0 = first GPU, 1 = second, etc.)
export GLGPUS_ADAPTER_OS_INDEX=1
./your_opengl_app
```

### C API

```c
#include <GLGpuSelect.h>
#include <stdlib.h>
#include <stdio.h>

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

---

## 📚 Examples & Documentation

* Demos: see `examples/` folder
* Full docs & tutorials: [https://arthurvasseur.github.io/GLGpuSelect](https://arthurvasseur.github.io/GLGpuSelect)
* FAQ & troubleshooting: see `docs/FAQ.md`

---

## 🤝 Contributing

We welcome contributions!

1. Fork the repo
2. Create a branch (`git checkout -b feat/awesome`)
3. Commit (`git commit -m "Add awesome feature"`)
4. Push (`git push origin feat/awesome`)
5. Open a Pull Request

See [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines on code style, testing, and reviews.

---

## 📝 Releases & Changelog

* Stay up‑to‑date: new features, fixes, and enhancements are tagged as `vX.Y.Z` in GitHub Releases.
* See [CHANGELOG.md](CHANGELOG.md).

---

## 📜 License

Dual‑licensed under:

* **GPL‑3.0** (open‑source): See [LICENSE](LICENSE)
* **Commercial**: See [COMMERCIAL\_LICENSE.txt](COMMERCIAL_LICENSE.txt)

---

## 🔖 Topics

`opengl`, `gpu-selection`, `gpu-affinity`, `gpu-routing`, `multi-gpu`, `wgl`, `egl`, `nvoptimus`, `amd-powerxpress`, `cross-platform`, `drop-in`, `shim`, `xmake`, `c-api`, `environment-variables`, `performance`, `hardware-acceleration`, `rendering`, `game-development`, `gpl-3.0`, `dual-license`
