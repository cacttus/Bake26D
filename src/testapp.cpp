#include <fstream>
#include <sstream>
#include <iterator>
#include <numeric>
#include <regex>
#include "./testapp.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "./BinaryFile.h"

namespace std {

std::string to_string(const char* __val) { return std::string(__val); }
std::string to_string(const GLubyte* __val) { return std::string((const char*)__val); }

}  // namespace std

namespace B26D {

#pragma region static data

std::string Log::CC_BLACK = "30";
std::string Log::CC_RED = "31";
std::string Log::CC_GREEN = "32";
std::string Log::CC_YELLOW = "33";
std::string Log::CC_BLUE = "34";
std::string Log::CC_PINK = "35";
std::string Log::CC_CYAN = "36";
std::string Log::CC_WHITE = "37";
std::string Log::CC_NORMAL = "39";
std::map<GLFWwindow*, std::unique_ptr<Window>> Gu::_windows;
path_t Gu::_appPath = "";
path_t Gu::_assetsPath = "";
Window* Gu::_context = nullptr;
std::unique_ptr<AppConfig> Gu::_appConfig;
uint64_t Bobj::s_idgen = 1;

#pragma endregion
#pragma region Log

void Log::err(std::string s, const char* file, int line) { _output(CC_RED, false, "E", file, line, s); }
void Log::dbg(std::string s, const char* file, int line) { _output(CC_CYAN, false, "D", file, line, s); }
void Log::inf(std::string s, const char* file, int line) { _output(CC_WHITE, false, "I", file, line, s); }
void Log::exception(Exception ex) {
  err(ex.what(), ex.file(), ex.line());
}
void Log::print(std::string s) { std::cout << s << std::endl; }
void Log::print(std::string s, std::string color, bool bold) { std::cout << cc_color(CC_WHITE, bold) + s + cc_reset(); }
std::string Log::cc_reset() { return "\033[0m"; }
std::string Log::cc_color(std::string color, bool bold) { return std::string() + "\033[" + (bold ? "1;" : "") + color + "m"; }
void Log::_output(std::string color, bool bold, std::string type, const char* file, int line, std::string s) {
  s = _header(color, bold, type, file, line) + s + cc_reset();
  print(s);
}
std::string Log::_header(std::string color, bool bold, std::string type, const char* file, int line) {
  auto pdir = std::filesystem::path(std::string(file)).parent_path().filename();
  auto fn = std::filesystem::path(std::string(file)).filename();
  return cc_color(color, bold) + "[" + type + "] " + std::string(pdir) + "/" + std::string(fn) + ":" + std::to_string(line) + " ";
}
#pragma endregion
#pragma region Gu

int Gu::run(int argc, char** argv) {
  int ret_code = 0;
  try {
    Assert(argc > 0);
    Gu::initGlobals(argv[0]);

    if (!glfwInit()) {
      Raise("Failed to init glfw");
    }

    auto win = Gu::createWindow(800, 600, "2.6D Test");
    glfwMakeContextCurrent(win->glfwWindow());
    _context = win;

    auto scene = std::make_shared<Scene>();
    win->scene() = scene;
    // auto win2 = Gu::createWindow(800, 600, "2.6D Test");
    // win2->scene() = scene;

    // nput controler update () requires a context.. but is in a scene ..

    double lasttime, curtime;
    lasttime = curtime = glfwGetTime();
    bool exit = false;
    while (!exit) {
      auto curtime = glfwGetTime();
      auto delta = (float)(curtime - lasttime);
      lasttime = curtime;

      Assert(scene);
      scene->update(delta);

      std::vector<GLFWwindow*> destroy;
      for (auto ite = _windows.begin(); ite != _windows.end(); ite++) {
        auto win = ite->second.get();
        glfwMakeContextCurrent(win->glfwWindow());
        _context = win;

        win->update();
        if (win->state() == Window::WindowState::Quit) {
          destroy.push_back(win->glfwWindow());
        }
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
  // catch (std::exception ex) {
  // LogError("Caught std::exception " + ex.what());
  //   throw ex;
  // }
  catch (Exception ex) {
    Log::exception(ex);
    ret_code = 1;
  }

  glfwTerminate();

  return ret_code;
}
Window* Gu::createWindow(int w, int h, std::string title) {
  // share ctx with all windows.
  GLFWwindow* sharedctx = nullptr;
  if (_windows.begin() != _windows.end()) {
    sharedctx = _windows.begin()->second->glfwWindow();
  }
  auto win = std::make_unique<Window>(w, h, title, sharedctx);
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
void Gu::initGlobals(std::string exe_path) {
  _appPath = path_t(exe_path).parent_path();
  _assetsPath = _appPath / path_t("../../data/");
  _appConfig = std::make_unique<AppConfig>();
}
std::string Gu::pad(std::string st, int width, char padchar) {
  std::ostringstream out;
  out << std::internal << std::setfill(padchar) << std::setw(width) << st;
  return out.str();
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
auto Gu::createTexture(path_t impath) {
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

  CheckErrorsDbg();
  return tex;
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
#pragma endregion
#pragma region Image

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
    Raise("could not load image " + path);
  }
  return nullptr;
}

#pragma endregion
#pragma region Shader

Shader::Shader(path_t vert_loc, path_t geom_loc, path_t frag_loc) {
  _name = vert_loc.filename() + "_" + (geom_loc.empty() ? "" : (geom_loc.filename() + "_")) + frag_loc.filename();
  _state = ShaderLoadState::CompilingShaders;
  _glId = glCreateProgram();
  Assert(Gu::exists(vert_loc));
  Assert(Gu::exists(frag_loc));

  _vert_src = loadSourceLines(vert_loc, ShaderStage::Vertex);
  auto vert = compileShader(GL_VERTEX_SHADER, _vert_src);
  glAttachShader(_glId, vert);

  _frag_src = loadSourceLines(frag_loc, ShaderStage::Fragment);
  auto frag = compileShader(GL_FRAGMENT_SHADER, _frag_src);
  glAttachShader(_glId, frag);

  GLuint geom = 0;
  if (!geom_loc.empty()) {
    Assert(Gu::exists(geom_loc));
    _geom_src = loadSourceLines(geom_loc, ShaderStage::Geometry);
    geom = compileShader(GL_GEOMETRY_SHADER, _geom_src);
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
    _state = ShaderLoadState::Failed;
    // return;
    Raise("Failed to link program");
  }
  glDetachShader(_glId, vert);
  glDetachShader(_glId, frag);
  if (geom) {
    glDetachShader(_glId, geom);
  }

  parseUniformBlocks();
  parseUniforms();

  _state = ShaderLoadState::Success;
}

Shader::~Shader() {
  glDeleteProgram(_glId);
}
void Shader::bind() {
  glUseProgram(_glId);
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
    string_t u_name = "DEADDEADDEADDEADDEADDEADDEADDEADDEADDEADDEADDEADDEADDEADDEADDEAD";  // idk.
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

    bool active = true;
    int location = glGetUniformLocation(_glId, u_name.c_str());

    if (location < 0) {
      active = false;
      if (u_name.find('.') == std::string::npos) {
        // There will be tons of inactive uniforms for structures. struct.name
        // But for what we're doing if it isn't a structure it should be used.
        Gu::debugBreak();
      }
      if (Gu::config()->Debug_Print_Shader_Uniform_Details_Verbose_NotFound) {
        LogDebug(_name + ": .. Inactve uniform: " + u_name);
      }
      // Not necessarily an error
      // GetUniformLocation "This function returns -1 if name does not correspond to an active uniform variable in program,
      // if name starts with the reserved prefix "gl_", or if name is associated with an atomic counter or a named uniform block."
      // Uniform variables that are structures or arrays of structures may be queried by calling glGetUniformLocation for each
      // field within the structure. The array element operator "[]" and the structure field operator "." may be used in name in
      // order to select elements within an array or fields within a structure. The result of using these operators is not allowed
      // to be another structure, an array of structures, or a subcomponent of a vector or a matrix. Except if the last part of name
      // indicates a uniform variable array, the location of the first element of an array can be retrieved by using the name of the
      // array, or by using the name appended by "[0]".
    }
    else {
      if (Gu::config()->Debug_Print_Shader_Uniform_Details_Verbose_NotFound) {
        LogDebug(_name + ": .. Active uniform: " + u_name);
      }
    }

    _uniforms.push_back(std::make_unique<Uniform>(u_name, location, u_size, u_type, active));
  }
}
void Shader::parseUniformBlocks() {
  int u_count = 0;
  int u_block_count = 0;
  glGetProgramiv(_glId, GL_ACTIVE_UNIFORMS, &u_count);
  glGetProgramiv(_glId, GL_ACTIVE_UNIFORM_BLOCKS, &u_block_count);
  CheckErrorsRt();
  for (auto iBlock = 0; iBlock < u_block_count; iBlock++) {
    int buffer_size_bytes = 0;
    glGetActiveUniformBlockiv(_glId, iBlock, GL_UNIFORM_BLOCK_DATA_SIZE, &buffer_size_bytes);
    CheckErrorsRt();

    int binding = 0;
    glGetActiveUniformBlockiv(_glId, iBlock, GL_UNIFORM_BLOCK_BINDING, &binding);
    CheckErrorsRt();

    std::string u_name = "DEADDEADDEADDEADDEADDEADDEADDEADDEADDEADDEADDEADDEADDEADDEADDEAD";  // idk.
    int u_name_len = 0;
    glGetActiveUniformBlockName(_glId, iBlock, u_name.length(), &u_name_len, u_name.data());
    CheckErrorsRt();
    u_name = u_name.substr(0, u_name_len);

    bool active = true;
    if (binding < 0) {
      active = false;
      if (Gu::config()->Debug_Print_Shader_Uniform_Details_Verbose_NotFound) {
        LogDebug(_name + ": ..Inactive uniform block: " + u_name);
      }
    }
    else {
      if (Gu::config()->Debug_Print_Shader_Uniform_Details_Verbose_NotFound) {
        LogDebug(_name + ": ..Active uniform block: " + u_name);
      }

      _maxBufferBindingIndex = glm::max(_maxBufferBindingIndex, binding);
    }

    _uniformBlocks.push_back(std::make_unique<UniformBlock>(u_name, binding, buffer_size_bytes, active));
  }
  // check duplicate binding indexes for blocks
  for (int dupe_loc = 0; dupe_loc < _uniformBlocks.size(); dupe_loc++) {
    for (int dupe_loc2 = dupe_loc + 1; dupe_loc2 < _uniformBlocks.size(); dupe_loc2++) {
      auto ub0 = _uniformBlocks[dupe_loc].get();
      auto ub1 = _uniformBlocks[dupe_loc2].get();

      if (ub0->_bindingIndex == ub1->_bindingIndex) {
        LogError("Duplicate Uniform buffer binding index " + ub0->_bindingIndex + " for " + ub0->_name + " and " + ub1->_name + "");
      }
      Gu::debugBreak();
      _state = ShaderLoadState::Failed;
    }
  }
}
void Shader::bindBlockFast(UniformBlock* u, GpuBuffer* b) {
  Assert(b != nullptr);
  // Assert(b->_rangeTarget != nullptr);
  glBindBufferBase(GL_UNIFORM_BUFFER, u->_bindingIndex, b->glId());  // GL_UNIFORM_BUFFER GL_SHADER_STO..
  CheckErrorsDbg();
  // if (_bufferTarget == BufferTarget.UniformBuffer) { RangeTarget = BufferRangeTarget.UniformBuffer; }
  // else if (_bufferTarget == BufferTarget.ShaderStorageBuffer) { RangeTarget = BufferRangeTarget.ShaderStorageBuffer; }
  // else if (_bufferTarget == BufferTarget.ArrayBuffer) { RangeTarget = null; }
  // else if (_bufferTarget == BufferTarget.ElementArrayBuffer) { RangeTarget = null;
  glBindBuffer(GL_UNIFORM_BUFFER, b->glId());
  CheckErrorsDbg();
  u->_hasBeenBound = true;

  // u->_hasBeenCopiedInitially = b->CopyToGpuCalled;
  // u->_hasBeenCopiedThisFrame = b->CopyToGpuCalledFrameId == Gu.Context.FrameStamp;
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
std::string Shader::getHeader(ShaderStage stage) {
  return "head";
}
std::vector<std::string> Shader::loadSourceLines(path_t& loc, ShaderStage stage) {
  auto src = Gu::readFile(loc);
  auto str_globals = Gu::readFile(Gu::assetpath("shader/globals.glsl"));
  auto structs_raw = Gu::readFile(Gu::relpath("../src/gpu_structs.h"));

  std::string c_shared = "//SHADER_SHARED";

  auto da = structs_raw.find(c_shared);
  Assert(da != string_t::npos);
  da += c_shared.length();
  auto db = structs_raw.find(c_shared, da);
  Assert(db != string_t::npos);
  auto strStructs = structs_raw.substr(da, db - da);
  str_globals.append("\n");
  str_globals.append(strStructs);
  str_globals.append("\n");

  // struct inputs
  std::vector<std::string> def_structs;
  std::string strBuffers = "";
  std::regex rex("\\s*struct\\s+([a-zA-Z0-9_]+)[\\{\\s\\n]");
  std::smatch matches;
  auto res = std::regex_search(strStructs, matches, rex);
  auto xxx = matches.length();
  if (matches.length() > 0) {
    def_structs.push_back(matches[1]);
  }
  int binding = 0;
  for (auto& s : def_structs) {
    std::string buftype = "";
    buftype = "uniform";  // "buffer" << for ssbo
    std::string ufName = "_uf" + s;
    if (src.find(ufName) != std::string::npos) {
      strBuffers.append("layout(std140, binding = " + std::to_string(binding++) + ") " + buftype + " " + ufName + "_Block {\n");
      strBuffers.append("  " + s + " " + ufName + ";\n");
      strBuffers.append("};\n");
    }
  }
  str_globals.append(strBuffers);

  // append headers
  src = str_globals.append(src);

  // break into lines.
  std::vector<std::string> lines;
  int last = 0;
  int next = -1;
  for (int xxx = 0; xxx < 99999999; xxx++) {
    last = next + 1;
    next = src.find('\n', last);
    if (next == std::string::npos) {
      next = src.length();
    }
    if (next - last > 0) {
      auto line = src.substr(last, next - last);
      lines.push_back(line);
    }
    if (next == std::string::npos) {
      break;
    }
  }

  printSrc(lines);

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
    Raise(exinfo);
  }
  return shader;
}
void Shader::printSrc(std::vector<std::string> lines) {
  Log::print(debugFormatSrc(lines), Log::CC_CYAN);
}
void Shader::setCameraUfs(Camera* cam) {
  // msg("cam=\nproj\n" + cam->proj() + "\nview\n" + cam->view() + "\n\n");

  for (auto ite = _uniformBlocks.begin(); ite != _uniformBlocks.end(); ite++) {
    if (ite->get()->_name == "_ufGpuCamera_Block") {
      bindBlockFast(ite->get(), cam->uniformBuffer());
    }
  }
  //
  //
  //   auto proj_loc = glGetUniformLocation(_glId, "_projMatrix");
  //   Assert(proj_loc == -1);
  //   glProgramUniformMatrix4fv(_glId, proj_loc, 1, GL_FALSE, (GLfloat*)&cam->proj());
  //
  //   auto view_loc = glGetUniformLocation(_glId, "_viewMatrix");
  //   Assert(view_loc == -1);
  //   glProgramUniformMatrix4fv(_glId, view_loc, 1, GL_FALSE, (GLfloat*)&cam->view());
}
void Shader::setTextureUf(Texture* tex, GLuint channel, string_t loc) {
  auto tex_loc1 = glGetUniformLocation(_glId, loc.c_str());
  glProgramUniform1i(_glId, tex_loc1, channel);
  tex->bind(channel);
  CheckErrorsDbg();
}

#pragma endregion
#pragma region ALL OTHER CLASSES

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

GpuBuffer::GpuBuffer(size_t size, void* data, uint32_t flags) {
  glCreateBuffers(1, &_glId);
  // glNamedBufferStorage(_glId, size, data, flags);//for immutable buffers
  glNamedBufferData(_glId, size, data, flags);
  CheckErrorsDbg();
}
GpuBuffer::~GpuBuffer() {
  glDeleteBuffers(1, &_glId);
}
void GpuBuffer::copyToGpu(size_t size, void* data, size_t offset) {
  // glNamedBufferSubData(_glId, offset, size, data);
  glNamedBufferData(_glId, size, data, GL_DYNAMIC_DRAW);
  CheckErrorsDbg();
}

Texture::Texture() {
  glCreateTextures(GL_TEXTURE_2D, 1, &_glId);
}
Texture::Texture(Image* img, bool mipmaps) : Texture() {
  Assert(img);
  copyToGpu(img->width(), img->height(), img->data());
  CheckErrorsDbg();
}
Texture::~Texture() {
  glDeleteTextures(1, &_glId);
}
std::unique_ptr<Texture> Texture::singlePixel(vec4 color) {
  std::unique_ptr<Texture> ret = std::make_unique<Texture>();

  unsigned char pix[4];
  pix[0] = 255, pix[1] = 0, pix[2] = 255, pix[3] = 255;
  ret->copyToGpu(1, 1, (void*)pix);
  return ret;
}
void Texture::copyToGpu(int w, int h, void* data) {
  copyToGpu(0, 0, w, h, data);
}
void Texture::copyToGpu(int x, int y, int w, int h, void* data) {
  float mwidth = w, mheight = h;
  int mlevels = 0;
  while (mwidth > 1 && mheight > 1) {
    mlevels++;
    mwidth /= 2.0f;
    mheight /= 2.0f;
  }

  glTextureStorage2D(_glId, mlevels, GL_RGBA8, w, h);
  glTextureSubImage2D(_glId, 0, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, (void*)data);
  if (mlevels > 1) {
    glGenerateTextureMipmap(_glId);
  }
  glTextureParameteri(_glId, GL_TEXTURE_BASE_LEVEL, 0);
  glTextureParameteri(_glId, GL_TEXTURE_MAX_LEVEL, mlevels - 1);  // max(0, mips - 1));
  // TODO: mips
  glTextureParameterf(_glId, GL_TEXTURE_MIN_FILTER, GL_NEAREST);  // GL_LINEAR_MIPMAP_LINEAR);
  glTextureParameterf(_glId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  CheckErrorsDbg();
}
void Texture::bind(int32_t channel) {
  glBindTextureUnit(channel, _glId);
}
void Texture::unbind(int32_t channel) {
  glBindTextureUnit(channel, 0);
}

Framebuffer::Framebuffer() {
  glGenFramebuffers(1, &_glId);
}
Framebuffer ::~Framebuffer() {
  glDeleteFramebuffers(1, &_glId);
}

Input::Input() {}
Input::~Input() {}
void Input::addKeyEvent(int32_t key, bool press) {
  KeyEvent k;
  k._key = key;
  k._press = press;
  _key_events.push_back(k);
  msg("key event " + key + " press=" + press);
}
bool Input::press(int key) {
  auto it = _keys.find(key);
  if (it != _keys.end()) {
    return it->second == KeyState::Press;
  }
  return false;
}
bool Input::pressOrDown(int key) {
  auto it = _keys.find(key);
  if (it != _keys.end()) {
    return it->second == KeyState::Press || it->second == KeyState::Down;
  }
  return false;
}
void Input::update() {
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
}

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
  glVertexArrayAttribIFormat(_vao->glId(), i_idx, 1, GL_UNSIGNED_INT, sizeof(glm::vec3));
  glVertexArrayAttribFormat(_vao->glId(), x_idx, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec3) + sizeof(uint32_t));
  glVertexArrayAttribFormat(_vao->glId(), c_idx, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec3) + sizeof(uint32_t) + sizeof(glm::vec2));

  glVertexArrayAttribBinding(_vao->glId(), v_idx, vbo_binding_idx);
  glVertexArrayAttribBinding(_vao->glId(), i_idx, vbo_binding_idx);
  glVertexArrayAttribBinding(_vao->glId(), x_idx, vbo_binding_idx);
  glVertexArrayAttribBinding(_vao->glId(), c_idx, vbo_binding_idx);

  glVertexArrayVertexBuffer(_vao->glId(), vbo_binding_idx, _vbo->glId(), 0, sizeof(GpuQuadVert));
  glVertexArrayElementBuffer(_vao->glId(), _ibo->glId());

  CheckErrorsDbg();
}
DrawQuads::~DrawQuads() {
}
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
  _shader->bind();
  _shader->setCameraUfs(cam);
  _shader->setTextureUf(tex, 0, "_texture");
  _vao->bind();
  glDrawElements(GL_TRIANGLES, _index * 6, GL_UNSIGNED_INT, 0);
  _vao->unbind();
  _shader->unbind();
}
void GpuQuad::zero() {
  _verts[0]._v = glm::vec3(0, 0, 0);
  _verts[1]._v = glm::vec3(1, 0, 0);
  _verts[2]._v = glm::vec3(0, 1, 0);
  _verts[3]._v = glm::vec3(1, 1, 0);
  _verts[0]._x = glm::vec2(0, 0);
  _verts[1]._x = glm::vec2(1, 0);
  _verts[2]._x = glm::vec2(0, 1);
  _verts[3]._x = glm::vec2(1, 1);
  _verts[0]._c = glm::vec4(1, 1, 1, 1);
  _verts[1]._c = glm::vec4(1, 1, 1, 1);
  _verts[2]._c = glm::vec4(1, 1, 1, 1);
  _verts[3]._c = glm::vec4(1, 1, 1, 1);
}
void DrawQuads::testMakeQuads() {
  for (int i = 0; i < 1000; i++) {
    GpuQuad* cpy = getQuad();
    cpy->zero();

    glm::vec3 rnd;
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
      glm::vec4 rnd4;
      rnd4.r = Gu::random(0.1f, 1.0f);
      rnd4.g = Gu::random(0.6f, 1.0f);
      rnd4.b = Gu::random(0.6f, 1.0f);
      rnd4.a = Gu::random(0.1f, 1.0f);
      v._c = rnd4;
    }
  }
}

Camera::Camera(string_t&& name) : Bobj(std::move(name)), _gpudata(std::make_unique<GpuCamera>()), _buffer(std::make_unique<GpuBuffer>(sizeof(GpuCamera))) {
  // dummy vals.
  _pos = vec3(-5, 5, -5);
  lookAt(vec3(0, 0, 0));
  updateViewport(800, 600);
}
void Camera::updateViewport(int width, int height) {
  _fov = glm::clamp(_fov, 0.1f, 179.9f);
  _gpudata->_proj = glm::perspectiveFov(glm::radians(_fov), (float)width, (float)height, 1.0f, _far);
  _buffer->copyToGpu(sizeof(GpuCamera), _gpudata.get());
  //_view = glm::lookAt(vec3(10,10,10),vec3(0,0,0), vec3(0,1,0));
}
void Camera::lookAt(vec3&& at) {
  _lookat = at;
}
void Camera::update(float dt, mat4* parent) {
  Bobj::update(dt, parent);
  _gpudata->_view = _world;  // glm::lookAt(_pos, _lookat, _up);
  // todo update basis vectors
  // msg("forward="+forward());
  // msg("up="+up());
  // msg("right="+right());
}
glm::mat4& Camera::proj() { return _gpudata->_proj; }
glm::mat4& Camera::view() { return _gpudata->_view; }
GpuBuffer* Camera::uniformBuffer() { return _buffer.get(); }

Bobj::Bobj(string_t&& name) {
  _id = s_idgen++;
  _name = name;
}
void Bobj::update(float dt, mat4* parent) {
  for (auto& comp : _components) {
    comp->update(this, dt);
  }
  for (auto& child : _children) {
    child->update(dt);
  }

  // compile mat
  _world = parent ? *parent : mat4(1);
  _world = _world * glm::mat4_cast(_rot);
  _world = glm::translate(_world, _pos);
  _world = glm::scale(_world, _scl);

  // basis
  // msg("world=\n"+_world+"\n");
  _right = glm::normalize(vec3(vec4(1, 0, 0, 1) * _world));
  _up = glm::normalize(vec3(vec4(0, 1, 0, 1) * _world));
  _forward = glm::normalize(vec3(vec4(0, 0, 1, 1) * _world));
}

InputController::InputController() {
}
void InputController::update(Bobj* obj, float delta) {
  Assert(obj);
  Component::update(obj, delta);

  auto ct = Gu::currentContext();
  if (ct == nullptr) {
    LogError("Input: Context was null.");
    return;
  }

  float ang = (float)(M_PI * 2.0) * obj->rspeed() * delta;
  auto inp = ct->input();
  auto spdmul = inp->pressOrDown(GLFW_KEY_LEFT_SHIFT) || inp->pressOrDown(GLFW_KEY_RIGHT_SHIFT) ? 5.0f : 1.0f;
  if (inp->pressOrDown(GLFW_KEY_UP) || inp->pressOrDown(GLFW_KEY_W)) {
    obj->pos() += obj->forward() * obj->speed() * spdmul * delta;
  }
  if (inp->pressOrDown(GLFW_KEY_DOWN) || inp->pressOrDown(GLFW_KEY_S)) {
    obj->pos() -= obj->forward() * obj->speed() * spdmul * delta;
  }
  if (inp->pressOrDown(GLFW_KEY_LEFT) || inp->pressOrDown(GLFW_KEY_A)) {
    obj->rot() = glm::rotate(obj->rot(), -ang, obj->up());
  }
  if (inp->pressOrDown(GLFW_KEY_RIGHT) || inp->pressOrDown(GLFW_KEY_D)) {
    obj->rot() = glm::rotate(obj->rot(), ang, obj->up());
  }
}

Scene::Scene() {
  auto normpath = Gu::relpath("../output/mt_normal_depth.png");
  auto colpath = Gu::relpath("../output/mt_color.png");
  auto metapath = Gu::relpath("../output/b2_meta.bin");
  if (!Gu::exists(normpath) || !Gu::exists(colpath) || !Gu::exists(metapath)) {
    Raise(std::string() + "could not find textures: \n" + (normpath.string()) + "\n" + (colpath.string()) + "\n run the blender python script first");
  }
  loadD26Meta(metapath);
  auto normimg = Image::from_file(normpath.string());
  _normal_depth = std::make_unique<Texture>(normimg.get());
  auto colimg = Image::from_file(colpath.string());
  _color = std::make_unique<Texture>(colimg.get());

  _activeCamera = std::make_shared<Camera>("MainCamera");
  _activeCamera->components().push_back(std::make_unique<InputController>());
  _activeCamera->lookAt(vec3(0, 0, 0));
  _objects.push_back(_activeCamera);
}
void Scene::update(float dt) {
  for (auto& obj : _objects) {
    obj->update(dt);
  }
}
void Scene::loadD26Meta(path_t loc) {
  msg("Loading metadata " + loc.string());

  BinaryFile bf;
  bf.loadFromDisk(loc.string());
  int hdr_b = bf.readByte();
  int hdr_2 = bf.readByte();
  int hdr_m = bf.readByte();
  int hdr_d = bf.readByte();

  int ntexs = bf.readInt32();
  for (int i = 0; i < ntexs; i++) {
    b2_mtex b;
    b.deserialize(&bf);
    _mtexs.push_back(b);
  }
  int nobjs = bf.readInt32();
  for (int i = 0; i < nobjs; i++) {
    b2_objdata ob;
    ob.deserialize(&bf);
    _objdatas.push_back(ob);
  }
}
#pragma endregion
#pragma region MainWindow

Window::Window(int w, int h, std::string title, GLFWwindow* share) {
  msg("Window " + std::to_string(w) + "x" + std::to_string(h) + " title=" + title);
  _width = w;
  _height = h;

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
  glfwSwapInterval(0);

  glfwSetKeyCallback(_window, [](auto win, auto key, auto code, auto action, auto mods) {
    auto mw = Gu::getWindow(win);
    Assert(mw);
    Assert(mw->_input);
    if (action == GLFW_PRESS || action == GLFW_RELEASE) {
      mw->_input->addKeyEvent(key, action == GLFW_PRESS);
    }
  });
  glfwSetFramebufferSizeCallback(
      _window, [](auto win, auto w, auto h) {
        Gu::getWindow(win)->on_resize(w, h);
        ;
      });
  glfwSetWindowCloseCallback(
      _window, [](auto win) {
        Gu::getWindow(win)->quit_everything();
      });

  // glew
  glewExperimental = GL_FALSE;
  auto glewinit = glewInit();
  if (glewinit != GLEW_OK) {
    Raise("glewInit failed.");
  }
  glUseProgram(0);  // test  glew

  initEngine();

  _state = WindowState::Running;
}
Window::~Window() {
  glfwDestroyWindow(_window);
}
void Window::update() {
  do_input();
  render();

  glfwSwapBuffers(_window);
  glfwPollEvents();
  if (glfwWindowShouldClose(_window)) {
    _state = WindowState::Quit;
  }
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
void Window::do_input() {
  Assert(_input);
  _input->update();
  if (_input->press(GLFW_KEY_F11)) {
    toggleFullscreen();
  }
  if (_input->press(GLFW_KEY_ESCAPE)) {
    quit_everything();
  }
}
void Window::initEngine() {
  _input = std::make_unique<Input>();

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
  CheckErrorsDbg();

  _drawQuads = std::make_unique<DrawQuads>();
  _drawQuads->testMakeQuads();
  _drawQuads->copyToGpu();

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

  CheckErrorsRt();
}
void Window::render() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  _scene->activeCamera()->updateViewport(_width, _height);  //_proj shouldbe on the window
  _drawQuads->draw(_scene->activeCamera().get(), _scene->_color.get());

  CheckErrorsRt();
}

#pragma endregion
#pragma region b2

void b2_objdata::deserialize(BinaryFile* bf) {
  _id = bf->readInt32();
  _name = bf->readString();
  auto actioncount = bf->readInt32();
  for (int iact = 0; iact < actioncount; iact++) {
    b2_action act;
    act.deserialize(bf);
    _actions.push_back(act);
  }
}
void b2_action::deserialize(BinaryFile* bf) {
  _id = bf->readInt32();
  _name = bf->readString();
  auto framecount = bf->readInt32();
  for (int ifr = 0; ifr < framecount; ifr++) {
    b2_frame fr;
    fr.deserialize(bf);
    _frames.push_back(fr);
  }
}
void b2_frame::deserialize(BinaryFile* bf) {
  _seq = bf->readFloat();
  _texid = bf->readInt32();
  _x = bf->readInt32();
  _y = bf->readInt32();
  _w = bf->readInt32();
  _h = bf->readInt32();
}
void b2_mtex::deserialize(BinaryFile* bf) {
  _texid = bf->readInt32();
  auto numimgs = bf->readInt32();
  for (int img = 0; img < numimgs; img++) {
    std::string image = bf->readString();
    _images.push_back(image);
  }
}

#pragma endregion

}  // namespace B26D

extern "C" {
DLL_EXPORT int main(int argc, char** argv) {
  return B26D::Gu::run(argc, argv);
}
}  // extern C