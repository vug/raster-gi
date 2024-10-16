/*
 * TODO(vug): New opengl.hpp file with OpenGL types. put in gl namespace.
 * TODO(vug): Either remove the double window/context creation path for modern
 * pixel buffer choice or make it optional
 * TODO(vug): Better function pointer naming scheme than PFNGLCLEARCOLORPROC
 * ref:
 * https://www.khronos.org/opengl/wiki/Platform_specifics:_Windows
 * https://gist.github.com/nickrolfe/1127313ed1dbf80254b614a721b3ee9c
 * https://www.khronos.org/opengl/wiki/Creating_an_OpenGL_Context_(WGL)
 * https://glad.dav1d.de/
 */

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
// #include <gl/GL.h>
typedef unsigned int GLenum;
typedef float GLfloat;
typedef unsigned int GLbitfield;
typedef unsigned char GLubyte;
typedef unsigned int GLuint;
#define GL_TRUE 1
#define GL_FALSE 0
#define GL_VERSION 0x1F02
// WINGDIAPI const GLubyte *APIENTRY glGetString(GLenum name);
#define GL_COLOR_BUFFER_BIT 0x00004000

#include <print>

void fatal(const char *msg) {
  HANDLE stdErr = GetStdHandle(STD_ERROR_HANDLE);
  const char *prefix = "[FATAL] ";
  WriteConsoleA(stdErr, prefix, static_cast<DWORD>(strlen(prefix)), nullptr, nullptr);
  WriteConsoleA(stdErr, msg, static_cast<DWORD>(strlen(msg)), nullptr, nullptr);
}

using PFNWGLCREATECONTEXTATTRIBSARBPROC =
    HGLRC(APIENTRY *)(HDC hDC, HGLRC hShareContext, const int *attribList);
PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB{};

// See
// https://www.khronos.org/registry/OpenGL/extensions/ARB/WGL_ARB_create_context.txt
// for all values
#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092
#define WGL_CONTEXT_PROFILE_MASK_ARB 0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001

using PFNWGLCHOOSEPIXELFORMATARBPROC = bool(APIENTRY *)(
    HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList,
    UINT nMaxFormats, int *piFormats, UINT *nNumFormats);
PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB;

// See
// https://www.khronos.org/registry/OpenGL/extensions/ARB/WGL_ARB_pixel_format.txt
// for all values
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
    fatal("failed to get a valid device context.");
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
    fatal("Failed to initialize OpenGL function wglCreateContextAttribsARB");
  }
  wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress(
      "wglChoosePixelFormatARB");
  if (!wglChoosePixelFormatARB) {
    fatal("Failed to initialize OpenGL function wglChoosePixelFormatARB");
  }

  wglMakeCurrent(dummyDeviceContextHandle, 0);
  wglDeleteContext(dummyGlContext);
  ReleaseDC(dummyWindowHandle, dummyDeviceContextHandle);
  DestroyWindow(dummyWindowHandle);
}

// from: https://www.khronos.org/opengl/wiki/Load_OpenGL_Functions
// Functions belonging to OpenGL v1.1 or less cannot be loaded via
// wglGetProcAddress we'll get them directly from opengl32.dll that comes with
// Windows
void *GetAnyGLFuncAddress(const char *name) {
  void *p = (void *)wglGetProcAddress(name);
  if (p == 0 || (p == (void *)0x1) || (p == (void *)0x2) ||
      (p == (void *)0x3) || (p == (void *)-1)) {
    HMODULE module = LoadLibraryA("opengl32.dll");
    p = (void *)GetProcAddress(module, name);
  }

  return p;
}

// OpenGL function pointers
// APIENTRY is WINAPI which is __stdcall

#define MAKE_FUNC_PTR_TYPE(method, funcPtrType, returnType, ...) \
  using funcPtrType = returnType(APIENTRY *)(__VA_ARGS__);       \
  funcPtrType method{};

#define GET_PROC_ADDRESS(method, funcPtrType)                         \
  method = (funcPtrType)GetAnyGLFuncAddress(#method);                 \
  if (!method) {                                                      \
    std::println("Failed to initialize OpenGL function {}", #method); \
  }

// struct GlLib {
MAKE_FUNC_PTR_TYPE(glClearColor, PFNGLCLEARCOLORPROC, void, GLfloat, GLfloat,
                   GLfloat, GLfloat);
MAKE_FUNC_PTR_TYPE(glClear, PFNGLCLEARPROC, void, GLbitfield);
MAKE_FUNC_PTR_TYPE(glCreateProgram, PFNGLCREATEPROGRAMPROC, GLuint);
// MAKE_FUNC_PTR_TYPE(glViewport, PFNGLVIEWPORTPROC, void, GLint, GLint,
// GLsizei, GLsizei);

void initFunctions() {
  GET_PROC_ADDRESS(glClearColor, PFNGLCLEARCOLORPROC);
  GET_PROC_ADDRESS(glClear, PFNGLCLEARPROC);
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
    fatal("Failed to choose the pixel format.");
  }
  PIXELFORMATDESCRIPTOR pfd;
  DescribePixelFormat(deviceContextHandle, pixel_format, sizeof(pfd), &pfd);
  if (!SetPixelFormat(deviceContextHandle, pixel_format, &pfd)) {
    fatal("Failed to set the pixel format.");
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
    fatal("Failed to change OpenGL context to version 'Core 4.6'");
  }

  ShowWindow(windowHandle, SW_SHOW);
  UpdateWindow(windowHandle);

  // gl.initFunctions();
  initFunctions();

  // std::println("OpenGL version: {}", (char *)glGetString(GL_VERSION));

  GLuint p1 = glCreateProgram();
  GLuint p2 = glCreateProgram();
  std::println("program1 {}, program2 {}", p1, p2);

  while (!GetAsyncKeyState(VK_ESCAPE)) {
    glClearColor(1.0f, 0.5f, 0.25f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    SwapBuffers(deviceContextHandle);
  }
}
