/*
  ==============================================================================

    reverb.cpp
    Created: 10 Sep 2025 12:56:58pm
    Author:  Jake Morgan

  ==============================================================================
*/

//TODO: interpolation algo is alright but needs work.

#include "reverb.h"
using namespace termite;

delay::delay(){
    
}
delay::~delay(){
    
}


// ALLPASS
allpass_filter::allpass_filter(){
    smoothed_feedback.setCurrentAndTargetValue(0.7f);
    smoothed_delay_time.setCurrentAndTargetValue(220.f);
    last_delayed_sample = {0.0f, 0.0f};
}
allpass_filter::~allpass_filter(){
    
}
// okay i think interpolation will be a problem but i really don't have time to worry about that rn.
// could technically do a "onValueChanged" lambda thingy and have the process block interpolate the samples whenever "onValueChanged" is called.
// idk tho
void allpass_filter::prepare_to_play(double sample_rate){
    smoothed_delay_time.reset(sample_rate, 0.001f);
    smoothed_feedback.reset(sample_rate, 0.01f);
    circular_buffer.setSize(2, sample_rate * 2);
    circular_buffer.clear();
    f_sample_rate = sample_rate;
    last_delayed_sample = {0.f, 0.f};
    writeheads = {0,0};
}
void allpass_filter::process_block(juce::AudioBuffer<float>& in_buffer){
    auto total_input_channels = in_buffer.getNumChannels();
    for(int channel = 0; channel < total_input_channels; ++channel){
        auto* writehead = &writeheads[channel];
        auto* channel_data = in_buffer.getWritePointer(channel);
        auto* circular_buffer_data = circular_buffer.getWritePointer(channel);
        auto num_samples = in_buffer.getNumSamples();
        
        for(int i = 0; i < num_samples; i++){
            float input = channel_data[i];
            float delay_time = ms_to_samps(f_sample_rate, smoothed_delay_time.getNextValue());
            delay_time = juce::jlimit(1.0f, (float)(circular_buffer.getNumSamples() - 2), delay_time);
            float feedback = smoothed_feedback.getNextValue();
            
            // Calculate read position
            float read_pos = *writehead - delay_time;
            if(read_pos < 0)
                read_pos += circular_buffer.getNumSamples();
            
            int read_index = (int)read_pos;
            float frac = read_pos - read_index;
            
            int next_index = read_index + 1;
            if(next_index >= circular_buffer.getNumSamples())
                next_index = 0;
            
            // Spatial interpolation between buffer positions
            float sample1 = circular_buffer_data[read_index];
            float sample2 = circular_buffer_data[next_index];
            float current_delayed = sample1 + frac * (sample2 - sample1);
            
            // Temporal interpolation between last frame and current frame
//            float delayed_sample = last_delayed_sample[channel] * 0.5f + current_delayed * 0.5f;
//            last_delayed_sample[channel] = current_delayed;
            float delayed_sample = current_delayed;
            
            circular_buffer_data[*writehead] = ((delayed_sample * feedback) + input) * (1.0f - feedback);
            float delay_out = delayed_sample + (input * (-feedback));
            
            channel_data[i] = delay_out;
            
            ++*writehead;
            if(*writehead >= circular_buffer.getNumSamples())
                *writehead = 0;
        }
    }
}
/*
void allpass_filter::process_block(juce::AudioBuffer<float>& in_buffer){
    ///circular_buffer.clear();
    //
    float delay_time = ms_to_samps(f_sample_rate, smoothed_delay_time.getNextValue());
    float feedback = smoothed_feedback.getNextValue();
    //
    auto total_input_channels = in_buffer.getNumChannels();
    for(int channel = 0; channel < total_input_channels; ++channel){
        auto* readhead = &readheads[channel];
        auto* writehead = &writeheads[channel];
        //
        auto* channel_data = in_buffer.getWritePointer(channel);
        auto* circular_buffer_data = circular_buffer.getWritePointer(channel);
        auto num_samples  = in_buffer.getNumSamples();
        for(int i = 0; i < num_samples; i++){
            float input = channel_data[i];

            // put last sample into memory before new position is determined
            float last_sample = circular_buffer_data[*readhead];
            *readhead = *writehead - (int)delay_time;
            if(*readhead<0)
                *readhead += circular_buffer.getNumSamples();
            auto next_readhead = *readhead + 1;
            if(next_readhead > circular_buffer.getNumSamples())
                next_readhead -= circular_buffer.getNumSamples();
            
            
            // delay buffer will have different data than output data...
            // remember that when debugging.
            float delayed_sample = circular_buffer_data[*readhead];
            float next_sample = circular_buffer_data[next_readhead];
            
            //delayed_sample = interpolate_sample(delayed_sample, last_sample, next_sample);
            // original calculation.
//            circular_buffer_data[*writehead] = ((
//                                                (circular_buffer_data[*readhead] * feedback)
//                                                + input)
//                                                * (1 - feedback)
//                                                );
            
            circular_buffer_data[*writehead] = ((
                                                ((circular_buffer_data[*readhead] + next_sample)/2 * feedback)
                                                + input)
                                                * (1 - feedback)
                                                );
            
//            circular_buffer_data[*writehead] = ((
//                                                (delayed_sample * feedback)
//                                                + input)
//                                                * (1 - feedback)
//                                                );
            // ORIGINAL CALCULATION
            float delay_out = circular_buffer_data[*readhead]
                                + (input * (feedback * -1));
//            float delay_out = delayed_sample
//                                    + (input * (feedback * -1));
            channel_data[i] = delay_out;
            ++*writehead;
            if(*writehead >= circular_buffer.getNumSamples())
                *writehead = 0;
        }
    }
    
}
 */
void allpass_filter::set_parameters(float delay_time_ms, float fb_gain){
    smoothed_delay_time.setTargetValue(delay_time_ms);
    smoothed_feedback.setTargetValue(fb_gain);
}
//

// COMB FILTER
comb_filter::comb_filter(){
    smoothed_feedback.setCurrentAndTargetValue(0.7f);
    smoothed_delay_time.setCurrentAndTargetValue(220.f);
    last_delayed_sample = {0.f, 0.f};
}
comb_filter::~comb_filter(){
    
}
void comb_filter::prepare_to_play(double sample_rate){
    smoothed_delay_time.reset(sample_rate, 0.001f);
    smoothed_feedback.reset(sample_rate, 0.01f);
    circular_buffer.setSize(2, sample_rate * 2);
    circular_buffer.clear();
    f_sample_rate = sample_rate;
    last_delayed_sample = {0.f, 0.f};
    writeheads = {0,0};
}
//
void comb_filter::process_block(juce::AudioBuffer<float>& in_buffer){
    auto total_input_channels = in_buffer.getNumChannels();
    for(int channel = 0; channel < total_input_channels; ++channel){
        auto* writehead = &writeheads[channel];
        auto* channel_data = in_buffer.getWritePointer(channel);
        auto* circular_buffer_data = circular_buffer.getWritePointer(channel);
        auto num_samples = in_buffer.getNumSamples();
        
        for(int i = 0; i < num_samples; i++){
            float input = channel_data[i];
            float delay_time = ms_to_samps(f_sample_rate, smoothed_delay_time.getNextValue());
            delay_time = juce::jlimit(1.0f, (float)(circular_buffer.getNumSamples() - 2), delay_time);
            float feedback = smoothed_feedback.getNextValue();

            float read_pos = *writehead - delay_time;
            if(read_pos < 0)
                read_pos += circular_buffer.getNumSamples();
            
            int read_index = (int)read_pos;
            float frac = read_pos - read_index;
            
            int next_index = read_index + 1;
            if(next_index >= circular_buffer.getNumSamples())
                next_index = 0;
            
            float sample1 = circular_buffer_data[read_index];
            float sample2 = circular_buffer_data[next_index];
            float current_delayed = sample1 + frac * (sample2 - sample1);
            
            // Temporal interpolation between last frame and current frame
//            float delayed_sample = last_delayed_sample[channel] * 0.5f + current_delayed * 0.5f;
//            last_delayed_sample[channel] = current_delayed;
            float delayed_sample = current_delayed;
            
            circular_buffer_data[*writehead] = input + (delayed_sample * feedback);
            channel_data[i] = delayed_sample;
            
            ++*writehead;
            if(*writehead >= circular_buffer.getNumSamples())
                *writehead = 0;
        }
    }
}

/*
void comb_filter::process_block(juce::AudioBuffer<float>& in_buffer){
    ///circular_buffer.clear();
    // NEED TO CALCULATE THE PROPER NORMALIZED TO SAMPLE REVERBERATION.
    float delay_time = ms_to_samps(f_sample_rate, smoothed_delay_time.getNextValue());
    float feedback = smoothed_feedback.getNextValue();
    //
    auto total_input_channels = in_buffer.getNumChannels();
    for(int channel = 0; channel < total_input_channels; ++channel){
        auto* readhead = &readheads[channel];
        auto* writehead = &writeheads[channel];
        auto* channel_data = in_buffer.getWritePointer(channel);
        auto* circular_buffer_data = circular_buffer.getWritePointer(channel);
        auto num_samples  = in_buffer.getNumSamples();
        for(int i = 0; i < num_samples; i++){
            float input = channel_data[i];
            //channel_data[i] = 0.f;
            //
            float last_sample = circular_buffer_data[*readhead];
            *readhead = *writehead - delay_time;
            //
            if(*readhead < 0)
                *readhead += circular_buffer.getNumSamples();
            auto next_readhead = *readhead + 1;
            if(next_readhead > circular_buffer.getNumSamples())
                next_readhead -= circular_buffer.getNumSamples();
            
            float delayed_sample = circular_buffer_data[*readhead];
            float next_sample = circular_buffer_data[next_readhead];
            
            //delayed_sample = interpolate_sample(delayed_sample, last_sample, next_sample);
            //ORIGINAL CALCULATION
            //circular_buffer_data[*writehead] = input + (circular_buffer_data[*readhead] * feedback);
            
            circular_buffer_data[*writehead] = input + ((circular_buffer_data[*readhead] + next_sample)/2 * feedback);
            // circular_buffer_data[*writehead] = input + (delayed_sample * feedback);
            float delay_out = delayed_sample;
            channel_data[i] = delayed_sample;
            ++*writehead;
            if(*writehead >= circular_buffer.getNumSamples())
                *writehead = 0;
            //DBG(*readhead);
        }
    }
}
 */
void comb_filter::set_parameters(float delay_time_ms, float fb_gain){
    smoothed_delay_time.setTargetValue(delay_time_ms);
    smoothed_feedback.setTargetValue(fb_gain);
}
//

//
delay_matrix::delay_matrix(){
    smoothed_feedback.setCurrentAndTargetValue(0.7f);
    smoothed_delay_time.setCurrentAndTargetValue(220.f);
    comb_filters.reserve(num_comb);
    comb_buffers.reserve(num_comb);
    allpass_filters.reserve(num_allpass);
    for(int i = 0; i < num_comb; i++){
        comb_filters.emplace_back();
        comb_buffers.emplace_back();
    }
    for(int i = 0; i < num_allpass; i++){
        allpass_filters.emplace_back();
    }
}
delay_matrix::~delay_matrix(){
    
}
// TODO: okay so the original Bell labs delay times are a little funky so claude and other agents recommend using prime numbers as a way to spread out the filters properly.
// actually sounds really nice so let's figure out a good way to make this work dynamically then hook it up to the rotation matrix! Sep 15 204pm
void delay_matrix::set_parameters(const float in_delay_time){
    // const int total_num_filters = num_comb + num_allpass;
    const float comb_times[4] = {29.0f, 37.0f, 43.0f, 47.0f};
    const float allpass_times[2] = {5.0f,1.7f};
    for(int i = 0; i < num_comb; i++){
        const float b = 1.005f;
        // change 0.035 to a const instead of magic number.
//        comb_filters[i].set_parameters(((in_delay_time) / ((b * i) + 1)),
//                                       comb_gain - (i * 0.035));
        comb_filters[i].set_parameters(comb_times[i] * std::abs(in_delay_time * b), comb_gain - (i * 0.035));
    }
    for(int i = 0; i < num_allpass; i++){
        const float b = 1.01f;
//        allpass_filters[i].set_parameters(((in_delay_time) / ((b * (i + num_comb)) + 1)),
//                                          allpass_gain);
        allpass_filters[i].set_parameters(allpass_times[i] * std::abs(in_delay_time * b), allpass_gain);
        
    }
}
void delay_matrix::prepare_to_play(double sample_rate){
    for(int i = 0; i < num_comb; i++){
        comb_buffers[i].setSize(2, sample_rate);
        comb_buffers[i].clear();
        comb_filters[i].prepare_to_play(sample_rate);
    }
    for(int i = 0; i < num_allpass; i++){
        allpass_filters[i].prepare_to_play(sample_rate);
    }
}
void delay_matrix::process_block(juce::AudioBuffer<float>& in_buffer){
    comb_matrix(in_buffer);
    for(int i = 0; i < num_allpass; i++){
        allpass_filters[i].process_block(in_buffer);
    }
}
void delay_matrix::comb_matrix(juce::AudioBuffer<float>& in_buffer){
    for(int i = 0; i < num_comb; i++){
        comb_buffers[i].clear();
        comb_buffers[i].makeCopyOf(in_buffer);
        comb_filters[i].process_block(comb_buffers[i]);
    }
    for(int channels = 0; channels < in_buffer.getNumChannels(); channels++){
        auto* channel_data = in_buffer.getWritePointer(channels);
        //hardcode cause lazy.
        // change back to channels
        auto* buf1 = comb_buffers[0].getWritePointer(channels);
        auto* buf2 = comb_buffers[1].getWritePointer(channels);
        auto* buf3 = comb_buffers[2].getWritePointer(channels);
        auto* buf4 = comb_buffers[3].getWritePointer(channels);
        
        int num_samples = in_buffer.getNumSamples();
        for(int i = 0; i < num_samples; i++){
            channel_data[i] = ((buf1[i] + buf2[i]) + (buf3[i] + buf4[i])) * 0.25f;
        }
    }
}
delay_matrix* delay_matrix::get_self(){
    return this;
}
//
