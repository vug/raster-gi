// Original idea: https://iquilezles.org/articles/simplegi/

#include "opengl.hpp"
// #include <gl/GL.h>
// #include "math.hpp"
#include <vendor/HandmadeMath.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <print>

static inline float getTime() {
  static LARGE_INTEGER freq = []() {
    LARGE_INTEGER f;
    QueryPerformanceFrequency(&f);
    return f;
  }();
  static LARGE_INTEGER startTicks = []() {
    LARGE_INTEGER t0;
    QueryPerformanceCounter(&t0);
    return t0;
  }();
  LARGE_INTEGER ticks;
  QueryPerformanceCounter(&ticks);
  return (ticks.QuadPart - startTicks.QuadPart) /
         static_cast<float>(freq.QuadPart);
}

struct Mesh {
  unsigned int numVertices{};
  unsigned int numIndices{};
  HMM_Vec3* positions{};
  HMM_Vec3* normals{};
  HMM_Vec3* colors{};
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

  const DWORD attrSizeBytes = mesh.numVertices * sizeof(HMM_Vec3);
  mesh.positions = new HMM_Vec3[mesh.numVertices];
  mesh.normals = new HMM_Vec3[mesh.numVertices];
  mesh.colors = new HMM_Vec3[mesh.numVertices];
  for (HMM_Vec3* attr : {mesh.positions, mesh.normals, mesh.colors}) {
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
  const uint32_t winWidth = 1024;
  const uint32_t winHeight = 1024;
  createAndShowWindow("RasterGI", winWidth, winHeight, dev);
  setPixelFormatFancy(dev);
  createAndMakeOpenGlContext(dev);

  initGlFunctions();
  std::println("OpenGL version: {}", (char*)glGetString(GL_VERSION));

  const HMM_Vec3 kUp = HMM_V3(0, 0, 1);

  char path[MAX_PATH];
  GetCurrentDirectoryA(MAX_PATH, path);
  strcat_s(path, "\\assets\\");
  strcat_s(path, "spiky.mesh");
  Mesh mesh = readMeshFromFile(path);
  std::println("numVertices {}, numIndices {}", mesh.numVertices,
               mesh.numIndices);
  for (unsigned int vertIx = 0; vertIx < 3; vertIx++) {
    const HMM_Vec3& pos = mesh.positions[vertIx];
    const HMM_Vec3& norm = mesh.normals[vertIx];
    const HMM_Vec3& col = mesh.colors[vertIx];
  }

  auto createVertexBuffer = [](GLuint& id, GLsizeiptr size, const void* data) {
    glCreateBuffers(1, &id);
    glBindBuffer(GL_ARRAY_BUFFER, id);
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
  };
  const GLsizeiptr bufferSizeBytes = mesh.numVertices * sizeof(HMM_Vec3);
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

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aColor;

layout(std430, binding = 0) buffer CamPositions {
    vec3 camPositions[3];
};

uniform float uTime = 0.0f;
uniform mat4 uWorldFromObject = mat4(1);
uniform mat4 uViewFromWorld = mat4(1);
uniform mat4 uProjectionFromView = mat4(1);

out vec3 vWorldPos;
out vec3 vNormal;
out vec3 vColor;

void main() {
    const mat4 MVP = uProjectionFromView * uViewFromWorld * uWorldFromObject;
    gl_Position = MVP * vec4(aPosition, 1.0);
    
    vWorldPos = vec3(uWorldFromObject * vec4(aPosition, 1));
    vNormal = aNormal;
    vColor = aColor;
}
)glsl";

  const char* fragSrc = R"glsl(
#version 460
in vec3 vNormal;
in vec3 vColor;
in vec3 vWorldPos;

uniform float uTime = 0.0f;

out vec4 fragColor; 

void main() {
    fragColor = vec4(vColor, 1.0);
}
)glsl";
  const GLuint prog = compileShader(vertSrc, fragSrc);
  glUseProgram(prog);

  uint32_t vao;
  glCreateVertexArrays(1, &vao);

  const unsigned int numMatrices = 3;
  HMM_Vec3 camPositions[3] = {HMM_Vec3{2, 2, 2}, HMM_Vec3{2, -2, 2},
                              HMM_Vec3{-2, 0, -2}};
  GLuint sb{};
  glCreateBuffers(1, &sb);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, sb);
  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(HMM_Vec3) * 3, camPositions,
               GL_STATIC_DRAW);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, sb);

  const GLint uTimeLoc = glGetUniformLocation(prog, "uTime");
  // const GLint uMVPMatrixLoc = glGetUniformLocation(prog, "uMVPMatrix");
  // const float uMVPMatrix[4][4] = { {1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0,0,1}
  // }; glUniformMatrix4fv(uMVPMatrixLoc, 1, GL_FALSE, &uMVPMatrix[0][0]);
  const GLint uWorldFromObjectLoc =
      glGetUniformLocation(prog, "uWorldFromObject");
  const GLint uViewFromWorldLoc = glGetUniformLocation(prog, "uViewFromWorld");
  const GLint uProjectionFromViewLoc =
      glGetUniformLocation(prog, "uProjectionFromView");

  const GLsizei viewportSize = 32;
  const GLsizei numViewports = 1;

  GLuint texOffScreen{};
  glGenTextures(1, &texOffScreen);
  glBindTexture(GL_TEXTURE_2D, texOffScreen);
  const GLsizei texWidth = viewportSize * numViewports;
  const GLsizei texHeight = viewportSize;
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, texWidth, texHeight, 0, GL_RGB,
               GL_FLOAT, nullptr);
  std::println("texOffscreen: {}", texOffScreen);

  GLuint fbOffScreen{};
  glGenFramebuffers(1, &fbOffScreen);
  glBindFramebuffer(GL_FRAMEBUFFER, fbOffScreen);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         texOffScreen, 0);
  const GLenum fbOffScreenStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (fbOffScreenStatus != GL_FRAMEBUFFER_COMPLETE) {
    if (fbOffScreenStatus == GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT)
      fatal("Framebuffer not complete due to incomplete attachment.");
    if (fbOffScreenStatus == GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT)
      fatal("Framebuffer not complete due to missing attachment.");
    fatal("Failed to complete offscreen framebuffer");
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  // glEnable(GL_CULL_FACE);

  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbPosition);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, vbNormal);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, vbColor);
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
  glEnableVertexAttribArray(2);  // color
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib);

  const float t0 = getTime();
  float tP = t0;
  while (!GetAsyncKeyState(VK_ESCAPE)) {
    const float t = getTime() - t0;
    const float dt = t - tP;
    tP = t;
    glUniform1f(uTimeLoc, t);

    const HMM_Mat4 worldFromObject = HMM_M4D(1.f);
    glUniformMatrix4fv(uWorldFromObjectLoc, 1, GL_FALSE,
                       &worldFromObject.Elements[0][0]);

    const HMM_Mat4 projectionFromView =
        HMM_Perspective_RH_ZO(HMM_PI / 4.f, 1.0f, 0.01f, 100.0f);
    glUniformMatrix4fv(uProjectionFromViewLoc, 1, GL_FALSE,
                       &projectionFromView.Elements[0][0]);

    // Loop over every vertex, render the scene from vertex position into normal direction into a small texture
    // take average pixel of the texture and store it as the incoming radiance for that vertex
    glBindFramebuffer(GL_FRAMEBUFFER, fbOffScreen);
    glViewport(0, 0, viewportSize, viewportSize);
    glClearColor(0.f, 0.f, 0.f, 1.0f);
    // copy initial light emissions
    HMM_Vec3* radiances = new HMM_Vec3[mesh.numVertices];
    CopyMemory(radiances, mesh.colors, sizeof(HMM_Vec3) * mesh.numVertices);

    for (uint32_t vertIx = 0; vertIx < mesh.numVertices; ++vertIx) {
      const HMM_Vec3 camPos = mesh.positions[vertIx];
      const HMM_Vec3 camTarget = camPos + mesh.normals[vertIx];
      HMM_Mat4 viewFromWorld = HMM_LookAt_RH(camPos, camTarget, kUp);
      glUniformMatrix4fv(uViewFromWorldLoc, 1, GL_FALSE,
                         &viewFromWorld.Elements[0][0]);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      glDrawElements(GL_TRIANGLES, mesh.numIndices, GL_UNSIGNED_INT, nullptr);

      HMM_Vec3 pixels[viewportSize * viewportSize];
      glReadPixels(0, 0, viewportSize, viewportSize, GL_RGB, GL_FLOAT, pixels);
      HMM_Vec3 totalRadiance = HMM_V3(0, 0, 0);
      for (uint32_t i = 0; i < viewportSize; ++i) {
        for (uint32_t j = 0; j < viewportSize; ++j) {
          const uint32_t pIx = i * viewportSize + j;
          const HMM_Vec3& px = pixels[pIx];
          totalRadiance += px;
        }
      }
      const HMM_Vec3 radiance = totalRadiance / (viewportSize * viewportSize);
      // adding direct lighting to emissive values (will loop later for indirect lighting)
      radiances[vertIx] += radiance;
    }
    glBindBuffer(GL_ARRAY_BUFFER, vbColor);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(HMM_Vec3) * mesh.numVertices, radiances);

    // Render the world from camera POV
    const HMM_Mat4 worldFromObject2 = HMM_M4D(1.f);
    glUniformMatrix4fv(uWorldFromObjectLoc, 1, GL_FALSE,
                       &worldFromObject2.Elements[0][0]);

    HMM_Mat4 viewFromWorld2 = HMM_LookAt_RH(
        HMM_V3(10 * HMM_CosF(t), 10 * HMM_SinF(t), 1), HMM_V3(0, 0, 0), kUp);
    glUniformMatrix4fv(uViewFromWorldLoc, 1, GL_FALSE,
                       &viewFromWorld2.Elements[0][0]);

    const HMM_Mat4 projectionFromView2 =
        HMM_Perspective_RH_ZO(HMM_PI / 4, 1.0f, 0.01f, 100.0f);
    glUniformMatrix4fv(uProjectionFromViewLoc, 1, GL_FALSE,
                       &projectionFromView2.Elements[0][0]);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, winWidth, winHeight);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClearColor(0.f, 0.f, 0.f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDrawElements(GL_TRIANGLES, mesh.numIndices, GL_UNSIGNED_INT, nullptr);

    SwapBuffers(dev);
  }

  // glDeleteBuffers(vbPosition);
}
