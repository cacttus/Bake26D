
#pragma once
#ifndef __908357602394876023948723423TESTAPP_H__
#define __908357602394876023948723423TESTAPP_H__

#include "./GlobalDefines.h"
#include "./gpu_structs.h"

namespace B26D {

#pragma region core

template <typename Tx>
class VirtualMemoryShared : public std::enable_shared_from_this<Tx> {
public:
  VirtualMemoryShared() {}
  virtual ~VirtualMemoryShared() DOES_NOT_OVERRIDE {}
  template <typename Ty>
  std::shared_ptr<Ty> getThis() {
    return std::dynamic_pointer_cast<Ty>(this->shared_from_this());
  }
};

#pragma endregion
#pragma region math

class plane {
public:
  vec3 n;
  float d;
  plane() {}
  plane(const vec3& tri_p1, const vec3& tri_p2, const vec3& tri_p3) {
    // verts CCW for RHS -> normal points towards
    float u = 1.0f;
    vec3 origin = tri_p1;

    vec3 t = (tri_p2 - tri_p1);
    vec3 b = (tri_p3 - tri_p1);

    t /= u;
    glm::normalize(t);
    n = glm::cross(b, t);

    n = glm::normalize(n);
    b = glm::cross(n, t);

    d = -glm::dot(n, origin);
  }
  plane(vec3 dn, vec3 dpt) {
    d = -glm::dot(n, dpt);
    n = dn;
  }
  float dist(vec3& pt) { return (float)(glm::dot(n, pt) + d); }
};

class tri3 {
public:
  vec3 v[3];
  tri3() {}
  tri3(const vec3& p0, const vec3& p1, const vec3& p2) {
    v[0] = p0;
    v[1] = p1;
    v[2] = p2;
  }
  plane getPlane() { return plane(v[0], v[1], v[2]); }
};

template <typename Tx, int Dx>
class box2x {
public:
  typedef glm::vec<Dx, Tx, glm::defaultp> _vec;
  _vec _min;
  _vec _max;

  Tx x() const { return _min.x; }
  Tx y() const { return _min.y; }
  Tx width() const { return _max.x - _min.x; }
  Tx height() const { return _max.y - _min.y; }
  void x(const Tx& rhs) { _min.x = rhs; }
  void y(const Tx& rhs) { _min.y = rhs; }
  void width(const Tx& rhs) { _max.x = _min.x + rhs; }
  void height(const Tx& rhs) { _max.y = _min.y + rhs; }

  static box2x one() { return box2x(_vec(0), _vec(1)); }
  static box2x zero() { return box2x(_vec(0), _vec(0)); }

  box2x() {}
  box2x(const _vec& dmin, const _vec& dmax) {
    _min = dmin;
    _max = dmax;
  }
  virtual ~box2x() {}
  bool contains_inclusive(const _vec& pt) { return glm::vec_gte(pt, _min) && glm::vec_gte(pt, _max); }
  bool contains_LT_inclusive(const _vec& pt) { return glm::vec_gte(pt, _min) && glm::vec_gt(pt, _max); }
  box2x clip(const box2x& other) const {
    // clip by other box
    box2x ret = *this;
    glm::clamp(ret._min, other._min, glm::max(ret._min, other._min));
    glm::clamp(ret._max, other._max, glm::min(ret._max, other._max));
    return ret;
  }
  box2x intersect(const box2x& other) const {
    box2x ret;
    ret._max = glm::min(_max, other._max);
    ret._min = glm::max(_min, other._min);
    return ret;
  }
  void genReset() {
    _min = _vec(-FLT_MAX);
    _max = _vec(FLT_MAX);
  }
  void genExpandByPoint(const _vec& pt) {
    _min = glm::min(_min, pt);
    _max = glm::max(_max, pt);
  }
  bool valid() const { return _max >= _min; }
  bool has_volume() const { return _max != _min; }
  bool validate(bool debug_break = true, float min_volume = 0) const;
};

typedef box2x<float, 2> box2;
typedef box2x<int, 2> box2i;
typedef box2x<float, 3> box3;
typedef box2x<int, 3> box3i;

class oobox3 {
public:
  enum { c_vcount = 8 };
  vec3 verts[c_vcount];
  oobox3() {}
  oobox3(const vec3& i, const vec3& a) {
    // Creates an AABB
    verts[0] = vec3(i.x, i.y, i.z);
    verts[1] = vec3(a.x, i.y, i.z);
    verts[2] = vec3(i.x, a.y, i.z);
    verts[3] = vec3(a.x, a.y, i.z);
    verts[4] = vec3(i.x, i.y, a.z);
    verts[5] = vec3(a.x, i.y, a.z);
    verts[6] = vec3(i.x, a.y, a.z);
    verts[7] = vec3(a.x, a.y, a.z);
  }
  oobox3 clone() { return *this; }
  vec3 center() { return verts[0] + (verts[7] - verts[0]) * 0.5f; }
  void getTrianglesAndPlanes(std::vector<tri3>& tris, std::vector<plane>& planes, bool triangles_face_outside = true) {
    tris.push_back(tri3(verts[4], verts[2], verts[0]));  // l0
    tris.push_back(tri3(verts[4], verts[6], verts[2]));  // l1
    tris.push_back(tri3(verts[1], verts[3], verts[5]));  // r0
    tris.push_back(tri3(verts[5], verts[3], verts[7]));  // r1
    tris.push_back(tri3(verts[0], verts[1], verts[4]));  // b0
    tris.push_back(tri3(verts[1], verts[5], verts[4]));  // b1
    tris.push_back(tri3(verts[6], verts[7], verts[3]));  // t0
    tris.push_back(tri3(verts[6], verts[3], verts[2]));  // t1
    tris.push_back(tri3(verts[0], verts[2], verts[1]));  // n0
    tris.push_back(tri3(verts[1], verts[2], verts[3]));  // n1
    tris.push_back(tri3(verts[4], verts[6], verts[7]));  // f0
    tris.push_back(tri3(verts[4], verts[7], verts[5]));  // f1

    planes.push_back(plane(verts[4], verts[2], verts[0]));  // l
    planes.push_back(plane(verts[1], verts[3], verts[5]));  // r
    planes.push_back(plane(verts[0], verts[1], verts[4]));  // b
    planes.push_back(plane(verts[3], verts[2], verts[6]));  // t
    planes.push_back(plane(verts[0], verts[2], verts[1]));  // n
    planes.push_back(plane(verts[5], verts[7], verts[6]));  // f
    if (triangles_face_outside) {
      for (auto& tr : tris) {
        auto x = tr.v[0];
        tr.v[0] = tr.v[1];
        tr.v[1] = x;
      }
    }
  }
};

#pragma endregion
#pragma region GPU structs NON -SHARED

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

#pragma endregion
#pragma region testapp

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
  static Window* _master_context;
  static uptr<AppConfig> _appConfig;
  static std::map<GLFWwindow*, uptr<Window>> _windows;
  static std::map<ImageFormatType, uptr<ImageFormat>> _imageFormats;
  static uptr<World> _world;
  static uint64_t s_idgen;
  static std::unordered_map<std::string, sptr<Material>> _materials;

public:
  static bool isDebug();
  static World* world() { return _world.get(); }
  static AppConfig* config() { return _appConfig.get(); }
  inline static void trap() {}
  static uint64_t genId() { return s_idgen++; }
  static void initGlobals(std::string exe_path);
  static int run(int argc, char** argv);
  static ImageFormat* imageFormat(ImageFormatType fmt, bool throwinfnotfound = true);
  static Window* createWindow(int w, int h, std::string title);
  static Window* getWindow(GLFWwindow* win);
  static Window* context() { return _context; }
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
  static bool whileTrueGuard(int& x, int max = 999999999);
  static float ubtof(uint8_t c);
  static vec4 rgba_ub(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);
  static vec4 rgb_ub(uint hex);
  static vec4 rgb_ub(uint8_t r, uint8_t g, uint8_t b);
  static sptr<Material> findMaterial(const string_t& name);
  static bool fuzzyNotZero(float val, float ep = FUZZY_ZERO_EPSILON) { return glm::epsilonNotEqual(val, 0.0f, ep); }
  static bool fuzzyNotZero(double val, double ep = FUZZY_ZERO_EPSILON) { return glm::epsilonNotEqual(val, 0.0, ep); }
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
  str _name = "";
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
  str name() { return _name; }

  Image(int w, int h, ImageFormat* fmt, const char* data = nullptr);
  virtual ~Image();

  void init(int w, int h, ImageFormat* fmt, const char* data = nullptr);
  void* pixel(int32_t x, int32_t y);
  static uptr<Image> crop(Image* img, const box2i& box);
  static uptr<Image> from_file(std::string path);
  static void copy(Image* dst, const box2i& dstbox, Image* src, const box2i& srcbox);
  static uptr<Image> scale(Image* img, float s, ImageFormat* changefmt = nullptr);
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
  //
  //   void bind(int32_t channel);
  //   void unbind(int32_t channel);
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
  static uptr<TextureArray> test();

  void copyToGpu(int index, const void* data);
  void copyToGpu(int x, int y, int w, int h, int index, const void* data);
  static void conform(std::vector<uptr<Image>>& imgs);
};
class Framebuffer : public GLObject {
public:
  Framebuffer();
  virtual ~Framebuffer();
};
class Shader : public GLObject {
public:
  enum class ShaderStage { Vertex, Geometry, Fragment };
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
  class ShaderVar {
  public:
    std::string _name = "";
    int _size = 0;
    bool _hasBeenBound = false;
    ShaderVar(const str& name, size_t size) {
      _name = name;
      _size = size;
    }
    virtual ~ShaderVar() {}
  };
  class Uniform : public ShaderVar {
  public:
    GLuint _location = 0;
    GLenum _type;
    GLenum _rangeTarget;
    GLenum _bufferTarget;
    Uniform(std::string name, GLuint location, size_t size, GLenum type) : ShaderVar(name, size) {
      _location = location;
      _type = type;
    }
    virtual ~Uniform() override {}
  };
  class BufferBlock : public ShaderVar {  // UniformBlock, ShaderStorageBlock
  public:
    GLuint _blockIndex = -1;
    GLuint _bindingPoint = -1;
    GLenum _buftype;
    BufferBlock(std::string name, int blockindex, int bindpoint, size_t size, GLenum buftype) : ShaderVar(name, size) {
      _buftype = buftype;
      _bindingPoint = bindpoint;
      _blockIndex = blockindex;
    }
    virtual ~BufferBlock() override {}
  };

private:
  static uptr<ShaderMeta> _meta;

  std::vector<std::string> _vert_src;
  std::vector<std::string> _frag_src;
  std::vector<std::string> _geom_src;

  std::unordered_map<std::string, uptr<ShaderVar>> _vars;
  // std::vector<uptr<BufferBlock>> _blocks;
  // std::vector<uptr<Uniform>> _uniforms;

  std::string _name;
  ShaderLoadState _state = ShaderLoadState::None;
  GLuint _nextBufferBindingIndex = 0;

  ShaderVar* getVar(const str& name);
  void parseUniformBlocks();
  void parseUniforms();
  void parseSSBOs();

  std::vector<std::string> processSource(path_t& loc, ShaderStage);
  static GLuint compileShader(GLenum type, std::vector<std::string>& src);
  static std::string getShaderInfoLog(GLuint prog);
  static std::string getProgramInfoLog(GLuint prog);
  static void printSrc(std::vector<std::string> src);
  static std::string debugFormatSrc(std::vector<std::string> src);
  void checkDupeBindings();
  void setTextureUf(GLuint glid, GLuint channel, string_t loc);

public:
  Shader(path_t vert_src, path_t geom_src, path_t frag_src);
  ~Shader();
  void bindBlock(BufferBlock*, GpuBuffer*);
  void bindBlock(const string_t&& name, GpuBuffer*);
  void setTextureUf(Texture* tex, GLuint index, string_t loc);
  void setTextureUf(TextureArray* tex, GLuint index, string_t loc);
  void bind();
  void unbind();
  void beginRender();
  void endRender();
};
class GpuBuffer : public GLObject {
public:
  GpuBuffer();
  GpuBuffer(size_t size, const void* data = nullptr, uint32_t flags = GL_DYNAMIC_DRAW);
  virtual ~GpuBuffer();
  void copyToGpu(size_t size, const void* data, size_t offset = 0);
};
template <typename Tx>
class GpuArray : public GpuBuffer {
  size_t _count = 0;
  // GLuint _bindPoint = 0;
  // index 0-1024= system, 1024+.. but most will be sytem
public:
  size_t count() { return _count; }

  GpuArray(size_t count = 0) : GpuBuffer() {
    // note: count==0 => no size limit
    _count = count;
  }
  void copyToGpu(const std::vector<Tx>& pt) {
    Assert((_count == 0) || (_count > 0 && (pt.size() <= _count)));
    if (pt.size() == 0) {
      return;
    }
    GpuBuffer::copyToGpu(sizeof(Tx) * pt.size(), pt.data(), 0);
  }
  void copyToGpu(const Tx& pt) { GpuBuffer::copyToGpu(sizeof(Tx), &pt, 0); }
  void copyToGpu(const Tx* pt) {
    Assert(pt);
    GpuBuffer::copyToGpu(sizeof(Tx), pt, 0);
  }
};
class VertexArray : public GLObject {
public:
  VertexArray();
  virtual ~VertexArray();
  void setAttrib(int attr_idx, int attr_compsize, GLenum attr_datatype, size_t attr_offset, bool attr_inttype, int binding_index, GpuBuffer* gbuf, size_t stride);
  void bind();
  void unbind();
};
class VertexFormat {
  class VertexComponent {
  public:
    // int _index;//ordered
    int _size;
    GLenum _type;
    GLboolean _normalized;
    int _offset;
  };

public:
  void comp() {}
};
class Gpu {
public:
private:
  uptr<GpuRenderState> _last;
  int _maxTextureSize = 0;
  int _maxFragmentTextureImageUnits = 0;
  int _maxVertexTextureImageUnits = 0;

public:
  Gpu();
  int maxTextureSize() { return _maxTextureSize; }
  int maxFragmentTextureImageUnits() { return _maxFragmentTextureImageUnits; }
  int maxVertexTextureImageUnits() { return _maxVertexTextureImageUnits; }

  void setState(const GpuRenderState& rs, bool force = false);
};
class PipelineStage {
  int _width;
  int _height;

public:
  int width() { return _width; }
  int height() { return _height; }
  bool beginRender(bool forceclear);
  void endRender();
};
class Renderer {
  std::vector<uptr<PipelineStage>> _pipelineStages;
  PipelineStage* _currentStage = nullptr;
  RenderView* _currentView = nullptr;
  bool beginRenderToView(RenderView* rv);
  void endRenderToView(RenderView* rv);

public:
  void beginRenderToWindow();
  void renderViewToWindow(RenderView* rv);
  void endRenderToWindow();
};
class Picker {
public:
  void updatePickedPixel() {}
};
class RenderView {
private:
  vec2 _uv0;
  vec2 _uv1;
  string_t _name;
  uptr<Viewport> _viewport;
  sptr<Camera> _camera;
  uptr<Overlay> _overlay;
  bool _enabled = true;
  void syncCamera();
  Viewport* getClipViewport();

public:
  sptr<Camera> camera() { return _camera; }
  Viewport* viewport() { return _viewport.get(); }
  Overlay* overlay() { return _overlay.get(); }
  bool enabled() { return _enabled; }

  RenderView(string_t name, vec2 uv0, vec2 uv1, int sw, int sh);
  void setSize(vec2 uv0, vec2 uv1, int sw, int sh);
  void onResize(int sw, int sh);
  void updateDimensions(int cur_output_fbo_w, int cur_output_fbo_h);
  box2i computeScaledView(vec2 uv0, vec2 uv1, int width, int height);
  bool beginPipelineStage(PipelineStage* ps);
  void endPipelineStage(PipelineStage* ps);
};
class ContextObject {
public:
  virtual void createNew() = 0;
};
class Mesh : public VirtualMemoryShared<Mesh>, public ContextObject {
  // uptr<VertexArray> _vao;
  std::vector<sptr<GpuBuffer>> _vbos;
  sptr<GpuBuffer> _ibo;
  box3 _boundbox = box3::zero();
  PrimType _primtype;
  string_t _name;

public:
  PrimType primtype() { return _primtype; }
  // const VertexArray* vao() { return _vao.get(); }
  std::vector<sptr<GpuBuffer>>& vbos() { return _vbos; }
  GpuBuffer* ibo() { return _ibo.get(); }
  const box3& boundbox() { return _boundbox; }
  Mesh();
  Mesh(const string_t& name, PrimType pt, sptr<GpuBuffer> vertexBuffer, sptr<GpuBuffer> indexBuffer, sptr<GpuBuffer>* faceData = nullptr, bool computeBoundBox = true);
  virtual ~Mesh() override;
  void createNew() override;
};
class Material : public VirtualMemoryShared<Bobj> {
public:
  Material();
  virtual ~Material() override;
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
  vec2 _mouseLast;
  vec2 _mouse;
  void updateCursor();
  Window* _window = nullptr;

public:
  Input(Window*);
  virtual ~Input();
  void update();
  const vec2& mouseLast() { return _mouseLast; }
  const vec2& mouse() { return _mouse; }
  void addKeyEvent(int32_t key, bool press);
  bool pressOrDown(int key);
  bool press(int key);
};
class Bobj : public VirtualMemoryShared<Bobj> {
public:
  typedef std::function<void(Bobj* ob, double dt)> OnUpdateFunc;

private:
  uint64_t _id = 0;
  string_t _name = "";

protected:
  bool _visible = true;

  vec3 _pos = vec3(0, 0, 0);
  quat _rot = quat(0, 0, 0, 1);
  vec3 _scl = vec3(1, 1, 1);
  mat4 _world = mat4(1);
  vec3 _right = vec3(1, 0, 0);
  vec3 _up = vec3(0, 1, 0);
  vec3 _forward = vec3(0, 0, 1);
  std::vector<uptr<Component>> _components;
  std::vector<sptr<Bobj>> _children;
  wptr<Bobj> _parent;
  b2_objdata* _data = nullptr;
  b2_action* _action = nullptr;
  b2_frame* _frame = nullptr;
  sptr<Mesh> _mesh = nullptr;
  sptr<Material> _material = nullptr;
  box3 _boundBox = box3::one();
  box3 _boundBoxMeshAA;
  oobox3 _boundBoxMeshOO;
  GpuObj _gpuObj;
  GpuLight _gpuLight;
  vec3 _vel = vec3(0, 0, 0);
  OnUpdateFunc _onUpdate;

  void calcBoundBox(box3* parent);
  void volumizeBoundBox(box3& b);

public:
  uint64_t id() { return _id; }
  string_t& name() { return _name; }

  bool isBobj() { return _data != nullptr; }

  bool& visible() { return _visible; }
  OnUpdateFunc& onUpdate() { return _onUpdate; }

  b2_action*& action() { return _action; }
  b2_frame* frame() { return _frame; }

  vec3& pos() { return _pos; }
  quat& rot() { return _rot; }
  vec3& scl() { return _scl; }
  mat4& world() { return _world; }
  vec3 world_pos() { return glm::vec3(_world[3]); }

  vec3& up() { return _up; }
  vec3& right() { return _right; }
  vec3& forward() { return _forward; }

  box3& boundBox() { return _boundBox; }

  vec3& lightColor() { return _gpuLight._color; }
  float& lightPower() { return _gpuLight._power; }
  float& lightRadius() { return _gpuLight._radius; }
  vec3& lightDir() { return _gpuLight._dir; }

  vec3& vel() { return _vel; }

  sptr<Mesh> mesh() { return _mesh; }
  sptr<Material> material() { return _material; }

  wptr<Bobj> parent() { return _parent; }
  const std::vector<sptr<Bobj>>& children() { return _children; }
  std::vector<uptr<Component>>& components() { return _components; }

  GpuLight& gpuLight() { return _gpuLight; }
  GpuObj& gpuObj() { return _gpuObj; }

  Bobj(string_t&& name, b2_objdata* data = nullptr);
  virtual ~Bobj();
  virtual void update(double dt, mat4* parent = nullptr);
  void addChild(sptr<Bobj> ob);
  // uptr<Bobj> removeChild(uptr<Bobj* ob);
  void lookAt(const vec3&& at);
};
class Viewport : public box2i {
public:
  virtual ~Viewport() override {}
  bool Contains_Point_Window_Relative_BR_Exclusive(const vec2& win_rel) {
    // note:  viewports aTOp left coords, opengl we convert to bottom left
    return (win_rel.x >= x()) && (win_rel.y >= y()) && (win_rel.x < (x() + width())) && (win_rel.y < (y() + height()));
  }
};
class Frustum {
  const int fpt_nbl = 0;
  const int fpt_fbl = 1;
  const int fpt_fbr = 2;
  const int fpt_nbr = 3;
  const int fpt_ntl = 4;
  const int fpt_ftl = 5;
  const int fpt_ftr = 6;
  const int fpt_ntr = 7;
  const int fp_near = 0;
  const int fp_far = 1;
  const int fp_left = 2;
  const int fp_right = 3;
  const int fp_top = 4;
  const int fp_bottom = 5;

  // Camera3D _camera;
  float _widthNear = 1;
  float _heightNear = 1;
  float _widthFar = 1;
  float _heightFar = 1;
  // plane[] _planes = new Plane3f[6];
  vec3 _nearCenter = vec3(0, 0, 0);
  vec3 _farCenter = vec3(0, 0, 0);
  vec3 _nearTopLeft = vec3(0, 0, 0);
  vec3 _farTopLeft = vec3(0, 0, 0);
  box3 _boundBox = box3(vec3(0, 0, 0), vec3(1, 1, 1));
  vec3 _points[8];
  plane _planes[6];
  void constructPointsAndPlanes(vec3 farCenter, vec3 nearCenter, vec3 upVec, vec3 rightVec, float w_near_2, float w_far_2, float h_near_2, float h_far_2);

public:
  Frustum();
  virtual ~Frustum();
  void update(Camera* cam);
  bool hasBox(const box3& pCube);
};
class Camera : public Bobj {
  float _fov = 40.0f;
  float _far = 1000.0f;
  float _near = 1.0f;
  mat4 _proj;
  mat4 _view;
  uptr<Viewport> _computedViewport;
  uptr<Frustum> _frustum;

public:
  float& fov() { return _fov; }
  float& near() { return _near; }
  float& far() { return _far; }
  mat4& proj() { return _proj; }
  mat4& view() { return _view; }
  Frustum* frustum() { return _frustum.get(); }
  Viewport* viewport() { return _computedViewport.get(); }

  Camera(string_t&& name);
  void updateViewport(int width, int height);
  virtual void update(double dt, mat4* parent = nullptr) override;
  void computeView(RenderView* rv);
};
class GpuRenderState {
public:
  // State switches to prevent unnecessary gpu context changes.
  bool depthTestEnabled = true;
  bool cullFaceEnabled = true;
  bool scissorTestEnabled = true;
  bool blendEnabled = false;
  GLenum blendFactor = GL_ONE_MINUS_SRC_ALPHA;  // BlendingFactor.OneMinusSrcAlpha;
  GLenum blendFunc = GL_FUNC_ADD;               // BlendEquationMode.FuncAdd;
  GLenum frontFaceDirection = GL_CCW;           // FrontFaceDirection.Ccw;
  bool depthMask = true;                        // enable writing to depth bufer
  GLenum cullFaceMode = GL_BACK;                // CullFaceMode _cullFaceMode = CullFaceMode.Back;
};
class Component {
public:
  virtual void update(Bobj* obj, float dt) {}
};
class InputController : public Component {
  float _speed = 10;
  float _sspeed = 6;
  float _rspeed = 0.5f;

public:
  float& speed() { return _speed; }
  float& rspeed() { return _rspeed; }
  float& sspeed() { return _sspeed; }

  InputController();
  virtual void update(Bobj* obj, float dt) override;
};
class WorldTime {
private:
  double _start = 0;
  double _last = 0;
  double _delta = 0;

public:
  double elapsed() { return glfwGetTime() - _start; }
  double delta() { return _delta; }
  double modSeconds(double seconds) { return fmod(elapsed(), seconds) / seconds; }
  WorldTime() {
    _start = _last = glfwGetTime();
    update();
  }
  double update() {
    auto cur = glfwGetTime();
    _delta = (float)(cur - _last);
    _last = cur;

    double LIMIT_DELTA = 1.0;
    if (_delta > LIMIT_DELTA) {
      LogWarn("Delta " + _delta + " > " + LIMIT_DELTA + ". (possibly paused in debug mode)");
      _delta = LIMIT_DELTA;
    }
    return _delta;
  }
};
class World {
  const int c_MetaFileVersionMajor = 0;
  const int c_MetaFileVersionMinor = 2;

  std::vector<b2_mtex> _mtexs;
  std::vector<b2_objdata> _objdatas;
  uptr<TextureArray> _objtexs;  // we're going to use arrays
  sptr<Bobj> _root;
  sptr<Camera> _activeCamera;
  uptr<VisibleStuff> _visibleStuff;
  uptr<WorldTime> _time;
  uptr<GpuWorld> _gpuWorld;

  void loadD26Meta(path_t);

public:
  Bobj* root() { return _root.get(); }
  TextureArray* objtexs() { return _objtexs.get(); }
  sptr<Camera> activeCamera() { return _activeCamera; }
  VisibleStuff* visibleStuff() { return _visibleStuff.get(); }
  std::vector<b2_mtex>& bobjTexs() { return _mtexs; }
  WorldTime* time() { return _time.get(); }
  GpuWorld* gpuWorld() { return _gpuWorld.get(); }

  World();
  void update();
  void cull(RenderView* rv, Bobj* ob = nullptr);
  uptr<Bobj> createOb(const string_t& name, b2_objdata* dat);
  void pick();
  void renderPipeStage(RenderView* rv, PipelineStage* ps);
};
class VisibleStuff {
public:
  std::map<RenderView*, std::unordered_set<Bobj*>> _stuff;
  void clear() { _stuff.clear(); }
  void addObject(RenderView* rv, Bobj* ob) {
    auto f = _stuff.find(rv);
    if (f == _stuff.end()) {
      _stuff.insert(std::make_pair(rv, std::unordered_set<Bobj*>()));
      f = _stuff.find(rv);
    }
    Assert(f->second.find(ob) == f->second.end());
    f->second.insert(ob);
  }
};
class Window {
public:
  enum class WindowState { Created, Running, Quit };

private:
  int _lastx = 0, _lasty = 0, _lastw = 0, _lasth = 0;
  bool _fullscreen = false;
  int _width = 0;
  int _height = 0;
  GLFWwindow* _window = nullptr;
  WindowState _state = WindowState::Created;
  uptr<Input> _input;
  uptr<DrawQuads> _drawQuads;
  std::vector<uptr<RenderView>> _views;
  uptr<Shader> _objShader;
  uptr<GpuArray<GpuObj>> _objBuf;
  uptr<GpuArray<GpuLight>> _lightBuf;
  uptr<GpuArray<GpuWorld>> _worldBuf;
  uptr<GpuArray<GpuViewData>> _dataBuf;
  uptr<VertexArray> _objVao;
  uptr<Texture> _testTex;
  uptr<Renderer> _renderer;
  uptr<Picker> _picker;
  uptr<Gpu> _gpu;

  bool _focused = false;
  bool _minimized = false;
  bool _visible = false;

  void toggleFullscreen();
  void initFramebuffer();
  void initEngine();
  void createRenderView(vec2 xy, vec2 wh);

public:
  Input* input() { return _input.get(); }
  WindowState state() { return _state; }
  GLFWwindow* glfwWindow() { return _window; }
  bool focused() { return _focused; }
  bool minimized() { return _minimized; }
  bool visible() { return _visible; }
  Renderer* renderer() { return _renderer.get(); }
  Picker* picker() { return _picker.get(); }
  Gpu* gpu() { return _gpu.get(); }
  int width() { return _width; }
  int height() { return _height; }

  Window();
  virtual ~Window();
  void init(int, int, std::string, GLFWwindow* share = nullptr);

  void updateState();
  void pick();
  void cullViews();
  void updateSelectedView() {}
  void renderViews();
  void updateInput();
  void swap();

  void on_resize(int w, int h);
  void quit_everything();
};
class AppConfig {
public:
  bool EnableDebug = true;  // enables debugging
  bool BreakOnGLError = true;
  bool Debug_Print_Shader_Details_Verbose = true;
};

#pragma endregion
#pragma region B26

class b2_objdata {
public:
  int32_t _id = -1;
  std::string _name = "";
  std::vector<b2_action> _actions;

  std::vector<b2_action>& actions() { return _actions; }
  void deserialize(BinaryFile* bf);
};
class b2_action {
public:
  int32_t _id = -1;
  std::string _name = "";
  std::vector<b2_frame> _frames;

  std::vector<b2_frame>& frames() { return _frames; }
  void deserialize(BinaryFile* bf);
};
class b2_frame {
public:
  float _seq = -1;
  std::string _name = "";
  int32_t _mtexid = -1;
  int32_t _x = -1;
  int32_t _y = -1;
  int32_t _w = -1;
  int32_t _h = -1;

  vec4 texpos() const { return vec4(_x, _y, _w, _h); }

  void deserialize(BinaryFile* bf);
};
class b2_mtex {
public:
  int32_t _texid = -1;
  int32_t _w;
  int32_t _h;
  std::vector<std::string> _images;
  void deserialize(BinaryFile* bf);
};

#pragma endregion
#pragma region Post Templates

template <typename Tx, int Dx>
bool box2x<Tx, Dx>::validate(bool debug_break, float min_volume) const {
  // Validate and correct box
  // specify 0 to not check volume.
  bool wasvalid = true;
  if (_min.x > _max.x) {
    if (debug_break) {
      Gu::debugBreak();
    }
    //_min.x = _max.x;
    wasvalid = false;
  }
  if (_max.y < _min.y) {
    if (debug_break) {
      Gu::debugBreak();
    }
    //_max.y = _min.y;
    wasvalid = false;
  }
  if (min_volume > 0) {
    // wasvalid = wasvalid && ValidateVolume(min_volume);
  }

  if (!wasvalid && debug_break) {
    Gu::debugBreak();
  }

  return wasvalid;
}

template <typename Tx>
void GrowList<Tx>::push_back(Tx item) {
  if (_count == _items.size()) {
    if (_count >= _maxcount) {
      LogError("grow list exceeded max count " + _maxcount);
      Gu::debugBreak();
    }
    else {
      _items.push_back(item);
      _count++;
    }
  }
  else {
    _items[_count] = item;
    _count++;
  }
}

#pragma endregion

}  // namespace B26D

#endif