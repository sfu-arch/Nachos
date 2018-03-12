cmake_minimum_required(VERSION 2.8)
project(bloom)

set(PACKAGE_NAME bloom)
set(PACKAGE_VERSION 0.1)
set(PACKAGE_STRING "${PACKAGE_NAME} ${PACKAGE_VERSION}")
set(PACKAGE_BUGREPORT "nvedula@sfu.ca")

set(CMAKE_CXX_FLAGS "-std=c++11 -O3 -fpermissive -Wno-write-strings -Wno-unused-label")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall" ) # -DMY_DEBUG")
# set(CMAKE_CXX_COMPILER g++-4.8)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Find and include boost 

  include_directories( ${CMAKE_SOURCE_DIR}/include 
    ${CMAKE_SOURCE_DIR}../gems-lib-ooo/ruby_clean/O3sim_ruby)

set ( PROJECT_LINK_LIBS libruby.so )
link_directories(
  ${CMAKE_SOURCE_DIR}/../gems-lib-ooo/ruby_clean/amd64-linux/generated/MESI_CMP_directory_m/lib
  )

include_directories(
  ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/common 
  ${CMAKE_SOURCE_DIR}/common.ruby 
  ${CMAKE_SOURCE_DIR}/config 
  ${CMAKE_SOURCE_DIR}/common/ioutil
  )

file(GLOB SOURCES "src/*.C")

# add_executable(bloom ${SOURCES})
add_library(bloom STATIC ${SOURCES})
# add_library(bloom SHARED ${SOURCES})
install(TARGETS bloom DESTINATION ${CMAKE_SOURCE_DIR}/build)

target_link_libraries(bloom  ${PROJECT_LINK_LIBS} -lz )
