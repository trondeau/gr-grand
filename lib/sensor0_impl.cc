/* -*- c++ -*- */
/*
 * Copyright 2014 <+YOU OR YOUR COMPANY+>.
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
#include <gnuradio/logger.h>
#include "sensor0_impl.h"

namespace gr {
  namespace grand {

    sensor0::sptr
    sensor0::make()
    {
      return gnuradio::get_initial_sptr
        (new sensor0_impl());
    }

    sensor0_impl::sensor0_impl()
      : gr::sync_block("sensor0",
                       gr::io_signature::make(0, 0, 0),
                       gr::io_signature::make(3, 3, sizeof(float))),
        gr::grand::sensor_base(ASENSOR_TYPE_ACCELEROMETER)
    {
      set_max_noutput_items(200);
    }

    sensor0_impl::~sensor0_impl()
    {
    }

    bool
    sensor0_impl::start()
    {
      init();
      return sync_block::start();
    }

    int
    sensor0_impl::work(int noutput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
      float *outx = (float*) output_items[0];
      float *outy = (float*) output_items[1];
      float *outz = (float*) output_items[2];

      //GR_INFO("grand::sensor0", boost::str(boost::format("entered: %1%") % noutput_items));

      for(int i = 0; i < noutput_items; i++) {
        int ident = ALooper_pollOnce(-1, NULL, NULL, NULL);
        //GR_INFO("grand::sensor0", boost::str(boost::format("LOOPER POLLED, ret: %1%") % ident));

        // Wait for callback to signal us
        block_on_sensor();

        if(ident == ALOOPER_POLL_CALLBACK) {
          if(d_sensor != NULL) {
            ASensorEvent event;
            if(ASensorEventQueue_getEvents(d_event_queue, &event, 1) > 0) {
              //GR_INFO("grand::sensor0", boost::str(boost::format("GETTING DATA: %1%") % event.acceleration.x));
              outx[i] = event.acceleration.x;
              outy[i] = event.acceleration.y;
              outz[i] = event.acceleration.z;
            }
          }
        }
      }

      //GR_INFO("grand::sensor0", boost::str(boost::format("ret: %1% -> %2%") \
      //                                     % noutput_items % (outx[0])));
      return noutput_items;
    }


  } /* namespace grand */
} /* namespace gr */
