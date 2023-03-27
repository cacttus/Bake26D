
#pragma once
#ifndef __908357602394876023948723423TESTAPP_H__
#define __908357602394876023948723423TESTAPP_H__

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

// #include <GL/gl.h>
#include <GL/glew.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/ext.hpp>
// #include <glm/glm.hpp>


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

#ifdef _WIN32
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT
#endif
#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950
#endif

namespace std {

std::string to_string(const char* __val);
std::string to_string(const GLubyte* __val);

}  // namespace std

#define LogDebug(msg) \
  TestApp::Log::dbg(std::string() + msg)
#define LogInfo(msg) \
  TestApp::Log::inf(std::string() + msg)
#define LogWarn(msg) \
  TestApp::Log::err(std::string() + msg)
#define LogError(msg) \
  TestApp::Log::err(std::string() + msg)
#define msg(msg) LogInfo(msg)

#define Assert(__x)                                        \
  do {                                                     \
    if (!__x) {                                            \
      TestApp::Gu::debugBreak();                           \
      throw TestApp::Exception("Assertion Failed: " #__x); \
    }                                                      \
  } while (0);


namespace TestApp{

typedef std::string string_t ; 

class Log;
class MainWindow;
class Exception;
class Gu;
class Image;
struct RenderQuad;
struct BinaryFile;
struct QuadVert;
class b2_obj;
class b2_action;
class b2_frame;
class b2_tex;

struct QuadVert {
  glm::vec3 _v;
  float _pd;
  glm::vec2 _x;
  glm::vec4 _c;
};
struct RenderQuad {
  QuadVert _verts[4];
  // TODO: model matrix
  void translate(glm::vec3 off) {
    for (auto& v : _verts) {
      v._v += off;
    }
  }
  void scale(glm::vec3 scl) {
    auto c = (_verts[2]._v - _verts[0]._v) * 0.5f;
    for (auto& v : _verts) {
      v._v = (v._v - c) * scl + c;
    }
  }
};
class Exception {
  std::string _what = "";

public:
  Exception(std::string what) { _what = what; }
  std::string toString() { return _what; }
};
class Log {
public:
  static std::string CC_BLACK;
  static std::string CC_RED;
  static std::string CC_GREEN;
  static std::string CC_YELLOW;
  static std::string CC_BLUE;
  static std::string CC_PINK;
  static std::string CC_CYAN;
  static std::string CC_WHITE;
  static std::string CC_NORMAL;

  static std::string cc_reset() { return "\033[0m"; }
  static std::string cc_color(std::string color, bool bold) { return std::string()+"\033[" + (bold ? "1;" : "") + color + "m" ; }
  static void err(std::string s) { _output(CC_RED, false, "E", s); }
  static void dbg(std::string s) { _output(CC_CYAN, false, "D", s); }
  static void inf(std::string s) { _output(CC_WHITE, false, "I", s); }
  static void exception(Exception ex) {
    err(ex.toString());
    auto tbout = cc_color(CC_WHITE, true) + std::string(ex.toString()) + cc_reset();
    _print(tbout);
  }
  static void _output(std::string color, bool bold, std::string type, std::string s) {
    s = _header(color, bold, type) + s + cc_reset();
    _print(s);
  }
  static void _print(std::string s) { std::cout << s << std::endl; }
  static std::string _header(std::string color, bool bold, std::string type) {
    // secs = time(NULL) - Log::_start_time
    // dt = str(datetime.timedelta(seconds=secs))
    return cc_color(color, bold) + "[" + type + "] ";
  }
};

class Gu {
public:
  static std::filesystem::path relpath(std::string rel);
  static bool exists(std::filesystem::path path);
  static std::filesystem::path app_path;
  static auto createShader(std::string vert_src, std::string geom_src, std::string frag_src);
  static auto compileShader(GLenum type, std::string src);
  static void checkErrors();
  static auto createTexture(std::filesystem::path fileloc);
  static std::string printDebugMessages();
  static std::string getShaderInfoLog(GLint prog);
  static std::string getProgramInfoLog(GLint prog);
  static void rtrim(std::string& s);
  static GLint glGetInteger(GLenum arg);
  static float shitrand(float a, float b) { return (((float)a) + (((float)rand() / (float)(RAND_MAX))) * (((float)b) - ((float)a))); }
  static void debugBreak();
  static uint64_t getMicroSeconds();
  static uint64_t getMilliSeconds();
  static std::string executeReadOutput(const std::string& cmd);
};
class Image {
private:
  int _width = 0;
  int _height = 0;
  int _bpp = 0;
  std::unique_ptr<char[]> _data;

public:
  int width() { return _width; }
  int height() { return _height; }
  int bpp() { return _bpp; }
  char* data() { return _data.get(); }

  Image(int w, int h, int bpp, const char* data) {
    _width = w;
    _height = h;
    _bpp = bpp;
    size_t len = w * h * bpp;
    _data = std::make_unique<char[]>(len);
    memcpy(_data.get(), data, len);
  }
  virtual ~Image() {
  }
  static std::unique_ptr<Image> from_file(std::string path);
};








}//ns testapp


#endif