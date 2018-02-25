file(GLOB SOURCES *.cc *.cpp)                
add_executable(ooo-sim ${SOURCES})
target_link_libraries(ooo-sim ${Boost_LIBRARIES} dafaxc ruby z)
