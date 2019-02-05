cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(libvle VERSION 2.1.0
  DESCRIPTION "Library for the Multi-Modeling and Simulation Environment VLE"
  LANGUAGES CXX)

set(private_libvle_source
  devs/Coordinator.cpp
  devs/Coordinator.hpp
  devs/Dynamics.cpp
  devs/DynamicsDbg.cpp
  devs/DynamicsDbg.hpp
  devs/DynamicsInit.hpp
  devs/DynamicsObserver.hpp
  devs/DynamicsWrapper.cpp
  devs/Executive.cpp
  devs/ExternalEvent.cpp
  devs/ExternalEventList.cpp
  devs/InitEventList.cpp
  devs/InternalEvent.cpp
  devs/InternalEvent.hpp
  devs/ModelFactory.cpp
  devs/ModelFactory.hpp
  devs/RootCoordinator.cpp
  devs/RootCoordinator.hpp
  devs/Scheduler.cpp
  devs/Scheduler.hpp
  devs/Simulator.cpp
  devs/Simulator.hpp
  devs/Thread.hpp
  devs/Time.cpp
  devs/View.cpp
  devs/ViewEvent.cpp
  devs/ViewEvent.hpp
  devs/View.hpp
  gvle/aboutbox.cpp
  gvle/dom_tools.cpp
  gvle/dom_tools.hpp
  gvle/gvle_file.cpp
  gvle/gvle_plugins.cpp
  gvle/gvle_widgets.cpp
  gvle/gvle_win.cpp
  gvle/helpbrowser.cpp
  gvle/help.cpp
  gvle/logger.cpp
  gvle/notification.cpp
  gvle/vle_dom.cpp
  gvle/vle_dom.hpp
  gvle/vlevpz.cpp
  gvle/vlevpz.hpp
  gvle/widgets/GvleCodeEdit.cpp
  gvle/widgets/GvleTextEdit.cpp
  manager/ExperimentGenerator.cpp
  manager/Manager.cpp
  manager/Simulation.cpp
  oov/Plugin.cpp
  translator/GraphTranslator.cpp
  translator/MatrixTranslator.cpp
  utils/Context.cpp
  utils/ContextModule.cpp
  utils/ContextPrivate.hpp
  utils/ContextSettings.cpp
  utils/DateTime.cpp
  utils/details/Package.hpp
  utils/details/PackageManager.cpp
  utils/details/PackageManager.hpp
  utils/details/PackageParser.cpp
  utils/details/PackageParser.hpp
  utils/details/PathUnix.cpp
  utils/details/PathWin32.cpp
  utils/details/ShellUtils.hpp
  utils/details/SpawnUnix.cpp
  utils/details/SpawnWin.cpp
  utils/details/UtilsWin.cpp
  utils/details/UtilsWin.hpp
  utils/DownloadManager.cpp
  utils/Exception.cpp
  utils/Filesystem.cpp
  utils/i18n.hpp
  utils/Package.cpp
  utils/PackageTable.cpp
  utils/Parser.cpp
  utils/Rand.cpp
  utils/RemoteManager.cpp
  utils/Template.cpp
  utils/Tools.cpp
  value/Boolean.cpp
  value/Double.cpp
  value/Integer.cpp
  value/Map.cpp
  value/Matrix.cpp
  value/Null.cpp
  value/Set.cpp
  value/String.cpp
  value/Table.cpp
  value/Tuple.cpp
  value/Value.cpp
  value/XML.cpp
  vle.cpp
  vpz/AtomicModel.cpp
  vpz/BaseModel.cpp
  vpz/Class.cpp
  vpz/Classes.cpp
  vpz/Condition.cpp
  vpz/Conditions.cpp
  vpz/CoupledModel.cpp
  vpz/Dynamic.cpp
  vpz/Dynamics.cpp
  vpz/Experiment.cpp
  vpz/Model.cpp
  vpz/ModelPortList.cpp
  vpz/Observable.cpp
  vpz/Observables.cpp
  vpz/Output.cpp
  vpz/Outputs.cpp
  vpz/Project.cpp
  vpz/SaxParser.cpp
  vpz/SaxParser.hpp
  vpz/SaxStackValue.cpp
  vpz/SaxStackValue.hpp
  vpz/SaxStackVpz.cpp
  vpz/SaxStackVpz.hpp
  vpz/View.cpp
  vpz/Views.cpp
  vpz/Vpz.cpp)

add_library(libvle ${private_libvle_source})

option(BUILD_SHARED_LIBS "Build shared library" ON)
include(GenerateExportHeader)
generate_export_header(libvle
    EXPORT_MACRO_NAME VLE_API
    EXPORT_FILE_NAME ${CMAKE_BINARY_DIR}/src/DllDefines.hpp)

target_compile_features(libvle PUBLIC cxx_std_14)
target_compile_options(libvle
  PRIVATE
  $<$<BOOL:${WITH_FULL_OPTIMIZATION}>:BARYONYX_FULL_OPTIMIZATION>
  $<$<BOOL:${WITH_DEBUG}>:BARYONYX_ENABLE_DEBUG>
  $<$<BOOL:${WITH_LOG}>:BARYONYX_ENABLE_LOG>
  $<$<BOOL:${NLOPT_FOUND}>:BARYONYX_HAVE_NLOPT>
  $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
  $<$<CXX_COMPILER_ID:MSVC>:_SCL_SECURE_NO_WARNINGS>
  VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
  VERSION_MINOR=${PROJECT_VERSION_MINOR}
  VERSION_PATCH=${PROJECT_VERSION_PATCH})

target_include_directories(libvle
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include>
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR})

set_target_properties(libvle PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
  LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

target_link_libraries(libvle
  PRIVATE
  threads
  Boost::boost
  EXPAT::EXPAT)


install(TARGETS libvle
    EXPORT libvle-targets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${LIBLEGACY_INCLUDE_DIRS}
)

install(DIRECTORY
  ${CMAKE_SOURCE_DIR}/include/vle
  DESTINATION
  ${CMAKE_INSTALL_INCLUDEDIR}/vle-${VLE_ABI})

install(EXPORT libvle-targets
    FILE libvle-targets.cmake
    NAMESPACE libvle::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/yart)

configure_file(vle.pc.in
  ${CMAKE_BINARY_DIR}/src/vle-${VLE_ABI}.pc
  @ONLY)

install(FILES
    ${CMAKE_BINARY_DIR}/src/vle-${VLE_ABI}.pc
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

if (WITH_GVLE)
    configure_file(gvle.pc.in
      ${CMAKE_BINARY_DIR}/src/gvle-${VLE_ABI}.pc
      @ONLY)

    install(FILES
        ${CMAKE_BINARY_DIR}/src/gvle-${VLE_ABI}.pc
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
endif ()

math(EXPR VLE_MACRO_VERSION
    "${VLE_MAJOR} * 100000 + ${VLE_MINOR} * 100 + ${VLE_PATCH}")

configure_file(version.hpp.in
    ${CMAKE_BINARY_DIR}/src/version.hpp
    @ONLY)

install(FILES
    ${CMAKE_BINARY_DIR}/src/version.hpp
    ${CMAKE_BINARY_DIR}/src/DllDefines.hpp
    DESTINATION
    ${CMAKE_INSTALL_INCLUDEDIR}/vle-${VLE_ABI}})

# add_subdirectory(pkgs)
# add_subdirectory(apps)
