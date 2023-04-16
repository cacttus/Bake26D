#pragma once
#ifndef __GPU_STRUCTS_H__
#define __GPU_STRUCTS_H__

#include "./GlobalDefines.h"

namespace B26D {

SHADER_SHARED

GPU_STRUCT GpuCamera {
  mat4 _proj;
  mat4 _view;

  vec3 _viewPos;
  float _windowWidth;

  vec3 _viewDir;
  float _windowHeight;

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
  int _texid;
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
  uint _lightCount;
  float _zrange;
  float dummy0;
  float dummy1;
  vec4 _ambient;
};


SHADER_SHARED

}  // namespace B26D

#endif