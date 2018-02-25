cmake_minimum_required(VERSION 2.8)
project(dafaxc)

set(PACKAGE_NAME dafaxc)
set(PACKAGE_VERSION 0.1)
set(PACKAGE_STRING "${PACKAGE_NAME} ${PACKAGE_VERSION}")
set(PACKAGE_BUGREPORT "nvedula@sfu.ca")

set(CMAKE_CXX_FLAGS "-std=c++11 -O0 -fpermissive -Wno-write-strings -Wno-unused-label")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")
# set(CMAKE_CXX_COMPILER g++-4.8)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Find and include boost 

set(Boost_USE_STATIC_LIBS OFF) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF) 
# set (CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "/home/nvedula/software/boost_1_61_0-install/include")
# set (CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "/home/nvedula/software/boost_1_61_0-install/lib")

find_package(Boost 1.61.0 COMPONENTS graph regex system filesystem program_options) 
# find_package(Boost  COMPONENTS graph regex system filesystem program_options) 

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS}) 
else()
    message( FATAL_ERROR "dafaxc simulator needs Boost 1.61.0 or higher, CMake will exit.")
endif()

include_directories(${CMAKE_SOURCE_DIR}/include)

include_directories( ${CMAKE_SOURCE_DIR}/../macsim-mem-axc-64/gems-lib-ooo/ruby_clean/O3sim_ruby) 
set ( PROJECT_LINK_LIBS libruby.so )
link_directories( ${CMAKE_SOURCE_DIR}/../macsim-mem-axc-64/gems-lib-ooo/ruby_clean/amd64-linux/generated/MESI_CMP_directory_m/lib )


# target_link_libraries(dafaxc ${Boost_LIBRARIES} z)
#However, the file(GLOB...) allows for wildcard additions:
file(GLOB SOURCES "src/*.cpp")

# add_executable(dafaxc ${SOURCES})
add_library(dafaxc SHARED ${SOURCES})
install(TARGETS dafaxc DESTINATION "/home/nvedula/summer2016/mem-axc-64/macsim.r.d.b.s/build-dafaxc")
target_link_libraries(dafaxc ${Boost_LIBRARIES}  ${PROJECT_LINK_LIBS} -lz )
