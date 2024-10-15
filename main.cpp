#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <gl/GL.h>

#include <print>
using PFNWGLCREATECONTEXTATTRIBSARBPROC =
    HGLRC(APIENTRY *)(HDC hDC, HGLRC hShareContext, const int *attribList);
PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB{};

// See https://www.khronos.org/registry/OpenGL/extensions/ARB/WGL_ARB_create_context.txt for all values
#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092
#define WGL_CONTEXT_PROFILE_MASK_ARB 0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001

using PFNWGLCHOOSEPIXELFORMATARBPROC = bool(APIENTRY *)(
    HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList,
    UINT nMaxFormats, int *piFormats, UINT *nNumFormats);
PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB;

// See https://www.khronos.org/registry/OpenGL/extensions/ARB/WGL_ARB_pixel_format.txt for all values
#define WGL_DRAW_TO_WINDOW_ARB 0x2001
#define WGL_ACCELERATION_ARB 0x2003
#define WGL_SUPPORT_OPENGL_ARB 0x2010
#define WGL_DOUBLE_BUFFER_ARB 0x2011
#define WGL_PIXEL_TYPE_ARB 0x2013
#define WGL_COLOR_BITS_ARB 0x2014
#define WGL_DEPTH_BITS_ARB 0x2022
#define WGL_STENCIL_BITS_ARB 0x2023
#define WGL_FULL_ACCELERATION_ARB 0x2027
#define WGL_TYPE_RGBA_ARB 0x202B

void loadWglCreateContextAttribsARB() {
  HWND dummyWindowHandle = CreateWindowEx(
      0,                                // Optional window styles
      L"STATIC",                        // Predefined class name (STATIC)
      L"Dummy Window",                  // Window title
      WS_OVERLAPPEDWINDOW | SS_CENTER,  // Window style with centering text
      CW_USEDEFAULT, CW_USEDEFAULT, 1024, 768,  // Size and position
      NULL,                                     // Parent window
      NULL,                                     // Menu
      GetModuleHandle(NULL),                    // Instance handle
      NULL                                      // Additional application data
  );
  HDC dummyDeviceContextHandle = GetDC(dummyWindowHandle);
  if (!dummyDeviceContextHandle) {
    std::println(stderr, "failed to get a valid device context.");
    ExitProcess(0);
  }

  // clang-format off
  PIXELFORMATDESCRIPTOR pixelFormatDescriptor = {
    sizeof(PIXELFORMATDESCRIPTOR),
    1,
    PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER, // Support OpenGL, double buffering
    PFD_TYPE_RGBA,
    32, // 32-bit color depth
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    24, // 24-bit depth buffer
    8,  // 8-bit stencil buffer
    0,  // No auxiliary buffer
    PFD_MAIN_PLANE,
    0, 0, 0, 0
  };
  // clang-format on

  int pixelFormat =
      ChoosePixelFormat(dummyDeviceContextHandle, &pixelFormatDescriptor);
  SetPixelFormat(dummyDeviceContextHandle, pixelFormat, &pixelFormatDescriptor);

  HGLRC dummyGlContext = wglCreateContext(dummyDeviceContextHandle);
  wglMakeCurrent(dummyDeviceContextHandle, dummyGlContext);

  wglCreateContextAttribsARB =
      (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress(
          "wglCreateContextAttribsARB");
  if (!wglCreateContextAttribsARB) {
    std::println(stderr, "Failed to initialize OpenGL function {}",
                 "wglCreateContextAttribsARB");
    ExitProcess(0);
  }
  wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress(
      "wglChoosePixelFormatARB");
  if (!wglChoosePixelFormatARB) {
    std::println(stderr, "Failed to initialize OpenGL function {}",
                 "wglChoosePixelFormatARB");
    ExitProcess(0);
  }

  wglMakeCurrent(dummyDeviceContextHandle, 0);
  wglDeleteContext(dummyGlContext);
  ReleaseDC(dummyWindowHandle, dummyDeviceContextHandle);
  DestroyWindow(dummyWindowHandle);
}

// OpenGL function pointers
// APIENTRY is WINAPI which is __stdcall

#define MAKE_FUNC_PTR_TYPE(method, funcPtrType, returnType, ...) \
  using funcPtrType = returnType(APIENTRY *)(__VA_ARGS__);       \
  funcPtrType method{};

#define GET_PROC_ADDRESS(method, funcPtrType)                         \
  method = (funcPtrType)wglGetProcAddress(#method);                   \
  if (!method) {                                                      \
    std::println("Failed to initialize OpenGL function {}", #method); \
  }

// struct GlLib {
// MAKE_FUNC_PTR_TYPE(glClearColor, PFNGLCLEARCOLORPROC, void, GLfloat, GLfloat,
// GLfloat, GLfloat);
// MAKE_FUNC_PTR_TYPE(glClear, PFNGLCLEARPROC, void);
MAKE_FUNC_PTR_TYPE(glCreateProgram, PFNGLCREATEPROGRAMPROC, GLuint);
// MAKE_FUNC_PTR_TYPE(glViewport, PFNGLVIEWPORTPROC, void, GLint, GLint,
// GLsizei, GLsizei);

void initFunctions() {
  // GET_PROC_ADDRESS(glClearColor, PFNGLCLEARCOLORPROC);
  // GET_PROC_ADDRESS(glClear, PFNGLCLEARPROC);
  GET_PROC_ADDRESS(glCreateProgram, PFNGLCREATEPROGRAMPROC);
  // GET_PROC_ADDRESS(glViewport, PFNGLVIEWPORTPROC);
}
//};

// GlLib gl;

int main() {
  HWND windowHandle = CreateWindowEx(
      0,                                // Optional window styles
      L"STATIC",                        // Predefined class name (STATIC)
      L"RasterGI",                      // Window title
      WS_OVERLAPPEDWINDOW | SS_CENTER,  // Window style with centering text
      CW_USEDEFAULT, CW_USEDEFAULT, 1024, 768,  // Size and position
      NULL,                                     // Parent window
      NULL,                                     // Menu
      GetModuleHandle(NULL),                    // Instance handle
      NULL                                      // Additional application data
  );
  HDC deviceContextHandle = GetDC(windowHandle);

  loadWglCreateContextAttribsARB();

  // Now we can choose a pixel format the modern way, using
  // wglChoosePixelFormatARB.
  // clang-format off
  int pixel_format_attribs[] = {
    WGL_DRAW_TO_WINDOW_ARB,     GL_TRUE,
    WGL_SUPPORT_OPENGL_ARB,     GL_TRUE,
    WGL_DOUBLE_BUFFER_ARB,      GL_TRUE,
    WGL_ACCELERATION_ARB,       WGL_FULL_ACCELERATION_ARB,
    WGL_PIXEL_TYPE_ARB,         WGL_TYPE_RGBA_ARB,
    WGL_COLOR_BITS_ARB,         32,
    WGL_DEPTH_BITS_ARB,         24,
    WGL_STENCIL_BITS_ARB,       8,
    0
  };
  // clang-format on

  int pixel_format;
  UINT num_formats;
  wglChoosePixelFormatARB(deviceContextHandle, pixel_format_attribs, 0, 1,
                          &pixel_format, &num_formats);
  if (!num_formats) {
    std::println(stderr, "Failed to choose the OpenGL 3.3 pixel format.");
    ExitProcess(0);
  }
  PIXELFORMATDESCRIPTOR pfd;
  DescribePixelFormat(deviceContextHandle, pixel_format, sizeof(pfd), &pfd);
  if (!SetPixelFormat(deviceContextHandle, pixel_format, &pfd)) {
    std::println(stderr, "Failed to set the OpenGL 3.3 pixel format.");
    ExitProcess(0);
  }

  // Load OpenGL 4.6 context attributes
  // clang-format off
  int attribs[] = {
      WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
      WGL_CONTEXT_MINOR_VERSION_ARB, 6,
      WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
      0  // End of attributes list
  };
  // clang-format on

  // Create OpenGL 4.6 core context
  HGLRC modernContext =
      wglCreateContextAttribsARB(deviceContextHandle, nullptr, attribs);
  if (!wglMakeCurrent(deviceContextHandle, modernContext)) {
    std::println(stderr, "Failed to change OpenGL context to version {}",
                 "core 4.6");
    ExitProcess(0);
  }

  ShowWindow(windowHandle, SW_SHOW);
  UpdateWindow(windowHandle);

  // gl.initFunctions();
  initFunctions();

  std::println("OpenGL version: {}", (char *)glGetString(GL_VERSION));

  GLuint p1 = glCreateProgram();
  GLuint p2 = glCreateProgram();
  std::println("program1 {}, program2 {}", p1, p2);

  while (!GetAsyncKeyState(VK_ESCAPE)) {
    glClearColor(1.0f, 0.5f, 0.25f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    SwapBuffers(deviceContextHandle);
  }

  ExitProcess(0);
}
