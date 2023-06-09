# Bake26 2.6D Gamedev Toolchain Script

## Description
Blender script to output layered "2.6D" sprites from object animations, for games

* Renders a series of color and depth/normal layers from a given object in a .blend file
* Can render any number of angles + animations
* (Note: Not sure what this kind of sprite is called so calling it 2.6D for now)

## Install
### Manual install
* Python
  * Install PIL for *BLENDER PYTHON*:
    * cd PATH_TO_BLENDER_PYTHON && ./python* -m ensurepip --default-pip && ./python* -m pip install Pillow
    * Ex:
      * cd ~/Desktop/apps/blender*/3.*/python/bin && ./python* -m ensurepip --default-pip && ./python* -m pip install Pillow
* C++ Test App
  * Debian Dependencies
    * sudo apt install build-essential libglew-dev libglfw3-dev ninja-build libglm-dev libncurses-dev -y
* TODO: finish install.sh

## Usage
### Blender script (Bake26.py)
1. Command:
  * [path-to-blender-exe] -b --log-level 0 -P [path-to-script]/Bake26.py  -- -i [single-file-or-directory-of-blend-files] -o [output-directory] -l [path-to-library]/_library.blend -p
    * -o output path
    * -l library file path
    * -p pack textures in -o (-p OR -P)
    * -P only pack textures in -o, dont export (-p OR -P)
    * -v generate video preview of animation
    * -g generate gif preview of animation
    * -gj export metadata as .json 
    * -gb export metadata as .bin (default)

* Note: Script *requires* the _library.blend file (-l)
* Note: Metadata (B2MT.BIN) placed in -o (output dir)
* Note: Metadata texture paths are relative to B2MT.BIN file
* Note: Script *must* run from terminal/cmd (blender headless).
* Note: Object actions must be in the NLA editor.
* Note: VSCode python format settings (for autopep) are in ~/.config/.pycodestyle, on my PC at least
* Note: Metadata exports sprite origin = top left corner

### C++ Test App
* Dependencies (Debian = sudo apt install)
  * sudo apt install build-essential libglew-dev libglfw3-dev ninja-build libglm-dev libncurses-dev -y

* Note: set global compiler
  * sudo update-alternatives --config c++

* Controls:
  * WSAD + Mouse - Move camera
  * LRUD - Move player

### TODO
* testapp: idle animation (test)
* testapp: test varied animation frame sample rates 
* testapp: object quad w/h must conform to texture size 
* testapp: bezier path for bk quads animation
* script: remove _library.blend and put in code
* script: remove PIL - use blender's save image (16bit png)
* script: move bake26 constants into cmd line vars
* script: 16bit png export note:  Image::from_file hard code 4Bpp







