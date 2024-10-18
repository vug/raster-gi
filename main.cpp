/*
 * Original idea: https://iquilezles.org/articles/simplegi/
 * TODO(vug): Either remove the double window/context creation path for modern
 * pixel buffer choice or make it optional -> Old and Modern versions of pixel
 * and context functions.
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

static inline float getTime() {
  static LARGE_INTEGER freq = []() {
    LARGE_INTEGER f;
    QueryPerformanceFrequency(&f);
    return f;
  }();
  LARGE_INTEGER ticks;
  QueryPerformanceCounter(&ticks);
  return ticks.QuadPart / static_cast<float>(freq.QuadPart);
}

struct Vec3 {
  float x;
  float y;
  float z;
};

struct Mesh {
  unsigned int numVertices{};
  Vec3* positions{};
  Vec3* normals{};
  Vec3* colors{};
  // ~Mesh() { delete[] positions; delete[] normals; delete[] colors; }
};

Mesh readMeshFromFile(LPCWSTR fileName) {
  HANDLE hFile = CreateFile(fileName, GENERIC_READ, 0, NULL, OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL, NULL);
  if (hFile == INVALID_HANDLE_VALUE) {
    fatal("Failed to open file");
  }

  Mesh mesh;
  DWORD bytesRead = 0;
  if (!ReadFile(hFile, &mesh.numVertices, sizeof(int), &bytesRead, NULL) ||
      bytesRead != sizeof(int)) {
    fatal("Failed to read numVertices from file.");
  };

  const size_t attrSizeBytes = mesh.numVertices * sizeof(Vec3);
  for (Vec3* attr : {mesh.positions, mesh.normals, mesh.colors}) {
    if (!ReadFile(hFile, attr, attrSizeBytes, &bytesRead, NULL) ||
        bytesRead != attrSizeBytes) {
      fatal("Failed to read position data.");
    }
  }

  return mesh;
}

int main() {
  loadWglCreateContextAttribsARB();
  HDC dev;
  createAndShowWindow("RasterGI", 1024, 1024, dev);
  setPixelFormatFancy(dev);
  createAndMakeOpenGlContext(dev);

  initGlFunctions();
  std::println("OpenGL version: {}", (char*)glGetString(GL_VERSION));

  const char* vertSrc = R"glsl(
#version 460

out vec3 fragColor;

vec2 positions[3] = vec2[](vec2 (-0.5, -0.5), vec2 (0.5, -0.5), vec2 (0, 0.5));
vec3 colors[3] = vec3[](vec3 (1.0, 0.0, 0.0), vec3 (0.0, 1.0, 0.0), vec3 (0.0, 0.0, 1.0));

void main ()
{
	gl_Position = vec4 (positions[gl_VertexID], 0.0, 1.0);
	fragColor = colors[gl_VertexID];
}  
)glsl";

  const char* fragSrc = R"glsl(
#version 460
in vec3 fragColor;

layout (location = 0) out vec4 outColor;

void main () { 
  outColor = vec4(fragColor, 1.0); 
}
)glsl";
  GLuint prog = compileShader(vertSrc, fragSrc);
  glUseProgram(prog);

  uint32_t vao;
  glGenVertexArrays(1, &vao);

  float t0 = getTime();
  while (!GetAsyncKeyState(VK_ESCAPE)) {
    const float t = getTime() - t0;
    const float dt = t - t0;

    glClearColor(fmod(t, 1.f), 1.f - fmod(t * 2.f + .5f, 1.f),
                 fmod(t * 0.33f + 0.33f, 1.f), 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    SwapBuffers(dev);
  }
}
