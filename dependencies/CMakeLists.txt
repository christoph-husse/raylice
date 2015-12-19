# M.I.T. License - http://opensource.org/licenses/mit-license.php/
# 
# Copyright (c) 2012 Christoph Husse
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated 
# documentation files (the "Software"), to deal in the Software without restriction, including without limitation 
# the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, 
# and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED 
# TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL 
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF 
# CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
# DEALINGS IN THE SOFTWARE.
#  

cmake_minimum_required (VERSION 2.8)
cmake_policy(VERSION 2.8)

default_source_scan("${SRCDIR}dependencies/")

add_library(RayliceDepend STATIC ${sources} ${headers})

require_special_directory(BOOST_ROOT "Path to the root directory of BOOST.")

include_directories("${CMAKE_SOURCE_DIR}/dependencies/embree/common")
include_directories("${CMAKE_SOURCE_DIR}/dependencies/embree/rtcore")
include_directories("${CMAKE_SOURCE_DIR}/dependencies/openexr")
include_directories("${CMAKE_SOURCE_DIR}/dependencies/vectormathlibrary/include")
include_directories("${BOOST_ROOT}")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	target_link_libraries(RayliceDepend opengl32.lib winmm.lib)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
else()
	target_link_libraries(RayliceDepend -lGL -lX11)


	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14" )	

	IF(APPLE)
		set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++" )
		set (CMAKE_EXE_LINKER_FLAGS "-stdlib=libc++")
		set (CMAKE_SHARED_LINKER_FLAGS "-stdlib=libc++")
	ENDIF()

endif()

add_definitions(-DFREEGLUT_STATIC)

make_project(RayliceDepend)

