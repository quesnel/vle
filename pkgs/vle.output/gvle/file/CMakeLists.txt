DeclareGvle(output gvleOutputFile vle.output gvleOutputFile 
  FilePluginGUI.cpp
  FilePluginGUI.h
  FilePluginGUItab.cpp
  FilePluginGUItab.h
  FilePluginGvle.ui)

# QT5_WRAP_UI(FilePluginGvle_h FilePluginGvle.ui)
# ADD_DEFINITIONS(${Qt5Widgets_DEFINITIONS} ${Qt5Gui_DEFINITIONS}
#   ${Qt5Xml_DEFINITIONS})
# 
# ADD_DEFINITIONS(-DQT_PLUGIN)
# 
# ADD_LIBRARY(gvleOutputFile MODULE
#   ${FilePlugin_SOURCES}
#   ${FilePluginGvle_h} )
# 
# TARGET_LINK_LIBRARIES(gvleOutputFile
#   gvlelib ${Qt5Xml_LIBRARIES} ${Qt5Widgets_LIBRARIES} ${Qt5Gui_LIBRARIES}
#   ${Qt5Help_LIBRARIES} ${Qt5PrintSupport_LIBRARIES} vlelib ${EXPAT_LIBRARIES}
#   ${CMAKE_THREAD_LIBS_INIT} ${OS_SPECIFIC_LIBRARIES})
# 
# INSTALL(TARGETS gvleOutputFile
#   RUNTIME DESTINATION lib/vle-${VLE_VERSION_SHORT}/pkgs/vle.output/plugins/gvle/output
#   LIBRARY DESTINATION lib/vle-${VLE_VERSION_SHORT}/pkgs/vle.output/plugins/gvle/output
#   ARCHIVE DESTINATION lib/vle-${VLE_VERSION_SHORT}/pkgs/vle.output/plugins/gvle/output)
