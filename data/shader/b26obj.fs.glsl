
//uniform sampler2D _texture;

uniform sampler2DArray  _textures;

in vec3 _vertGS;
in vec2 _texGS;
flat in uint _mtexidGS;
flat in uint _objIDGS;
flat in float _objScaleZGS;

out vec4 _output;

float attenuate_light_distance(in vec3 in_vertex, in vec3 light_pos, in float light_power, in float light_radius)
{
  //more accurate function that uses light distance 
  //clamped to light radius.
  float dist = pow(min(length(light_pos - in_vertex),light_radius),2);
  dist = light_power / (pow(dist,1));
  return dist;
}
vec3 doBlinnPhong(in vec3 in_vertex, in vec3 in_albedo, in vec3 light_vector, in vec3 eye_vector, in vec3 in_normal, in vec3 light_color, float atten,  vec4 spec_color)
{
  //Blinn-Phong
  vec3 half_vector = (light_vector + eye_vector) / length(light_vector + eye_vector);
  float lambert = max(dot(light_vector, in_normal), 0.0);
  float spec = pow(max(dot(normalize(light_vector + eye_vector), in_normal), 0.0), spec_color.w);
  return (in_albedo * lambert + spec_color.xyz * spec) * light_color * atten;
}
vec3 lightFragmentBlinnPhong(in vec3 in_vertex, in vec3 in_albedo, in vec3 in_normal, vec4 spec_color) {
  
  vec3 eye_vector = normalize(_ufGpuViewData._viewPos - in_vertex);

  vec3 finalColor = vec3(0,0,0);

  for(int iLight = 0; iLight <  _ufGpuViewData._lightCount; iLight++) 
  {
    vec3 vLightPos = _ufGpuLight[iLight]._pos;
    vec3 vLightColor =  _ufGpuLight[iLight]._color;
    float fLightPower = _ufGpuLight[iLight]._power+100;
    float fLightRadius = _ufGpuLight[iLight]._radius;

    vec3 light_vector;
    float atten = 1;
    if(dot(_ufGpuLight[iLight]._dir, _ufGpuLight[iLight]._dir) > 0.1) // _isDir 
    {
      light_vector = -_ufGpuLight[iLight]._dir;    
    }
    else //point
    {
      light_vector = normalize(vLightPos - in_vertex);    
      atten = attenuate_light_distance(in_vertex, vLightPos, fLightPower, fLightRadius);
    }

    finalColor += doBlinnPhong(in_vertex, in_albedo, light_vector, eye_vector, in_normal, vLightColor, atten,  spec_color);
  }

  finalColor += in_albedo * _ufGpuWorld._ambient.xyz * _ufGpuWorld._ambient.w;

  return finalColor;
}

void main() {
  uint layers = 2;

  vec4 col = texture(_textures, vec3(_texGS, _mtexidGS * _ufGpuWorld._mtex_layers + _ufGpuWorld._mtex_color));
  vec4 nd = texture(_textures, vec3(_texGS,  _mtexidGS * _ufGpuWorld._mtex_layers + _ufGpuWorld._mtex_depthnormal));
  
  vec3 n = normalize(nd.xyz * 2 - 1);
  float d = nd.w * 2 - 1;

  vec3 p =  vec3(_vertGS.xy, d * _objScaleZGS);

  _output = vec4(lightFragmentBlinnPhong(p, col.rgb, n, vec4(1.0f, 1.0f, 1.0f, 100.0f)), col.a);

}

