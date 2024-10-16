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

#include "opengl.hpp"
// #include <gl/GL.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <print>

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
