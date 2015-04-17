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
     * \brief Fills an array created in the Java app with samples
     * passed into this block from the flowgraph.
     * \ingroup grand
     */
    class GRAND_API float_array : virtual public gr::sync_block
    {
     public:
      typedef boost::shared_ptr<float_array> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of
       * grand::float_array.
       *
       * \param array A global reference to the array from the Java
       *  app. Use (jfloatArray)env->NewGlobalRef([array]).
       * \param len Length of \p array.
       * \param vm Pointer to the Java Virtual Machine, from which we
       *  will extract the JNI Environment from for the work thread.
       */
      static sptr make(jfloatArray array, int len, JavaVM *vm);

      /*!
       * Set the internal array to a new value. Make sure that this is
       * a global reference to the Java array.
       *
       * \param array A global reference to the array from the Java
       *  app. Use (jfloatArray)env->NewGlobalRef([array]).
       * \param len Length of \p array.
       */
      virtual void set_array(jfloatArray array, int len) = 0;
    };

  } // namespace grand
} // namespace gr

#endif /* INCLUDED_GRAND_FLOAT_ARRAY_H */
