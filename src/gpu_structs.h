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


//SHADER_SHARED

}

#endif