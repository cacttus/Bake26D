
cmake_minimum_required(VERSION 3.0.0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(PRJ_NAME testapp)
project(${PRJ_NAME} VERSION 0.1.0)


set(PRJ_NAME_EXE ${PRJ_NAME})
#set(PRJ_NAME_LIB lib${PRJ_NAME})
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src)
add_executable(${PRJ_NAME_EXE} ${PRJ_SOURCE_FILES} ${PRJ_HEADER_FILES})
#add_library(${PRJ_NAME_LIB} SHARED ${PRJ_SOURCE_FILES} ${PRJ_HEADER_FILES})

#find_package(glfw REQUIRED)
#find_package(PkgConfig REQUIRED)
set(OpenGL_GL_PREFERENCE GLVND) #GLVND or LEGACY
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED) 
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(Threads REQUIRED)
#find_package(PkgConfig REQUIRED)
#pkg_check_modules(JSONCPP jsoncpp)

set(PRJ_BIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}/)
set(PRJ_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}/)

#install mono-devel
#pkg_check_modules(MONO REQUIRED mono)
# Includes
#get_target_property(TMP_DIRS ${PRJ_NAME} INCLUDE_DIRECTORIES)
#message("includedirs=" ${TMP_DIRS})

function(configure_target targetname)
  #props
  set_target_properties(${targetname} PROPERTIES 
    OUTPUT_NAME ${targetname}
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PRJ_BIN_DIR} 
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PRJ_BIN_DIR}
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${PRJ_BIN_DIR}
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${PRJ_BIN_DIR}
    ARCHIVE_OUTPUT_DIRECTORY ${PRJ_BIN_DIR} 
    LIBRARY_OUTPUT_DIRECTORY ${PRJ_BIN_DIR} 
    RUNTIME_OUTPUT_DIRECTORY ${PRJ_BIN_DIR}
  )

  # Libraries
  set(PRJ_ADDL_LIBS glfw)
  if(WIN32)
    set(PRJ_WIN32_COMPILE_OPTS "")

    #Unicode required for building with Unicode Character Set (vs MBCS)
    string(APPEND PRJ_WIN32_COMPILE_OPTS " -DUNICODE")
    string(APPEND PRJ_WIN32_COMPILE_OPTS " -D_UNICODE")

    #Suppress error
    string(APPEND PRJ_WIN32_COMPILE_OPTS " -D_CRT_NO_VA_START_VALIDATION")

    #Multicore build
    string(APPEND PRJ_WIN32_COMPILE_OPTS /MP) #NOte: originally CMAKE_CXX_FLAGS

    target_compile_options(${targetname} PRIVATE "$<$<CONFIG:RELEASE>:" ${PRJ_WIN32_COMPILE_OPTS})
    target_compile_options(${targetname} PRIVATE "$<$<CONFIG:DEBUG>:" ${PRJ_WIN32_COMPILE_OPTS} -D_DEBUG -D_GLIBCXX_DEBUG)

    find_package(SDL2_net REQUIRED)
    set(PRJ_ADDL_LIBS ${PRJ_ADDL_LIBS} "ws2_32.lib" "winmm.lib" "imm32.lib" "version.lib" "dbghelp.lib" "setupapi.lib" "iphlpapi.lib")
  elseif(CMAKE_COMPILER_IS_GNUCXX OR MINGW OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    set(PRJ_GCC_LINK_GLOBAL -pthread) 

    target_compile_options(${targetname} PRIVATE "$<$<CONFIG:DEBUG>:" -D_DEBUG -D_GLIBCXX_DEBUG ">")
    target_compile_options(${targetname} PRIVATE "$<$<CONFIG:RELEASE>:" -Ofast ">")

    target_link_options(${targetname} PRIVATE "$<$<CONFIG:RELEASE>:" ${PRJ_GCC_LINK_GLOBAL} ">")
    target_link_options(${targetname} PRIVATE "$<$<CONFIG:DEBUG>:" ${PRJ_GCC_LINK_GLOBAL} ">")
    
    target_compile_options(${targetname} PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/Zi> $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-g>)

    set(PRJ_ADDL_LIBS ${PRJ_ADDL_LIBS} ncurses)
  endif() 
  target_link_directories(${targetname} PRIVATE ${PRJ_LIB_DIR})
  target_link_libraries(${targetname} PRIVATE  ${GLFW_LIBRARIES} ${GLEW_LIBRARIES} ${OPENGL_LIBRARIES} ${PRJ_ADDL_LIBS} ${X11_LIBRARIES} ${JSONCPP_LIBRARIES})
  target_include_directories(${targetname} PUBLIC ${INCLUDE_DIRECTORIES} ${JSONCPP_INCLUDEDIR})
endfunction()

#message("LIBSSS=" ${GLFW_LIBRARIES} ${GLEW_LIBRARIES} ${OPENGL_LIBRARIES} ${PRJ_ADDL_LIBS} ${X11_LIBRARIES} ${JSONCPP_LIBRARIES})

configure_target(${PRJ_NAME_EXE})
#configure_target(${PRJ_NAME_LIB})
  # libmono-2.0.so

#SET_COMPILE_OPTS(${PRJ_EXE_NAME} False)
#SET_COMPILE_OPTS(${PRJ_LIB_NAME} True)

function(dump_cmake_variables)
  message("print ALL vars")
  get_cmake_property(_variableNames VARIABLES)
  list (SORT _variableNames)
  foreach (_variableName ${_variableNames})
    if (ARGV0)
      unset(MATCHED)
      string(REGEX MATCH ${ARGV0} MATCHED ${_variableName})
      if (NOT MATCHED)
        continue()
      endif()
    endif()
    message(STATUS "${_variableName}=${${_variableName}}")
  endforeach()
endfunction()
#dump_cmake_variables()

#message("masdfasdf " ${mono_LIBS} ${mono_STATIC_CFLAGS} ${mono_STATIC_LIBRARIES})

#target_link_libraries(my_app ${ ${MONO_LIBRARIES})
#target_include_directories(my_app PUBLIC ${MONO_INCLUDE_DIRS})
#target_compile_options(my_app PUBLIC ${MONO_CFLAGS_OTHER})

#gcc sample.c -rdynamic `pkg-config --cflags --libs mono-2`





# set(CPACK_PROJECT_NAME ${PRJ_NAME})
# set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
# include(CPack)
  