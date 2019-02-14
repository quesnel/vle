cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(VLE VERSION 2.1.0
  DESCRIPTION "A Multi-Modeling and Simulation Environment"
  LANGUAGES CXX)

# Use the GNU standard installation directories. See
# https://cmake.org/cmake/help/v3.0/module/GNUInstallDirs.html
include(GNUInstallDirs)

# Options
option(WITH_FULL_OPTIMIZATION "disable all logging facilities and active heavy optimization code to speed up simulation. [default: off]" OFF)
option(WITH_DEBUG "enable debug log message. It slows simulation [default: ON]" ON)
option(WITH_GVLE "use QT to build gvle [default: on]" ON)
option(WITH_DOXYGEN "build the documentation with doxygen [default: off]" OFF)
option(WITH_CVLE "build cvle [default: off]" OFF)

# Usefull variables
set(VLE_MAJOR ${PROJECT_VERSION_MAJOR})
set(VLE_MINOR ${PROJECT_VERSION_MINOR})
set(VLE_PATCH ${PROJECT_VERSION_PATCH})
set(VLE_VERSION "${VLE_MAJOR}.${VLE_MINOR}.${VLE_PATCH}")
set(VLE_ABI "${VLE_MAJOR}.${VLE_MINOR}")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

find_package(EXPAT REQUIRED)
if (NOT EXPAT_FOUND)
  message(FATAL_ERROR "expat library is required. Install libexpat first (libexpat1-dev Debian/Ubuntu)")
endif ()

find_package(Boost REQUIRED)
if (NOT Boost_FOUND)
  message(FATAL_ERROR "boost header only library is required. Install boost-dev first (libboost-dev Debian/Ubuntu)")
endif ()

find_package(EXPAT REQUIRED)
if (NOT EXPAT_FOUND)
  message(FATAL_ERROR "expat library is required. Install libexpat first (libexpat1-dev Debian/Ubuntu)")
endif ()

find_package(Threads REQUIRED)
add_library(threads INTERFACE IMPORTED)
set_property(TARGET threads PROPERTY
  INTERFACE_LINK_LIBRARIES ${CMAKE_THREAD_LIBS_INIT})

if (WITH_GVLE)
  set(CMAKE_INCLUDE_CURRENT_DIR ON)
  set(CMAKE_AUTOMOC ON)
  set(CMAKE_AUTOUIC ON)
  set(CMAKE_AUTORCC ON)

  find_package(Qt5 COMPONENTS Widgets Xml Gui Help PrintSupport Svg REQUIRED)

  set (VLE_HAVE_GVLE 1)
else ()
  set (VLE_HAVE_GVLE 0)
endif ()

if (WITH_CVLE)
  find_package(MPI REQUIRED)

  if (NOT MPI_FOUND)
    message(FATAL_ERROR "MPI is required for cvle. Install openmpi, mpich2")
  endif ()

  message(STATUS "mpi include ${MPI_INCLUDE_PATH}")
  message(STATUS "mpi lib ${MPI_LIBRARY}")
  message(STATUS "mpi extra lib ${MPI_EXTRA_LIBRARY}")
  message(STATUS "mpi link ${MPI_LINK_FLAGS}")

  set(VLE_HAVE_CVLE 1)
else ()
  set(VLE_HAVE_CVLE 0)
endif ()

# Tries to found asciidoctor to generate manpages of the share/doc directory.
find_program(ASCIIDOCTOR_EXECUTABLE NAMES asciidoctor)
if (ASCIIDOCTOR_EXECUTABLE)
  message(STATUS "asciidoctor bin ${ASCIIDOCTOR_EXECUTABLE}")
else ()
  message(STATUS "asciidoctor not found.")
endif ()

if (UNIX)
  install(FILES README.md
    DESTINATION ${CMAKE_INSTALL_DATADIR}/vle-${VLE_ABI}/doc
    RENAME README)
  install(FILES COPYING
    DESTINATION ${CMAKE_INSTALL_DATADIR}/vle-${VLE_ABI}/doc)
else ()
  install(FILES README.md
    DESTINATION ${CMAKE_INSTALL_DATADIR}/vle-${VLE_ABI}/doc
    RENAME Readme.txt)
  install(FILES COPYING
    DESTINATION ${CMAKE_INSTALL_DATADIR}/vle-${VLE_ABI}/doc
    RENAME CopyRight.txt)
endif ()

# Gettext utilities
# find_package(Gettext)
# if (GETTEXT_FOUND)
#   set(VLE_HAVE_NLS 1 CACHE INTERNAL "" FORCE)
#   add_subdirectory(i18n)
# else (GETTEXT_FOUND)
#   set(VLE_HAVE_NLS 0 CACHE INTERNAL "" FORCE)
# endif (GETTEXT_FOUND)

# Generating documentation with doxygen
if (WITH_DOXYGEN)
  find_package(Doxygen)
  if (DOXYGEN)
    add_subdirectory(doxygen)
  else (DOXYGEN)
    message(STATUS "WARNING: Doxygen not found - No reference manual create")
  endif (DOXYGEN)
endif (WITH_DOXYGEN)


# Build a generic make uninstall command
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)

message(STATUS "- - - -")
message(STATUS "VLE ${VLE_VERSION} configured successfully")
message(STATUS "Using ${CMAKE_INSTALL_PREFIX} for installation")
message(STATUS "Build type ${CMAKE_BUILD_TYPE}")
message(STATUS "Full optimization.............: ${WITH_FULL_OPTIMIZATION}")
message(STATUS "Show debug message............. ${WITH_DEBUG}")
message(STATUS "Build with gvle...............: ${VLE_HAVE_GVLE}")
message(STATUS "Build with cvle...............: ${VLE_HAVE_CVLE}")

# DeclareSimulator defines system plug-ins for VLE.
function(Declare plugin targetname package name sources)
  add_library(${targetname} MODULE ${sources})

  target_compile_definitions(${targetname}
    PRIVATE
    $<$<BOOL:${WITH_FULL_OPTIMIZATION}>:VLE_FULL_OPTIMIZATION>
    $<$<NOT:$<BOOL:${WITH_DEBUG}>>:VLE_DISABLE_DEBUG>
    $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
    $<$<CXX_COMPILER_ID:MSVC>:_SCL_SECURE_NO_WARNINGS>
    VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
    VERSION_MINOR=${PROJECT_VERSION_MINOR}
    VERSION_PATCH=${PROJECT_VERSION_PATCH})

  target_include_directories(${targetname}
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:include>
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR})

  target_link_libraries(${targetname}
    PRIVATE
    libvle)

  set_target_properties(${targetname} PROPERTIES
    OUTPUT_NAME ${name}
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON)

  install(TARGETS ${targetname}
    RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}/vle-${VLE_ABI}/pkgs/${package}/plugins/${plugin}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/vle-${VLE_ABI}/pkgs/${package}/plugins/${plugin}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/vle-${VLE_ABI}/pkgs/${package}/plugins/${plugin})
endfunction()

function(DeclareGvle plugin targetname package name sources)
  add_library(${targetname} MODULE ${sources})

  target_compile_definitions(${targetname}
    PRIVATE
    $<$<BOOL:${WITH_FULL_OPTIMIZATION}>:VLE_FULL_OPTIMIZATION>
    $<$<NOT:$<BOOL:${WITH_DEBUG}>>:VLE_DISABLE_DEBUG>
    $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
    $<$<CXX_COMPILER_ID:MSVC>:_SCL_SECURE_NO_WARNINGS>
    VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
    VERSION_MINOR=${PROJECT_VERSION_MINOR}
    VERSION_PATCH=${PROJECT_VERSION_PATCH}
    QT_PLUGIN)

  target_include_directories(${targetname}
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:include>
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR})

  target_link_libraries(${targetname}
    PRIVATE
    Qt5::Widgets
    Qt5::Xml
    Qt5::Gui
    Qt5::Help
    Qt5::PrintSupport
    Qt5::Svg)

  set_target_properties(${targetname} PROPERTIES
    OUTPUT_NAME ${name}
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON)

  install(TARGETS ${targetname}
    RUNTIME DESTINATION
    ${CMAKE_INSTALL_LIBDIR}/vle-${VLE_ABI}/pkgs/${package}/plugins/gvle/${plugin}
    LIBRARY DESTINATION
    ${CMAKE_INSTALL_LIBDIR}/vle-${VLE_ABI}/pkgs/${package}/plugins/gvle/${plugin}
    ARCHIVE DESTINATION
    ${CMAKE_INSTALL_LIBDIR}/vle-${VLE_ABI}/pkgs/${package}/plugins/gvle/${plugin})
endfunction()

enable_testing()
add_subdirectory(share)
add_subdirectory(src)
add_subdirectory(pkgs)
add_subdirectory(tests)
include(CMakeCPack.cmake)
