#pragma once
#ifndef __OVERLAY_904829657238942398472_H__
#define __OVERLAY_904829657238942398472_H__

#include "./GlobalDefines.h"

namespace B26D {
class OffColor {
public:
  // https://yorktown.cbe.wwu.edu/sandvig/shared/NetColors.aspx
  static vec4 AliceBlue;
  static vec4 AntiqueWhite;
  static vec4 Aqua;
  static vec4 Aquamarine;
  static vec4 Azure;
  static vec4 Beige;
  static vec4 Bisque;
  static vec4 Black;
  static vec4 BlanchedAlmond;
  static vec4 Blue;
  static vec4 BlueViolet;
  static vec4 Brown;
  static vec4 BurlyWood;
  static vec4 CadetBlue;
  static vec4 Chartreuse;
  static vec4 Chocolate;
  static vec4 Coral;
  static vec4 CornflowerBlue;
  static vec4 Cornsilk;
  static vec4 Crimson;
  static vec4 Cyan;
  static vec4 DarkBlue;
  static vec4 DarkCyan;
  static vec4 DarkGoldenrod;
  static vec4 DarkGray;
  static vec4 DarkGreen;
  static vec4 DarkKhaki;
  static vec4 DarkMagenta;
  static vec4 DarkOliveGreen;
  static vec4 DarkOrange;
  static vec4 DarkOrchid;
  static vec4 DarkRed;
  static vec4 DarkSalmon;
  static vec4 DarkSeaGreen;
  static vec4 DarkSlateBlue;
  static vec4 DarkSlateGray;
  static vec4 DarkTurquoise;
  static vec4 DarkViolet;
  static vec4 DeepPink;
  static vec4 DeepSkyBlue;
  static vec4 DimGray;
  static vec4 DodgerBlue;
  static vec4 Firebrick;
  static vec4 FloralWhite;
  static vec4 ForestGreen;
  static vec4 Fuchsia;
  static vec4 Gainsboro;
  static vec4 GhostWhite;
  static vec4 Gold;
  static vec4 Goldenrod;
  static vec4 Gray;
  static vec4 Green;
  static vec4 GreenYellow;
  static vec4 Honeydew;
  static vec4 HotPink;
  static vec4 IndianRed;
  static vec4 Indigo;
  static vec4 Ivory;
  static vec4 Khaki;
  static vec4 Lavender;
  static vec4 LavenderBlush;
  static vec4 LawnGreen;
  static vec4 LemonChiffon;
  static vec4 LightBlue;
  static vec4 LightCoral;
  static vec4 LightCyan;
  static vec4 LightGoldenrodYellow;
  static vec4 LightGray;
  static vec4 LightGreen;
  static vec4 LightPink;
  static vec4 LightSalmon;
  static vec4 LightSeaGreen;
  static vec4 LightSkyBlue;
  static vec4 LightSlateGray;
  static vec4 LightSteelBlue;
  static vec4 LightYellow;
  static vec4 Lime;
  static vec4 LimeGreen;
  static vec4 Linen;
  static vec4 Magenta;
  static vec4 Maroon;
  static vec4 MediumAquamarine;
  static vec4 MediumBlue;
  static vec4 MediumOrchid;
  static vec4 MediumPurple;
  static vec4 MediumSeaGreen;
  static vec4 MediumSlateBlue;
  static vec4 MediumSpringGreen;
  static vec4 MediumTurquoise;
  static vec4 MediumVioletRed;
  static vec4 MidnightBlue;
  static vec4 MintCream;
  static vec4 MistyRose;
  static vec4 Moccasin;
  static vec4 NavajoWhite;
  static vec4 Navy;
  static vec4 OldLace;
  static vec4 Olive;
  static vec4 OliveDrab;
  static vec4 Orange;
  static vec4 OrangeRed;
  static vec4 Orchid;
  static vec4 PaleGoldenrod;
  static vec4 PaleGreen;
  static vec4 PaleTurquoise;
  static vec4 PaleVioletRed;
  static vec4 PapayaWhip;
  static vec4 PeachPuff;
  static vec4 Peru;
  static vec4 Pink;
  static vec4 Plum;
  static vec4 PowderBlue;
  static vec4 Purple;
  static vec4 Red;
  static vec4 RosyBrown;
  static vec4 RoyalBlue;
  static vec4 SaddleBrown;
  static vec4 Salmon;
  static vec4 SandyBrown;
  static vec4 SeaGreen;
  static vec4 SeaShell;
  static vec4 Sienna;
  static vec4 Silver;
  static vec4 SkyBlue;
  static vec4 SlateBlue;
  static vec4 SlateGray;
  static vec4 Snow;
  static vec4 SpringGreen;
  static vec4 SteelBlue;
  static vec4 Tan;
  static vec4 Teal;
  static vec4 Thistle;
  static vec4 Tomato;
  static vec4 Turquoise;
  static vec4 Violet;
  static vec4 Wheat;
  static vec4 White;
  static vec4 WhiteSmoke;
  static vec4 Yellow;
  static vec4 YellowGreen;
};

class Overlay {
public:
  enum class MeshRenderMode {
    Points = 0,
    Lines = 1,
    Solid = 2,
    Material = 3,
  };
  enum class ObjectRenderMode {
    Solid,
    Wire,
    Textured,  // blender:material preview
    Rendered
  };

private:
  ObjectRenderMode _objectRenderMode = ObjectRenderMode::Rendered;
  bool _drawWireframeOverlay = false;  // overlay, or base render.
  bool _drawObjectBasis = false;
  bool _drawBoundBoxes = false;
  bool _drawVertexAndFaceNormalsAndTangents = false;
  bool _showSelectionOrigin = true;

  int DbgTotalIndsThisFrame = 0;
  enum { c_pointOB = 0, c_lineOB = 1, c_triOB = 2 };

  RenderView* _view=nullptr;
  GrowList<GpuDebugVert> _verts;
  GrowList<int> _inds[3];
  sptr<Bobj> _debugOb[3];
  sptr<GpuBuffer> _mesh_verts;

public:
  ObjectRenderMode objectRenderMode() { return _objectRenderMode; }
  bool drawWireframeOverlay() { return _drawWireframeOverlay; }
  bool drawObjectBasis() { return _drawObjectBasis; }
  bool drawBoundBoxes() { return _drawBoundBoxes; }
  bool drawVertexAndFaceNormalsAndTangents() { return _drawVertexAndFaceNormalsAndTangents; }
  bool showSelectionOrigin() { return _showSelectionOrigin; }

  Overlay(RenderView* rv);
  void buildDebugDrawMeshes(VisibleStuff* s);
  void endFrame();
  //     void DrawFrustum(Frustum f, float length, MeshRenderMode m = MeshRenderMode.Lines)
  //     {
  //       length = glm::clamp(length, 0.0001f, 99999);
  //       //     6         7
  //       //          f
  //       //     4         5
  //       //2     3
  //       //   a
  //       //0     1
  //       vec2 ptsize(5, 5);
  //       vec2 linsize(3, 3);
  //
  //       int n = _verts.size();
  //
  //       auto fbl = f.Points[Frustum.fpt_nbl] + (f.Points[Frustum.fpt_fbl] - f.Points[Frustum.fpt_nbl]).normalize() * length;
  //       auto fbr = f.Points[Frustum.fpt_nbr] + (f.Points[Frustum.fpt_fbr] - f.Points[Frustum.fpt_nbr]).normalize() * length;
  //       auto ftl = f.Points[Frustum.fpt_ntl] + (f.Points[Frustum.fpt_ftl] - f.Points[Frustum.fpt_ntl]).normalize() * length;
  //       auto ftr = f.Points[Frustum.fpt_ntr] + (f.Points[Frustum.fpt_ftr] - f.Points[Frustum.fpt_ntr]).normalize() * length;
  //
  //       addVert(new v_debug_draw() { _v = f.Points[Frustum.fpt_nbl], _c = OffColor.White, _size = linsize, _outl = vec3.Zero });
  //       addVert(new v_debug_draw() { _v = f.Points[Frustum.fpt_nbr], _c = OffColor.White, _size = linsize, _outl = vec3.Zero });
  //       addVert(new v_debug_draw() { _v = f.Points[Frustum.fpt_ntl], _c = OffColor.White, _size = linsize, _outl = vec3.Zero });
  //       addVert(new v_debug_draw() { _v = f.Points[Frustum.fpt_ntr], _c = OffColor.White, _size = linsize, _outl = vec3.Zero });
  //       addVert(new v_debug_draw() { _v = fbl, _c = OffColor.White, _size = linsize, _outl = vec3.Zero });
  //       addVert(new v_debug_draw() { _v = fbr, _c = OffColor.White, _size = linsize, _outl = vec3.Zero });
  //       addVert(new v_debug_draw() { _v = ftl, _c = OffColor.White, _size = linsize, _outl = vec3.Zero });
  //       addVert(new v_debug_draw() { _v = ftr, _c = OffColor.White, _size = linsize, _outl = vec3.Zero });
  //
  //       Box(new int[] { n + 0, n + 1, n + 2, n + 3, n + 4, n + 5, n + 6, n + 7 }, m);
  //
  //       Point(n + 0);
  //       Point(n + 1);
  //       Point(n + 2);
  //       Point(n + 3);
  //       Point(n + 4);
  //       Point(n + 5);
  //       Point(n + 6);
  //       Point(n + 7);
  //     }
  //     void Box(Box3f b, vec4 color, float width = 1, MeshRenderMode rm = MeshRenderMode.Lines)
  //     {
  //       Box(b._min, b._max, color, width, rm);
  //     }
  //     void Box(OOBox3f b, vec4 color, float width = 1, MeshRenderMode rm = MeshRenderMode.Lines)
  //     {
  //       Box(b.Verts, color, width, rm);
  //     }
  //     void Box(vec3 i, vec3 a, vec4 color, float width = 1, MeshRenderMode rm = MeshRenderMode.Lines)
  //     {
  //       vec3[] points = new vec3[]
  //       {
  //         new vec3(i.x, i.y, i.z),
  //         new vec3(a.x, i.y, i.z),
  //         new vec3(i.x, a.y, i.z),
  //         new vec3(a.x, a.y, i.z),
  //         new vec3(i.x, i.y, a.z),
  //         new vec3(a.x, i.y, a.z),
  //         new vec3(i.x, a.y, a.z),
  //         new vec3(a.x, a.y, a.z)
  //       };
  //       Box(points, color, width, rm);
  //     }
  //     void Box(vec3[] points, vec4 color, float width = 1, MeshRenderMode rm = MeshRenderMode.Lines)
  //     {
  //       Gu.Assert(points.Length == 8);
  //       int n = _verts.Count;
  //       for (int i = 0; i < 8; ++i)
  //       {
  //         AddVert(new v_debug_draw() { _v = points[i], _c = color, _size = new vec2(width, width), _outl = vec3.Zero });
  //       }
  //       Box(new int[] { n + 0, n + 1, n + 2, n + 3, n + 4, n + 5, n + 6, n + 7 }, rm);
  //     }
  //     void Box(int[] inds, MeshRenderMode rm)
  //     {
  //       //      6     7 a
  //       //   2     3
  //       //      4      5
  //       // i 0     1
  //       Gu.Assert(inds.Length == 8);
  //
  //       if (rm == MeshRenderMode.Lines)
  //       {
  //         Line(inds[0], inds[1]);
  //         Line(inds[1], inds[3]);
  //         Line(inds[3], inds[2]);
  //         Line(inds[2], inds[0]);
  //         Line(inds[5], inds[4]);
  //         Line(inds[4], inds[6]);
  //         Line(inds[6], inds[7]);
  //         Line(inds[7], inds[5]);
  //         Line(inds[0], inds[4]);
  //         Line(inds[1], inds[5]);
  //         Line(inds[3], inds[7]);
  //         Line(inds[2], inds[6]);
  //       }
  //       else if (rm == MeshRenderMode.Points)
  //       {
  //         Point(inds[0]);
  //         Point(inds[1]);
  //         Point(inds[2]);
  //         Point(inds[3]);
  //         Point(inds[4]);
  //         Point(inds[5]);
  //         Point(inds[6]);
  //         Point(inds[7]);
  //       }
  //       else if (rm == MeshRenderMode.Solid)
  //       {
  //         Tri(inds[0], inds[1], inds[3]);//a
  //         Tri(inds[0], inds[3], inds[2]);
  //
  //         Tri(inds[5], inds[4], inds[6]);//f
  //         Tri(inds[5], inds[6], inds[7]);
  //
  //         Tri(inds[1], inds[5], inds[7]);//r
  //         Tri(inds[1], inds[7], inds[3]);
  //
  //         Tri(inds[4], inds[0], inds[2]);//l
  //         Tri(inds[4], inds[2], inds[6]);
  //
  //         Tri(inds[5], inds[4], inds[0]);//b
  //         Tri(inds[5], inds[0], inds[1]);
  //
  //         Tri(inds[2], inds[3], inds[7]);//t --room 237!
  //         Tri(inds[2], inds[7], inds[6]);
  //       }
  //       else
  //       {
  //         Gu.BRThrowNotImplementedException();
  //       }
  //     }
  void line(vec3 a, vec3 b, vec4 color, float width = 3);
  void triangle(vec3 v0, vec3 v1, vec3 v2, vec4 c);
  void point(vec3 v, float size = 3);
  void point(vec3 v, vec4 c, float size = 3, vec3* outline = nullptr);
  void point(int idx);
  void line(int a, int b);
  void tri(int a, int b, int c);
  void addVert(GpuDebugVert&& v) { _verts.push_back(v); }
  void createMeshes();
  void createDebugMesh(const string_t& name_pfx, const string_t& materialname, PrimType pt, int index);
};

}  // namespace B26D

#endif