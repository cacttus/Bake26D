
#pragma once
#ifndef __BUFFERED_FILE_H__
#define __BUFFERED_FILE_H__

#include "./testapp.h"

namespace B26D {
/**
 *  @class BinaryFile
 *  @brief Binary parsed file which is loaded into a static buffer in memory via the memory manager.
 */
class BinaryFile {
public:
  enum { file_eof = -1,
         memsize_max = 999999999 };
  typedef int32_t t_filepos;

public:
  std::vector<char>& getData() { return _data; }

  BinaryFile();
  BinaryFile(size_t buffer_size);
  virtual ~BinaryFile();

  void reallocBuffer(size_t i) {
    _data.resize(i);
  }

  size_t pos() { return iFilePos; }
  int8_t at();
  int8_t at(t_filepos _pos);

  void rewind();  // - Rewinds the file pointer.
  int32_t get();
  string_t getTok();                       // - Returns a string token
  string_t getTokSameLineOrReturnEmpty();  // - Returns a string token only if the token is on the same line.
  bool eatWs();                            // - Eat Whitespace (ALSO EATS \n, \r and spaces!!!)
  bool eatWsExceptNewline();               // - Eat Whitespace Does not eat \n
  bool eatLine();                          // - Eats past \n and \r
  bool eatTo(int8_t c);
  bool eof();
  string_t getVersion() { return _strFileVersion; }

  bool readBool(size_t offset = memsize_max);
  int8_t readByte(size_t offset = memsize_max);
  int16_t readInt16(size_t offset = memsize_max);
  int32_t readInt32(size_t offset = memsize_max);
  float readFloat(size_t offset = memsize_max);
  int64_t readInt64(size_t offset = memsize_max);
  uint32_t readUint32(size_t offset = memsize_max);
  glm::vec2 readVec2(size_t offset = memsize_max);
  glm::vec3 readVec3(size_t offset = memsize_max);
  glm::vec4 readVec4(size_t offset = memsize_max);
  glm::ivec3 readivec3(size_t offset = memsize_max);
  std::string readString(size_t offset = memsize_max);
  glm::mat4 readMat4(size_t offset = memsize_max);

  // - Read
  void readBool(bool& val, size_t offset = memsize_max);
  void readVec2(glm::vec2& val, size_t offset = memsize_max);
  void readVec3(glm::vec3& val, size_t offset = memsize_max);
  void readVec4(glm::vec4& val, size_t offset = memsize_max);
  void readivec3(glm::ivec3& val, size_t offset = memsize_max);
  void readByte(int8_t& val, size_t offset = memsize_max);
  void readInt32(int32_t& val, size_t offset = memsize_max);
  void readInt16(int16_t& val, size_t offset = memsize_max);
  void readInt64(int64_t& val, size_t offset = memsize_max);
  void readUint32(uint32_t& val, size_t offset = memsize_max);
  void readFloat(float& val, size_t offset = memsize_max);
  void readString(std::string& val, size_t offset = memsize_max);
  void readMat4(glm::mat4& val, size_t offset = memsize_max);
  void read(const char* buf, size_t bufsiz, size_t offset = memsize_max);
  bool readVersion();

  void writeBool(bool&& val, size_t offset = memsize_max);
  void writeVec2(glm::vec2&& val, size_t offset = memsize_max);
  void writeVec3(glm::vec3&& val, size_t offset = memsize_max);
  void writeVec4(glm::vec4&& val, size_t offset = memsize_max);
  void writeivec3(glm::ivec3&& val, size_t offset = memsize_max);
  void writeByte(int8_t&& val, size_t offset = memsize_max);
  void writeInt32(int32_t&& val, size_t offset = memsize_max);
  void writeInt16(int16_t&& val, size_t offset = memsize_max);
  void writeInt64(int64_t&& val, size_t offset = memsize_max);
  void writeUint32(uint32_t&& val, size_t offset = memsize_max);
  void writeFloat(const float&& val, size_t offset = memsize_max);
  void writeString(std::string&& val, size_t offset = memsize_max);
  void writeMat4(glm::mat4&& val, size_t offset = memsize_max);
  void writeVersion();

  void write(const char* buf, size_t bufsiz, size_t offset = memsize_max);

  //- File Operations
  bool loadFromDisk(const path_t& fileLoc, bool bAddNull = false);                                 // - Read the whole file into the buffer.
  bool loadFromDisk(const path_t& fileLoc, size_t offset, int64_t length, bool bAddNull = false);  // - Read a part of the file.
  bool writeToDisk(const string_t& fileLoc);                                                         // - Read a part of the file.

  std::string toString();

private:
  std::vector<char> _data;
  size_t iFilePos = 0;
  string_t _strFileVersion = "<not_set>";

  void validateRead(size_t outSize, size_t readCount);
  int read(const char* buf, size_t count, size_t bufcount, size_t offset);
  int write(const char* buf, size_t count, size_t bufcount, size_t offset);
};

}  // namespace TestApp

#endif