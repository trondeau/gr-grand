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

#ifndef INCLUDED_GRAND_SENSOR1_IMPL_H
#define INCLUDED_GRAND_SENSOR1_IMPL_H

#include <grand/sensor1.h>
#include <android/sensor.h>

namespace gr {
  namespace grand {

    int get_sensor1_event(int fd, int events, void* data);

    class sensor1_impl : public sensor1
    {
     private:
      ASensorManager* d_manager;
      const ASensor* d_light;
      struct android_app* d_state;
      ALooper* d_looper;

      bool start();

     public:
      ASensorEventQueue* d_event_queue;

      sensor1_impl();
      ~sensor1_impl();

      gr::thread::mutex mutex_lock;
      gr::thread::condition_variable condition;
      bool signal;

      int work(int noutput_items,
	       gr_vector_const_void_star &input_items,
	       gr_vector_void_star &output_items);
    };

  } // namespace grand
} // namespace gr

#endif /* INCLUDED_GRAND_SENSOR1_IMPL_H */
