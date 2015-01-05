/* -*- c++ -*- */

#define GRAND_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "grand_swig_doc.i"

%{
#include "grand/sensor0.h"
%}


%include "grand/sensor0.h"
GR_SWIG_BLOCK_MAGIC2(grand, sensor0);
