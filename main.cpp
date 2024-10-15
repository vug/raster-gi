#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

int main() {
  HWND hwnd = CreateWindowEx(
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

  ShowWindow(hwnd, SW_SHOW);

  while (!GetAsyncKeyState(VK_ESCAPE)) {
  }

  ExitProcess(0);
}