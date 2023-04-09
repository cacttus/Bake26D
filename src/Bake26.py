# python3
# Blender MRT sprite exporter (2.6D)
#
# blender:
# currently disabled
#   ___x='/home/mario/git/Bake26D/Bake26.py';exec(compile(open(___x).read(), ___x, 'exec'))

# terminal:
#  clear; ~/Desktop/apps/blender*/blender -b --log-level 0 -P ~/git/Bake26D/src/Bake26.py  -- -i ~/git/Bake26D/assets/ -o ~/git/Bake26D/b26out -l ~/git/Bake26D/assets/_library.blend -p
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

class MtRegion: pass  # nopep8
class MtNode: pass  # nopep8
class PackedTexture: pass  # nopep8
class ImagePacker: pass  # nopep8
class ActionInfo: pass  # nopep8
class ObjectInfo: pass  # nopep8
class RenderInfo: pass  # nopep8
class Bake26: pass  # nopep8
class ExportFormat: pass  # nopep8

class ExportFormat:
  PNG = 'PNG'
  EXR = 'EXR'
class ExportLayer:
  Color = 'Color'
  DepthNormal = 'DepthNormal'

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
class b2_obj:
  _ob_idgen = 1
  def __init__(self):
    self._id = -1
    self._name = ""
    self._actions = {}
  def serialize(self, bf: BinaryFile):
    bf.writeInt32(self._id)
    bf.writeString(self._name)
    bf.writeInt32(len(self._actions))
    for act in self._actions:
      self._actions[act].serialize(bf)
class b2_action:
  def __init__(self):
    self._id = -1
    self._name = ""
    self._frames = {}
  def serialize(self, bf: BinaryFile):
    bf.writeInt32(self._id)
    bf.writeString(self._name)
    bf.writeInt32(len(self._frames))
    for fr in self._frames:
      self._frames[fr].serialize(bf)
class b2_frame:
  def __init__(self):
    self._seq: float = -1  # float
    self._texid = -1
    self._x = -1
    self._y = -1
    self._w = -1
    self._h = -1
  def serialize(self, bf: BinaryFile):
    bf.writeFloat(self._seq)
    bf.writeInt32(self._texid)
    bf.writeInt32(self._x)
    bf.writeInt32(self._y)
    bf.writeInt32(self._w)
    bf.writeInt32(self._h)
class b2_tex:
  def __init__(self):
    self._texid = -1
    self._texs: str = []

class PackedTexture:
  def __init__(self):
    self._texid = 0
    self._size = 0  # w and h
    self._regioncount = 0
    self._root: MtNode = None
    self._texnames = []

  def compose(self, outpath):
    assert(self._root)
    # todo loop generic
    self.packLayer(outpath, 'mt_color', 0)
    self.packLayer(outpath, 'mt_normal_depth', 1)

  def packLayer(self, outpath, name, layer):
    msg("packing "+name+" layer="+str(layer)+" size="+str(self._size))
    ctest = (255, 0, 255, 255)
    czero = (0, 0, 0, 0)
    with Image.new(mode="RGBA", size=(self._size, self._size), color=czero) as master_img:
      # note imae ha 'I' and 'F' for 32 bits
      #master_img = Image.new(mode="RGBA", size= (self._size, self._size), color=(0,0,0,0))
      self.copyImages(self._root, master_img, layer)
      texname = name+".png"
      self._texnames.append(texname)
      path = os.path.join(outpath, texname)
      msg("saving " + path)
      msg(str(master_img.info))
      master_img.save(path)

  def copyImages(self, node, master, layer):
    assert(node)

    # also create objs for metadata
    if node._texture != None:
      path = node._texture._layers[layer]
      if os.path.exists(path):
        with Image.open(path) as img:
          Image.Image.paste(master, img, (node._rect.left(), node._rect.top()))

    if node._child[0] != None:
      self.copyImages(node._child[0], master, layer)
    if node._child[1] != None:
      self.copyImages(node._child[1], master, layer)

  def serialize(self, bf: BinaryFile):
    bf.writeInt32(self._texid)
    bf.writeInt32(len(self._texnames))
    for tn in self._texnames:
      bf.writeString(tn)

class ImagePacker:
  def packImages(texs, startsize=256, growsize=256):
    # returns [PackedTextureLayers ]
    mega_texs = []
    size = int(startsize)
    growsize = int(growsize)
    maxsize = int(4096)

    # note you cant get gpu maxsize in headless blender.. we just need to pick a value
    msg("packing " + str(len(texs)) + " texs. maxsize=" + str(maxsize))

    # pack until max wh
    # then remove from texs
    # then pack again

    cur_tex_id = 1
    while size <= maxsize:
      size = min(int(size), int(maxsize))

      pt = ImagePacker.packForSize(texs, size, int(size) == int(maxsize), cur_tex_id)

      if pt != None:
        pt._id = cur_tex_id
        cur_tex_id += 1
        ImagePacker.removePlopped(pt._root, texs)
        mega_texs.append(pt)
        size = startsize
      else:
        size = min(int(size + growsize), maxsize)

      if len(texs) == 0:
        break

    return mega_texs

  def packForSize(texs, size, force, tex_id):
    pt = PackedTexture()
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

class ActionInfo:
  def __init__(self):
    self._name = ""
    self._action = None  # if none then this is sprite
    self._sweepBox = None
    self._start = 0
    self._end = 0
    self._texs = {ExportLayer.Color: [], ExportLayer.DepthNormal: []}
class ObjectInfo:
  def __init__(self):
    self._name = ""
    self._object = None
    self._enclosingBox = None
    self._sweepBox = None  # of all obs and anims
    self._actions = {}
    self._min_start = 99999  # of all anims
    self._max_end = -99999
class FileOutput:
  def __init__(self, node, format, layer):
    self._node = node
    self._format: ExportFormat = format
    self._layer: ExportLayer = layer
  def get_extension(self):
    return FileOutput.extension(self._format, self._layer)
  @staticmethod
  def extension(fmt: ExportFormat, layer: ExportLayer):
    return '.'+layer+'.'+fmt.lower()
  @staticmethod
  def parse_extension(path):
    d = path.rfind('.')
    if d > 0:
      d = path[:d].rfind('.')
      if d > 0:
        return path[d:]
    return None

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
  _edge_thicknes = 0.8  # 0=minimal edge 1 = maximal edge
  _edge_blur = 0.4  # 0= no blur edges 1 = full blur edges
  _camera = None
  _empty = None
  _customCamera = False
  _camname = '__b26camera'
  _empname = '__b26empty'
  _dbg_renderCount = 0
  _fileOutputs: FileOutput = {}
  _exportFormat: ExportFormat = ExportFormat.PNG
  _exportBitDepth = 8

class Bake26:
  # region Constants

  c_file_version = '0.01'
  c_sprite_name = 'Sprite'
  c_inputFileSwitch = '-i'
  c_framePlaceholder = "####"
  c_OBNM = 'B26'

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
      self.makeOutputs()
      self.focusCamera(inf)

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
    bpy.context.scene.render.engine = 'BLENDER_EEVEE'  # 'BLENDER_EEVEE' 'BLENDER_WORKBENCH' 'CYCLES'
    bpy.context.scene.eevee.taa_render_samples = self._data._samples
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
    # https://docs.blender.org/api/current/bpy.types.ImageFormatSettings.html
    #bpy.context.scene.render.image_settings.color_mode = 'RGBA'
    #bpy.context.scene.render.image_settings.file_format = 'PNG'
    # bpy.context.scene.render.image_settings.color_depth = '8'  # ('8', '16')
    # bpy.context.scene.render.image_settings.use_zbuffer = False  # TRYING TO DISABLE DEFAULT RENDER BUT USE OUTUPTS
    #bpy.context.scene.render.image_settings.compression = 100

  def makeOutputs(self):
    # https://docs.blender.org/api/current/bpy.types.CompositorNodeRLayers.html
    # https://docs.blender.org/api/current/bpy.types.CompositorNodeOutputFile.html
    # https://docs.blender.org/api/current/bpy.types.ViewLayer.html
    # https://blender.stackexchange.com/questions/105193/unable-to-store-renderings-in-openexr-when-using-the-multiprocessing-in-python
    # https://blender.stackexchange.com/questions/194564/is-it-possible-to-save-depth-values-and-rendered-image-in-different-formats

    bpy.context.scene.use_nodes = True
    bpy.context.scene.view_layers["ViewLayer"].use_pass_ambient_occlusion = False
    bpy.context.scene.view_layers["ViewLayer"].use_pass_combined = False
    bpy.context.scene.view_layers["ViewLayer"].use_pass_diffuse_color = True
    bpy.context.scene.view_layers["ViewLayer"].use_pass_diffuse_direct = True
    bpy.context.scene.view_layers["ViewLayer"].use_pass_diffuse_indirect = False
    bpy.context.scene.view_layers["ViewLayer"].use_pass_emit = False
    bpy.context.scene.view_layers["ViewLayer"].use_pass_environment = True
    bpy.context.scene.view_layers["ViewLayer"].use_pass_glossy_color = True
    bpy.context.scene.view_layers["ViewLayer"].use_pass_glossy_direct = True
    bpy.context.scene.view_layers["ViewLayer"].use_pass_glossy_indirect = True
    bpy.context.scene.view_layers["ViewLayer"].use_pass_normal = True
    bpy.context.scene.view_layers["ViewLayer"].use_pass_position = False
    bpy.context.scene.view_layers["ViewLayer"].use_pass_shadow = False
    bpy.context.scene.view_layers["ViewLayer"].use_pass_z = True
    # several more

    # remove all existing compositor nodes
    for node in bpy.context.scene.node_tree.nodes:
      bpy.context.scene.node_tree.nodes.remove(node)

    render_layers = bpy.context.scene.node_tree.nodes.new(type='CompositorNodeRLayers')

    # get groups
    deblur = bpy.context.scene.node_tree.nodes.new(type='CompositorNodeGroup')
    deblur.node_tree = bpy.data.node_groups['EdgeDeblurGroup']
    #deblur.name = 'Deblur'
    combine_depth_normal = bpy.context.scene.node_tree.nodes.new(type='CompositorNodeGroup')
    combine_depth_normal.node_tree = bpy.data.node_groups['DepthNormalCombine']
    #combine_depth_normal.name = 'CombineDepthNormal'

    # deblur render
    bpy.context.scene.node_tree.links.new(render_layers.outputs["Image"], deblur.inputs["Image"])
    bpy.context.scene.node_tree.links.new(render_layers.outputs["Alpha"], deblur.inputs["Alpha"])
    deblur.inputs['Thickness'].default_value = self._data._edge_thicknes
    deblur.inputs['Blur'].default_value = self._data._edge_blur

    # composite
    # composite_node = bpy.context.scene.node_tree.nodes.new(type='CompositorNodeComposite')  # leave empty
    #composite_node.name = 'Composite'
    #bpy.context.scene.node_tree.links.new(deblur.outputs["Image"], composite_node.inputs['Image'])

    colorOut = self.makeFileOutput(ExportLayer.Color)
    bpy.context.scene.node_tree.links.new(deblur.outputs["Image"], colorOut.inputs['Image'])
    #bpy.context.scene.node_tree.links.new(deblur.outputs["Alpha"], colorOut.inputs['Alpha'])

    depthnormal = self.makeFileOutput(ExportLayer.DepthNormal)
    bpy.context.scene.node_tree.links.new(render_layers.outputs['Normal'], combine_depth_normal.inputs['Normal'])
    bpy.context.scene.node_tree.links.new(render_layers.outputs['Depth'], combine_depth_normal.inputs['Depth'])
    bpy.context.scene.node_tree.links.new(combine_depth_normal.outputs['Image'], depthnormal.inputs['Image'])


  def makeFileOutput(self, layer: ExportLayer):
    # https://docs.blender.org/api/current/bpy.types.ImageFormatSettings.html
    output = bpy.context.scene.node_tree.nodes.new(type='CompositorNodeOutputFile')
    #output.name = layer
    output.label = layer
    output.base_path = ""
    fmt = self._data._exportFormat
    if fmt == ExportFormat.PNG:
      output.format.file_format = 'PNG'
      output.format.color_depth = str(self._data._exportBitDepth)
      output.format.color_mode = 'RGBA'
      output.format.compression = 100
      # output.format.use_zbuffer = True #getting weird depth settings
    elif fmt == ExportFormat.EXR:
      output.format.file_format = 'EXR'  # 'OPEN_EXR'  # HDR
      output.format.color_depth = str(self._data._exportBitDepth)  # '8'  # '32'
      output.format.color_mode = 'RGBA'
      output.format.exr_codec = 'ZIP'
      output.format.compression = 100
    else:
      throw()
    self._data._fileOutputs[layer] = FileOutput(output, fmt, layer)
    return output

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

    def renderob(frame, actioninf):
      if self._data._customCamera == False:
        self.renderAngles(actioninf, inf._name, frame)
      else:
        self.renderAction(actioninf, inf._name, 0, 0, 0)

    msg("exporting: " + str(inf._min_start) + "-" + str(inf._max_end))
    self.animateObjectActions(inf, renderob)

    for actname in inf._actions:
      if actname != Bake26.c_sprite_name:
        self.exportPreview(inf._name, actname)

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

  def exportPreview(self, obname, actname):
    path = self.actionOutputPath(obname, actname)
    assert(os.path.exists(path))
    ext = self._data._fileOutputs[ExportLayer.Color].get_extension()

    fcount = len(glob.glob1(path, "*"+ext))
    if fcount > 1:
      if self._doVid:
        self.exportMp4(path, fcount, ext)
      if self._doGif:
        self.exportGif(path, fcount, ext)
  def exportGif(self, path, fcount, ext):
    msg("doing gif, fcount="+str(fcount))
    csframe = (100.0 / (float(bpy.context.scene.render.fps) / float(self._data._sampleRate)))
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

    # this is very important for blender's relpaths
    dn = os.path.dirname(bpy.data.filepath)
    if not os.getcwd() == os.path.dirname(bpy.data.filepath):
      os.chdir(dn)

    for outname in self._data._fileOutputs:
      out = self._data._fileOutputs[outname]
      abs_path = fpath + out.get_extension()
      rel_path = bpy.path.relpath(abs_path)
      out.base_path = ''
      out._node.file_slots[0].path = rel_path  # must be in blender cwd
      actioninf._texs[out._layer].append(abs_path)

    bpy.ops.render.render(write_still=False)  # use only compositor outputs
    self._data._dbg_renderCount += 1

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

  def actionOutputPath(self, obname, actname):
    path = self.objectOutputPath(obname, False, 0)
    path = os.path.join(path, os.path.normpath(actname))
    return path

  def actionFilePath(self, obname, actname, frame, azumith, zenith):
    # actionFileName spriteFileName
    sep = Bake26.spritePathSeparator()
    # con obj act az   zen  fr  frf ext
    # ID.guy.run.0025.0008.0034.200.Color.png

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
    Bake26.gatherTextures(outpath, texs, True)  # skip /output because we put the mt's in there
    texs_cpy = dict(texs)
    pts = ImagePacker.packImages(texs)

    for pt in pts:
      msg("writing " + " id=" + str(pt._id) + " count=" + str(pt._regioncount) + " size=" + str(pt._size))
      pt.compose(outpath)
    self.writeMetadata(outpath, texs_cpy, pts)

  def writeMetadata(self, outpath, texs, pts):
    '''
    ptex_count [int32]
    .. foreach new b2_tex
      ptex_id [int32]
      num_texs [int32]
      .. for num texs
        tex name [string]
    sprite_count [int32]
      .. foreach new b2_obj 
        id [int32]
        name [string]
        action_count [int32]
        ..foreach new b2_action
          id [int32]
          name [string]
          frame_count [int32]
          ..foreach new b2_frame
            tex_id [int32] 
            sequence_id [float32]
            x [int32]
            y [int32]
            w [int32]
            h [int32]
    '''

    obj_dict = {}

    # build objects
    for region_fname in texs:
      region = texs[region_fname]
      assert(region._node != None)  # call this AFTER  build the nodetree
      Bake26.buildB2Obj(region, region_fname, obj_dict)

    # write file
    mdpath = os.path.join(outpath, "b2_meta.bin")
    msg("metadata path="+mdpath)
    with open(mdpath, 'wb') as file:
      bf = BinaryFile(file)

      msg("byte thing = " + str(int(ord('B'))))
      bf.writeByte(int(ord('B'.encode('utf-8'))))
      bf.writeByte(int(ord('2'.encode('utf-8'))))
      bf.writeByte(int(ord('M'.encode('utf-8'))))
      bf.writeByte(int(ord('D'.encode('utf-8'))))
      # texs
      bf.writeInt32(len(pts))
      for i in range(0, len(pts)):
        pts[i].serialize(bf)

      # objs
      bf.writeInt32(len(obj_dict))
      for obname in obj_dict:
        ob = obj_dict[obname]
        ob.serialize(bf)

  def buildB2Obj(region: MtRegion, region_fname: str, obj_dict: dict):
    sep = Bake26.spritePathSeparator()
    grp = '([^'+sep+']+)'
    # parse name
    rxstr = Bake26.c_OBNM
    rxstr += sep + grp  # ob 0
    rxstr += sep + grp  # act 1
    rxstr += sep + grp  # a 2
    rxstr += sep + grp  # z 3
    rxstr += sep + grp  # ifr 4
    rxstr += sep + grp  # ffr 5
    rxstr += sep + '(.+)'  # ext 6
    # msg("fname="+region_fname)
    # msg("rxstr="+rxstr)
    fname = os.path.basename(region_fname)
    rx = re.search(rxstr, fname)

    if (not rx) or (len(rx.groups()) != 7):
      raise Exception("Invalid file format group count ="+str(len(rx.groups())))
    # msg("rx="+str(rx.groups()))

    obname = rx.groups()[0]
    actname = rx.groups()[1]
    ifr = rx.groups()[4]
    ffr = rx.groups()[5]

    if not obname in obj_dict:
      obj = b2_obj()
      obj._id = b2_obj._ob_idgen
      b2_obj._ob_idgen += 1
      obj._name = obname
      obj_dict[obname] = obj
    obj = obj_dict[obname]
    if not actname in obj._actions:
      act = b2_action()
      act._id = len(obj._actions)+1
      act._name = actname
      obj._actions[actname] = act
    act = obj._actions[actname]
    fr = b2_frame()
    fr._texid = region._texid  # id of megatex
    fr._x = region._node._rect.left()  # id of megatex
    fr._y = region._node._rect.top()  # id of megatex
    fr._w = region._node._rect.width()  # id of megatex
    fr._h = region._node._rect.height()  # id of megatex
    seq_str = ifr+"."+ffr
    assert(not seq_str in act._frames)
    seq = float(seq_str)  # "0004.200"
    msg("seq="+str(seq))
    fr._seqid = seq
    act._frames[seq_str] = fr

  def gatherTextures(fpath, texs: dict, skipdir):
    fpath = os.path.abspath(fpath)
    fds = os.listdir(fpath)  # get all files' and folders' names in the current directory
    color_extension = FileOutput.extension(ExportFormat.PNG, ExportLayer.Color)

    for item in fds:  # loop through all the files and folders

      color_path = os.path.join(fpath, item)

      if os.path.isfile(color_path):
        if skipdir == False:

          layer_ext = FileOutput.parse_extension(item)

          if layer_ext == color_extension:
            w, h = Utils.get_image_size(color_path)
            assert(w > 0)
            assert(h > 0)
            # now set norms and shit
            texs[color_path] = MtRegion()
            texs[color_path]._pathColor = color_path
            texs[color_path]._width = int(w)
            texs[color_path]._height = int(h)
            texs[color_path]._texid = -1
            texs[color_path]._node = None
            # map region to sprite and create sprite if none

            norm_ext = FileOutput.extension(ExportFormat.PNG, ExportLayer.DepthNormal)
            norm_path = color_path.replace(layer_ext, norm_ext)

            texs[color_path]._layers = [color_path, norm_path]


      elif os.path.isdir(color_path):
        if item != '.':
          Bake26.gatherTextures(color_path, texs, False)


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
