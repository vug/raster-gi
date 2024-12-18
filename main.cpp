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

  for (uint32_t vertIx = 0; vertIx < mesh.numVertices; ++vertIx) {
    mesh.colors[vertIx] *=
        4;  // temporarily increase intensity here, should be done in Blender
  }

  return mesh;
}

int main() {
  loadWglCreateContextAttribsARB();
  HDC dev;
  const uint32_t winWidth = 1920;
  const uint32_t winHeight = 1080;
  createAndShowWindow("RasterGI", winWidth, winHeight, dev);
  setPixelFormatFancy(dev);
  createAndMakeOpenGlContext(dev);

  initGlFunctions();
  std::println("OpenGL version: {}", (char*)glGetString(GL_VERSION));

  const HMM_Vec3 kUp = HMM_V3(0, 0, 1);

  char path[MAX_PATH];
  GetCurrentDirectoryA(MAX_PATH, path);
  strcat_s(path, "\\assets\\");
  strcat_s(path, "trees.mesh");
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

  const GLint uTimeLoc = glGetUniformLocation(prog, "uTime");

  const GLint uWorldFromObjectLoc =
      glGetUniformLocation(prog, "uWorldFromObject");
  const GLint uViewFromWorldLoc = glGetUniformLocation(prog, "uViewFromWorld");
  const GLint uProjectionFromViewLoc =
      glGetUniformLocation(prog, "uProjectionFromView");

  const GLsizei viewportSide = 32;
  const GLsizei viewportArea = viewportSide * viewportSide;
  const GLsizei numViewports = 256;

  const GLsizei texWidth = viewportSide * numViewports;
  const GLsizei texHeight = viewportSide;
  // moving texture data out of stack because surpassed memory limit :-O
  HMM_Vec3* pixels = new HMM_Vec3[texWidth * texHeight];

  GLuint colorTexOffScreen{};
  glGenTextures(1, &colorTexOffScreen);
  glBindTexture(GL_TEXTURE_2D, colorTexOffScreen);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, texWidth, texHeight, 0, GL_RGB,
               GL_FLOAT, nullptr);

  GLuint depthTexOffScreen{};
  glGenTextures(1, &depthTexOffScreen);
  glBindTexture(GL_TEXTURE_2D, depthTexOffScreen);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, texWidth, texHeight, 0,
               GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

  GLuint fbOffScreen{};
  glGenFramebuffers(1, &fbOffScreen);
  glBindFramebuffer(GL_FRAMEBUFFER, fbOffScreen);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         colorTexOffScreen, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                         depthTexOffScreen, 0);
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

  glEnable(GL_SCISSOR_TEST);
  const float t0 = getTime();
  float tP = t0;
  while (!GetAsyncKeyState(VK_ESCAPE)) {
    const float t = getTime() - t0;
    const float dt = t - tP;
    tP = t;
    //std::println("dt {}, FPS {}", dt, 1.f / dt);
    glUniform1f(uTimeLoc, t);

    const HMM_Mat4 worldFromObject = HMM_M4D(1.f);
    glUniformMatrix4fv(uWorldFromObjectLoc, 1, GL_FALSE,
                       &worldFromObject.Elements[0][0]);

    const float highFOV = HMM_PI / 1.25;  //  HMM_PI - 0.05f; // ~179 deg
    const HMM_Mat4 projectionFromView =
        HMM_Perspective_RH_ZO(highFOV, 1.0f, 0.01f, 100.0f);
    glUniformMatrix4fv(uProjectionFromViewLoc, 1, GL_FALSE,
                       &projectionFromView.Elements[0][0]);

    // Loop over every vertex, render the scene from vertex position into normal
    // direction into a small texture take average pixel of the texture and
    // store it as the incoming radiance for that vertex
    glBindFramebuffer(GL_FRAMEBUFFER, fbOffScreen);
    glClearColor(0.f, 0.f, 0.f, 1.0f);
    // copy original light emissions from mesh data
    HMM_Vec3* accumulatedRadiances =
        new HMM_Vec3[mesh.numVertices];  // total lighting from all bounces
    HMM_Vec3* bounceRadiances =
        new HMM_Vec3[mesh.numVertices];  // lighting from current bounce
    // Custom lighting pattern
    //CopyMemory(accumulatedRadiances, mesh.colors, sizeof(HMM_Vec3) * mesh.numVertices);
    //CopyMemory(bounceRadiances, mesh.colors, sizeof(HMM_Vec3) * mesh.numVertices);
    for (uint32_t vertIx = 0; vertIx < mesh.numVertices; ++vertIx) {
      //const bool illumVert = static_cast<uint32_t>(t) * 100 < vertIx &&
      //                       vertIx < (static_cast<uint32_t>(t) + 1) * 100;
      const bool illumVert = HMM_MOD(static_cast<uint32_t>(2 * t * 100), mesh.numVertices) < vertIx &&
                             vertIx < HMM_MOD(static_cast<uint32_t>((2 * t + 1) * 100), mesh.numVertices);
      accumulatedRadiances[vertIx] = illumVert ? HMM_V3(HMM_SinF(t),HMM_CosF(t),1) : HMM_V3(0,0,0);
      bounceRadiances[vertIx] = accumulatedRadiances[vertIx];
    }

    const uint32_t numBounces = 3;
    for (uint32_t bounceNo = 0; bounceNo < numBounces; bounceNo++) {
      glBindBuffer(GL_ARRAY_BUFFER, vbColor);
      glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(HMM_Vec3) * mesh.numVertices,
                      bounceRadiances);

      uint32_t numDownloads =
          std::ceil(static_cast<float>(mesh.numVertices) / numViewports);
      for (uint32_t d = 0; d < numDownloads; ++d) {
        for (uint32_t v = 0; v < numViewports; ++v) {
          glViewport(v * viewportSide, 0, viewportSide, viewportSide);
          glScissor(v * viewportSide, 0, viewportSide, viewportSide);
          const uint32_t vertIx = d * numViewports + v;
          if (vertIx >= mesh.numVertices) {
            break;
          }
          const HMM_Vec3 camPos = mesh.positions[vertIx];
          const HMM_Vec3 camTarget = camPos + mesh.normals[vertIx];
          HMM_Mat4 viewFromWorld = HMM_LookAt_RH(camPos, camTarget, kUp);
          glUniformMatrix4fv(uViewFromWorldLoc, 1, GL_FALSE,
                             &viewFromWorld.Elements[0][0]);
          glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
          glDrawElements(GL_TRIANGLES, mesh.numIndices, GL_UNSIGNED_INT,
                         nullptr);
        }

        glReadPixels(0, 0, texWidth, texHeight, GL_RGB, GL_FLOAT, pixels);
        for (uint32_t v = 0; v < numViewports; ++v) {
          const uint32_t vertIx = d * numViewports + v;
          if (vertIx >= mesh.numVertices) {
            break;
          }
          HMM_Vec3 totalRadiance = HMM_V3(0, 0, 0);
          // const float viewportCenter = viewportSide * 0.5;
          for (uint32_t i = 0; i < viewportSide; ++i) {
            for (uint32_t j = v * viewportSide; j < (v + 1) * viewportSide;
                 ++j) {
              // non-physical weight to emphasize central pixels more than
              // peripheral pixels
              // const float weight = HMM_CosF((i - viewportCenter) /
              //                              viewportCenter * HMM_PI * 0.5f) *
              //                     HMM_CosF((j - viewportCenter) /
              //                              viewportCenter * HMM_PI * 0.5f) *
              //                     2.f;
              const uint32_t pIx = i * texWidth + j;
              const HMM_Vec3& px = pixels[pIx];
              totalRadiance += px;  // * weight;
            }
          }
          const HMM_Vec3 radiance = totalRadiance / viewportArea;
          accumulatedRadiances[vertIx] += radiance;
          bounceRadiances[vertIx] = radiance;
        }
      }
    }
    glBindBuffer(GL_ARRAY_BUFFER, vbColor);
    // TODO(vug): option to choose among accumulatedRadiances (result) and
    // bounceRadiances (bounce contribution)
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(HMM_Vec3) * mesh.numVertices,
                    accumulatedRadiances);

    // Render the world from camera POV
    const HMM_Mat4 worldFromObject2 = HMM_M4D(1.f);
    glUniformMatrix4fv(uWorldFromObjectLoc, 1, GL_FALSE,
                       &worldFromObject2.Elements[0][0]);
    // t = 19;
    HMM_Mat4 viewFromWorld2 = HMM_LookAt_RH(
        HMM_V3(20.f * HMM_CosF(t * 0.5f), 20.f * HMM_SinF(t * 0.5f), 10),
        HMM_V3(0, 0, 0), kUp);
    glUniformMatrix4fv(uViewFromWorldLoc, 1, GL_FALSE,
                       &viewFromWorld2.Elements[0][0]);

    const HMM_Mat4 projectionFromView2 = HMM_Perspective_RH_ZO(
        HMM_PI / 2, static_cast<float>(winWidth) / winHeight, 0.01f, 100.0f);
    glUniformMatrix4fv(uProjectionFromViewLoc, 1, GL_FALSE,
                       &projectionFromView2.Elements[0][0]);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, winWidth, winHeight);
    glScissor(0, 0, winWidth, winHeight);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClearColor(0.f, 0.f, 0.f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDrawElements(GL_TRIANGLES, mesh.numIndices, GL_UNSIGNED_INT, nullptr);

    SwapBuffers(dev);
  }

  // glDeleteBuffers(vbPosition);
}
