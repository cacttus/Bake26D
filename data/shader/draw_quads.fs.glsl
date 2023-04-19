
uniform sampler2D _texture;
in vec2 _tcoord;
in vec4 _color;
out vec4 _output;

void main() {
  _output = texture(_texture, _tcoord) * _color;
}