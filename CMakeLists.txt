cmake_minimum_required(VERSION 3.0.0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PRJ_NAME testapp)
project(${PRJ_NAME} VERSION 0.1.0)

#add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src)
#add_executable(${PRJ_NAME} ${PRJ_SOURCE_FILES} ${PRJ_HEADER_FILES})

#add_library(${PRJ_NAME} ./src/testapp.cpp)
add_executable(${PRJ_NAME} ./src/testapp.cpp)

set(PRJ_BIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bin/)
set(PRJ_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/)
set_target_properties(${PRJ_NAME} PROPERTIES 
  OUTPUT_NAME ${PRJ_NAME}
  RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PRJ_BIN_DIR}
  RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PRJ_BIN_DIR}
  RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${PRJ_BIN_DIR}
  RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${PRJ_BIN_DIR}
  ARCHIVE_OUTPUT_DIRECTORY ${PRJ_LIB_DIR}
  LIBRARY_OUTPUT_DIRECTORY ${PRJ_LIB_DIR}
  RUNTIME_OUTPUT_DIRECTORY ${PRJ_BIN_DIR}
)

#find_package(glfw REQUIRED)
set(OpenGL_GL_PREFERENCE GLVND)#LEGACY) #GLVND) #GLVND or LEGACY
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(Threads REQUIRED) #This is better than -pthread

# Includes
#target_include_directories(${PRJ_NAME} PUBLIC ${SDL2_INCLUDE_DIRS} ${SDLNET_INCLUDE_DIR} ${LUA_INCLUDE_DIR} ${LUAINTF_INCLUDE_DIR})

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

  target_compile_options(${PRJ_NAME} PRIVATE "$<$<CONFIG:RELEASE>:" ${PRJ_WIN32_COMPILE_OPTS})
  target_compile_options(${PRJ_NAME} PRIVATE "$<$<CONFIG:DEBUG>:" ${PRJ_WIN32_COMPILE_OPTS} -D_DEBUG -D_GLIBCXX_DEBUG)

  find_package(SDL2_net REQUIRED)
  set(PRJ_ADDL_LIBS ${PRJ_ADDL_LIBS} "ws2_32.lib" "winmm.lib" "imm32.lib" "version.lib" "dbghelp.lib" "setupapi.lib" "iphlpapi.lib")
elseif(CMAKE_COMPILER_IS_GNUCXX OR MINGW OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
  set(PRJ_GCC_LINK_GLOBAL -pthread) 

  target_compile_options(${PRJ_NAME} PRIVATE "$<$<CONFIG:DEBUG>:" -D_DEBUG -D_GLIBCXX_DEBUG ">")
  target_compile_options(${PRJ_NAME} PRIVATE "$<$<CONFIG:RELEASE>:" -Ofast ">")

  target_link_options(${PRJ_NAME} PRIVATE "$<$<CONFIG:RELEASE>:" ${PRJ_GCC_LINK_GLOBAL} ">")
  target_link_options(${PRJ_NAME} PRIVATE "$<$<CONFIG:DEBUG>:" ${PRJ_GCC_LINK_GLOBAL} ">")
  
  set(SDLNET_LIBRARIES SDL2_net) 
  set(PRJ_ADDL_LIBS ${PRJ_ADDL_LIBS} ncurses)
endif() 
target_link_directories(${PRJ_NAME} PRIVATE ${PRJ_LIB_DIR})
target_link_libraries(${PRJ_NAME} PRIVATE  ${GLFW_LIBRARIES} ${GLEW_LIBRARIES} ${OPENGL_LIBRARIES} ${PRJ_ADDL_LIBS} ${X11_LIBRARIES})
# ${OPENGL_LIBRARIES}
# message("print ALL vars")
# get_cmake_property(_variableNames VARIABLES)
# list (SORT _variableNames)
# foreach (_variableName ${_variableNames})
#     message(STATUS "${_variableName}=${${_variableName}}")
# endforeach() 
# function(dump_cmake_variables)
#     get_cmake_property(_variableNames VARIABLES)
#     list (SORT _variableNames)
#     foreach (_variableName ${_variableNames})
#         if (ARGV0)
#             unset(MATCHED)
#             string(REGEX MATCH ${ARGV0} MATCHED ${_variableName})
#             if (NOT MATCHED)
#                 continue()
#             endif()
#         endif()
#         message(STATUS "${_variableName}=${${_variableName}}")
#     endforeach()
# endfunction()
# dump_cmake_variables()



# set(CPACK_PROJECT_NAME ${PRJ_NAME})
# set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
# include(CPack)
