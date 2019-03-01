set(glvle_sources main.cpp glvle.hpp menubar.cpp main_window.cpp
    dialog.cpp package_window.cpp text_window.cpp vpz_window.cpp
  ${PROJECT_SOURCE_DIR}/external/imgui/imgui.cpp
  ${PROJECT_SOURCE_DIR}/external/imgui/imgui.h
  ${PROJECT_SOURCE_DIR}/external/imgui/imgui_demo.cpp
  ${PROJECT_SOURCE_DIR}/external/imgui/imgui_draw.cpp
  ${PROJECT_SOURCE_DIR}/external/imgui/imgui_widgets.cpp
  ${PROJECT_SOURCE_DIR}/external/imgui/imgui_internal.h
  ${PROJECT_SOURCE_DIR}/external/imgui/imconfig.h
  ${PROJECT_SOURCE_DIR}/external/imgui/imstb_rectpack.h
  ${PROJECT_SOURCE_DIR}/external/imgui/imstb_textedit.h
  ${PROJECT_SOURCE_DIR}/external/imgui/imstb_truetype.h
  ${PROJECT_SOURCE_DIR}/external/imgui/examples/imgui_impl_glfw.cpp
  ${PROJECT_SOURCE_DIR}/external/imgui/examples/imgui_impl_glfw.h
  ${PROJECT_SOURCE_DIR}/external/imgui/examples/imgui_impl_opengl3.cpp
  ${PROJECT_SOURCE_DIR}/external/imgui/examples/imgui_impl_opengl3.h
  ${PROJECT_SOURCE_DIR}/external/imgui/examples/libs/gl3w/GL/gl3w.c
  ${PROJECT_SOURCE_DIR}/external/imgui/examples/libs/gl3w/GL/gl3w.h
  ${PROJECT_SOURCE_DIR}/external/imgui/examples/libs/gl3w/GL/glcorearb.h)

if (WIN32)
  add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/vle.o
    COMMAND ${CMAKE_RC_COMPILER}
    -I${CMAKE_BINARY_DIR}/share
    -i${CMAKE_BINARY_DIR}/share/vle.rc
    -o${CMAKE_CURRENT_BINARY_DIR}/vle.o)

  list(APPEND vle_sources ${CMAKE_CURRENT_BINARY_DIR}/vle.o)
endif ()

add_executable(glvle ${glvle_sources})

target_include_directories(glvle
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include>
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}/external/imgui
  ${PROJECT_SOURCE_DIR}/external/imgui/examples
  ${PROJECT_SOURCE_DIR}/external/imgui/examples/libs/gl3w)

target_compile_definitions(glvle
  PRIVATE
  $<$<BOOL:${WITH_FULL_OPTIMIZATION}>:VLE_FULL_OPTIMIZATION>
  $<$<NOT:$<BOOL:${WITH_DEBUG}>>:VLE_DISABLE_DEBUG>
  $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
  $<$<CXX_COMPILER_ID:MSVC>:_SCL_SECURE_NO_WARNINGS>
  VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
  VERSION_MINOR=${PROJECT_VERSION_MINOR}
  VERSION_PATCH=${PROJECT_VERSION_PATCH})

set_target_properties(glvle
  PROPERTIES
  SOVERSION "${VLE_MAJOR}.${VLE_MINOR}"
  VERSION "${VLE_MAJOR}.${VLE_MINOR}"
  CXX_VISIBILITY_PRESET hidden
  VISIBILITY_INLINES_HIDDEN ON
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED ON)

target_link_libraries(glvle
  PRIVATE
  libvle
  threads
  Boost::boost
  EXPAT::EXPAT
  $<$<PLATFORM_ID:Linux>:dl>
  glfw
  GL)

install(TARGETS glvle DESTINATION bin)
