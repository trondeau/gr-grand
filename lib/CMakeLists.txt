# Copyright 2011,2012 Free Software Foundation, Inc.
#
# This file is part of GNU Radio
#
# GNU Radio is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# GNU Radio is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with GNU Radio; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.

########################################################################
# Setup library
########################################################################
include(GrPlatform) #define LIB_SUFFIX

include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIRS})

list(APPEND grand_sources
    sensor_base.cc
    accelerometer_impl.cc
    light_sensor_impl.cc
    opensl_sink_impl.cc
    opensl_source_impl.cc
    float_array_impl.cc
)

list(APPEND GRAND_LIBS
  ${Boost_LIBRARIES}
  ${GNURADIO_ALL_LIBRARIES}
  -llog
  -lOpenSLES
  -landroid
)

set(grand_sources "${grand_sources}" PARENT_SCOPE)
if(NOT grand_sources)
	MESSAGE(STATUS "No C++ sources... skipping lib/")
	return()
endif(NOT grand_sources)

add_library(gnuradio-grand SHARED ${grand_sources})
target_link_libraries(gnuradio-grand ${GRAND_LIBS})
set_target_properties(gnuradio-grand PROPERTIES DEFINE_SYMBOL "gnuradio_grand_EXPORTS")

if(APPLE)
    set_target_properties(gnuradio-grand PROPERTIES
        INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib"
    )
endif(APPLE)


add_library(gnuradio-grand_static STATIC ${grand_sources})

add_dependencies(gnuradio-grand_static
  gnuradio-runtime_static)

if(NOT WIN32)
  set_target_properties(gnuradio-grand_static
    PROPERTIES OUTPUT_NAME gnuradio-grand)
endif(NOT WIN32)

install(TARGETS gnuradio-grand_static
  ARCHIVE DESTINATION lib${LIB_SUFFIX} COMPONENT "grand_devel"   # .lib file
  )



########################################################################
# Install built library files
########################################################################
install(TARGETS gnuradio-grand
    LIBRARY DESTINATION lib${LIB_SUFFIX} # .so/.dylib file
    ARCHIVE DESTINATION lib${LIB_SUFFIX} # .lib file
    RUNTIME DESTINATION bin              # .dll file
)

########################################################################
# Build and register unit test
########################################################################
#include(GrTest)
#
#include_directories(${CPPUNIT_INCLUDE_DIRS})
#
#list(APPEND test_grand_sources
#    ${CMAKE_CURRENT_SOURCE_DIR}/test_grand.cc
#    ${CMAKE_CURRENT_SOURCE_DIR}/qa_grand.cc
#)
#
#add_executable(test-grand ${test_grand_sources})
#
#target_link_libraries(
#  test-grand
#  ${GNURADIO_RUNTIME_LIBRARIES}
#  ${Boost_LIBRARIES}
#  ${CPPUNIT_LIBRARIES}
#  gnuradio-grand
#)
#
#GR_ADD_TEST(test_grand test-grand)
