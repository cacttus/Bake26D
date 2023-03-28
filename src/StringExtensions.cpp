#include <iostream>
#include "./testapp.h"

namespace B26D {

std::string operator+(const std::string& str, const char& rhs) {
  return str + std::to_string(rhs);
}
std::string operator+(const std::string& str, const int8_t& rhs) {
  return str + std::to_string(rhs);
}
std::string operator+(const std::string& str, const int16_t& rhs) {
  return str + std::to_string(rhs);
}
std::string operator+(const std::string& str, const int32_t& rhs) {
  return str + std::to_string(rhs);
}
std::string operator+(const std::string& str, const int64_t& rhs) {
  return str + std::to_string(rhs);
}

std::string operator+(const std::string& str, const uint8_t& rhs) {
  return str + std::to_string(rhs);
}
std::string operator+(const std::string& str, const uint16_t& rhs) {
  return str + std::to_string(rhs);
}
std::string operator+(const std::string& str, const uint32_t& rhs) {
  return str + std::to_string(rhs);
}
std::string operator+(const std::string& str, const uint64_t& rhs) {
  return str + std::to_string(rhs);
}

std::string operator+(const std::string& str, const double& rhs) {
  return str + std::to_string(rhs);
}
std::string operator+(const std::string& str, const float& rhs) {
  return str + std::to_string(rhs);
}

std::string operator+(const std::string& str, const vec2& rhs) {
  return str + std::to_string(rhs.x) + " " + std::to_string(rhs.y);
}
std::string operator+(const std::string& str, const vec3& rhs) {
  return str + std::to_string(rhs.x) + " " + std::to_string(rhs.y) + " " + std::to_string(rhs.z);
}
std::string operator+(const std::string& str, const vec4& rhs) {
  return str + std::to_string(rhs.x) + " " + std::to_string(rhs.y) + " " + std::to_string(rhs.z) + " " + std::to_string(rhs.w);
}

std::string operator+(const std::string& str, const ivec2& rhs) {
  return str + std::to_string(rhs.x) + " " + std::to_string(rhs.y);
}
std::string operator+(const std::string& str, const ivec3& rhs) {
  return str + std::to_string(rhs.x) + " " + std::to_string(rhs.y) + " " + std::to_string(rhs.z);
}
std::string operator+(const std::string& str, const ivec4& rhs) {
  return str + std::to_string(rhs.x) + " " + std::to_string(rhs.y) + " " + std::to_string(rhs.z) + " " + std::to_string(rhs.w);
}

std::string operator+(const std::string& str, const uvec2& rhs) {
  return str + std::to_string(rhs.x) + " " + std::to_string(rhs.y);
}
std::string operator+(const std::string& str, const uvec3& rhs) {
  return str + std::to_string(rhs.x) + " " + std::to_string(rhs.y) + " " + std::to_string(rhs.z);
}
std::string operator+(const std::string& str, const uvec4& rhs) {
  return str + std::to_string(rhs.x) + " " + std::to_string(rhs.y) + " " + std::to_string(rhs.z) + " " + std::to_string(rhs.w);
}

std::string strMat(int cols, int rows, float* vals, std::string separator = ",", int numwidth = 8, char padchar = ' ', int precision = 2) {
  std::ostringstream ret;
  std::string app = " ";
  for (auto ci = 0; ci < cols; ++ci) {
    for (auto ri = 0; ri < rows; ++ri) {
      float f = *(vals + (cols * ci + ri));
      ret << app;
      ret << std::internal << std::setfill(padchar) << std::setw(numwidth) << std::setprecision(precision) << f;
      app = separator;
    }
    ret << "\n";
  }
  return ret.str();
}
std::string operator+(const std::string& str, const mat2& rhs) {
  return str + strMat(2, 2, (float*)&rhs);
}
std::string operator+(const std::string& str, const mat3& rhs) {
  return str + strMat(3, 3, (float*)&rhs);
}
std::string operator+(const std::string& str, const mat4& rhs) {
  return str + strMat(4, 4, (float*)&rhs);
}
std::string operator+(const std::string& str, const path_t& rhs) {
  return str + rhs.string();
}

}  // namespace B26D
