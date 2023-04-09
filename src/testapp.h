
#pragma once
#ifndef __908357602394876023948723423TESTAPP_H__
#define __908357602394876023948723423TESTAPP_H__

#include "./testapp_defines.h"
#include "./gpu_structs.h"

namespace B26D {

#pragma region testapp

template <class Tx>
class box2x {
public:
  typedef glm::vec<2, Tx, glm::defaultp> _vec;
  _vec _min;
  _vec _max;
  Tx width() const { return _max.x - _min.x; }
  Tx height() const { return _max.y - _min.y; }
  box2x() {}
  box2x(Tx minx, Tx miny, Tx maxx, Tx maxy) {
    _min.x = minx;
    _min.y = miny;
    _max.x = maxx;
    _max.y = maxy;
  }
  box2x(const _vec& dmin, const _vec& dmax) {
    _min = dmin;
    _max = dmax;
  }
  bool contains_inclusive(const _vec& pt) {
    return pt.x >= _min.x && pt.y >= _min.y && pt.x <= _max.x && pt.y <= _max.y;
  }
  bool contains_LT_inclusive(const _vec& pt) {
    return pt.x >= _min.x && pt.y >= _min.y && pt.x < _max.x && pt.y < _max.y;
  }
  box2x clip(const box2x& other) const {
    // clip by other box
    box2x ret = *this;
    if (ret._min.x < other._min.x) {
      ret._min.x = other._min.x;
    }
    if (ret._min.y > other._min.y) {
      ret._min.y = other._min.y;
    }
    if (ret._max.x > other._max.x) {
      ret._max.x = other._max.x;
    }
    if (ret._max.y > other._max.y) {
      ret._max.y = other._max.y;
    }
    return ret;
  }
  box2x intersect(const box2x& other) const {
    box2x ret;
    ret._max = glm::min(_max, other._max);
    ret._min = glm::max(_min, other._min);
    return ret;
  }
  bool valid() const {
    return _max.x >= _min.x && _max.y >= _min.y;
  }
  bool has_volume() const {
    return _max != _min;
  }
};

typedef box2x<float> box2;
typedef box2x<int> box2i;

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
  vec3 _v;
  uint32_t _id;
  vec2 _x;
  vec4 _c;
};
struct GpuQuad {
  GpuQuadVert _verts[4];
  // TODO: model matrix
  void translate(vec3 off) {
    for (auto& v : _verts) {
      v._v += off;
    }
  }
  void scale(vec3 scl) {
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
  static void warn(std::string s, const char* file, int line);
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
  static uptr<AppConfig> _appConfig;
  static std::map<GLFWwindow*, uptr<Window>> _windows;
  static std::map<ImageFormatType, uptr<ImageFormat>> _imageFormats;

public:
  inline static void trap(){
    int n=0;
    n++;
  }
  static void initGlobals(std::string exe_path);
  static int run(int argc, char** argv);
  static ImageFormat* imageFormat(ImageFormatType fmt, bool throwinfnotfound = true);
  static Window* createWindow(int w, int h, std::string title);
  static Window* getWindow(GLFWwindow* win);
  static AppConfig* config() { return _appConfig.get(); }
  static Window* currentContext() { return _context; }
  static std::string pad(std::string st, int width, char padchar = '0');
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
  // template < typename Tx >
  // static void move_uptr(std::vector<uptr<Tx>> from, std::vector<uptr<Tx>> to, ){
  //     std::vector<uptr<Tx>> source;
  //     source.push_back(std::make_unique<Tx>());
  //     std::vector<uptr<Tx>> destination;
  //     std::move(begin(source), end(source), std::back_inserter(destination));
  // }
  template <class Tx>
  static inline bool whileTrueGuard(Tx& x, Tx max = 999999999) {
    if (x >= max) {
      RaiseDebug("while(true) guard failed. max=" + max);
      return false;
    }
    x++;
    return true;
  }
};

class ImageFormat {
private:
  ImageFormatType _format;
  GLenum _glFormat;
  GLenum _glInternalFormat;
  GLenum _glDatatype;
  int _bpp;

public:
  ImageFormatType dataType() { return _format; }
  GLenum glFormat() { return _glFormat; }
  GLenum glInternalFormat() { return _glInternalFormat; }
  GLenum glDatatype() { return _glDatatype; }
  int bpp() { return _bpp; }

  ImageFormat(ImageFormatType fmt, GLenum glfmt, GLenum glinternalfmt, GLenum gldatatype, int bpp) {
    _format = fmt;
    _glFormat = glfmt;
    _glInternalFormat = glinternalfmt;
    _glDatatype = gldatatype;
    _bpp = bpp;
  }
};

class Image {
private:
  int _width = 0;
  int _height = 0;
  ImageFormat* _format;
  uptr<char[]> _data;

public:
  void check() const;
  int width() const { return _width; }
  int height() const { return _height; }
  ImageFormat* format() const { return _format; }
  char* data() { return _data.get(); }

  Image(int w, int h, ImageFormat* fmt, const char* data = nullptr);
  virtual ~Image();

  void init(int w, int h, ImageFormat* fmt, const char* data = nullptr);
  void* pixel(int32_t x, int32_t y);
  static uptr<Image> crop(Image* img, const box2i& box);
  static uptr<Image> from_file(std::string path);
  static void copy(Image* dst, const box2i& dstbox, Image* src, const box2i& srcbox);
  static uptr<Image> scale(Image* img, float s,  ImageFormat* changefmt = nullptr);
};

class GLObject {
protected:
  GLuint _glId;

public:
  GLuint glId() { return _glId; }
};

class TextureBase : public GLObject {
public:
protected:
  int _width = 0;
  int _height = 0;
  int _levels = 1;
  ImageFormat* _format;
  GLenum _type;

  static int calcMipLevels(int w, int h, int minwh = 1);
  void setStorageMode();
public:
  TextureBase(int w, int h, ImageFormat* format, bool mipmaps, GLenum type);
  ~TextureBase();

  void bind(int32_t channel);
  void unbind(int32_t channel);
};
class Texture : public TextureBase {
public:
  Texture();
  Texture(int w, int h, ImageFormat* fmt, bool mipmaps);
  Texture(Image* img, bool mipmaps = false);
  virtual ~Texture();

  static uptr<Texture> singlePixel(vec4 color);

  void copyToGpu(int w, int h, const void* data);
  void copyToGpu(int x, int y, int w, int h, const void* data);
};
class TextureArray : public TextureBase {
private:
  int _count = 0;

public:
  TextureArray(int w, int h, ImageFormat* fmt, bool mipmaps, int count);
  TextureArray(const std::vector<uptr<Image>>& imgs, bool mipmaps);
  virtual ~TextureArray();

  static uptr<TextureArray> singlePixel(const vec4& color, int count);  // array of single pixels

  void copyToGpu(int index, const void* data);
  void copyToGpu(int x, int y, int w, int h, int index, const void* data);
  static void conform( std::vector<uptr<Image>>& imgs);
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
  static uptr<ShaderMeta> _meta;

  // std::map<std::string, int> _buffer_bindings;

  std::vector<std::string> _vert_src;
  std::vector<std::string> _frag_src;
  std::vector<std::string> _geom_src;

  std::vector<uptr<BufferBlock>> _uniformBlocks;
  std::vector<uptr<Uniform>> _uniforms;
  std::vector<uptr<BufferBlock>> _ssbos;

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
  uptr<VertexArray> _vao;
  uptr<GpuBuffer> _vbo;
  uptr<GpuBuffer> _ibo;
  uptr<Shader> _shader;
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
  mat4 _world = mat4(1);
  vec3 _right = vec3(1, 0, 0);
  vec3 _up = vec3(0, 1, 0);
  vec3 _forward = vec3(0, 0, 1);
  std::vector<uptr<Component>> _components;
  std::vector<uptr<Bobj>> _children;

public:
  uint64_t id() { return _id; }
  string_t& name() { return _name; }
  vec3& up() { return _up; }
  vec3& right() { return _right; }
  vec3& forward() { return _forward; }
  mat4& world() { return _world; }
  vec3& pos() { return _pos; }
  quat& rot() { return _rot; }
  vec3& scl() { return _scl; }
  std::vector<uptr<Bobj>>& children() { return _children; }
  float& speed() { return _speed; }
  float& rspeed() { return _rspeed; }
  std::vector<uptr<Component>>& components() { return _components; }

  void lookAt(const vec3&& at);

  Bobj(string_t&& name);
  virtual void update(float dt, mat4* parent = nullptr);
};
class Camera : public Bobj {
  float _fov = 40.0f;
  float _far = 1000.0f;
  uptr<GpuCamera> _gpudata;
  uptr<GpuBuffer> _buffer;

public:
  mat4& proj();
  mat4& view();

  GpuBuffer* uniformBuffer();

  Camera(string_t&& name);
  void updateViewport(int width, int height);
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
  uptr<Texture> _color;
  uptr<Texture> _normal_depth;
  uptr<TextureArray> _test_array;  // we're going to use arrays

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
  uptr<Input> _input;
  uptr<DrawQuads> _drawQuads;
  std::shared_ptr<Scene> _scene = nullptr;

  std::vector<GpuObj> _objs;
  uptr<Shader> _objShader;
  uptr<GpuBuffer> _objBuf;
  uptr<VertexArray> _objVao;

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

#pragma endregion
#pragma region B26

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

#pragma endregion

}  // namespace B26D

#endif