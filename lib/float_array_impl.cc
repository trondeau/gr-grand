/* -*- c++ -*- */
/*
 * Copyright 2015 <+YOU OR YOUR COMPANY+>.
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "float_array_impl.h"

namespace gr {
  namespace grand {

    float_array::sptr
    float_array::make(jfloatArray array, JNIEnv *env)
    {
      return gnuradio::get_initial_sptr
        (new float_array_impl(array, env));
    }

    /*
     * The private constructor
     */
    float_array_impl::float_array_impl(jfloatArray array, JNIEnv* env)
      : gr::sync_block("float_array",
                       gr::io_signature::make(1, 1, sizeof(float)),
                       gr::io_signature::make(0, 0, 0))
    {
      d_array = array;
      d_env = env;
    }

    /*
     * Our virtual destructor.
     */
    float_array_impl::~float_array_impl()
    {
    }

    int
    float_array_impl::work(int noutput_items,
                           gr_vector_const_void_star &input_items,
                           gr_vector_void_star &output_items)
    {
      const float *in = (const float*)input_items[0];

      jfloat* array = d_env->GetFloatArrayElements(d_array, NULL);
      memcpy(array, in, noutput_items*sizeof(float));
      d_env->ReleaseFloatArrayElements(d_array, array, 0);

      // Tell runtime system how many output items we produced.
      return noutput_items;
    }

  } /* namespace grand */
} /* namespace gr */
