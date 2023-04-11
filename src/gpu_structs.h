#pragma once
#ifndef __GPU_STRUCTS_H__
#define __GPU_STRUCTS_H__

#include "./GlobalDefines.h"

namespace B26D {

//SHADER_SHARED

struct GpuCamera {
  mat4 _proj;
  mat4 _view;
};
struct GpuObj {
  mat4 _mat;
  vec4 _tex; //x,y,w,h
  int _texid;
  int _dummy0;
  int _dummy1; 
  int _dummy2;
};
struct GpuBObj {
  mat4 _mat;
  int _objid;
  int _actid;
  int _frameid;
  int _dummy;
};

//SHADER_SHARED

}

#endif