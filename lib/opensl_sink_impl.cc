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
#include <volk/volk.h>
#include "opensl_sink_impl.h"

namespace gr {
  namespace grand {

    opensl_sink::sptr
    opensl_sink::make(int sampling_rate)
    {
      return gnuradio::get_initial_sptr
        (new opensl_sink_impl(sampling_rate));
    }

    /*
     * The private constructor
     */
    opensl_sink_impl::opensl_sink_impl(int sampling_rate)
      : gr::sync_block("opensl_sink",
                       gr::io_signature::make(1, 1, sizeof(float)),
                       gr::io_signature::make(0, 0, 0)),
        d_sample_rate(sampling_rate)
    {
      unsigned int bufferframes = 1024;
      d_size = 2048;
      uint32_t channels = 1;
      //int speakers = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
      int speakers = SL_SPEAKER_FRONT_CENTER;
      uint32_t sr = SL_SAMPLINGRATE_32;

      SLresult result;

      result = slCreateEngine(&d_engine, 0, NULL, 0, NULL, NULL);
      if(result != SL_RESULT_SUCCESS) {
        std::string e = boost::str
          (boost::format("Unable to open OpenSLES audio engine"));
        GR_ERROR("grand::audio_sink", e);
        throw std::runtime_error(e);
      }
      (void)result;

      result = (*d_engine)->Realize(d_engine, SL_BOOLEAN_FALSE);
      if(result != SL_RESULT_SUCCESS) {
        std::string e = boost::str
          (boost::format("Unable to realize engine"));
        GR_ERROR("grand::audio_sink", e);
        throw std::runtime_error(e);
      }
      (void)result;


      result = (*d_engine)->GetInterface(d_engine, SL_IID_ENGINE, &d_engine_eng);
      if(result != SL_RESULT_SUCCESS) {
        std::string e = boost::str
          (boost::format("Unable to open OpenSLES engine interface"));
        GR_ERROR("grand::audio_sink", e);
        throw std::runtime_error(e);
      }
      (void)result;

      const SLInterfaceID ids[] = {};
      const SLboolean req[] = {};
      result = (*d_engine_eng)->CreateOutputMix(d_engine_eng,
                                                &d_output_mix_obj, 0, ids, req);
      if(result != SL_RESULT_SUCCESS) {
        std::string e = boost::str
          (boost::format("Unable to create OpenSLES output mix"));
        GR_ERROR("grand::audio_sink", e);
        throw std::runtime_error(e);
      }
      (void)result;

      result = (*d_output_mix_obj)->Realize(d_output_mix_obj,
                                            SL_BOOLEAN_FALSE);
      if(result != SL_RESULT_SUCCESS) {
        std::string e = boost::str
          (boost::format("Unable to realize OpenSLES output mix"));
        GR_ERROR("grand::audio_sink", e);
        throw std::runtime_error(e);
      }
      (void)result;


      SLDataLocator_AndroidSimpleBufferQueue loc_bufq =
        {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
      SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, channels, sr,
                                     SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
                                     speakers, SL_BYTEORDER_LITTLEENDIAN};
      SLDataSource audioSrc = {&loc_bufq, &format_pcm};




      SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX,
                                            d_output_mix_obj};
      SLDataSink audioSnk = {&loc_outmix, NULL};



      const SLInterfaceID ids1[] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE, SL_IID_VOLUME};
      const SLboolean req1[] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
      result = (*d_engine_eng)->CreateAudioPlayer(d_engine_eng,
                                                  &d_bq_player_obj, &audioSrc, &audioSnk,
                                                  2, ids1, req1);
      if(result != SL_RESULT_SUCCESS) {
        std::string e = boost::str
          (boost::format("Unable to create OpenSLES audio player"));
        GR_ERROR("grand::audio_sink", e);
        throw std::runtime_error(e);
      }
      (void)result;

      result = (*d_bq_player_obj)->Realize(d_bq_player_obj,
                                           SL_BOOLEAN_FALSE);
      if(result != SL_RESULT_SUCCESS) {
        std::string e = boost::str
          (boost::format("Unable to realize OpenSLES audio player"));
        GR_ERROR("grand::audio_sink", e);
        throw std::runtime_error(e);
      }
      (void)result;


      result = (*d_bq_player_obj)->GetInterface(d_bq_player_obj,
                                                SL_IID_PLAY,&(d_bq_player_play));
      if(result != SL_RESULT_SUCCESS) {
        std::string e = boost::str
          (boost::format("Unable to get OpenSLES audio player interface"));
        GR_ERROR("grand::audio_sink", e);
        throw std::runtime_error(e);
      }
      (void)result;

      result = (*d_bq_player_obj)->GetInterface(d_bq_player_obj, SL_IID_ANDROIDSIMPLEBUFFERQUEUE,
						&d_bq_player_buffer_queue);
      if(result != SL_RESULT_SUCCESS) {
        std::string e = boost::str
          (boost::format("Unable to get OpenSLES audio player buffer queue"));
        GR_ERROR("grand::audio_sink", e);
        throw std::runtime_error(e);
      }
      (void)result;


      result = (*d_bq_player_buffer_queue)->RegisterCallback(d_bq_player_buffer_queue, queue_callback, this);
      if(result != SL_RESULT_SUCCESS) {
        std::string e = boost::str
          (boost::format("Unable to register OpenSLES buffer queue callback"));
        GR_ERROR("grand::audio_sink", e);
        throw std::runtime_error(e);
      }
      (void)result;

      result = (*d_bq_player_play)->SetPlayState(d_bq_player_play,
                                                 SL_PLAYSTATE_PLAYING);
      if(result != SL_RESULT_SUCCESS) {
        std::string e = boost::str
          (boost::format("Unable to set OpenSLES audio player state to playing"));
        GR_ERROR("grand::audio_sink", e);
        throw std::runtime_error(e);
      }
      (void)result;


      d_buffer = (short*)volk_malloc(d_size, volk_get_alignment());
      if(!d_buffer) {
        //android_CloseAudioDevice(d_dev);
        std::string e = boost::str
          (boost::format("Unable to allocate audio buffer of %1% bytes") \
           % d_size);
        GR_ERROR("grand::audio_sink", e);
        throw std::runtime_error(e);
      }

      set_output_multiple(d_size);

      signal = true;
    }

    opensl_sink_impl::~opensl_sink_impl()
    {
      //android_CloseAudioDevice(d_dev);
    }

    void
    queue_callback(SLAndroidSimpleBufferQueueItf bq, void *context)
    {
      opensl_sink_impl *c = (opensl_sink_impl*)context;
      gr::thread::scoped_lock lock(c->mutex_lock);
      c->signal = true;
      c->condition.notify_one();
    }

    int
    opensl_sink_impl::work(int noutput_items,
                           gr_vector_const_void_star &input_items,
                           gr_vector_void_star &output_items)
    {
      const float *in = (const float*) input_items[0];

      GR_DEBUG("grand::audio_sink", boost::format("noutput_items: %1%") % noutput_items);

      float scale_factor = 16384.0f;
      int count = 0;
      while(count < noutput_items) {
        for(int i = 0; i < d_size; i++) {
          d_buffer[i] = static_cast<short>(in[0]*scale_factor);
          in++;
        }
        count += d_size;

        gr::thread::scoped_lock lock(mutex_lock);
        while(!signal) {
          condition.wait(lock);
        }
        signal = false;

        (*d_bq_player_buffer_queue)->Enqueue(d_bq_player_buffer_queue,
                                             d_buffer, d_size*sizeof(short));
      }


      // Tell runtime system how many output items we produced.
      return noutput_items;
    }

  } /* namespace grand */
} /* namespace gr */
