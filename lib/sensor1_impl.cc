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
#include <gnuradio/logger.h>
#include "sensor1_impl.h"

namespace gr {
  namespace grand {

    sensor1::sptr
    sensor1::make()
    {
      return gnuradio::get_initial_sptr
        (new sensor1_impl());
    }

    sensor1_impl::sensor1_impl()
      : gr::sync_block("sensor1",
                       gr::io_signature::make(0, 0, 0),
                       gr::io_signature::make(1, 1, sizeof(float))),
        gr::grand::sensor_base(ASENSOR_TYPE_LIGHT)
    {
      set_max_noutput_items(200);
    }

    sensor1_impl::~sensor1_impl()
    {
    }

    bool
    sensor1_impl::start()
    {
      init();
      return sync_block::start();
    }

    int
    sensor1_impl::work(int noutput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
      float *out = (float*) output_items[0];

      //GR_INFO("grand::sensor1", boost::str(boost::format("entered: %1%") % noutput_items));

      for(int i = 0; i < noutput_items; i++) {
        int ident = ALooper_pollOnce(-1, NULL, NULL, NULL);
        //GR_INFO("grand::sensor1", boost::str(boost::format("LOOPER POLLED, ret: %1%") % ident));

        // Wait for callback to signal us
        block_on_sensor();

        if(ident == ALOOPER_POLL_CALLBACK) {
          if(d_sensor != NULL) {
            ASensorEvent event;
            if(ASensorEventQueue_getEvents(d_event_queue, &event, 1) > 0) {
              out[i] = event.light;
            }
          }
        }
      }

      //GR_INFO("grand::sensor1", boost::str(boost::format("ret: %1% -> %2%") \
      //                                     % noutput_items % (out[0])));
      return noutput_items;
    }

  } /* namespace grand */
} /* namespace gr */
