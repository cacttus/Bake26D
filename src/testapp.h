
#pragma once
#ifndef __908357602394876023948723423TESTAPP_H__
#define __908357602394876023948723423TESTAPP_H__

#include "./testapp_defines.h"
#include "./gpu_structs.h"

namespace B26D {

class b2_objdata {
public:
  int32_t _id = -1;
  std::string _name = "";
  std::vector<b2_action> _actions;
  void deserialize(BinaryFile* bf);
};
class b2_action {
public:
  int32_t _id = -1;
  std::string _name = "";
  std::vector<b2_frame> _frames;
  void deserialize(BinaryFile* bf);
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
  void deserialize(BinaryFile* bf);
};
class b2_mtex {
public:
  int32_t _texid = -1;
  std::vector<std::string> _images;
  void deserialize(BinaryFile* bf);
};
// #define PixelSize = 1.0f/4.0f //1/4 meter - do in shader
struct GpuSpriteData {
  uint32_t _texId;  // normal or color
  uvec4 _quad;      // quad in texture
  mat4 _model;
};
struct GpuSpriteVertex {
  uint32_t _id;
};
struct GpuQuadVert {
  glm::vec3 _v;
  uint32_t _id;
  glm::vec2 _x;
  glm::vec4 _c;
};
struct GpuQuad {
  GpuQuadVert _verts[4];
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
  void zero();
};
class Exception {
  std::string _what = "";
  const char* _file;
  int _line;

public:
  const char* file() { return _file; }
  int line() { return _line; }
  std::string what() { return _what; }
  Exception(const char* file, int line, std::string what) {
    _file = file;
    _line = line;
    _what = what;
  }
};

class Log {
private:
  static void _output(std::string color, bool bold, std::string type, const char* file, int line, std::string s);
  static std::string _header(std::string color, bool bold, std::string type, const char* file, int line);

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

  static void err(std::string s, const char* file, int line);
  static void dbg(std::string s, const char* file, int line);
  static void inf(std::string s, const char* file, int line);
  static void exception(Exception ex);
  static void print(std::string s);
  static void print(std::string s, std::string color, bool bold = false);
  static std::string cc_reset();
  static std::string cc_color(std::string color, bool bold);
};

class Gu {
  static path_t _appPath;
  static path_t _assetsPath;
  static Window* _context;
  static std::unique_ptr<AppConfig> _appConfig;
  static std::map<GLFWwindow*, std::unique_ptr<Window>> _windows;

public:
  template <typename T>
  static inline bool whileTrueGuard(T& x, T max = 9999999) {
    if (x >= max) {
      RaiseDebug("while(true) guard failed. max=" + max);
      return false;
    }
    x++;
    return true;
  }

  static Window* createWindow(int w, int h, std::string title);
  static Window* getWindow(GLFWwindow* win);
  static AppConfig* config() { return _appConfig.get(); }
  static Window* currentContext() { return _context; }
  static std::string pad(std::string st, int width, char padchar = '0');
  static void initGlobals(std::string exe_path);
  static path_t relpath(std::string rel);
  static path_t assetpath(std::string rel);
  static bool exists(path_t path);
  static void checkErrors(const char* file, int line);
  static int printDebugMessages(string_t&);
  static std::string getShaderInfoLog(GLint prog);
  static std::string getProgramInfoLog(GLint prog);
  static void rtrim(std::string& s);
  static GLint glGetInteger(GLenum arg);
  static float random(float a, float b) { return (((float)a) + (((float)rand() / (float)(RAND_MAX))) * (((float)b) - ((float)a))); }
  static void debugBreak();
  static uint64_t getMicroSeconds();
  static uint64_t getMilliSeconds();
  static std::string executeReadOutput(const std::string& cmd);
  static std::string readFile(path_t fileLoc);
  static int run(int argc, char** argv);
};

class GLObject {
protected:
  GLuint _glId;

public:
  GLuint glId() { return _glId; }
};
class Image {
private:
  int _width = 0;
  int _height = 0;
  int _bpp = 0;  // bytes
  std::unique_ptr<char[]> _data;

public:
  void check();
  int width() { return _width; }
  int height() { return _height; }
  int bpp() { return _bpp; }
  char* data() { return _data.get(); }

  Image(int w, int h, int bpp=4, const char* data= nullptr);
  virtual ~Image();

  void init(int w, int h, int bpp=4, const char* data = nullptr);
  glm::u8vec4* pixelOff(int32_t x, int32_t y);
  void copySubImageFrom(const ivec2& myOff, const ivec2& otherOff, const ivec2& size, Image* pOtherImage);
  static std::unique_ptr<Image> crop(Image* img, int w, int h);
  static std::unique_ptr<Image> from_file(std::string path);
};
class Texture : public GLObject {
public:
  Texture();
  Texture(Image* img, bool mipmaps = false);
  virtual ~Texture();

  static int calcMipLevels(int w, int h, int minwh = 1);
  static std::unique_ptr<Texture> singlePixel(vec4 color);

  void copyToGpu(int w, int h, void* data);
  void copyToGpu(int x, int y, int w, int h, void* data);
  void bind(int32_t channel);
  void unbind(int32_t channel);
};
class TextureArray : public GLObject {
private:
  int _count = 0;
  int _width = 0;
  int _height = 0;
  int _levels = 1;

public:
  TextureArray();
  TextureArray(int w, int h, int count, bool mipmaps);
  TextureArray(std::vector<Image*> imgs, bool mipmaps);
  virtual ~TextureArray();

  static std::unique_ptr<TextureArray> singlePixel(const vec4& color, int count);  // array of single pixels

  void copyToGpu(int index, void* data);
  void copyToGpu(int x, int y, int w, int h, int index, void* data);
  void bind(int32_t channel);
  void unbind(int32_t channel);
};
class Framebuffer : public GLObject {
public:
  Framebuffer();
  virtual ~Framebuffer();
};
class Shader : public GLObject {
public:
  enum class ShaderStage { Vertex,
                           Geometry,
                           Fragment };
  enum class ShaderLoadState {
    None,
    CompilingShaders,
    Success,
    Failed,
    MaxShaderLoadStates,
  };
  class ShaderMeta {
  public:
    std::string _globals;
    std::vector<std::string> _structs;
    ShaderMeta();
  };
  class Uniform {
  public:
    std::string _name = "";
    int _location = -1;
    int _sizeBytes = 0;
    bool _active = false;
    GLenum _type;
    bool _hasBeenBound = false;
    GLenum _rangeTarget;
    GLenum _bufferTarget;
    Uniform(std::string name, int location, int size, GLenum type, bool active) {
      _name = name;
      _location = location;
      _sizeBytes = size;
      _type = type;
      _active = active;
    }
  };
  class BufferBlock {  // UniformBlock, ShaderStorageBlock
  public:
    std::string _name = "";
    int _bindingIndex = -1;
    int _bufferSizeBytes = 0;
    bool _active = false;
    bool _hasBeenBound = false;
    GLenum _buftype;
    BufferBlock(std::string name, int bind, int size, bool active, GLenum buftype) {
      _name = name;
      _buftype = buftype;
      _bindingIndex = bind;
      _bufferSizeBytes = size;
      _active = active;
    }
  };

private:
  static std::unique_ptr<ShaderMeta> _meta;

  // std::map<std::string, int> _buffer_bindings;

  std::vector<std::string> _vert_src;
  std::vector<std::string> _frag_src;
  std::vector<std::string> _geom_src;

  std::vector<std::unique_ptr<BufferBlock>> _uniformBlocks;
  std::vector<std::unique_ptr<Uniform>> _uniforms;
  std::vector<std::unique_ptr<BufferBlock>> _ssbos;

  std::string _name;
  ShaderLoadState _state = ShaderLoadState::None;
  int _maxBufferBindingIndex = 0;
  void parseUniformBlocks();
  void parseUniforms();
  void parseSSBOs();

  std::vector<std::string> processSource(path_t& loc, ShaderStage);
  static GLuint compileShader(GLenum type, std::vector<std::string>& src);
  static std::string getShaderInfoLog(GLuint prog);
  static std::string getProgramInfoLog(GLuint prog);
  static void printSrc(std::vector<std::string> src);
  static std::string debugFormatSrc(std::vector<std::string> src);

public:
  void bindBlockFast(BufferBlock*, GpuBuffer*);
  void bindSSBBlock(const string_t&& name, GpuBuffer*);
  int maxBufferBindingIndex() { return _maxBufferBindingIndex; }
  void setCameraUfs(Camera* cam);
  void setTextureUf(Texture* tex, GLuint index, string_t loc);
  void setTextureUf(TextureArray* tex, GLuint index, string_t loc);
  void bind();
  void unbind();
  Shader(path_t vert_src, path_t geom_src, path_t frag_src);
  ~Shader();
};
class GpuBuffer : public GLObject {
public:
  GpuBuffer(size_t size, void* data = nullptr, uint32_t flags = GL_DYNAMIC_DRAW);
  virtual ~GpuBuffer();
  void copyToGpu(size_t size, void* data, size_t offset = 0);
};
class VertexArray : public GLObject {
public:
  VertexArray();
  virtual ~VertexArray();
  void setAttrib(int attr_idx, int attr_compsize, GLenum attr_datatype, size_t attr_offset, bool attr_inttype, int binding_index, GpuBuffer* gbuf, size_t stride);
  void bind();
  void unbind();
};
class DrawQuads {
private:
  std::vector<GpuQuad> _quads;
  std::unique_ptr<VertexArray> _vao;
  std::unique_ptr<GpuBuffer> _vbo;
  std::unique_ptr<GpuBuffer> _ibo;
  std::unique_ptr<Shader> _shader;
  size_t _index = 0;
  bool _dirty = true;

public:
  Shader* shader() { return _shader.get(); }
  VertexArray* vao() { return _vao.get(); }
  std::vector<GpuQuad>& quads() { return _quads; }

  DrawQuads(uint32_t maxQuads = 1024);
  virtual ~DrawQuads();
  void copyToGpu();
  void draw(Camera* cm, Texture* color);
  GpuQuad* getQuad();
  void testMakeQuads();
  void reset();
};
class Input {
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
  std::map<int, KeyState> _keys;
  std::vector<KeyEvent> _key_events;

public:
  Input();
  virtual ~Input();
  void update();
  void addKeyEvent(int32_t key, bool press);
  bool pressOrDown(int key);
  bool press(int key);
};

class Bobj {
  static uint64_t s_idgen;
  uint64_t _id;
  string_t _name;

protected:
  vec3 _pos = vec3(0, 0, 0);
  quat _rot = quat(0, 0, 0, 1);
  vec3 _scl = vec3(1, 1, 1);
  float _speed = 10;     // 1;
  float _rspeed = 0.5f;  // 0.5f;
  glm::mat4 _world = mat4(1);
  glm::vec3 _right = vec3(1, 0, 0);
  glm::vec3 _up = vec3(0, 1, 0);
  glm::vec3 _forward = vec3(0, 0, 1);
  std::vector<std::unique_ptr<Component>> _components;
  std::vector<std::unique_ptr<Bobj>> _children;

public:
  uint64_t id() { return _id; }
  string_t& name() { return _name; }
  glm::vec3& up() { return _up; }
  glm::vec3& right() { return _right; }
  glm::vec3& forward() { return _forward; }
  glm::mat4& world() { return _world; }
  glm::vec3& pos() { return _pos; }
  glm::quat& rot() { return _rot; }
  glm::vec3& scl() { return _scl; }
  std::vector<std::unique_ptr<Bobj>>& children() { return _children; }
  float& speed() { return _speed; }
  float& rspeed() { return _rspeed; }
  std::vector<std::unique_ptr<Component>>& components() { return _components; }

  Bobj(string_t&& name);
  virtual void update(float dt, mat4* parent = nullptr);
};
class Camera : public Bobj {
  vec3 _lookat;
  float _fov = 40.0f;
  float _far = 1000.0f;
  std::unique_ptr<GpuCamera> _gpudata;
  std::unique_ptr<GpuBuffer> _buffer;

public:
  glm::mat4& proj();
  glm::mat4& view();

  GpuBuffer* uniformBuffer();

  Camera(string_t&& name);
  void updateViewport(int width, int height);
  void lookAt(vec3&& at);
  virtual void update(float dt, mat4* parent = nullptr) override;
};
class Component {
public:
  virtual void update(Bobj* obj, float dt) {}
};
class InputController : public Component {
  std::shared_ptr<Bobj> _obj = nullptr;

public:
  InputController();
  virtual void update(Bobj* obj, float dt) override;
};
class Scene {
  std::vector<std::shared_ptr<Bobj>> _objects;
  std::shared_ptr<Camera> _activeCamera = nullptr;

  // temp stuff.
  std::vector<b2_mtex> _mtexs;
  std::vector<b2_objdata> _objdatas;

  void loadD26Meta(path_t);

public:
  std::shared_ptr<Camera> activeCamera() { return _activeCamera; }
  std::unique_ptr<Texture> _color;
  std::unique_ptr<Texture> _normal_depth;
  std::unique_ptr<TextureArray> _test_array; //we're going to use arrays

  Scene();
  void update(float dt);
};
class Window {
public:
  enum class WindowState { Created,
                           Running,
                           Quit };

private:
  int _lastx = 0, _lasty = 0, _lastw = 0, _lasth = 0;
  bool _fullscreen = false;
  int _width = 0;
  int _height = 0;
  GLFWwindow* _window = nullptr;
  WindowState _state = WindowState::Created;
  std::unique_ptr<Input> _input;
  std::unique_ptr<DrawQuads> _drawQuads;
  std::shared_ptr<Scene> _scene = nullptr;

  std::vector<GpuObj> _objs;
  std::unique_ptr<Shader> _objShader;
  std::unique_ptr<GpuBuffer> _objBuf;
  std::unique_ptr<VertexArray> _objVao;

  void do_input();
  void toggleFullscreen();

public:
  Input* input() { return _input.get(); }
  WindowState state() { return _state; }
  GLFWwindow* glfwWindow() { return _window; }
  std::shared_ptr<Scene>& scene() { return _scene; }

  Window(int, int, std::string, GLFWwindow* share = nullptr);
  virtual ~Window();
  void initFramebuffer();
  void render();
  void on_resize(int w, int h);
  void initEngine();
  void quit_everything();
  void update();
};

class AppConfig {
public:
  bool BreakOnGLError = true;
  bool Debug_Print_Shader_Uniform_Details_Verbose_NotFound = true;
};

}  // namespace B26D

#endif