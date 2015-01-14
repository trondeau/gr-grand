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


#ifndef INCLUDED_GRAND_FLOAT_ARRAY_H
#define INCLUDED_GRAND_FLOAT_ARRAY_H

#include <grand/api.h>
#include <gnuradio/sync_block.h>
#include <jni.h>

namespace gr {
  namespace grand {

    /*!
     * \brief <+description of block+>
     * \ingroup grand
     *
     */
    class GRAND_API float_array : virtual public gr::sync_block
    {
     public:
      typedef boost::shared_ptr<float_array> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of grand::float_array.
       *
       * To avoid accidental use of raw pointers, grand::float_array's
       * constructor is in a private implementation
       * class. grand::float_array::make is the public interface for
       * creating new instances.
       */
      static sptr make(jfloatArray array, JNIEnv *env);
    };

  } // namespace grand
} // namespace gr

#endif /* INCLUDED_GRAND_FLOAT_ARRAY_H */
