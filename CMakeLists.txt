cmake_minimum_required(VERSION 2.8)
project(player)

INCLUDE(FindPkgConfig)

pkg_check_modules(GST REQUIRED gstreamer-1.0)
pkg_check_modules(GSTPB REQUIRED gstreamer-pbutils-1.0)
pkg_check_modules(GLIB REQUIRED glib-2.0)

include_directories(${GST_INCLUDE_DIRS} ${GSTPB_INCLUDE_DIRS} ${GLIB_INCLUDE_DIRS})
link_directories(${GST_LIBRARY_DIRS} ${GSTPB_LIBRARY_DIRS} ${GLIB_LIBRARY_DIRS})

include_directories("${PROJECT_BINARY_DIR}")
include_directories("${PROJECT_BINARY_DIR}/include/")
add_subdirectory(src)
set(EXTRA_LIBS ${EXTRA_LIBS} player_lib)

add_executable(player player_testsuite.c)
target_link_libraries(player ${EXTRA_LIBS} ${GST_LDFLAGS} ${GSTPB_LDFLAGS} ${GLIB_LDFLAGS})

install(TARGETS player DESTINATION /usr/local/bin/)
