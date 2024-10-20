/*
 * Original idea: https://iquilezles.org/articles/simplegi/
 * TODO(vug): lookAt function
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
  // TODO(vug): remove debug prints, it works
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

  auto createVertexBuffer = [](GLuint& id, GLsizeiptr size, const void* data) {
    glCreateBuffers(1, &id);
    glBindBuffer(GL_ARRAY_BUFFER, id);
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
  };
  const GLsizeiptr bufferSizeBytes = mesh.numVertices * sizeof(Vec3);
  GLuint vbPosition;
  createVertexBuffer(vbPosition, bufferSizeBytes, mesh.positions);
  GLuint vbNormal;
  createVertexBuffer(vbNormal, bufferSizeBytes, mesh.normals);
  GLuint vbColor;
  createVertexBuffer(vbColor, bufferSizeBytes, mesh.colors);
  GLuint ib;
  glCreateBuffers(1, &ib);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.numIndices * sizeof(unsigned int),
               mesh.indices, GL_STATIC_DRAW);


  const char* vertSrc = R"glsl(
#version 460

layout (location = 0) in vec3 aPosition;  // Position attribute at location 0
layout (location = 1) in vec3 aNormal;    // Normal attribute at location 1
layout (location = 2) in vec3 aColor;     // Color attribute at location 2

uniform mat4 uMVPMatrix = mat4(1);  // Model-View-Projection matrix

out vec3 vNormal;  // Pass normal to fragment shader
out vec3 vColor;   // Pass color to fragment shader

void main() {
    // Transform the vertex position by the MVP matrix
    gl_Position = uMVPMatrix * vec4(aPosition, 1.0);
    
    // Pass normal and color to the fragment shader
    vNormal = aNormal;
    vColor = aColor;
}
)glsl";

  const char* fragSrc = R"glsl(
#version 460
in vec3 vNormal;  // Interpolated normal from vertex shader
in vec3 vColor;   // Interpolated color from vertex shader

out vec4 fragColor;  // Output color

void main() {
    // For now, we'll just output the color passed in
    fragColor = vec4(vColor, 1.0);
}
)glsl";
  GLuint prog = compileShader(vertSrc, fragSrc);
  glUseProgram(prog);

  uint32_t vao;
  glCreateVertexArrays(1, &vao);


  float t0 = getTime();
  while (!GetAsyncKeyState(VK_ESCAPE)) {
    const float t = getTime() - t0;
    const float dt = t - t0;

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClearColor(0.f, 0.f, 0.f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbPosition);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbNormal);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);  
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, vbColor);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(2); // color
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib);

    glDrawElements(GL_TRIANGLES, mesh.numIndices, GL_UNSIGNED_INT, nullptr);

    SwapBuffers(dev);
  }

  // glDeleteBuffers(vbPosition);
}
