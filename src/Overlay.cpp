#include "./testapp.h"
#include "./Overlay.h"

namespace B26D {
#pragma region offcoolor
vec4 OffColor::AliceBlue = Gu::rgb_ub(0xF0F8FF);
vec4 OffColor::AntiqueWhite = Gu::rgb_ub(0xFAEBD7);
vec4 OffColor::Aqua = Gu::rgb_ub(0x00FFFF);
vec4 OffColor::Aquamarine = Gu::rgb_ub(0x7FFFD4);
vec4 OffColor::Azure = Gu::rgb_ub(0xF0FFFF);
vec4 OffColor::Beige = Gu::rgb_ub(0xF5F5DC);
vec4 OffColor::Bisque = Gu::rgb_ub(0xFFE4C4);
vec4 OffColor::Black = Gu::rgb_ub(0x000000);
vec4 OffColor::BlanchedAlmond = Gu::rgb_ub(0xFFEBCD);
vec4 OffColor::Blue = Gu::rgb_ub(0x0000FF);
vec4 OffColor::BlueViolet = Gu::rgb_ub(0x8A2BE2);
vec4 OffColor::Brown = Gu::rgb_ub(0xA52A2A);
vec4 OffColor::BurlyWood = Gu::rgb_ub(0xDEB887);
vec4 OffColor::CadetBlue = Gu::rgb_ub(0x5F9EA0);
vec4 OffColor::Chartreuse = Gu::rgb_ub(0x7FFF00);
vec4 OffColor::Chocolate = Gu::rgb_ub(0xD2691E);
vec4 OffColor::Coral = Gu::rgb_ub(0xFF7F50);
vec4 OffColor::CornflowerBlue = Gu::rgb_ub(0x6495ED);
vec4 OffColor::Cornsilk = Gu::rgb_ub(0xFFF8DC);
vec4 OffColor::Crimson = Gu::rgb_ub(0xDC143C);
vec4 OffColor::Cyan = Gu::rgb_ub(0x00FFFF);
vec4 OffColor::DarkBlue = Gu::rgb_ub(0x00008B);
vec4 OffColor::DarkCyan = Gu::rgb_ub(0x008B8B);
vec4 OffColor::DarkGoldenrod = Gu::rgb_ub(0xB886BB);
vec4 OffColor::DarkGray = Gu::rgb_ub(0xA9A9A9);
vec4 OffColor::DarkGreen = Gu::rgb_ub(0x006400);
vec4 OffColor::DarkKhaki = Gu::rgb_ub(0xBDB76B);
vec4 OffColor::DarkMagenta = Gu::rgb_ub(0x8B008B);
vec4 OffColor::DarkOliveGreen = Gu::rgb_ub(0x556B2F);
vec4 OffColor::DarkOrange = Gu::rgb_ub(0xFF8C00);
vec4 OffColor::DarkOrchid = Gu::rgb_ub(0x9932CC);
vec4 OffColor::DarkRed = Gu::rgb_ub(0x8B0000);
vec4 OffColor::DarkSalmon = Gu::rgb_ub(0xE9967A);
vec4 OffColor::DarkSeaGreen = Gu::rgb_ub(0x8FBC8B);
vec4 OffColor::DarkSlateBlue = Gu::rgb_ub(0x483D8B);
vec4 OffColor::DarkSlateGray = Gu::rgb_ub(0x2F4F4F);
vec4 OffColor::DarkTurquoise = Gu::rgb_ub(0x00CED1);
vec4 OffColor::DarkViolet = Gu::rgb_ub(0x9400D3);
vec4 OffColor::DeepPink = Gu::rgb_ub(0xFF1493);
vec4 OffColor::DeepSkyBlue = Gu::rgb_ub(0x00BFFF);
vec4 OffColor::DimGray = Gu::rgb_ub(0x696969);
vec4 OffColor::DodgerBlue = Gu::rgb_ub(0x1E90FF);
vec4 OffColor::Firebrick = Gu::rgb_ub(0xB22222);
vec4 OffColor::FloralWhite = Gu::rgb_ub(0xFFFAF0);
vec4 OffColor::ForestGreen = Gu::rgb_ub(0x228B22);
vec4 OffColor::Fuchsia = Gu::rgb_ub(0xFF00FF);
vec4 OffColor::Gainsboro = Gu::rgb_ub(0xDCDCDC);
vec4 OffColor::GhostWhite = Gu::rgb_ub(0xF8F8FF);
vec4 OffColor::Gold = Gu::rgb_ub(0xFFD700);
vec4 OffColor::Goldenrod = Gu::rgb_ub(0xDAA520);
vec4 OffColor::Gray = Gu::rgb_ub(0x808080);
vec4 OffColor::Green = Gu::rgb_ub(0x008000);
vec4 OffColor::GreenYellow = Gu::rgb_ub(0xADFF2F);
vec4 OffColor::Honeydew = Gu::rgb_ub(0xF0FFF0);
vec4 OffColor::HotPink = Gu::rgb_ub(0xFF69B4);
vec4 OffColor::IndianRed = Gu::rgb_ub(0xCD5C5C);
vec4 OffColor::Indigo = Gu::rgb_ub(0x4B0082);
vec4 OffColor::Ivory = Gu::rgb_ub(0xFFFFF0);
vec4 OffColor::Khaki = Gu::rgb_ub(0xF0E68C);
vec4 OffColor::Lavender = Gu::rgb_ub(0xE6E6FA);
vec4 OffColor::LavenderBlush = Gu::rgb_ub(0xFFF0F5);
vec4 OffColor::LawnGreen = Gu::rgb_ub(0x7CFC00);
vec4 OffColor::LemonChiffon = Gu::rgb_ub(0xFFFACD);
vec4 OffColor::LightBlue = Gu::rgb_ub(0xADD8E6);
vec4 OffColor::LightCoral = Gu::rgb_ub(0xF08080);
vec4 OffColor::LightCyan = Gu::rgb_ub(0xE0FFFF);
vec4 OffColor::LightGoldenrodYellow = Gu::rgb_ub(0xFAFAD2);
vec4 OffColor::LightGray = Gu::rgb_ub(0xD3D3D3);
vec4 OffColor::LightGreen = Gu::rgb_ub(0x90EE90);
vec4 OffColor::LightPink = Gu::rgb_ub(0xFFB6C1);
vec4 OffColor::LightSalmon = Gu::rgb_ub(0xFFA07A);
vec4 OffColor::LightSeaGreen = Gu::rgb_ub(0x20B2AA);
vec4 OffColor::LightSkyBlue = Gu::rgb_ub(0x87CEFA);
vec4 OffColor::LightSlateGray = Gu::rgb_ub(0x778899);
vec4 OffColor::LightSteelBlue = Gu::rgb_ub(0xB0C4DE);
vec4 OffColor::LightYellow = Gu::rgb_ub(0xFFFFE0);
vec4 OffColor::Lime = Gu::rgb_ub(0x00FF00);
vec4 OffColor::LimeGreen = Gu::rgb_ub(0x32CD32);
vec4 OffColor::Linen = Gu::rgb_ub(0xFAF0E6);
vec4 OffColor::Magenta = Gu::rgb_ub(0xFF00FF);
vec4 OffColor::Maroon = Gu::rgb_ub(0x800000);
vec4 OffColor::MediumAquamarine = Gu::rgb_ub(0x66CDAA);
vec4 OffColor::MediumBlue = Gu::rgb_ub(0x0000CD);
vec4 OffColor::MediumOrchid = Gu::rgb_ub(0xBA55D3);
vec4 OffColor::MediumPurple = Gu::rgb_ub(0x9370DB);
vec4 OffColor::MediumSeaGreen = Gu::rgb_ub(0x3CB371);
vec4 OffColor::MediumSlateBlue = Gu::rgb_ub(0x7B68EE);
vec4 OffColor::MediumSpringGreen = Gu::rgb_ub(0x00FA9A);
vec4 OffColor::MediumTurquoise = Gu::rgb_ub(0x48D1CC);
vec4 OffColor::MediumVioletRed = Gu::rgb_ub(0xC71585);
vec4 OffColor::MidnightBlue = Gu::rgb_ub(0x191970);
vec4 OffColor::MintCream = Gu::rgb_ub(0xF5FFFA);
vec4 OffColor::MistyRose = Gu::rgb_ub(0xFFE4E1);
vec4 OffColor::Moccasin = Gu::rgb_ub(0xFFE4B5);
vec4 OffColor::NavajoWhite = Gu::rgb_ub(0xFFDEAD);
vec4 OffColor::Navy = Gu::rgb_ub(0x000080);
vec4 OffColor::OldLace = Gu::rgb_ub(0xFDF5E6);
vec4 OffColor::Olive = Gu::rgb_ub(0x808000);
vec4 OffColor::OliveDrab = Gu::rgb_ub(0x6B8E23);
vec4 OffColor::Orange = Gu::rgb_ub(0xFFA500);
vec4 OffColor::OrangeRed = Gu::rgb_ub(0xFF4500);
vec4 OffColor::Orchid = Gu::rgb_ub(0xDA70D6);
vec4 OffColor::PaleGoldenrod = Gu::rgb_ub(0xEEE8AA);
vec4 OffColor::PaleGreen = Gu::rgb_ub(0x98FB98);
vec4 OffColor::PaleTurquoise = Gu::rgb_ub(0xAFEEEE);
vec4 OffColor::PaleVioletRed = Gu::rgb_ub(0xDB7093);
vec4 OffColor::PapayaWhip = Gu::rgb_ub(0xFFEFD5);
vec4 OffColor::PeachPuff = Gu::rgb_ub(0xFFDAB9);
vec4 OffColor::Peru = Gu::rgb_ub(0xCD853F);
vec4 OffColor::Pink = Gu::rgb_ub(0xFFC0CB);
vec4 OffColor::Plum = Gu::rgb_ub(0xDDA0DD);
vec4 OffColor::PowderBlue = Gu::rgb_ub(0xB0E0E6);
vec4 OffColor::Purple = Gu::rgb_ub(0x800080);
vec4 OffColor::Red = Gu::rgb_ub(0xFF0000);
vec4 OffColor::RosyBrown = Gu::rgb_ub(0xBC8F8F);
vec4 OffColor::RoyalBlue = Gu::rgb_ub(0x4169E1);
vec4 OffColor::SaddleBrown = Gu::rgb_ub(0x8B4513);
vec4 OffColor::Salmon = Gu::rgb_ub(0xFA8072);
vec4 OffColor::SandyBrown = Gu::rgb_ub(0xF4A460);
vec4 OffColor::SeaGreen = Gu::rgb_ub(0x2E8B57);
vec4 OffColor::SeaShell = Gu::rgb_ub(0xFFF5EE);
vec4 OffColor::Sienna = Gu::rgb_ub(0xA0522D);
vec4 OffColor::Silver = Gu::rgb_ub(0xC0C0C0);
vec4 OffColor::SkyBlue = Gu::rgb_ub(0x87CEEB);
vec4 OffColor::SlateBlue = Gu::rgb_ub(0x6A5ACD);
vec4 OffColor::SlateGray = Gu::rgb_ub(0x708090);
vec4 OffColor::Snow = Gu::rgb_ub(0xFFFAFA);
vec4 OffColor::SpringGreen = Gu::rgb_ub(0x00FF7F);
vec4 OffColor::SteelBlue = Gu::rgb_ub(0x4682B4);
vec4 OffColor::Tan = Gu::rgb_ub(0xD2B48C);
vec4 OffColor::Teal = Gu::rgb_ub(0x008080);
vec4 OffColor::Thistle = Gu::rgb_ub(0xD8BFD8);
vec4 OffColor::Tomato = Gu::rgb_ub(0xFF6347);
vec4 OffColor::Turquoise = Gu::rgb_ub(0x40E0D0);
vec4 OffColor::Violet = Gu::rgb_ub(0xEE82EE);
vec4 OffColor::Wheat = Gu::rgb_ub(0xF5DEB3);
vec4 OffColor::White = Gu::rgb_ub(0xFFFFFF);
vec4 OffColor::WhiteSmoke = Gu::rgb_ub(0xF5F5F5);
vec4 OffColor::Yellow = Gu::rgb_ub(0xFFFF00);
vec4 OffColor::YellowGreen = Gu::rgb_ub(0x9ACD32);
#pragma endregion

Overlay::Overlay(RenderView* rv) {
  //_mesh_verts = std::make_unique<GpuBuffer>();
  createMeshes();
  _view = rv;
}
void Overlay::buildDebugDrawMeshes(VisibleStuff* s) {
  DbgTotalIndsThisFrame = _inds[c_pointOB].size() + _inds[c_lineOB].size() + _inds[c_triOB].size();

  if (_verts.size() > 0 && DbgTotalIndsThisFrame > 0) {
    _mesh_verts->copyToGpu(sizeof(GpuDebugVert) * _verts.size(), _verts.data());

    for (int iprim = 0; iprim < 3; iprim++) {
      if (_inds[iprim].size() > 0) {
        if (_view->camera()->frustum()->hasBox(_debugOb[iprim]->boundBox())) {
          _debugOb[iprim]->mesh()->ibo()->copyToGpu(_inds->size()*sizeof(int), _inds->data());
          s->addObject(_view, _debugOb[iprim].get());
        }
      }
    }
  }
}
void Overlay::endFrame() {
  for (int i = 0; i < 3; ++i) {
    _debugOb[i]->boundBox().genReset();
    _inds[i].reset();
  }
  _verts.reset();
}
void Overlay::line(vec3 a, vec3 b, vec4 color, float width ) {
  addVert(GpuDebugVert(a, color, vec2(width, width), vec3(0, 0, 0)));
  addVert(GpuDebugVert(b, color, vec2(width, width), vec3(0, 0, 0)));
  line(_verts.size() - 2, _verts.size() - 1);
}
void Overlay::triangle(vec3 v0, vec3 v1, vec3 v2, vec4 c) {
  addVert(GpuDebugVert(v0, c, vec2(0, 0), vec3(0, 0, 0)));
  addVert(GpuDebugVert(v1, c, vec2(0, 0), vec3(0, 0, 0)));
  addVert(GpuDebugVert(v2, c, vec2(0, 0), vec3(0, 0, 0)));
  tri(_verts.size() - 3, _verts.size() - 2, _verts.size() - 1);
}
void Overlay::point(vec3 v, float size ) {
  point(v, OffColor::White, size);
}
void Overlay::point(vec3 v, vec4 c, float size, vec3* outline ) {
  // add a single point.
  addVert(GpuDebugVert(v, c, vec2(size, size), ((outline == nullptr) ? (vec3(OffColor::White)) : *outline)));
  point(_verts.size() - 1);
}
void Overlay::point(int idx) {
  _inds[c_pointOB].push_back(idx);
  _debugOb[c_pointOB]->boundBox().genExpandByPoint(_verts[idx]._v);
}
void Overlay::line(int a, int b) {
  _inds[c_lineOB].push_back(a);
  _inds[c_lineOB].push_back(b);
  _debugOb[c_lineOB]->boundBox().genExpandByPoint(_verts[a]._v);
  _debugOb[c_lineOB]->boundBox().genExpandByPoint(_verts[b]._v);
}
void Overlay::tri(int a, int b, int c) {
  _inds[c_triOB].push_back(a);
  _inds[c_triOB].push_back(b);
  _inds[c_triOB].push_back(c);
  _debugOb[c_triOB]->boundBox().genExpandByPoint(_verts[a]._v);
  _debugOb[c_triOB]->boundBox().genExpandByPoint(_verts[b]._v);
  _debugOb[c_triOB]->boundBox().genExpandByPoint(_verts[c]._v);
}
void Overlay::createMeshes() {
  _mesh_verts = std::make_shared<GpuBuffer>(sizeof(GpuDebugVert)*_verts.size(), _verts.data());
//std::make_shared<Material>();
  createDebugMesh("points", res::material::DebugDrawMaterial_Points(), PrimType::Points, c_pointOB);
  createDebugMesh("lines",  res::material::DebugDrawMaterial_Lines(),   PrimType::Lines, c_lineOB);
  createDebugMesh("tris",   res::material::DebugDrawMaterial_Tris(),     PrimType::Triangles, c_triOB);
}
void Overlay::createDebugMesh(const string_t& name_pfx, const string_t& materialname, PrimType pt, int index) {
  auto pind = std::make_shared<GpuBuffer>(sizeof(int)*_inds[index].size(), _inds[index].data());
  _debugOb[index] = std::make_shared<Bobj>("debug_" + name_pfx + "ob");
  _debugOb[index]->material() = Gu::findMaterial(materialname); 
  _debugOb[index]->mesh() = std::make_shared<Mesh>("debug_"+name_pfx, pt, _mesh_verts, pind, nullptr, false);
}

}  // namespace B26D