layout(points) in;
layout(triangle_strip, max_vertices=4) out;

in uint _objIDVS[];

out vec3 _vertGS;
out vec2 _texGS;
flat out uint _mtexidGS;
flat out uint _objIDGS;
flat out float _objScaleZGS;

void setGS() {
  _objIDGS = _objIDVS[0];
  _mtexidGS = _ufGpuObj[_objIDVS[0]]._mtexid;
  mat4 tm = _ufGpuObj[_objIDVS[0]]._mat;
  _objScaleZGS = length(vec3(tm[3][1], tm[3][2], tm[3][3]));
}
void main() {

  /*
                          z,w
      2------------ ----3
      |               / |
      |           /     |
      |        c        |
      |     /           |
      |  /              |
      0 ----------------1
  x,y                   
  */
  mat4 viewproj = _ufGpuViewData._proj * _ufGpuViewData._view;
  
  mat4 wmat = _ufGpuObj[_objIDVS[0]]._mat;
  vec4 tex = _ufGpuObj[_objIDVS[0]]._tex;

  vec3 center = vec3(0,0,0); 
  vec3 right  = vec3(1,0,0);
  vec3 up     = vec3(0,1,0);

  setGS();
  _texGS          = tex.xy;
  _vertGS         = vec3(wmat * vec4(vec3(center - right - up), 1));
  gl_Position     = viewproj * vec4(_vertGS, 1);
  EmitVertex();

  setGS();
  _texGS          = tex.zy;
  _vertGS         =  vec3(wmat * vec4(vec3(center + right - up), 1));
  gl_Position     = viewproj * vec4(_vertGS, 1);
  EmitVertex();

  setGS();
  _texGS          = tex.xw;
  _vertGS         =  vec3(wmat * vec4(vec3(center - right + up), 1));
  gl_Position     = viewproj * vec4(_vertGS, 1);
  EmitVertex();

  setGS();
  _texGS          = tex.zw;
  _vertGS         =  vec3(wmat * vec4(vec3(center + right + up), 1));
  gl_Position     = viewproj * vec4(_vertGS, 1);
  EmitVertex();
  
  EndPrimitive();
}

