# python -m ensurepip
# python -m pip install glfw
# python -m pip install PyOpenGL
# python -m pip install PyGLM ** *NOT glm


from operator import attrgetter
import struct
import pickle
import traceback
import datetime
import time
from inspect import getframeinfo, stack
import math
from PIL import Image
from array import array
# from OpenGL.arrays
# from OpenGL.arrays import arraydatatype
import os
import sys
import random
import glm
import glfw
import OpenGL  # nopep8
OpenGL.ERROR_CHECKING = True  # nopep8
OpenGL.ERROR_LOGGING = True  # nopep8
OpenGL.FULL_LOGGING = True  # nopep8
OpenGL.ERROR_ON_COPY = True  # npep8
from OpenGL.GL import *  # nopep8
# from OpenGL.arrays import * #nopep8
from dataclasses import dataclass

#marshalling from python is a nightmare - so far
# i can't even find working examples of 4.5 opengl but got byrefs to work using
# some of the pyopengl lib cstruct.byref
# 
# # @dataclass
# # class Point:
# #   x:float = 4
# #   y:float = 1
# # 
# # p = Point(0,1)
# # 
# # print(str(sys.getsizeof(p)))
# # #b = bytes(p)
# # arr = [Point(1,5), Point(9,10)]
# # print(str(sys.getsizeof(arr)))
# # #b2 = bytes(arr)
# #https://stackoverflow.com/questions/18082211/unpacking-nested-c-structs-in-python
# import construct as cstruct
# t_point3 = cstruct.Struct(
#   'x' / glm.vec2, 
#   'y' / glm.vec2,
# )
# 
# t_quad = cstruct.Struct(
#   'tl' / t_point3, 
#   'tr' / t_point3,
#   'bl' / t_point3,
#   'br' / t_point3,
# )
# 
# fmt = t_point3.build(dict(x=1,y=1))
# 
# #xxx.setattr('y',0)
# print(str(fmt))
# #print(str())
# 
# print(str(t_quad.sizeof()))
# arr = cstruct.Array(10, t_quad())
# print(str(arr.sizeof()))
# print(str(arr.count))
# #cstruct.buil
# 
# 
# pt3 = t_point3
# #pt3.tl.x = 4
# #pt3.tl.y = 1
# #print(str(pt3))
# 
# #bt=bytearray(arr)
# 
# 
# 
# print(str(sys.getsizeof(p)))
# #b = bytes(p)
# arr = [Point(1,5), Point(9,10)]
# print(str(sys.getsizeof(arr)))
# #b2 = bytes(arr)


# https://stackoverflow.com/questions/6892570/python-byte-buffer-object
# "buffer protocol"
# .ctypesparameters
#https://stackoverflow.com/questions/36906222/how-do-i-construct-an-array-of-ctype-structures
# 
# class Pack:
#   @staticmethod
#   def pack_floats(data): return array("f", data).tobytes()
#   def pack_uints(data): return array("I", data).tobytes()
#   def pack_bytes(data): return array("B", data).tobytes()
# 
#   def pack(obj):
#     ba = bytearray()
#     Pack._pack(obj, ba)
#     return bytes(ba)
# 
#   def _pack(obj, ba):
#     # https://docs.python.org/3/library/struct.html#format-characters
#     if isinstance(obj, list):
#       for item in obj:
#         Pack._pack(item, ba)
#     else:
#       assert (obj.__slots__)
#       for xx in obj.__slots__:
#         attr = getattr(obj, xx)
#         if isinstance(attr, list):
#           for item in attr:
#             Pack._pack(item, ba)
#         elif isinstance(attr, Byte): ba.extend(struct.pack('B', attr))
#         elif isinstance(attr, Double): ba.extend(struct.pack('d', attr))
#         elif isinstance(attr, float): ba.extend(struct.pack('f', attr))
#         elif isinstance(attr, int): ba.extend(struct.pack('i', attr))
#         elif isinstance(attr, Long): ba.extend(struct.pack('q', attr))
#         else: ba.extend(attr.to_bytes())
# 
# 
# class GPUStruct():
#   __slots__ = ()
#   def __bytes__(self):
#     return Pack.pack(self)
# 
# class Byte(GPUStruct):
#   __slots__ = 'val',
# class Double(GPUStruct):
#   __slots__ = 'val',
# class Long(GPUStruct):
#   __slots__ = 'val',

#@dataclass


class QuadVert(ctypes.Structure):
  #__slots__ = '_v', '_pad', '_x', '_c',
  _fields_ = [('_v', glm.vec3), ('_pad', glm.float32), ('_x', glm.vec2), ('_c', glm.vec4)]
  def __init__(self, pos: glm.vec3, tex: glm.vec2, color: glm.vec4):
    self._v = pos
    self._pad = 0.0
    self._x = tex
    self._c = color

class Log:
  _start_time = time.time()
  CC_BLACK = "\033[30"
  CC_RED = "\033[31"
  CC_GREEN = "\033[32"
  CC_YELLOW = "\033[33"
  CC_BLUE = "\033[34"
  CC_PINK = "\033[35"
  CC_CYAN = "\033[36"
  CC_WHITE = "\033[37"
  CC_NORMAL = "\033[0;39"
  @staticmethod
  def cc_reset(): return "\033[0m"
  @staticmethod
  def cc_color(color, bold=False): return str(color + (";1" if bold else "") + "m")
  @staticmethod
  def err(s): Log._output(Log.CC_RED, False, "E", s)
  @staticmethod
  def dbg(s): Log._output(Log.CC_CYAN, False, "D", s)
  @staticmethod
  def msg(s): Log._output(Log.CC_WHITE, False, "I", s)
  @staticmethod
  def exception(ex):
    Log.err(str(ex))
    tbout = Log.cc_color(Log.CC_WHITE, True)+str(traceback.format_exc())+Log.cc_reset()
    Log._print(tbout)
  @staticmethod
  def _output(color, bold, type, s):
    s = Log._header(color, bold, type)+s+Log.cc_reset()
    Log._print(s)
  @staticmethod
  def _print(s):
    print(s, flush=True)
  @staticmethod
  def _header(color, bold: bool, type):
    secs = time.time() - Log._start_time
    dt = str(datetime.timedelta(seconds=secs))
    h = f"{Log.cc_color(color, bold)}[{dt}][{type}]"
    return h

class RenderQuad(ctypes.Structure):
  _fields_=[('_verts', ctypes.Array)]
  def __init__(self):
    c = glm.vec4(1.0, 0.0, 1.0, 1.0)
    self._verts = [
        QuadVert(glm.vec3(-1, -1, 0), glm.vec2(0, 0), c),
        QuadVert(glm.vec3(1, -1, 0), glm.vec2(1, 0), c),
        QuadVert(glm.vec3(1, 1, 0), glm.vec2(0, 1), c),
        QuadVert(glm.vec3(-1, 1, 0), glm.vec2(1, 1), c),
    ]
class MainWindow:
  def __init__(self, w, h, title):
    Log.msg(f"Initializing {w} {h}")
    self._width = w
    self._height = h
    if not glfw.init():
      raise Exception("Failed to init glfw")
    glfw.window_hint(glfw.RESIZABLE, True)
    glfw.window_hint(glfw.CONTEXT_VERSION_MAJOR, 4)
    glfw.window_hint(glfw.CONTEXT_VERSION_MINOR, 5)
    glfw.window_hint(glfw.SAMPLES, 1)
    glfw.window_hint(glfw.CLIENT_API, glfw.OPENGL_API)
    glfw.window_hint(glfw.OPENGL_FORWARD_COMPAT, GL_TRUE)
    glfw.window_hint(glfw.OPENGL_PROFILE, glfw.OPENGL_CORE_PROFILE)
    glfw.window_hint(glfw.DOUBLEBUFFER, True)
    glfw.window_hint(glfw.DEPTH_BITS, 24)

    self._context = glfw.create_window(w, h, title, None, None)
    if not self._context:
      glfw.terminate()
      raise Exception("Failed to create glfw window.")

    glfw.make_context_current(self._context)
    # print("has45="+str(OpenGL.extensions.hasGLExtension( 'GL_VERSION_GL_4_5' )))

    glfw.swap_interval(1)
    glfw.set_framebuffer_size_callback(self._context, self.on_resize)
    # glfw.set_cursor_pos_callback(window, mouse_callback)
    # glfw.set_scroll_callback(window, scroll_callback)
    # glfw.set_input_mode(self._context, glfw.CURSOR, glfw.CURSOR_DISABLED)

  def run(self):
    self.createRenderEngine()
    running = True
    while running:
      self.render()
      glfw.swap_buffers(self._context)
      glfw.poll_events()

  def createRenderEngine(self):
    Log.msg("GL version: " + glGetString(GL_VERSION).decode('utf-8'))

    glEnable(GL_DEPTH_TEST)
    glEnable(GL_SCISSOR_TEST)
    glEnable(GL_CULL_FACE)
    glFrontFace(GL_CCW)
    glEnable(GL_BLEND)
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)
    glEnable(GL_DEBUG_OUTPUT)
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS)
    glScissor(0, 0, self._width, self._height)
    glClearColor(0.89, 0.91, 0.91, 1.0)

    Gu.checkErrors()

    # tex
    imname = "kratos.jpg"
    assets_dir = "../assets/"
    fpath = os.path.join(os.path.dirname(os.path.realpath(__file__)))
    assets_dir = os.path.join(fpath, assets_dir)
    impath = os.path.join(assets_dir, imname)
    assert (os.path.exists(impath))
    with Image.open(impath) as img:
      Log.msg(f"Loaded {imname} w={img.width} h={img.height}")
      self._tex = 0
      # there is actually no way to use the 4.5 API here
      # glCreateTextures(GL_TEXTURE_2D, 1, byref(self._tex))
      self._tex = glGenTextures(1)
      Gu.checkErrors()
      glBindTexture(GL_TEXTURE_2D, self._tex)
      Gu.checkErrors()
      print("tex="+str(self._tex))
      n = int(min(img.width, img.height))
      mips = 0
      while n > 16:
        n = int(n/2)
        mips += 1
      # glPixelStorei(GL_UNPACK_ALIGNMENT, 1)
      Gu.checkErrors()
      glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, img.width, img.height)
      Gu.checkErrors()
      # glTexSubImage2D(self._tex, 0, 0, 0, img.width, img.height, GL_RGBA, GL_UNSIGNED_BYTE, img.tostring("raw", "RGBA", 0, -1))
      # glTextureSubImage2D(self._tex, 0, 0, 0, img.width, img.height, GL_RGBA, GL_UNSIGNED_BYTE, img.tostring("raw", "RGBA", 0, -1))
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, img.width, img.height, GL_RGBA, GL_UNSIGNED_BYTE, fr"{img.tobytes()}")
      Gu.checkErrors()
      # glTextureSubImage2D(self._tex, 0, 0, 0, img.width, img.height, GL_RGBA, GL_UNSIGNED_BYTE, Pack.pack_byte(img.tobytes()))
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0)
      Gu.checkErrors()
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, max(0, mips - 1))
      Gu.checkErrors()
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR)
      Gu.checkErrors()
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR)
      Gu.checkErrors()
      glBindTexture(GL_TEXTURE_2D, 0)

    Gu.checkErrors()

    #tv = QuadVert(glm.vec3(0, 0, 0), glm.vec2(0, 0), glm.vec4(0, 0, 0, 0))
    #rq = RenderQuad()
    #structs = [A(1, 2), A(1, 2)]
    #arr = (A * 2)(*structs)
    #print("btsqv="+str(len(bytes(tv))))
    #print("pcl="+str(len(pickle.dumps(rq))))
    #print("bts(Rq)="+str(len(bytes(rq))))
    #print("pack(Rq)="+str(len(Pack.pack(rq))))

    # geom
    self._quads = [RenderQuad()]

    
    # bufs
    vbo_binding_idx = 0
    self._vbo = GLuint()
    glCreateBuffers(1, ctypes.byref(self._vbo))
    vertdata = (RenderQuad * len(self._quads))(*self._quads) # Pack.pack(self._quads)
    glNamedBufferStorage(self._vbo, len(vertdata), vertdata, vbo_binding_idx)

    ibo_binding_idx = 1
    self._ibo = GLuint()
    glCreateBuffers(1, ctypes.byref(self._ibo))
    inds = [0, 1, 3, 0, 3, 2]
    #indexdata = Pack.pack_uints([0, 1, 3, 0, 3, 2])
    #glm.ivec1(0)
    #a = glm.array([0, 1, 3, 0, 3, 2])
    #from OpenGL.arrays import GLuintArray
    #arr = GLuintArray.asArray([0,1,3,0,3,2])
    indexdata = (ctypes.c_uint32 * len(inds))(*inds)
    #Log.msg("arrcount="+str(len(arr)))
    glNamedBufferStorage(self._ibo, len(indexdata), ctypes.byref(indexdata), ibo_binding_idx)

    Gu.checkErrors()

    # arr = GLuintArray.asArray(1)
    # x = GLuintArray.asArray([0,0,0,0,0])
    # note to convert back to python use list(arr), or ctype.value
    self._vao = GLuint()
    glCreateVertexArrays(1, ctypes.byref(self._vao))

    # vao
    v_idx = 0
    x_idx = 1
    c_idx = 2
    glEnableVertexAttribArray(x_idx)
    glEnableVertexAttribArray(v_idx)
    glEnableVertexAttribArray(c_idx)

    glEnableVertexArrayAttrib(self._vao, int(v_idx))
    glEnableVertexArrayAttrib(self._vao, int(x_idx))
    glEnableVertexArrayAttrib(self._vao, int(c_idx))

    glVertexArrayAttribFormat(self._vao, v_idx, 3, GL_FLOAT, GL_FALSE, 0)
    glVertexArrayAttribFormat(self._vao, x_idx, 2, GL_FLOAT, GL_FALSE, glm.sizeof(glm.vec4))
    glVertexArrayAttribFormat(self._vao, c_idx, 4, GL_FLOAT, GL_FALSE, glm.sizeof(glm.vec4)+glm.sizeof(glm.vec2))

    glVertexArrayAttribBinding(self._vao, v_idx, vbo_binding_idx)
    glVertexArrayAttribBinding(self._vao, x_idx, vbo_binding_idx)
    glVertexArrayAttribBinding(self._vao, c_idx, vbo_binding_idx)

    qvsize = glm.sizeof(glm.vec4)+glm.sizeof(glm.vec4)+glm.sizeof(glm.vec2)
    glVertexArrayVertexBuffer(self._vao, vbo_binding_idx, self._vbo, 0, qvsize)
    glVertexArrayElementBuffer(self._vao, self._ibo)

    Gu.checkErrors()

    # ortho = glm.ortho(0,self._width, self._height, 0, 0, 1)
    cpos = glm.vec3(0, 0, -10)
    up = glm.cross(cpos, glm.vec3(1, 0, 0))
    Log.msg("up="+str(up))
    self._view = glm.lookAt(glm.vec3(0, 0, -10), glm.vec3(0, 0, 0), up)
    self._model = glm.identity(glm.mat4)
    self.initFramebuffer()

    # shader
    self.makeProgram()

    Gu.checkErrors()

  def on_resize(self, win, w, h):
    self._width = w
    self._height = h
    self.initFramebuffer()

  def initFramebuffer(self):
    glViewport(0, 0, self._width, self._height)
    self._proj = glm.perspectiveFov(40, self._width, self._height, 1.0, 1000.0)
    # todo: update fbos


  def render(self):
    model_loc = glGetUniformLocation(self._shader, "_modelMatrix")
    view_loc = glGetUniformLocation(self._shader, "_viewMatrix")
    proj_loc = glGetUniformLocation(self._shader, "_projMatrix")
    tex_loc = glGetUniformLocation(self._shader, "_texture")
    Gu.checkErrors()

    glProgramUniformMatrix4fv(self._shader, model_loc, 1, GL_FALSE, glm.value_ptr(self._model))
    glProgramUniformMatrix4fv(self._shader, view_loc, 1, GL_FALSE, glm.value_ptr(self._view))
    glProgramUniformMatrix4fv(self._shader, proj_loc, 1, GL_FALSE, glm.value_ptr(self._proj))
    Gu.checkErrors()

    glBindTextureUnit(0, self._tex)
    glProgramUniform1f(self._shader, tex_loc, 0)

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
    Gu.checkErrors()
    glBindVertexArray(self._vao)
    glUseProgram(self._shader)
    Gu.checkErrors()

    glDrawElements(GL_TRIANGLES, len(self._quads) * 2, GL_UNSIGNED_INT, None)
    glBindVertexArray(0)
    glUseProgram(0)

    Gu.checkErrors()

  def makeProgram(self):
    vert_src = """
      #version 450
      layout(location = 0) in vec3 _v3;
      layout(location = 1) in vec2 _x2;
      layout(location = 2) in vec4 _c4;

      uniform mat4 _projMatrix;
      uniform mat4 _viewMatrix;
      uniform mat4 _modelMatrix;

      out vec2 _tcoord;
      out vec4 _color;

      void main() {
        _tcoord = _x2;
        _color = _c4;
        vec4 vert = _modelMatrix * _viewMatrix * _projMatrix * vec4(_v3, 1);
        gl_Position = vert;
      }
    """
    frag_src = """#version 450
      
      layout(binding=0) uniform sampler2D tex0;
      
      in vec2 _tcoord;
      in vec4 _color;

      out vec4 _output;

      void main() {
        _output = texture(tex0, _tcoord) * _color; 
      }
    """
    self._shader = Gu.createShader(vert_src, "", frag_src)

class Gu:
  @staticmethod
  def createShader(vert_src, geom_src, frag_src):
    prog = glCreateProgram()
    assert (vert_src)
    assert (frag_src)
    vert = Gu.compileShader(GL_VERTEX_SHADER, vert_src)
    frag = Gu.compileShader(GL_FRAGMENT_SHADER, frag_src)
    glAttachShader(prog, vert)
    glAttachShader(prog, frag)
    if geom_src:
      geom = Gu.compileShader(GL_GEOMETRY_SHADER, geom_src)
      glAttachShader(prog, geom)
    else:
      geom = None
    glLinkProgram(prog)
    if not glGetProgramiv(prog, GL_LINK_STATUS):
      info = glGetProgramInfoLog(prog).decode('UTF-8')
      print(str(info))
      glDeleteProgram(prog)
      Log.dbg(info)
      raise Exception('Failed to link program')
    glDetachShader(prog, vert)
    glDetachShader(prog, frag)
    if geom:
      glDetachShader(prog, geom)
    return prog

  @staticmethod
  def compileShader(type, src):
    shader = glCreateShader(type)
    glShaderSource(shader, src)
    glCompileShader(shader)
    status = glGetShaderiv(shader, GL_COMPILE_STATUS)
    if not status:
      info = glGetShaderInfoLog(shader).decode('utf-8')
      Log.dbg(info)
      raise Exception("Failed to compile "+str(type))
    return shader

  @staticmethod
  def checkErrors():
    n = glGetError()
    err = ""
    while n != GL_NO_ERROR:
      err += "GL Error: " + str(n)+"\n"
      n = glGetError()

    err += Gu.printDebugMessages()
    if err:
      caller = getframeinfo(stack()[1][0])
      err += caller.filename+":"+str(caller.lineno)
      Log.err(err)

  @staticmethod
  def printDebugMessages():
    ret = ""
    count = glGetIntegerv(GL_DEBUG_LOGGED_MESSAGES)
    if count > 0:
      max_size = int(glGetIntegerv(GL_MAX_DEBUG_MESSAGE_LENGTH))
      source = GLenum()
      type = GLenum()
      id = GLenum()
      severity = GLenum()
      length = GLsizei()
      buffer = ctypes.create_string_buffer(max_size)
      for i in range(count):
        result = glGetDebugMessageLog(1, max_size, source, type, id, severity, length, buffer)
        if result:
          msg = f"""
              [{type.value}][{severity.value}][{source.value}]:
              {buffer[:length.value]}
          """
          ret += msg+"\n"
    return ret

# ======================================================================


try:
  g_mw = MainWindow(300, 300, '2.6D Test')
  g_mw.run()
except Exception as ex:
  Log.exception(ex)

glfw.terminate()
