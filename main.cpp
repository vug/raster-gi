/*
 * TODO(vug): Either remove the double window/context creation path for modern
 * pixel buffer choice or make it optional
 * TODO(vug): try not using CRT and produce small executable
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
  loadWglCreateContextAttribsARB();
  HDC dev;
  createAndShowWindow("RasterGI", 1024, 768, dev);
  setPixelFormatFancy(dev);
  createAndMakeOpenGlContext(dev);
  // std::println("OpenGL version: {}", (char *)glGetString(GL_VERSION));

  initGlFunctions();

  GLuint p1 = glCreateProgram();
  GLuint p2 = glCreateProgram();
  std::println("program1 {}, program2 {}", p1, p2);

  while (!GetAsyncKeyState(VK_ESCAPE)) {
    glClearColor(1.0f, 0.5f, 0.25f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    SwapBuffers(dev);
  }
}
