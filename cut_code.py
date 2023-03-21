

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
  # Utils.M_2PI / float(self._rotation_slices)
  #  Matrix.Rotation(self._initial_azumith, 4, 'Z')
  # Matrix.Rotation(self._initial_zenith, 4, 'Z')
  # m = Matrix.Identity(4)
  # m[0].xyzw = 1, 0, 0, 0
  # m[1].xyzw = 0, 1, 0, 0
  # m[2].xyzw = 0, 0, 1, 0
  # m[3].xyzw = 0, 0, 0, 1
  # (sinθcosϕ,sinθsinϕ,cosθ)

  # what is this for?
  # bpy.ops.object.visual_transform_apply()
  # for c in self._dummyCamera.constraints:
  #  self._dummyCamera.constraints.remove(c)
  # self._dummyCamera.data.ortho_scale = self._fDist

  # _oldObjMat = None
  # _rotMat = Matrix.Identity(4)
  # _azumith_slice = 0
  # _zenith_slice = 0
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

#   def exportAction(self, ob, modelOut, actName):
#     # https://docs.blender.org/api/blender_python_api_2_62_release/bpy.types.AnimData.html
#     if ob.animation_data == None:
#       return
#
#     msg(ob.name + " has animation data " +
#         str(len(ob.animation_data.nla_tracks)) + " NLA Tracks")
#     sys.stdout.flush()
#     for nla in ob.animation_data.nla_tracks:
#       nla.select = True
#       msg("# NLA STrips: " + str(len(nla.strips)))
#       for strip in nla.strips:
#         curAction = strip.action
#         if curAction.name == actName:
#           msg("#Action Found : " + curAction.name)
#           # keyrames
#           keyframes = []
#
#           iMinKeyf = self.getMinKeyframeForAction(curAction)
#           iMaxKeyf = self.getMaxKeyframeForAction(curAction)
#
#           # dump keyframes to file.
#           if iMinKeyf < iMaxKeyf == 0:
#             msg("Animation had no keyframes.")
#           else:
#             self.renderKeyframeBlockForSelectedObject(
#               ob, iMinKeyf, iMaxKeyf, curAction, modelOut)
#
#   def renderKeyframeBlockForSelectedObject(self, ob, iMinKey, iMaxKey, curAction, modelOut):
#     # Create the directory for the action
#     actionOut = os.path.join(modelOut, os.path.normpath(curAction.name))
#     if not os.path.exists(actionOut):
#       os.makedirs(actionOut)
#
#     msg("exporting " + curAction.name + " to " + str(actionOut))
#
#     # if ob.type == 'ARMATURE':
#     self.renderKeyframeBlock(ob, iMinKey, iMaxKey, curAction, actionOut)
#     # elif ob.type == 'MESH':

#   def renderKeyframeBlock(self, ob, iMinKey, iMaxKey, curAction, actionOut):
#     ob.animation_data.action = curAction
#     iGrainKey = 0
#
#     msg("Writing keyframe block for ", str(iMinKey), " to ", str(iMaxKey))
#     sys.stdout.flush()
#     for iKeyFrame in range(iMinKey, iMaxKey + 1):
#       # this little block gets the final keyframe
#       iGrainMax = self._iSubframes
#       if iKeyFrame == iMaxKey:
#         iGrainMax = 1
#       for iGrain in range(0, iGrainMax):
#         fGrain = float(iGrain) / float(iGrainMax)
#         bpy.context.scene.frame_set(iKeyFrame, fGrain)
#         # context.scene.frame_set(scene.frame_end)
#         imgOut = os.path.join(actionOut, self.imgFrame(iGrainKey))
#         bpy.context.scene.render.filepath = imgOut
#         bpy.ops.render.render(write_still=True)
#
#         iGrainKey += 1


    # may be required

    #norm = bpy.context.scene.node_tree.nodes.new(type='CompositorNodeNormalize')
    #bpy.context.scene.node_tree.links.new(layers.outputs["Position"], norm.inputs[0])
    #print("vl="+str([x for x in bpy.context.scene.view_layers]))

    #bpy.context.scene.node_tree.links.new(norm.outputs[0], fileout.inputs[0])

    #print("fileout.inputs="+str([x for x in fileout.inputs]))
    #print(str([x for x in norm.inputs]))
    #print("normout="+str([x for x in norm.outputs]))

    # https://github.com/zhenpeiyang/HM3D-ABO/blob/master/my_blender_abo.py

    #fileout.file_slots[0].use_node_format = True
    #fileout.format.color_depth = args.color_depth

    #output = bpy.context.scene.node_tree.nodes.new(type='CompositorNodeOutputFile')

    # for out in layers.outputs:
    #  print("out=" + str(out))
    #       assert(type(node) is bpy.types.NodeSocketColor)
    #img = self.get_shader_node_image_input(node)
    # self.writeVec4(node.default_value)
    # if img != None:
    #   self.writeNodeID(self._image_ids[img])
    # else:
    #   self.writeNodeID(0)

    # print("layesr="+str(layers))

    # for node in bpy.context.scene.node_tree.nodes:
    #  print("node="+str(node))
        # open the library file as the scene, then we will import objects from other files
        #mainfile='/home/mario/git/Bake26D/main.blend'
    #self.blendFileExistsOrThrow(mainfile)
    #self.blendFileExistsOrThrow(blendFile)
       
    #append mesh & armatures from blend

    #individual meshes - it makes more sense to treat each file as one thing
    # so importing "scene" and disabling other scene. 
# 
#     with bpy.data.libraries.load(blendFile) as (data_from, data_to):
#       data_to.objects = data_from.objects
#     for ob in data_to.objects:
#       if (not ob.type == 'ARMATURE') and (not ob.type == 'MESH'):
#         msg("removing imported obj " + ob.name)
#         bpy.data.objects.remove(ob)
#         
#     self._objects = data_to.objects
    
    #if no actions export just frame0 (for like main sprites walls, etc)
    
    # this appends correctly but names get changed so how would we know which ones so we can unload
    #with bpy.data.libraries.load(blendFile) as (data_from, data_to):
    #  for name in data_from.objects:
    #      files.append({'name': name})
    #bpy.ops.wm.append(directory=blendFile+"/Object/", files=files)
    #msg("obs="+str([ob for ob in bpy.data.objects]))
    
    #TODO: you can remove with BlendData.objects.remove()
    # batch
    
    #    bpy.ops.wm.open_mainfile(filepath=blendFile)
    #    msg(bpy.data.filepath)
    #    msg(bpy.context.blend_data.filepath)
    #  else:

    #bpy.data.libraries.load("./Bake26DLibrary.blend")
    #data_to.materials = data_from.materials 
    # else:
      # backup
      # bpy.ops.wm.save_as_mainfile(filepath=bpy.data.filepath)
    
#     if not self._is_cmd:
#       bpy.ops.object.select_all(action='DESELECT')
#       if self._customCamera == False:
#         self._dummyCamera.select_set(True)
#       if self._customEmpty == False:
#         self._dummyEmpty.select_set(True)
# 
#       dbg_dont_delete = False
#       if not dbg_dont_delete:
#         bpy.ops.object.delete()
#       else:
#         dbg("debug: didnt delete dummy objects")
#       bpy.context.scene.camera = self._oldCamera
#       bpy.ops.object.select_all(action='DESELECT')
#       for ob in self._oldSelection:
#         ob.select_set(True)
#       bpy.context.view_layer.objects.active = self._oldActive  # None

    # restore render settings
    #   bpy.context.scene.cycles.aa_samples                 =  self._old_aa_samples
    # bpy.context.scene.render.film_transparent = self._old_film_transparent
    # bpy.context.scene.render.resolution_x = self._old_resolution_x
    # bpy.context.scene.render.resolution_y = self._old_resolution_y
    # bpy.context.scene.render.resolution_percentage = self._old_resolution_percentage
    # bpy.context.scene.render.filepath = self._old_render_filepath
    # # bpy.context.scene.render.engine = self._old_renderer
    # bpy.context.scene.render.image_settings.color_mode = self._old_color_mode
#   def selectArmature(self):
#     arm = None
#     # use selected armature, or find the first armature, report if more than one
#     arms = [ob for ob in self._oldSelection if ob != None and ob.type == 'ARMATURE']
#     # msg("arms=" + str(arms))
#     if len(arms) == 1:
#       arm = arms[0]
#     elif len(arms) > 0:
#       msg("More than 1 armature was selected")
#     else:
#       arms = [ob for ob in bpy.context.visible_objects if ob != None and ob.type == 'ARMATURE']
#       if len(arms) == 0:
#         throw("No armatures found. Can only do this with armature right now (later we can do other stuff)")
#       elif len(arms) > 0:
#         arm = arms[0]
#         if len(arms) > 1:
#           msg("More than 1 armature found, using first armature " + arm.name)
# 
#     return arm

  #compositor output slots
  #_colorOutput = None

      # for node in bpy.context.scene.node_tree.nodes:
    #  print("node="+str(node))
      # open the library file as the scene, then we will import objects from other files
      # mainfile='/home/mario/git/Bake26D/main.blend'
    # self.blendFileExistsOrThrow(mainfile)
    # self.blendFileExistsOrThrow(blendFile)

    # append mesh & armatures from blend

    # individual meshes - it makes more sense to treat each file as one thing
    # so importing "scene" and disabling other scene.
    #
    # https://docs.blender.org/api/current/bpy.types.Library.html


   # for ob in data_to.objects:
   #   if (not ob.type == 'ARMATURE') and (not ob.type == 'MESH'):
   #     msg("removing imported obj " + ob.name)
   #     bpy.data.objects.remove(ob)
    #
    #     self._objects = data_to.objects

    # if no actions export just frame0 (for like main sprites walls, etc)

    #bpy.ops.wm.append(directory=mainfilepath+"/Object/", files=files)

    # this appends correctly but names get changed so how would we know which ones so we can unload
    # with bpy.data.libraries.load(blendFile) as (data_from, data_to):
    #  for name in data_from.objects:
    #      files.append({'name': name})
    #bpy.ops.wm.append(directory=blendFile+"/Object/", files=files)
    #msg("obs="+str([ob for ob in bpy.data.objects]))

    # Utils.assertBlendFileExists(mainfile)
    # Utils.assertBlendFileExists(blendFile)
    # with bpy.data.libraries.load(self._libFile) as (data_from, data_to):
    #  #xx = print(builtins.dir(data_from))
    #  data_to.node_groups = data_from.node_groups
    #  #for ng in data_from.node_groups:
    #    #print(str(ng))
    #  #data_to.objects = data_from.objects
    # for ng in data_to.node_groups:
    #  print(str(ng.name))
    #  if ng.name == 'EdgeDeblur':
    #    bpy.data.node_groups.nodes.append(ng)
        # TODO: we should select the NLA track when we do this.
    #min_start = 99999
    #max_end = -99999
    #ob_actions = []

    # for nla in ob.animation_data.nla_tracks:
    #   if not "[Action Stash]" in nla.name:
    #     if len(nla.strips) > 0:
    #       act = nla.strips[0].action
    #       ob_actions.append(act)
    #       min_start = min(act.frame_range[0], min_start)
    #       max_end = max(act.frame_range[1], max_end)

#   def sweepBounds(self, ob, min_start, max_end, ob_actions):
#     # get sweep bb of all anims
#     bdic = self.sweepBoundsAll(ob, min_start, max_end, ob_actions)
# 
#     sweep = BoundBox.zero()
#     sweep.genReset()
#     for k in bdic:
#       sweep.genExpandByBox(bdic[k])
#     return sweep
# 
#   def sweepBoundsAll(self, ob, min_start, max_end, ob_actions):
#     # return dictionary of sweep bounds for all action [name->BoundBox]
#     bbdict = {}
#     for act in ob_actions:
#       bbdict[act.name] = BoundBox.zero()
#       bbdict[act.name].genReset()
# 
#     frame = min_start
#     while frame <= max_end:
#       Utils.set_frame(frame)
#       frame += self._data._sampleRate
#       for act in ob_actions:
#         ob.animation_data.action = act
#         bpy.context.view_layer.update()
#         bbdict[act.name].genExpandByBox(BoundBox.fromBlender(ob.bound_box))
# 
#     return bbdict

  # def animateFrameRange(self, min_start, max_end, lamb):
  #   frame = min_start
  #   while frame <= max_end:
  #     Utils.set_frame(frame)
  #     lamb(frame)
  #     frame += self._data._sampleRate
    # 
    # frame = inf._min_start
    # while frame <= inf._max_end:
    #   Utils.set_frame(frame)
    #   frame += self._data._sampleRate
    #   for act in inf._actions:
    #     ob.animation_data.action = act._action
    #     bpy.context.view_layer.update()
    #     bb = BoundBox.fromBlender(ob.bound_box)
    #     inf._actions[act.name]._sweepBox.genExpandByBox(bb)
    #     inf._sweepBox.genExpandByBox(bb)
