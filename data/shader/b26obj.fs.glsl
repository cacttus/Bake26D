
//uniform sampler2D _texture;

uniform sampler2DArray  _textures;

in vec3 _vertGS;
in vec2 _texGS;
flat in uint _texidGS;

out vec4 _output;

void main() {
  _output = texture(_textures, vec3(_texGS, _texidGS));
}