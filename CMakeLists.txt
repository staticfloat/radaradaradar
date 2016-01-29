cmake_minimum_required(VERSION 2.8.5)
project(radaradaradar)


set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
add_executable( radar
                src/device.cpp
                src/main.cpp
                src/options.cpp
                src/util.cpp
                src/conversions.cpp)

# Add libraries like FFTW, bladeRF
list( APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake/modules )
find_package( FFTW REQUIRED )
if( FFTW_FOUND )
    include_directories( ${FFTW_INCLUDE_DIRS} )
    target_link_libraries( radar ${FFTW_LIBRARIES} )
else( FFTW_FOUND )
    error("FFTW not found!  Required to build radaradaradar!")
endif( FFTW_FOUND )

find_package( bladeRF REQUIRED )
if( bladeRF_FOUND )
    include_directories( ${bladeRF_INCLUDE_DIRS} )
    target_link_libraries( radar ${bladeRF_LIBRARIES} )
else( bladeRF_FOUND )
    error("libbladeRF not found!  Required to build radaradaradar!")
endif( bladeRF_FOUND)

install( TARGETS radar DESTINATION bin )
