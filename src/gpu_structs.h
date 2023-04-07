#pragma once
#ifndef __GPU_STRUCTS_H__
#define __GPU_STRUCTS_H__

#include "./testapp_defines.h"

namespace B26D {

//SHADER_SHARED

struct GpuCamera {
  mat4 _proj;
  mat4 _view;
};
struct GpuObj {
  vec4 _tex; //x,y,w,h
  //vec4 _pos; //x,y,w,h -2d //mat is the pos
  mat4 _mat;
  int _texid;
  int _dummy0;
  int _dummy1;
  int _dummy2;
};

//SHADER_SHARED

}

#endif