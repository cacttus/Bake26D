#pragma once
#ifndef __GPU_STRUCTS_H__
#define __GPU_STRUCTS_H__

#include "./GlobalDefines.h"

namespace B26D {

SHADER_SHARED

GPU_STRUCT GpuViewData {
  mat4 _proj;
  mat4 _view;

  vec3 _viewPos;
  float _windowWidth;

  vec3 _viewDir;
  float _windowHeight;

  uint _lightCount;
  float dummy0;
  float dummy1;
  float dummy2;

//   vec4 _vWindowViewport;
// 
//   float _fRenderWidth;
//   float _fRenderHeight;
//   float _fZNear;
//   float _fZFar;
// 
//   float _widthNear;
//   float _heightNear;
//   float _widthFar;
//   float _heightFar;
};
GPU_STRUCT GpuObj {
  mat4 _mat;
  
  vec4 _tex;  // x,y,w,h
  
  int _mtexid;
  int _dummy0;
  int _dummy1;
  int _dummy2;
};
GPU_STRUCT GpuLight {
  vec3 _color;
  float _power;

  vec3 _pos;
  float _radius;

  vec3 _dir;
  float _pad0;
};
GPU_STRUCT GpuWorld {
  float _zrange;
  int _mtex_layers;
  int _mtex_color;
  int _mtex_depthnormal;
  int _mtex_w;
  int _mtex_h;

  vec4 _ambient;
};


SHADER_SHARED

}  // namespace B26D

#endif