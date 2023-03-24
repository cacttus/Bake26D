#include <iostream>
#include <string>
#include <vector>
#include <exception>
#include <cstdarg>
#include <filesystem>
#include <map>
#include <vector>

// #include <GL/gl.h>
#include <GL/glew.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/ext.hpp>
// #include <glm/glm.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace std {

std::string to_string(const char* __val) { return std::string(__val); }
std::string to_string(const GLubyte* __val) { return std::string((const char*)__val); }

}  // namespace std

namespace TestApp {

// utils
#define Assert(__x)                               \
  {                                               \
    if (!__x) {                                   \
      Gu::debugBreak();                           \
      throw Exception("Assertion Failed: " #__x); \
    }                                             \
  }

// fwd
class Log;
class MainWindow;
class Exception;
class Gu;
class Image;
class RenderQuad;

// clss dcl
class Gu {
public:
  static std::filesystem::path app_path;
  static auto createShader(std::string vert_src, std::string geom_src, std::string frag_src);
  static auto compileShader(GLenum type, std::string src);
  static void checkErrors();
  static void debugBreak();
  static auto createTexture(std::string fileloc);
  static std::string printDebugMessages();
  static std::string getShaderInfoLog(GLint prog);
  static std::string getProgramInfoLog(GLint prog);
  static void rtrim(std::string& s);
  static GLint glGetInteger(GLenum arg);
  static float shitrand(float a, float b) { return (((float)a) + (((float)random() / (float)(RAND_MAX))) * (((float)b) - ((float)a))); }
};
enum class KeyState {
  Up,
  Press,
  Down,
  Release,
};
struct KeyEvent {
  int _key;
  bool _press;
};
class MainWindow {
private:
  std::map<int, KeyState> _keys;
  std::vector<KeyEvent> _key_events;
  int _lastx = 0, _lasty = 0, _lastw = 0, _lasth = 0;
  bool _fullscreen = false;
  int _width = 0;
  int _height = 0;
  GLFWwindow* _window = nullptr;
  bool _running = false;
  glm::mat4 _proj;
  glm::mat4 _view;
  glm::mat4 _model;
  std::vector<RenderQuad> _quads;
  glm::vec3 _pos = glm::vec3(0, 3, -10);
  glm::vec3 _heading = glm::vec3(0, 0, -1);
  glm::vec3 _up = glm::vec3(0, 1, 0);
  float _speed = 10;     // 1;
  float _rspeed = 0.5f;  // 0.5f;
  float _delta = 0;

  GLuint _vao = 0;
  GLuint _vbo = 0;
  GLuint _ibo = 0;
  GLuint _shader = 0;
  GLuint _fbo = 0;
  GLuint _test_tex = 0;
  GLuint _color= 0;
  GLuint _normal_depth= 0;

  bool pressOrDown(int key);
  void do_input();
  void toggleFullscreen() {
    bool fs = false, dec = false;
    int x = 0, y = 0, w = 0, h = 0;
    if (_fullscreen) {
      dec = GLFW_TRUE;
      fs = false;
      x = _lastx;
      y = _lasty;
      w = _lastw;
      h = _lasth;
    }
    else {
      auto m = glfwGetWindowMonitor(_window);
      if (!m) {
        m = glfwGetPrimaryMonitor();
      }
      if (m) {
        glfwGetWindowPos(_window, &_lastx, &_lasty);
        auto vm = glfwGetVideoMode(m);
        if (vm) {
          x = 0;
          y = 0;
          w = vm->width;
          h = vm->height;
          dec = GLFW_FALSE;
          fs = true;
        }
      }
    }
    if (w == 0) {
      w = 1;
    }
    if (h == 0) {
      h = 1;
    }
    glfwSetWindowAttrib(_window, GLFW_DECORATED, dec);
    glfwSetWindowSize(_window, w, h);
    glfwSetWindowPos(_window, x, y);
    _fullscreen = fs;
  }
  void exitApp() {
    _running = false;
  }

  static std::map<GLFWwindow*, MainWindow*> g_windows;
  static MainWindow* getWindow(GLFWwindow* win);

public:
  MainWindow(int, int, std::string);
  virtual ~MainWindow();
  void initFramebuffer();
  void render();
  void on_resize(int w, int h);
  void makeProgram();
  void createRenderEngine();
  void quit();
  void run();
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
  static std::string cc_color(std::string color, bool bold) { return color + ";1" + (bold ? "m" : ""); }
  static void err(std::string s) { _output(CC_RED, false, "E", s); }
  static void dbg(std::string s) { _output(CC_CYAN, false, "D", s); }
  static void msg(std::string s) { _output(CC_WHITE, false, "I", s); }
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
    return std::string("[") + cc_color(color, bold) + "][" + type + "]";
  }
};
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

// static data
std::string Log::CC_BLACK = "\033[30";
std::string Log::CC_RED = "\033[31";
std::string Log::CC_GREEN = "\033[32";
std::string Log::CC_YELLOW = "\033[33";
std::string Log::CC_BLUE = "\033[34";
std::string Log::CC_PINK = "\033[35";
std::string Log::CC_CYAN = "\033[36";
std::string Log::CC_WHITE = "\033[37";
std::string Log::CC_NORMAL = "\033[0;39";
std::map<GLFWwindow*, MainWindow*> MainWindow::g_windows;
std::filesystem::path Gu::app_path = "";

#pragma region GU
void Gu::rtrim(std::string& s) {
  int len = 0;
  for (len = s.length(); len > 0; len--) {
    if (!std::isspace((int)s.at(len - 1))) {
      break;
    }
  }
  s.erase(s.begin() + len, s.end());
}
GLint Gu::glGetInteger(GLenum arg) {
  GLint out = 0;
  glGetIntegerv(arg, &out);
  return out;
}
void Gu::debugBreak() {
#if defined(__debugbreak)
  __debugbreak
#elif defined(SIGTRAP)
  raise(SIGTRAP);
#endif
}
std::string Gu::getShaderInfoLog(GLint prog) {
  GLint maxlen = 0;
  glGetShaderiv(prog, GL_INFO_LOG_LENGTH, &maxlen);
  std::vector<char> buf(maxlen);
  GLint outlen = 0;
  glGetShaderInfoLog(prog, maxlen, &outlen, buf.data());
  auto info = std::string(buf.begin(), buf.begin() + outlen);
  return info;
}
std::string Gu::getProgramInfoLog(GLint prog) {
  GLint maxlen = 0;
  glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &maxlen);
  std::vector<char> buf(maxlen);
  GLint outlen = 0;
  glGetProgramInfoLog(prog, maxlen, &outlen, buf.data());
  auto info = std::string(buf.begin(), buf.begin() + outlen);
  return info;
}
auto Gu::compileShader(GLenum type, std::string src) {
  auto shader = glCreateShader(type);
  std::vector<char*> lines;
  int last = 0;
  int next = -1;  // src.find('\n', 0);
  for (int xxx = 0; xxx < 99999999; xxx++) {
    last = next + 1;
    next = src.find('\n', last);
    if (next == std::string::npos) {
      next = src.length();
    }
    if (next - last > 0) {
      auto line = src.substr(last, next - last);
      Gu::rtrim(line);
      char* ch = new char[line.size() + 2];
      strcpy(ch, line.c_str());
      ch[line.size()] = '\n';
      ch[line.size() + 1] = '\0';
      lines.push_back(ch);
    }
    if (next == std::string::npos) {
      break;
    }
  }
  assert(lines.size() > 0);

  glShaderSource(shader, lines.size(), (const char**)lines.data(), NULL);
  for (auto line : lines) {
    delete[] line;
  }

  glCompileShader(shader);
  Gu::checkErrors();
  GLint status = 0;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
  if (status == GL_FALSE) {
    std::string info = getShaderInfoLog(shader);
    std::string st = "";
    if (type == GL_VERTEX_SHADER) {
      st = "vertex";
    }
    else if (type == GL_FRAGMENT_SHADER) {
      st = "fragment";
    }
    else if (type == GL_GEOMETRY_SHADER) {
      st = "geometry";
    }
    throw Exception("Failed to compile " + st + " shader \n" + info);
  }
  return shader;
}

auto Gu::createShader(std::string vert_src, std::string geom_src, std::string frag_src) {
  auto prog = glCreateProgram();
  assert(vert_src.length());
  assert(frag_src.length());
  auto vert = compileShader(GL_VERTEX_SHADER, vert_src);
  auto frag = compileShader(GL_FRAGMENT_SHADER, frag_src);
  // glBindFragDataLocation(frag, 0, "_output");
  GLuint geom = 0;
  glAttachShader(prog, vert);
  glAttachShader(prog, frag);
  if (geom_src.length()) {
    geom = compileShader(GL_GEOMETRY_SHADER, geom_src);
    glAttachShader(prog, geom);
  }
  else {
    geom = 0;
  }
  glLinkProgram(prog);
  GLint status = 0;
  glGetProgramiv(prog, GL_LINK_STATUS, &status);
  if (status == GL_FALSE) {
    Log::dbg(getProgramInfoLog(prog));
    glDeleteProgram(prog);
    throw Exception("Failed to link program");
  }
  glDetachShader(prog, vert);
  glDetachShader(prog, frag);
  if (geom) {
    glDetachShader(prog, geom);
  }
  return prog;
}
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
std::unique_ptr<Image> Image::from_file(std::string path) {
  int w = 0, h = 0, n = 0;
  auto* data = stbi_load(path.c_str(), &w, &h, &n, 4);
  if (data != NULL) {
    Log::msg(std::string() + "Loaded " + path + " w=" + std::to_string(w) + " h=" + std::to_string(h) + "");
    auto img = std::make_unique<Image>(w, h, n, (char*)data);
    stbi_image_free(data);
    return img;
  }
  else {
    throw Exception("could not load image " + path);
  }
  return nullptr;
}

auto Gu::createTexture(std::string fileloc) {
  auto dirpath = std::filesystem::path(Gu::app_path).parent_path();
  auto impath = dirpath / std::filesystem::path(fileloc);
  assert(std::filesystem::exists(impath));

  auto img = Image::from_file(impath);

  GLuint tex = 0;
  glCreateTextures(GL_TEXTURE_2D, 1, &tex);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  // test
  // unsigned char pix[4];
  // pix[0]=255,pix[1]=0,pix[2]=255,pix[3]=255;
  // glTextureStorage2D(tex, 1, GL_RGBA8, 1, 1);
  // glTextureSubImage2D(tex, 0, 0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, (void*)pix);

  glTextureStorage2D(tex, 1, GL_RGBA8, img->width(), img->height());
  glTextureSubImage2D(tex, 0, 0, 0, img->width(), img->height(), GL_RGBA, GL_UNSIGNED_BYTE, (void*)img->data());
  glTextureParameteri(tex, GL_TEXTURE_BASE_LEVEL, 0);
  glTextureParameteri(tex, GL_TEXTURE_MAX_LEVEL, 0);  // max(0, mips - 1));
  // TODO: mips
  glTextureParameterf(tex, GL_TEXTURE_MIN_FILTER, GL_NEAREST);  // GL_LINEAR_MIPMAP_LINEAR);
  glTextureParameterf(tex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  Gu::checkErrors();
  return tex;
}
void Gu::checkErrors() {
  auto n = glGetError();
  std::string err = "";
  while (n != GL_NO_ERROR) {
    err += "GL Error: " + std::to_string(n) + "\n";
    n = glGetError();
  }
  err += printDebugMessages();
  if (err.length() > 0) {
    // caller = getframeinfo(stack()[1][0])
    // err += caller.filename+":"+str(caller.lineno)
    Log::err(err);
  }
}

std::string Gu::printDebugMessages() {
  std::string ret = "";
  auto count = Gu::glGetInteger(GL_DEBUG_LOGGED_MESSAGES);
  if (count > 0) {
    int max_size = Gu::glGetInteger(GL_MAX_DEBUG_MESSAGE_LENGTH);
    // buffer = ctypes.create_string_buffer(max_size)
    for (auto i = 0; i < count; i++) {
      char* buf = new char[max_size];
      int len = 0;
      auto result = glGetDebugMessageLog(1, max_size, NULL, NULL, NULL, NULL, &len, buf);
      if (result) {
        ret += std::string(buf, len) + "\n";
      }
    }
  }
  return ret;
}
#pragma endregion
#pragma region MainWindow

// class def
MainWindow::MainWindow(int w, int h, std::string title) {
  Log::msg("Initializing " + std::to_string(w) + " " + std::to_string(h));
  _width = w;
  _height = h;

  if (!glfwInit()) {
    throw Exception("Failed to init glfw");
  }
  glfwWindowHint(GLFW_RED_BITS, 8);
  glfwWindowHint(GLFW_GREEN_BITS, 8);
  glfwWindowHint(GLFW_BLUE_BITS, 8);
  glfwWindowHint(GLFW_ALPHA_BITS, 8);
  glfwWindowHint(GLFW_RESIZABLE, true);
  glfwWindowHint(GLFW_DEPTH_BITS, 24);
  glfwWindowHint(GLFW_SAMPLES, 1);
  glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_DOUBLEBUFFER, true);

  _window = glfwCreateWindow(w, h, title.c_str(), NULL, NULL);
  if (!_window) {
    glfwTerminate();
    throw Exception("Failed to create glfw window.");
  }

  glfwMakeContextCurrent(_window);
  glfwSwapInterval(0);

  g_windows.insert(std::make_pair(_window, this));

  glfwSetKeyCallback(_window, [](auto win, auto key, auto code, auto action, auto mods) {
    auto mw = MainWindow::getWindow(win);
    if (key == GLFW_KEY_F11 && action == GLFW_PRESS) {
      mw->toggleFullscreen();
    }
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
      mw->quit();
    }
    if (action == GLFW_PRESS) {
      KeyEvent k;
      k._key = key;
      k._press = true;
      mw->_key_events.push_back(k);
    }
    else if (action == GLFW_RELEASE) {
      KeyEvent k;
      k._key = key;
      k._press = false;
      mw->_key_events.push_back(k);
    }
  });

  glfwSetFramebufferSizeCallback(
      _window, [](auto win, auto w, auto h) {
        MainWindow::getWindow(win)->on_resize(w, h);
        ;
      });
  glfwSetWindowCloseCallback(
      _window, [](auto win) {
        MainWindow::getWindow(win)->quit();
      });

  // glew
  glewExperimental = GL_FALSE;
  auto glewinit = glewInit();
  if (glewinit != GLEW_OK) {
    throw Exception("glewInit failed.");
  }
  glUseProgram(0);  // test  glew
}
MainWindow::~MainWindow() {
  glDeleteVertexArrays(1, &_vao);
  glDeleteTextures(1, &_test_tex);
  glDeleteTextures(1, &_normal_depth);
  glDeleteTextures(1, &_color);
  glDeleteBuffers(1, &_vbo);
  glDeleteBuffers(1, &_ibo);
  glDeleteFramebuffers(1, &_fbo);
  glDeleteProgram(_shader);
  glfwTerminate();
}
MainWindow* MainWindow::getWindow(GLFWwindow* win) {
  auto ite = g_windows.find(win);
  if (ite != g_windows.end()) {
    return ite->second;
  }
  throw Exception("could not find window given GLFW window ");
  return nullptr;
}
void MainWindow::run() {
  createRenderEngine();
  _running = true;
  auto last = glfwGetTime();
  while (_running) {
    auto t = glfwGetTime();
    _delta = (float)(t - last);
    last = t;
    do_input();
    render();

    glfwSwapBuffers(_window);
    glfwPollEvents();
    if (glfwWindowShouldClose(_window)) {
      _running = false;
    }
    // glfwWaitEvents(100);
  }
}
void MainWindow::quit() {
  _running = false;
}
bool MainWindow::pressOrDown(int key) {
  auto it = _keys.find(key);
  if (it != _keys.end()) {
    return it->second == KeyState::Press || it->second == KeyState::Down;
  }
  return false;
}
void MainWindow::do_input() {
  if (_key_events.size()) {
    for (auto& ke : _key_events) {
      auto it = _keys.find(ke._key);
      if (it == _keys.end()) {
        _keys.insert(std::make_pair(ke._key, KeyState::Up));
        it = _keys.find(ke._key);
      }
      auto state = it->second;
      if (state == KeyState::Up && ke._press) {
        state = KeyState::Press;
      }
      else if (state == KeyState::Press && ke._press) {
        state = KeyState::Down;
      }
      else if (state == KeyState::Press && !ke._press) {
        state = KeyState::Release;
      }
      else if (state == KeyState::Down && !ke._press) {
        state = KeyState::Release;
      }
      else if (state == KeyState::Release && ke._press) {
        state = KeyState::Press;
      }
      else if (state == KeyState::Release && !ke._press) {
        state = KeyState::Up;
      }
      it->second = state;
    }
    _key_events.clear();
  }

  if (pressOrDown(GLFW_KEY_UP)) {
    _pos += _heading * _speed * _delta;
  }
  if (pressOrDown(GLFW_KEY_DOWN)) {
    _pos -= _heading * _speed * _delta;
  }
  if (pressOrDown(GLFW_KEY_LEFT)) {
    glm::mat4 m(1.0);
    // todo: up x h x h = "up"
    m = glm::rotate(m, (float)(M_PI * 2.0) * _rspeed * _delta, _up);
    _heading = glm::normalize(glm::vec3(m * glm::vec4(_heading, 1)));
  }
  if (pressOrDown(GLFW_KEY_RIGHT)) {
    glm::mat4 m(1.0);
    m = glm::rotate(m, (float)(M_PI * 2.0) * -_rspeed * _delta, _up);
    _heading = glm::normalize(glm::vec3(m * glm::vec4(_heading, 1)));
  }
}
void MainWindow::createRenderEngine() {
  Log::msg("GL version: " + std::to_string(glGetString(GL_VERSION)));

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_SCISSOR_TEST);
  glDisable(GL_CULL_FACE);
  glFrontFace(GL_CCW);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_DEBUG_OUTPUT);
  glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
  glClearColor(0.89, 0.91, 0.91, 1.0);

  Gu::checkErrors();

  // texs
  _test_tex = Gu::createTexture("../assets/kratos.jpg");

  // bufs
  auto vbo_binding_idx = 0;
  auto ibo_binding_idx = 1;

  RenderQuad q0;
  q0._verts[0]._v = glm::vec3(0, 0, 0);
  q0._verts[1]._v = glm::vec3(1, 0, 0);
  q0._verts[2]._v = glm::vec3(0, 1, 0);
  q0._verts[3]._v = glm::vec3(1, 1, 0);
  q0._verts[0]._x = glm::vec2(0, 0);
  q0._verts[1]._x = glm::vec2(1, 0);
  q0._verts[2]._x = glm::vec2(0, 1);
  q0._verts[3]._x = glm::vec2(1, 1);
  q0._verts[0]._c = glm::vec4(1, 1, 1, 1);
  q0._verts[1]._c = glm::vec4(1, 1, 1, 1);
  q0._verts[2]._c = glm::vec4(1, 1, 1, 1);
  q0._verts[3]._c = glm::vec4(1, 1, 1, 1);
  _quads = {q0};
  for (int i = 0; i < 1000; i++) {
    RenderQuad cpy(q0);
    glm::vec3 rnd;
    rnd.x = Gu::shitrand(-50, 50);
    rnd.y = Gu::shitrand(-50, 50);
    rnd.z = Gu::shitrand(-50, 50);
    cpy.translate(rnd);
    rnd.x = Gu::shitrand(0.1f, 4.0f);
    rnd.y = Gu::shitrand(0.1f, 4.0f);
    rnd.z = Gu::shitrand(0.1f, 4.0f);
    cpy.scale(rnd);

    // color
    for (auto& v : cpy._verts) {
      glm::vec4 rnd4;
      rnd4.r = Gu::shitrand(0.1f, 1.0f);
      rnd4.g = Gu::shitrand(0.6f, 1.0f);
      rnd4.b = Gu::shitrand(0.6f, 1.0f);
      rnd4.a = Gu::shitrand(0.1f, 1.0f);
      v._c = rnd4;
    }

    _quads.push_back(cpy);
  }
  int x = sizeof(RenderQuad) * _quads.size();
  glCreateBuffers(1, &_vbo);
  glNamedBufferStorage(_vbo, sizeof(RenderQuad) * _quads.size(), _quads.data(), vbo_binding_idx);
  // glNamedBufferData(_vbo, sizeof(RenderQuad) * _quads.size(), _quads.data(), GL_STATIC_DRAW);
  glCreateBuffers(1, &_ibo);
  auto inds = std::vector<uint32_t>();
  for (auto i = 0; i < _quads.size(); i++) {
    inds.push_back(i * 4 + 0);
    inds.push_back(i * 4 + 1);
    inds.push_back(i * 4 + 3);
    inds.push_back(i * 4 + 0);
    inds.push_back(i * 4 + 3);
    inds.push_back(i * 4 + 2);
  }
  glNamedBufferStorage(_ibo, sizeof(uint32_t) * inds.size(), inds.data(), ibo_binding_idx);
  // glNamedBufferData(_ibo, sizeof(uint32_t) * inds.size(), inds.data(), GL_STATIC_DRAW);
  Gu::checkErrors();

  // vao
  glCreateVertexArrays(1, &_vao);
  GLuint v_idx = 0;
  GLuint x_idx = 1;
  GLuint c_idx = 2;

  glEnableVertexArrayAttrib(_vao, v_idx);
  glEnableVertexArrayAttrib(_vao, x_idx);
  glEnableVertexArrayAttrib(_vao, c_idx);

  glVertexArrayAttribFormat(_vao, v_idx, 3, GL_FLOAT, GL_FALSE, 0);
  glVertexArrayAttribFormat(_vao, x_idx, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec4));
  glVertexArrayAttribFormat(_vao, c_idx, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4) + sizeof(glm::vec2));

  glVertexArrayAttribBinding(_vao, v_idx, vbo_binding_idx);
  glVertexArrayAttribBinding(_vao, x_idx, vbo_binding_idx);
  glVertexArrayAttribBinding(_vao, c_idx, vbo_binding_idx);

  glVertexArrayVertexBuffer(_vao, vbo_binding_idx, _vbo, 0, sizeof(QuadVert));
  glVertexArrayElementBuffer(_vao, _ibo);

  Gu::checkErrors();

  // scene
  _model = glm::mat4(1.0f);

  glfwGetWindowPos(_window, &_lastx, &_lasty);
  glfwGetWindowSize(_window, &_lastw, &_lasth);
  on_resize(_lastw, _lasth);
  makeProgram();

  Gu::checkErrors();
}
void MainWindow::on_resize(int w, int h) {
  _lastw = _width;
  _lasth = _height;
  _width = w;
  _height = h;
  //    glfwSetWindowAttrib(_window, GLFW_FOCUSED, true);

  initFramebuffer();
}
void MainWindow::initFramebuffer() {
  // TODO:framebuffer
  glViewport(0, 0, _width, _height);
  glScissor(0, 0, _width, _height);
  float fovx = 40.0f;
  _proj = glm::perspectiveFov(fovx, (float)_width, (float)_height, 1.0f, 1000.0f);
}
void MainWindow::render() {
  _view = glm::lookAt(_pos, _pos + _heading, _up);

  auto model_loc = glGetUniformLocation(_shader, "_modelMatrix");
  auto view_loc = glGetUniformLocation(_shader, "_viewMatrix");
  auto proj_loc = glGetUniformLocation(_shader, "_projMatrix");
  auto tex_loc = glGetUniformLocation(_shader, "_texture");

  Assert(model_loc == -1);
  Assert(view_loc == -1);
  Assert(proj_loc == -1);
  Assert(tex_loc == -1);

  glProgramUniformMatrix4fv(_shader, model_loc, 1, GL_FALSE, (GLfloat*)&_model);
  glProgramUniformMatrix4fv(_shader, view_loc, 1, GL_FALSE, (GLfloat*)&_view);
  glProgramUniformMatrix4fv(_shader, proj_loc, 1, GL_FALSE, (GLfloat*)&_proj);
  glProgramUniform1i(_shader, tex_loc, 0);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glBindVertexArray(_vao);
  glBindTextureUnit(0, _test_tex);
  glUseProgram(_shader);
  glDrawElements(GL_TRIANGLES, _quads.size() * 6, GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
  glUseProgram(0);

  Gu::checkErrors();
}
void MainWindow::makeProgram() {
  std::string vert_src = "                                                \n\
  #version 450                                       \n\
  layout(location = 0) in vec3 _v3;                                       \n\
  layout(location = 1) in vec2 _x2;                                       \n\
  layout(location = 2) in vec4 _c4;                                       \n\
                                                                          \n\
  uniform mat4 _projMatrix;                                               \n\
  uniform mat4 _viewMatrix;                                               \n\
  uniform mat4 _modelMatrix;                                              \n\
                                                                          \n\
  out vec2 _tcoord;                                                       \n\
  out vec4 _color;                                                        \n\
                                                                          \n\
  void main() {                                                           \n\
    _tcoord = _x2;                                                        \n\
    _color = _c4;                                                         \n\
    vec4 vert = _projMatrix * _viewMatrix * _modelMatrix *  vec4(_v3, 1);  \n\
    gl_Position = vert;                                                   \n\
  }                                                                       \n\
  ";
  std::string frag_src = "                         \n\
  #version 450                                     \n\
  uniform sampler2D _texture;  \n\
                                                   \n\
  in vec2 _tcoord;                                 \n\
  in vec4 _color;                                  \n\
                                                   \n\
  out vec4 _output;                                \n\
                                                   \n\
  void main() {                                    \n\
    _output = texture(_texture, _tcoord); \n\
    //_output = _color; //vec4(1,0,1,1); //texture(_texture, _tcoord) * _color; \n\
    //_output.a=1.0;                                 \n\
  }                                                \n\
  ";
  _shader = Gu::createShader(vert_src, "", frag_src);
}

#pragma endregion

}  // namespace TestApp

int main(int argc, char** argv) {
  try {
    TestApp::Gu::app_path = argv[0];
    auto win = std::make_unique<TestApp::MainWindow>(500, 300, "2.6D Test");
    win->run();
  }
  catch (TestApp::Exception ex) {
    TestApp::Log::exception(ex);
  }
}
