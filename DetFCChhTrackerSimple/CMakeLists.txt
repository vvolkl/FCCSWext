################################################################################
# Package: DetFCChhTrackerSimple
################################################################################
gaudi_subdir(DetFCChhTrackerSimple v1r0)

gaudi_depends_on_subdirs(GaudiKernel Detector/DetCommon)


find_package(DD4hep)
find_package(Geant4)
include(${Geant4_USE_FILE})

set(CMAKE_MODULE_PATH  ${CMAKE_MODULE_PATH}  ${DD4hep_ROOT}/cmake )
include( DD4hep )

find_package(ROOT COMPONENTS MathCore GenVector Geom REQUIRED)

install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/compact DESTINATION Detector/DetFCChhTrackerSimple)

gaudi_add_library(DetFCChhTrackerSimple
                 src/*.cpp
                 INCLUDE_DIRS DD4hep ROOT Geant4 DetCommon
                 LINK_LIBRARIES GaudiKernel DD4hep ROOT Geant4 DetCommon)

set(LIBRARY_OUTPUT_PATH ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
message(STATUS "LIBRARY_OUTPUT_PATH -> ${LIBRARY_OUTPUT_PATH}")
dd4hep_generate_rootmap(DetFCChhTrackerSimple)

