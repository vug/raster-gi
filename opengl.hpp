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
typedef signed long long int khronos_ssize_t; // _WIN64
typedef khronos_ssize_t GLsizeiptr;
typedef signed long long int khronos_intptr_t; // _WIN64
typedef khronos_intptr_t GLintptr;
typedef unsigned char GLboolean;
#define GL_TRUE 1
#define GL_FALSE 0
#define GL_VERSION 0x1F02
// WINGDIAPI const GLubyte *APIENTRY glGetString(GLenum name);
#define GL_COLOR_BUFFER_BIT 0x00004000
#define GL_DEPTH_BUFFER_BIT 0x00000100
#define GL_VERTEX_SHADER 0x8B31
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_COMPILE_STATUS 0x8B81
#define GL_LINK_STATUS 0x8B82
#define GL_TRIANGLES 0x0004
#define GL_ARRAY_BUFFER 0x8892
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#define GL_STATIC_DRAW 0x88E4
#define GL_FLOAT 0x1406
#define GL_UNSIGNED_INT 0x1405
#define GL_DEPTH_TEST 0x0B71
#define GL_LESS 0x0201
#define GL_SHADER_STORAGE_BUFFER 0x90D2
#define GL_TEXTURE_2D 0x0DE1
#define GL_RGB 0x1907
#define GL_RGB32F 0x8815
#define GL_FRAMEBUFFER 0x8D40
#define GL_COLOR_ATTACHMENT0 0x8CE0
#define GL_FRAMEBUFFER_COMPLETE 0x8CD5
#define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT 0x8CD6 //
#define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT 0x8CD7
#define GL_CULL_FACE 0x0B44

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
DECLARE_FUNC_PTR_TYPE(glCreateBuffers, void, GLsizei n, GLuint *buffers); 
DECLARE_FUNC_PTR_TYPE(glDeleteBuffers, void, GLsizei n, const GLuint *buffers);
DECLARE_FUNC_PTR_TYPE(glBindBuffer, void, GLenum target, GLuint buffer);
DECLARE_FUNC_PTR_TYPE(glBufferData, void, GLenum target, GLsizeiptr size, const void *data, GLenum usage);
DECLARE_FUNC_PTR_TYPE(glCreateVertexArrays, void, GLsizei n, GLuint *arrays); 
DECLARE_FUNC_PTR_TYPE(glDeleteVertexArrays, void, GLsizei n, const GLuint *arrays);
DECLARE_FUNC_PTR_TYPE(glEnableVertexAttribArray, void, GLuint index);
DECLARE_FUNC_PTR_TYPE(glVertexAttribPointer, void, GLuint index, GLint size,
                      GLenum type, GLboolean normalized, GLsizei stride,
                      const void *pointer); 
DECLARE_FUNC_PTR_TYPE(glDrawElements, void, GLenum mode, GLsizei count,
                      GLenum type, const void *indices);
DECLARE_FUNC_PTR_TYPE(glEnable, void, GLenum cap);
DECLARE_FUNC_PTR_TYPE(glDepthFunc, void, GLenum func);
DECLARE_FUNC_PTR_TYPE(glGetUniformLocation, GLint, GLuint program, const GLchar* name);
DECLARE_FUNC_PTR_TYPE(glUniform1f, void, GLint location, GLfloat v0);
DECLARE_FUNC_PTR_TYPE(glUniformMatrix4fv, void, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
DECLARE_FUNC_PTR_TYPE(glBindBufferBase, void, GLenum target, GLuint index, GLuint buffer);
DECLARE_FUNC_PTR_TYPE(glGenTextures, void, GLsizei n, GLuint* textures);
DECLARE_FUNC_PTR_TYPE(glBindTexture, void, GLenum target, GLuint texture);
DECLARE_FUNC_PTR_TYPE(glTexImage2D, void, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels);
DECLARE_FUNC_PTR_TYPE(glGenFramebuffers, void, GLsizei n, GLuint* framebuffers);
DECLARE_FUNC_PTR_TYPE(glBindFramebuffer, void, GLenum target, GLuint framebuffer);
DECLARE_FUNC_PTR_TYPE(glFramebufferTexture2D, void, GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
DECLARE_FUNC_PTR_TYPE(glViewport, void, GLint x, GLint y, GLsizei width, GLsizei height);
DECLARE_FUNC_PTR_TYPE(glReadPixels, void, GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void* pixels);
DECLARE_FUNC_PTR_TYPE(glCheckFramebufferStatus, GLenum, GLenum target);
DECLARE_FUNC_PTR_TYPE(glBufferSubData, void, GLenum target, GLintptr offset,
                      GLsizeiptr size, const void *data);

//DECLARE_FUNC_PTR_TYPE(glFuncName, void, GLint foo);

void initGlFunctions();

GLuint compileShader(const char *vertexSrc, const char *fragSrc);