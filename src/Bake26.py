# python3
# Blender MRT sprite exporter (2.6D)
#
# blender:
# currently disabled
#   ___x='/home/mario/git/Bake26D/Bake26.py';exec(compile(open(___x).read(), ___x, 'exec'))

# terminal:
#  clear; ~/Desktop/apps/blender*/blender -b --log-level 0 -P ~/git/Bake26D/src/Bake26.py  -- -i ~/git/Bake26D/assets/ -o ~/git/Bake26D/output -l ~/git/Bake26D/assets/_library.blend
#
# 1. Make sure object actions are in the NLA editor
# 2. VSCode python format settings (autopep) in ~/.config/.pycodestyle
#
# Note about modules
#   blender python interpreter caches the loaded modules so wont get any edits, use reload()
#   could not get python modules to work with blender (im doing something wrong)
#   adding __init__.py and setup.py to the BlendTools causes odd caching issues
#   restart blender will reload clean python interpreter
#   blender uses a bundled python so system pip &c does not work with it


import bpy
import gpu  # dont use bgl
from mathutils import Vector, Matrix, Euler
import sys
import os
import imp
import shutil
import subprocess
import glob
import builtins
import struct
import imghdr

from inspect import currentframe, getframeinfo
# fmt: off
dir = os.path.join(os.path.dirname(os.path.realpath(__file__)), '../modules/BlenderTools')  # nopep8
if not dir in sys.path:  # nopep8
  sys.path.append(dir)  # nopep8
import BlenderTools  # nopep8
imp.reload(BlenderTools)  # nopep8
from BlenderTools import *  # nopep8
# fmt: on

c_inputFileSwitch = '-i'

class ivec2: pass
class Box2i: pass
class MtTex: pass
class MtNode: pass
class PackedTexture: pass
class ImagePacker: pass
class ActionInfo: pass
class ObjectInfo: pass
class RenderInfo: pass

class ivec2:
  x: int
  y: int

  def __init__(self, dx, dy):
    self.x = int(dx)
    self.y = int(dy)

  def __str__(self):
    return "" + str(self.x) + " " + str(self.y)


class Box2i:
  _min: ivec2
  _max: ivec2
  def left(self): return self._min.x
  def top(self): return self._min.y
  def right(self): return self._max.x
  def bottom(self): return self._max.y
  def width(self): return self._max.x - self._min.x
  def height(self): return self._max.y - self._min.y
  def construct(self, left, top, right, bot):
    self._min = ivec2(left, top)
    self._max = ivec2(right, bot)

class MtTex:
  _pathPNG: str  # path to root png
  _layers: list  # EXR layers
  _width: int
  _height: int
  # _obj_info :
  _node: MtNode

  def __init__(self):
    self._pathPNG = ""
    self._layers = []
    self._width = 0
    self._height = 0
    self._obj_info = None
    self._node = None


class MtNode:
  # declaring a class variable - these are all shared not instance, only created when accessing self
  _child: list
  _texture: MtTex
  _rect: Box2i

  def __init__(self):
    self._rect = Box2i()
    self._child = [None, None]
    self._texture = None

  def plop(self, tex):
    assert(tex != None)
    assert(tex._width > 0)
    assert(tex._height > 0)

    if self._child[0] != None and self._child[1] != None:
      # leaf
      ret = self._child[0].plop(tex)
      if ret != None:
        return ret

      return self._child[1].plop(tex)
    else:
      if self._texture != None:
        return None

      bw = self._rect.width()  # int!!
      bh = self._rect.height()

      if tex._height > bh:
        return None

      if tex._width > bw:
        return None

      if (tex._width == bw) and (tex._height == bh):
        # prefect fit
        self._texture = tex
        return self

      self._child[0] = MtNode()
      self._child[1] = MtNode()

      dw = int(bw - tex._width)
      dh = int(bh - tex._height)

      if dw > dh:
        self._child[0]._rect.construct(
          self._rect.left(),
          self._rect.top(),
          self._rect.left() + tex._width,
          self._rect.bottom())
        self._child[1]._rect.construct(
          self._rect.left() + tex._width,
          self._rect.top(),
          self._rect.right(),
          self._rect.bottom())
      else:
        self._child[0]._rect.construct(
          self._rect.left(),
          self._rect.top(),
          self._rect.right(),
          self._rect.top() + tex._height)
        self._child[1]._rect.construct(
          self._rect.left(),
          self._rect.top() + tex._height,
          self._rect.right(),
          self._rect.bottom())
      return self._child[0].plop(tex)


class PackedTexture:
  _id:int #1 based in array
  _path: str
  _size: int
  _imagecount: int
  _root: MtNode

  def __init__(self):
    self._id=0
    self._path = ""
    self._size = 0  # w and h
    self._imagecount = 0
    self._root: MtNode = None


class ImagePacker:

  # so for this one we are going to actually use just a fixed texture size.
  # can only fit x textures into this one?
  #  then make another one

  def packImages(texs, startsize=256, growsize=256):
    # https://docs.blender.org/api/current/gpu.capabilities.html
    # return list of packedtexture obejcts
    mega_texs = []
    size = int(startsize)
    growsize = int(growsize)
    maxsize = int(8192)

    # note you cant get gpu maxsize in headless blender.. we just need to pick a value
    msg("packing " + str(len(texs)) + " texs. maxsize=" + str(maxsize))

    # pack until max wh
    # then remove from texs
    # then pack again

    while size <= maxsize:
      size = min(int(size), int(maxsize))

      pt = ImagePacker.packForSize(texs, size, int(size) == int(maxsize))

      if pt != None:
        pt._id = len(mega_texs)+1
        ImagePacker.removePlopped(pt._root, texs)
        mega_texs.append(pt)
        size = startsize
      else:
        size = min(int(size + growsize), maxsize)

      if len(texs) == 0:
        break

    return mega_texs

  def packForSize(texs, size, force):
    # return the packtex or none
    pt = PackedTexture()
    pt._size = int(size)
    pt._root = MtNode()
    pt._root._rect = Box2i()
    pt._root._rect._min = ivec2(0, 0)
    pt._root._rect._max = ivec2(size, size)

    for texpath in texs:
      texs[texpath]._node = None
      #msg("texwh "+str(texs[texpath]._width)+" "+str(texs[texpath]._height))
      found = pt._root.plop(texs[texpath])
      if (found != None):
        texs[texpath]._node = found
        pt._imagecount += 1
      else:
        if force:
          return pt
        return None
    return pt

  def removePlopped(node, texs):
    if node._texture != None:
      if node._texture._pathPNG in texs:
        assert(texs[node._texture._pathPNG]._node == node)
        del texs[node._texture._pathPNG]
    if node._child[0] != None:
      ImagePacker.removePlopped(node._child[0], texs)
    if node._child[1] != None:
      ImagePacker.removePlopped(node._child[1], texs)


class ActionInfo:
  _name: str
  _action: bpy.types.Action
  _sweepBox: BoundBox
  _start: int
  _end: int
  _texs: dict

  def __init__(self):
    self._name = ""
    self._action = None  # if none then this is sprite
    self._sweepBox = None
    self._start = 0
    self._end = 0
    self._texs = {
      'Normal': [],  # these names are important we strrep them
      'Color': [],
      'Depth': [],
      'PNGColor': []
      }


class ObjectInfo:
  _name: str
  _object: bpy.types.Object
  _enclosingBox: BoundBox
  _sweepBox: BoundBox
  _actions: dict
  _min_start: int
  _max_end: int

  def __init__(self):
    self._name = ""
    self._object = None
    self._enclosingBox = None
    self._sweepBox = None  # of all obs and anims
    self._actions = {}
    self._min_start = 99999  # of all anims
    self._max_end = -99999

class RenderInfo:
  _azumith_slices = 0
  _zenith_slices = 0
  # _initial_azumith = 0 todo
  # _initial_zenith = 0
  _samples = 10  # note at 1 sample there is no edge blur
  _render_width = 0  # 256
  _render_height = 0  # 256
  _render_major = 64  # major of w/h dimension
  # _aaSamples = 0
  _sampleRate = 1
  # _bFitModel = False
  _perspective = False  # True #True  # perspective / orthographic changing to ortho causes the image to shrink we'd need to change orthographic bounds as well (ortho_scale)
  _fov = 40.0
  _edge_thicknes = 0.2  # 0=minimal edge 1 = maximal edge
  _edge_blur = 0.4  # 0= no blur edges 1 = full blur edges
  _camera = None
  _empty = None
  _customCamera = False
  _camname = '__b26camera'
  _empname = '__b26empty'
  _dbg_renderCount = 0
  _fileOutputs = {}

class Bake26:
  _fVersion = 0.01
  _outPath = ""
  _libFile = ""
  _data = RenderInfo()
  _is_cmd = False  # running in cmd
  _fileObjName = ""  # default obj name for sprites
  _spriteName = '_sprite'

  def __init__(self, inpath, outpath, libfile):
    libfile = os.path.normpath(libfile)
    outpath = os.path.normpath(outpath)

    if not os.path.exists(outpath):
      msg("creating '" + outpath + "'")
      os.makedirs(outpath)
    self._outPath = outpath
    assert(os.path.exists(libfile))
    self._libFile = libfile
    self._is_cmd = not bpy.data.filepath
    if not self._is_cmd:
      throw("can only run from terminal for now")

    msg("TESTING!!!!")
    msg("TESTING!!!!")
    msg("TESTING!!!!")
    msg("TESTING!!!!")
    msg("TESTING!!!!")
    bpy.ops.wm.open_mainfile(filepath=libfile)  # TESTING###
    self.createTextures(outpath)
    return

    # blender is crashing doing multiple files, so we're going to do subprocess and re-enter this script with the .blend
    if os.path.isfile(inpath):
      self._fileObjName, fe = os.path.splitext(os.path.basename(inpath))
      bpy.ops.wm.open_mainfile(filepath=inpath)
      self.appendLibData()
      self.export()
      # self._data = None
      # bpy.ops.wm.quit_blender() #?? crashing
      # msg(str(bpy.data.filepath))
    else:
      def exportBlend(fpath):
        if fpath != self._libFile:
          cmd = ""
          for i in range(0, len(sys.argv)):
            if i > 0 and sys.argv[i - 1] == c_inputFileSwitch:
              cmd += " " + fpath
            elif cmd != "":
              cmd += " " + sys.argv[i]
            else:
              cmd = sys.argv[i]
          subprocess.run(cmd, shell=True)
      Utils.loopFilesByExt(inpath, '.blend', exportBlend)

  def appendLibData(self):
    msg("lib=" + self._libFile)
    in_node_groups = []
    with bpy.data.libraries.load(self._libFile) as (data_from, data_to):
      for name in data_from.node_groups:
        in_node_groups.append({'name': name})
    bpy.ops.wm.append(directory=self._libFile + "/NodeTree/", files=in_node_groups)
    msg("groups=" + str([g.name for g in bpy.data.node_groups]))

  def getRootName(self):
    name = ""
    if bpy.data.filepath:
      name = os.path.splitext(os.path.basename(bpy.data.filepath))[0]
    return name

  def export(self):
    try:
      bpy.ops.object.mode_set(mode='OBJECT')
      # just render the object with animations

      inf = self.getRenderInfo()
      self.setup()
      self.setRenderSettings()
      self.makeOutputs()
      self.focusCamera(inf)

      # for ob in bpy.context.visible_objects:
      # if ob.type == 'MESH' or ob.type == 'ARMATURE':
      self.renderObjects(inf)

      # self.renderTrack(arm, None, 0, 0, 0)
    except Exception as e:
      Utils.printExcept(e)
    self.finish()

  def getAnimatedObject(self):
    for ob in bpy.context.visible_objects:
      if ob.type == 'MESH' or ob.type == 'ARMATURE':
        for nla in ob.animation_data.nla_tracks:
          if not "[Action Stash]" in nla.name:
            if len(nla.strips) > 0:
              return ob
    return None

  def setup(self):
    bpy.ops.object.select_all(action='DESELECT')
    bpy.context.view_layer.objects.active = None

    # create  camera
    exists = [ob for ob in bpy.context.view_layer.objects if ob.name == self._data._camname and ob.hide_render == False]
    if len(exists) > 0:
      self._data._camera = exists[0]
      self._data._customCamera = True
      msg("existing camera '" + str(exists[0]) + "' found")
    else:
      # add camera
      self._data._customCamera = False
      bpy.ops.object.camera_add()
      self._data._camera = bpy.context.view_layer.objects.active
      self._data._camera.name = self._data._camname
      self._data._camera.data.name = self._data._camname

      # add empty target
      bpy.ops.object.empty_add()
      self._data._empty = bpy.context.view_layer.objects.active
      self._data._empty.name = self._data._empname
      constraint = self._data._camera.constraints.new('TRACK_TO')
      constraint.track_axis = 'TRACK_NEGATIVE_Z'
      constraint.up_axis = 'UP_Y'
      constraint.target = self._data._empty

    bpy.context.scene.camera = self._data._camera

  def finish(self):
    # stats
    msg("render count = " + str(self._data._dbg_renderCount))

  def setRenderSettings(self):
    # https://docs.blender.org/api/current/bpy.types.RenderSettings.html
    bpy.context.scene.render.engine = 'BLENDER_EEVEE'  # 'BLENDER_EEVEE' 'BLENDER_WORKBENCH' 'CYCLES'
    bpy.context.scene.eevee.taa_render_samples = self._data._samples
    bpy.context.scene.render.resolution_x = self._data._render_width
    bpy.context.scene.render.resolution_y = self._data._render_height
    bpy.context.scene.render.resolution_percentage = 100
    bpy.context.scene.render.film_transparent = True  # disable background
    bpy.context.scene.render.filter_size = 1  # cause blurs edges
    bpy.context.scene.render.use_motion_blur = False
    bpy.context.scene.render.use_compositing = True
    bpy.context.scene.render.use_file_extension = True
    bpy.context.scene.render.filepath = ''
    # we are not outputting an image we just use file outputs in compositor
    # bpy.context.scene.render.image_settings.color_mode = 'RGBA'
    # bpy.context.scene.render.image_settings.file_format = 'PNG'
    # bpy.context.scene.render.image_settings.color_depth = '16'  # ('8', '16')
    # bpy.context.scene.render.image_settings.use_zbuffer = True  # save depth

  def makeOutputs(self):
    # https://docs.blender.org/api/current/bpy.types.CompositorNodeRLayers.html
    # https://docs.blender.org/api/current/bpy.types.CompositorNodeOutputFile.html
    # https://docs.blender.org/api/current/bpy.types.ViewLayer.html
    # https://blender.stackexchange.com/questions/105193/unable-to-store-renderings-in-openexr-when-using-the-multiprocessing-in-python
    # https://blender.stackexchange.com/questions/194564/is-it-possible-to-save-depth-values-and-rendered-image-in-different-formats

    bpy.context.scene.use_nodes = True
    bpy.context.scene.view_layers["ViewLayer"].use_pass_normal = True
    bpy.context.scene.view_layers["ViewLayer"].use_pass_z = True

    # remove all existing nodes
    for node in bpy.context.scene.node_tree.nodes:
      bpy.context.scene.node_tree.nodes.remove(node)

    render_layers = bpy.context.scene.node_tree.nodes.new(type='CompositorNodeRLayers')

    # ** the deblur filter is only necessary if eevee samples > 1
    pngOut = self.makeFileOutput('PNGColor', True)
    deblur = bpy.context.scene.node_tree.nodes.new(type='CompositorNodeGroup')
    deblur.node_tree = bpy.data.node_groups['EdgeDeblurGroup']
    bpy.context.scene.node_tree.links.new(render_layers.outputs["Image"], deblur.inputs["Image"])
    bpy.context.scene.node_tree.links.new(render_layers.outputs["Alpha"], deblur.inputs["Alpha"])
    bpy.context.scene.node_tree.links.new(deblur.outputs["Image"], pngOut.inputs[0])
    deblur.inputs['Thickness'].default_value = self._data._edge_thicknes
    deblur.inputs['Blur'].default_value = self._data._edge_blur

    colorOutput = self.makeFileOutput('Color')
    bpy.context.scene.node_tree.links.new(deblur.outputs["Image"], colorOutput.inputs[0])

    depthOutput = self.makeFileOutput('Depth')
    norm = bpy.context.scene.node_tree.nodes.new(type='CompositorNodeNormalize')
    bpy.context.scene.node_tree.links.new(render_layers.outputs["Depth"], norm.inputs[0])
    bpy.context.scene.node_tree.links.new(norm.outputs[0], depthOutput.inputs[0])

    normalOutput = self.makeFileOutput('Normal')
    normalOutput.format.color_mode = 'RGB'
    bpy.context.scene.node_tree.links.new(render_layers.outputs["Normal"], normalOutput.inputs[0])

    bpy.context.scene.render.use_sequencer = False  # dont video edit
    bpy.context.view_layer.update()

  def makeFileOutput(self, name, png=False):
    output = bpy.context.scene.node_tree.nodes.new(type='CompositorNodeOutputFile')
    output.label = name
    output.base_path = ""
    if png:
      output.format.file_format = 'PNG'
      output.format.color_depth = '8'
      output.format.color_mode = 'RGBA'
    else:
      output.format.file_format = 'OPEN_EXR'  # HDR
      output.format.color_depth = '32'
      output.format.color_mode = 'RGB'
      output.format.exr_codec = 'ZIP'
    self._data._fileOutputs[name] = output
    return output

  def renderObjects(self, inf):
    msg("name=" + inf._name)

    # ob.select_set(True)

    # backup output
    self.backupObjectOutput(inf._name)

    self.renderAnimations(inf)

  def getRenderableObjects(self):
    # return only the objects that get rendered out
    meshes = [ob for ob in bpy.context.view_layer.objects if ob.type == 'MESH']
    return meshes

  def getRenderInfo(self):
    # get kf min / max and sweep bounds
    # https://blender.stackexchange.com/questions/118369/bounding-box-coordinates-taking-into-consideration-but-not-applying-rotation
    inf = ObjectInfo()

    render_objs = self.getRenderableObjects()

    inf._object = self.getAnimatedObject()
    inf._name = self._fileObjName
    inf._enclosingBox = Utils.enclosingBoundBox(render_objs)
    inf._sweepBox = inf._enclosingBox.clone()
    inf._actions = dict()
    if inf._object != None:
      for nla in inf._object.animation_data.nla_tracks:
        if not "[Action Stash]" in nla.name:
          if len(nla.strips) > 0:
            action = nla.strips[0].action
            ainf = ActionInfo()
            ainf._name = action.name
            ainf._action = action
            ainf._start = action.frame_range[0]
            ainf._end = action.frame_range[1]
            ainf._sweepBox = BoundBox.zero()
            ainf._sweepBox.genReset()
            inf._min_start = min(inf._min_start, ainf._start)
            inf._max_end = max(inf._max_end, ainf._end)
            inf._actions[ainf._name] = ainf

    # create the "sprite" if we have no actions
    if len(inf._actions) == 0:
      ainf = ActionInfo()
      ainf._name = self._spriteName
      ainf._action = None
      ainf._start = 1
      ainf._end = 1
      ainf._sweepBox = BoundBox.zero()
      ainf._sweepBox.genReset()
      inf._min_start = min(inf._min_start, ainf._start)
      inf._max_end = max(inf._max_end, ainf._end)
      inf._actions[ainf._name] = ainf

    def generate_sweep_boxes(frame, ainf):
      for rob in render_objs:
        bb = Utils.getWorldBoundBox(rob)
        # msg('bb='+str(bb)+" "+str(bb.volume()))
        ainf._sweepBox.genExpandByBox(bb)
        inf._sweepBox.genExpandByBox(bb)

    self.animateObjectActions(inf, generate_sweep_boxes)

    return inf

  def renderAnimations(self, inf):

    def renderob(frame, actioninf):
      if self._data._customCamera == False:
        self.renderAngles(actioninf, inf._name, frame)
      else:
        self.renderAction(actioninf, inf._name, 0, 0, 0)

    msg("exporting: " + str(inf._min_start) + "-" + str(inf._max_end))
    self.animateObjectActions(inf, renderob)

    msg("doing vids")
    for actname in inf._actions:
      if actname != self._spriteName:
        self.exportVid(inf._name, actname)

  def focusCamera(self, inf):
    bb = inf._sweepBox
    msg("enclosing=" + str(inf._enclosingBox) + " v=" + str(inf._enclosingBox.volume()))
    msg("sweep=" + str(inf._sweepBox) + " v=" + str(inf._enclosingBox.volume()))

    self._data._empty.location = bb.center()
    self._data._camera.location = self._data._empty.location

    if self._data._perspective == False:
      self._data._camera.data.type = 'ORTHO'
      self._data._camera.data.ortho_scale = max(bb._max.x - bb._min.x, bb._max.z - bb._min.z)
      self._data._camera.location.y = -1
    else:
      self._data._camera.data.type = 'PERSP'
      self._data._camera.data.lens_unit = 'FOV'
      self._data._camera.data.angle = math.radians(self._data._fov)

    # try to create an automatic render size
    sx = bb._max.x - bb._min.x
    sz = bb._max.z - bb._min.z

    assert(sx > 0 and sz > 0)

    if sx > sz:
      self._data._render_width = self._data._render_major
      self._data._render_height = int(round(self._data._render_major * sz / sx))
    else:
      self._data._render_height = self._data._render_major
      self._data._render_width = int(round(self._data._render_major * sx / sz))
    msg("render res =" + str(self._data._render_width) + "x" + str(self._data._render_height))

    bpy.context.scene.render.resolution_x = self._data._render_width
    bpy.context.scene.render.resolution_y = self._data._render_height
    bpy.context.view_layer.update()

  def animateObjectActions(self, inf, func):
    # loop all actions in object and animate them at all keyframe intervals
    def animateAnims(frame):
      for actname in inf._actions:
        if inf._object != None:
          inf._object.animation_data.action = inf._actions[actname]._action
        func(frame, inf._actions[actname])

    self.animate(inf._min_start, inf._max_end, animateAnims)

  def animate(self, start, end, func):
    frame = start
    while frame <= end:
      Utils.set_frame(frame)
      func(frame)
      frame += self._data._sampleRate

  def backupObjectOutput(self, obname):
    # so find the name and backup
    oldp = self.objectOutputPath(obname, False, 0)
    bakp = ""
    if os.path.exists(oldp):
      # afraid to delete recursively in code. just do it manually
      for count in range(1, 500):
        if count == 499:
          throw("Warning: too many backups exist '" + str(count) + "'. Delete backups in output directory.")
        bkp = self.objectOutputPath(obname, True, count)
        if not os.path.exists(bkp):
          bakp = bkp
          break
      print("backup:")
      print("  old:" + oldp)
      print("  bak:" + bakp)
      shutil.move(oldp, bakp)

  def exportVid(self, obname, actname):
    spritepath = self.trackOutputPath(obname, actname)
    assert(os.path.exists(spritepath))
    fcount = len(glob.glob1(spritepath, "*.png"))
    if fcount > 1:
      # mp4
      ss = 1
      sclxy = ""
      if ss != 1:
        sx = max(int(self._data._render_width * ss), 1024)
        sy = max(int(self._data._render_height * ss), 1024)
        sclxy = " -s " + str(sx) + "x" + str(sy) + " "
      mp4frame = (float(bpy.context.scene.render.fps) / float(self._data._sampleRate))
      # cmd = "ffmpeg -framerate " + str(int(mp4frame)) + " -stream_loop 5 -pattern_type glob -i '" + spritepath + "/*.png' -crf 0 "+sclxy+" -c:v libx264 '" + spritepath + "/output.mp4'"
      cmd = "ffmpeg -framerate " + str(int(mp4frame)) + " -stream_loop 5 -pattern_type glob -i '" + spritepath + "/*.png' -crf 0 " + sclxy + " -c:v libx265 -x265-params lossless=1 '" + spritepath + "/output.mp4'"
      msg("cmd=" + cmd)
      subprocess.run(cmd, shell=True)

      # gif
      csframe = (100.0 / (float(bpy.context.scene.render.fps) / float(self._data._sampleRate)))
      cmd = "convert -delay " + str(csframe) + " -dispose Background -loop 0 " + spritepath + "/*.png " + spritepath + "/output.gif &"
      subprocess.run(cmd, shell=True)
    else:
      msg("gif: had <=1 png files")

  def renderAngles(self, actioninf, obname, frame):
    if self._data._azumith_slices > 0:
      rot_mat = Matrix.Rotation((Utils.M_2PI / float(self._data._azumith_slices)), 4, 'Z')

    for zenith in range(0, self._data._zenith_slices + 1):
      for azumith in range(0, self._data._azumith_slices + 1):
        radius = (self._data._camera.location - self._data._empty.location).length
        pmat = Matrix.Translation((0, radius, 0))
        rot_mat = Matrix.Identity(4)
        if azumith > 0:
          slice = Utils.M_2PI / float(self._data._azumith_slices)
          rot_mat @= Matrix.Rotation(slice * float(azumith), 4, 'Z')
        if zenith > 0:
          slice = Utils.M_2PI / float(self._data._zenith_slices)
          rot_mat @= Matrix.Rotation(slice * float(zenith), 4, 'X')

        self._data._camera.matrix_world = pmat @ rot_mat
        # msg("mat="+str(self._dummyCamera.matrix_world))

        self.renderAction(actioninf, obname, frame, azumith, zenith)

  def renderAction(self, actioninf, obname, frame, azumith, zenith):
    fpath = self.actionFilePath(obname, actioninf._name, frame, azumith, zenith)

    bpy.context.scene.render.filepath = fpath + ".Color.png"
    for oname in self._data._fileOutputs:
      out = self._data._fileOutputs[oname]
      out.base_path = os.path.dirname(fpath)
      out.file_slots[0].path = os.path.basename(fpath) + "." + out.label + ".exr"
      actioninf._texs[oname].append(out.file_slots[0].path)

    bpy.ops.render.render(write_still=False)
    self._data._dbg_renderCount += 1

  def actionFilePath(self, obname, actname, frame, azumith, zenith):
    sep = self.spritePathSeparator()
    fname = obname + sep
    fname += actname + sep
    fname += str(azumith) + sep
    fname += str(zenith) + sep
    frmi = int(frame)
    frmf = int(round(frame - float(frmi), 3) * 1000)
    # https://blender.stackexchange.com/questions/5280/how-to-use-the-current-file-name-as-the-the-output-name-in-the-file-output-node
    fname += "####" + sep  # str(str(frmi).zfill(3)) + sep #blender must always have frame numbers on output files so we put them here
    fname += str(str(frmf).zfill(3))

    path = self.trackOutputPath(obname, actname)
    path = os.path.join(path, fname)

    return path

  def trackOutputPath(self, obname, actname):
    path = self.objectOutputPath(obname, False, 0)
    path = os.path.join(path, os.path.normpath(actname))
    return path

  def objectOutputPath(self, obname, backup, backup_id):
    path = os.path.abspath(os.path.normpath(self._outPath))
    if backup:
      path = os.path.join(path, os.path.normpath("backup"))
      path = os.path.join(path, os.path.normpath(os.path.basename(bpy.data.filepath)))
      path = os.path.join(path, os.path.normpath(obname + "." + str(backup_id)))
    else:
      path = os.path.join(path, os.path.normpath(os.path.basename(bpy.data.filepath)))
      path = os.path.join(path, os.path.normpath(obname))
    return path

  def spritePathSeparator(self):
    return "."

  def imgFrame(self, frameid):
    # Critical that we have EXACTLY 7 digits here, because the C# will parse 7 digits out of the frame 4/6/18
    return "img%07d.png" % frameid

  def createTextures(self, path):
    texs = dict()
    path = os.path.abspath(path)
    Bake26.gatherTextures(path, texs)
    pts = ImagePacker.packImages(texs)

    for i in range(0, len(pts)):
      pt = pts[i]
      msg("PackedTexture: "+ " id="+ str(pt._id)+ " count=" + str(pt._imagecount) + " size="+ str(pt._size))

    # with BinaryFile(path) as binfile:
    #  self._out.

  def gatherTextures(fpath, texs):
    fpath = os.path.abspath(fpath)
    fds = os.listdir(fpath)  # get all files' and folders' names in the current directory
    for item in fds:  # loop through all the files and folders
      itempath = os.path.join(fpath, item)
      if os.path.isfile(itempath):
        fn, fe = os.path.splitext(item)

        if fe.lower() == '.png':
          x = Bake26.get_image_size(itempath)

          # now set norms and shit
          if os.path.exists(itempath):
            texs[itempath] = MtTex()
            texs[itempath]._pathPNG = itempath
            texs[itempath]._layers = [
              itempath.replace(".PNGColor.png", ".Color.exr"),
              itempath.replace(".PNGColor.png", ".Normal.exr"),
              itempath.replace(".PNGColor.png", ".Depth.exr")
              ]
            texs[itempath]._width = int(x[0])
            texs[itempath]._height = int(x[1])

      elif os.path.isdir(itempath):
        if item != '.':
          Bake26.gatherTextures(itempath, texs)

  def get_image_size(fname):
    '''Determine the image type of fhandle and return its size.
    from draco'''
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
        return
      return width, height
# ======================================================================================


class BinaryFile:
  _binFile = None
  _block = None
  _blockname = None
  _bytesWritten = 0

  def __init__(self, filepath):
    # this came from a with statement, so make sure this works
    # with BinaryFile(path) as self._out:
    self._binFile = open(filepath, 'wb')

  def writeString(self, str):
    # https://docs.python.org/3/library/struct.html
    bts = bytes(str, 'utf-8')
    self.writeInt32(len(bts))
    self.writeData(bts)

  def writeBool(self, val):
    self.writeData(struct.pack('?', val))

  def writeByte(self, val):
    # unsigned char..
    self.writeData(struct.pack('B', val))

  def writeInt16(self, val):
    self.writeData(struct.pack('h', val))

  def writeUInt16(self, val):
    self.writeData(struct.pack('H', val))

  def writeInt32(self, val):
    self.writeData(struct.pack('i', val))

  def writeUInt32(self, val):
    self.writeData(struct.pack('I', val))

  def writeInt64(self, val):
    self.writeData(struct.pack('q', val))

  def writeUInt64(self, val):
    self.writeData(struct.pack('Q', val))

  def writeFloat(self, val):
    self.writeData(struct.pack('f', val))

  def writeDouble(self, val):
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

  def writeVec3(self, val):
    v = val
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


def getArgs():
  argv = sys.argv
  if "--" not in argv:
    argv = []
  else:
    argv = argv[argv.index("--") + 1:]
  parser = argparse.ArgumentParser(description="MRT sprite baker")
  parser.add_argument(c_inputFileSwitch, dest="inpath", type=str, required=True, help="path to .blend files")
  parser.add_argument("-o", dest="outpath", type=str, required=True, help="output path")
  parser.add_argument("-l", dest="libpath", type=str, required=True, help="library file path")
  args = parser.parse_args(argv)
  return args


try:
  try:
    args = getArgs()
  except Exception as e:
    Utils.printExcept(e)

  mob = Bake26(args.inpath, args.outpath, args.libpath)
except Exception as e:
  Utils.printExcept(e)
