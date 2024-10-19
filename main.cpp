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
  unsigned int numIndices{};
  Vec3* positions{};
  Vec3* normals{};
  Vec3* colors{};
  unsigned int* indices{};
  // ~Mesh() { delete[] positions; delete[] normals; delete[] colors; }
};

Mesh readMeshFromFile(const char* fileName) {
  HANDLE hFile = CreateFileA(fileName, GENERIC_READ, 0, NULL, OPEN_EXISTING,
                             FILE_ATTRIBUTE_NORMAL, NULL);
  if (hFile == INVALID_HANDLE_VALUE) {
    fatal("Failed to open file");
  }

  Mesh mesh;
  DWORD bytesRead = 0;
  if (!ReadFile(hFile, &mesh.numVertices, sizeof(unsigned int), &bytesRead,
                nullptr) ||
      bytesRead != sizeof(unsigned int)) {
    fatal("Failed to read numVertices from file.");
  };

  if (!ReadFile(hFile, &mesh.numIndices, sizeof(unsigned int), &bytesRead,
                nullptr) ||
      bytesRead != sizeof(unsigned int)) {
    fatal("Failed to read numIndices from file.");
  }

  const DWORD attrSizeBytes = mesh.numVertices * sizeof(Vec3);
  mesh.positions = new Vec3[mesh.numVertices];
  mesh.normals = new Vec3[mesh.numVertices];
  mesh.colors = new Vec3[mesh.numVertices];
  for (Vec3* attr : {mesh.positions, mesh.normals, mesh.colors}) {
    if (!ReadFile(hFile, attr, attrSizeBytes, &bytesRead, NULL) ||
        bytesRead != attrSizeBytes) {
      fatal("Failed to read position data.");
    }
  }

  const DWORD indicesSizeBytes = mesh.numIndices * sizeof(unsigned int);
  mesh.indices = new unsigned int[mesh.numIndices];
  if (!ReadFile(hFile, mesh.indices, indicesSizeBytes, &bytesRead, NULL) ||
      bytesRead != indicesSizeBytes) {
    fatal("Failed to read position data.");
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

  char path[MAX_PATH];
  GetCurrentDirectoryA(MAX_PATH, path);
  strcat_s(path, "\\assets\\");
  strcat_s(path, "suzanne.mesh");
  Mesh mesh = readMeshFromFile(path);
  std::println("numVertices {}, numIndices {}", mesh.numVertices,
               mesh.numIndices);
  for (unsigned int vertIx = 0; vertIx < 3; vertIx++) {
    const Vec3& pos = mesh.positions[vertIx];
    const Vec3& norm = mesh.normals[vertIx];
    const Vec3& col = mesh.colors[vertIx];
    std::println(
        "vert[{}] ({:.3f}, {:.3f}, {:.3f}), ({:.3f}, {:.3f}, {:.3f}), ({:.3f}, "
        "{:.3f}, {:.3f})",
        vertIx, pos.x, pos.y, pos.z, norm.x, norm.y, norm.z, col.x, col.y,
        col.z);
  }
  for (unsigned int triIx = 0; triIx < 5; triIx++) {
    std::print("{} ", triIx);
    for (unsigned int c = 0; c < 3; c++) {
      std::print("{} ", mesh.indices[triIx * 3 + c]);
    }
    std::println();
  }

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
