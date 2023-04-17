# python3
# Blender MRT sprite exporter (2.6D)
#
# blender:
# currently disabled
#   ___x='/home/mario/git/Bake26D/Bake26.py';exec(compile(open(___x).read(), ___x, 'exec'))

# terminal:
#   clear; ~/Desktop/apps/blender*/blender -b --log-level 0 -P ~/git/Bake26D/src/Bake26.py  -- -i ~/git/Bake26D/data/blend/ -o ~/git/Bake26D/b26out -l ~/git/Bake26D/data/blend/_library.blend -p
#
# Packing:
#   Note: Image packing requires blender to have PIL which requires run these commands
#     blender-3.4.1-linux-x64/3.4/python/bin$ ./python* -m ensurepip --default-pip
#     blender-3.4.1-linux-x64/3.4/python/bin$ ./python* -m pip install Pillow
#       Srouce:https://blender.stackexchange.com/questions/119051/how-do-i-install-pillow-and-or-pip-in-blender-python-in-windows-os
#
# Exporting:
#   1. Make sure object actions are in the NLA editor
#   2. VSCode python format settings (autopep) in ~/.config/.pycodestyle
#
# Note about modules
#   blender python interpreter caches the loaded modules so wont get any edits, use reload()
#   could not get python modules to work with blender (im doing something wrong)
#   adding __init__.py and setup.py to the BlendTools causes odd caching issues
#   restart blender will reload clean python interpreter
#   blender uses a bundled python so system pip &c does not work with it



import sys
import os
import imp
import shutil
import argparse
import subprocess
import glob
import multiprocessing
import bpy
from mathutils import Vector, Matrix, Euler
from PIL import Image
import re

def import_file(path, import_global=True):
  try:
    import os
    if not os.path.isabs(path):
      path = os.path.join(os.path.dirname(os.path.realpath(__file__)), os.path.normpath(path))
    module_dir, module_file = os.path.split(path)
    module_name, module_ext = os.path.splitext(module_file)
    save_cwd = os.getcwd()
    os.chdir(module_dir)
    if not module_dir in sys.path:
      sys.path.append(module_dir)
    module_obj = __import__(module_name)
    module_obj.__file__ = path
    if import_global:
      for key, val in vars(module_obj).items():
        if key.startswith('__') and key.endswith('__'):
          continue
        thismod = sys.modules[__name__]
        vars(thismod)[key] = val
    else:
      globals()[module_name] = module_obj
    os.chdir(save_cwd)
  except Exception as e:
    raise Exception(e)
  return module_obj

import_file('./BlenderTools.py')

msg("cwd="+os.getcwd())

class ExportFormat: pass  # nopep8
class ExportLayer: pass  # nopep8
class LayerOutput: pass  # nopep8
class MtRegion: pass  # nopep8
class MtNode: pass  # nopep8
class MtIsland: pass  # nopep8
class ImagePacker: pass  # nopep8
class ActionInfo: pass  # nopep8
class ObjectInfo: pass  # nopep8
class RenderInfo: pass  # nopep8
class ExportSettings: pass  # nopep8
class Bake26: pass  # nopep8

class ExportFormat:
  PNG = 'PNG'
  OPEN_EXR = 'EXR'
class ExportLayer:
  Color = 'Color'
  DepthNormal = 'DepthNormal'
  Position = 'Position'
class RenderEngine:
  Eevee = 'BLENDER_EEVEE'
  Cycles = 'CYCLES'

class LayerOutput:
  # def __init__(self, node, format, layer):
  def __init__(self, layer: ExportLayer, format: ExportFormat, bitdepth: int):
    self._layer: ExportLayer = layer
    self._format: ExportFormat = format
    self._bitdepth: int = bitdepth
    self._compression: int = 100  # 0-100
    self._mode: str = 'RGBA'  # RGB RGBA
  def file_ext(self):
    ext = '.'
    if self._format == ExportFormat.PNG:
      ext += 'png'
    elif self._format == ExportFormat.OPEN_EXR:
      ext += 'exr'
    else:
      throw()
    return ext
  def layer_ext(self):
    ext = '.' + str(self._layer) + self.file_ext()
    return ext
  def mtex_filename(self, index: int):
    s = 'B2MT.' + str(int(index)).zfill(3) + self.layer_ext()
    return s
  @staticmethod
  def parse_extension(path):
    d = path.rfind('.')
    if d > 0:
      d = path[:d].rfind('.')
      if d > 0:
        return path[d:]
    return None

class MtRegion:
  def __init__(self):
    self._layers = []
    self._width = 0
    self._height = 0
    self._texid = -1
    self._node = None
class MtNode:
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
class MtIsland:
  # individual texture of a multiple image mega texure
  def __init__(self):
    self._texid = 0
    self._size = 0  # w and h
    self._regioncount = 0
    self._root: MtNode = None
    self._texnames = []

  def compose(self, outpath):
    assert(self._root)
    for layer in Bake26.c_layers:
      self.packLayer(outpath, layer)

  def packLayer(self, outpath, layeroutput: LayerOutput):
    fname = layeroutput.mtex_filename(self._texid)
    msg("packing layer "+fname+" size="+str(self._size))

    cpink = (255, 0, 255, 255)
    cblack = (0, 0, 0, 0)
    # bpy.ops.image.new(name='Untitled', width=1024, height=1024, color=(0.0, 0.0, 0.0, 1.0), alpha=True, generated_type='BLANK', float=False, use_stereo_3d=False, tiled=False)
    # https://pillow.readthedocs.io/en/stable/handbook/concepts.html#concept-modes
    # PIL only supports 8 bit pixels.
    with Image.new(mode="RGBA", size=(self._size, self._size), color=cblack) as master_img:
      # master_img.convert("PA")
      # note imae ha 'I' and 'F' for 32 bits
      #master_img = Image.new(mode="RGBA", size= (self._size, self._size), color=(0,0,0,0))
      self.copyImages(self._root, master_img, layeroutput)
      self._texnames.append(fname)
      path = os.path.join(outpath, fname)
      msg(logcol.greenb + "" + path + logcol.reset)
      master_img.save(path)

  def copyImages(self, node, master, layeroutput: LayerOutput):
    assert(node)

    # also create objs for metadata
    if node._texture != None:  # node._texture = MtRegion
      path = node._texture._layers[layeroutput._layer]
      if os.path.exists(path):
        with Image.open(path) as img:
          Image.Image.paste(master, img, (node._rect.left(), node._rect.top()))

    if node._child[0] != None:
      self.copyImages(node._child[0], master, layeroutput)
    if node._child[1] != None:
      self.copyImages(node._child[1], master, layeroutput)

  def serialize(self, bf: BinaryFile):
    bf.writeInt32(self._texid)
    bf.writeInt32(self._size)
    bf.writeInt32(self._size)
    bf.writeInt32(len(self._texnames))
    for tn in self._texnames:
      bf.writeString(tn)

class ImagePacker:
  def packImages(texs, startsize=256, growsize=256):
    mega_texs = []
    size = int(startsize)
    growsize = int(growsize)
    maxsize = int(Bake26.c_max_tex_size_px)

    # note you cant get gpu maxsize in headless blender.. we just need to pick a value
    msg("packing " + str(len(texs)) + " texs. maxsize=" + str(maxsize))

    cur_tex_id = 1
    while size <= maxsize:
      size = min(int(size), int(maxsize))

      pt = ImagePacker.packForSize(texs, size, int(size) == int(maxsize), cur_tex_id)

      if pt != None:
        pt._id = cur_tex_id
        cur_tex_id += 1
        ImagePacker.removePlopped(pt._root, texs)
        mega_texs.append(pt)

        if int(size) == int(maxsize) and Bake26.c_uniform_tex_size == True:
          size = maxsize
        else:
          size = startsize
      else:
        size = min(int(size + growsize), maxsize)

      if len(texs) == 0:
        break

    return mega_texs

  def packForSize(texs, size, force, tex_id):
    pt = MtIsland()
    pt._size = int(size)
    pt._texid = tex_id
    pt._root = MtNode()
    pt._root._rect = Box2i()
    pt._root._rect._min = ivec2(0, 0)
    pt._root._rect._max = ivec2(size, size)

    for texpath in texs:
      texs[texpath]._node = None
      texs[texpath]._texid = -1
      # msg("texwh "+str(texs[texpath]._width)+" "+str(texs[texpath]._height))
      found = pt._root.plop(texs[texpath])
      if (found != None):
        texs[texpath]._node = found
        texs[texpath]._texid = tex_id
        pt._regioncount += 1
      else:
        if force:
          return pt
        return None
    return pt

  def removePlopped(node, texs):
    if node._texture != None:
      if node._texture._pathColor in texs:
        assert(texs[node._texture._pathColor]._node == node)
        del texs[node._texture._pathColor]
    if node._child[0] != None:
      ImagePacker.removePlopped(node._child[0], texs)
    if node._child[1] != None:
      ImagePacker.removePlopped(node._child[1], texs)

class b2_obj:
  _ob_idgen = 1
  def __init__(self):
    self._id = -1
    self._name = ""
    self._framerate = 0
    self._actions = []
  def serialize(self, bf: BinaryFile):
    bf.writeInt32(self._id)
    bf.writeString(self._name)
    bf.writeInt32(self._framerate)
    bf.writeInt32(len(self._actions))
    for i in range(0,len(self._actions)):
      self._actions[i].serialize(bf)
  def deserialize(self, bf: BinaryFile):
    self._id = bf.readInt32()
    self._name = bf.readString()
    self._framerate = bf.readInt32()
    alen = bf.readInt32()
    for i in range(0, alen):
      a = b2_action()
      a.deserialize(bf)
      self._actions.append(a)
class b2_action:
  def __init__(self):
    self._id = -1
    self._name = ""
    self._frames = []
  def serialize(self, bf: BinaryFile):
    bf.writeInt32(self._id)
    bf.writeString(self._name)
    bf.writeInt32(len(self._frames))
    for i in range(0,len(self._frames)):
      self._frames[i].serialize(bf)
  def deserialize(self, bf: BinaryFile):
    self._id = bf.readInt32()
    self._name = bf.readString()
    flen = bf.readInt32()
    for i in range(0, flen):
      f = b2_frame()
      f.deserialize(bf)
      self._frames.append(f)
class b2_frame:
  def __init__(self):
    self._seq: float = -1  # float
    self._texid: int = -1
    self._x: int = -1
    self._y: int = -1
    self._w: int = -1
    self._h: int = -1
    self._imgs = []  # temp just for objs
  def serialize(self, bf: BinaryFile):
    bf.writeFloat(self._seq)
    bf.writeInt32(self._texid)
    bf.writeInt32(self._x)
    bf.writeInt32(self._y)
    bf.writeInt32(self._w)
    bf.writeInt32(self._h)
    bf.writeInt32(len(self._imgs))
    for img in self._imgs:
      bf.writeString(img)

  def deserialize(self, bf: BinaryFile):
    self._seq = bf.readFloat()
    self._texid = bf.readInt32()
    self._x = bf.readInt32()
    self._y = bf.readInt32()
    self._w = bf.readInt32()
    self._h = bf.readInt32()
    ilen = bf.readInt32()
    for i in range(0, ilen):
      self.imgs.append(bf.readString())

class ActionInfo:
  def __init__(self):
    self._name = ""
    self._action = None  # if none then this is sprite
    self._sweepBox = None
    self._start = 0
    self._end = 0
    self._texs: dict = {}
class ObjectInfo:
  def __init__(self):
    self._name = ""
    self._object = None
    self._enclosingBox = None
    self._sweepBox = None  # of all obs and anims
    self._actions = {}
    self._min_start = 99999  # of all anims
    self._max_end = -99999

class RenderInfo:
  _render_engine: RenderEngine = RenderEngine.Eevee
  _render_samples: int = 3  # note at 1 sample there is no edge blur in eevee
  _render_size: int = 256  # rendered image size, scaled to maximum of w/h dimension
  _azumith_slices: int = 0
  _zenith_slices: int = 0
  _sampleRate: float = 1
  _normalizeDepth: bool = True
  _perspective: bool = False  # True #True  # perspective / orthographic changing to ortho causes the image to shrink we'd need to change orthographic bounds as well (ortho_scale)
  _fov: float = 40.0
  _edge_thicknes: float = 0.8  # 0=minimal edge 1 = maximal edge
  _edge_blur: float = 0.4  # 0= no blur edges 1 = full blur edges

  #temp (computed)
  _render_width: int = 0
  _render_height: int = 0
  _camera = None
  _empty = None
  _customCamera = False
  _camname = '__b26camera'
  _empname = '__b26empty'
  _dbg_renderCount = 0
  _compNodes: dict = {}

class Bake26:
  # region Constants

  c_file_version = '0.01'
  c_sprite_name = 'Sprite'
  c_inputFileSwitch = '-i'
  c_framePlaceholder = "####"
  c_OBNM = 'B26'

  c_backupDirName = 'backup'
  c_metafile_version_major = 0
  c_metafile_version_minor = 2
  c_bobj_binext='.bobj.bin'
  c_max_tex_size_px = int(2048)
  c_uniform_tex_size = True  # TextureArrays cant be variable sizes this must always be true

  c_layers = [
    LayerOutput(ExportLayer.Color, ExportFormat.PNG, 8),
    LayerOutput(ExportLayer.DepthNormal, ExportFormat.PNG, 8),
    #LayerOutput(ExportLayer.Position, ExportFormat.PNG, 16),
    ]

  @staticmethod
  def getOutput(layer: ExportLayer):
    y = [x for x in Bake26.c_layers if x._layer == layer]
    if len(y) > 0:
      return y[0]
    return None

  # endregion

  def __init__(self, args):
    self._outPath = ""
    self._libFile = ""
    self._data = RenderInfo()
    self._is_cmd = not bpy.data.filepath
    self._fileObjName = ""  # default obj name for sprites
    self._doGif = True if args.do_gif else False
    self._doVid = True if args.do_vid else False

    if args.pack_only:
      self.packTextures(args.outpath)
    else:
      self.export_files(args)

  def export_files(self, args):
    args.libpath = os.path.normpath(args.libpath)
    args.outpath = os.path.normpath(args.outpath)

    if not os.path.exists(args.outpath):
      msg("creating '" + args.outpath + "'")
      os.makedirs(args.outpath)
    self._outPath = args.outpath
    assert(os.path.exists(args.libpath))
    self._libFile = args.libpath
    self._is_cmd = not bpy.data.filepath
    if not self._is_cmd:
      throw("can only run from terminal for now")

    # blender is crashing doing multiple files, so we're going to do subprocess and re-enter this script with the .blend
    if os.path.isfile(args.inpath):
      self._fileObjName, fe = os.path.splitext(os.path.basename(args.inpath))
      bpy.ops.wm.open_mainfile(filepath=args.inpath)
      self.appendLibData()
      self.exportBlendFile()
      # self._data = None
      # bpy.ops.wm.quit_blender() #?? crashing
      # msg(str(bpy.data.filepath))
    else:
      def asyncExportBlend(fpath):
        if fpath != self._libFile:
          cmd = ""
          for i in range(0, len(sys.argv)):
            if i > 0 and sys.argv[i - 1] == Bake26.c_inputFileSwitch:
              cmd += " " + fpath
            elif cmd != "":
              cmd += " " + sys.argv[i]
            else:
              cmd = sys.argv[i]
          subprocess.run(cmd, shell=True)
      Utils.loopFilesByExt(args.inpath, '.blend', asyncExportBlend)
      if args.pack_enable:
        self.packTextures(args.outpath)

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

  def exportBlendFile(self):
    try:
      bpy.ops.object.mode_set(mode='OBJECT')
      # just render the object with animations

      inf = self.getRenderInfo()
      self.setup()
      self.setRenderSettings()
      self.focusCamera(inf)
      self.makeOutputs()

      bpy.context.view_layer.update()
      self.renderObjectAnimations(inf)

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
    bpy.context.scene.render.engine = self._data._render_engine   # 'BLENDER_EEVEE'  # 'BLENDER_EEVEE' 'BLENDER_WORKBENCH' 'CYCLES'
    if self._data._render_engine == RenderEngine.Cycles:
      bpy.context.scene.cycles.samples = self._data._render_samples
      bpy.context.scene.cycles.aa_samples = 1
      bpy.context.scene.cycles.ao_samples = 1
      bpy.context.scene.cycles.diffuse_samples = 1
      bpy.context.scene.cycles.glossy_samples = 1
      bpy.context.scene.cycles.mesh_light_samples = 1
      bpy.context.scene.cycles.subsurface_samples = 1
      bpy.context.scene.cycles.transmission_samples = 1
      bpy.context.scene.cycles.volume_samples = 1
    elif self._data._render_engine == RenderEngine.Eevee:
      bpy.context.scene.eevee.taa_render_samples = self._data._render_samples
    bpy.context.scene.render.resolution_x = self._data._render_width
    bpy.context.scene.render.resolution_y = self._data._render_height
    bpy.context.scene.render.pixel_aspect_x = 1.0
    bpy.context.scene.render.pixel_aspect_y = 1.0
    bpy.context.scene.render.resolution_percentage = 100
    bpy.context.scene.render.film_transparent = True
    bpy.context.scene.render.filter_size = 1
    bpy.context.scene.render.use_motion_blur = False
    bpy.context.scene.render.use_stamp = False
    bpy.context.scene.render.use_persistent_data = True  # supposed to make animation renders faster
    #bpy.context.scene.render.use_overwrite = True
    bpy.context.scene.render.use_compositing = True
    bpy.context.scene.render.use_sequencer = False
    bpy.context.scene.render.threads_mode = 'FIXED'  # typically blender wont use all threads, but we want to use all threads except for 1
    bpy.context.scene.render.threads = max(multiprocessing.cpu_count()-2, 1)
    #bpy.context.scene.render.use_high_quality_normals = False
    #bpy.context.scene.render.use_simplify = False
    #bpy.context.scene.render.use_placeholder = True
    bpy.context.scene.render.filepath = ''
    bpy.context.scene.render.use_file_extension = False

  def makeOutputs(self):
    # https://docs.blender.org/api/current/bpy.types.CompositorNodeRLayers.html
    # https://docs.blender.org/api/current/bpy.types.CompositorNodeOutputFile.html
    # https://docs.blender.org/api/current/bpy.types.ViewLayer.html
    # https://blender.stackexchange.com/questions/105193/unable-to-store-renderings-in-openexr-when-using-the-multiprocessing-in-python
    # https://blender.stackexchange.com/questions/194564/is-it-possible-to-save-depth-values-and-rendered-image-in-different-formats

    # note: position only in cycles
    bpy.context.scene.use_nodes = True
    bpy.context.scene.view_layers["ViewLayer"].use_pass_ambient_occlusion = False
    bpy.context.scene.view_layers["ViewLayer"].use_pass_combined = False
    bpy.context.scene.view_layers["ViewLayer"].use_pass_diffuse_color = True
    bpy.context.scene.view_layers["ViewLayer"].use_pass_diffuse_direct = True
    bpy.context.scene.view_layers["ViewLayer"].use_pass_diffuse_indirect = False
    bpy.context.scene.view_layers["ViewLayer"].use_pass_emit = False
    bpy.context.scene.view_layers["ViewLayer"].use_pass_environment = False  # True
    bpy.context.scene.view_layers["ViewLayer"].use_pass_glossy_color = False  # True
    bpy.context.scene.view_layers["ViewLayer"].use_pass_glossy_direct = False  # True
    bpy.context.scene.view_layers["ViewLayer"].use_pass_glossy_indirect = False  # True
    bpy.context.scene.view_layers["ViewLayer"].use_pass_normal = True
    bpy.context.scene.view_layers["ViewLayer"].use_pass_position = True
    bpy.context.scene.view_layers["ViewLayer"].use_pass_shadow = False
    bpy.context.scene.view_layers["ViewLayer"].use_pass_z = True
    bpy.context.scene.view_layers["ViewLayer"].use_pass_mist = False

    for node in bpy.context.scene.node_tree.nodes:
      bpy.context.scene.node_tree.nodes.remove(node)

    render_layers = bpy.context.scene.node_tree.nodes.new(type='CompositorNodeRLayers')

    for output in Bake26.c_layers:
      self.makeOutput(output, render_layers)

  def makeOutput(self, fout: LayerOutput, render_layers):
    # https://docs.blender.org/api/current/bpy.types.ImageFormatSettings.html
    cnode = bpy.context.scene.node_tree.nodes.new(type='CompositorNodeOutputFile')
    cnode.label = fout._layer
    cnode.base_path = ""

    if fout._format == ExportFormat.PNG:
      cnode.format.file_format = 'PNG'
      cnode.format.color_depth = str(fout._bitdepth)
      cnode.format.color_mode = fout._mode
      cnode.format.compression = fout._compression
      # output.format.use_zbuffer = True #getting weird depth settings
    elif fout._format == ExportFormat.OPEN_EXR:
      cnode.format.file_format = 'OPEN_EXR'  # 'OPEN_EXR'  # HDR
      cnode.format.color_depth = str(fout._bitdepth)
      cnode.format.color_mode = fout._mode
      cnode.format.exr_codec = 'ZIP'
      cnode.format.use_zbuffer = False
      cnode.format.compression = fout._compression
    else:
      throw()
    if fout._layer == ExportLayer.Color:
      deblur = bpy.context.scene.node_tree.nodes.new(type='CompositorNodeGroup')
      deblur.node_tree = bpy.data.node_groups['EdgeDeblurGroup']
      bpy.context.scene.node_tree.links.new(render_layers.outputs["Image"], deblur.inputs["Image"])
      bpy.context.scene.node_tree.links.new(render_layers.outputs["Alpha"], deblur.inputs["Alpha"])
      deblur.inputs['Thickness'].default_value = self._data._edge_thicknes
      deblur.inputs['Blur'].default_value = self._data._edge_blur
      bpy.context.scene.node_tree.links.new(deblur.outputs["Image"], cnode.inputs['Image'])
    elif fout._layer == ExportLayer.DepthNormal:
      combine_depth_normal = bpy.context.scene.node_tree.nodes.new(type='CompositorNodeGroup')
      combine_depth_normal.node_tree = bpy.data.node_groups['DepthNormalCombine']

      dbg("cam="+str(self._data._camera.location) + " empty="+str(self._data._empty.location))
      campos = Bake26.makeInputVec3(self._data._camera.location)
      emptypos = Bake26.makeInputVec3(self._data._empty.location)
      normdepth = Bake26.makeInputFloat(1 if self._data._normalizeDepth == True else 0)

      bpy.context.scene.node_tree.links.new(render_layers.outputs['Normal'], combine_depth_normal.inputs['Normal'])
      bpy.context.scene.node_tree.links.new(render_layers.outputs['Depth'], combine_depth_normal.inputs['Z'])
      bpy.context.scene.node_tree.links.new(campos.outputs[0], combine_depth_normal.inputs['CamPos'])
      bpy.context.scene.node_tree.links.new(emptypos.outputs[0], combine_depth_normal.inputs['ProjectPos'])
      bpy.context.scene.node_tree.links.new(normdepth.outputs[0], combine_depth_normal.inputs['Normalize'])

      bpy.context.scene.node_tree.links.new(combine_depth_normal.outputs['Image'], cnode.inputs['Image'])
    elif fout._layer == ExportLayer.Position:
      bpy.context.scene.node_tree.links.new(render_layers.outputs['Position'], cnode.inputs['Image'])
    else:
      throw()

    self._data._compNodes[fout._layer] = cnode

  def makeInputVec3(vec: Vector):
    nv_x = bpy.context.scene.node_tree.nodes.new(type='CompositorNodeValue')
    nv_x.outputs[0].default_value = vec.x
    nv_y = bpy.context.scene.node_tree.nodes.new(type='CompositorNodeValue')
    nv_y.outputs[0].default_value = vec.y
    nv_z = bpy.context.scene.node_tree.nodes.new(type='CompositorNodeValue')
    nv_z.outputs[0].default_value = vec.z
    nv = bpy.context.scene.node_tree.nodes.new(type='CompositorNodeCombineXYZ')
    bpy.context.scene.node_tree.links.new(nv_x.outputs[0], nv.inputs[0])
    bpy.context.scene.node_tree.links.new(nv_y.outputs[0], nv.inputs[1])
    bpy.context.scene.node_tree.links.new(nv_z.outputs[0], nv.inputs[2])
    return nv

  def makeInputFloat(f: float):
    nv = bpy.context.scene.node_tree.nodes.new(type='CompositorNodeValue')
    nv.outputs[0].default_value = f
    return nv

  def renderObjectAnimations(self, inf):
    msg("name=" + inf._name)
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
      ainf._name = Bake26.c_sprite_name
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
    b2ob = b2_obj()
    b2ob._name = inf._name

    def renderob(frame: float, actioninf):
      b2a = b2_action()
      b2a._name = actioninf._name
      b2a._id = len(b2ob._actions)+1
      b2ob._actions.append(b2a)

      if self._data._customCamera == False:
        self.renderAngles(actioninf, inf._name, frame, b2a)
      else:
        self.renderAction(actioninf, inf._name, 0, 0, 0, b2a)

    msg("exporting: " + str(inf._min_start) + "-" + str(inf._max_end))
    self.animateObjectActions(inf, renderob)

    for actname in inf._actions:
      if actname != Bake26.c_sprite_name:
        self.exportPreview(inf._name, actname)

    self.writeObjMetadata(b2ob)

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
      self._data._render_width = self._data._render_size
      self._data._render_height = int(round(self._data._render_size * sz / sx))
    else:
      self._data._render_height = self._data._render_size
      self._data._render_width = int(round(self._data._render_size * sx / sz))
    msg("render res =" + str(self._data._render_width) + "x" + str(self._data._render_height))

    bpy.context.scene.render.resolution_x = self._data._render_width
    bpy.context.scene.render.resolution_y = self._data._render_height
    bpy.context.view_layer.update()

  def animateObjectActions(self, inf, func):
    # loop all actions in object and animate them at all keyframe intervals
    def animateAnims(frame: float):
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

  def exportPreview(self, obname, actname):
    path = self.actionOutputPath(obname, actname)
    #msg("path="+str(path))
    assert(os.path.exists(path))
    colorout = Bake26.getOutput(ExportLayer.Color)
    if colorout == None:
      err("could not output preview - no color format was output")
    else:
      ext = colorout.layer_ext()
      fcount = len(glob.glob1(path, "*"+ext))
      if fcount > 1:
        if self._doVid:
          self.exportMp4(path, fcount, ext)
        if self._doGif:
          self.exportGif(path, fcount, ext)
  @staticmethod
  def frameRate(dividend: float):
    csframe = (dividend / (float(bpy.context.scene.render.fps) / float(self._data._sampleRate)))

  def exportGif(self, path, fcount, ext):
    msg("doing gif, fcount="+str(fcount))
    csframe = Bake26.frameRate(100.0)  # (100.0 / (float(bpy.context.scene.render.fps) / float(self._data._sampleRate)))
    cmd = "convert -delay " + str(csframe) + " -dispose Background -loop 0 " + path + "/*"+ext+" " + path + "/output.gif &"
    subprocess.run(cmd, shell=True)

  def exportMp4(self, path, fcount, ext):
    msg("doing mp4, fcount="+str(fcount))
    ss = 1
    sclxy = ""
    if ss != 1:
      sx = max(int(self._data._render_width * ss), 1024)
      sy = max(int(self._data._render_height * ss), 1024)
      sclxy = " -s " + str(sx) + "x" + str(sy) + " "
    mp4frame = (float(bpy.context.scene.render.fps) / float(self._data._sampleRate))
    cmd = "ffmpeg -framerate " + str(int(mp4frame)) + " -stream_loop 5 -pattern_type glob -i '" + path + "/*"+ext+"' -crf 0 " + sclxy + " -c:v libx265 -x265-params lossless=1 '" + spritepath + "/output.mp4'"
    msg("cmd=" + cmd)
    subprocess.run(cmd, shell=True)

  def renderAngles(self, actioninf, obname, frame: float, act: b2_action):
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

        self.renderAction(actioninf, obname, frame, azumith, zenith, act)

  def renderAction(self, actioninf, obname, frame: float, azumith, zenith, act: b2_action):
    fpath = self.actionFilePath(obname, actioninf._name, frame, azumith, zenith)

    # blender's paths are just screwed up sorry to say. possible bug.
    # chdir to root then add the // relpath for blender
    # blender removes the relpath and appends the path to root
    # note: blender reports output as 'home/' without root
    b2f = b2_frame()
    b2f._seq = frame
    act._frames.append(b2f)

    dn = os.path.dirname('/')  # bpy.data.filepath)
    if not os.getcwd() == dn:
      os.chdir(dn)

    bpy.context.scene.render.filepath = ''

    for out in Bake26.c_layers:
      abs_path = fpath + out.layer_ext()
      #rel_path = bpy.path.relpath(rel_path)
      cnode = self._data._compNodes[out._layer]
      cnode.base_path = ''
      cnode.file_slots[0].path = '//' + abs_path
      abspath_w_frame = abs_path.replace(Bake26.c_framePlaceholder, str(int(frame)).zfill(len(Bake26.c_framePlaceholder)))
      if out._layer not in actioninf._texs:
        actioninf._texs[out._layer] = []
      actioninf._texs[out._layer].append(abspath_w_frame)
      b2f._imgs.append(abspath_w_frame)

    bpy.ops.render.render(write_still=False)  # use only compositor outputs
    self._data._dbg_renderCount += 1

  def objectOutputPath(self, obname, backup, backup_id):
    path = os.path.abspath(os.path.normpath(self._outPath))
    if backup:
      path = os.path.join(path, os.path.normpath(Bake26.c_backupDirName))
      path = os.path.join(path, os.path.normpath(os.path.basename(bpy.data.filepath)))
      path = os.path.join(path, os.path.normpath(obname + "." + str(backup_id)))
    else:
      path = os.path.join(path, os.path.normpath(os.path.basename(bpy.data.filepath)))
      path = os.path.join(path, os.path.normpath(obname))
    return path

  def actionOutputPath(self, obname, actname):
    path = self.objectOutputPath(obname, False, 0)
    path = os.path.join(path, os.path.normpath(actname))
    return path

  def actionFilePath(self, obname, actname, frame, azumith, zenith):
    # actionFileName spriteFileName
    sep = Bake26.spritePathSeparator()
    # con obj act az   zen  fr  frf ext
    # ID.guy.run.0025.0008.0034.200
    # --> output ext will be layer -> .Color.png etc

    # cant have delimiter in names.
    assert(not sep in obname)
    assert(not sep in actname)

    # TODO use the regex in the metadata and just replace

    fname = Bake26.c_OBNM + sep
    fname += obname + sep
    fname += actname + sep
    fname += str(str(azumith).zfill(len(Bake26.c_framePlaceholder))) + sep
    fname += str(str(zenith).zfill(len(Bake26.c_framePlaceholder))) + sep
    fname += Bake26.c_framePlaceholder + sep  # blender file outputs must always have frame numbers so we put them here
    frmf = int(round(frame - float(int(frame)), 3) * 1000)
    fname += str(str(frmf).zfill(3)) + sep

    path = self.actionOutputPath(obname, actname)
    path = os.path.join(path, fname)

    return path

  def spritePathSeparator():
    return "@"  # could use |

  def packTextures(self, outpath):
    texs = dict()
    outpath = os.path.abspath(outpath)

    #throw("TODO: gatherObjs and use their texture strings")
    objs = []
    Bake26.gatherObjs(outpath, objs)  # skip /output because we put the mt's in there

    #Bake26.gatherTextures(outpath, texs, True)  # skip /output because we put the mt's in there

    if len(texs) == 0:
      throw("no textures found. exporter must be run")

    texs_cpy = dict(texs)
    pts = ImagePacker.packImages(texs)

    for pt in pts:
      #msg("writing " + " id=" + str(pt._id) + " count=" + str(pt._regioncount) + " size=" + str(pt._size))
      pt.compose(outpath)
    self.writeMetadata(outpath, texs_cpy, pts)

  def writeMetadata(self, outpath, texs, pts):
    mdpath = os.path.join(outpath, "B2MT.bin")
    msg("saving metadata path="+mdpath)

    #obj_dict = {}

    # build objects
    #for region_fname in texs:
    #  region = texs[region_fname]
    #  assert(region._node != None)  # call this AFTER  build the nodetree
    throw("TODO")
#
    #  # **TODO: we ar3e getting rid of this in favor of just saving the obj metadata
    #  Bake26.buildB2Obj(region, region_fname, obj_dict)

    # write file
    with open(mdpath, 'wb') as file:
      bf = BinaryFile(file)

      bf.writeByte(int(ord('B'.encode('utf-8'))))
      bf.writeByte(int(ord('2'.encode('utf-8'))))
      bf.writeByte(int(ord('M'.encode('utf-8'))))
      bf.writeByte(int(ord('D'.encode('utf-8'))))

      bf.writeInt32(self.c_metafile_version_major)
      bf.writeInt32(self.c_metafile_version_minor)

      bf.writeInt32(bpy.context.scene.render.fps)
      bf.writeInt32(Bake26.bpy.context.scene.render.fps)


      # texs
      bf.writeInt32(len(pts))
      for i in range(0, len(pts)):
        pts[i].serialize(bf)

      # objs
      throw("TODO")
      # bf.writeInt32(len(obj_dict))
      # for obname in obj_dict:
      #   ob = obj_dict[obname]
      #   ob.serialize(bf)

  def writeObjMetadata(self, ob):
    # post render metadata write
    fpath = self.objectOutputPath(ob._name, False, 0)
    fpath = os.path.join(fpath, ob._name + Bake26.c_bobj_binext)
    msg("saving obj metadata to " + fpath)

    with open(fpath, 'wb') as file:
      bf = BinaryFile(file)
      ob.serialize(bf)

  def gatherObjs(fpath: str, objs : list):  # , color_extension: str, layers: dict
    fpath = os.path.abspath(fpath)
    fds = os.listdir(fpath)  # get all files' and folders' names in the current directory

    for item in fds:
      file_path = os.path.join(fpath, item)

      if os.path.isfile(file_path):
        if Bake26.c_bobj_binext in file_path:
          with open(file_path, 'wb') as file:
            bf = BinaryFile(file)
            ob = b2_obj ()
            ob.deserialize(bf)
            objs.append(ob)
            throw("TODO")
#           layer_ext = LayerOutput.parse_extension(item)
# 
#           if layer_ext != None:
#             if layer_ext == color_extension:
#               w, h = Utils.get_image_size(file_path)
#               assert(w > 0)
#               assert(h > 0)
#               texs[file_path] = MtRegion()
#               texs[file_path]._pathColor = file_path
#               texs[file_path]._width = int(w)
#               texs[file_path]._height = int(h)
#               texs[file_path]._texid = -1
#               texs[file_path]._node = None
#               texs[file_path]._layers = {}
# 
#               for output in Bake26.c_layers:
#                 out_layer_ext = output.layer_ext()
#                 layer_path = file_path.replace(layer_ext, out_layer_ext)
#                 texs[file_path]._layers[output._layer] = layer_path


      elif os.path.isdir(file_path):
        if item != '.' and not Bake26.c_backupDirName in item:
          Bake26.gatherTextures(file_path, objs)

#   def buildB2Obj(region: MtRegion, region_fname: str, obj_dict: dict):
#     sep = Bake26.spritePathSeparator()
#     grp = '([^'+sep+']+)'
#     # parse name
#     rxstr = Bake26.c_OBNM
#     rxstr += sep + grp  # ob 0
#     rxstr += sep + grp  # act 1
#     rxstr += sep + grp  # a 2
#     rxstr += sep + grp  # z 3
#     rxstr += sep + grp  # ifr 4
#     rxstr += sep + grp  # ffr 5
#     rxstr += sep + '(.+)'  # ext 6
#     # msg("fname="+region_fname)
#     # msg("rxstr="+rxstr)
#     fname = os.path.basename(region_fname)
#     rx = re.search(rxstr, fname)
#
#     if (not rx) or (len(rx.groups()) != 7):
#       raise Exception("Invalid file format group count ="+str(len(rx.groups())))
#
#     obname = rx.groups()[0]
#     actname = rx.groups()[1]
#     ifr = rx.groups()[4]
#     ffr = rx.groups()[5]
#
#     if not obname in obj_dict:
#       obj = b2_obj()
#       obj._id = b2_obj._ob_idgen
#       b2_obj._ob_idgen += 1
#       obj._name = obname
#       obj_dict[obname] = obj
#     obj = obj_dict[obname]
#     if not actname in obj._actions:
#       act = b2_action()
#       act._id = len(obj._actions)+1
#       act._name = actname
#       obj._actions[actname] = act
#     act = obj._actions[actname]
#     fr = b2_frame()
#     fr._texid = region._texid
#     fr._x = region._node._rect.left()
#     fr._y = region._node._rect.top()
#     fr._w = region._node._rect.width()
#     fr._h = region._node._rect.height()
#     seq_str = ifr+"."+ffr
#     assert(not seq_str in act._frames)
#     seq = float(seq_str)  # "0004.200"
#     fr._seq = seq
#     act._frames[seq_str] = fr          
#   def gatherTextures(fpath: str, texs: dict, skipdir: bool):  # , color_extension: str, layers: dict
#     fpath = os.path.abspath(fpath)
#     fds = os.listdir(fpath)  # get all files' and folders' names in the current directory
#     color_extension = Bake26.getOutput(ExportLayer.Color).layer_ext()
# 
#     for item in fds:
# 
#       file_path = os.path.join(fpath, item)
# 
#       if os.path.isfile(file_path):
#         if skipdir == False:
# 
#           layer_ext = LayerOutput.parse_extension(item)
# 
#           if layer_ext != None:
#             if layer_ext == color_extension:
#               w, h = Utils.get_image_size(file_path)
#               assert(w > 0)
#               assert(h > 0)
#               texs[file_path] = MtRegion()
#               texs[file_path]._pathColor = file_path
#               texs[file_path]._width = int(w)
#               texs[file_path]._height = int(h)
#               texs[file_path]._texid = -1
#               texs[file_path]._node = None
#               texs[file_path]._layers = {}
# 
#               for output in Bake26.c_layers:
#                 out_layer_ext = output.layer_ext()
#                 layer_path = file_path.replace(layer_ext, out_layer_ext)
#                 texs[file_path]._layers[output._layer] = layer_path
# 
#       elif os.path.isdir(file_path):
#         if item != '.' and not Bake26.c_backupDirName in item:
#           Bake26.gatherTextures(file_path, texs, False)


# ======================================================================================


def getArgs():
  argv = sys.argv
  if "--" not in argv:
    argv = []
  else:
    argv = argv[argv.index("--") + 1:]
  parser = argparse.ArgumentParser(description="MRT sprite baker")
  parser.add_argument(Bake26.c_inputFileSwitch, dest="inpath", type=str, required=True, help="path to .blend files")
  parser.add_argument("-o", dest="outpath", type=str, required=True, help="output path")
  parser.add_argument("-l", dest="libpath", type=str, required=True, help="library file path")
  parser.add_argument("-p", dest="pack_enable", action='store_true', required=False, help="pack textures in -o")
  parser.add_argument("-P", dest="pack_only", action='store_true', required=False, help="only pack textures in -o, dont export")
  parser.add_argument("-v", dest="do_vid", action='store_true', required=False, help="generate video preview of animation")
  parser.add_argument("-g", dest="do_gif", action='store_true', required=False, help="generate gif preview of animation")
  args = parser.parse_args(argv)
  return args

try:
  try:
    args = getArgs()
  except Exception as e:
    Utils.printExcept(e)

  mob = Bake26(args)
except Exception as e:
  Utils.printExcept(e)
