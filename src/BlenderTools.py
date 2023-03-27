
# submodules
# https://stackoverflow.com/questions/14233939/git-submodule-commit-push-pull

# make sure to put packages in setup.py
import os
import math
import time
import sys
import traceback
import builtins
import zlib
import struct
import imghdr
import bpy
from mathutils import Vector, Matrix, Euler

# region fwddcl
class ivec2: pass  # nopep8
class Box2i: pass  # nopep8
class BoundBox: pass  # nopep8
class FastBuffer: pass  # nopep8
class KeySamples: pass  # nopep8
class Utils: pass  # nopep8
class Convert: pass  # nopep8
# endregion
# region Globals
def msg(str): _msg("[I] " + str)
def dbg(str): _msg("[D] " + str)
def _msg(str):
  builtins.print(str)
  sys.stdout.flush()
  time.sleep(0)
def throw(ex):
  raise Exception("Exception: " + ex)
# endregion
# region math
class ivec2:
  def __init__(self, dx, dy):
    self.x = int(dx)
    self.y = int(dy)
  def __str__(self):
    return "" + str(self.x) + " " + str(self.y)
class Box2i:
  def left(self): return self._min.x
  def top(self): return self._min.y
  def right(self): return self._max.x
  def bottom(self): return self._max.y
  def width(self): return self._max.x - self._min.x
  def height(self): return self._max.y - self._min.y
  def construct(self, left, top, right, bot):
    self._min = ivec2(left, top)
    self._max = ivec2(right, bot)
class BoundBox:
  # region static methods
  def zero():
    a = Vector([0, 0, 0])
    i = Vector([0, 0, 0])
    bb = BoundBox(a, i)
    return bb
  def fromBlender(bbox):
    a = Vector([bbox[0][0], bbox[0][1], bbox[0][2]])
    i = Vector([bbox[1][0], bbox[1][1], bbox[1][2]])
    bb = BoundBox(a, i)
    return bb
  # endregion
  def __init__(self, min, max):
    self._min = Vector(min)
    self._max = Vector(max)
  def volume(self):
    v = self._max - self._min
    return v.x * v.y * v.z
  def size(self):
    return (self._min - self._max).length
  def genReset(self):
    self._min.x = sys.float_info.max
    self._min.y = sys.float_info.max
    self._min.z = sys.float_info.max
    self._max.x = -sys.float_info.max
    self._max.y = -sys.float_info.max
    self._max.z = -sys.float_info.max
  def genExpandByBox(self, bbox):
    self.genExpandByPoint(bbox._min)
    self.genExpandByPoint(bbox._max)
  def genExpandByPoint(self, pt):
    self._max.x = max(self._max.x, pt.x)
    self._max.y = max(self._max.y, pt.y)
    self._max.z = max(self._max.z, pt.z)
    self._min.x = min(self._min.x, pt.x)
    self._min.y = min(self._min.y, pt.y)
    self._min.z = min(self._min.z, pt.z)
  def validate(self):
    assert(self._max.x >= self._min.x)
    assert(self._max.y >= self._min.y)
    assert(self._max.z >= self._min.z)
  def center(self):
    return self._min + (self._max - self._min) / 2.0
  def clone(self):
    bb = BoundBox(self._min, self._max)  # Vector([self._min.x, self._min.y, self._min.z]), Vector([self._max.x, self._max.y, self._max.z]))
    return bb
  def __str__(self):
    return Convert.vec3ToString(self._min) + " " + Convert.vec3ToString(self._max)
# endregion
# region utils

class BinaryFile:
  def __init__(self, file):
    assert(file != None)
    self._binFile = file
    self._block = None
    self._blockname = None
    self._bytesWritten = 0
  def bytesWritten(self): return self._bytesWritten
  def writeString(self, str):
    # https://docs.python.org/3/library/struct.html
    bts = bytes(str, 'utf-8')
    self.writeInt32(len(bts))
    self.writeData(bts)
  def writeBool(self, val: bool):
    self.writeData(struct.pack('?', val))
  def writeByte(self, val: int):
    # unsigned char..
    self.writeData(struct.pack('B', val))
  def writeInt16(self, val: int):
    self.writeData(struct.pack('h', val))
  def writeUInt16(self, val: int):
    self.writeData(struct.pack('H', val))
  def writeInt32(self, val: int):
    self.writeData(struct.pack('i', val))
  def writeUInt32(self, val: int):
    self.writeData(struct.pack('I', val))
  def writeInt64(self, val: int):
    self.writeData(struct.pack('q', val))
  def writeUInt64(self, val: int):
    self.writeData(struct.pack('Q', val))
  def writeFloat(self, val: float):
    self.writeData(struct.pack('f', val))
  def writeDouble(self, val: float):
    self.writeData(struct.pack('d', val))
  def writeData(self, data: bytearray):
    # append to the current block, or, none
    if self._block != None:
      self._block.extend(data)  # append?
    else:
      self._binFile.write(data)
      self._bytesWritten += len(data)
  def writeMat4(self, val):
    # mat_4 = val.to_4x4()
    for row in range(4):
      for col in range(4):
        self.writeFloat(val[row][col])
    return
  def writeVec2(self, val):
    self.writeFloat(val[0])
    self.writeFloat(val[1])
    return
  def writeIVec2(self, val):
    self.writeInt32(val[0])
    self.writeInt32(val[1])
    return
  def writeVec3(self, val):
    self.writeFloat(val[0])
    self.writeFloat(val[1])
    self.writeFloat(val[2])
    return
  def writeVec4(self, val):
    self.writeFloat(val[0])
    self.writeFloat(val[1])
    self.writeFloat(val[2])
    self.writeFloat(val[3])
    return
  def writeQuat(self, val):
    self.writeVec4(val)
    return
  def writeMatrixPRS(self, mat):
    loc, rot, sca = mat.decompose()
    self.writeVec3(Convert.glVec3(loc))
    self.writeVec4(Convert.glQuat(rot))
    self.writeVec3(Convert.glVec3(sca))
  def writeNodeID(self, val):
    self.writeUInt32(val)
  def writeBoneID(self, val):
    self.writeUInt16(val)
  def startBlock(self, blockname):
    if self._block != None:
      throw("Tried to start a new block '" + blockname + "' within current block")
    self._block = bytearray()
    self._blockname = blockname
  def endBlock(self):
    if self._block == None:
      throw("Tried to end a not started block")
    cmp = zlib.compress(self._block)
    self.writeString(self._blockname)
    self.writeBool(True)  # Always compressing blocks in this one
    crc = zlib.crc32(cmp)
    self.writeUInt32(crc)
    self.writeUInt32(len(cmp))
    self._binFile.write(cmp)
    # msg("[" + self._blockname + "] " + str(len(self._block)) + " -> " + str(len(cmp)))
    self._block = None
    self._blockname = None

    # with open(binpath, 'wb') as self._binFile:

class FastBuffer:
  def __init__(self):
    self._chunksize = 8192
    self._buffer = bytearray(self._chunksize)
    self._index = 0
  def buffer(self): return self._buffer

  def pack(self, bytes: bytearray):
    newsize = self._index + len(bytes)
    if newsize > len(self._buffer):
      chunks = (newsize - len(self._buffer)) / self._chunksize
      newsize = int(chunks) * self._chunksize
      self._buffer.extend(newsize)
    self._buffer[self._index: self._index + len(bytes)] = bytes

  def packVec2(self, val):
    self.packFloat(val[0])
    self.packFloat(val[1])

  def packVec3(self, val):
    self.packFloat(val[0])
    self.packFloat(val[1])
    self.packFloat(val[2])

  def packVec4(self, val):
    self.packFloat(val[0])
    self.packFloat(val[1])
    self.packFloat(val[2])
    self.packFloat(val[3])

  def packInt16(self, val): self.pack(struct.pack('h', val))
  def packUInt16(self, val): self.pack(struct.pack('H', val))
  def packInt32(self, val): self.pack(struct.pack('i', val))
  def packUInt32(self, val): self.pack(struct.pack('I', val))
  def packBoneIDu16(self, val): self.packUInt16(val)
  def packNodeIDu32(self, val): self.packUInt32(val)
  def packFloat(self, val): self.pack(struct.pack('f', val))
  def packDouble(self, val): self.pack(struct.pack('d', val))

  def packString(self, str):
    bts = bytes(str, 'utf-8')
    self.packInt32(len(bts))
    self.pack(bts)
class KeySamples:
  def __init__(self):
    self.kmin: int = 0
    self.kmax: int = 0
    self.count: int = 0
    self.pos = bytearray()
    self.rot = bytearray()
    self.scl = bytearray()
class Utils:
  # blender utilities
  M_2PI = float(math.pi * 2)

  def loopFilesByExt(inpath, ext, func):
    inpath = os.path.normpath(inpath)
    for f in os.listdir(inpath):
      fpath = os.path.join(inpath, f)
      if os.path.isfile(fpath):
        fn, fe = os.path.splitext(fpath)
        if fe == ext:
          func(fpath)

  def getWorldBoundBox(ob):
    w_verts = [ob.matrix_world @ Vector(v) for v in ob.bound_box]
    vmax = Vector([-sys.float_info.max, -sys.float_info.max, -sys.float_info.max])
    vmin = Vector([sys.float_info.max, sys.float_info.max, sys.float_info.max])
    for v in w_verts:
      vmax.x = max(vmax.x, v.x)
      vmax.y = max(vmax.y, v.y)
      vmax.z = max(vmax.z, v.z)
      vmin.x = min(vmin.x, v.x)
      vmin.y = min(vmin.y, v.y)
      vmin.z = min(vmin.z, v.z)

    bb = BoundBox(vmin, vmax)
    bb.validate()
    return bb

  def enclosingBoundBox(obs):
    bb = BoundBox.zero()
    bb.genReset()
    Utils.set_frame(1)
    for ob in obs:
      if ob.animation_data != None:
        ob.animation_data.action = None
    bpy.context.view_layer.update()

    for ob in obs:
      obb = Utils.getWorldBoundBox(ob)
      msg("xxobb=" + str(obb) + " v=" + str(obb.volume()))
      bb.genExpandByBox(obb)
      msg("v=" + str(obb.volume()) + " bbv=" + str(bb.volume()))
    return bb

  def assertBlendFileExists(file):
    if not file:
      throw("No blend file specified.")
    elif not os.path.exists(file):
      throw("blend file path '" + file + "' not found.")

  def set_frame(sample):
    # frame_set is slowest this method comes first
    bpy.context.scene.frame_set(int(sample), subframe=math.fmod(sample, 1))
    bpy.context.view_layer.update()

  def getMinKeyframeForAction(curAction):
    iRet = 9999999
    for fcu in curAction.fcurves:
      for keyf in fcu.keyframe_points:
        x, y = keyf.co
        if x < iRet:
          iRet = x
    return int(iRet)

  def getMaxKeyframeForAction(curAction):
    iRet = -9999999
    for fcu in curAction.fcurves:
      for keyf in fcu.keyframe_points:
        x, y = keyf.co
        if x > iRet:
          iRet = x
    return int(iRet)

  def active_object():
    return bpy.context.view_layer.objects.active

  def blendFileIsOpen():
    return bpy.data.filepath

  def get_shader_node_image_input(node):
    img = None
    for input in node.links:
      if type(input.from_node) is bpy.types.ShaderNodeTexImage:  # .type== 'TEX_IMAGE'
        img = input.from_node.image
    return img

  def printObj(obj):
    if hasattr(obj, '__dict__'):
      for k, v in obj.__dict__.items():
        msg(str(k) + "," + str(v))
        Utils.printObj(v)
    else:
      msg(str(dir(obj)))

  def millis():
    return int(round(time.time() * 1000))

  def debugDumpMatrix(str, in_matrix):
    # return ""
    strDebug = ""
    loc, rot, sca = in_matrix.decompose()
    strDebug += "\n\n"
    strDebug += "#" + str + " mat\n" + Convert.matToString(in_matrix.to_4x4(), ",", True) + "\n"
    strDebug += "#  loc     (" + Convert.vec3ToString(loc) + ")\n"
    strDebug += "#  quat    (" + Convert.vec4ToString(rot) + ")\n"

    strDebug += "#gl_quat:  (" + Convert.vec4ToString(Convert.glQuat(rot)) + ")\n"
    strDebug += "#to_euler_deg: (" + Convert.vec3ToString(euler3ToDeg(rot.to_euler("XYZ"))) + ")\n"
    strDebug += "#gl_euler_deg: (" + Convert.vec3ToString(euler3ToDeg(Convert.glEuler3(rot.to_euler()))) + ")\n"

    strDebug += "#AxAng(Blender) " + Convert.vec3ToString(Convert.glVec3(in_matrix.to_quaternion().axis))
    strDebug += "," + Convert.fPrec() % ((180.0) * in_matrix.to_quaternion().angle / 3.14159)
    strDebug += "\n"

    strDebug += "#Ax,Ang(conv)   " + Convert.vec3ToString(Convert.glVec3(Convert.glMat4(in_matrix).to_quaternion().axis))
    strDebug += "," + Convert.fPrec() % ((180.0) * Convert.glMat4(in_matrix).to_quaternion().angle / 3.14159)
    strDebug += "\n"

    return strDebug

  def getFileInfo():
    # print info aobut .blend file in JSON format
    if bpy.context.mode != 'OBJECT':
      bpy.ops.object.mode_set(mode='OBJECT')
    bpy.ops.object.select_all(action='DESELECT')
    # JSON
    strOut = "$3{\"Objects\":["
    app1 = ""
    for ob in bpy.context.visible_objects:
      print("Found " + ob.name + " Type = " + str(ob.type))
      if ob != None:
        if ob.type == 'MESH' or ob.type == 'ARMATURE':
          strOut += app1 + "{"
          strOut += "\"Name\":\"" + ob.name + "\","
          strOut += "\"Type\":\"" + ob.type + "\","
          strOut += "\"Checked\":\"False\","
          strOut += "\"Actions\":["
          if ob.animation_data != None:
            if ob.animation_data.nla_tracks != None:
              for nla in ob.animation_data.nla_tracks:
                nla.select = True
                app2 = ""
                for strip in nla.strips:
                  curAction = strip.action
                  strOut += app2 + "{\"Name\":\"" + \
                    curAction.name + "\", \"Checked\":\"False\"}"
                  app2 = ","

          strOut += "]"
          strOut += "}"
          app1 = ","
    strOut = strOut + "]}$3"

    print(strOut)

  def printExcept(e):
    extype = type(e)
    tb = e.__traceback__
    traceback.print_exception(extype, e, tb)
    return
    msg(str(e))
    exc_tb = sys.exc_info()  # in python 3 - __traceback__
    print(str(exc_tb))
    fname = os.path.split(exc_tb.tb_frame.f_code.co_filename)[1]
    print("fname=" + fname)
    msg(fname + " line " + str(exc_tb.tb_lineno))
    msg(traceback.format_exc())
    sys.exc_clear()
  def get_image_size(fname):
    with open(fname, 'rb') as fhandle:
      head = fhandle.read(24)
      if len(head) != 24:
        return
      if imghdr.what(fname) == 'png':
        check = struct.unpack('>i', head[4:8])[0]
        if check != 0x0d0a1a0a:
          return
        width, height = struct.unpack('>ii', head[16:24])
      elif imghdr.what(fname) == 'gif':
        width, height = struct.unpack('<HH', head[6:10])
      elif imghdr.what(fname) == 'jpeg':
        try:
          fhandle.seek(0)  # Read 0xff next
          size = 2
          ftype = 0
          while not 0xc0 <= ftype <= 0xcf:
            fhandle.seek(size, 1)
            byte = fhandle.read(1)
            while ord(byte) == 0xff:
              byte = fhandle.read(1)
            ftype = ord(byte)
            size = struct.unpack('>H', fhandle.read(2))[0] - 2
          # We are at a SOFn block
          fhandle.seek(1, 1)  # Skip `precision' byte.
          height, width = struct.unpack('>HH', fhandle.read(4))
        except Exception:  # IGNORE:W0703
          return
      else:
        throw("invalid image type for '"+fname+"'")
      return width, height

class Convert:
  def fPrec(prec=None):
    # global toString fp precision
    if prec == None:
      return "%.8f"
    else:
      return "%." + str(prec) + "f"

  def matToString(mat, delim=',', sp=False):
    strRet = ""
    mat_4 = mat.to_4x4()
    strApp = ""
    for row in range(4):
      if sp == True:
        strRet += "#"
      for col in range(4):
        # strRet += str(row) + " " + str(col)
        strFormat = "" + strApp + Convert.fPrec() + ""
        strRet += strFormat % mat_4[row][col]
        strApp = delim
      if sp == True:
        strRet += "\n"
    return strRet

  def floatToString(float, prec=None):
    strFormat = "" + Convert.fPrec(prec) + ""
    strRet = strFormat % (float)
    return strRet

  def vec4ToString(vec4, delim=' '):
    strRet = ""
    strFormat = "" + Convert.fPrec() + delim + Convert.fPrec() + delim + Convert.fPrec() + delim + Convert.fPrec() + ""
    strRet = strFormat % (vec4.x, vec4.y, vec4.z, vec4.w)
    return strRet

  def vec3ToString(vec3, delim=' '):
    strRet = ""
    strFormat = "" + Convert.fPrec() + delim + Convert.fPrec() + delim + Convert.fPrec() + ""
    strRet = strFormat % (vec3.x, vec3.y, vec3.z)
    return strRet

  def color3ToString(vec3, delim=' '):
    strRet = ""
    strFormat = "" + Convert.fPrec() + delim + Convert.fPrec() + delim + Convert.fPrec() + ""
    strRet = strFormat % (vec3.r, vec3.g, vec3.b)
    return strRet

  def vec2ToString(vec2, delim=' '):
    strRet = ""
    strFormat = "" + Convert.fPrec() + delim + Convert.fPrec() + ""
    strRet = strFormat % (vec2.x, vec2.y)
    return strRet

  def glEuler3(eu, yup):
    # NOTE: use Deep exploration to test- same coordinate system as vault
    # Convert Vec3 tgo OpenGL coords
    # -x,-z,-y is the correct export into deep expl
    # This is the correct OpenGL conversion
    if yup:  # self._config._convertY_Up:
      ret = Euler([eu.x, eu.y, eu.z])
      tmp = ret.y
      ret.y = ret.z
      ret.z = tmp
      return ret
    else:
      return eu

  def glQuat(quat, yup):
    if yup:  # self._config._convertY_Up:
      e = quat.to_euler()
      e = glEuler3(e)
      return e.to_quaternion()
    else:
      return quat

  def glVec3(vec, yup):
    # NOTE: use Deep exploration to test- same coordinate system as vault
    # Convert Vec3 tgo OpenGL coords
    ret = Vector([vec.x, vec.y, vec.z])

    # -x,-z,-y is the correct export into deep expl
    # This is the correct OpenGL conversion
    if yup:  # self._config._convertY_Up
      tmp = ret.y
      ret.y = ret.z
      ret.z = tmp
      ret.x = ret.x

    return ret

  def glMat4(in_mat, yup):
    # NOTE this functio works
     # global_matrix = io_utils.axis_conversion(to_forward="-Z", to_up="Y").to_4x4()
     # mat_conv = global_matrix * in_mat * global_matrix.inverted()
     # mat_conv = mat_conv.transposed()
     # return mat_conv

    # NOTE: t12/20/17 this actually works but seems to return a negative z value?

    # NOTE: use Deep exploration to test- same coordinate system as vault
    # convert matrix from Blender to OpenGL Coords
    m1 = in_mat.copy()
    m1 = m1.to_4x4()

    x = 0
    y = 1
    z = 2
    w = 3

    # change of basis matrix
    if yup:  # self._config._convertY_Up:
      cbm = Matrix.Identity(4)
      cbm[x][0] = 1
      cbm[x][1] = 0
      cbm[x][2] = 0
      cbm[x][3] = 0

      cbm[y][0] = 0
      cbm[y][1] = 0
      cbm[y][2] = 1
      cbm[y][3] = 0

      cbm[z][0] = 0
      cbm[z][1] = 1
      cbm[z][2] = 0
      cbm[z][3] = 0

      cbm[w][0] = 0
      cbm[w][1] = 0
      cbm[w][2] = 0
      cbm[w][3] = 1

      # multiply CBM twice
      m1 = cbm.inverted() * m1 * cbm.inverted()

    # blender is row-major?
   # m1.transpose()

    return m1
# endregion
