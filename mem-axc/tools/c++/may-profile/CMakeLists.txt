cmake_minimum_required(VERSION 2.8)
project(may_prof)

set(PACKAGE_NAME may_prof)
set(PACKAGE_VERSION 0.1)
set(PACKAGE_STRING "${PACKAGE_NAME} ${PACKAGE_VERSION}")
set(PACKAGE_BUGREPORT "nvedula@sfu.ca")

set(CMAKE_CXX_FLAGS "-std=c++11 -O0 -g -fpermissive -Wno-write-strings -Wno-unused-label")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall") # -DMY_DEBUG")
# set(CMAKE_CXX_COMPILER g++-4.8)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Find and include boost 

set(Boost_USE_STATIC_LIBS OFF) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF) 
find_package(Boost 1.61.0 COMPONENTS graph regex system filesystem program_options iostreams) 

if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS} ) 
else()
    message( FATAL_ERROR "Chains simulator needs Boost 1.61.0 or higher, CMake will exit.")
endif()

# include_directories(
#   ${CMAKE_SOURCE_DIR}/include 
#   )


#However, the file(GLOB...) allows for wildcard additions:
file(GLOB SOURCES "src/*.cpp")

add_executable(may_prof ${SOURCES})
target_link_libraries(may_prof ${Boost_LIBRARIES} z )
