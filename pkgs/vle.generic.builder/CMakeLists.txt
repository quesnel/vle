install(FILES
  Authors.txt Description.txt License.txt News.txt Readme.txt
  DESTINATION lib/vle-${VLE_VERSION_SHORT}/pkgs/vle.generic.builder)

Declare(simulator pkg-builder vle.generic.builder Builder src/Builder.cpp)
