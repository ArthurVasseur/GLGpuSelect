#define CATCH_CONFIG_RUNNER
#include <catch2/catch_test_macros.hpp>
#include <Windows.h>
#include <GL/gl.h>

namespace
{
	class TestWindow
	{
	public:
		TestWindow()
		{
			WNDCLASSA wc = {};
			wc.style = CS_OWNDC;
			wc.lpfnWndProc = DefWindowProcA;
			wc.hInstance = GetModuleHandle(nullptr);
			wc.lpszClassName = "TestWindowClass";
			RegisterClassA(&wc);

			hwnd = CreateWindow(wc.lpszClassName, "WGL Test Window",
				WS_OVERLAPPEDWINDOW,
				CW_USEDEFAULT, CW_USEDEFAULT,
				100, 100,
				nullptr, nullptr,
				wc.hInstance, nullptr);
			REQUIRE(hwnd != nullptr);

			hdc = ::GetDC(hwnd);
			REQUIRE(hdc != nullptr);
		}

		~TestWindow()
		{
			if (hdc) {
				ReleaseDC(hwnd, hdc);
			}
			if (hwnd) {
				DestroyWindow(hwnd);
			}
			UnregisterClassA("TestWindowClass", GetModuleHandle(nullptr));
		}

		HDC GetDC() const { return hdc; }
		HWND GetHWND() const { return hwnd; }

	private:
		HWND hwnd = nullptr;
		HDC hdc = nullptr;
	};

	class GLContext
	{
	public:
		GLContext(HDC hdc) : hdc(hdc)
		{
			PIXELFORMATDESCRIPTOR pfd = {};
			pfd.nSize = sizeof(pfd);
			pfd.nVersion = 1;
			pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
			pfd.iPixelType = PFD_TYPE_RGBA;
			pfd.cColorBits = 24;
			pfd.cDepthBits = 16;
			pfd.iLayerType = PFD_MAIN_PLANE;

			int pixelFormat = ChoosePixelFormat(hdc, &pfd);
			REQUIRE(pixelFormat != 0);

			BOOL setPF = SetPixelFormat(hdc, pixelFormat, &pfd);
			REQUIRE(setPF == TRUE);

			hglrc = wglCreateContext(hdc);
			REQUIRE(hglrc != nullptr);

			BOOL madeCurrent = wglMakeCurrent(hdc, hglrc);
			REQUIRE(madeCurrent == TRUE);
		}

		~GLContext()
		{
			if (hdc && hglrc)
			{
				wglMakeCurrent(nullptr, nullptr);
				wglDeleteContext(hglrc);
			}
		}

	private:
		HDC hdc;
		HGLRC hglrc = nullptr;
	};
}

TEST_CASE("Pixel Format Selection and Setting", "[wgl][pixelformat]")
{
	TestWindow win;
	HDC hdc = win.GetDC();

	PIXELFORMATDESCRIPTOR pfd;
	ZeroMemory(&pfd, sizeof(pfd));
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cAlphaBits = 8;

	int fmt = ChoosePixelFormat(hdc, &pfd);
	REQUIRE(fmt > 0);

	BOOL ok = SetPixelFormat(hdc, fmt, &pfd);
	REQUIRE(ok == TRUE);
}

TEST_CASE("OpenGL Context Creation and Management", "[wgl][context]")
{
	TestWindow win;
	GLContext ctx(win.GetDC());

	const GLubyte* vendor = glGetString(GL_VENDOR);
	auto glError = glGetError();
	REQUIRE(glError == GL_NO_ERROR);
}

TEST_CASE("Resource Cleanup and Error Handling", "[wgl][cleanup]")
{
	TestWindow window;
	HDC hdc = window.GetDC();
	{
		GLContext ctx(hdc);
		// Intentionally cause an error: invalid enum
		glBindTexture(GL_TEXTURE_2D, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, 0xDEADBEEF);
		GLenum err = glGetError();
		REQUIRE(err != GL_NO_ERROR);
	}

	REQUIRE(wglGetCurrentContext() == nullptr);

}