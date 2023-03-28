#include "global.sh"

layout(location=0) in uint _id;


out vec2 _tcoord;
out vec4 _color;


void main() {
  _tcoord = _x2;
  _color = _c4;
  vec4 vert = _projMatrix * _viewMatrix*  vec4(_v3, 1);
  gl_Position = vert;
}