#pragma once
#ifndef __TESTAPP_DEFINES_H__
#define __TESTAPP_DEFINES_H__

#include <iostream>
#include <string>
#include <vector>
#include <exception>
#include <cstdarg>
#include <filesystem>
#include <map>
#include <vector>
#include <functional>
#include <math.h>
#include <memory>
// #include <format>

#include <GL/glew.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/ext.hpp>

#ifdef __linux__
// This gets the OS name
#include <sys/utsname.h>
// Sort of similar to windows GetLastError
#include <sys/errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/prctl.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#endif


namespace std {

std::string to_string(const char* __val);
std::string to_string(const GLubyte* __val);

}  // namespace std

namespace glm {
inline float clamp(float x, float _min, float _max) {
  return glm::max(glm::min(x, _max), _min);
}
inline double clamp(double x, double _min, double _max) {
  return glm::max(glm::min(x, _max), _min);
}
}  // namespace glm

namespace B26D {

#pragma region macros

#ifdef _WIN32
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT
#endif
#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950
#endif
#define OS_NOT_SUPPORTED #pragma message("OS not supported")

#define LogDebug(msg) \
  B26D::Log::dbg(std::string() + msg, __FILE__, __LINE__)
#define LogInfo(msg) \
  B26D::Log::inf(std::string() + msg, __FILE__, __LINE__)
#define LogWarn(msg) \
  B26D::Log::err(std::string() + msg, __FILE__, __LINE__)
#define LogError(msg) \
  B26D::Log::err(std::string() + msg, __FILE__, __LINE__)
#define msg(msg) LogInfo(msg)

#define Assert(__x)                                                       \
  do {                                                                    \
    if (!(__x)) {                                                           \
      std::string str = std::string("Runtime Assertion Failed: ") + #__x; \
      Log::print(str);                                                    \
      B26D::Gu::debugBreak();                                             \
      Raise(str);                                                         \
    }                                                                     \
  } while (0);

#define Raise(__str) throw B26D::Exception(__FILE__, __LINE__, (__str))

#define CheckErrorsRt() Gu::checkErrors(__FILE__, __LINE__)
#ifdef _DEBUG
#define CheckErrorsDbg() Gu::checkErrors(__FILE__, __LINE__)
#else
#define CheckErrorsDbg()
#endif

#pragma endregion
#pragma region typedef

typedef std::string string_t;
typedef std::filesystem::path path_t;
typedef glm::mat2 mat2;
typedef glm::mat3 mat3;
typedef glm::mat4 mat4;
typedef glm::quat quat;
typedef glm::vec2 vec2;
typedef glm::vec3 vec3;
typedef glm::vec4 vec4;
typedef glm::ivec2 ivec2;
typedef glm::ivec3 ivec3;
typedef glm::ivec4 ivec4;
typedef glm::uvec2 uvec2;
typedef glm::uvec3 uvec3;
typedef glm::uvec4 uvec4;

#pragma endregion
#pragma region forward decl

class Log;
class Window;
class Exception;
class Gu;
class Image;
class Shader;
class b2_objdata;
class b2_action;
class b2_frame;
class b2_mtex;
class BinaryFile;
class Bobj;
class Camera;
class Component;
class Texture;
class VertexArray;
class GpuBuffer;
class AppConfig;
class Scene;

struct GpuQuad;
struct GpuQuadVert;
struct GpuCamera;

#pragma endregion
#pragma region string extensions

std::string operator+(const std::string& str, const char& rhs);
std::string operator+(const std::string& str, const int8_t& rhs);
std::string operator+(const std::string& str, const int16_t& rhs);
std::string operator+(const std::string& str, const int32_t& rhs);
std::string operator+(const std::string& str, const int64_t& rhs);
std::string operator+(const std::string& str, const uint8_t& rhs);
std::string operator+(const std::string& str, const uint16_t& rhs);
std::string operator+(const std::string& str, const uint32_t& rhs);
std::string operator+(const std::string& str, const uint64_t& rhs);
std::string operator+(const std::string& str, const double& rhs);
std::string operator+(const std::string& str, const float& rhs);
std::string operator+(const std::string& str, const path_t& rhs);
std::string operator+(const std::string& str, const vec2& rhs);
std::string operator+(const std::string& str, const vec3& rhs);
std::string operator+(const std::string& str, const vec4& rhs);
std::string operator+(const std::string& str, const ivec2& rhs);
std::string operator+(const std::string& str, const ivec3& rhs);
std::string operator+(const std::string& str, const ivec4& rhs);
std::string operator+(const std::string& str, const uvec2& rhs);
std::string operator+(const std::string& str, const uvec3& rhs);
std::string operator+(const std::string& str, const uvec4& rhs);
std::string operator+(const std::string& str, const mat2& rhs);
std::string operator+(const std::string& str, const mat3& rhs);
std::string operator+(const std::string& str, const mat4& rhs);
std::string operator+(const std::string& str, const path_t& rhs);

#pragma endregion

}//ns 


#endif