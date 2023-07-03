#include <fstream>
#include <sstream>
#include <iterator>
#include <thread>
#include <numeric>
#include "./testapp.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "./BinaryFile.h"
#include "./StringUtil.h"
#include "./Overlay.h"
#include "./DebugHelper.h"

namespace std {

std::string to_string(const char* __val) {
  return std::string(__val);
}
std::string to_string(const GLubyte* __val) {
  return std::string((const char*)__val);
}

}  // namespace std

namespace TESTAPP_NS {
#pragma region OperatingSystem

string_t OperatingSystem::executeReadOutput(const string_t& cmd) {
  string_t data = "";
#if defined(BR2_OS_LINUX)
  // This works only if VSCode launches the proper terminal (some voodoo back there);
  const int MAX_BUFFER = 256;
  char buffer[MAX_BUFFER];
  std::memset(buffer, 0, MAX_BUFFER);
  string_t cmd_mod = std::string("") + cmd + " 2>&1";  // redirect stderr to stdout

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
  BRLogWarn("Tried to call invalid method on non-linux platform.");
  // Do nothing
#endif
  return data;
}

#pragma endregion
#pragma region static data

const std::string Log::CC_BLACK = "30";
const std::string Log::CC_RED = "31";
const std::string Log::CC_GREEN = "32";
const std::string Log::CC_YELLOW = "33";
const std::string Log::CC_BLUE = "34";
const std::string Log::CC_PINK = "35";
const std::string Log::CC_CYAN = "36";
const std::string Log::CC_WHITE = "37";
const std::string Log::CC_NORMAL = "39";
std::map<GLFWwindow*, uptr<Window>> Gu::_windows;
path_t Gu::_appPath = "";
path_t Gu::_assetsPath = "";
Window* Gu::_master_context = nullptr;
Window* Gu::_context = nullptr;
uptr<AppConfig> Gu::_appConfig;
uptr<World> Gu::_world;
uptr<Prof> Gu::_prof;
uint64_t Gu::s_idgen = 1;
std::map<ImageFormatType, uptr<ImageFormat>> Gu::_imageFormats;
std::unordered_map<std::string, sptr<Material>> Gu::_materials;
uptr<Shader::ShaderMeta> Shader::_meta;

#pragma endregion
#pragma region Prof

Prof::Prof() {
  _last = Gu::getMicroSeconds();
}
void Prof::dump(const char* file, int line) {
  if (Gu::context()->input()->pressOrDown(GLFW_KEY_P)) {
    float res = (float)(((double)(Gu::getMicroSeconds() - _last)) / 1000.0);

    auto cs = DebugHelper::getCallStack();
    std::string tabs = "";
    std::string tab = "  ";
    std::string method = "";
    int depth = -1;
    std::string ns = std::string(TO_STRING(TESTAPP_NS));
    std::function<void(std::string)> methodName = [ns, &method](std::string s) {
      str rex = std::string() + "\\s(" + ns + "::[a-zA-Z0-9_:]+)";
      method = str(" ") + StringUtil::submatch(s, rex);
    };
    for (int i = cs.size() - 1; i >= 0; i--) {
      if (depth != -1) {
        if (cs[i].find(ns + "::Prof") != std::string::npos) {
          break;
        }

        methodName(cs[i]);
        tabs += tab;
        depth += 1;
      }
      if (cs[i].find(ns + "::Gu::run") != std::string::npos) {
        depth = 0;
        methodName(cs[i]);
      }
    }

    Log::print(StringUtil::format("%.2fms", res), res > 2.9f ? Log::CC_RED : Log::CC_CYAN, true);
    Log::print(tabs + method, Log::CC_PINK, true);
    auto fn = std::filesystem::path(std::string(file)).filename().string();
    Log::print(" " + fn + ":" + std::to_string(line), Log::CC_GREEN, true);
    Log::nl();

    _last = Gu::getMicroSeconds();  // filter out the conversion junk above
  }
}

#pragma endregion
#pragma region Log

void Log::err(std::string s, const char* file, int line) {
  _output(CC_RED, false, "E", file, line, s);
}
void Log::warn(std::string s, const char* file, int line) {
  _output(CC_YELLOW, false, "W", file, line, s);
}
void Log::dbg(std::string s, const char* file, int line) {
  _output(CC_CYAN, false, "D", file, line, s);
}
void Log::inf(std::string s, const char* file, int line) {
  _output(CC_WHITE, false, "I", file, line, s);
}
void Log::exception(Exception ex) {
  err(ex.what(), ex.file(), ex.line());
}
void Log::nl() {
  std::cout << std::endl;
}

void Log::println(const std::vector<std::string>& s, std::string color, bool bold) {
  print(s, color, bold, true);
}
void Log::print(const std::vector<std::string>& s, std::string color, bool bold, bool newline) {
  for (auto& i : s) {
    print(i, color, bold, newline);
  }
}
void Log::println(std::string s, std::string color, bool bold) {
  print(s, color, bold, true);
}
void Log::print(std::string s, std::string color, bool bold, bool newline) {
  std::string scolor = "";
  if (color.length()) {
    scolor = cc_color(color, bold);
  }
  std::cout << scolor + s + cc_reset();
  if (newline) {
    nl();
  }
}
std::string Log::cc_reset() {
  return "\033[0m";
}
std::string Log::cc_color(std::string color, bool bold) {
  return std::string() + "\033[" + (bold ? "1;" : "") + color + "m";
}
void Log::_output(std::string color, bool bold, std::string type, const char* file, int line, std::string s) {
  s = _header(color, bold, type, file, line) + s + cc_reset();
  println(s);
}
std::string Log::_header(std::string color, bool bold, std::string type, const char* file, int line) {
  auto pdir = std::filesystem::path(std::string(file)).parent_path().filename();
  auto fn = std::filesystem::path(std::string(file)).filename();
  return cc_color(color, bold) + "[" + type + "] " + std::string(pdir) + "/" + std::string(fn) + ":" + std::to_string(line) + " ";
}

#pragma endregion
#pragma region Exception

Exception::Exception(const char* file, int line, std::string what) {
  _file = file;
  _line = line;
  _what = what;
  _what += OperatingSystem::newline() + DebugHelper::getStackTrace();
}

#pragma endregion
#pragma region Gu

void Gu::initGlobals(std::string exe_path) {
  _appPath = path_t(exe_path).parent_path();
  _assetsPath = _appPath / path_t("../../data/");
  _appConfig = std::make_unique<AppConfig>();
  _prof = std::make_unique<Prof>();

  // Most likely 3 components not supported in array textures so just adding 4 would make it ok.

  _imageFormats.insert(std::make_pair(ImageFormatType::R16, std::make_unique<ImageFormat>(ImageFormatType::R16, GL_RED, GL_R16F, GL_FLOAT, sizeof(float) * 1)));
  _imageFormats.insert(std::make_pair(ImageFormatType::R32, std::make_unique<ImageFormat>(ImageFormatType::R32, GL_RED, GL_R32F, GL_FLOAT, sizeof(float) * 1)));
  _imageFormats.insert(std::make_pair(ImageFormatType::RGB8, std::make_unique<ImageFormat>(ImageFormatType::RGB8, GL_RGB, GL_RGB8, GL_UNSIGNED_BYTE, sizeof(int8_t) * 3)));
  _imageFormats.insert(std::make_pair(ImageFormatType::RGBA8, std::make_unique<ImageFormat>(ImageFormatType::RGBA8, GL_RGBA, GL_RGBA8, GL_UNSIGNED_BYTE, sizeof(int8_t) * 4)));
  _imageFormats.insert(std::make_pair(ImageFormatType::RGBA16, std::make_unique<ImageFormat>(ImageFormatType::RGBA16, GL_RGBA, GL_RGBA16F, GL_FLOAT, sizeof(int16_t) * 4)));
  _imageFormats.insert(std::make_pair(ImageFormatType::RGBA32, std::make_unique<ImageFormat>(ImageFormatType::RGBA32, GL_RGBA, GL_RGBA32F, GL_FLOAT, sizeof(float) * 4)));
  // depth internal may be wrong (probahbl not oging to use anyway)
  //_imageFormats.insert(std::make_pair(ImageFormatType::DEPTH16, std::make_unique<ImageFormat>(ImageFormatType::DEPTH16, GL_DEPTH_COMPONENT16, GL_RG, GL_UNSIGNED_SHORT, sizeof(float))));
  //_imageFormats.insert(std::make_pair(ImageFormatType::DEPTH24, std::make_unique<ImageFormat>(ImageFormatType::DEPTH24, GL_DEPTH_COMPONENT24, GL_RED, GL_UNSIGNED_INT, sizeof(float))));
  _imageFormats.insert(std::make_pair(ImageFormatType::DEPTH32, std::make_unique<ImageFormat>(ImageFormatType::DEPTH32, GL_DEPTH_COMPONENT32F, GL_R32F, GL_FLOAT, sizeof(float))));
}
int Gu::run(int argc, char** argv) {
  int ret_code = 0;
  try {
    Assert(argc > 0);
    Gu::initGlobals(argv[0]);

    if (!glfwInit()) {
      Raise("Failed to init glfw");
    }

    // windows
    auto win = Gu::createWindow(800, 600, "2.6D Test");
    // glfwMakeContextCurrent(win->glfwWindow());
    //_context = win;
    _master_context = win;

    // world
    _world = std::make_unique<World>();

    // auto win2 = Gu::createWindow(800, 600, "2.6D Test");
    // win2->scene() = scene;
    // nput controler update () requires a context.. but is in a scene ..

    bool exit = false;
    while (!exit) {
      std::vector<GLFWwindow*> destroy;
      for (auto ite = _windows.begin(); ite != _windows.end(); ite++) {
        prof();
        auto win = ite->second.get();
        glfwMakeContextCurrent(win->glfwWindow());
        _context = win;
        prof();

        win->updateState();
        prof();

        if (win->visible() && !win->minimized()) {
          win->updateInput();
        }
        prof();

        if (_master_context == win) {
          _world->update();
        }
        win->cullViews();
        prof();

        win->updateSelectedView();
        prof();

        if (win->visible() && !win->minimized()) {
          win->renderViews();
          prof();
        }

        win->swap();

        //

        prof();

        if (win->state() == Window::WindowState::Quit) {
          destroy.push_back(win->glfwWindow());
        }
        prof();
      }
      for (auto fw : destroy) {
        auto it = _windows.find(fw);
        if (it != _windows.end()) {
          _windows.erase(it);
          glfwMakeContextCurrent(0);
        }
      }
      if (_windows.size() == 0) {
        break;
      }
    }
  }
  catch (Exception ex) {
    Log::exception(ex);
    ret_code = 1;
  }

  glfwTerminate();

  return ret_code;
}
void Gu::sleep(int ms) {
  std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}
Window* Gu::createWindow(int w, int h, std::string title) {
  // share ctx with all windows.
  GLFWwindow* sharedctx = nullptr;
  if (_windows.begin() != _windows.end()) {
    sharedctx = _windows.begin()->second->glfwWindow();
  }
  auto win = std::make_unique<Window>();
  _context = win.get();
  win->init(w, h, title, sharedctx);
  auto ret = win.get();
  auto glfwwin = win->glfwWindow();
  _windows.insert(std::make_pair(glfwwin, std::move(win)));
  return ret;
}
Window* Gu::getWindow(GLFWwindow* win) {
  auto ite = _windows.find(win);
  if (ite != _windows.end()) {
    return ite->second.get();
  }
  Raise("could not find window given GLFW window ");
  return nullptr;
}
ImageFormat* Gu::imageFormat(ImageFormatType fmt, bool throwifnotfound) {
  auto it = _imageFormats.find(fmt);
  if (it == _imageFormats.end()) {
    if (throwifnotfound) {
      Raise("Value not found.");
    }
    return nullptr;
  }
  return it->second.get();
}
std::string Gu::pad(std::string st, int width, char padchar) {
  std::ostringstream out;
  out << std::internal << std::setfill(padchar) << std::setw(width) << st;
  return out.str();
}
bool Gu::isDebug() {
  return Gu::config()->EnableDebug;
}
void Gu::debugBreak() {
#if defined(__debugbreak)
  __debugbreak
#elif _WIN32
  DebugBreak();
#elif __linux__
  raise(SIGTRAP);
#else
  OS_NOT_SUPPORTED
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
GLint Gu::glGetInteger(GLenum arg) {
  GLint out = 0;
  glGetIntegerv(arg, &out);
  return out;
}
std::string Gu::readFile(path_t fileLoc) {
  LogInfo("Reading File " + fileLoc.string());

  std::fstream fs;
  fs.open(fileLoc.c_str(), std::ios::in | std::ios::binary);
  if (!fs.good()) {
    fs.close();
    Raise("Could not open file '" + fileLoc.string() + "' for read.");
  }

  size_t size_len;
  fs.seekg(0, std::ios::end);
  size_len = (size_t)fs.tellg();
  fs.seekg(0, std::ios::beg);
  auto ret = std::string(std::istreambuf_iterator<char>(fs), {});
  return ret;
}
bool Gu::exists(path_t path) {
  return std::filesystem::exists(path);
}
path_t Gu::relpath(std::string relpath) {
  auto dirpath = path_t(Gu::_appPath).parent_path();
  auto impath = dirpath / path_t(relpath);
  return impath;
}
path_t Gu::assetpath(std::string path) {
  if (path[0] == '/' || path[0] == '\\') {
    path = path.substr(1, path.length() - 1);
  }
  return _assetsPath / path_t(path);
}
void Gu::checkErrors(const char* file, int line) {
  auto n = glGetError();
  bool error_break = false;
  std::string err = "";
  while (n != GL_NO_ERROR) {
    err += "GL Error: " + std::to_string(n) + "\n";
    n = glGetError();
    error_break = 1;
  }
  error_break += printDebugMessages(err);
  if (err.length() > 0) {
    // caller = getframeinfo(stack()[1][0])
    // err += caller.filename+":"+str(caller.lineno)
    if (error_break != 0) {
      Log::err(err, file, line);
      if (Gu::config()->BreakOnGLError) {
        Gu::debugBreak();
      }
    }
    else {
      Log::dbg(err, file, line);
    }
  }
}
int Gu::printDebugMessages(string_t& ret) {
  // return 1 if error 0 if not
  int level = 0;
  auto count = Gu::glGetInteger(GL_DEBUG_LOGGED_MESSAGES);
  if (count > 0) {
    int max_size = Gu::glGetInteger(GL_MAX_DEBUG_MESSAGE_LENGTH);
    // buffer = ctypes.create_string_buffer(max_size)
    for (auto i = 0; i < count; i++) {
      char* buf = new char[max_size];
      int len = 1;
      GLenum type;
      auto result = glGetDebugMessageLog(1, max_size, NULL, &type, NULL, NULL, &len, buf);

      if (result) {
        if (type == GL_DEBUG_TYPE_ERROR) {
          level = 1;
        }
        ret += std::string(buf, len) + "\n";
      }
    }
  }
  return level;
}
bool Gu::whileTrueGuard(int& x, int max) {
  if (x >= max) {
    RaiseDebug("while(true) guard failed. max=" + max);
    return false;
  }
  x++;
  return true;
}
float Gu::ubtof(uint8_t c) {
  return glm::clamp((float)c / 255.0f, 0.0f, 1.0f);
}
vec4 Gu::rgba_ub(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
  return vec4(Gu::ubtof(r), Gu::ubtof(g), Gu::ubtof(b), Gu::ubtof(a));
}
vec4 Gu::rgb_ub(uint hex) {
  auto r = (uint8_t)(hex >> 16 & 0xFF);
  auto g = (uint8_t)(hex >> 8 & 0xFF);
  auto b = (uint8_t)(hex >> 0 & 0xFF);
  return rgb_ub(r, g, b);
}
vec4 Gu::rgb_ub(uint8_t r, uint8_t g, uint8_t b) {
  return vec4(Gu::ubtof(r), Gu::ubtof(g), Gu::ubtof(b), 1);
}
sptr<Material> Gu::findMaterial(const string_t& name) {
  auto it = _materials.find(name);
  if (it == _materials.end()) {
    return nullptr;
  }
  return it->second;
}

#pragma endregion
#pragma region Image

Image::Image(int w, int h, ImageFormat* fmt, const char* data) {
  init(w, h, fmt, data);
}
Image::~Image() {
  Gu::trap();
}
void Image::init(int w, int h, ImageFormat* fmt, const char* data) {
  Assert(w > 0);
  Assert(h > 0);
  _width = w;
  _height = h;
  _format = fmt;
  size_t len = w * h * format()->bpp();
  _data = std::make_unique<char[]>(len);
  if (data != nullptr) {
    memcpy(_data.get(), data, len);
  }
}
void Image::check() const {
  Assert(this->_width > 0);
  Assert(this->_height > 0);
  Assert(this->_data != nullptr);
}
uptr<Image> Image::from_file(std::string path) {
  int w = 0, h = 0, bp = 0;
  // note if you add req comp = 4 for bpp - then the returned bpp VALUE will be the actual BPP, but
  // the actual IMAGE will have been converted to the req comp
  bool force_4bpp = true;
  auto* data = stbi_load(path.c_str(), &w, &h, &bp, force_4bpp ? 4 : 0);
  if (data != NULL) {
    msg(std::string() + "Loaded " + path + " w=" + w + " h=" + h + " bpp=" + bp);
    bp = force_4bpp ? 4 : bp;
    Assert(bp == 3 || bp == 4);
    auto fmtt = bp == 3 ? ImageFormatType::RGB8 : ImageFormatType::RGBA8;
    auto fmt = Gu::imageFormat(fmtt);
    auto img = std::make_unique<Image>(w, h, fmt, (char*)data);
    img->_name = path;
    stbi_image_free(data);
    return img;
  }
  else {
    Raise("could not load image " + path);
  }
  return nullptr;
}
uptr<Image> Image::scale(Image* img, float s, ImageFormat* changefmt) {
  Assert(img != nullptr);
  Assert(s > 0);

  auto fmt = changefmt != nullptr ? changefmt : img->format();
  auto dst = std::make_unique<Image>(floor((float)img->width() * s), floor((float)img->height() * s), fmt);
  auto srcbox = box2i(ivec2(0, 0), ivec2(img->width(), img->height()));
  auto dstbox = box2i(ivec2(0, 0), ivec2(dst->width(), dst->height()));
  Image::copy(dst.get(), dstbox, img, srcbox);
  return dst;
}
uptr<Image> Image::crop(Image* img, const box2i& region) {
  Assert(img != nullptr);
  auto dst = std::make_unique<Image>(region.width(), region.height(), img->format());
  auto dstbox = box2i(ivec2(0, 0), ivec2(region.width(), region.height()));
  Image::copy(dst.get(), dstbox, img, region);
  return dst;
}
void Image::copy(Image* dst, const box2i& dstbox, Image* src, const box2i& srcbox) {
  Assert(dst != nullptr);
  Assert(src != nullptr);
  Assert(src->_data != nullptr);
  Assert(dst->_data != nullptr);
  Assert(src->format()->bpp() > 0);
  Assert(dst->format()->bpp() > 0);
  Assert(src->format()->bpp() <= 4);  // for now
  Assert(dst->format()->bpp() <= 4);
  Assert(src->width() > 0);
  Assert(src->height() > 0);
  Assert(dst->width() > 0);
  Assert(dst->height() > 0);
  Assert(dstbox.width() > 0);
  Assert(dstbox.height() > 0);
  Assert(srcbox.width() > 0);
  Assert(srcbox.height() > 0);

  auto dstbounds = box2i(ivec2(0, 0), ivec2(dst->width(), dst->height()));
  auto srcbounds = box2i(ivec2(0, 0), ivec2(src->width(), src->height()));
  auto c_dst = dstbox.clip(dstbounds);
  auto c_src = srcbox.clip(srcbounds);

  Assert(c_dst.width() > 0);
  Assert(c_dst.height() > 0);

  float incx_dst = 0, incx_src = 0;
  float incy_dst = 0, incy_src = 0;
  int row_x = 0, row_y = 0;

  if (dstbox.width() > srcbox.width()) {
    row_x = srcbox.width();
    incx_src = 1;
    incx_dst = (float)dstbox.width() / (float)srcbox.width();
  }
  else {
    row_x = dstbox.width();
    incx_dst = 1;
    incx_src = (float)srcbox.width() / (float)dstbox.width();
  }
  if (dstbox.height() > srcbox.height()) {
    row_y = srcbox.height();
    incy_src = 1;
    incy_dst = (float)dstbox.height() / (float)srcbox.height();
  }
  else {
    row_y = dstbox.height();
    incy_dst = 1;
    incy_src = (float)srcbox.height() / (float)dstbox.height();
  }

  float sx = (float)srcbox._min.x;
  float sy = (float)srcbox._min.y;
  float dx = (float)dstbox._min.x;
  float dy = (float)dstbox._min.y;

  u8vec4 fillcolor = u8vec4(0, 0, 0, 0);
  int copy_bpp = glm::min(src->format()->bpp(), dst->format()->bpp());
  size_t dbg_bytes_copied = 0;

  for (int yi = 0; yi < row_y; yi++) {
    for (int xi = 0; xi < row_x; xi++) {
      int idx = (int)glm::round(dx);
      int idy = (int)glm::round(dy);
      int isx = (int)glm::round(sx);
      int isy = (int)glm::round(sy);

      if (c_dst.contains_LT_inclusive(ivec2(idx, idy))) {
        void* dpx = dst->pixel(idx, idy);

        void* spx = nullptr;
        if (c_src.contains_LT_inclusive(ivec2(isx, isy))) {
          spx = src->pixel(isx, isy);
        }
        else {
          spx = &fillcolor;
          Gu::trap();
        }

        memcpy(dpx, spx, copy_bpp);

        // debug
        // if (copy_bpp == 4) {
        //   char p_s0 = *((char*)spx + 0);
        //   char p_s1 = *((char*)spx + 1);
        //   char p_s2 = *((char*)spx + 2);
        //   char p_s3 = *((char*)spx + 3);
        //   char p_d0 = *((char*)dpx + 0);
        //   char p_d1 = *((char*)dpx + 1);
        //   char p_d2 = *((char*)dpx + 2);
        //   char p_d3 = *((char*)dpx + 3);
        //   if (*((char*)spx + 3) == 0) {
        //     *((char*)dpx + 0) = 255;
        //     *((char*)dpx + 1) = 0;
        //     *((char*)dpx + 2) = 255;
        //     *((char*)dpx + 3) = 255;
        //   }
        // }

        dbg_bytes_copied += copy_bpp;
      }
      else {
        Gu::trap();
      }
      sx += incx_src;
      dx += incx_dst;
    }
    sx = 0;
    dx = 0;
    sy += incy_src;
    dy += incy_dst;
  }
}
void* Image::pixel(int32_t x, int32_t y) {
  Assert(_data != nullptr);
  Assert(x >= 0, std::to_string(x) + " " + std::to_string(_width));
  Assert(y >= 0, std::to_string(y) + " " + std::to_string(_height));
  Assert(x < _width, std::to_string(x) + " " + std::to_string(_width));
  Assert(y < _height, std::to_string(y) + " " + std::to_string(_height));

  size_t off = (y * _width + x) * _format->bpp();  // vofftos((size_t)x, (size_t)y, (size_t)_width);
  char* dg = (char*)_data.get();
  void* pt = (void*)(dg + off);  // unsafe cast
  return pt;
}

#pragma endregion
#pragma region Shader

Shader::Shader(path_t vert_loc, path_t geom_loc, path_t frag_loc) {
  _name = vert_loc.filename() + "_" + (geom_loc.empty() ? "" : (geom_loc.filename() + "_")) + frag_loc.filename();
  _state = ShaderLoadState::CompilingShaders;
  _glId = glCreateProgram();
  Assert(Gu::exists(vert_loc));
  Assert(Gu::exists(frag_loc));

  _vert_src = processSource(vert_loc, ShaderStage::Vertex);
  auto vert = compileShader(GL_VERTEX_SHADER, _vert_src);
  if (vert == 0) {
    _state = ShaderLoadState::Failed;
    Gu::debugBreak();
    return;
  }
  glAttachShader(_glId, vert);

  _frag_src = processSource(frag_loc, ShaderStage::Fragment);
  auto frag = compileShader(GL_FRAGMENT_SHADER, _frag_src);
  if (frag == 0) {
    _state = ShaderLoadState::Failed;
    Gu::debugBreak();
    return;
  }
  glAttachShader(_glId, frag);

  GLuint geom = 0;
  if (!geom_loc.empty()) {
    Assert(Gu::exists(geom_loc));
    _geom_src = processSource(geom_loc, ShaderStage::Geometry);
    geom = compileShader(GL_GEOMETRY_SHADER, _geom_src);
    if (geom == 0) {
      _state = ShaderLoadState::Failed;
      Gu::debugBreak();
      return;
    }
    glAttachShader(_glId, geom);
  }

  // glBindFragDataLocation(frag, 0, "_output");

  glLinkProgram(_glId);
  GLint status = 0;
  glGetProgramiv(_glId, GL_LINK_STATUS, &status);
  if (status == GL_FALSE) {
    printSrc(_vert_src);
    LogDebug(Shader::getProgramInfoLog(_glId));
    glDeleteProgram(_glId);
    LogError("Failed to link program");
    _state = ShaderLoadState::Failed;
    Gu::debugBreak();
    return;
  }
  glDetachShader(_glId, vert);
  glDetachShader(_glId, frag);
  if (geom) {
    glDetachShader(_glId, geom);
  }

  parseUniformBlocks();
  parseUniforms();
  parseSSBOs();

  _state = ShaderLoadState::Success;
}

Shader::~Shader() {
  glDeleteProgram(_glId);
}
void Shader::bind() {
  if (_state == ShaderLoadState::Success) {
    glUseProgram(_glId);
  }
}
void Shader::unbind() {
  glUseProgram(0);
}
void Shader::parseUniforms() {
  int u_count = 0;
  glGetProgramiv(_glId, GL_ACTIVE_UNIFORMS, &u_count);
  CheckErrorsRt();
  for (auto i = 0; i < u_count; i++) {
    GLenum u_type;
    GLsizei u_size = 0;
    string_t u_name = "DEADDEADDEADDEADDEADDEADDEADDEADDEADDEADDEADDEADDEADDEADDEADDEAD";
    int u_name_len = 0;

    glGetActiveUniform(_glId, i, u_name.length(), &u_name_len, &u_size, &u_type, u_name.data());
    CheckErrorsRt();
    u_name = u_name.substr(0, u_name_len);
    // glGetActiveUniformName(_glId, i,  );
    // CheckErrorsRt();

    if (u_name.find("[") != std::string::npos) {
      // This is a unifrom block
      continue;
    }

    int location = glGetUniformLocation(_glId, u_name.c_str());
    bool is_struct;
    if (location < 0) {
      if (u_name.find('.') == std::string::npos) {
        Raise("Active uniform '" + u_name + "' had an invalid location and was not a structure variable (e.g. Struct._value).");
      }
    }
    else {
      _vars.insert(std::make_pair(u_name, std::make_unique<Uniform>(u_name, location, u_size, u_type)));
      if (Gu::config()->Log_Shader_Details_Verbose) {
        LogDebug(_name + ": Uniform name:" + u_name + " size:" + u_size);
      }
    }
  }
}
void Shader::parseUniformBlocks() {
  int u_block_count = 0;
  glGetProgramiv(_glId, GL_ACTIVE_UNIFORM_BLOCKS, &u_block_count);
  CheckErrorsRt();
  for (auto blockIndex = 0; blockIndex < u_block_count; blockIndex++) {
    int blockSize = 0;
    glGetActiveUniformBlockiv(_glId, blockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize);
    CheckErrorsRt();

    std::string u_name = "DEADDEADDEADDEADDEADDEADDEADDEADDEADDEADDEADDEADDEADDEADDEADDEAD";
    int u_name_len = 0;
    glGetActiveUniformBlockName(_glId, blockIndex, u_name.length(), &u_name_len, u_name.data());
    CheckErrorsRt();
    u_name = u_name.substr(0, u_name_len);

    GLuint blockBinding = _nextBufferBindingIndex++;

    // these are all the same.
    auto blockidx_test2 = glGetProgramResourceIndex(_glId, GL_UNIFORM_BLOCK, u_name.c_str());
    GLuint blockidx_test = glGetUniformBlockIndex(_glId, u_name.c_str());
    Assert(blockidx_test == blockIndex);
    Assert(blockidx_test2 == blockIndex);

    glUniformBlockBinding(_glId, blockIndex, blockBinding);

    if (Gu::config()->Log_Shader_Details_Verbose) {
      LogDebug(_name + ": Uniform block: " + u_name + " index: " + blockIndex + " binding: " + blockBinding + " size(B): " + blockSize);
    }

    _vars.insert(std::make_pair(u_name, std::make_unique<BufferBlock>(u_name, blockIndex, blockBinding, blockSize, GL_UNIFORM_BUFFER)));
  }
  checkDupeBindings();
}
void Shader::parseSSBOs() {
  CheckErrorsRt();
  int ssb_count;
  glGetProgramInterfaceiv(_glId, GL_SHADER_STORAGE_BLOCK, GL_ACTIVE_RESOURCES, &ssb_count);

  for (int blockIndex = 0; blockIndex < ssb_count; blockIndex++) {
    string_t blockName = "DEADDEADDEADDEADDEADDEADDEADDEADDEADDEADDEADDEADDEADDEADDEADDEAD";
    int u_name_len = 0;
    glGetProgramResourceName(_glId, GL_SHADER_STORAGE_BLOCK, blockIndex, blockName.length(), &u_name_len, blockName.data());
    blockName = blockName.substr(0, u_name_len);
    if (glGetError() == GL_NO_ERROR) {
      auto block_index = glGetProgramResourceIndex(_glId, GL_SHADER_STORAGE_BLOCK, blockName.c_str());
      Assert(block_index >= 0);

      GLuint blockBinding = _nextBufferBindingIndex++;
      glShaderStorageBlockBinding(_glId, block_index, blockBinding);
      _vars.insert(std::make_pair(blockName, std::make_unique<BufferBlock>(blockName, 0, blockBinding, true, GL_SHADER_STORAGE_BUFFER)));

      if (Gu::config()->Log_Shader_Details_Verbose) {
        LogDebug(_name + ": Shader Storage Block: " + blockName + " index: " + blockIndex + " binding: " + blockBinding);
      }
    }
    else {
      CheckErrorsRt();
      break;
    }
  }
  CheckErrorsRt();
  checkDupeBindings();
}
void Shader::checkDupeBindings() {
  // check duplicate binding indexes for blocks
  for (auto it1 = _vars.begin(); it1 != _vars.end(); it1++) {
    auto it2 = it1;
    std::advance(it2, 1);
    for (; it2 != _vars.end(); it2++) {
      auto ub0 = dynamic_cast<BufferBlock*>(it1->second.get());
      auto ub1 = dynamic_cast<BufferBlock*>(it2->second.get());

      if (ub0 != nullptr && ub1 != nullptr && ub0 != ub1) {
        if (ub0->_bindingPoint == ub1->_bindingPoint) {
          LogError("Duplicate Uniform buffer binding index " + ub0->_bindingPoint + " for " + ub0->_name + " and " + ub1->_name + "");
          _state = ShaderLoadState::Failed;
          Gu::debugBreak();
        }
      }
    }
  }
}
std::string Shader::debugFormatSrc(std::vector<std::string> lines) {
  std::ostringstream st;
  int iLine = 1;
  st << Log::cc_color(Log::CC_CYAN, true);
  for (auto line : lines) {
    st << std::internal << std::setfill('0') << std::setw(5) << iLine++;
    st << ": " << line << std::endl;
  }
  st << Log::cc_reset();
  return st.str();
}
std::string Shader::getShaderInfoLog(GLuint prog) {
  GLint maxlen = 0;
  glGetShaderiv(prog, GL_INFO_LOG_LENGTH, &maxlen);
  std::vector<char> buf(maxlen);
  GLint outlen = 0;
  glGetShaderInfoLog(prog, maxlen, &outlen, buf.data());
  std::string info = std::string(buf.begin(), buf.begin() + outlen);
  return info;
}
std::string Shader::getProgramInfoLog(GLuint prog) {
  GLint maxlen = 0;
  glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &maxlen);
  std::vector<char> buf(maxlen);
  GLint outlen = 0;
  glGetProgramInfoLog(prog, maxlen, &outlen, buf.data());
  std::string info = std::string(buf.begin(), buf.begin() + outlen);
  return info;
}
Shader::ShaderMeta::ShaderMeta() {
  LogInfo("Loading shader metadata");

  std::string c_shared = std::string(_TO_STRING(SHADER_SHARED));

  auto structs_raw = Gu::readFile(Gu::relpath("../src/gpu_structs.h"));

  auto da = structs_raw.find(c_shared);
  Assert(da != string_t::npos);
  da += c_shared.length();
  auto db = structs_raw.find(c_shared, da);
  Assert(db != string_t::npos);
  auto strStructs = structs_raw.substr(da, db - da);

  strStructs = StringUtil::replaceAll(strStructs, str("GPU_STRUCT"), str("struct"));

  _globals = Gu::readFile(Gu::assetpath("shader/globals.glsl"));
  _globals.append("\n");
  _globals.append(strStructs);
  _globals.append("\n");

  // struct inputs
  auto matches = StringUtil::submatches(strStructs, "struct\\s+([a-zA-Z0-9_]+)");
  for (auto& m : matches) {
    _structs.push_back(m);
  }
}
std::vector<std::string> Shader::processSource(path_t& loc, ShaderStage stage) {
  if (_meta == nullptr) {
    _meta = std::make_unique<ShaderMeta>();
  }
  Assert(_meta);

  // LogDebug("Loading " + loc.string());
  auto src_raw = Gu::readFile(loc);

  // find inputs.
  Assert(_meta);
  std::string str_buffers = "";
  auto src_nocoms = StringUtil::strip(src_raw, std::string("/*"), std::string("*/"), true, true);
  src_nocoms = StringUtil::strip(src_nocoms, std::string("//"), std::string("\n"), true, true);
  src_nocoms = StringUtil::strip(src_nocoms, std::string("\""), std::string("\""), true, true);
  src_nocoms = StringUtil::strip(src_nocoms, std::string("\'"), std::string("\'"), true, true);

  // if (false) {
  //   LogDebug("========");
  //   printSrc(StringUtil::split(src_raw, '\n'));
  //   LogDebug("========");
  //   printSrc(StringUtil::split(src_nocoms, '\n'));
  //   LogDebug("========");
  // }
  int binding = 0;

  for (auto& str_struct : _meta->_structs) {
    std::string ufName = "_uf" + str_struct;
    std::string buftype = "uniform";
    std::string arrsuffix = "";
    std::string std = "std140";
    std::string location = "binding";

    auto ufnpos = src_raw.find(ufName);
    if (ufnpos != std::string::npos) {
      //**ugh buffer binding location mismatch.. fix this ..
      if (src_raw.at(ufnpos + ufName.length()) == '[') {
        buftype = "buffer";  // ssbo
        arrsuffix = "[]";
        std = "std430";
      }
      if (src_raw.find(ufName) != std::string::npos) {
        // str_buffers.append("layout(" + std + ", " + location + " = " + std::to_string(binding) + ") " + buftype + " " + ufName + "_Block {\n");
        str_buffers.append("layout(" + std + ") " + buftype + " " + ufName + "_Block {\n");
        str_buffers.append("  " + str_struct + " " + ufName + arrsuffix + ";\n");
        str_buffers.append("};\n");
      }
    }
  }

  auto src = _meta->_globals + str_buffers + src_raw;
  auto lines = StringUtil::split(src, '\n');

  if (Gu::config()->Log_Shader_Processed_Source) {
    printSrc(lines);
  }

  return lines;
}
GLuint Shader::compileShader(GLenum type, std::vector<std::string>& src_lines) {
  GLuint shader = glCreateShader(type);
  // combine into string again..
  std::ostringstream imploded;
  std::copy(src_lines.begin(), src_lines.end(), std::ostream_iterator<std::string>(imploded, "\n"));
  std::string stt = imploded.str() + '\0';
  const char* ccc = stt.c_str();
  glShaderSource(shader, 1, &ccc, NULL);
  glCompileShader(shader);
  CheckErrorsDbg();
  GLint status = 0;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
  if (status == GL_FALSE) {
    std::string info = Shader::getShaderInfoLog(shader);
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
    std::string exinfo = "Failed to compile " + st + " shader \n";
    exinfo += "\n";
    exinfo += debugFormatSrc(src_lines);
    exinfo += "\n";
    exinfo += info;
    LogError(exinfo);
    return 0;
    // Raise(exinfo);
  }
  return shader;
}
void Shader::printSrc(std::vector<std::string> lines) {
  Log::println(debugFormatSrc(lines), Log::CC_CYAN);
}
void Shader::bindBlock(const string_t&& name, GpuBuffer* g) {
  auto v = getVar(name);
  bindBlock(dynamic_cast<BufferBlock*>(v), g);
}
void Shader::beginRender() {
  bind();
}
void Shader::endRender() {
  for (auto ite = _vars.begin(); ite != _vars.end(); ite++) {
    if (!ite->second->_hasBeenBound) {
      if (Gu::config()->Log_Shader_Bind_Warnings) {
        LogWarn(_name + ": Variable '" + ite->second->_name + "' was not bound before render.");
      }
    }
    ite->second->_hasBeenBound = false;
  }
}
void Shader::bindBlock(BufferBlock* u, GpuBuffer* b) {
  Assert(u != nullptr);
  Assert(b != nullptr);
  glBindBufferBase(u->_buftype, u->_bindingPoint, b->glId());
  CheckErrorsDbg();
  // glBindBufferRange

  u->_hasBeenBound = true;
}
Shader::ShaderVar* Shader::getVar(const str& name) {
  auto it = _vars.find(name);
  Assert(it != _vars.end(), "Could not find shader var name:" + name);
  return it->second.get();
}
void Shader::setTextureUf(Texture* tex, GLuint channel, string_t loc) {
  setTextureUf(tex->glId(), channel, loc, tex->sampler()->glId());
}
void Shader::setTextureUf(TextureArray* tex, GLuint channel, string_t loc) {
  setTextureUf(tex->glId(), channel, loc, tex->sampler()->glId());
}
void Shader::setTextureUf(GLuint glid, GLuint channel, string_t loc, GLuint samplerid) {
  auto sv = getVar(loc);
  auto tex_loc1 = glGetUniformLocation(_glId, loc.c_str());
  glProgramUniform1i(_glId, tex_loc1, channel);
  glBindTextureUnit(channel, glid);
  glBindSampler(channel, samplerid);
  sv->_hasBeenBound = true;
  CheckErrorsDbg();
}

#pragma endregion
#pragma region VertexArray

VertexArray::VertexArray() {
  glCreateVertexArrays(1, &_glId);
}
VertexArray::~VertexArray() {
  glDeleteVertexArrays(1, &_glId);
}
void VertexArray::bind() {
  glBindVertexArray(_glId);
}
void VertexArray::unbind() {
  glBindVertexArray(0);
}
void VertexArray::setAttrib(int attr_idx, int attr_compsize, GLenum attr_datatype, size_t attr_offset, bool attr_inttype, int binding_index, GpuBuffer* gbuf, size_t stride) {
  glEnableVertexArrayAttrib(_glId, attr_idx);
  if (attr_inttype) {
    glVertexArrayAttribIFormat(_glId, attr_idx, attr_compsize, attr_datatype, attr_offset);
  }
  else {
    glVertexArrayAttribFormat(_glId, attr_idx, attr_compsize, attr_datatype, GL_FALSE, attr_offset);
  }
  glVertexArrayAttribBinding(_glId, attr_idx, binding_index);
  glVertexArrayVertexBuffer(_glId, binding_index, gbuf->glId(), 0, stride);
  // glVertexArrayElementBuffer(_vao->glId(), _ibo->glId());
}

#pragma endregion
#pragma region Mesh

Mesh::Mesh() {}
Mesh::Mesh(const string_t& name, PrimType pt, sptr<GpuBuffer> vertexBuffer, sptr<GpuBuffer> indexBuffer, sptr<GpuBuffer>* faceData, bool computeBoundBox) {
  _name = name;
  _primtype = pt;
  _vbos.push_back(vertexBuffer);
  _ibo = indexBuffer;
  // FaceData = faceData;

  //  Gu::context()->vaos();

  if (computeBoundBox) {
    // ComputeBoundBox();
  }
}

Mesh::~Mesh() {}
void Mesh::createNew() {
  //         CheckErrorsDbg();
  //       auto vao = std::make_unique<VertexArray>();
  //       var vao = new VertexArrayObject(this.Name + "-VAO");
  //       vao.Bind();
  //
  //       GpuDataFormat fmtLast = null;
  //
  //       foreach (var vb in _vertexBuffers)
  //       {
  //         vb.Bind();
  //         Gu.Assert(vb.Format != null);
  //         vb.Format.BindVertexAttribs(fmtLast);
  //         fmtLast = vb.Format;
  //       }
  //       if (_indexBuffer != null)
  //       {
  //         _indexBuffer.Bind();
  //         //_indexBuffer.Format.BindAttribs(null);
  //       }
  //
  //         CheckErrorsDbg();
  //       vao.Unbind();
  //
  //       GPUBuffer.UnbindBuffer(BufferTarget.ArrayBuffer);
  //       GPUBuffer.UnbindBuffer(BufferTarget.ElementArrayBuffer);
  //
  //       return vao;
}
#pragma endregion
#pragma region Mesh

Material::Material() {}
Material::~Material() {}

#pragma endregion
#pragma region GpuBuffer

GpuBuffer::GpuBuffer() {
  glCreateBuffers(1, &_glId);
  CheckErrorsDbg();
}
GpuBuffer::GpuBuffer(size_t size, const void* data, uint32_t flags) : GpuBuffer() {
  glNamedBufferData(_glId, size, data, flags);
  CheckErrorsDbg();
}
GpuBuffer::~GpuBuffer() {
  glDeleteBuffers(1, &_glId);
}
void GpuBuffer::copyToGpu(size_t size, const void* data, size_t offset) {
  // glNamedBufferSubData(_glId, offset, size, data);
  glNamedBufferData(_glId, size, data, GL_DYNAMIC_DRAW);
  CheckErrorsDbg();
}

#pragma endregion
#pragma region sampler
Sampler::Sampler() {
  glCreateSamplers(1, &_glId);
}
Sampler::Sampler(GLenum mag, GLenum min, GLenum mip, GLenum wrap, glm::vec4 borderColor, GLenum compare) : Sampler() {
  CheckErrorsDbg();
  assert(mag == GL_LINEAR || mag == GL_NEAREST);
  assert(min == GL_LINEAR || min == GL_NEAREST);
  assert(mip == GL_LINEAR || mip == GL_NEAREST);

  _mag = mag;
  _min = min;
  _mip = mip;
  _wrap = wrap;

  GLint minFilter = GL_NONE;
  if (min == GL_LINEAR) {
    minFilter = mip == GL_LINEAR ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR_MIPMAP_NEAREST;
  }
  else {
    minFilter = mip == GL_LINEAR ? GL_NEAREST_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_NEAREST;
  }

  glSamplerParameteri(_glId, GL_TEXTURE_MIN_FILTER, minFilter);
  glSamplerParameteri(_glId, GL_TEXTURE_MAG_FILTER, mag);
  glSamplerParameteri(_glId, GL_TEXTURE_WRAP_S, wrap);
  glSamplerParameteri(_glId, GL_TEXTURE_WRAP_T, wrap);
  glSamplerParameteri(_glId, GL_TEXTURE_WRAP_R, wrap);
  glSamplerParameterfv(_glId, GL_TEXTURE_BORDER_COLOR, (GLfloat*)&borderColor);
  glSamplerParameteri(_glId, GL_TEXTURE_COMPARE_MODE, compare == GL_NONE ? GL_NONE : GL_COMPARE_R_TO_TEXTURE);
  glSamplerParameteri(_glId, GL_TEXTURE_COMPARE_FUNC, compare != GL_NONE ? compare : GL_LEQUAL);
  CheckErrorsDbg();
}
Sampler::~Sampler() {
  glDeleteSamplers(1, &_glId);
}
#pragma endregion
#pragma region TextureBase

TextureBase::TextureBase(int w, int h, ImageFormat* fmt, bool mipmaps, GLenum type) {
  _width = w;
  _height = h;
  _type = type;
  _format = fmt;
  _levels = mipmaps ? Texture::calcMipLevels(_width, _height) : 1;
  Assert(_levels > 0);
  glCreateTextures(_type, 1, &_glId);

  glTextureParameteri(_glId, GL_TEXTURE_BASE_LEVEL, 0);
  glTextureParameteri(_glId, GL_TEXTURE_MAX_LEVEL, _levels - 1);

  _sampler = std::make_unique<Sampler>(GL_LINEAR, GL_LINEAR, GL_LINEAR);

  CheckErrorsDbg();
}
TextureBase::~TextureBase() {
  glDeleteTextures(1, &_glId);
}
void TextureBase::setStorageMode() {
  if (_format->bpp() % 8 == 0) {
    glPixelStorei(GL_UNPACK_ALIGNMENT, 8);
  }
  if (_format->bpp() % 4 == 0) {
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
  }
  if (_format->bpp() % 2 == 0) {
    glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
  }
  else {
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  }
}
// void TextureBase::bind(int32_t channel) {
//   glBindTextureUnit(channel, _glId);
// }
// void TextureBase::unbind(int32_t channel) {
//   glBindTextureUnit(channel, 0);
// }
int TextureBase::calcMipLevels(int w, int h, int minwh) {
  float mwidth = w;
  float mheight = h;
  int mlevels = 0;
  while (mwidth > (float)minwh && mheight > (float)minwh) {
    mlevels++;
    mwidth /= 2.0f;
    mheight /= 2.0f;
  }
  return mlevels;
}

#pragma endregion
#pragma region Texture

Texture::Texture(int w, int h, ImageFormat* fmt, bool mipmaps) : TextureBase(w, h, fmt, mipmaps, GL_TEXTURE_2D) {}
Texture::Texture(Image* img, bool mipmaps) : Texture(img->width(), img->height(), img->format(), mipmaps) {
  Assert(img);
  copyToGpu(img->width(), img->height(), img->data());
  CheckErrorsDbg();
}
Texture::~Texture() {}
uptr<Texture> Texture::singlePixel(vec4 color) {
  uptr<Texture> ret = std::make_unique<Texture>(1, 1, Gu::imageFormat(ImageFormatType::RGBA8), false);

  unsigned char pix[4];
  pix[0] = 255, pix[1] = 0, pix[2] = 255, pix[3] = 255;
  ret->copyToGpu(1, 1, (void*)pix);
  return ret;
}
void Texture::copyToGpu(int w, int h, const void* data) {
  copyToGpu(0, 0, w, h, data);
}
void Texture::copyToGpu(int x, int y, int w, int h, const void* data) {
  setStorageMode();

  glTextureStorage2D(_glId, _levels, _format->glInternalFormat(), w, h);
  glTextureSubImage2D(_glId, 0, 0, 0, w, h, _format->glFormat(), _format->glDatatype(), data);
  if (_levels > 1) {
    glGenerateTextureMipmap(_glId);
    CheckErrorsRt();
  }
}

#pragma endregion
#pragma region TextureArray

TextureArray::TextureArray(int w, int h, ImageFormat* fmt, bool mipmaps, int count) : TextureBase(w, h, fmt, mipmaps, GL_TEXTURE_2D_ARRAY) {
  _count = count;
  glTextureStorage3D(_glId, _levels, _format->glInternalFormat(), (GLsizei)_width, (GLsizei)_height, (GLsizei)_count);
  CheckErrorsRt();
}
TextureArray::TextureArray(const std::vector<uptr<Image>>& imgs, bool mipmaps) : TextureArray(imgs[0]->width(), imgs[0]->height(), imgs[0]->format(), mipmaps, (int)imgs.size()) {
  for (int index = 0; index < imgs.size(); index++) {
    imgs.at(index)->check();
    Assert(imgs.at(index)->format() == _format);
    Assert(imgs.at(index)->width() == _width, std::string("") + imgs.at(index)->width() + " " + _width);
    Assert(imgs.at(index)->height() == _height, std::string("") + imgs.at(index)->height() + " " + _height);
    copyToGpu(index, imgs.at(index)->data());
  }

  if (_levels > 1) {
    glGenerateTextureMipmap(_glId);
    CheckErrorsRt();
  }
}
TextureArray::~TextureArray() {
  glDeleteTextures(1, &_glId);
}
uptr<TextureArray> TextureArray::test() {
  std::vector<uptr<Image>> images;
  images.push_back(std::move(Image::from_file(Gu::relpath("../data/tex/Церковь Спаса на Крови.jpg").string())));
  images.push_back(std::move(Image::from_file(Gu::relpath("../data/tex/stpeter.jpg").string())));
  images.push_back(std::move(Image::from_file(Gu::relpath("../data/tex/дэвид.jpg").string())));
  images.push_back(std::move(Image::from_file(Gu::relpath("../data/tex/Сикстинская капелла.jpg").string())));
  TextureArray::conform(images);
  return std::make_unique<TextureArray>(images, true);
}
uptr<TextureArray> TextureArray::singlePixel(const vec4& color, int count) {
  uptr<TextureArray> ret = std::make_unique<TextureArray>(1, 1, Gu::imageFormat(ImageFormatType::RGBA8), false, count);

  uint8_t pix[4];
  pix[0] = (uint8_t)glm::round(glm::clamp(color.r, 0.0f, 1.0f) * 255.0f);
  pix[1] = (uint8_t)glm::round(glm::clamp(color.g, 0.0f, 1.0f) * 255.0f);
  pix[2] = (uint8_t)glm::round(glm::clamp(color.b, 0.0f, 1.0f) * 255.0f);
  pix[3] = (uint8_t)glm::round(glm::clamp(color.a, 0.0f, 1.0f) * 255.0f);
  for (int i = 0; i < count; i++) {
    ret->copyToGpu(i, (void*)pix);
  }
  return ret;
}
void TextureArray::copyToGpu(int index, const void* data) {
  copyToGpu(0, 0, _width, _height, index, data);
}
void TextureArray::copyToGpu(int x, int y, int w, int h, int index, const void* data) {
  Assert(index < _count);
  setStorageMode();
  // glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, format, width, height, num_layers, ...);
  // glTextureSubImage3D(_glId, 0, x, y, index, w, h, 1, _format->glFormat(), _format->glDatatype(), data);
  // For two-dimensional array textures, the z index refers to the slice index.
  glTextureSubImage3D(_glId, 0, x, y, index, w, h, 1, _format->glFormat(), _format->glDatatype(), data);
  CheckErrorsDbg();
}
void TextureArray::conform(std::vector<uptr<Image>>& imgs) {
  // scale images to first tex
  if (imgs.size() > 1) {
    for (auto i = 1; i < imgs.size(); i++) {
      auto sw = (float)imgs[0]->width() / (float)imgs[i]->width();
      auto sh = (float)imgs[0]->height() / (float)imgs[i]->height();
      auto ss = glm::min(sw, sh);
      if (ss < 1.0f) {
        auto up = Image::scale(imgs[i].get(), ss, imgs[0]->format());
        auto up2 = Image::crop(up.get(), box2i(ivec2(0, 0), ivec2(imgs[0]->width(), imgs[0]->height())));
        imgs[i].swap(up2);
      }
      else if (ss != 1.0f) {
        auto up2 = Image::crop(imgs[i].get(), box2i(ivec2(0, 0), ivec2(imgs[0]->width(), imgs[0]->height())));
        imgs[i].swap(up2);
      }
    }
  }
}

#pragma endregion
#pragma region Framebuffer

Framebuffer::Framebuffer() {
  glGenFramebuffers(1, &_glId);
}
Framebuffer ::~Framebuffer() {
  glDeleteFramebuffers(1, &_glId);
}

#pragma endregion
#pragma region Input

Input::Input(Window* w) {
  _window = w;
  updateCursor();
  _mouseLast = _mouse;
}
Input::~Input() {}
void Input::addKeyEvent(int32_t key, bool press) {
  KeyEvent k;
  k._key = key;
  k._press = press;
  _key_events.push_back(k);
}
Input::KeyState Input::state(int key) {
  auto it = _keys.find(key);
  if (it != _keys.end()) {
    return it->second.get()->state();
  }
  return KeyState::Up;
}
bool Input::press(int key) {
  return state(key) == KeyState::Press;
}
bool Input::pressOrDown(int key) {
  return state(key) == KeyState::Press || state(key) == KeyState::Down;
}
void Input::updateCursor() {
  _mouseLast = _mouse;
  double mx = 0, my = 0;
  glfwGetCursorPos(_window->glfwWindow(), &mx, &my);
  _mouse = vec2((float)mx, (float)my);
}
void Input::Key::upateKey() {
  auto state = _state;
  if (state == KeyState::Up && _press) {
    state = KeyState::Press;
  }
  else if (state == KeyState::Press && _press) {
    state = KeyState::Down;
  }
  else if (state == KeyState::Press && !_press) {
    state = KeyState::Release;
  }
  else if (state == KeyState::Down && !_press) {
    state = KeyState::Release;
  }
  else if (state == KeyState::Release && _press) {
    state = KeyState::Press;
  }
  else if (state == KeyState::Release && !_press) {
    state = KeyState::Up;
  }
  _state = state;
}
void Input::update() {
  updateCursor();

  if (_key_events.size()) {
    for (auto& ke : _key_events) {
      auto it = _keys.find(ke._key);
      if (it == _keys.end()) {
        _keys.insert(std::make_pair(ke._key, std::make_unique<Key>(ke._key)));
        it = _keys.find(ke._key);
      }
      it->second.get()->press() = ke._press;
    }
    _key_events.clear();
  }

  for (auto& it1 : _keys) {
    it1.second.get()->upateKey();
  }
}

#pragma endregion
#pragma region DrawQuads

DrawQuads::DrawQuads(uint32_t maxQuads) {
  _shader = std::make_unique<Shader>(Gu::assetpath("shader/draw_quads.vs.glsl"), path_t(""), Gu::assetpath("/shader/draw_quads.fs.glsl"));

  GpuQuad q;
  q.zero();
  _quads = std::vector<GpuQuad>(maxQuads, q);

  _vbo = std::make_unique<GpuBuffer>(sizeof(GpuQuad) * _quads.size());
  CheckErrorsDbg();
  auto inds = std::vector<uint32_t>();
  for (auto i = 0; i < _quads.size(); i++) {
    inds.push_back(i * 4 + 0);
    inds.push_back(i * 4 + 1);
    inds.push_back(i * 4 + 3);
    inds.push_back(i * 4 + 0);
    inds.push_back(i * 4 + 3);
    inds.push_back(i * 4 + 2);
  }
  _ibo = std::make_unique<GpuBuffer>(sizeof(uint32_t) * inds.size(), inds.data(), GL_STATIC_DRAW);
  CheckErrorsDbg();

  GLuint v_idx = 0;
  GLuint i_idx = 1;
  GLuint x_idx = 2;
  GLuint c_idx = 3;
  GLuint vbo_binding_idx = 0;
  // GLuint ibo_binding_idx = 1;

  _vao = std::make_unique<VertexArray>();
  glEnableVertexArrayAttrib(_vao->glId(), v_idx);
  glEnableVertexArrayAttrib(_vao->glId(), i_idx);
  glEnableVertexArrayAttrib(_vao->glId(), x_idx);
  glEnableVertexArrayAttrib(_vao->glId(), c_idx);

  glVertexArrayAttribFormat(_vao->glId(), v_idx, 3, GL_FLOAT, GL_FALSE, 0);
  glVertexArrayAttribIFormat(_vao->glId(), i_idx, 1, GL_UNSIGNED_INT, sizeof(vec3));
  glVertexArrayAttribFormat(_vao->glId(), x_idx, 2, GL_FLOAT, GL_FALSE, sizeof(vec3) + sizeof(uint32_t));
  glVertexArrayAttribFormat(_vao->glId(), c_idx, 4, GL_FLOAT, GL_FALSE, sizeof(vec3) + sizeof(uint32_t) + sizeof(vec2));

  glVertexArrayAttribBinding(_vao->glId(), v_idx, vbo_binding_idx);
  glVertexArrayAttribBinding(_vao->glId(), i_idx, vbo_binding_idx);
  glVertexArrayAttribBinding(_vao->glId(), x_idx, vbo_binding_idx);
  glVertexArrayAttribBinding(_vao->glId(), c_idx, vbo_binding_idx);

  glVertexArrayVertexBuffer(_vao->glId(), vbo_binding_idx, _vbo->glId(), 0, sizeof(GpuQuadVert));
  glVertexArrayElementBuffer(_vao->glId(), _ibo->glId());

  CheckErrorsDbg();
}
DrawQuads::~DrawQuads() {}
GpuQuad* DrawQuads::getQuad() {
  if (_index < _quads.size()) {
    return &_quads[_index++];
  }
  else {
    LogError("Reached limit of quad buffer  size=(" + _quads.size() + ")");
    Gu::debugBreak();
    if (_quads.size() > 0) {
      return &_quads[_quads.size() - 1];
    }
    Raise("DrawQuads: Quads not initialized");
  }
  _dirty = true;
}
void DrawQuads::copyToGpu() {
  if (_dirty) {
    _vbo->copyToGpu(sizeof(GpuQuad) * _index, _quads.data());
    _dirty = false;
  }
}
void DrawQuads::reset() {
  if (_index != 0) {
    _dirty = true;
  }
  _index = 0;
}
void DrawQuads::draw(Camera* cam, Texture* tex) {
  copyToGpu();
  _shader->beginRender();
  _shader->setTextureUf(tex, 0, "_texture");
  _vao->bind();
  glDrawElements(GL_TRIANGLES, _index * 6, GL_UNSIGNED_INT, 0);
  _vao->unbind();
  _shader->endRender();
}
void GpuQuad::zero() {
  _verts[0]._v = vec3(0, 0, 0);
  _verts[1]._v = vec3(1, 0, 0);
  _verts[2]._v = vec3(0, 1, 0);
  _verts[3]._v = vec3(1, 1, 0);
  _verts[0]._x = vec2(0, 0);
  _verts[1]._x = vec2(1, 0);
  _verts[2]._x = vec2(0, 1);
  _verts[3]._x = vec2(1, 1);
  _verts[0]._c = vec4(1, 1, 1, 1);
  _verts[1]._c = vec4(1, 1, 1, 1);
  _verts[2]._c = vec4(1, 1, 1, 1);
  _verts[3]._c = vec4(1, 1, 1, 1);
}
void DrawQuads::testMakeQuads() {
  for (int i = 0; i < 1000; i++) {
    GpuQuad* cpy = getQuad();
    cpy->zero();

    vec3 rnd;
    rnd.x = Gu::random(-50, 50);
    rnd.y = Gu::random(-50, 50);
    rnd.z = Gu::random(-50, 50);
    cpy->translate(rnd);
    rnd.x = Gu::random(0.1f, 4.0f);
    rnd.y = Gu::random(0.1f, 4.0f);
    rnd.z = Gu::random(0.1f, 4.0f);
    cpy->scale(rnd);

    // color
    for (auto& v : cpy->_verts) {
      vec4 rnd4;
      rnd4.r = Gu::random(0.1f, 1.0f);
      rnd4.g = Gu::random(0.6f, 1.0f);
      rnd4.b = Gu::random(0.6f, 1.0f);
      rnd4.a = Gu::random(0.1f, 1.0f);
      v._c = rnd4;
    }
  }
}

#pragma endregion
#pragma region Viewport
#pragma endregion
#pragma region RenderView

RenderView::RenderView(string_t name, vec2 uv0, vec2 uv1, int sw, int sh) {
  CheckErrorsDbg();
  _name = name;
  _overlay = std::make_unique<Overlay>(this);
  _viewport = std::make_unique<Viewport>();
  setSize(uv0, uv1, sw, sh);
}
void RenderView::setSize(vec2 uv0, vec2 uv1, int sw, int sh) {
  _uv0 = uv0;
  _uv1 = uv1;
  Assert(_uv0.x < _uv1.x);
  Assert(_uv0.y < _uv1.y);
  onResize(sw, sh);
}
void RenderView::onResize(int sw, int sh) {
  updateDimensions(sw, sh);
  // Gui?.OnResize();//Gui is translated to the current FBO size in the shader.
}
void RenderView::updateDimensions(int cur_output_fbo_w, int cur_output_fbo_h) {
  //** CALLED EVERY PIPE STAGE **
  auto b = computeScaledView(_uv0, _uv1, cur_output_fbo_w, cur_output_fbo_h);

  if (b.width() <= 0 || b.height() <= 0) {
    LogInfo("Resize View " + _name + ": " + b.x() + "," + b.y() + " " + b.width() + "," + b.height());
    LogError("Resize View " + _name + " w/h was zero, setting to 1");
    Gu::debugBreak();
    if (b.width() <= 0) {
      b.width(1);
    }
    if (b.height() <= 0) {
      b.height(1);
    }
  }

  _viewport->x(b.x());
  _viewport->y(b.y());
  _viewport->width(b.width());
  _viewport->height(b.height());

  syncCamera();
}
box2i RenderView::computeScaledView(vec2 uv0, vec2 uv1, int width, int height) {
  // render views are defined as taking up a uv % of the window, this computes the w/h from the uv %
  box2i b = box2i();
  b.x((int)(glm::round(uv0.x * (float)width)));
  b.y((int)(glm::round(uv0.y * (float)height)));
  b.width((int)(glm::round((uv1.x - uv0.x) * (float)width)));
  b.height((int)(glm::round((uv1.y - uv0.y) * (float)height)));
  return b;
}
void RenderView::syncCamera() {
  // cam must always be in sync
  if (_camera != nullptr && _enabled) {
    _camera->computeView(this);
  }
}
bool RenderView::beginPipelineStage(PipelineStage* ps) {
  updateDimensions(ps->width(), ps->height());
  glViewport(_viewport->x(),
             ps->height() - _viewport->y() - _viewport->height(),  // OpenGL Y = Bottom left!!!
             _viewport->width(), _viewport->height());

  // if we have an active camera set the scissor to the camera, else set to the viewport. e.g. make "black bars"
  auto clip = getClipViewport();
  glScissor(clip->x(),
            ps->height() - clip->y() - clip->height(),  // OpenGL Y = Bottom left!!!
            clip->width(), clip->height());

  // TODO:
  //  CompileGpuData();

  return true;
}
void RenderView::endPipelineStage(PipelineStage* ps) {}
Viewport* RenderView::getClipViewport() {
  if (_camera != nullptr) {
    return _camera->viewport();
  }
  else {
    return _viewport.get();
  }
}
#pragma endregion
#pragma region Gpu

Gpu::Gpu() {
  _last = std::make_unique<GpuRenderState>();
}
void Gpu::setState(const GpuRenderState& state, bool force) {
  if (state.cullFaceEnabled != _last->cullFaceEnabled || force) {
    if (state.cullFaceEnabled) {
      glEnable(GL_CULL_FACE);
    }
    else {
      glDisable(GL_CULL_FACE);
    }
  }
  if (state.depthTestEnabled != _last->depthTestEnabled || force) {
    if (state.depthTestEnabled) {
      glEnable(GL_DEPTH_TEST);
    }
    else {
      glDisable(GL_DEPTH_TEST);
    }
  }
  if (state.scissorTestEnabled != _last->scissorTestEnabled || force) {
    if (state.scissorTestEnabled) {
      glEnable(GL_SCISSOR_TEST);
    }
    else {
      glDisable(GL_SCISSOR_TEST);
    }
  }

  if (state.cullFaceEnabled) {
    if (state.cullFaceMode != _last->cullFaceMode || force) {
      glFrontFace(state.cullFaceMode);
    }
    if (state.frontFaceDirection != _last->frontFaceDirection || force) {
      glFrontFace(state.frontFaceDirection);
    }
  }

  if (state.depthTestEnabled) {
    if (state.depthMask != _last->depthMask || force) {
      glDepthMask(state.depthMask);
    }
  }

  if (state.blendEnabled != _last->blendEnabled || state.blendFactor != _last->blendFactor || state.blendFunc != _last->blendFunc || force) {
    Assert(Gu::context() != nullptr);
    // if (Gu.Context.Renderer != null && Gu.Context.Renderer.CurrentStage != null && Gu.Context.Renderer.CurrentStage.OutputFramebuffer != null) {
    if (state.blendEnabled) {
      glEnable(GL_BLEND);
      // Blending is now controlled per-framebuffer attachment
      // Gu::context()->renderer()->.CurrentStage.OutputFramebuffer.SetBlendParams();
    }
    else {
      glDisable(GL_BLEND);
    }
  }

  *(_last.get()) = state;
}

#pragma endregion
#pragma region Frustum

Frustum::Frustum() {}
Frustum::~Frustum() {}
void Frustum::update(Camera* cam) {
  // width n/f will remain the same regardless of projection, using FOV and near plane to get the orthographic width.
  float tanfov2 = tanf(cam->fov() / 2.0f);
  float ar = (float)cam->viewport()->width() / (float)cam->viewport()->height();
  _widthNear = tanfov2 * cam->near() * 2.0f;
  _heightNear = _widthNear / ar;

  // if (cam->ProjectionMode == ProjectionMode.Orthographic)
  // {
  //   _widthFar = _widthNear;
  //   _heightFar = _heightNear;
  // }
  // else if (cam->ProjectionMode == ProjectionMode.Perspective)
  // {
  _widthFar = tanfov2 * cam->far() * 2.0f;
  _heightFar = _widthFar / ar;
  // }
  // else
  // {
  //   Gu.BRThrowNotImplementedException();
  // }

  _nearCenter = cam->world_pos() + cam->forward() * cam->near();
  _farCenter = cam->world_pos() + cam->forward() * cam->far();
  _nearTopLeft = _nearCenter - cam->right() * _widthNear * 0.5f + cam->up() * _heightNear * 0.5f;
  _farTopLeft = _farCenter - cam->right() * _widthFar * 0.5f + cam->up() * _heightFar * 0.5f;

  constructPointsAndPlanes(_farCenter, _nearCenter, cam->up(), cam->right(), _widthNear * 0.5f, _widthFar * 0.5f, _heightNear * 0.5f, _heightFar * 0.5f);
}
void Frustum::constructPointsAndPlanes(vec3 farCenter, vec3 nearCenter, vec3 upVec, vec3 rightVec, float w_near_2, float w_far_2, float h_near_2, float h_far_2) {
  _points[fpt_nbl] = (nearCenter - (upVec * h_near_2) - (rightVec * w_near_2));
  _points[fpt_fbl] = (farCenter - (upVec * h_far_2) - (rightVec * w_far_2));

  _points[fpt_nbr] = (nearCenter - (upVec * h_near_2) + (rightVec * w_near_2));
  _points[fpt_fbr] = (farCenter - (upVec * h_far_2) + (rightVec * w_far_2));

  _points[fpt_ntl] = (nearCenter + (upVec * h_near_2) - (rightVec * w_near_2));
  _points[fpt_ftl] = (farCenter + (upVec * h_far_2) - (rightVec * w_far_2));

  _points[fpt_ntr] = (nearCenter + (upVec * h_near_2) + (rightVec * w_near_2));
  _points[fpt_ftr] = (farCenter + (upVec * h_far_2) + (rightVec * w_far_2));

  // - Construct AA bound box
  _boundBox.genReset();
  //_boundBox._min = -vec3(FLT_MAX,FLT_MAX,FLT_MAX);
  //_boundBox._max = vec3(FLT_MAX,FLT_MAX,FLT_MAX);

  for (int i = 0; i < 8; ++i) {
    _boundBox.genExpandByPoint(_points[i]);
    //_boundBox._min = glm::min(_boundBox._min, _points[i]);
    //_boundBox._max = glm::min(_boundBox._max, _points[i]);
  }
  // TODO: Optimize:
  //         1) we don't use the fourth value of the QuadPlane4 at all
  //         2) QuadPLane4 calculates a TBN basis.  We don't need that.
  //   1   2
  //
  //   3   4
  //
  //  - Construct so that the normals are facing into the frustum  - Checked all is good
  _planes[fp_near] = plane(_points[fpt_ntl], _points[fpt_ntr], _points[fpt_nbl]);
  _planes[fp_far] = plane(_points[fpt_ftr], _points[fpt_ftl], _points[fpt_fbr]);

  _planes[fp_left] = plane(_points[fpt_ftl], _points[fpt_ntl], _points[fpt_fbl]);
  _planes[fp_right] = plane(_points[fpt_ntr], _points[fpt_ftr], _points[fpt_nbr]);

  _planes[fp_top] = plane(_points[fpt_ntr], _points[fpt_ntl], _points[fpt_ftr]);
  _planes[fp_bottom] = plane(_points[fpt_fbr], _points[fpt_fbl], _points[fpt_nbr]);
}
bool Frustum::hasBox(const box3& pCube) {
  vec3 min, max;
  float d1, d2;
  pCube.validate();
  // if (!pCube-(false, false))
  // {
  //   Gu.LogErrorCycle("Box was invalid");
  //   Gu.DebugBreak();
  //   return false;
  // }

  for (int i = 0; i < 6; ++i) {
    min = pCube._min;
    max = pCube._max;

    //  - Calculate the negative and positive vertex
    if (_planes[i].n.x < 0) {
      min.x = pCube._max.x;
      max.x = pCube._min.x;
    }

    if (_planes[i].n.y < 0) {
      min.y = pCube._max.y;
      max.y = pCube._min.y;
    }

    if (_planes[i].n.z < 0) {
      min.z = pCube._max.z;
      max.z = pCube._min.z;
    }

    d1 = _planes[i].dist(max);
    d2 = _planes[i].dist(min);

    if (d1 < 0.0f && d2 < 0.0f) {
      return false;
    }
    // if(d2< 0.0f)
    // ret = true; // Currently we intersect the frustum.  Keep checking the rest of the planes to see if we're outside.
  }
  return true;
}

#pragma endregion
#pragma region Camera

Camera::Camera(string_t&& name) : Bobj(std::move(name)) {
  _computedViewport = std::make_unique<Viewport>();
  _frustum = std::make_unique<Frustum>();
}
void Camera::updateViewport(int width, int height) {
  _fov = glm::clamp(_fov, 0.1f, 179.9f);
  _proj = glm::perspectiveFov(glm::radians(_fov), (float)width, (float)height, 1.0f, _far);
}
void Camera::update(double dt, mat4* parent) {
  Bobj::update(dt, parent);
  _frustum->update(this);
  _view = _world;
}
void Camera::computeView(RenderView* rv) {
  // todo: scale camera to dimensions of view with 'bars'
  _computedViewport->x(rv->viewport()->x());
  _computedViewport->y(rv->viewport()->y());
  _computedViewport->width(rv->viewport()->width());
  _computedViewport->height(rv->viewport()->height());
}

#pragma endregion
#pragma region Bobj

Bobj::Bobj(string_t&& name, b2_objdata* data) {
  _id = Gu::genId();
  _name = name;
  _data = data;
}
Bobj::~Bobj() {
  // msg("destroying " + _name);
}
void Bobj::update(double dt, mat4* parent) {
  // phy
  if (Gu::fuzzyNotZero((double)glm::dot(_vel, _vel), FUZZY_ZERO_EPSILON * dt)) {
    _pos += _vel * (float)dt;
  }
  else {
    _vel = glm::vec3(0);
  }

  if (_onUpdate) {
    _onUpdate(this, dt);
  }

  // compile mat
  _world = parent ? *parent : mat4(1);
  _world = glm::scale(_world, _scl);
  _world = _world * glm::mat4_cast(_rot);
  _world = glm::translate(_world, _pos);

  // basis
  _right = glm::normalize(vec3(vec4(1, 0, 0, 1) * _world));
  _up = glm::normalize(vec3(vec4(0, 1, 0, 1) * _world));
  _forward = glm::normalize(vec3(vec4(0, 0, 1, 1) * _world));

  for (auto& comp : _components) {
    comp->update(this, dt);
  }

  for (auto& child : _children) {
    if (child->visible()) {
      child->update(dt, &_world);
    }
  }

  // create gpuobj
  if (_data != nullptr) {
    // animation
    b2_framedata* fr = nullptr;
    if (_action != nullptr) {
      _atime += dt * _aspeed;
      auto f1 = 1.0 / ((double)_data->_fps);
      if (_atime >= f1) {
        _atime = fmod(_atime, f1);
        _frameidx = (_frameidx + 1) % (int)_action->frames().size();
      }
      fr = _action->frames()[_frameidx].get();
    }
    if (fr == nullptr) {
      fr = _data->actions()[0]->frames()[0].get();
    }

    _gpuObj._mat = _world;
    Assert(fr != nullptr);
    int gpuTexID = fr->_iid - 1;
    auto tex = Gu::world()->data()->_texs[gpuTexID].get();
    float w1 = 1.0f / ((float)tex->_w);
    float h1 = 1.0f / ((float)tex->_h);
    if (_flip) {
      _gpuObj._tex = vec4((fr->_x + fr->_w) * w1, (fr->_y + fr->_h) * h1, fr->_x * w1, fr->_y * h1);
    }
    else {
      _gpuObj._tex = vec4(fr->_x * w1, (fr->_y + fr->_h) * h1, (fr->_x + fr->_w) * w1, fr->_y * h1);
    }
    _gpuObj._iid = gpuTexID;
  }
  if (Gu::fuzzyNotZero(_gpuLight._radius)) {
    _gpuLight._pos = _pos;
  }
}
double Bobj::duration(b2_actiondata* a) {
  Assert(a);
  return (1.0 / ((double)_data->_fps)) * ((double)_data->actions().size());
}
void Bobj::play(b2_actiondata* a, double time) {
  Assert(a);
  _action = a;
  _frameidx = 0;
  _atime = time * duration(a);
}
void Bobj::calcBoundBox(box3* parent) {
  if (_mesh != nullptr) {
    _boundBoxMeshAA.genReset();
    _boundBoxMeshOO = oobox3(_mesh->boundbox()._min, _mesh->boundbox()._max);
    for (int vi = 0; vi < oobox3::c_vcount; ++vi) {
      _boundBoxMeshOO.verts[vi] = vec3(_world * vec4(_boundBoxMeshOO.verts[vi], 1));
      _boundBoxMeshAA.genExpandByPoint(_boundBoxMeshOO.verts[vi]);
      _boundBox.genExpandByPoint(_boundBoxMeshOO.verts[vi]);
    }

    volumizeBoundBox(_boundBoxMeshAA);

    if (!_boundBoxMeshAA.validate(false, false)) {
      LogErrorCycle(_name + " BoundBox was invalid.");
      Gu::debugBreak();
    }
  }
  else {
    //_boundBoxMeshAA = box3::zero();
    //_boundBoxMeshOO = box3::zero();
    _boundBox.genExpandByPoint(world_pos());
  }

  // SubclassModifyBoundBox();

  // bound box can be just a point - but not invalid.
  volumizeBoundBox(_boundBox);
  if (!_boundBox.validate(false, false)) {
    LogErrorCycle(_name + "  BoundBox was invalid.");
    Gu::debugBreak();
  }

  parent->genExpandByPoint(_boundBox._min);
  parent->genExpandByPoint(_boundBox._max);
}
void Bobj::volumizeBoundBox(box3& b) {
  float epsilon = 0.01f;  // float.Epsilon
  if (b._max.y - b._min.y == 0) {
    b._max.y += epsilon;
    b._min.y -= epsilon;
  }
  if (b._max.x - b._min.x == 0) {
    b._max.x += epsilon;
    b._min.x -= epsilon;
  }
  if (b._max.z - b._min.z == 0) {
    b._max.z += epsilon;
    b._min.z -= epsilon;
  }
}
void Bobj::addChild(sptr<Bobj> ob) {
  Assert(ob->parent().expired());
  ob->parent() = getThis<Bobj>();
  _children.push_back(ob);
}
void Bobj::lookAt(const vec3&& at) {
  mat4 m;
  auto e = 0.0000001;
  auto vat = at - _pos;
  if (glm::dot(vat, vat) < e * e) {
    LogWarn("lookAt |at-pos| = 0");
    return;
  }
  if (glm::dot(_forward, _forward) < e * e) {
    LogWarn("lookAt |_forward| = 0");
    return;
  }
  vat = glm::normalize(vat);
  auto fw = _forward;
  auto dot = glm::dot(fw, vat);
  if (glm::epsilonEqual(dot, 1.0f, 0.00001f)) {
    return;
  }
  auto am = -1.0f;
  if (glm::epsilonEqual(dot, -1.0f, 0.00001f)) {
    fw = _right;
    am = 1.0f;
  }
  auto axis = glm::cross(fw, vat);
  axis = glm::normalize(axis);
  auto a = glm::acos(glm::dot(vat, fw));
  if (a < -e || a > e) {
    _rot = glm::rotate(_rot, a * am, axis);
  }
}

#pragma endregion
#pragma region InputController

InputController::InputController() {}
void InputController::update(Bobj* obj, float delta) {
  Assert(obj);
  Component::update(obj, delta);

  auto ct = Gu::context();
  if (ct == nullptr) {
    LogError("Input: Context was null.");
    return;
  }
  auto inp = ct->input();

  // aim
  float cursor_sensitivity = 0.01f;
  if (inp->pressOrDown(GLFW_MOUSE_BUTTON_LEFT)) {
    auto dm = ct->input()->mouse() - ct->input()->mouseLast();
    float rotx_ang = glm::pi<float>() / (128.0f * 3.0f);
    float roty_ang = glm::pi<float>() / (128.0f * 3.0f);
    if (Gu::fuzzyNotZero(dm.y, cursor_sensitivity)) {
      obj->rot() = glm::rotate(obj->rot(), roty_ang * dm.y, obj->right());
    }
    if (Gu::fuzzyNotZero(dm.x, cursor_sensitivity)) {
      obj->rot() = glm::rotate(obj->rot(), rotx_ang * dm.x, vec3(0, 1, 0));
      ;
    }
  }

  // strafe
  float ang = (float)(M_PI * 2.0) * rspeed() * delta;
  float fmul = 1;
  float smul = 1;
  if (inp->pressOrDown(GLFW_KEY_LEFT_SHIFT) || inp->pressOrDown(GLFW_KEY_RIGHT_SHIFT)) {
    fmul = 5.0f;
    smul = 3.0f;
  }
  if (inp->pressOrDown(GLFW_KEY_W)) {
    obj->pos() += obj->forward() * speed() * fmul * delta;
  }
  if (inp->pressOrDown(GLFW_KEY_S)) {
    obj->pos() -= obj->forward() * speed() * fmul * delta;
  }
  if (inp->pressOrDown(GLFW_KEY_A)) {
    obj->pos() += obj->right() * sspeed() * smul * delta;
    // obj->rot() = glm::rotate(obj->rot(), -ang, obj->up());
  }
  if (inp->pressOrDown(GLFW_KEY_D)) {
    obj->pos() -= obj->right() * sspeed() * smul * delta;
    // obj->rot() = glm::rotate(obj->rot(), ang, obj->up());
  }

  // other keys
  if (inp->press(GLFW_KEY_R)) {
    GLenum smp = Gu::world()->textureArray()->sampler()->min() == GL_LINEAR ? GL_NEAREST : GL_LINEAR;
    Gu::world()->textureArray()->sampler() = std::make_unique<Sampler>(smp, smp, smp) ;
  }
  if (inp->press(GLFW_KEY_L)) {
    obj->lookAt(vec3(0, 0, 0));
  }
  if (inp->press(GLFW_KEY_O)) {
    obj->pos() = vec3(0, 0, 0);
  }
}

#pragma endregion
#pragma region Scene

World::World() {
  Assert(Gu::context() != nullptr);

  _time = std::make_unique<WorldTime>();

  auto b2data_root = Gu::relpath("../data/test/");

  // load meta
  auto metapath = b2data_root / "B2MT.bin";
  if (!Gu::exists(metapath)) {
    Raise(stz"Could not find meta file: \n  " + (metapath.string()) + "\nrun (check) python script");
  }
  msg("Loading metadata " + metapath.string());
  BinaryFile bf;
  bf.loadFromDisk(metapath.string());
  _data = std::make_unique<b2_datafile>();
  _data->deserialize(&bf);

  // init world
  _gpuWorld = std::make_unique<GpuWorld>();
  _gpuWorld->_zrange = 1.0f;
  _gpuWorld->_ambient = vec4(1, 1, 1, 0.1f);
  _gpuWorld->_mtex_layers = -1;
  _gpuWorld->_mtex_color = -1;
  _gpuWorld->_mtex_depthnormal = -1;
  _gpuWorld->_mtex_w = _data->_mtex_w;
  _gpuWorld->_mtex_h = _data->_mtex_h;

  _gpuWorld->_mtex_layers = _data->_layers.size();
  for (int i = 0; i < _data->_layers.size(); i++) {
    if (_data->_layers[i] == "Color") {
      _gpuWorld->_mtex_color = i;
    }
    else if (_data->_layers[i] == "DepthNormal") {
      _gpuWorld->_mtex_depthnormal = i;
    }
    else {
      Raise("invalid layer type" + _data->_layers[i]);
    }
  }

  // create texture layers
  std::vector<uptr<Image>> images;
  for (auto& mt : _data->_texs) {
    for (auto& img : mt->_images) {
      auto imgpath = b2data_root / img;
      Assert(Gu::exists(imgpath));
      images.push_back(std::move(Image::from_file(imgpath)));
    }
  }
  _textureArray = std::make_unique<TextureArray>(images, true);

  // world root
  _root = std::make_shared<Bobj>("_root");

  // default camera
  auto cam = std::make_shared<Camera>("MainCamera");
  cam->components().push_back(std::make_unique<InputController>());
  cam->pos() = vec3(0, 0, -15);
  cam->lookAt(vec3(0, 0, 0));
  _activeCamera = cam;
  _root->addChild(cam);

  // test - objs
  // test - objs
  // test - objs
  // test - objs
  std::shared_ptr<Bobj> newob = nullptr;
  Assert(_data->_objs.size() > 0);
  auto dat = _data->_objs[0].get();
  newob = std::make_shared<Bobj>("test_object", dat);
  _root->addChild(newob);
  Assert(newob->data()->actions().size() > 0);
  newob->play(newob->data()->actions()[0].get());
  newob->onUpdate() = [](auto bobj, auto dt) {
    double incspd = 1.6;
    double spd = 2.0;
    double aspdmax = 2.0;
    double aspeedmin = 0.8;
    if (Gu::context()->input()->pressOrDown(GLFW_KEY_RIGHT)) {
      bobj->pos().x += spd * dt;
      bobj->aspeed() = glm::clamp(bobj->aspeed() + incspd * dt, aspeedmin, aspdmax);
      bobj->flip() = true;
    }
    else if (Gu::context()->input()->pressOrDown(GLFW_KEY_LEFT)) {
      bobj->pos().x -= spd * dt;
      bobj->aspeed() = glm::clamp(bobj->aspeed() + incspd * dt, aspeedmin, aspdmax);
      bobj->flip() = false;
    }
    else {
      bobj->aspeed() = glm::clamp(bobj->aspeed() - incspd * dt, aspeedmin, 999.0);
    }
  };

  std::weak_ptr<Bobj> obw = std::weak_ptr(newob);

  newob = std::make_shared<Bobj>("test_light");
  newob->lightRadius() = 10;
  newob->lightPower() = 0.75f;
  newob->lightColor() = vec3(0.99124, 0.98315f, 0.71814);
  newob->lightDir() = vec3(0, 0, 0);
  newob->onUpdate() = [obw](auto bobj, auto dt) {
    double r = 7.0;
    double s = 3;
    auto a = Gu::world()->time()->modSeconds(s) * glm::two_pi<double>();
    bobj->pos().x = (float)(glm::cos(a) * r);
    bobj->pos().y = 0;
    bobj->pos().z = (float)(glm::sin(a) * r);
    if (auto ob = obw.lock()) {
      bobj->pos() += ob->pos();
    }
  };
  _root->addChild(newob);

  // newob = std::make_shared<Bobj>("test_light2");
  // newob->lightRadius() = 10;
  // newob->lightPower() = 0.75f;
  // newob->lightColor() = vec3(0.0099713, 0.0021412, 0.9414);
  // newob->lightDir() = vec3(0, 0, 0);
  // newob->onUpdate() = [](auto bobj, auto dt) {
  //   double r = 5.0;
  //   double s = 12;
  //   auto a = Gu::world()->time()->modSeconds(s) * glm::two_pi<double>();
  //   bobj->pos().x = (float)(glm::cos(a) * r);
  //   bobj->pos().y = (float)(glm::sin(a) * r);
  //   bobj->pos().z = 0;
  //    if (auto ob = obw.lock()) {
  //    bobj->pos() += ob->pos();
  //  }
  // };
  // _root->addChild(newob);

  _visibleStuff = std::make_unique<VisibleStuff>();
}
void World::update() {
  auto dt = _time->update();
  Assert(_root != nullptr);
  _root->update(dt);
}
void World::cull(RenderView* rv, Bobj* ob) {
  if (ob == nullptr) {
    _visibleStuff->clear();
    ob = _root.get();
  }

  if (rv->camera()->frustum()->hasBox(ob->boundBox())) {
    for (auto ch : ob->children()) {
      cull(rv, ch.get());
    }
  }
}
void World::renderPipeStage(RenderView* rv, PipelineStage* ps) {
  // if (stage == PipelineStageEnum.Deferred)
  // {

  // TODO:
  // find rv on stuff, then draw it

  // _visibleStuff->draw(rv, DrawMode.Deferred, _worldProps);

  // }
  // else if (stage == PipelineStageEnum.Forward)
  // {
  //   _visibleStuff.Draw(rv, DrawMode.Forward, _worldProps);
  // }
  // else if (stage == PipelineStageEnum.Debug)
  // {
  //   _visibleStuff.Draw(rv, DrawMode.Debug, _worldProps);
  // }
}

#pragma endregion
#pragma region PipelineStage

bool PipelineStage::beginRender(bool forceclear) {
  //**TODO: this would bind & clear framebuffer target(s)
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  return true;
}
void PipelineStage::endRender() {}

#pragma endregion
#pragma region Renderer

void Renderer::beginRenderToWindow() {
  // hypothetical use of framebuffers
  glViewport(0, 0, Gu::context()->width(), Gu::context()->height());
  glScissor(0, 0, Gu::context()->width(), Gu::context()->height());

  // clears stages
  for (auto& ps : _pipelineStages) {
    ps->beginRender(true);
    ps->endRender();
  }
}
void Renderer::renderViewToWindow(RenderView* rv) {
  if (beginRenderToView(rv)) {
    for (auto& ps : _pipelineStages) {
      // Gu.Prof($"Begin {ps.PipelineStageEnum.ToString()}");

      // if (!IsActiveStage(rv, ps))
      // {
      //   continue;
      // }

      _currentStage = ps.get();

      if (rv->beginPipelineStage(ps.get())) {
        if (ps->beginRender(false)) {
          Gu::world()->renderPipeStage(rv, ps.get());

          // If we are a blit stage, execute a blit.
          //               if (ps.BlitObj != null && ps.BlitMat != null)
          //               {
          //                 //blit
          //                 rv.BeginRender2D(ps.BlitMat);
          //                 {
          //                   //Set the viewport to the whole window to blit the fullscreen quad however set the
          //                   //scissor to be just the viewport area.
          //                   //TODO: it would make more sense to have the quad blit just to the given area, and not have to re-set the viewport.
          //                   //https://stackoverflow.com/questions/33718237/do-you-have-to-call-glviewport-every-time-you-bind-a-frame-buffer-with-a-differe
          //
          //                   //This w/h should automatically be set to the size of the current output framebuffer
          //                   GL.Viewport(0, 0, ps.Size.width, ps.Size.height);
          //                   DrawCall.Draw(Gu.World.WorldProps, rv, ps.BlitObj);
          //                 }
          //                 rv.EndRender2D();
          //               }

          // SaveFBOsPostRender();

          ps->endRender();
          _currentStage = nullptr;
        }
      }
      // Gu.Prof($ "End {ps.PipelineStageEnum.ToString()}");
    }
  }
  endRenderToView(rv);
}
void Renderer::endRenderToWindow() {}
bool Renderer::beginRenderToView(RenderView* rv) {
  _currentView = rv;
  Gu::context()->gpu()->setState(GpuRenderState());  // clear
  CheckErrorsDbg();
  return true;
}
void Renderer::endRenderToView(RenderView* rv) {
  _currentView = nullptr;
}
#pragma region Window

Window::Window() {}
void Window::init(int w, int h, std::string title, GLFWwindow* share) {
  Assert(Gu::context() == this);
  msg("Window " + std::to_string(w) + "x" + std::to_string(h) + " title=" + title);
  _width = w;
  _height = h;

  if (Gu::isDebug()) {
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
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

  _window = glfwCreateWindow(w, h, title.c_str(), NULL, share);
  if (!_window) {
    Raise("Failed to create glfw window.");
  }

  glfwMakeContextCurrent(_window);

  if (Gu::context() == nullptr) {
    // first ctx
    msg("GL version: " + std::to_string(glGetString(GL_VERSION)));
  }

  glfwSwapInterval(0);
  glfwSetMouseButtonCallback(_window, [](auto* win, auto button, auto action, auto mods) {
    B26D::Window* mw = Gu::getWindow(win);
    Assert(mw);
    Assert(mw->_input);
    if (action == GLFW_PRESS || action == GLFW_RELEASE) {
      mw->_input->addKeyEvent(button, action == GLFW_PRESS);
    }
  });
  glfwSetKeyCallback(_window, [](auto win, auto key, auto code, auto action, auto mods) {
    B26D::Window* mw = Gu::getWindow(win);
    Assert(mw);
    Assert(mw->_input);
    if (action == GLFW_PRESS || action == GLFW_RELEASE) {
      mw->_input->addKeyEvent(key, action == GLFW_PRESS);
    }
  });
  glfwSetWindowSizeCallback(_window, [](auto win, auto w, auto h) {
    // glfwSetFramebufferSizeCallback(_window, [](auto win, auto w, auto h) {
    Gu::getWindow(win)->on_resize(w, h);
  });
  glfwSetWindowCloseCallback(_window, [](auto win) {
    Gu::getWindow(win)->quit_everything();
  });

  // glew
  glewExperimental = GL_FALSE;
  auto glewinit = glewInit();
  if (glewinit != GLEW_OK) {
    Raise("glewInit failed.");
  }
  glUseProgram(0);  // test  glew

  // context data
  _input = std::make_unique<Input>(this);
  _gpu = std::make_unique<Gpu>();
  _renderer = std::make_unique<Renderer>();
  _picker = std::make_unique<Picker>();

  createRenderView(vec2(0, 0), vec2(1, 1));
  initEngine();

  _state = WindowState::Running;
}
Window::~Window() {
  glfwDestroyWindow(_window);
}
void Window::toggleFullscreen() {
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
void Window::quit_everything() {
  _state = WindowState::Quit;
}
void Window::initEngine() {
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_SCISSOR_TEST);
  glDisable(GL_CULL_FACE);
  glFrontFace(GL_CCW);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  if (Gu::glGetInteger(GL_CONTEXT_FLAGS) & GL_CONTEXT_FLAG_DEBUG_BIT) {
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(
        [](GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message, const void* userParam) {
          // ignore non-significant error/warning codes
          if (id == 131169 || id == 131185 || id == 131218 || id == 131204) {
            return;
          }
          str st = "";

          st += "---------------\n";
          st += str("") + "Debug message (" + id + "): \n";

          switch (source) {
            case GL_DEBUG_SOURCE_API:
              st += "Source: API";
              break;
            case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
              st += "Source: Window System";
              break;
            case GL_DEBUG_SOURCE_SHADER_COMPILER:
              st += "Source: Shader Compiler";
              break;
            case GL_DEBUG_SOURCE_THIRD_PARTY:
              st += "Source: Third Party";
              break;
            case GL_DEBUG_SOURCE_APPLICATION:
              st += "Source: Application";
              break;
            case GL_DEBUG_SOURCE_OTHER:
              st += "Source: Other";
              break;
          }
          st += "\n";

          switch (type) {
            case GL_DEBUG_TYPE_ERROR:
              st += "Type: Error";
              break;
            case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
              st += "Type: Deprecated Behaviour";
              break;
            case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
              st += "Type: Undefined Behaviour";
              break;
            case GL_DEBUG_TYPE_PORTABILITY:
              st += "Type: Portability";
              break;
            case GL_DEBUG_TYPE_PERFORMANCE:
              st += "Type: Performance";
              break;
            case GL_DEBUG_TYPE_MARKER:
              st += "Type: Marker";
              break;
            case GL_DEBUG_TYPE_PUSH_GROUP:
              st += "Type: Push Group";
              break;
            case GL_DEBUG_TYPE_POP_GROUP:
              st += "Type: Pop Group";
              break;
            case GL_DEBUG_TYPE_OTHER:
              st += "Type: Other";
              break;
          }
          st += "\n";

          switch (severity) {
            case GL_DEBUG_SEVERITY_HIGH:
              st += "Severity: high";
              break;
            case GL_DEBUG_SEVERITY_MEDIUM:
              st += "Severity: medium";
              break;
            case GL_DEBUG_SEVERITY_LOW:
              st += "Severity: low";
              break;
            case GL_DEBUG_SEVERITY_NOTIFICATION:
              st += "Severity: notification";
              break;
          }
          st += "\n";
          st += str(message, length);
          st += "\n";
          LogError(st);
        },
        nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
  }

  glClearColor(0.89, 0.91, 0.91, 1.0);
  CheckErrorsDbg();

  auto testpath = Gu::relpath("../data/tex/stpeter.jpg");
  _testTex = Texture::singlePixel(vec4(1, 1, 1, 1));

  _drawQuads = std::make_unique<DrawQuads>();
  _drawQuads->testMakeQuads();
  _drawQuads->copyToGpu();

  _objShader = std::make_unique<Shader>(Gu::assetpath("shader/b26obj.vs.glsl"), Gu::assetpath("/shader/b26obj.gs.glsl"), Gu::assetpath("/shader/b26obj.fs.glsl"));

  _objBuf = std::make_unique<GpuArray<GpuObj>>();
  _lightBuf = std::make_unique<GpuArray<GpuLight>>();
  _worldBuf = std::make_unique<GpuArray<GpuWorld>>(1);
  _dataBuf = std::make_unique<GpuArray<GpuViewData>>(1);

  _objVao = std::make_unique<VertexArray>();

  glfwGetWindowPos(_window, &_lastx, &_lasty);
  glfwGetWindowSize(_window, &_lastw, &_lasth);
  on_resize(_lastw, _lasth);
}
void Window::on_resize(int w, int h) {
  _lastw = _width;
  _lasth = _height;
  _width = w;
  _height = h;

  initFramebuffer();
}
void Window::initFramebuffer() {
  // TODO:framebuffer
  glViewport(0, 0, _width, _height);
  glScissor(0, 0, _width, _height);

  _renderer = std::make_unique<Renderer>();

  CheckErrorsRt();
}
void Window::updateState() {
  _visible = glfwGetWindowAttrib(_window, GLFW_VISIBLE);
  _minimized = glfwGetWindowAttrib(_window, GLFW_ICONIFIED);
  _focused = glfwGetWindowAttrib(_window, GLFW_FOCUSED);

  glfwPollEvents();
  if (glfwWindowShouldClose(_window)) {
    _state = WindowState::Quit;
  }
}
void Window::updateInput() {
  Assert(_input);
  _input->update();
  if (_input->press(GLFW_KEY_F11)) {
    toggleFullscreen();
  }
  if (_input->press(GLFW_KEY_ESCAPE)) {
    quit_everything();
  }
}
void Window::cullViews() {}
void Window::createRenderView(vec2 xy, vec2 wh) {
  auto rv = std::make_unique<RenderView>("default", xy, wh, _width, _height);
  _views.push_back(std::move(rv));
}
void Window::renderViews() {
  if (false) {
    //**NEW SYSTM
    //**NEW SYSTM
    //**NEW SYSTM
    //**NEW SYSTM
    //**NEW SYSTM
    //**NEW SYSTM
    //**NEW SYSTM
    //**NEW SYSTM
    //**NEW SYSTM
    //**NEW SYSTM
    //**NEW SYSTM
    //**NEW SYSTM
    //**NEW SYSTM
    //**NEW SYSTM
    //**NEW SYSTM
    //**NEW SYSTM
    _renderer->beginRenderToWindow();
    for (auto& rv : _views) {
      if (rv->enabled()) {
        _renderer->renderViewToWindow(rv.get());
      }
    }
    _renderer->endRenderToWindow();
    _picker->updatePickedPixel();
    // Gu.Context.GameWindow.Context.SwapBuffers();
    // Gu.Context.Gpu.ExecuteCallbacks_RenderThread();
  }
  else {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    auto cam = Gu::world()->activeCamera();

    cam->updateViewport(_width, _height);  //_proj shouldbe on the window

    // draw quads test
    _drawQuads->draw(cam.get(), _testTex.get());

    GpuViewData gvd;
    gvd._proj = cam->proj();
    gvd._view = cam->view();
    gvd._viewPos = cam->pos();
    gvd._windowWidth = width();
    gvd._viewDir = cam->forward();
    gvd._windowHeight = height();

    // test
    // test
    // test
    // test
    std::vector<GpuLight> lights;
    std::vector<GpuObj> objs;
    for (auto& ch : Gu::world()->root()->children()) {
      if (ch->visible() && ch->isBobj()) {
        objs.push_back(ch->gpuObj());
      }
      if (Gu::fuzzyNotZero(ch->lightRadius())) {
        lights.push_back(ch->gpuLight());
      }
    }
    gvd._lightCount = lights.size();

    _dataBuf->copyToGpu(gvd);
    _objBuf->copyToGpu(objs);
    _lightBuf->copyToGpu(lights);
    _worldBuf->copyToGpu(Gu::world()->gpuWorld());

    _objShader->beginRender();
    _objShader->setTextureUf(Gu::world()->textureArray(), 0, "_textures");
    _objShader->bindBlock("_ufGpuViewData_Block", dynamic_cast<GpuBuffer*>(_dataBuf.get()));
    _objShader->bindBlock("_ufGpuWorld_Block", dynamic_cast<GpuBuffer*>(_worldBuf.get()));
    _objShader->bindBlock("_ufGpuObj_Block", dynamic_cast<GpuBuffer*>(_objBuf.get()));
    _objShader->bindBlock("_ufGpuLight_Block", dynamic_cast<GpuBuffer*>(_lightBuf.get()));
    _objVao->bind();
    glDrawArrays(GL_POINTS, 0, objs.size());
    _objVao->unbind();
    _objShader->endRender();
  }

  CheckErrorsRt();
  prof();
}
void Window::swap() {
  prof();
  glfwSwapBuffers(_window);
  prof();
}

#pragma endregion
#pragma region b2

void b2_datafile::deserialize(BinaryFile* bf) {
  int hdr_b = bf->readByte();
  int hdr_2 = bf->readByte();
  int hdr_m = bf->readByte();
  int hdr_d = bf->readByte();

  _major = bf->readInt32();
  _minor = bf->readInt32();
  _mtex_w = bf->readInt32();
  _mtex_h = bf->readInt32();

  Assert(bf->readInt32() == b2_datafile::c_magic);

  Assert(_major == b2_datafile::c_MetaFileVersionMajor && _minor == b2_datafile::c_MetaFileVersionMinor);
  int clayers = bf->readInt32();
  // msgv(clayers);
  for (int i = 0; i < clayers; i++) {
    _layers.push_back(bf->readString());
  }

  int ntexs = bf->readInt32();
  // msgv(ntexs);
  for (int i = 0; i < ntexs; i++) {
    auto bt = std::make_unique<b2_mtexdata>();
    bt->deserialize(bf);
    _texs.push_back(std::move(bt));
  }
  int nobjs = bf->readInt32();
  // msgv(nobjs);
  for (int i = 0; i < nobjs; i++) {
    auto ob = std::make_unique<b2_objdata>();
    ob->deserialize(bf);
    _objs.push_back(std::move(ob));
  }

  Assert(bf->readInt32() == b2_datafile::c_magic);

  // debug
  Assert(_objs.size() > 0);
  for (auto& ob : _objs) {
    msgv(ob->_name);
  }
}
void b2_mtexdata::deserialize(BinaryFile* bf) {
  Assert(bf->readInt32() == b2_datafile::c_magic);
  _iid = bf->readInt32();
  _w = bf->readInt32();
  _h = bf->readInt32();
  auto imgcount = bf->readInt32();
  // msgv(_iid);
  // msgv(_w);
  // msgv(_h);
  // msgv(imgcount);
  Assert(bf->readInt32() == b2_datafile::c_magic);
  for (int img = 0; img < imgcount; img++) {
    std::string image = bf->readString();
    _images.push_back(image);
  }
}
void b2_objdata::deserialize(BinaryFile* bf) {
  Assert(bf->readInt32() == b2_datafile::c_magic);
  _id = bf->readInt32();
  _name = bf->readString();
  _fps = bf->readFloat();
  auto actioncount = bf->readInt32();
  // msgv(_id);
  // msgv(_name);
  // msgv(_fps);
  // msgv(actioncount);
  Assert(bf->readInt32() == b2_datafile::c_magic);
  for (int iact = 0; iact < actioncount; iact++) {
    auto act = std::make_unique<b2_actiondata>();
    act->deserialize(bf);
    _actions.push_back(std::move(act));
  }
}
void b2_actiondata::deserialize(BinaryFile* bf) {
  Assert(bf->readInt32() == b2_datafile::c_magic);
  _id = bf->readInt32();
  _name = bf->readString();
  auto framecount = bf->readInt32();
  // msgv(_id);
  // msgv(_name);
  // msgv(framecount);
  Assert(bf->readInt32() == b2_datafile::c_magic);
  for (int ifr = 0; ifr < framecount; ifr++) {
    auto fr = std::make_unique<b2_framedata>();
    fr->deserialize(bf);
    _frames.push_back(std::move(fr));
  }
}
void b2_framedata::deserialize(BinaryFile* bf) {
  Assert(bf->readInt32() == b2_datafile::c_magic);
  _seq = bf->readFloat();
  _iid = bf->readInt32();
  _x = bf->readInt32();
  _y = bf->readInt32();
  _w = bf->readInt32();
  _h = bf->readInt32();
  int32_t c = bf->readInt32();
  Assert(bf->readInt32() == b2_datafile::c_magic);
  for (int ii = 0; ii < c; ii++) {
    _imgs.push_back(bf->readString());
  }
  // msgv(_seq);
  // msgv(_iid);
  // msgv(_x);
  // msgv(_y);
  // msgv(_w);
  // msgv(_h);
  // msgv(_imgs.size());
}

#pragma endregion

}  // namespace TESTAPP_NS

extern "C" {
DLL_EXPORT int main(int argc, char** argv) {
  return B26D::Gu::run(argc, argv);
}
}  // extern C