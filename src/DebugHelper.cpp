#include "./DebugHelper.h"
#include "./testapp.h"
#include "./StringUtil.h"

#include <iostream>
#include <mutex>

#if defined(BR2_OS_WINDOWS)
#include <DbgHelp.h>
#include <TlHelp32.h>
#endif

#if defined(BR2_OS_LINUX)
//required to backtrace_symbols
#include <execinfo.h>
#include <sstream>
//required to demangle names
#include <cxxabi.h>
#endif

namespace B26D {

#if defined(BR2_OS_WINDOWS)
HANDLE hCrtLog;
#endif

bool _bDoDebug = false;

void staticDebugBreak(const string_t& str) {
  LogError(str);
  Gu::debugBreak();
}
void runtimeAssertion(const string_t& str) {
  Raise(str);
}
DebugHelper::DebugHelper() {
}
DebugHelper::~DebugHelper() {
}
void DebugHelper::breakPoint() {
  Gu::debugBreak();
}

void DebugHelper::debugBreak() {
#ifdef _DEBUG
  //  DebugValidate(NULL);    // asm int 3
#endif
}
void DebugHelper::setCheckAlways() {
#ifdef _DEBUG
#if defined(BR2_OS_WINDOWS)
  std::cout << ("****SETTING CHECK ALWAYS***") << std::endl;
  int flags = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
  //clear
  flags &= ~_CRTDBG_CHECK_ALWAYS_DF;
  flags &= ~_CRTDBG_CHECK_EVERY_16_DF;
  flags &= ~_CRTDBG_CHECK_EVERY_128_DF;
  flags &= ~_CRTDBG_CHECK_EVERY_1024_DF;
  //set
  flags |= _CRTDBG_CHECK_ALWAYS_DF;
  _CrtSetDbgFlag(flags);
#else
#pragma warning("Heap debugging not available on Linux.")
#endif
#endif
}
void DebugHelper::setCheck16() {
#ifdef _DEBUG
#if defined(BR2_OS_WINDOWS)
  std::cout << ("****SETTING CHECK 16***") << std::endl;
  int flags = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
  //clear
  flags &= ~_CRTDBG_CHECK_ALWAYS_DF;
  flags &= ~_CRTDBG_CHECK_EVERY_16_DF;
  flags &= ~_CRTDBG_CHECK_EVERY_128_DF;
  flags &= ~_CRTDBG_CHECK_EVERY_1024_DF;
  //set
  flags |= _CRTDBG_CHECK_EVERY_16_DF;
  _CrtSetDbgFlag(flags);
#else
#pragma warning("Heap debugging not available on Linux.")
#endif
#endif
}
void DebugHelper::debugHeapBegin(bool bDoDebug) {
  _bDoDebug = bDoDebug;
  if (_bDoDebug == false) {
    return;
  }

#ifdef _DEBUG
#if defined(BR2_OS_WINDOWS)
  //This chekcs every block for problems
#define VERBOSITY
  //every 16 blocks
  //
  //
  int flags = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
  //https://msdn.microsoft.com/en-us/library/5at7yxcs.aspx
  //flags |= _CRTDBG_CHECK_ALWAYS_DF;
  //flags |= _CRTDBG_CHECK_CRT_DF;
  flags |= _CRTDBG_CHECK_EVERY_16_DF;
  flags |= _CRTDBG_ALLOC_MEM_DF;
  flags |= _CRTDBG_LEAK_CHECK_DF;
  flags |= _CRTDBG_CHECK_CRT_DF;
  // flags |= _CRTDBG_DELAY_FREE_MEM_DF;//Don't actually free blocks.

  _CrtSetDbgFlag(flags);
  hCrtLog = CreateFile(TEXT("crt.log"), GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  SetLastError(0);
  if (hCrtLog != INVALID_HANDLE_VALUE) {
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_WARN, hCrtLog);
    _CrtSetReportFile(_CRT_ERROR, hCrtLog);
  }
  else {
    //_CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_FILE );
    //_CrtSetReportFile( _CRT_ERROR, _CRTDBG_FILE_STDERR );
  }
#else
#pragma warning("Heap debugging not available on Linux.")

#endif
#endif
}
void DebugHelper::setBreakAlloc(int allocNum) {
  if (_bDoDebug == false) {
    return;
  }
#if defined(BR2_OS_WINDOWS)
  _CrtSetBreakAlloc(allocNum);
#else
#pragma warning("Heap debugging not available on Linux.")
#endif
}

void DebugHelper::debugHeapEnd() {
  if (_bDoDebug == false) {
    return;
  }
#ifdef _DEBUG
#if defined(BR2_OS_WINDOWS)
  _CrtCheckMemory();
  //LEAK_CHECK_DF flag will call this
  _CrtDumpMemoryLeaks();

  //http://avid-insight.co.uk/2008/02/tracing-memory-leaks-in-c-microsoft-specific/

  if (hCrtLog != INVALID_HANDLE_VALUE) {
    CloseHandle(hCrtLog);
  }
#else
#pragma warning("Heap debugging not available on Linux.")
#endif
#endif
}
void DebugHelper::checkMemory() {
#ifdef _DEBUG
#if defined(BR2_OS_WINDOWS)
  _CrtCheckMemory();
#else
#pragma warning("Heap debugging not available on Linux.")
#endif
#endif
}

string_t DebugHelper::modList() {
  string_t ret = "";
#if defined(BR2_OS_WINDOWS)

  MODULEENTRY32 me32;
  HANDLE hModuleSnap = INVALID_HANDLE_VALUE;
  DWORD procid = GetCurrentProcessId();
  hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, procid);

  if (hModuleSnap == INVALID_HANDLE_VALUE) {
    ret = "Failed to get module snapshot.";
    return ret;
  }

  me32.dwSize = sizeof(MODULEENTRY32);

  // - Get first module
  if (!Module32First(hModuleSnap, &me32)) {
    ret = "Failed to get the module for call stack analysis.";
    CloseHandle(hModuleSnap);  // Must clean up the snapshot object!
    return ret;
  }

  string_t app = "";
  // - Loop through all modules.
  do {
    ret += app;
    //TDOO: check this for errors 8/11/19
    std::wstring ws = std::wstring(me32.szModule);
    ret += StringUtil::wStrToStr(ws);
    app = ",";
    //me32.
  } while (Module32Next(hModuleSnap, &me32));

  // - Close up shop.
  CloseHandle(hModuleSnap);

#else
  ret = "No module support for Linux, &c.";
#endif
  return ret;
}
#if defined(BR2_OS_LINUX)

string_t linux_bt_demangle_sym(const string_t& stringsym) {
  string_t sym_copy = stringsym;
  //Demangle a linux symbol and return a easy to read result.
  string_t mangled = StringUtil::slice(sym_copy, '(', '+');
  string_t caller_addr = StringUtil::slice(sym_copy, '+', ')');
  string_t unmangled = "";
  string_t call_addr = "";
  string_t result = "";

  int st;
  char* cxx_sname = abi::__cxa_demangle(mangled.c_str(), nullptr, 0, &st);
  if (st == 0) {
    unmangled.assign(cxx_sname);
    free(cxx_sname);
    call_addr = StringUtil::slice(sym_copy, '[', ']', true);
    result = std::string("") + call_addr + " " + unmangled;
  }
  else if (st == -1) {
    Log::print("linux_bt_demangle_sym Error: A memory allocation failure occurred.\n");
  }
  else if (st == -2) {
    //ColoredConsole::print(Stz "linux_bt_demangle_sym Error: @a mangled_name is not a valid name under the C++ ABI mangling rules.\n");
    result = stringsym;
  }
  else if (st == -3) {
    Log::print("linux_bt_demangle_sym Error: One of the arguments is invalid. \n");
  }
  else {
    Log::print("linux_bt_demangle_sym Error: Invalid Argument\n");
  }
  return result;
}
std::vector<std::string> linux_bt_syms(void** buffer, int nptrs) {
  //Return a string backtrace of symbols received by backtrace()
  std::vector<std::string> callStack;
  char** strings = backtrace_symbols(buffer, nptrs);

  if (strings != NULL) {
    for (int j = 0; j < nptrs; j++) {
      string_t demangled_sym = linux_bt_demangle_sym(strings[j]);
      callStack.push_back(demangled_sym);
    }
  }
  else {
    LogError("Could not perform linux stack trace.");
  }
  free(strings);
  return callStack;
}
std::vector<std::string> linux_bt_addr2line(void** buffer, int nptrs) {
  //Use Addr2Line for stacktrace (cleaner).
  std::vector<std::string> callStack;
  string_t sym_addrs_line = "";
  string_t delim = "";
  for (int i = 0; i < nptrs; ++i) {
    std::stringstream my_ss;
    my_ss << std::hex << buffer[i];
    std::string addr_sz = my_ss.str();
    sym_addrs_line += delim + addr_sz;
    delim = " ";
  }
  string_t addr_cmd = std::string("") + "addr2line " + "-e \"" + Gu::appPath().string() + "\" " + "-f -C gcc -p -s " + sym_addrs_line;
  string_t sym_result = OperatingSystem::executeReadOutput(addr_cmd);
  callStack = StringUtil::split(sym_result, '\n');
  if (callStack.size() > 0) {
    callStack.erase(callStack.begin());  //Remove the first which is   ?? ??:0
  }
  return callStack;
}
#endif
std::vector<std::string> DebugHelper::getCallStack(bool bIncludeFrameId) {
  static std::mutex _stackMtx;
  std::lock_guard<std::mutex> guard(_stackMtx);

  static bool s_callstackFailed = false;
  std::vector<std::string> callStack;

  if (s_callstackFailed == false) {
#if defined(BR2_OS_WINDOWS)
    //Code copied from Msdn.
    uint32_t i;
    void* stack[512];
    unsigned short frames;
    SYMBOL_INFO* symbol;
    HANDLE process;

    process = GetCurrentProcess();
    SymInitialize(process, NULL, TRUE);
    frames = CaptureStackBackTrace(0, 100, stack, NULL);
    symbol = (SYMBOL_INFO*)calloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char), 1);
    symbol->MaxNameLen = 255;
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

    for (i = 0; i < frames; i++) {
      SymFromAddr(process, (DWORD64)(stack[i]), 0, symbol);

      if (bIncludeFrameId) {
        callStack.push_back(StringUtil::format("[%i] %s", frames - i - 1, symbol->Name));
      }
      else {
        callStack.push_back(StringUtil::format("%s", symbol->Name));
      }
    }

    free(symbol);
#elif defined(BR2_OS_LINUX)
#define BT_BUF_SIZE 1024
    void *buffer[BT_BUF_SIZE];
    std::memset(buffer, 0, BT_BUF_SIZE);
    int nptrs = backtrace(buffer, BT_BUF_SIZE);

    callStack = linux_bt_syms(buffer, nptrs);

#endif
  }

  return callStack;
}
string_t DebugHelper::getStackTrace() {
  //How to keep debug info in release builds:
  //*https://msdn.microsoft.com/en-us/library/fsk896zz.aspx
  string_t ret = "";
  std::vector<std::string> cs = getCallStack(true);
  for (std::string s : cs) {
    ret.append(s);
    ret.append("\n");
  }
  return ret;
}
string_t DebugHelper::getCallingMethod() {
  //Returns the method that called the current one.
  std::vector<std::string> str = getCallStack(false);
  if (str.size() < 4) {
    return "";
  }
  else {
    return str[3];
  }
}

}  // namespace BR2
