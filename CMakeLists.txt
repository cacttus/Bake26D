# composer application for D26
# Example commands
# in project dir:
#     cmake -B ./build -S .
#     cmake --build ./build -v -t all -j
# ./bin/*x86*

message("CMAKE_VERSION " ${CMAKE_VERSION})

cmake_minimum_required(VERSION 3.0.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Utils
function(GET_ARCH_CONFIG)
  set(ARCH_CONFIG "_")  

  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(ARCH_SIZE Lib)
    string(APPEND ARCH_CONFIG x86)
  else()
    string(APPEND ARCH_CONFIG x64) 
  endif()
   
  # Target
  if(GENERATOR_IS_MULTI_CONFIG)
    message("Error: Generator is MULTI CONFIG")
    return()
    # "Multi-config generators use CMAKE_CONFIGURATION_TYPES as the set of configurations and ignore CMAKE_BUILD_TYPE."
  else()
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
      set(ARCH_CONFIG ${ARCH_CONFIG}d PARENT_SCOPE)
    else ()
      set(ARCH_CONFIG ${ARCH_CONFIG}r PARENT_SCOPE)
    endif()
  endif() 
endfunction()  
function(EXEC_CMD INPUT_CMD)
  #/bin/bash "${CMAKE_CURRENT_SOURCE_DIR}/post-build.sh" ${PRJ_OUTPUT_NAME} "./.vscode/launch.json"  
  add_custom_command  (
    OUTPUT ${CMAKE_SOURCE_DIR}/dependencies/library/lib.o
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND INPUT_CMD
  )
  # if(NOT ECHO_TO)
  #   set(ECHO_TO STDOUT) #NONE
  # endif()
  # execute_process(
  #   COMMAND /bin/bash ${INPUT_CMD}
  #   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
  #   COMMAND_ECHO ${ECHO_TO}
  #   OUTPUT_VARIABLE CMD_STDOUT
  #   ERROR_VARIABLE CMD_STDERR
  #   RESULT_VARIABLE CMD_RES
  # )
  # #message("cmd debug:\nres=${CMD_RES}\nout=${CMD_STDOUT}\nerr=${CMD_STDERR}")
  # message(""${CMD_STDOUT})
  # message(""${CMD_STDERR})  
  # if(NOT((${CMD_RES} EQUAL "0")OR(${CMD_RES} EQUAL "")))
  #     message(FATAL_ERROR "Command failed:${CMD_RES}.")
  # endif()
endfunction()

message("CMAKE_MODULE_PATH='" ${CMAKE_MODULE_PATH} "' CMAKE_PREFIX_PATH='" ${CMAKE_PREFIX_PATH} "' CMAKE_TOOLCHAIN_PATH='" ${CMAKE_TOOLCHAIN_PATH} "'")

# ** Global Directories **  
set(PRJ_BIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bin/)
set(PRJ_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/)

message(PRJ_BIN_DIR= ${PRJ_BIN_DIR})
message(PRJ_LIB_DIR= ${PRJ_LIB_DIR})

set(PRJ_PROJECT_VERSION 0.0.1)
set(PRJ_TARGET_NAME d26-composer)
project(${PRJ_TARGET_NAME} VERSION ${PRJ_PROJECT_VERSION} LANGUAGES C CXX)

EXEC_CMD("echo hi")

if(WIN32) 
  set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/ext/") 
  message("Set Module Path to " ${CMAKE_MODULE_PATH} "Win32")
endif() 
  
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src)

add_executable(${PRJ_TARGET_NAME} ${PRJ_SOURCE_FILES} ${PRJ_HEADER_FILES})

get_arch_config()  

set(PRJ_OUTPUT_NAME ${PRJ_TARGET_NAME}-${PROJECT_VERSION}${ARCH_CONFIG})
message("Output: " ${PRJ_OUTPUT_NAME})

#set prj name in .vscode
# execute_process(
#   COMMAND /bin/bash "${CMAKE_CURRENT_SOURCE_DIR}/post-build.sh" ${PRJ_OUTPUT_NAME} "./.vscode/launch.json"
#   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
#   COMMAND_ECHO STDOUT
#   OUTPUT_VARIABLE CMD_STDOUT
#   ERROR_VARIABLE CMD_STDERR
#   RESULT_VARIABLE CMD_RES
# )
# message(""${CMD_STDOUT})
# if(${CMD_RES} EQUAL "1")
#     message(FATAL_ERROR "Script failed.")
# endif()
# message(""${CMD_STDERR})


set_target_properties(${PROJECT_NAME} PROPERTIES 
  OUTPUT_NAME ${PRJ_OUTPUT_NAME}
  RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PRJ_BIN_DIR}
  RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PRJ_BIN_DIR}
  RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${PRJ_BIN_DIR}
  RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${PRJ_BIN_DIR}
  ARCHIVE_OUTPUT_DIRECTORY ${PRJ_LIB_DIR}
  LIBRARY_OUTPUT_DIRECTORY ${PRJ_LIB_DIR}
  RUNTIME_OUTPUT_DIRECTORY ${PRJ_BIN_DIR}
)

# Packages
#set(OpenGL_GL_PREFERENCE LEGACY) #GLVND or LEGACY
#find_package(OpenGL REQUIRED)
#find_package(SDL2 REQUIRED)
find_package(Threads REQUIRED) #This is better than -pthread

#find_package(Lua REQUIRED)
# if(${LUA_FOUND})
#   message("Lua " ${LUA_VERSION_MAJOR} "." ${LUA_VERSION_MINOR} " found.")
# endif()
if(UNIX)
find_package(X11 REQUIRED)
endif()

# Includes
#target_include_directories(${PROJECT_NAME} PUBLIC ${SDL2_INCLUDE_DIRS} ${SDLNET_INCLUDE_DIR} ${LUA_INCLUDE_DIR} ${LUAINTF_INCLUDE_DIR})

# Libraries
set(ADDL_LIBS "")
if(WIN32)
  set(PRJ_WIN32_COMPILE_OPTS "")

  #Unicode required for building with Unicode Character Set (vs MBCS)
  string(APPEND PRJ_WIN32_COMPILE_OPTS " -DUNICODE")
  string(APPEND PRJ_WIN32_COMPILE_OPTS " -D_UNICODE")

  #Suppress error
  string(APPEND PRJ_WIN32_COMPILE_OPTS " -D_CRT_NO_VA_START_VALIDATION")

  #Multicore build
  string(APPEND PRJ_WIN32_COMPILE_OPTS /MP) #NOte: originally CMAKE_CXX_FLAGS

  target_compile_options(${PRJ_TARGET_NAME} PRIVATE "$<$<CONFIG:RELEASE>:" ${PRJ_WIN32_COMPILE_OPTS})
  target_compile_options(${PRJ_TARGET_NAME} PRIVATE "$<$<CONFIG:DEBUG>:" ${PRJ_WIN32_COMPILE_OPTS} -D_DEBUG -D_GLIBCXX_DEBUG)

  find_package(SDL2_net REQUIRED)
  set(PRJ_ADDL_LIBS "ws2_32.lib" "winmm.lib" "imm32.lib" "version.lib" "dbghelp.lib" "setupapi.lib" "iphlpapi.lib")
elseif(CMAKE_COMPILER_IS_GNUCXX OR MINGW OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
set(PRJ_GCC_LINK_GLOBAL -pthread) 

  target_compile_options(${PRJ_TARGET_NAME} PRIVATE "$<$<CONFIG:DEBUG>:" -D_DEBUG -D_GLIBCXX_DEBUG ">")
  target_compile_options(${PRJ_TARGET_NAME} PRIVATE "$<$<CONFIG:RELEASE>:" -Ofast ">")

  target_link_options(${PRJ_TARGET_NAME} PRIVATE "$<$<CONFIG:RELEASE>:" ${PRJ_GCC_LINK_GLOBAL} ">")
  target_link_options(${PRJ_TARGET_NAME} PRIVATE "$<$<CONFIG:DEBUG>:" ${PRJ_GCC_LINK_GLOBAL} ">")
  
  set(SDLNET_LIBRARIES SDL2_net) 
  set(PRJ_ADDL_LIBS ncurses)
endif() 
target_link_directories(${PROJECT_NAME} PRIVATE ${PRJ_LIB_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE ${OPENGL_LIBRARIES} ${SDL2_LIBRARIES} ${SDLNET_LIBRARIES} ${PRJ_ADDL_LIBS} ${X11_LIBRARIES} ${LUA_LIBRARIES})

#"__prjnm_=$(egrep 'CMAKE_PROJECT_NAME:STATIC=.*' ./build/CMakeCache.txt | cut -f2 -d '=') && __exnm_=$(echo ./bin/${__prjnm_}* | sed -E 's/.*\\/bin\\/(.*)/\\1/g') && "

EXEC_CMD("dummy=${CMAKE_CURRENT_SOURCE_DIR}/build/prjname.dumb && touch ${dummy} && echo ${PRJ_OUTPUT_NAME} >${dummy}")


# set(THE_COMMAND ")
# message("command="${THE_COMMAND})
set(PRJ_DUMMY_FILE ${CMAKE_CURRENT_SOURCE_DIR}/build/dummy.out)
add_custom_command(
  TARGET ${PRJ_TARGET_NAME} 
  POST_BUILD
  COMMAND /bin/bash -c 'touch '${PRJ_DUMMY_FILE} && echo -n ${PRJ_OUTPUT_NAME} 2>&1 1>${PRJ_DUMMY_FILE}
)
