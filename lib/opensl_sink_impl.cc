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

    opensl_sink_impl::opensl_sink_impl(int sampling_rate)
      : gr::sync_block("opensl_sink",
                       gr::io_signature::make(1, 1, sizeof(float)),
                       gr::io_signature::make(0, 0, 0))
    {
      set_sample_rate(sampling_rate);
      d_size = 16384;
      d_once = false;
      signal = true;

      setup_interface();

      d_buffer = (short*)volk_malloc(d_size*sizeof(short), volk_get_alignment());
      if(!d_buffer) {
        std::string e = boost::str
          (boost::format("Unable to allocate audio buffer of %1% bytes") \
           % (d_size*sizeof(short)));
        GR_ERROR("grand::audio_sink", e);
        throw std::runtime_error("grand::audio_sink");
      }

      set_output_multiple(d_size);
    }

    opensl_sink_impl::~opensl_sink_impl()
    {
      volk_free(d_buffer);
    }

    void
    opensl_sink_impl::set_sample_rate(int samp_rate)
    {
      switch(samp_rate) {
      case 8000:   d_sample_rate = SL_SAMPLINGRATE_8; break;
      case 11025:  d_sample_rate = SL_SAMPLINGRATE_11_025; break;
      case 16000:  d_sample_rate = SL_SAMPLINGRATE_16; break;
      case 22050:  d_sample_rate = SL_SAMPLINGRATE_22_05;  break;
      case 24000:  d_sample_rate = SL_SAMPLINGRATE_24; break;
      case 32000:  d_sample_rate = SL_SAMPLINGRATE_32; break;
      case 44100:  d_sample_rate = SL_SAMPLINGRATE_44_1; break;
      case 48000:  d_sample_rate = SL_SAMPLINGRATE_48;  break;
      case 64000:  d_sample_rate = SL_SAMPLINGRATE_64; break;
      case 88200:  d_sample_rate = SL_SAMPLINGRATE_88_2; break;
      case 96000:  d_sample_rate = SL_SAMPLINGRATE_96; break;
      case 192000: d_sample_rate = SL_SAMPLINGRATE_192; break;
      default:
        std::string e = boost::str(boost::format("Invalid sample rateL %1%") % samp_rate);
        GR_ERROR("grand::audio_source", e);
        throw std::runtime_error("grand::audio_source");
      }
    }

    void
    opensl_sink_impl::setup_interface()
    {
      SLresult result;
      uint32_t channels = 1;
      int speakers;
      if(channels > 1)
        speakers = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
      else
        speakers = SL_SPEAKER_FRONT_CENTER;

      // Open audio engine
      result = slCreateEngine(&d_engine, 0, NULL, 0, NULL, NULL);
      if(result != SL_RESULT_SUCCESS) {
        GR_ERROR("grand::audio_sink", "Unable to open OpenSLES audio engine");
        throw std::runtime_error("grand::audio_sink");
      }
      (void)result;

      // Realize audio engine
      result = (*d_engine)->Realize(d_engine, SL_BOOLEAN_FALSE);
      if(result != SL_RESULT_SUCCESS) {
        GR_ERROR("grand::audio_sink", "Unable to realize engine");
        throw std::runtime_error("grand::audio_sink");
      }
      (void)result;

      // Get audio engine interface
      result = (*d_engine)->GetInterface(d_engine, SL_IID_ENGINE, &d_engine_eng);
      if(result != SL_RESULT_SUCCESS) {
        GR_ERROR("grand::audio_sink", "Unable to open OpenSLES engine interface");
        throw std::runtime_error("grand::audio_sink");
      }
      (void)result;

      // Create output mixer object
      const SLInterfaceID ids[] = {};
      const SLboolean req[] = {};
      result = (*d_engine_eng)->CreateOutputMix(d_engine_eng,
                                                &d_output_mix_obj, 0, ids, req);
      if(result != SL_RESULT_SUCCESS) {
        GR_ERROR("grand::audio_sink", "Unable to create OpenSLES output mix");
        throw std::runtime_error("grand::audio_sink");
      }
      (void)result;

      // Realize output mixer
      result = (*d_output_mix_obj)->Realize(d_output_mix_obj,
                                            SL_BOOLEAN_FALSE);
      if(result != SL_RESULT_SUCCESS) {
        GR_ERROR("grand::audio_sink", "Unable to realize OpenSLES output mix");
        throw std::runtime_error("grand::audio_sink");
      }
      (void)result;


      // Audio source and sink configurations
      SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
      SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, channels, d_sample_rate,
                                     SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
                                     speakers, SL_BYTEORDER_LITTLEENDIAN};
      SLDataSource audioSrc = {&loc_bufq, &format_pcm};

      SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX,
                                            d_output_mix_obj};
      SLDataSink audioSnk = {&loc_outmix, NULL};

      // Create audio player (set the CAPTURE_AUDIO_OUTPUT permission)
      const SLInterfaceID ids1[] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE, SL_IID_VOLUME};
      const SLboolean req1[] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
      result = (*d_engine_eng)->CreateAudioPlayer(d_engine_eng,
                                                  &d_bq_player_obj, &audioSrc, &audioSnk,
                                                  2, ids1, req1);
      if(result != SL_RESULT_SUCCESS) {
        GR_ERROR("grand::audio_sink", "Unable to create OpenSLES audio player");
        throw std::runtime_error("grand::audio_sink");
      }
      (void)result;

      // Realize audio player
      result = (*d_bq_player_obj)->Realize(d_bq_player_obj, SL_BOOLEAN_FALSE);
      if(result != SL_RESULT_SUCCESS) {
        GR_ERROR("grand::audio_sink", "Unable to realize OpenSLES audio player");
        throw std::runtime_error("grand::audio_sink");
      }
      (void)result;

      // Get interface to audio player
      result = (*d_bq_player_obj)->GetInterface(d_bq_player_obj,
                                                SL_IID_PLAY,&(d_bq_player_play));
      if(result != SL_RESULT_SUCCESS) {
        GR_ERROR("grand::audio_sink", "Unable to get OpenSLES audio player interface");
        throw std::runtime_error("grand::audio_sink");
      }
      (void)result;

      // Get interface to the buffer queue
      result = (*d_bq_player_obj)->GetInterface(d_bq_player_obj, SL_IID_ANDROIDSIMPLEBUFFERQUEUE,
						&d_bq_player_buffer_queue);
      if(result != SL_RESULT_SUCCESS) {
        GR_ERROR("grand::audio_sink", "Unable to get OpenSLES audio player buffer queue");
        throw std::runtime_error("grand::audio_sink");
      }
      (void)result;

      // Set callback function, called when done writing output
      result = (*d_bq_player_buffer_queue)->RegisterCallback(d_bq_player_buffer_queue, queue_callback, this);
      if(result != SL_RESULT_SUCCESS) {
        GR_ERROR("grand::audio_sink", "Unable to register OpenSLES buffer queue callback");
        throw std::runtime_error("grand::audio_sink");
      }
      (void)result;

      // Set player state to play
      result = (*d_bq_player_play)->SetPlayState(d_bq_player_play, SL_PLAYSTATE_PLAYING);
      if(result != SL_RESULT_SUCCESS) {
        GR_ERROR("grand::audio_sink", "Unable to set OpenSLES audio player state to playing");
        throw std::runtime_error("grand::audio_sink");
      }
      (void)result;
    }

    void
    queue_callback(SLAndroidSimpleBufferQueueItf bq, void *context)
    {
      opensl_sink_impl *c = (opensl_sink_impl*)context;
      (*bq)->Enqueue(bq, c->d_buffer, c->d_size*sizeof(short));

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

      gr::thread::scoped_lock lock(mutex_lock);
      while(!signal)
        condition.wait(lock);
      signal = false;

      volk_32f_s32f_convert_16i(d_buffer, in, scale_factor, d_size);

      if(!d_once) {
        (*d_bq_player_buffer_queue)->Enqueue(d_bq_player_buffer_queue,
                                             d_buffer, d_size*sizeof(short));
        d_once = true;
      }

      // Tell runtime system how many output items we produced.
      return d_size;
    }

  } /* namespace grand */
} /* namespace gr */
