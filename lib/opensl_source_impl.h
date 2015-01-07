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

#ifndef INCLUDED_GRAND_OPENSL_SOURCE_IMPL_H
#define INCLUDED_GRAND_OPENSL_SOURCE_IMPL_H

#include <grand/opensl_source.h>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

namespace gr {
  namespace grand {

    void recorder_callback(SLAndroidSimpleBufferQueueItf bq, void *context);

    class opensl_source_impl : public opensl_source
    {
     private:
      int d_sample_rate;
      int d_size;
      short *d_buffer;

      SLObjectItf d_engine;
      SLEngineItf d_engine_eng;
      SLObjectItf d_recorder_obj;
      SLRecordItf d_recorder_record;
      SLPlayItf   d_bq_player_play;
      SLAndroidSimpleBufferQueueItf d_recorder_buffer_queue;

      void setup_interface();

     public:
      opensl_source_impl(int sampling_rate);
      ~opensl_source_impl();

      gr::thread::mutex mutex_lock;
      gr::thread::condition_variable condition;
      bool signal;

      void set_sample_rate(int samp_rate);

      // Where all the action really happens
      int work(int noutput_items,
	       gr_vector_const_void_star &input_items,
	       gr_vector_void_star &output_items);
    };

  } // namespace grand
} // namespace gr

#endif /* INCLUDED_GRAND_OPENSL_SOURCE_IMPL_H */
