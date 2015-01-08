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


#ifndef INCLUDED_GRAND_SENSOR_BASE_H
#define INCLUDED_GRAND_SENSOR_BASE_H

#include <grand/api.h>
#include <gnuradio/thread/thread.h>
#include <android/sensor.h>

namespace gr {
  namespace grand {

    int get_sensor_event(int fd, int events, void* data);

    /*!
     * \brief <+description of block+>
     * \ingroup grand
     *
     */
    class GRAND_API sensor_base
    {
    protected:
      int d_type;
      int d_num_ports;
      std::string d_type_str;
      std::string d_units;

      // Android sensor manager objects
      ASensorManager* d_manager;
      const ASensor* d_sensor;
      struct android_app* d_state;
      ALooper* d_looper;
      ASensorEventQueue* d_event_queue;

      // Used for signalling between work and callback
      gr::thread::mutex d_mutex_lock;
      gr::thread::condition_variable d_condition;
      bool d_signal;

      void block_on_sensor();

     public:
      gr::thread::mutex* get_lock();
      void set_signal();
      void clear_signal();
      bool signal();
      void notify();

      bool init();
      bool set_sensor_type(int sensor_type);

      sensor_base(void) {}
      sensor_base(int sensor_type);
      ~sensor_base();
    };

  } // namespace grand
} // namespace gr

#endif /* INCLUDED_GRAND_SENSOR0_H */
