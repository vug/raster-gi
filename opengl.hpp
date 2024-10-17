#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

void fatal(const char *msg);
void createAndShowWindow(const char *name, int with, int height,
                         HDC &deviceContextHandle);
void setPixelFormatFancy(HDC deviceContextHandle);
void createAndMakeOpenGlContext(HDC deviceContextHandle);

typedef char GLchar;
typedef unsigned char GLubyte;
typedef int GLint;
typedef int GLsizei;
typedef unsigned int GLbitfield;
typedef unsigned int GLuint;
typedef unsigned int GLenum;
typedef float GLfloat;
#define GL_TRUE 1
#define GL_FALSE 0
#define GL_VERSION 0x1F02
// WINGDIAPI const GLubyte *APIENTRY glGetString(GLenum name);
#define GL_COLOR_BUFFER_BIT 0x00004000
#define GL_VERTEX_SHADER 0x8B31
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_COMPILE_STATUS 0x8B81
#define GL_LINK_STATUS 0x8B82
#define GL_TRIANGLES 0x0004

// Creates symbol for function pointer type of given method name
#define FnPtrT(method) FnPtr_##method##_Proc

using FnPtrT(wglCreateContextAttribsARB) =
    HGLRC(APIENTRY *)(HDC hDC, HGLRC hShareContext, const int *attribList);
extern FnPtrT(wglCreateContextAttribsARB) wglCreateContextAttribsARB;

// https://www.khronos.org/registry/OpenGL/extensions/ARB/WGL_ARB_create_context.txt
#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092
#define WGL_CONTEXT_PROFILE_MASK_ARB 0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001

using FnPtrT(wglChoosePixelFormatARB) = bool(APIENTRY *)(
    HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList,
    UINT nMaxFormats, int *piFormats, UINT *nNumFormats);
extern FnPtrT(wglChoosePixelFormatARB) wglChoosePixelFormatARB;

// https://www.khronos.org/registry/OpenGL/extensions/ARB/WGL_ARB_pixel_format.txt
#define WGL_DRAW_TO_WINDOW_ARB 0x2001
#define WGL_ACCELERATION_ARB 0x2003
#define WGL_SUPPORT_OPENGL_ARB 0x2010
#define WGL_DOUBLE_BUFFER_ARB 0x2011
#define WGL_PIXEL_TYPE_ARB 0x2013
#define WGL_COLOR_BITS_ARB 0x2014
#define WGL_DEPTH_BITS_ARB 0x2022
#define WGL_STENCIL_BITS_ARB 0x2023
#define WGL_FULL_ACCELERATION_ARB 0x2027
#define WGL_TYPE_RGBA_ARB 0x202B

void loadWglCreateContextAttribsARB();

// from: https://www.khronos.org/opengl/wiki/Load_OpenGL_Functions
// Functions belonging to OpenGL v1.1 or less cannot be loaded via
// wglGetProcAddress we'll get them directly from opengl32.dll that comes with
// Windows
void *GetAnyGLFuncAddress(const char *name);

// APIENTRY is WINAPI which is __stdcall
// Assigns a function pointer type to the symbol for this method
#define DECLARE_FUNC_PTR_TYPE(method, returnType, ...)        \
  using FnPtrT(method) = returnType(APIENTRY *)(__VA_ARGS__); \
  extern FnPtrT(method) method;

DECLARE_FUNC_PTR_TYPE(glClearColor, void, GLfloat red, GLfloat green,
                      GLfloat blue, GLfloat alpha);
DECLARE_FUNC_PTR_TYPE(glClear, void, GLbitfield mask);
DECLARE_FUNC_PTR_TYPE(glGetString, const GLubyte *, GLenum name);
DECLARE_FUNC_PTR_TYPE(glCreateShader, GLuint, GLenum type);
DECLARE_FUNC_PTR_TYPE(glShaderSource, void, GLuint shader, GLsizei count,
                      const GLchar *const *string, const GLint *length);
DECLARE_FUNC_PTR_TYPE(glCompileShader, void, GLuint shader);
DECLARE_FUNC_PTR_TYPE(glGetShaderiv, void, GLuint shader, GLenum pname,
                      GLint *params);
DECLARE_FUNC_PTR_TYPE(glCreateProgram, GLuint);
DECLARE_FUNC_PTR_TYPE(glAttachShader, void, GLuint program, GLuint shader);
DECLARE_FUNC_PTR_TYPE(glLinkProgram, void, GLuint program);
DECLARE_FUNC_PTR_TYPE(glGetProgramiv, void, GLuint program, GLenum pname,
                      GLint *params);
DECLARE_FUNC_PTR_TYPE(glGetShaderInfoLog, void, GLuint shader, GLsizei bufSize,
                      GLsizei *length, GLchar *infoLog);
DECLARE_FUNC_PTR_TYPE(glGetProgramInfoLog, void, GLuint shader, GLsizei bufSize,
                      GLsizei *length, GLchar *infoLog);
DECLARE_FUNC_PTR_TYPE(glGenVertexArrays, void, GLsizei n, GLuint *arrays);
DECLARE_FUNC_PTR_TYPE(glUseProgram, void, GLuint program);
DECLARE_FUNC_PTR_TYPE(glBindVertexArray, void, GLuint array);
DECLARE_FUNC_PTR_TYPE(glDrawArrays, void, GLenum mode, GLint first,
                      GLsizei count);

void initGlFunctions();

GLuint compileShader(const char *vertexSrc, const char *fragSrc);