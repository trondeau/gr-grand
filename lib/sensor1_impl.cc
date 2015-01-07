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
#include "sensor1_impl.h"

#define MY_LOOPER_ID 3

namespace gr {
  namespace grand {

    int get_sensor1_event(int fd, int events, void* data)
    {
      //GR_INFO("grand::sensor1", "LOOPER CALLBACK CALLED");

      sensor1_impl *p = (sensor1_impl*)data;
      gr::thread::scoped_lock lock(p->mutex_lock);
      p->signal = true;
      p->condition.notify_one();

      return 1;
    }

    sensor1::sptr
    sensor1::make()
    {
      return gnuradio::get_initial_sptr
        (new sensor1_impl());
    }

    sensor1_impl::sensor1_impl()
      : gr::sync_block("sensor1",
                       gr::io_signature::make(0, 0, 0),
                       gr::io_signature::make(1, 1, sizeof(float)))
    {
      signal = false;
      set_max_noutput_items(200);
    }

    sensor1_impl::~sensor1_impl()
    {
    }

    bool
    sensor1_impl::start()
    {
      // Do this in start so that we're in the right thread when we
      // ask for/prepare the looper that will be used during work with
      // the callback function get_sensor1_event.

      int result;
      d_looper = ALooper_forThread();
      GR_INFO("grand::sensor1", boost::str(boost::format("Got looper: %1%") % d_looper));
      if(d_looper == NULL) {
        d_looper = ALooper_prepare(0);
        GR_INFO("grand::sensor1", boost::str(boost::format("    prepared looper: %1%") % d_looper));

        // If still NULL, we have a problem
        if(d_looper == NULL) {
          GR_INFO("grand::sensor1", ("Could not get or prepare a looper"));
          throw std::runtime_error("grand::sensor1");
        }
      }

      // Get singleton to the sensor manager and use to open the light sensor
      d_manager = ASensorManager_getInstance();
      d_light = ASensorManager_getDefaultSensor(d_manager,
                                                ASENSOR_TYPE_LIGHT);
      if(d_light == NULL) {
        GR_INFO("grand::sensor1", "Could not get sensor Light");
        throw std::runtime_error("grand::sensor1");
      }

      // Create an event queue for the sensor
      d_event_queue = ASensorManager_createEventQueue(d_manager,
                                                      d_looper,
                                                      MY_LOOPER_ID,
                                                      get_sensor1_event, this);
      if(d_event_queue == NULL) {
        GR_INFO("grand::sensor1", "Could not create sensor event queue");
        throw std::runtime_error("grand::sensor1");
      }

      // Enable the sensor
      result = ASensorEventQueue_enableSensor(d_event_queue, d_light);
      if(result < 0) {
        GR_INFO("grand::sensor1", "Could not enable sensor Light");
        throw std::runtime_error("grand::sensor1");
      }

      // Get the minimum delay supported by the sensor
      int min_light_delay = ASensor_getMinDelay(d_light);
      GR_INFO("grand::sensor1", boost::str(boost::format("light sensor's min delay %1%") % min_light_delay));

      // Set the event rate to the minimum
      //result = ASensorEventQueue_setEventRate(d_event_queue, d_light, (1000L/100)*10);
      result = ASensorEventQueue_setEventRate(d_event_queue, d_light, min_light_delay);
      if(result < 0) {
        GR_INFO("grand::sensor1", "Could not enable sensor Acclerometer");
        throw std::runtime_error("grand::sensor1");
      }

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
        gr::thread::scoped_lock lock(mutex_lock);
        while(!signal) {
          condition.wait(lock);
        }
        signal = false;

        if(ident == ALOOPER_POLL_CALLBACK) {
          if(d_light != NULL) {
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
