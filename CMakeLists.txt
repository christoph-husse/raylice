cmake_minimum_required(VERSION 2.8)

set(CMAKE_CONFIGURATION_TYPES Release;Debug)

project (Raylice)

include_directories(
	dependencies/
	dependencies/embree/common
	dependencies/embree/rtcore
)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14" )	

IF(APPLE)
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++" )
	set (CMAKE_EXE_LINKER_FLAGS "-stdlib=libc++")
	set (CMAKE_SHARED_LINKER_FLAGS "-stdlib=libc++")
ENDIF()

file(GLOB_RECURSE SOURCES "./src/BSDFMaterial.cpp")

set(CMAKE_OSX_DEPLOYMENT_TARGET "10.7")

add_executable(Raylice WIN32 ${SOURCES})

set(Boost_USE_STATIC_LIBS ON) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF) 
find_package(Boost) 

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})  
    target_link_libraries(Raylice ${Boost_LIBRARIES})
endif()


#target_link_libraries(UnitTests GoogleTest)
