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
option(WITH_WIN32_INSTALLER "prepare win32 installer [default: off]" OFF)

# Usefull variables
set(VLE_MAJOR ${PROJECT_VERSION_MAJOR})
set(VLE_MINOR ${PROJECT_VERSION_MINOR})
set(VLE_PATCH ${PROJECT_VERSION_PATCH})
set(VLE_VERSION "${VLE_MAJOR}.${VLE_MINOR}.${VLE_PATCH}")
set(VLE_ABI "${VLE_MAJOR}.${VLE_MINOR}")

# Installation variables
set(VLE_LIBRARY_DIRS "lib/vle-${VLE_VERSION_SHORT}")
set(VLE_INCLUDE_DIRS "include/vle-${VLE_VERSION_SHORT}/vle")
set(VLE_INCLUDE_DIR "include/vle-${VLE_VERSION_SHORT}")
set(VLE_MANPAGE_PATH "share/man/man1")
set(VLE_SHARE_DIRS "share/vle-${VLE_VERSION_SHORT}")

# Assign C++14 without extension to the whole project.
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# FIXME use old-style link directories for now
# if (COMMAND CMAKE_POLICY)
#   cmake_policy(SET CMP0003 OLD)
#   cmake_policy(SET CMP0020 NEW)#for qt link
# endif (COMMAND CMAKE_POLICY)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

find_package(EXPAT REQUIRED)
if (NOT EXPAT_FOUND)
  message(FATAL_ERROR "expat library is required. Install libexpat first (libexpat1-dev Debian/Ubuntu)")
endif ()

find_package(Boost REQUIRED)
if (NOT Boost_FOUND)
  message(FATAL_ERROR "boost header only library is required. Install boost-dev first (libboost-dev Debian/Ubuntu)")
endif ()

if (COMMAND CMAKE_POLICY)
  cmake_policy(SET CMP0003 OLD)
  cmake_policy(SET CMP0020 NEW)#for qt link
endif (COMMAND CMAKE_POLICY)

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

  find_package(Qt5 COMPONENTS Widgets Xml Gui Help PrintSupport Svg REQUIRED)
  if (NOT Qt5_FOUND)
    message(FATAL_ERROR "Qt5 is required")
  endif (NOT Qt5_FOUND)
  if (NOT Qt5Widgets_FOUND)
    message(FATAL_ERROR "Qt5Widgets is required")
  endif (NOT Qt5Widgets_FOUND)
  if (NOT Qt5Xml_FOUND)
    message(FATAL_ERROR "Qt5Xml is required")
  endif (NOT Qt5Xml_FOUND)
  if (NOT Qt5Gui_FOUND)
    message(FATAL_ERROR "Qt5Gui is required")
  endif (NOT Qt5Gui_FOUND)
  if (NOT Qt5Help_FOUND)
    message(FATAL_ERROR "Qt5Help is required")
  endif (NOT Qt5Help_FOUND)
  if (NOT Qt5PrintSupport_FOUND)
    message(FATAL_ERROR "Qt5PrintSupport is required")
  endif (NOT Qt5PrintSupport_FOUND)
  if (NOT Qt5Svg_FOUND)
    message(FATAL_ERROR "Qt5Svg is required")
  endif (NOT Qt5Svg_FOUND)

  set (VLE_HAVE_GVLE 1)
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
endif ()

# Tries to found asciidoctor to generate manpages of the share/doc directory.
find_program(ASCIIDOCTOR_EXECUTABLE NAMES asciidoctor)
if (ASCIIDOCTOR_EXECUTABLE)
  message(STATUS "asciidoctor -> ${ASCIIDOCTOR_EXECUTABLE}")
else ()
  message(STATUS "asciidoctor not found.")
endif ()

# include_directories(${VLE_BINARY_DIR})


# generate the vle.pc pkg-config file.
# Build some variable to build a correct Win32 pkg config file with:
# prefix=/target to detect the DIRNAME
# cflag=-Iboost_1_34_1 for the directory

# set(VLE_PKGCONFIG_CFLAGS_OTHER "${VLEDEPS_CFLAGS_OTHER_STRING}")
# if (UNIX)
#   set(VLE_PKGCONFIG_PREFIXPATH "${CMAKE_INSTALL_PREFIX}")
#   set(VLE_PKGCONFIG_BOOSTINCLUDE_DIRS "${Boost_INCLUDE_DIRS}")
#   set(VLE_PKGCONFIG_LIBDIR "lib")
# else ()
#   set(VLE_PKGCONFIG_PREFIXPATH "/target")
#   set(VLE_PKGCONFIG_BOOSTINCLUDE_DIRS "\${includedir}")
#   set(VLE_PKGCONFIG_LIBDIR "bin")
# endif ()

#
# Install the README
#
if (UNIX)
  install(FILES "README.md" DESTINATION "${VLE_SHARE_DIRS}")
  install(FILES "COPYING" DESTINATION "${VLE_SHARE_DIRS}")
else ()
  install(FILES "README.md" DESTINATION "${VLE_SHARE_DIRS}" RENAME "Readme.txt")
  install(FILES "COPYING" DESTINATION "${VLE_SHARE_DIRS}" RENAME "CopyRight.txt")
endif ()

#
# Add sources for a target
# add_sources(<target> <source1> [<source2> ...])
#
function(add_sources target)
  get_property(prop_defined GLOBAL PROPERTY ${target}_SRCS DEFINED)
  if(NOT prop_defined)
    define_property(GLOBAL PROPERTY ${target}_SRCS
      BRIEF_DOCS "Sources for the ${target} target"
      FULL_DOCS "List of source files for the ${target} target")
  endif()

  set(SRCS)
  foreach(src ${ARGN})
    if(NOT IS_ABSOLUTE "${src}")
      get_filename_component(src "${src}" ABSOLUTE)
    endif()
    list(APPEND SRCS "${src}")
  endforeach()
  set_property(GLOBAL APPEND PROPERTY "${target}_SRCS" "${SRCS}")
endfunction()


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
message(STATUS "${VLE_NAME_COMPLETE} configured successfully")
message(STATUS "Using ${CMAKE_INSTALL_PREFIX} for installation")
message(STATUS "Build type ${CMAKE_BUILD_TYPE}")
message(STATUS "Full optimization.............: ${WITH_FULL_OPTIMIZATION}")
message(STATUS "Show debug message............. ${WITH_DEBUG}")
message(STATUS "Build with gvle...............: ${VLE_HAVE_GVLE}")
message(STATUS "Build with cvle...............: ${VLE_HAVE_CVLE}")

enable_testing()
add_subdirectory(share)
add_subdirectory(src)
include(CMakeCPack.cmake)
