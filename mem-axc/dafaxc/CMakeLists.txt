cmake_minimum_required(VERSION 2.8)
project(dafaxc)

# set(BLOOM_PATH /home/nvedula/summer2016/mem-axc-64-build/bloom-build) 
# set(BLOOM_PATH $NACHOSROOT/build_repo/build-mem-axc-64/bloom) 
option(BLOOM_PATH "BLOOM_PATH")
set(PACKAGE_NAME dafaxc)
set(PACKAGE_VERSION 0.1)
set(PACKAGE_STRING "${PACKAGE_NAME} ${PACKAGE_VERSION}")
set(PACKAGE_BUGREPORT "nvedula@sfu.ca")

set(CMAKE_CXX_FLAGS "-std=c++11 -O3 -g -fpermissive -Wno-write-strings -Wno-unused-label")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")# -DMY_DEBUG")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Find and include boost 

# include(BoostConfig.cmake)

# set( Boost_NO_BOOST_CMAKE ON)
set(Boost_USE_STATIC_LIBS OFF) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF) 

# /media/vnaveen0/shared/vm-shared/softwares/boost/boost_1_61_0/build
# set (BOOST_ROOT "/media/vnaveen0/shared/vm-shared/softwares/boost/boost_1_61_0/build")
# set(BOOST_INCLUDEDIR /media/vnaveen0/shared/vm-shared/softwares/boost/boost_1_61_0/build/include )
# set(BOOST_LIBRARYDIR /media/vnaveen0/shared/vm-shared/softwares/boost/boost_1_61_0/build/lib)

# find_package(Boost 1.58.0 COMPONENTS graph regex system filesystem program_options iostreams) 
find_package(Boost 1.66.0 COMPONENTS graph regex system filesystem program_options iostreams) 


if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS}) 
else()
    message( FATAL_ERROR "dafaxc simulator needs Boost 1.66.0 or higher, CMake will exit.")
endif()

include_directories(
  ${CMAKE_SOURCE_DIR}/../bloom/include 
  ${CMAKE_SOURCE_DIR}/../bloom/common 
  ${CMAKE_SOURCE_DIR}/../bloom/config 
  ${CMAKE_SOURCE_DIR}/../bloom/common/ioutil
  # /media/vnaveen0/shared/vm-shared/softwares/boost/boost_1_61_0/build/include 

  #------------------------------------------
  ${CMAKE_SOURCE_DIR}/../bloom/common.ruby 
  #------------------------------------------

  )



include_directories(
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/../gems-lib-ooo/ruby_clean/O3sim_ruby
  )

set ( PROJECT_LINK_LIBS libruby.so libbloom.a  z  )
# set ( PROJECT_LINK_LIBS libruby.so  z libbloom.so )

link_directories( 
  ${CMAKE_SOURCE_DIR}/../gems-lib-ooo/ruby_clean/amd64-linux/generated/MESI_CMP_directory_m/lib 
  ${BLOOM_PATH}
  # /media/vnaveen0/shared/vm-shared/softwares/boost/boost_1_61_0/build/lib 
  )

#However, the file(GLOB...) allows for wildcard additions:
file(GLOB SOURCES "src/*.cpp")


# FOR DAFAXC BINARY
# add_executable(dafaxc ${SOURCES})
# target_link_libraries(dafaxc ${Boost_LIBRARIES}  ${PROJECT_LINK_LIBS} )

# FOR BUILDING  A SHARED LIBRARY
add_library(dafaxc SHARED ${SOURCES})
install(TARGETS dafaxc DESTINATION "/home/vnaveen0/wind_drive/sfu/nachos/Nachos/build_repo/dafaxc/bin/")
target_link_libraries(dafaxc ${Boost_LIBRARIES}  ${PROJECT_LINK_LIBS} -lz )



# /usr/share/cmake-3.5/Modules/FindBoost.cmake
