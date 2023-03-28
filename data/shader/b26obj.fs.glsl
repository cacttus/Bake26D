#version 450
uniform sampler2D _texture;
in vec2 _tcoord;
in vec4 _color;
out vec4 _output;
void main() {
  _output = texture(_texture, _tcoord);
  //_output = _color//vec4(1,0,1,1)//texture(_texture, _tcoord) * _color;
  //_output.a=1.0
}