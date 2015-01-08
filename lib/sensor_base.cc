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

#include <gnuradio/logger.h>
#include <grand/sensor_base.h>

#define MY_LOOPER_ID 3

namespace gr {
  namespace grand {

    int get_sensor_event(int fd, int events, void* data)
    {
      //GR_INFO("grand::sensor_base", "LOOPER CALLBACK CALLED");

      sensor_base *p = (sensor_base*)data;
      gr::thread::scoped_lock l(*p->get_lock());
      p->set_signal();
      p->notify();

      return 1;
    }

    sensor_base::sensor_base(int sensor_type)
    {
      d_signal = false;
      d_num_ports = 0;
      d_units = "";
      bool ret = set_sensor_type(sensor_type);
      if(!ret) {
        GR_ERROR("grand::sensor_base",
                 boost::str(boost::format("Failed to set sensor %1%") % sensor_type));
        throw std::runtime_error("grand::sensor_base");
      }
    }

    sensor_base::~sensor_base()
    {
    }

    gr::thread::mutex*
    sensor_base::get_lock()
    {
      return &d_mutex_lock;
    }

    inline void
    sensor_base::set_signal()
    {
      d_signal = true;
    }

    inline void
    sensor_base::clear_signal()
    {
      d_signal = false;
    }

    inline bool
    sensor_base::signal()
    {
      return d_signal;
    }

    inline void
    sensor_base::notify()
    {
      d_condition.notify_one();
    }

    bool
    sensor_base::set_sensor_type(int sensor_type)
    {
      // Check if valid sensor type. See android/sensor.h.
      switch(sensor_type) {
      case(ASENSOR_TYPE_ACCELEROMETER):
        d_type_str = "accelerometer";
        d_num_ports = 3;
        d_units = "m/s^2";
        break;
      case(ASENSOR_TYPE_LIGHT):
        d_type_str = "light"; break;
        d_num_ports = 1;
        d_units = "lux";
        break;
      case(ASENSOR_TYPE_MAGNETIC_FIELD):
        d_type_str = "magnetic_field";
        d_num_ports = 3;
        d_units = "uT";
        break;
      case(ASENSOR_TYPE_GYROSCOPE):
        d_type_str = "gyroscope";
        d_num_ports = 3;
        d_units = "rads/sec";
        break;
      case(ASENSOR_TYPE_PROXIMITY):
        d_type_str = "proximity";
        d_num_ports = 1;
        d_units = "on/off";
        break;
      default:
        d_type_str = "invalid";
        return false;
      }

      d_type = sensor_type;
      return true;
    }

    bool
    sensor_base::init()
    {
      // Do this in start so that we're in the right thread when we
      // ask for/prepare the looper that will be used during work with
      // the callback function get_sensor_base_event.

      int result;
      d_looper = ALooper_forThread();
      GR_INFO("grand::sensor_base", boost::str(boost::format("Got looper: %1%") % d_looper));
      if(d_looper == NULL) {
        d_looper = ALooper_prepare(0);
        GR_INFO("grand::sensor_base", boost::str(boost::format("    prepared looper: %1%") % d_looper));

        // If still NULL, we have a problem
        if(d_looper == NULL) {
          GR_INFO("grand::sensor_base", ("Could not get or prepare a looper"));
          throw std::runtime_error("grand::sensor_base");
        }
      }

      // Get singleton to the sensor manager and use to open the accelerometer
      d_manager = ASensorManager_getInstance();
      d_sensor = ASensorManager_getDefaultSensor(d_manager, d_type);
      if(d_sensor == NULL) {
        GR_INFO("grand::sensor_base",
                boost::str(boost::format("Could not get sensor %1%") % d_type_str));
        throw std::runtime_error("grand::sensor_base");
      }

      // Create an event queue for the sensor
      d_event_queue = ASensorManager_createEventQueue(d_manager,
                                                      d_looper,
                                                      MY_LOOPER_ID,
                                                      get_sensor_event, this);
      if(d_event_queue == NULL) {
        GR_INFO("grand::sensor_base", "Could not create sensor event queue");
        throw std::runtime_error("grand::sensor_base");
      }

      // Enable the sensor
      result = ASensorEventQueue_enableSensor(d_event_queue, d_sensor);
      if(result < 0) {
        GR_INFO("grand::sensor_base",
                boost::str(boost::format("Could not enable sensor %1%") % d_type_str));
        throw std::runtime_error("grand::sensor_base");
      }

      // Get the minimum delay supported by the sensor
      int min_accel_delay = ASensor_getMinDelay(d_sensor);
      GR_INFO("grand::sensor_base",
              boost::str(boost::format("%1%'s min delay %2%") % d_type_str % min_accel_delay));

      // Set the event rate to the minimum
      //result = ASensorEventQueue_setEventRate(d_event_queue, d_sensor, (1000L/100)*10);
      result = ASensorEventQueue_setEventRate(d_event_queue, d_sensor, min_accel_delay);
      if(result < 0) {
        GR_INFO("grand::sensor_base",
                boost::str(boost::format("Could not enable sensor %1%") % d_type_str));
        throw std::runtime_error("grand::sensor_base");
      }
    }


    void
    sensor_base::block_on_sensor()
    {
      gr::thread::scoped_lock l(*get_lock());
      while(!signal()) {
        d_condition.wait(l);
      }
      clear_signal();
    }


  } /* namespace grand */
} /* namespace gr */
