#include "opengl.hpp"

void fatal(const char *msg) {
  HANDLE stdErr = GetStdHandle(STD_ERROR_HANDLE);
  const char *prefix = "[FATAL] ";
  WriteConsoleA(stdErr, prefix, static_cast<DWORD>(strlen(prefix)), nullptr,
                nullptr);
  WriteConsoleA(stdErr, msg, static_cast<DWORD>(strlen(msg)), nullptr, nullptr);
}

#define DEFINE_FUNC_PTR_TYPE(method) FnPtrT(method) method{};

#define GET_PROC_ADDRESS(method)                           \
  method = (FnPtrT(method))GetAnyGLFuncAddress(#method);   \
  if (!method) {                                           \
    fatal("Failed to initialize OpenGL function #method"); \
  }

DEFINE_FUNC_PTR_TYPE(wglCreateContextAttribsARB);
DEFINE_FUNC_PTR_TYPE(wglChoosePixelFormatARB);
DEFINE_FUNC_PTR_TYPE(glClearColor);
DEFINE_FUNC_PTR_TYPE(glClear);
DEFINE_FUNC_PTR_TYPE(glCreateProgram);

void *GetAnyGLFuncAddress(const char *name) {
  void *p = (void *)wglGetProcAddress(name);
  if (p == 0 || (p == (void *)0x1) || (p == (void *)0x2) ||
      (p == (void *)0x3) || (p == (void *)-1)) {
    HMODULE module = LoadLibraryA("opengl32.dll");
    p = (void *)GetProcAddress(module, name);
  }

  return p;
}

void initFunctions() {
  GET_PROC_ADDRESS(glClearColor);
  GET_PROC_ADDRESS(glClear);
  GET_PROC_ADDRESS(glCreateProgram);
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

  wglCreateContextAttribsARB =
      (FnPtrT(wglCreateContextAttribsARB))wglGetProcAddress(
          "wglCreateContextAttribsARB");
  if (!wglCreateContextAttribsARB) {
    fatal("Failed to initialize OpenGL function wglCreateContextAttribsARB");
  }
  wglChoosePixelFormatARB = (FnPtrT(wglChoosePixelFormatARB))wglGetProcAddress(
      "wglChoosePixelFormatARB");
  if (!wglChoosePixelFormatARB) {
    fatal("Failed to initialize OpenGL function wglChoosePixelFormatARB");
  }

  wglMakeCurrent(dummyDeviceContextHandle, 0);
  wglDeleteContext(dummyGlContext);
  ReleaseDC(dummyWindowHandle, dummyDeviceContextHandle);
  DestroyWindow(dummyWindowHandle);
}