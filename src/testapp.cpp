#include "./testapp.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "./BinaryFile.h"

namespace std {

std::string to_string(const char* __val) { return std::string(__val); }
std::string to_string(const GLubyte* __val) { return std::string((const char*)__val); }

}  // namespace std

void main_thread_handler(void* user_data) {
}

namespace TestApp {

#pragma region fwd
// utils

// fwd

#pragma endregion
#pragma region defs
//
// class b2_obj{
//   int32_t _id = -1
//   std::string _name = ""
//   std::vector<b2_action> _actions ;
//
//   def serialize(self, bf : BinaryFile):
//     bf.writeInt32(self._id)
//     bf.writeString(self._name)
//     bf.writeInt32(len(self._actions))
//     for act in self._actions:
//       self._actions[act].serialize(bf)
// class b2_action:
//   def __init__(self):
//     self._id = -1
//     self._name = ""
//     self._frames = {}
//   def serialize(self, bf : BinaryFile):
//     bf.writeInt32(self._id)
//     bf.writeString(self._name)
//     bf.writeInt32(len(self._frames))
//     for fr in self._frames:
//       self._frames[fr].serialize(bf)
// class b2_frame:
//   def __init__(self):
//     self._seq: float = -1  # float
//     self._name = ""
//     self._texid = -1
//     self._x = -1
//     self._y = -1
//     self._w = -1
//     self._h = -1
//   def serialize(self, bf : BinaryFile):
//     bf.writeFloat(self._seq)
//     bf.writeString(self._name)
//     bf.writeInt32(self._texid)
//     bf.writeInt32(self._x)
//     bf.writeInt32(self._y)
//     bf.writeInt32(self._w)
//     bf.writeInt32(self._h)
// class b2_tex:
//   def __init__(self):
//     self._texid = -1
//     self._texs : str = []
// clss dcl

class MainWindow {
private:
  enum class KeyState {
    Up,
    Press,
    Down,
    Release,
  };
  struct KeyEvent {
    int _key = 0;
    bool _press = false;
  };

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
  GLuint _color = 0;
  GLuint _normal_depth = 0;

  std::vector<b2_tex> _texs;
  std::vector<b2_obj> _objs;

  void loadMeta(std::string);
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
  void quit_everything();
  void run();
};

#pragma endregion
#pragma region static data
// static data
std::string Log::CC_BLACK = "30";
std::string Log::CC_RED = "31";
std::string Log::CC_GREEN = "32";
std::string Log::CC_YELLOW = "33";
std::string Log::CC_BLUE = "34";
std::string Log::CC_PINK = "35";
std::string Log::CC_CYAN = "36";
std::string Log::CC_WHITE = "37";
std::string Log::CC_NORMAL = "39";
std::map<GLFWwindow*, MainWindow*> MainWindow::g_windows;
std::filesystem::path Gu::app_path = "";

#pragma endregion
#pragma region b2
class b2_obj;
class b2_action;
class b2_frame;
class b2_tex;
class BinaryFile;

class b2_obj {
public:
  int32_t _id = -1;
  std::string _name = "";
  std::vector<b2_action> _actions;
  void deserialize(BinaryFile* bf) {
    // bf.writeInt32(self._id)
    //     bf.writeString(self._name)
    //     bf.writeInt32(len(self._actions))
    //     for act in self._actions:
    //       self._actions[act].serialize(bf)
  }
};
class b2_action {
public:
  int32_t _id = -1;
  std::string _name = "";
  std::vector<b2_frame> _frames;
  void deserialize(BinaryFile* bf) {
    //  bf.writeInt32(self._id)
    //  bf.writeString(self._name)
    //  bf.writeInt32(len(self._frames))
    //  for fr in self._frames:
    //    self._frames[fr].serialize(bf)
    //    };
  }
};
class b2_frame {
public:
  float _seq = -1;
  std::string _name = "";
  int32_t _texid = -1;
  int32_t _x = -1;
  int32_t _y = -1;
  int32_t _w = -1;
  int32_t _h = -1;
  void deserialize(BinaryFile* bf) {
    // bf.writeFloat(self._seq)
    // bf.writeString(self._name)
    // bf.writeInt32(self._texid)
    // bf.writeInt32(self._x)
    // bf.writeInt32(self._y)
    // bf.writeInt32(self._w)
    // bf.writeInt32(self._h)
  }
};
class b2_tex {
public:
  int32_t _texid = -1;
  std::vector<std::string> _images;
};
#pragma endregion
#pragma region Gu
void Gu::debugBreak() {
#if defined(__debugbreak)
  //__debugbreak
#elif _WIN32
  //  DebugBreak();
#elif __linux__
  //  raise(SIGTRAP);
#else
  //   OS_NOT_SUPPORTED_ERROR
#endif
}
uint64_t Gu::getMicroSeconds() {
  int64_t ret;
  std::chrono::nanoseconds ns = std::chrono::high_resolution_clock::now().time_since_epoch();
  ret = std::chrono::duration_cast<std::chrono::microseconds>(ns).count();
  return ret;
}
uint64_t Gu::getMilliSeconds() {
  return getMicroSeconds() / 1000;
}
std::string Gu::executeReadOutput(const std::string& cmd) {
  std::string data = "";
#if defined(__linux__)
  // This works only if VSCode launches the proper terminal (some voodoo back there);
  const int MAX_BUFFER = 8192;
  char buffer[MAX_BUFFER];
  std::memset(buffer, 0, MAX_BUFFER);
  std::string cmd_mod = std::string() + cmd + " 2>&1";  // redirect stderr to stdout

  FILE* stream = popen(cmd_mod.c_str(), "r");
  if (stream) {
    while (fgets(buffer, MAX_BUFFER, stream) != NULL) {
      data.append(buffer);
    }
    if (ferror(stream)) {
      std::cout << "Error executeReadOutput() " << std::endl;
    }
    clearerr(stream);
    pclose(stream);
  }
#else
  LogWarn("Tried to call invalid method on non-linux platform.");
  // Do nothing
#endif
  return data;
}
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

std::unique_ptr<Image> Image::from_file(std::string path) {
  int w = 0, h = 0, n = 0;
  auto* data = stbi_load(path.c_str(), &w, &h, &n, 4);
  if (data != NULL) {
    msg(std::string() + "Loaded " + path + " w=" + std::to_string(w) + " h=" + std::to_string(h) + "");
    auto img = std::make_unique<Image>(w, h, n, (char*)data);
    stbi_image_free(data);
    return img;
  }
  else {
    throw Exception("could not load image " + path);
  }
  return nullptr;
}
bool Gu::exists(std::filesystem::path path) {
  return std::filesystem::exists(path);
}
std::filesystem::path Gu::relpath(std::string relpath) {
  auto dirpath = std::filesystem::path(Gu::app_path).parent_path();
  auto impath = dirpath / std::filesystem::path(relpath);
  return impath;
}
auto Gu::createTexture(std::filesystem::path impath) {
  LogInfo("Loading " + impath.string());
  assert(std::filesystem::exists(impath));

  auto img = Image::from_file(impath.string());

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
  std::cout << "got here 1" << std::endl;
  msg("Initializing " + std::to_string(w) + " " + std::to_string(h));
  std::cout << "got here 2" << std::endl;
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
      mw->quit_everything();
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
        MainWindow::getWindow(win)->quit_everything();
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
void MainWindow::quit_everything() {
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

  auto spdmul = pressOrDown(GLFW_KEY_LEFT_SHIFT) || pressOrDown(GLFW_KEY_RIGHT_SHIFT) ? 5.0f : 1.0f;

  if (pressOrDown(GLFW_KEY_UP) || pressOrDown(GLFW_KEY_W)) {
    _pos += _heading * _speed * spdmul * _delta;
  }
  if (pressOrDown(GLFW_KEY_DOWN) || pressOrDown(GLFW_KEY_S)) {
    _pos -= _heading * _speed * spdmul * _delta;
  }
  if (pressOrDown(GLFW_KEY_LEFT) || pressOrDown(GLFW_KEY_A)) {
    glm::mat4 m(1.0);
    // todo: up x h x h = "up"
    m = glm::rotate(m, (float)(M_PI * 2.0) * _rspeed * _delta, _up);
    _heading = glm::normalize(glm::vec3(m * glm::vec4(_heading, 1)));
  }
  if (pressOrDown(GLFW_KEY_RIGHT) || pressOrDown(GLFW_KEY_D)) {
    glm::mat4 m(1.0);
    m = glm::rotate(m, (float)(M_PI * 2.0) * -_rspeed * _delta, _up);
    _heading = glm::normalize(glm::vec3(m * glm::vec4(_heading, 1)));
  }
}

void MainWindow::createRenderEngine() {
  msg("GL version: " + std::to_string(glGetString(GL_VERSION)));

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
  _test_tex = Gu::createTexture(Gu::relpath("../../data/tex/kratos.jpg"));
  auto normpath = Gu::relpath("../../output/mt_normal_depth.png");
  auto colpath = Gu::relpath("../../output/mt_color.png");
  if (!Gu::exists(normpath) || !Gu::exists(colpath)) {
    throw Exception(std::string() + "could not find textures: \n" + (normpath.string()) + "\n" + (colpath.string()) + "\n run the blender python script first");
  }
  _normal_depth = Gu::createTexture(normpath);
  _color = Gu::createTexture(colpath);

  loadMeta("../../output/b2_meta.bin");

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
void MainWindow::loadMeta(std::string loc) {
  msg("Loading metadata " + loc);
  
  BinaryFile bf;
  bf.loadFromDisk(Gu::relpath(loc).string());
  int hdr_b = bf.readByte();
  int hdr_2 = bf.readByte();
  int hdr_m = bf.readByte();
  int hdr_d = bf.readByte();

  int ntexs = bf.readInt32();
  for (int i = 0; i < ntexs; i++) {
    b2_tex b;
    b._texid = bf.readInt32();
    auto numimgs = bf.readInt32();
    for (int img = 0; img < numimgs; img++) {
      std::string image = bf.readString();
      b._images.push_back(image);
    }
    _texs.push_back(b);
  }
  int nsprites = bf.readInt32();
  for (int i = 0; i < nsprites; i++) {
    b2_obj ob;
    ob._id = bf.readInt32();
    ob._name = bf.readString();
    auto actioncount = bf.readInt32();
    for (int iact = 0; iact < actioncount; iact++) {
      b2_action act;
      act._id = bf.readInt32();
      act._name = bf.readString();
      auto framecount = bf.readInt32();
      for (int ifr = 0; ifr < framecount; ifr++) {
        b2_frame fr;
        fr._seq = bf.readFloat();
        fr._texid = bf.readInt32();
        fr._x = bf.readInt32();
        fr._y = bf.readInt32();
        fr._w = bf.readInt32();
        fr._h = bf.readInt32();
        act._frames.push_back(fr);
      }
      ob._actions.push_back(act);
    }
    _objs.push_back(ob);
  }
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
  float fovx = 40.0f * ((float)M_PI / 180.0f);
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
  glBindTextureUnit(0, _color);
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

extern "C" {

DLL_EXPORT void DoSomething() {
  std::cout << "hello world c++" << std::endl;
}

DLL_EXPORT int main(int argc, char** argv) {
  try {
    std::cout << "c++ > executing main() argc=" << argc << std::endl;

    Assert(argc > 0);

    TestApp::Gu::app_path = argv[0];
    auto win = std::make_unique<TestApp::MainWindow>(800, 600, "2.6D Test");
    win->run();
  }
  catch (TestApp::Exception ex) {
    TestApp::Log::exception(ex);
    return 1;
  }
  return 0;
}
}