

layout(location = 0) in vec3 _v3;
layout(location = 1) in uint _id;
layout(location = 2) in vec2 _x2;
layout(location = 3) in vec4 _c4;

//uniform mat4 _projMatrix;
//uniform mat4 _viewMatrix;
out vec2 _tcoord;
out vec4 _color;
void main() {
  _tcoord = _x2;
  _color = _c4;
  vec4 vert = _ufGpuCamera._proj* _ufGpuCamera._view*  vec4(_v3, 1);
  gl_Position = vert;
}