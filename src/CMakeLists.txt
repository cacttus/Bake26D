# VG Sources

#message("/src/ current CMAKE src dir is " ${CMAKE_CURRENT_SOURCE_DIR})
file(GLOB_RECURSE PRJ_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp *.c)
file(GLOB_RECURSE PRJ_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.h *.hpp)

list(LENGTH PRJ_SOURCE_FILES PRJ_SOURCE_FILES_LEN)
list(LENGTH PRJ_HEADER_FILES PRJ_HEADER_FILES_LEN)
#message(" + " ${PRJ_SOURCE_FILES_LEN} )# " Sources: " ${PRJ_SOURCE_FILES})
#message(" + " ${PRJ_HEADER_FILES_LEN} ) #" Headers: " ${PRJ_HEADER_FILES})

set(PRJ_SOURCE_FILES ${PRJ_SOURCE_FILES} PARENT_SCOPE)
set(PRJ_HEADER_FILES ${PRJ_HEADER_FILES} PARENT_SCOPE)

