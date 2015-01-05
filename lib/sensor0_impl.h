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

#ifndef INCLUDED_GRAND_SENSOR0_IMPL_H
#define INCLUDED_GRAND_SENSOR0_IMPL_H

#include <grand/sensor0.h>
#include <android/sensor.h>

namespace gr {
  namespace grand {

    class sensor0_impl : public sensor0
    {
     private:
      ASensorManager* d_manager;
      const ASensor* d_accel;
      ASensorEventQueue* d_event_queue;
      struct android_app* d_state;
      ALooper* d_looper;

     public:
      std::vector<float> d_sensor_x;
      std::vector<float> d_sensor_y;
      std::vector<float> d_sensor_z;
      thread::condition_variable d_condition;
      bool d_cond_is_done;
      thread::mutex d_cond_lock;
      int d_sensor_count;
      int d_condition_trigger;

      sensor0_impl();
      ~sensor0_impl();

      void set_condition(int n) { d_condition_trigger = n; }
      int condition() { return d_condition_trigger; }

      // Where all the action really happens
      int work(int noutput_items,
	       gr_vector_const_void_star &input_items,
	       gr_vector_void_star &output_items);
    };

  } // namespace grand
} // namespace gr

#endif /* INCLUDED_GRAND_SENSOR0_IMPL_H */
