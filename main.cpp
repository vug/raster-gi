#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <gl/GL.h>

#include <print>

void createOpenGlContext(HWND windowHandle) {
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
  HDC deviceContextHandle = GetDC(windowHandle);
  if (!deviceContextHandle) {
    std::println(stderr, "failed to get a valid device context.");
    ExitProcess(0);
  }

  int pixelFormat =
      ChoosePixelFormat(deviceContextHandle, &pixelFormatDescriptor);
  SetPixelFormat(deviceContextHandle, pixelFormat, &pixelFormatDescriptor);

  HGLRC hglrc = wglCreateContext(deviceContextHandle);
  wglMakeCurrent(deviceContextHandle, hglrc);
}

// OpenGL function pointers
// APIENTRY is WINAPI which is __stdcall
using PFNGLCREATEPROGRAMPROC = GLuint(APIENTRY *)();
PFNGLCREATEPROGRAMPROC glCreateProgram{};

void InitializeOpenGLFunctions() {
  glCreateProgram =
      (PFNGLCREATEPROGRAMPROC)wglGetProcAddress("glCreateProgram");
  if (!glCreateProgram) {
    std::println(stderr, "Failed to initialize OpenGL function {}",
                 "glCreateProgram");
  }
}

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
  ShowWindow(windowHandle, SW_SHOW);

  createOpenGlContext(windowHandle);

  InitializeOpenGLFunctions();

  GLuint p1 = glCreateProgram();
  GLuint p2 = glCreateProgram();
  std::println("program1 {}, program2 {}", p1, p2);

  while (!GetAsyncKeyState(VK_ESCAPE)) {
  }

  ExitProcess(0);
}