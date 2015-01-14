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

#ifndef INCLUDED_GRAND_FLOAT_ARRAY_IMPL_H
#define INCLUDED_GRAND_FLOAT_ARRAY_IMPL_H

#include <grand/float_array.h>

namespace gr {
  namespace grand {

    class float_array_impl : public float_array
    {
     private:
      jfloatArray d_array;
      JNIEnv *d_env;

     public:
      float_array_impl(jfloatArray array, JNIEnv *env);
      ~float_array_impl();

      // Where all the action really happens
      int work(int noutput_items,
	       gr_vector_const_void_star &input_items,
	       gr_vector_void_star &output_items);
    };

  } // namespace grand
} // namespace gr

#endif /* INCLUDED_GRAND_FLOAT_ARRAY_IMPL_H */
