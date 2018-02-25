cmake_minimum_required(VERSION 2.8)
project(mem-axc)

set(PACKAGE_NAME mem-axc)
set(PACKAGE_VERSION 0.1)
set(PACKAGE_STRING "${PACKAGE_NAME} ${PACKAGE_VERSION}")
set(PACKAGE_BUGREPORT "nvedula@sfu.ca")

set(CMAKE_CXX_FLAGS "-std=c++11 -O3 -fpermissive -Wno-write-strings -Wno-unused-label")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -g -O3")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

#TODO:
#Call shell command : make -C knobs/
#Call shell command to build ruby
add_custom_command(OUTPUT make-knob  COMMAND make -C knobs/  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}  )

# Include Directories ------------------------------------------------ 
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/gems-lib-ooo/ruby_clean/O3sim_ruby )
include_directories(${CMAKE_SOURCE_DIR}/../dafaxc/include/)
# Find and include boost 

set(Boost_USE_STATIC_LIBS OFF) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF) 
# set (CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "/home/nvedula/software/boost_1_61_0-install/include")
# set (CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "/home/nvedula/software/boost_1_61_0-install/lib")

find_package(Boost 1.61.0 COMPONENTS graph regex system filesystem program_options) 
# find_package(Boost  COMPONENTS graph regex system filesystem program_options) 

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS} ) 
else()
    message( FATAL_ERROR "dafaxc simulator needs Boost 1.61.0 or higher, CMake will exit.")
endif()

# LINK DIRECTORIES------------------------------------------------ 

link_directories(${CMAKE_SOURCE_DIR}/gems-lib-ooo/ruby_clean/amd64-linux/generated/MESI_CMP_directory_m/lib)
link_directories(${CMAKE_SOURCE_DIR}/../dafaxc/build)

#------------------------------------------------ 

add_subdirectory(${CMAKE_SOURCE_DIR}/src)
# add_subdirectory(def)



