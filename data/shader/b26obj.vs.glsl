
//layout(location=0) in uint _id;

// out vec3 _posVS;
// flat out vec3 _normalVS;
// flat out vec4 _texVS;
flat out uint _objIDVS;

void main() {
  // 
  // mat4 wmat = _ufGpuObj[gl_VertexID]._mat;
  // vec4 xform = wmat * vec4(0,0,0,1);
  // 
   _objIDVS = gl_VertexID;
  // _posVS = vec3(xform);
  // _normalVS = normalize(vec3( wmat * vec4(0,0,1,1) ));
  // _texVS = _ufGpuObj[gl_VertexID]._tex;
  gl_Position = vec4(0,0,0,1); //xform;
}