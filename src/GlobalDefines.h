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
#include <unordered_set>
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
template <typename Tx, qualifier Qx, int Dx>
inline bool vec_gt(vec<Dx, Tx, Qx> const& x, vec<Dx, Tx, Qx> const& y) {
  for (length_t i = 0; i < x.length(); ++i)
    if (!(x[i] > y[i])) {
      return false;
    }
  return true;
}
template <typename Tx, qualifier Qx, int Dx>
inline bool vec_gte(vec<Dx, Tx, Qx> const& x, vec<Dx, Tx, Qx> const& y) {
  for (length_t i = 0; i < x.length(); ++i)
    if (!(x[i] >= y[i])) {
      return false;
    }
  return true;
}
template <typename Tx, qualifier Qx, int Dx>
inline bool vec_lt(vec<Dx, Tx, Qx> const& x, vec<Dx, Tx, Qx> const& y) {
  for (length_t i = 0; i < x.length(); ++i)
    if (!(x[i] < y[i])) {
      return false;
    }
  return true;
}
template <typename Tx, qualifier Qx, int Dx>
inline bool vec_lte(vec<Dx, Tx, Qx> const& x, vec<Dx, Tx, Qx> const& y) {
  for (length_t i = 0; i < x.length(); ++i)
    if (!(x[i] <= y[i])) {
      return false;
    }
  return true;
}

}  // namespace glm

namespace B26D {

#pragma region macros

#if defined(linux)
//__unix__
#define BR2_OS_LINUX
#define DLL_EXPORT
#elif defined(_WIN32)
// Note Win64 is defined WITH win32
#define BR2_OS_WINDOWS
#define DLL_EXPORT __declspec(dllexport)
// #define BRO_USE_DIRECTX
// We're not using winsock anymore due to SDL_Net
// #define BRO_USE_WINSOCK
#endif

#define OS_NOT_SUPPORTED #pragma message("OS not supported")
#define DOES_NOT_OVERRIDE

#define __inout_
#define __in_
#define __out_

//#define GPU_STRUCT struct alignas(16)
#define GPU_STRUCT struct
#define SHADER_SHARED
#define TO_STRING(__x) #__x
#define SHADER_SHARED_STR TO_STRING(SHADER_SHARED)

#define LogDebug(_msgx) B26D::Log::dbg(std::string() + _msgx, __FILE__, __LINE__)
#define LogInfo(_msgx) B26D::Log::inf(std::string() + _msgx, __FILE__, __LINE__)
#define LogWarn(_msgx) B26D::Log::warn(std::string() + _msgx, __FILE__, __LINE__)
#define LogError(_msgx) B26D::Log::err(std::string() + _msgx, __FILE__, __LINE__)

#define _LogXOnce(_msgx, _func)     \
  {                                 \
    static bool ___logged_ = false; \
    if (___logged_ == false) {      \
      ___logged_ = true;            \
      _func(_msgx);                 \
    }                               \
  }
#define LogErrorOnce(_msgx) _LogXOnce(_msgx, LogError);
#define LogWarnOnce(_msgx) _LogXOnce(_msgx, LogWarn);
#define LogDebugOnce(_msgx) _LogXOnce(_msgx, LogDebug);

#define _LogXCycle(_msgx, _func)                   \
  {                                                \
    int _ms = 1000;                                \
    static uint64_t _last = Gu::getMilliSeconds(); \
    if (Gu::getMilliSeconds() - _last >= _ms) {    \
      _last = Gu::getMilliSeconds();               \
      _func(_msgx);                                \
    }                                              \
  }
#define LogWarnCycle(_msgx) _LogXCycle(_msgx, LogWarn)
#define LogErrorCycle(_msgx) _LogXCycle(_msgx, LogError)
#define LogDebugCycle(_msgx) _LogXCycle(_msgx, LogDebug)

#define msg(msg) LogInfo(msg)

#define Raise(__str) throw B26D::Exception(__FILE__, __LINE__, (__str))
#define RaiseDebug(__str) \
  do {                    \
    LogDebug(__str);      \
    Gu::debugBreak();     \
  } while (0);

#define RaiseNotImplemented() Raise("Method/Function not implemented.")

#define Assert(__x, ...)                                                                                                                                                                  \
  do {                                                                                                                                                                                    \
    if (!(__x)) {                                                                                                                                                                         \
      std::string str = std::string("Runtime Assertion Failed: ") + std::string(#__x) + (std::string(__VA_ARGS__).length() == 0 ? "" : (std::string(" -> ") + std::string(__VA_ARGS__))); \
      Log::print(str);                                                                                                                                                                    \
      B26D::Gu::debugBreak();                                                                                                                                                             \
      Raise(str);                                                                                                                                                                         \
    }                                                                                                                                                                                     \
  } while (0);

#define CheckErrorsRt() Gu::checkErrors(__FILE__, __LINE__)
#ifdef _DEBUG
#define CheckErrorsDbg() Gu::checkErrors(__FILE__, __LINE__)
#else
#define CheckErrorsDbg()
#endif

#define BR2_FORCE_INLINE inline

#define FUZZY_ZERO_EPSILON 0.0000001

#pragma endregion
#pragma region typedef

typedef std::string str;
typedef std::string string_t;
typedef std::wstring wstring_t;
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
typedef glm::u8vec4 u8vec4;

template <typename Tx>
using uptr = std::unique_ptr<Tx>;
template <typename Tx>
using sptr = std::shared_ptr<Tx>;
template <typename Tx>
using wptr = std::weak_ptr<Tx>;

#pragma endregion
#pragma region enums

enum class ImageFormatType { RGBA8, RGB8, RGBA16, RGBA32, R16, R32, DEPTH16, DEPTH24, DEPTH32 };
enum class LineBreak { Unix, DOS };
enum class PrimType { Triangles, Points, Lines };

#pragma endregion
#pragma region forward decl

template <typename Tx, int Dx>
class box2x;

class plane;
class tri3;
class oobox3;

class Log;
class Window;
class Exception;

class BinaryFile;

class Gu;
class AppConfig;

class Viewport;
class Overlay;
class RenderView;
class Frustum;

class VertexArray;
class Gpu;
class GpuRenderState;
class GpuBuffer;
class Mesh;
class Material;

class PipelineStage;
class Renderer;
class Picker;

class Shader;
class Image;
class ImageFormat;
class Texture;
class TextureArray;

class Bobj;
class Camera;
class Component;
class VisibleStuff;
class World;

struct GpuQuad;
struct GpuQuadVert;
struct GpuCamera;

class b2_objdata;
class b2_action;
class b2_frame;
class b2_mtex;

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
#pragma region Resources

#define _RS_DEF(_id) static const char* _id() { return #_id; }
namespace res {
  namespace material {
    _RS_DEF(DebugDrawMaterial_Points)
    _RS_DEF(DebugDrawMaterial_Lines)
    _RS_DEF(DebugDrawMaterial_Tris)
  }
};

#pragma endregion

class MathUtils {
public:
  BR2_FORCE_INLINE static int32_t getNumberOfDigits(int32_t i) { return i > 0 ? (int)log10f((float)i) + 1 : 1; }
  BR2_FORCE_INLINE static uint32_t getNumberOfDigits(uint32_t i) { return i > 0 ? (int)log10((double)i) + 1 : 1; }
};

class OperatingSystem {
public:
  static inline string_t newline() {
    string_t ret = "";
#if defined(BR2_OS_WINDOWS)
    ret = "\r\n";
#elif defined(BR2_OS_LINUX)
    ret = "\n";
#else
    OS_METHOD_NOT_IMPLEMENTED
#endif
    return ret;
  }
};

template <typename Tx>
class GrowList {
private:
  std::vector<Tx> _items;
  int _count = 0;
  int _maxcount;

public:
  int maxCount() { return _maxcount; }
  int size() { return _count; }
  Tx* data() { return _items.data(); }

  GrowList(int max = 9999999) { _maxcount = max; }
  void reset() { _count = 0; }
  Tx& operator[](size_t idx) { return _items[idx]; }
  void push_back(Tx item);
};
struct GpuDebugVert {
  vec3 _v;
  vec4 _c;
  vec2 _size;
  vec3 _outl;
  GpuDebugVert() {}
  GpuDebugVert(const vec3& v, const vec4& c, const vec2& siz, const vec3& outl) {
    _v = v;
    _c = c;
    _size = siz;
    _outl = outl;
  }
};

}  // namespace B26D

#endif