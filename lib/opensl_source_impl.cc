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

#include <volk/volk.h>
#include <gnuradio/io_signature.h>
#include "opensl_source_impl.h"

namespace gr {
  namespace grand {

    opensl_source::sptr
    opensl_source::make(int sampling_rate)
    {
      return gnuradio::get_initial_sptr
        (new opensl_source_impl(sampling_rate));
    }

    opensl_source_impl::opensl_source_impl(int sampling_rate)
      : gr::sync_block("opensl_source",
                       gr::io_signature::make(0, 0, 0),
                       gr::io_signature::make(1, 1, sizeof(float)))
    {
      set_sample_rate(sampling_rate);
      d_size = 2048;
      signal = true;

      set_output_multiple(d_size);

      setup_interface();

      d_buffer = (short*)volk_malloc(d_size*sizeof(short), volk_get_alignment());
      if(!d_buffer) {
        std::string e = boost::str
          (boost::format("Unable to allocate audio buffer of %1% bytes") \
           % (d_size*sizeof(short)));
        GR_ERROR("grand::audio_source", e);
        throw std::runtime_error(e);
      }
    }

    opensl_source_impl::~opensl_source_impl()
    {
      volk_free(d_buffer);
    }

    void
    opensl_source_impl::set_sample_rate(int samp_rate)
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
    opensl_source_impl::setup_interface()
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
        GR_ERROR("grand::audio_source", "Unable to open OpenSLES audio engine");
        throw std::runtime_error("grand::audio_source");
      }
      (void)result;

      // Realize audio engine
      result = (*d_engine)->Realize(d_engine, SL_BOOLEAN_FALSE);
      if(result != SL_RESULT_SUCCESS) {
        GR_ERROR("grand::audio_source", "Unable to realize engine");
        throw std::runtime_error("grand::audio_source");
      }
      (void)result;

      // Get audio engine interface
      result = (*d_engine)->GetInterface(d_engine, SL_IID_ENGINE, &d_engine_eng);
      if(result != SL_RESULT_SUCCESS) {
        std::string e = boost::str
          (boost::format("Unable to open OpenSLES engine interface"));
        GR_ERROR("grand::audio_source", e);
        throw std::runtime_error("grand::audio_source");
      }
      (void)result;


      // Audio source and sink configurations
      SLDataLocator_IODevice loc_dev = {SL_DATALOCATOR_IODEVICE, SL_IODEVICE_AUDIOINPUT,
                                        SL_DEFAULTDEVICEID_AUDIOINPUT, NULL};
      SLDataSource audioSrc = {&loc_dev, NULL};

      SLDataLocator_AndroidSimpleBufferQueue loc_bq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
      SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, channels, d_sample_rate,
                                     SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
                                     speakers, SL_BYTEORDER_LITTLEENDIAN};
      SLDataSink audioSnk = {&loc_bq, &format_pcm};

      // Create audio recorder (set the RECORD_AUDIO permission)
      const SLInterfaceID id[1] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE};
      const SLboolean req[1] = {SL_BOOLEAN_TRUE};
      result = (*d_engine_eng)->CreateAudioRecorder(d_engine_eng, &(d_recorder_obj), &audioSrc,
                                                    &audioSnk, 1, id, req);
      if(result != SL_RESULT_SUCCESS) {
        GR_ERROR("grand::audio_source", "Unable to create OpenSLES audio recorder");
        throw std::runtime_error("grand::audio_source");
      }
      (void)result;

      // Realize the audio recorder
      result = (*d_recorder_obj)->Realize(d_recorder_obj, SL_BOOLEAN_FALSE);
      if(result != SL_RESULT_SUCCESS) {
        GR_ERROR("grand::audio_source", "Unable to realize OpenSLES audio recorder");
        throw std::runtime_error("grand::audio_source");
      }
      (void)result;


      // Get audio recorder interface
      result = (*d_recorder_obj)->GetInterface(d_recorder_obj, SL_IID_RECORD, &(d_recorder_record));
      if(result != SL_RESULT_SUCCESS) {
        std::string e = boost::str
          (boost::format("Unable to get OpenSLES audio recorder interface"));
        GR_ERROR("grand::audio_source", e);
        throw std::runtime_error("grand::audio_source");
      }
      (void)result;

      // Get interface to a buffer queue
      result = (*d_recorder_obj)->GetInterface(d_recorder_obj, SL_IID_ANDROIDSIMPLEBUFFERQUEUE,
                                              &(d_recorder_buffer_queue));
      if(result != SL_RESULT_SUCCESS) {
        std::string e = boost::str
          (boost::format("Unable to get OpenSLES audio recorder buffer interface"));
        GR_ERROR("grand::audio_source", e);
        throw std::runtime_error("grand::audio_source");
      }
      (void)result;

      // Set callback function, called when data is available from recorder
      result = (*d_recorder_buffer_queue)->RegisterCallback(d_recorder_buffer_queue, recorder_callback, this);
      if(result != SL_RESULT_SUCCESS) {
        GR_ERROR("grand::audio_source", "Unable to register OpenSLES audio recorder callback");
        throw std::runtime_error("grand::audio_source");
      }
      (void)result;

      // Set recorder to start recording
      result = (*d_recorder_record)->SetRecordState(d_recorder_record, SL_RECORDSTATE_RECORDING);
      if(result != SL_RESULT_SUCCESS) {
        GR_ERROR("grand::audio_source", "Unable to start OpenSLES recording");
        throw std::runtime_error("grand::audio_source");
      }
      (void)result;
    }

    // this callback handler is called every time a buffer finishes recording
    void recorder_callback(SLAndroidSimpleBufferQueueItf bq, void *context)
    {
      opensl_source_impl *c = (opensl_source_impl *) context;
      gr::thread::scoped_lock lock(c->mutex_lock);
      c->signal = true;
      c->condition.notify_one();
    }

    int
    opensl_source_impl::work(int noutput_items,
			  gr_vector_const_void_star &input_items,
			  gr_vector_void_star &output_items)
    {
      float *out = (float *) output_items[0];

      float scale_factor = 16384.0f;

      gr::thread::scoped_lock lock(mutex_lock);
      while(!signal) {
        condition.wait(lock);
      }
      signal = false;

      (*d_recorder_buffer_queue)->Enqueue(d_recorder_buffer_queue,
                                          d_buffer, d_size*sizeof(short));

      volk_16i_s32f_convert_32f(out, d_buffer, scale_factor, d_size);

      return d_size;
    }

  } /* namespace grand */
} /* namespace gr */
