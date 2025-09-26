/*
  ==============================================================================

    reverb.h
    Created: 10 Sep 2025 12:56:58pm
    Author:  Jake Morgan

  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"

namespace termite{

class delay{
public:
    delay();
    virtual ~delay();
    /*
    virtual void prepare_to_play(double sample_rate);
    virtual void process_block(juce::AudioBuffer<float>& in_buffer);
    virtual void set_parameters(float delay_time_ms, float fb_gain);
    */
protected:
    double ms_to_samps(double sample_rate, float in_ms){
        return (in_ms * sample_rate)/ 1000;
    }
    juce::LinearSmoothedValue<float> smoothed_delay_time;
    juce::LinearSmoothedValue<float> smoothed_feedback;
    juce::AudioBuffer<float> circular_buffer;
    std::vector<int> readheads{0,0};
    std::vector<int> writeheads{0,0};
    double f_sample_rate;
};

// lowkey better practice to have these inherit an object with virtual functions.
class allpass_filter : public delay{
public:
    allpass_filter();
    ~allpass_filter();
    void prepare_to_play(double sample_rate);
    void process_block(juce::AudioBuffer<float>& in_buffer);
    void set_parameters(const float delay_time_ms, const float fb_gain);
private:
};

class comb_filter{
public:
    comb_filter();
    ~comb_filter();
    void prepare_to_play(double sample_rate);
    void process_block(juce::AudioBuffer<float>& in_buffer);
    void set_parameters(const float delay_time_ms, const float fb_gain);
private:
    double ms_to_samps(double sample_rate, float in_ms){
        return (in_ms * sample_rate)/ 1000;
    }
    juce::LinearSmoothedValue<float> smoothed_delay_time;
    juce::LinearSmoothedValue<float> smoothed_feedback;
    juce::AudioBuffer<float> circular_buffer;
    std::vector<int> readheads{0,0};
    std::vector<int> writeheads{0,0};
    double f_sample_rate;
};

class delay_matrix{
public:
    delay_matrix();
    ~delay_matrix();
    void prepare_to_play(double sample_rate);
    void process_block(juce::AudioBuffer<float>& in_buffer);
    void set_parameters(const float in_delay_time);
    void comb_matrix(juce::AudioBuffer<float>& in_buffer);
    delay_matrix* get_self();
private:
    juce::LinearSmoothedValue<float> smoothed_delay_time;
    juce::LinearSmoothedValue<float> smoothed_feedback;
    std::vector<comb_filter> comb_filters;
    std::vector<allpass_filter> allpass_filters;
    std::vector<juce::AudioBuffer<float>> comb_buffers;
    const int num_allpass = 2;
    const int num_comb = 4;
    const float comb_gain = 0.4f;
    const float allpass_gain = 0.7f;
};

}
