/* -*- c++ -*- */

#define GRAND_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "grand_swig_doc.i"

%{
#include "grand/accelerometer.h"
#include "grand/opensl_source.h"
#include "grand/light_sensor.h"
#include "grand/float_array.h"
%}


%include "grand/accelerometer.h"
GR_SWIG_BLOCK_MAGIC2(grand, accelerometer);
%include "grand/opensl_source.h"
GR_SWIG_BLOCK_MAGIC2(grand, opensl_source);
%include "grand/light_sensor.h"
GR_SWIG_BLOCK_MAGIC2(grand, light_sensor);
%include "grand/float_array.h"
GR_SWIG_BLOCK_MAGIC2(grand, float_array);
