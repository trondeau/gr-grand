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

#include <android/log.h>
//#include <android_native_app_glue.h>

#define LOOPER_ID_USER 3

namespace gr {
  namespace grand {

    int keepgoing;
    ASensorEventQueue* global_event_queue;

    //typedef int (*ALooper_callbackFunc)(int fd, int events, void* data);

    int get_looper(int fd, int events, void* data)
    {
      GR_INFO("grand::sensor0", " -----> LOOPER CALLBACK CALLED");

      ASensorEvent event;
      sensor0_impl *s = (sensor0_impl*)data;

      //while(ASensorEventQueue_getEvents(global_event_queue, &event, 1) > 0) {
      //  if(event.type==ASENSOR_TYPE_ACCELEROMETER) {
      //    LOGI("accl(x,y,z,t): %f %f %f %lld",
      //         event.acceleration.x, event.acceleration.y,
      //         event.acceleration.z, event.timestamp);
      //    s->d_sensor_x.push_back(event.acceleration.x);
      //    s->d_sensor_y.push_back(event.acceleration.y);
      //    s->d_sensor_z.push_back(event.acceleration.z);
      //    s->d_sensor_count++;
      //  }
      //}

      std::string str = boost::str(boost::format("sensor count: %1%   condition: %2%") \
                                   % (s->d_sensor_count) % (s->condition()));
      GR_INFO("grand::sensor0", str);

      //if(s->d_sensor_count == s->condition()) {
      //  //boost::mutex::scoped_lock lock(s->d_cond_lock);
      //  s->d_cond_is_done = true;
      //  s->d_condition.notify_one();
      //}

      // 1 to continue receiving callbacks; 0 to unregister
      return keepgoing;
    }

    sensor0::sptr
    sensor0::make()
    {
      return gnuradio::get_initial_sptr
        (new sensor0_impl());
    }

    /*
     * The private constructor
     */
    sensor0_impl::sensor0_impl()
      : gr::sync_block("sensor0",
                       gr::io_signature::make(0, 0, 0),
                       gr::io_signature::make(3, 3, sizeof(float)))
    {
      // REMEMBER REMEMBER:
      // android_native_app_glue

//      d_looper = ALooper_forThread();
//      if(d_looper == NULL) {
//        LOGI("Could not get looper; preparing one");
//        d_looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
//      }
//
//      d_sensor_count = 0;
//      d_cond_is_done = false;
//      set_condition(-1);
//
//      d_manager = ASensorManager_getInstance();
//      d_accel = ASensorManager_getDefaultSensor(d_manager, ASENSOR_TYPE_ACCELEROMETER);
//      d_event_queue = ASensorManager_createEventQueue(d_manager, d_looper,
//                                                      LOOPER_ID_USER, get_looper, this);
//
//      keepgoing = 1;
//      global_event_queue = d_event_queue;
//
//      ASensorEventQueue_enableSensor(d_event_queue, d_accel);
//      ASensorEventQueue_setEventRate(d_event_queue, d_accel, (1000L/10)*1000);

      //set_max_noutput_items(1000);
    }

    /*
     * Our virtual destructor.
     */
    sensor0_impl::~sensor0_impl()
    {
      GR_INFO("grand::sensor0", "Tearing down sensor0");
      keepgoing = 0;
    }

    int
    sensor0_impl::work(int noutput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
      float *outx = (float*)output_items[0];
      float *outy = (float*)output_items[1];
      float *outz = (float*)output_items[2];

      set_condition(noutput_items);

      //boost::mutex::scoped_lock lock(d_cond_lock);
      //while(!d_cond_is_done) {
      //  LOGI("waiting...");
      //  d_condition.wait(lock);
      //  LOGI("... done waiting");
      //}

      //if(d_sensor_count == condition()) {
      //  memcpy(outx, (void*)&d_sensor_x[0], noutput_items*sizeof(float));
      //  memcpy(outy, (void*)&d_sensor_y[0], noutput_items*sizeof(float));
      //  memcpy(outz, (void*)&d_sensor_z[0], noutput_items*sizeof(float));
      //  d_sensor_count = 0;
      //  LOGI("  returning: %d", condition());
      //  return condition();
      //}
      //else {
      //  sleep(1);
      //  LOGI("  returning 0");
      //  return 0;
      //}

      d_looper = ALooper_forThread();
      if(d_looper == NULL) {
        GR_INFO("grand::sensor0", "Could not get looper; preparing one");
        d_looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
      }

      d_sensor_count = 0;
      d_cond_is_done = false;
      set_condition(-1);

      d_manager = ASensorManager_getInstance();
      d_accel = ASensorManager_getDefaultSensor(d_manager, ASENSOR_TYPE_ACCELEROMETER);
      d_event_queue = ASensorManager_createEventQueue(d_manager, d_looper,
                                                      LOOPER_ID_USER, NULL, NULL);
      ASensorEventQueue_enableSensor(d_event_queue, d_accel);
      ASensorEventQueue_setEventRate(d_event_queue, d_accel, (1000L/100)*1000);

      int ident = 0, events = 0;
      struct android_poll_source *source;
//
//      for(int i = 0; i < noutput_items; i++) {
//        ASensorEvent event;
//        while(ASensorEventQueue_getEvents(d_event_queue, &event, 1) > 0) {
//          LOGI("accelerometer: x=%f y=%f z=%f",
//               event.acceleration.x, event.acceleration.y,
//               event.acceleration.z);
//          outx[i] = event.acceleration.x;
//          outy[i] = event.acceleration.y;
//          outz[i] = event.acceleration.z;
//        }
//      }


      for(int i = 0; i < noutput_items; i++) {
        ident = ALooper_pollOnce(-1, NULL, &events, (void**)&source);

        //// Process this event.
        //if(source != NULL) {
        //  source->process(d_state, source);
        //}

        // If a sensor has data, process it now.
        if(ident == LOOPER_ID_USER) {
          if(d_accel != NULL) {
            ASensorEvent event;
            if(ASensorEventQueue_getEvents(d_event_queue,
                                           &event, 1) > 0) {
              //LOGI("accelerometer: x=%f y=%f z=%f",
              //     event.acceleration.x, event.acceleration.y,
              //     event.acceleration.z);
              outx[i] = event.acceleration.x;
              outy[i] = event.acceleration.y;
              outz[i] = event.acceleration.z;
            }
          }
        }
      }


      // Tell runtime system how many output items we produced.
      GR_INFO("grand::sensor0", boost::format("returning: %1%") % noutput_items);
      return noutput_items;
    }

  } /* namespace grand */
} /* namespace gr */
