/**
 *  @file StringUtil.h
 *  @date August 23, 2010
 *  @author MetalMario971
 */
#pragma once
#ifndef __STRING_286311063321698159639017_H__
#define __STRING_286311063321698159639017_H__

#include "./GlobalDefines.h"
// #include "../math/MathHeader.h"

namespace TESTAPP_NS {
/**
 *  @class String
 *  @brief String util.
 *  TODO: this class needs to be converted to a proper code page using wint_t instead of char.
 */
class StringUtil {
public:
  StringUtil();
  virtual ~StringUtil() DOES_NOT_OVERRIDE;
  static int32_t compare(const string_t& a, const string_t& b);
  static int32_t compare(const wstring_t& a, const wstring_t& b);
  static bool charIsLetterOrNumber(char c);
  static bool isEmpty(const string_t& str);
  static bool isNotEmpty(const string_t& str);
  static char* getBufferCopy(const string_t& str);
  static string_t createStringFromChars(char* c1, ...);
  static string_t uppercase(const string_t& in);
  static string_t lowercase(const string_t& in);
  static string_t lowercase(const char* _in);
  static string_t uppercase(const char* _in);
  static std::vector<string_t> split(const string_t& in, char del);
  static void split(const string_t& in, char del, std::vector<string_t>& __out_ ret);
  static std::vector<string_t> split(const string_t& in, const std::vector<char>& dels);
  static void split(const string_t& __in_ in, const std::vector<char>& __in_ dels, std::vector<string_t>& __out_ ret);
  static string_t::size_type findFirstOf(const string_t& sin, const std::vector<char>& chars);
  static bool isWspaceEx(int c);
  static bool isNewline(int c);
  static string_t stripDoubleQuotes(const string_t& lc);
  static string_t stripAllChars(const string_t& lc, char ch);
  static string_t getZeroPaddedNumber(int32_t number, int32_t numberOfZeros);
  static string_t getLeftPaddedNumber(int32_t number, int32_t numberOfChars, char padChar);
  static string_t getPaddedNumber(int32_t number, int32_t maxNumberOfChars, char padChar, bool rightAlignNumber = true, bool expand = true);
  static string_t repeat(const string_t& strToRepeat, int32_t nRepeats);
  static string_t empty();
  static string_t emptyString();
  static bool equals(const string_t& a, const string_t& b);
  static bool equals(const wstring_t& a, const wstring_t& b);
  static bool doesNotEqual(const string_t& a, const string_t& b);
  static bool equalsi(const string_t& a, const string_t& b);
  static string_t replaceAll(const string_t& str, char charToRemove, char charToAdd);
  static string_t replaceAll(const string_t& str, const string_t& strToRemove, const string_t& strToAdd);
  static string_t trim(const string_t& s);
  static string_t trimBeg(const string_t& s);
  static string_t trimEnd(const string_t& s);
  static string_t trimDoubleQuotes(const string_t& lc);
  static string_t trim(const string_t& astr, char trimch);
  static string_t trimBeg(const string_t& astr, char trimch);
  static string_t trimEnd(const string_t& astr, char trimch);
  static string_t addCommasToNumber(const string_t& __in_ str);
  static string_t strip(const string_t& str, const string_t& dstart, const string_t& dend, bool keepStart = false, bool keepEnd = false);
  static string_t toHex(int value, bool bIncludePrefix);
  static string_t toDec(int value);
  static string_t getEscapedCharLiteral(char c);
  static bool isDelim(char c, const string_t& delims);
  static bool isChar(char c);
  static bool isWs(int c);
  static bool isWsExceptNewline(char c);
  static string_t removeNewline(const string_t& str);
  static string_t appendLine(string_t& __inout_ str, const string_t& toAppend);
  static string_t generate();
  static bool contains(const string_t& a, const string_t& b);
  static bool contains(const wstring_t& a, const wstring_t& b);
  static string_t format(string_t fmt, ...);
  static string_t join(const string_t& delim, std::vector<string_t>& strings);
  static string_t tabify(const string_t& str_to_tabify, int number_of_tabs, bool use_spaces = true, bool tab_newlines = true, LineBreak lineBreak = LineBreak::DOS);
  static string_t wStrToStr(std::wstring str);
  static wstring_t strToWStr(std::string str);
  static string_t floatToStr2d(float f);
  static string_t slice(string_t& __inout_ str, char a, char b, bool includeDelims = false, bool modify_input_sring = false);
  static bool startsWith(const string_t& str, const string_t& sub);
  static bool endsWith(const string_t& str, const string_t& sub);

  static string_t match(const string_t& s, const string_t& regx);
  static std::vector<string_t> matches(const string_t& s, const string_t& regx);
  static string_t submatch(const string_t& s, const string_t& regx);
  static std::vector<string_t> submatches(const string_t& s, const string_t& regx);
};

}  // namespace TESTAPP_NS

#endif