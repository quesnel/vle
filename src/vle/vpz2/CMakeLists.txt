add_sources(vlelib json.hpp)

install(FILES json.hpp DESTINATION ${VLE_INCLUDE_DIRS}/vpz)

if (VLE_HAVE_UNITTESTFRAMEWORK)
  add_executable(test_json_api tests/api.cpp)
  target_link_libraries(test_json_api vlelib ${CMAKE_THREAD_LIBS_INIT})
endif ()
