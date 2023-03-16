# Blender MRT sprite exporter (2.6D)
#
# blender:
#   exec(compile(open("/home/mario/git/Bake26D/Bake26.py").read(), "/home/mario/git/Bake26D/Bake26.py", 'exec'))
#
# terminal:
#  ~/Desktop/apps/Blender*/blender -b -P ~/git/Bake26D/Bake26.py  ~/git/Bake26D/Bake26D_Test1.blend -- -o path
#
# autopep8 tabs: ./usr/lib/python3/dist-packages/autopep8.py
#
# Notes: Running from terminal is faster
#    Make sure actions are in the NLA editor
#    Output file name is same as file name

import bpy
from bpy import context
from mathutils import Vector, Matrix, Euler
import os
import math
import shutil
import time
import pprint
import sys
import traceback
import argparse
import zlib
import bmesh
import struct
import multiprocessing
import builtins as __builtins__


class Bake26:
  _fVersion = 0.01
  _outPath = ""  # This is actually the "temp" path.

  _azumith_slices = 0
  _zenith_slices = 0
  _initial_azumith = 0
  _initial_zenith = 0
  _initial_radius = 3.5
  _render_width = 50
  _render_height = 50
  _iAASamples = 0
  _iSubframes = 0
  _bFitModel = False

  _objectName = ""
  _actionName = ""

  _oldCamera = None
  _oldSelection = []
  _oldActive = None
  _dummyCamera = None
  _dummyEmpty = None
  _customCamera = None
  _customEmpty = None
  
  _old_aa_samples = None
  _old_film_transparent = None
  _old_color_mode = None
  _old_resolution_x = None
  _old_resolution_y = None
  _old_resolution_percentage = None
  _old_render_filepath = None
  _old_renderer = None
  
  def __init__(self, blendFile, outputDir, settingsPath):
    if not Utils.blendFileIsOpen():
      if not blendFile:
        throw("No blend file specified.")
      elif os.path.exists(blendFile):
        bpy.ops.wm.open_mainfile(filepath=blendFile)
        msg(bpy.data.filepath)
        msg(bpy.context.blend_data.filepath)
      else:
        throw("blend file path '" + blendFile + "' not found.")
    # else:
      # backup
      # bpy.ops.wm.save_as_mainfile(filepath=bpy.data.filepath)

    # if running within blender set default parameters TOOD: set them from settings file
    if not outputDir:
      assert(bpy.data.filepath)
      self._outPath = os.path.join(os.path.dirname(bpy.data.filepath), os.path.normpath("output"))
    else:
      self._outPath = outputDir
    msg("output path = " + self._outPath)

    self._objectName = 'Monk'
    self._actionName = 'MonkWalk'

    if not self._objectName:
      if bpy.context.view_layer.objects.active != None:
        self._objectName = bpy.context.view_layer.objects.active.Name
    if not self._objectName:
      throw('no object name specified and no active object selected')
    if not self._actionName:
      throw('no action name specified and no active object selected')

    self.export()

  def getRootName(self):
    name = ""
    if bpy.data.filepath:
      name = os.path.splitext(os.path.basename(bpy.data.filepath))[0]
    return name

  def export(self):
    self.setup()
    self.setRenderSettings()
    try:
      arm = self.selectArmature()
      self.render(arm)
    except Exception as e:
      printExcept(e)
    self.cleanup()

  
  def setup(self):
    self._oldSelection = bpy.context.selected_objects
    self._oldActive = bpy.context.view_layer.objects.active

    bpy.ops.object.select_all(action='DESELECT')
    bpy.context.view_layer.objects.active = None

    # create and swap camera
    camname ='__b26camera'
    exists = [ob for ob in bpy.context.view_layer.objects if ob.name == camname]
    if len(exists) > 0:
      self._dummyCamera = exists[0]
      self._customCamera = True
      msg("existing camera '"+camname+"' found")
    else:      
      bpy.ops.object.camera_add()
      self._dummyCamera = bpy.context.view_layer.objects.active
      self._dummyCamera.name = camname
      self._dummyCamera.data.name = camname
      self._dummyCamera.data.type = 'PERSP'  # 'ORTHO'
      self._dummyCamera.location.x = 0
      self._dummyCamera.location.y = -10
      self._dummyCamera.location.z = 0
      self._oldCamera = bpy.context.scene.camera
      self._customCamera = False
    
    bpy.context.scene.camera = self._dummyCamera

    # add empty target 
    empname ='__b26empty'
    exists = [ob for ob in bpy.context.view_layer.objects if ob.name == empname]
    if len(exists) > 0:
      self._dummyEmpty = exists[0]
      self._customEmpty = True
      msg("existing empty '"+empname+"' found")
    else:
      bpy.ops.object.empty_add()
      self._dummyEmpty = bpy.context.view_layer.objects.active
      self._dummyEmpty.name = empname
      #TODO: put empty at center of object

    #add constraint - todo - check if constraint exists
    if self._customCamera == False:
      constraint = self._dummyCamera.constraints.new('TRACK_TO')
      constraint.track_axis = 'TRACK_NEGATIVE_Z'
      constraint.up_axis = 'UP_Y'
      constraint.target = self._dummyEmpty
      self._customEmpty = False

    # save render state
    self._old_film_transparent = bpy.context.scene.render.film_transparent
    self._old_color_mode = bpy.context.scene.render.image_settings.color_mode
    self._old_resolution_x = bpy.context.scene.render.resolution_x
    self._old_resolution_y = bpy.context.scene.render.resolution_y
    self._old_resolution_percentage = bpy.context.scene.render.resolution_percentage
    self._old_render_filepath = bpy.context.scene.render.filepath
    self._old_renderer = bpy.context.scene.render.engine 
  
  def cleanup(self):
    bpy.ops.object.select_all(action='DESELECT')
    if self._customCamera == False:
      self._dummyCamera.select_set(True)
    if self._customEmpty == False:
      self._dummyEmpty.select_set(True)
  # ##
  #  bpy.ops.object.delete()
    ##
    bpy.context.scene.camera = self._oldCamera
    bpy.ops.object.select_all(action='DESELECT')
    for ob in self._oldSelection:
      ob.select_set(True)
    bpy.context.view_layer.objects.active = self._oldActive  # None

    # restore render settings
    #   bpy.context.scene.cycles.aa_samples                 =  self._old_aa_samples
    bpy.context.scene.render.film_transparent = self._old_film_transparent
    bpy.context.scene.render.resolution_x = self._old_resolution_x
    bpy.context.scene.render.resolution_y = self._old_resolution_y
    bpy.context.scene.render.resolution_percentage = self._old_resolution_percentage
    bpy.context.scene.render.filepath = self._old_render_filepath
    #bpy.context.scene.render.engine = self._old_renderer 
    bpy.context.scene.render.image_settings.color_mode = self._old_color_mode

  def setRenderSettings(self):
    #https://docs.blender.org/api/current/bpy.types.RenderSettings.html
    bpy.context.scene.render.film_transparent = True
    bpy.context.scene.render.resolution_x = self._render_width  # 512 #64
    bpy.context.scene.render.resolution_y = self._render_height  # 512 #64
    bpy.context.scene.render.resolution_percentage = 100
    bpy.context.scene.render.filepath = ''
    bpy.context.scene.render.image_settings.color_mode = 'RGBA'
    bpy.context.scene.render.image_settings.file_format = 'PNG' #'EXR' png would not save the floating point depth well, using EXR would fix this
    bpy.context.scene.render.image_settings.use_zbuffer = True #save depth .. 
    bpy.context.scene.render.filter_size = 1 # cause blurs edges
    bpy.context.scene.render.use_motion_blur = False
    
    #bpy.context.scene.render.pixel_aspect_x = 1 # anamorphic
    #bpy.context.scene.render.pixel_aspect_y = 1 # anamorphic
    #bpy.context.scene.render.engine = 'BLENDER_EEVEE' #'BLENDER_EEVEE' 'BLENDER_WORKBENCH' 'CYCLES'
    
    #https://docs.blender.org/api/current/bpy.types.SceneEEVEE.html
    #bpy.context.scene.eevee.motion_blur_max=0
    #
    ##https://docs.blender.org/api/2.80/bpy.types.CyclesRenderSettings.html
    #bpy.context.scene.cycles.samples = 100
    ##bpy.context.scene.cycles.diffuse_samples = 100
    #bpy.context.scene.cycles.aa_samples = self._iAASamples
    #bpy.context.scene.cycles.feature_set = 'SUPPORTED' #EXPERIMENTAL
    ##bpy.context.scene.cycles.device = 'CPU'
    #bpy.context.view_layer.cycles.denoising_store_passes = True
    
    #https://docs.blender.org/api/current/bpy.types.ImageFormatSettings.html

    msg("render engine="+str(bpy.context.scene.render.engine))
    
  def selectArmature(self):
    arm = None
    # use selected armature, or find the first armature, report if more than one
    arms = [ob for ob in self._oldSelection if ob != None and ob.type == 'ARMATURE']
    #msg("arms=" + str(arms))
    if len(arms) == 1:
      arm = arms[0]
    elif len(arms) > 0:
      msg("More than 1 armature was selected")
    else:
      arms = [ob for ob in context.visible_objects if ob != None and ob.type == 'ARMATURE']
      if len(arms) == 0:
        throw("No armatures found. Can only do this with armature right now (later we can do other stuff)")
      elif len(arms) > 0:
        arm = arms[0]
        if len(arms) > 1:
          msg("More than 1 armature found, using first armature " + arm.name)

    return arm

  #_oldObjMat = None
  #_rotMat = Matrix.Identity(4)
 # _azumith_slice = 0
  #_zenith_slice = 0
  # def updateCamera(self, cam):
  #   pmat = Matrix.Translation((0, 0, self._initial_radius))
  #   cam.matrix_world = pmat
  #   if self._azumith_slices > 0 or self._zenith_slices>0:
  #     rot_mat = Matrix.Identity(4)
  #     if self._azumith_slices > 0:
  #       slice = Utils.M_2PI / float(self._azumith_slices)
  #       rot_mat @= Matrix.Rotation(slice * float(self._azumith_slice), 4, 'Z')
  #     if self._zenith_slice > 0:
  #       slice = Utils.M_2PI / float(self._zenith_slices)
  #       rot_mat @= Matrix.Rotation(slice * float(self._zenith_slice), 4, 'X')
  #     cam.matrix_world = pmat * rot_mat

#     bpy.ops.object.mode_set(mode=self._oldMode)
#     if bpy.context.object != None:
#       self._oldMode = bpy.context.object.mode
#     else:
#       self._oldMode = None
#     bpy.ops.object.mode_set(mode='OBJECT')
# 

  def render(self, ob):
    assert(ob != None)

    if ob.hide_render == False:
      old_mat = ob.matrix_world

      if self._azumith_slices > 0:
        rot_mat = Matrix.Rotation((Utils.M_2PI / float(self._azumith_slices)), 4, 'Z')

      for zenith in range(0, self._zenith_slices+1):
        for azumith in range(0, self._azumith_slices+1):
          pmat = Matrix.Translation((0, -self._initial_radius, 0))
          rot_mat = Matrix.Identity(4)
          if azumith > 0:
            slice = Utils.M_2PI / float(self._azumith_slices)
            rot_mat @= Matrix.Rotation(slice * float(azumith), 4, 'Z')
          if zenith > 0:
            slice = Utils.M_2PI / float(self._zenith_slices)
            rot_mat @= Matrix.Rotation(slice * float(zenith), 4, 'X')
          self._dummyCamera.matrix_world = pmat @ rot_mat
          #msg("mat="+str(self._dummyCamera.matrix_world))

        self.exportTest(ob)
        #self.exportAction(ob, modelOut, self._strActName)

      ob.matrix_world = old_mat

    else:
      msg("object '" + ob.Name + "' was hidden")

  def exportTest(self, ob):
    bpy.context.view_layer.objects.active = ob
    bpy.data.objects[ob.name].select_set(True)

    path = self.spritePath(ob.name, 0, 0, 0, None)
    basepath = os.path.basename(path)
    if not os.path.exists(basepath):
      os.makedirs(basepath)

    msg("rendering " + path)
    
    bpy.context.scene.render.filepath = path
    bpy.ops.render.render(write_still=True)

  def spritePath(self, obname, frame=0, azumith=0, zenith=0, nlaTrack=None):
    #out/objname/obj_track_phi_theta_frame#
    sep = '.'
    trackname = nlaTrack.Name if nlaTrack != None else 'default'

    assert(obname)

    fname = obname + sep
    fname += trackname + sep
    fname += str(azumith) if azumith != None else '0' + sep
    fname += str(zenith) if zenith != None else '0' + sep
    fname += str(frame)

    path = os.path.abspath(os.path.normpath(self._outPath))
    path = os.path.join(path, os.path.normpath(obname))
    path = os.path.join(path, os.path.normpath(trackname))
    path = os.path.join(path, fname)

    return path

  def exportAction(self, ob, modelOut, actName):
    # https://docs.blender.org/api/blender_python_api_2_62_release/bpy.types.AnimData.html
    if ob.animation_data == None:
      return

    msg(ob.name + " has animation data " +
        str(len(ob.animation_data.nla_tracks)) + " NLA Tracks")
    sys.stdout.flush()
    for nla in ob.animation_data.nla_tracks:
      nla.select = True
      msg("# NLA STrips: " + str(len(nla.strips)))
      for strip in nla.strips:
        curAction = strip.action
        if curAction.name == actName:
          msg("#Action Found : " + curAction.name)
          # keyrames
          keyframes = []

          iMinKeyf = self.getMinKeyframeForAction(curAction)
          iMaxKeyf = self.getMaxKeyframeForAction(curAction)

          # dump keyframes to file.
          if iMinKeyf < iMaxKeyf == 0:
            msg("Animation had no keyframes.")
          else:
            self.renderKeyframeBlockForSelectedObject(
              ob, iMinKeyf, iMaxKeyf, curAction, modelOut)

  def renderKeyframeBlockForSelectedObject(self, ob, iMinKey, iMaxKey, curAction, modelOut):
    # Create the directory for the action
    actionOut = os.path.join(modelOut, os.path.normpath(curAction.name))
    if not os.path.exists(actionOut):
      os.makedirs(actionOut)

    msg("exporting " + curAction.name + " to " + str(actionOut))

    # if ob.type == 'ARMATURE':
    self.renderKeyframeBlock(ob, iMinKey, iMaxKey, curAction, actionOut)
    # elif ob.type == 'MESH':

  def renderKeyframeBlock(self, ob, iMinKey, iMaxKey, curAction, actionOut):
    ob.animation_data.action = curAction
    iGrainKey = 0

    msg("Writing keyframe block for ", str(iMinKey), " to ", str(iMaxKey))
    sys.stdout.flush()
    for iKeyFrame in range(iMinKey, iMaxKey + 1):
      # this little block gets the final keyframe
      iGrainMax = self._iSubframes
      if iKeyFrame == iMaxKey:
        iGrainMax = 1
      for iGrain in range(0, iGrainMax):
        fGrain = float(iGrain) / float(iGrainMax)
        bpy.context.scene.frame_set(iKeyFrame, fGrain)
        # context.scene.frame_set(scene.frame_end)
        imgOut = os.path.join(actionOut, self.imgFrame(iGrainKey))
        bpy.context.scene.render.filepath = imgOut
        bpy.ops.render.render(write_still=True)

        iGrainKey += 1

  def imgFrame(self, frameid):
    # Critical that we have EXACTLY 7 digits here, because the C# will parse 7 digits out of the frame 4/6/18
    return "img%07d.png" % frameid

  def getMinKeyframeForAction(self, curAction):
    iRet = 9999999
    for fcu in curAction.fcurves:
      for keyf in fcu.keyframe_points:
        x, y = keyf.co
        if x < iRet:
          iRet = x
    return int(iRet)

  def getMaxKeyframeForAction(self, curAction):
    iRet = -9999999
    for fcu in curAction.fcurves:
      for keyf in fcu.keyframe_points:
        x, y = keyf.co
        if x > iRet:
          iRet = x
    return int(iRet)


class BlenderFileInfo:
  def __init__(self):
    return

  def getFileInfo(self):
    if bpy.context.mode != 'OBJECT':
      bpy.ops.object.mode_set(mode='OBJECT')
    bpy.ops.object.select_all(action='DESELECT')
    # JSON
    strOut = "$3{\"Objects\":["
    app1 = ""
    for ob in context.visible_objects:
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

# ======================================================================================
# ======================================================================================
# ======================================================================================
# ======================================================================================
# ======================================================================================


class Utils:
  M_2PI = float(math.pi * 2)

  def active_object():
    # return bpy.context.view_layer.objects.active
    return bpy.context.view_layer.objects.active

  def blendFileIsOpen():
    return bpy.data.filepath

  def printObj(obj):
    if hasattr(obj, '__dict__'):
      for k, v in obj.__dict__.items():
        msg(str(k) + "," + str(v))
        Utils.printObj(v)
    else:
      msg(str(dir(obj)))


def throw(ex):
  raise Exception("Exception: " + ex)


def millis():
  return int(round(time.time() * 1000))


def dbg(str):
  _msg("[D] " + str)


def msg(str):
  _msg("[I] " + str)


def _msg(str):
  __builtins__.print(str)
  sys.stdout.flush()
  time.sleep(0)


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


def getArgs():
  argv = sys.argv
  if "--" not in argv:
    argv = []
  else:
    argv = argv[argv.index("--") + 1:]
  parser = argparse.ArgumentParser(description="MRT sprite baker")
  parser.add_argument("-b", dest="blendfile", type=str,
                      required=False, help=".blend file")
  parser.add_argument("-o", dest="outpath", type=str,
                      required=False, help="output path")
  parser.add_argument("-s", dest="settingspath", type=str, required=False,
                      help="Settings save directory. Default: blendfile directory")
  args = parser.parse_args(argv)

  return args


try:
  outpath = os.path.abspath("./")
  args = None
  try:
    args = getArgs()
  except Exception as e:
    printExcept(e)

  mob = Bake26(args.blendfile, args.outpath, args.settingspath)
except Exception as e:
  printExcept(e)


# you would ned to select all arm children too for this wt work
# def getOBBCenter(self, ob):
  # Compute center of geometry for all objects so we can look at that point
  # min = Vector([999,999,999])
  # max = Vector([-999,-999,-999])
  # for ob in context.visible_objects:
  #   if ob.type == 'MESH':
  #     # idk i just work here
  #     # https://blender.stackexchange.com/questions/129473/typeerror-element-wise-multiplication-not-supported-between-matrix-and-vect
  #     # Blender has since adjusted its mathutils module, replacing the asterisk * with the at symbol @
  #     #https://peps.python.org/pep-0465/

  #     bbox_corners = [ob.matrix_world @ Vector(corner) for corner in ob.bound_box]
  #     for bbc in bbox_corners:
  #       if bbc.x > max.x: max.x = bbc.x
  #       if bbc.y > max.y: max.y = bbc.y
  #       if bbc.z > max.z: max.z = bbc.z
  #       if bbc.x < min.x: min.x = bbc.x
  #       if bbc.y < min.y: min.y = bbc.y
  #       if bbc.z < min.z: min.z = bbc.z
  # avg = min + (max - min) * 0.5


    # initial position camera
    #Utils.M_2PI / float(self._rotation_slices)
  #  Matrix.Rotation(self._initial_azumith, 4, 'Z')
   # Matrix.Rotation(self._initial_zenith, 4, 'Z')
    #m = Matrix.Identity(4)
    #m[0].xyzw = 1, 0, 0, 0
    #m[1].xyzw = 0, 1, 0, 0
    #m[2].xyzw = 0, 0, 1, 0
    #m[3].xyzw = 0, 0, 0, 1
    # (sinθcosϕ,sinθsinϕ,cosθ)
    
    # what is this for?
    # bpy.ops.object.visual_transform_apply()
    # for c in self._dummyCamera.constraints:
    #  self._dummyCamera.constraints.remove(c)
    #self._dummyCamera.data.ortho_scale = self._fDist
