################################################################################
# Package: DetFCChhHCalTile
################################################################################
gaudi_subdir(newTileCal v1r0)



find_package(DD4hep)
find_package(Geant4)


find_package(ROOT COMPONENTS MathCore GenVector Geom REQUIRED)

install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/compact DESTINATION Detector/newTileCal)


gaudi_add_module(newTileCal
		  src/*.cpp
		  INCLUDE_DIRS DD4hep ROOT Geant4
		  LINK_LIBRARIES GaudiKernel DD4hep ROOT Geant4)

set(LIBRARY_OUTPUT_PATH ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
message(STATUS "LIBRARY_OUTPUT_PATH -> ${LIBRARY_OUTPUT_PATH}")
dd4hep_generate_rootmap(newTileCal)

