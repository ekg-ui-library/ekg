cmake_minimum_required(VERSION 3.10)

message(STATUS "EKG tests enabled")
message(STATUS "EKG GUI showcase test building")

project(ekg-gui-showcase-test)

if(
    CMAKE_CXX_COMPILER_ID STREQUAL "Clang"
    OR
    CMAKE_CXX_COMPILER_ID STREQUAL "GNU"
  )
  add_compile_options(-O3)
endif()

if (WIN32)
  set(PLATFORM               "windows")
  set(EXECUTABLE_OUTPUT_PATH ../bin/)
else()
  # Enable if your base OS is Debian-based (e.g Ubuntu).
  # It is a common issue related by the Linux community.
  if (EKG_LINUX_NOT_FOUND_FREETYPE)
    include_directories(/usr/include/freetype2)
    message(STATUS "Using var EKG_LINUX_NOT_FOUND_FREETYPE to include `/usr/include/freetype2` directly")
  endif()

  set(PLATFORM               "linux")
  set(EXECUTABLE_OUTPUT_PATH ../bin/)
endif()

include_directories(../include)
add_executable(ekg-gui-showcase-test "src/ekg_gui_showcase_test.cpp")

file(GLOB LIBRARY_PATH "../lib/${PLATFORM}/libekg.a")
message(STATUS "EKG GUI test found native library on: ${LIBRARY_PATH}")

if (WIN32)
  set(THIRD_PARTY_LIBRARIES mingw32 SDL2main SDL2 opengl32 glew32 freetype)
else()
  set(THIRD_PARTY_LIBRARIES SDL2main SDL2 GL GLEW freetype)
endif()

target_link_libraries(
  ekg-gui-showcase-test
  ${LIBRARY_PATH}
  ${THIRD_PARTY_LIBRARIES}
  -static-libgcc -static-libstdc++    
)

message(STATUS "EKG GUI showcase test building done")
