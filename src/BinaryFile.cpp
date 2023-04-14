#include <fstream>
#include "./BinaryFile.h"

namespace B26D {

#pragma region BinaryFile : Methods
BinaryFile::BinaryFile() {}
BinaryFile::BinaryFile(size_t buffer_size) {
  _data.resize(buffer_size);
}
BinaryFile::~BinaryFile() {
  _data.resize(0);
}
void BinaryFile::validateRead(size_t outSize, size_t readCount) {}
void BinaryFile::rewind() {
  // - Rewind the file pointer
  iFilePos = 0;
}
bool BinaryFile::eof() {
  // - Return true if the file is at eof.
  return (iFilePos >= _data.size()) || (iFilePos == -1);
}
bool BinaryFile::eatWs() {
  // - returns 0 on EOF
  // - Eat Whitespace (ALSO EATS \n, \r and spaces!!!)
  // while (StringUtil::isWs(at())) {  // is char is alphanumeric
  //   if (get() == -1) {
  //     return 0;  // inc pointer
  //   }
  // }
  throw 0;

  return 1;
}
bool BinaryFile::eatWsExceptNewline() {
  // - same as eatWs except it returns at newline.
  // while (StringUtil::isWsExceptNewline(at())) {  // is char is alphanumeric
  //   if (get() == -1) {
  //     return 0;  // inc pointer
  //   }
  // }
  throw 0;
  return 1;
}
bool BinaryFile::eatLine() {
  // - Eats the line past the carraige return
  if (!eatTo('\n')) {
    return 0;
  }

  while ((at() == '\r') || (at() == '\n')) {
    get();
  }

  return 1;
}
bool BinaryFile::eatTo(int8_t k) {
  // - eats until character (increments buffer pointer)
  int8_t c;
  while ((c = at()) != k) {
    if (get() == -1)
      return 0;
    {  // inc pointer
    }
  }

  return 1;
}
string_t BinaryFile::getTok() {
  //- Return the next whitespace separated token
  string_t ret;
  throw 0;
  //
  //   if (!eatWs()) {
  //     return ret;
  //   }
  //
  //   int8_t c = at();
  //   while (!StringUtil::isWs(at()) && !eof()) {
  //     c = get();
  //
  //     if (c == -1) {
  //       return ret;  //eof
  //     }
  //
  //     ret.append(1, c);
  //   }

  return ret;
}
/// - Returns empty string if we hit a newline character.
string_t BinaryFile::getTokSameLineOrReturnEmpty() {
  string_t ret;
  throw 0;
  //
  //   if (!eatWsExceptNewline())
  //     return ret;
  //
  //   int8_t c = at();
  //   if (c == '\n') {
  //     return "";
  //   }
  //
  //   while (!StringUtil::isWs(at()) && !eof()) {
  //     c = get();
  //
  //     if (c == -1) {
  //       return ret;  //eof
  //     }
  //
  //     ret += c;
  //   }

  return ret;
}
int8_t BinaryFile::at() {
  // returns character at pos
  if (eof()) {
    return -1;
  }
  return (int8_t)(*(getData().data() + iFilePos));
}
int8_t BinaryFile::at(t_filepos _pos) {
  if (eof()) {
    return -1;
  }
  return (int8_t)(*(getData().data() + _pos));
}
// Returns a character or also file_eof
int32_t BinaryFile::get() {
  if (iFilePos == -1 || iFilePos == getData().size()) {
    return (int32_t)(iFilePos = -1);
  }
  return (int32_t)(*(getData().data() + iFilePos++));
}
bool BinaryFile::loadFromDisk(const path_t& fileLoc, bool bAddNull) {
  return loadFromDisk(fileLoc, 0, -1, bAddNull);
}
bool BinaryFile::loadFromDisk(const path_t& fileLoc, size_t offset, int64_t length, bool bAddNull) {
  rewind();

  auto str = Gu::readFile(fileLoc);
  std::copy(str.begin(), str.end(), std::back_inserter(_data));

  return true;
}
std::string BinaryFile::toString() {
  std::string ret = "";
  if (_data.size() <= 0) {
    return ret;
  }
  char* b2 = new char[_data.size() + 1];
  memset(b2, 0, _data.size() + 1);
  memcpy(b2, _data.data(), _data.size());
  ret.assign(b2, 0, _data.size() + 1);
  delete[] b2;
  return ret;
}
bool BinaryFile::writeToDisk(const string_t& fileLoc) {
  throw 0;
  // if (FileSystem::SDLFileWrite(fileLoc, _data.data(), _data.size()) == 0) {
  //   return true;
  // }
  return false;
  // DiskFile df;
  // df.openForWrite(DiskLoc(fileLoc), FileWriteMode::Truncate);
  // df.write((char*)_data.ptr(), _data.count());
  // df.close();
}
#pragma endregion

#pragma region BinaryFile : Read

bool BinaryFile::readBool(size_t offset) {
  bool tmp;
  readBool(tmp, offset);
  return tmp;
}
int8_t BinaryFile::readByte(size_t offset) {
  int8_t tmp;
  readByte(tmp, offset);
  return tmp;
}
int16_t BinaryFile::readInt16(size_t offset) {
  int16_t tmp;
  readInt16(tmp, offset);
  return tmp;
}
int32_t BinaryFile::readInt32(size_t offset) {
  int32_t tmp;
  readInt32(tmp, offset);
  return tmp;
}
float BinaryFile::readFloat(size_t offset) {
  float tmp;
  readFloat(tmp, offset);
  return tmp;
}
int64_t BinaryFile::readInt64(size_t offset) {
  int64_t tmp;
  readInt64(tmp, offset);
  return tmp;
}
uint32_t BinaryFile::readUint32(size_t offset) {
  uint32_t tmp;
  readUint32(tmp, offset);
  return tmp;
}
glm::vec2 BinaryFile::readVec2(size_t offset) {
  glm::vec2 tmp;
  readVec2(tmp, offset);
  return tmp;
}
glm::vec3 BinaryFile::readVec3(size_t offset) {
  glm::vec3 tmp;
  readVec3(tmp, offset);
  return tmp;
}
glm::vec4 BinaryFile::readVec4(size_t offset) {
  glm::vec4 tmp;
  readVec4(tmp, offset);
  return tmp;
}
glm::ivec3 BinaryFile::readivec3(size_t offset) {
  glm::ivec3 tmp;
  readivec3(tmp, offset);
  return tmp;
}
glm::mat4 BinaryFile::readMat4(size_t offset) {
  glm::mat4 tmp;
  readMat4(tmp, offset);
  return tmp;
}
std::string BinaryFile::readString(size_t offset) {
  std::string tmp;
  readString(tmp, offset);
  return tmp;
}

void BinaryFile::readBool(bool& val, size_t offset) {
  int8_t b;
  readByte(b, offset);
  val = b > 0;
}
void BinaryFile::readByte(int8_t& val, size_t offset) {
  size_t readSize = sizeof(int8_t);
  read((char*)&val, readSize, readSize, offset == memsize_max ? iFilePos : offset);
  iFilePos += readSize;
}
void BinaryFile::readInt16(int16_t& val, size_t offset) {
  int32_t readSize = sizeof(int16_t);
  read((char*)&val, readSize, readSize, offset == memsize_max ? iFilePos : offset);
  iFilePos += readSize;
}
void BinaryFile::readInt32(int32_t& val, size_t offset) {
  int32_t readSize = sizeof(int32_t);
  read((char*)&val, readSize, readSize, offset == memsize_max ? iFilePos : offset);
  iFilePos += readSize;
}
void BinaryFile::readFloat(float& val, size_t offset) {
  int32_t readSize = sizeof(float);
  read((char*)&val, readSize, readSize, offset == memsize_max ? iFilePos : offset);
  iFilePos += readSize;
}
void BinaryFile::readInt64(int64_t& val, size_t offset) {
  int32_t readSize = sizeof(int64_t);
  read((char*)&val, readSize, readSize, offset == memsize_max ? iFilePos : offset);
  iFilePos += readSize;
}
void BinaryFile::readUint32(uint32_t& val, size_t offset) {
  int32_t readSize = sizeof(uint32_t);
  read((char*)&val, readSize, readSize, offset == memsize_max ? iFilePos : offset);
  iFilePos += readSize;
}
void BinaryFile::readVec2(glm::vec2& val, size_t offset) {
  size_t readSize = sizeof(glm::vec2);
  read((char*)&val, readSize, readSize, offset == memsize_max ? iFilePos : offset);
  iFilePos += readSize;
}
void BinaryFile::readVec3(glm::vec3& val, size_t offset) {
  size_t readSize = sizeof(glm::vec3);
  read((char*)&val, readSize, readSize, offset == memsize_max ? iFilePos : offset);
  iFilePos += readSize;
}
void BinaryFile::readVec4(glm::vec4& val, size_t offset) {
  size_t readSize = sizeof(glm::vec4);
  read((char*)&val, readSize, readSize, offset == memsize_max ? iFilePos : offset);
  iFilePos += readSize;
}
void BinaryFile::readivec3(glm::ivec3& val, size_t offset) {
  int32_t readSize = sizeof(glm::ivec3);
  read((char*)&val, readSize, readSize, offset == memsize_max ? iFilePos : offset);
  iFilePos += readSize;
}
void BinaryFile::readMat4(glm::mat4& val, size_t offset) {
  int32_t readSize = sizeof(glm::mat4);
  read((char*)&val, readSize, readSize, offset == memsize_max ? iFilePos : offset);
  iFilePos += readSize;
}
void BinaryFile::readString(std::string& val, size_t offset) {
  int32_t iStringLen = 0;
  readInt32(iStringLen);

  Assert(iStringLen < 32768);
  Assert(iStringLen >= 0);

  if (iStringLen > 0) {
    char* buf = new char[iStringLen + 1];
    read(buf, iStringLen, iStringLen, iFilePos);
    buf[iStringLen] = 0;  // null terminate
    val.assign(buf);
    delete[] buf;
    iFilePos += iStringLen;
  }
  else {
    val = "";
  }
}

void BinaryFile::read(const char* buf, size_t readSize, size_t offset) {
  read((char*)buf, readSize, readSize, offset == memsize_max ? iFilePos : offset);
  iFilePos += readSize;
}
int BinaryFile::read(const char* buf, size_t count, size_t bufcount, size_t offset) {
  Assert((offset >= 0) || (offset == memsize_max));

  if (count > bufcount) {
    Raise("DataBuffer - out of bounds.");
  }
  if (offset == memsize_max) {
    offset = 0;
  }

  memcpy((void*)buf, (void*)(_data.data() + offset), count);

  return 0;
}
bool BinaryFile::readVersion() {
  //   std::string version;
  //   readString(version);
  //
  //   if (!StringUtil::equals(_strFileVersion, version)) {
  //     LogInfo("Binary file version '" + version + "' did not match expected version '" + _strFileVersion + "'.");
  //     Gu::debugBreak();
  //     return false;
  //   }
  throw 0;
  return true;
}
#pragma endregion

#pragma region BinaryFile : Write
void BinaryFile::writeBool(bool&& val, size_t offset) {
  int8_t b = val ? (int8_t)1 : (int8_t)0;
  writeByte(std::move(b), offset);
}
void BinaryFile::writeByte(int8_t&& val, size_t offset) {
  size_t writeSize = sizeof(int8_t);
  write((char*)&val, writeSize, writeSize, offset == memsize_max ? iFilePos : offset);
  iFilePos += writeSize;
}
void BinaryFile::writeInt16(int16_t&& val, size_t offset) {
  int32_t writeSize = sizeof(int16_t);
  write((char*)&val, writeSize, writeSize, offset == memsize_max ? iFilePos : offset);
  iFilePos += writeSize;
}
void BinaryFile::writeInt32(int32_t&& val, size_t offset) {
  int32_t writeSize = sizeof(int32_t);
  write((char*)&val, writeSize, writeSize, offset == memsize_max ? iFilePos : offset);
  iFilePos += writeSize;
}
void BinaryFile::writeFloat(const float&& val, size_t offset) {
  int32_t writeSize = sizeof(float);
  write((char*)&val, writeSize, writeSize, offset == memsize_max ? iFilePos : offset);
  iFilePos += writeSize;
}
void BinaryFile::writeInt64(int64_t&& val, size_t offset) {
  int32_t writeSize = sizeof(int64_t);
  write((char*)&val, writeSize, writeSize, offset == memsize_max ? iFilePos : offset);
  iFilePos += writeSize;
}
void BinaryFile::writeUint32(uint32_t&& val, size_t offset) {
  int32_t writeSize = sizeof(uint32_t);
  write((char*)&val, writeSize, writeSize, offset == memsize_max ? iFilePos : offset);
  iFilePos += writeSize;
}
void BinaryFile::writeVec2(glm::vec2&& val, size_t offset) {
  size_t writeSize = sizeof(glm::vec2);
  write((char*)&val, writeSize, writeSize, offset == memsize_max ? iFilePos : offset);
  iFilePos += writeSize;
}
void BinaryFile::writeVec3(glm::vec3&& val, size_t offset) {
  size_t writeSize = sizeof(glm::vec3);
  write((char*)&val, writeSize, writeSize, offset == memsize_max ? iFilePos : offset);
  iFilePos += writeSize;
}
void BinaryFile::writeVec4(glm::vec4&& val, size_t offset) {
  size_t writeSize = sizeof(glm::vec4);
  write((char*)&val, writeSize, writeSize, offset == memsize_max ? iFilePos : offset);
  iFilePos += writeSize;
}
void BinaryFile::writeivec3(glm::ivec3&& val, size_t offset) {
  int32_t writeSize = sizeof(glm::ivec3);
  write((char*)&val, writeSize, writeSize, offset == memsize_max ? iFilePos : offset);
  iFilePos += writeSize;
}
void BinaryFile::writeString(std::string&& val, size_t offset) {
  writeInt16((int16_t)val.length());

  if (val.length() > 0) {
    write(val.c_str(), val.length(), val.length(), iFilePos);
    iFilePos += val.length();
  }
}
void BinaryFile::writeMat4(glm::mat4&& val, size_t offset) {
  int32_t writeSize = sizeof(glm::mat4);
  write((char*)&val, writeSize, writeSize, offset == memsize_max ? iFilePos : offset);
  iFilePos += writeSize;
}
void BinaryFile::write(const char* buf, size_t writeSize, size_t offset) {
  write(buf, writeSize, writeSize, offset == memsize_max ? iFilePos : offset);
  iFilePos += writeSize;
}
int BinaryFile::write(const char* buf, size_t count, size_t bufcount, size_t offset) {
  Assert((offset >= 0) || (offset == memsize_max));

  if (count > bufcount) {
    Raise("DataBuffer - out of bounds.");
  }
  if (offset == memsize_max) {
    offset = 0;
  }

  // Dynamically reallocate.
  if (_data.size() < offset + count) {
    _data.resize(offset + count);
  }
  memcpy((void*)_data.data(), (void*)(buf + offset), count);
  //_data.copyFrom(buf, count, offset, 0);

  return 0;
}

#pragma endregion

}  // namespace B26D