#include "opengl.hpp"

void fatal(const char *msg) {
  HANDLE stdErr = GetStdHandle(STD_ERROR_HANDLE);
  const char *prefix = "[FATAL] ";
  WriteConsoleA(stdErr, prefix, static_cast<DWORD>(strlen(prefix)), nullptr,
                nullptr);
  WriteConsoleA(stdErr, msg, static_cast<DWORD>(strlen(msg)), nullptr, nullptr);
  ExitProcess(0);
}

void createAndShowWindow(const char *name, int with, int height,
                         HDC &deviceContextHandle) {
  HWND windowHandle = CreateWindowEx(
      0,                                // Optional window styles
      L"STATIC",                        // Predefined class name (STATIC)
      L"RasterGI",                      // Window title
      WS_OVERLAPPEDWINDOW | SS_CENTER,  // Window style with centering text
      CW_USEDEFAULT, CW_USEDEFAULT, with, height,  // Size and position
      NULL,                                        // Parent window
      NULL,                                        // Menu
      GetModuleHandle(NULL),                       // Instance handle
      NULL  // Additional application data
  );
  deviceContextHandle = GetDC(windowHandle);

  ShowWindow(windowHandle, SW_SHOW);
  UpdateWindow(windowHandle);
}

void setPixelFormatFancy(HDC deviceContextHandle) {
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
}

void createAndMakeOpenGlContext(HDC deviceContextHandle) {
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
}

#define DEFINE_FUNC_PTR_TYPE(method) FnPtrT(method) method{};

#define GET_PROC_ADDRESS(method)                           \
  method = (FnPtrT(method))GetAnyGLFuncAddress(#method);   \
  if (!method) {                                           \
    fatal("Failed to initialize OpenGL function #method"); \
  }

DEFINE_FUNC_PTR_TYPE(wglCreateContextAttribsARB);
DEFINE_FUNC_PTR_TYPE(wglChoosePixelFormatARB);
//
DEFINE_FUNC_PTR_TYPE(glClearColor);
DEFINE_FUNC_PTR_TYPE(glClear);
DEFINE_FUNC_PTR_TYPE(glGetString);
DEFINE_FUNC_PTR_TYPE(glCreateShader);
DEFINE_FUNC_PTR_TYPE(glShaderSource);
DEFINE_FUNC_PTR_TYPE(glCompileShader);
DEFINE_FUNC_PTR_TYPE(glGetShaderiv);
DEFINE_FUNC_PTR_TYPE(glCreateProgram);
DEFINE_FUNC_PTR_TYPE(glAttachShader);
DEFINE_FUNC_PTR_TYPE(glLinkProgram);
DEFINE_FUNC_PTR_TYPE(glGetProgramiv);
DEFINE_FUNC_PTR_TYPE(glGetShaderInfoLog);
DEFINE_FUNC_PTR_TYPE(glGetProgramInfoLog);
DEFINE_FUNC_PTR_TYPE(glGenVertexArrays);
DEFINE_FUNC_PTR_TYPE(glUseProgram);
DEFINE_FUNC_PTR_TYPE(glBindVertexArray);
DEFINE_FUNC_PTR_TYPE(glDrawArrays);

void *GetAnyGLFuncAddress(const char *name) {
  void *p = (void *)wglGetProcAddress(name);
  if (p == 0 || (p == (void *)0x1) || (p == (void *)0x2) ||
      (p == (void *)0x3) || (p == (void *)-1)) {
    HMODULE module = LoadLibraryA("opengl32.dll");
    p = (void *)GetProcAddress(module, name);
  }

  return p;
}

void initGlFunctions() {
  GET_PROC_ADDRESS(glClearColor);
  GET_PROC_ADDRESS(glClear);
  GET_PROC_ADDRESS(glGetString);
  GET_PROC_ADDRESS(glCreateShader);
  GET_PROC_ADDRESS(glShaderSource);
  GET_PROC_ADDRESS(glCompileShader);
  GET_PROC_ADDRESS(glGetShaderiv);
  GET_PROC_ADDRESS(glCreateProgram);
  GET_PROC_ADDRESS(glAttachShader);
  GET_PROC_ADDRESS(glLinkProgram);
  GET_PROC_ADDRESS(glGetProgramiv);
  GET_PROC_ADDRESS(glGetShaderInfoLog);
  GET_PROC_ADDRESS(glGetProgramInfoLog);
  GET_PROC_ADDRESS(glGenVertexArrays);
  GET_PROC_ADDRESS(glUseProgram);
  GET_PROC_ADDRESS(glBindVertexArray);
  GET_PROC_ADDRESS(glDrawArrays);
  // GET_PROC_ADDRESS(glViewport, PFNGLVIEWPORTPROC);
}

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

  GET_PROC_ADDRESS(wglCreateContextAttribsARB);
  GET_PROC_ADDRESS(wglChoosePixelFormatARB);

  wglMakeCurrent(dummyDeviceContextHandle, 0);
  wglDeleteContext(dummyGlContext);
  ReleaseDC(dummyWindowHandle, dummyDeviceContextHandle);
  DestroyWindow(dummyWindowHandle);
}

GLuint compileShader(const char *vertSrc, const char *fragSrc) {
  GLint success{};

  GLuint vert = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vert, 1, &vertSrc, nullptr);
  glCompileShader(vert);
  glGetShaderiv(vert, GL_COMPILE_STATUS, &success);
  if (!success) {
    GLchar message[1024];
    glGetShaderInfoLog(vert, 1024, nullptr, message);
    fatal(message);
  }

  GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(frag, 1, &fragSrc, nullptr);
  glCompileShader(frag);
  glGetShaderiv(frag, GL_COMPILE_STATUS, &success);
  if (!success) {
    GLchar msg[1024];
    glGetShaderInfoLog(frag, 1024, nullptr, msg);
    fatal(msg);
  }

  GLuint prog = glCreateProgram();
  glAttachShader(prog, vert);
  glAttachShader(prog, frag);
  glLinkProgram(prog);

  glGetProgramiv(prog, GL_LINK_STATUS, &success);
  if (!success) {
    GLchar msg[1024];
    glGetProgramInfoLog(prog, 1024, nullptr, msg);
    fatal(msg);
  }

  return prog;
}