#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"

class PluginEditor : public juce::AudioProcessorEditor, 
                     private juce::Button::Listener,
                     private juce::Slider::Listener
{
public:
    explicit PluginEditor(PluginProcessor&);
    ~PluginEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void sliderValueChanged(juce::Slider* slider) override;

private:
    void buttonClicked(juce::Button*) override;
    void appendLog(const juce::String&);

    PluginProcessor& processor;

    // File selection
    juce::TextButton chooseMidiButton{"Load MIDI"};
    juce::Label midiFileLabel{"midiLabel", "No MIDI loaded"};

    // Temperature slider
    juce::Slider temperatureSlider;
    juce::Label temperatureLabel{"tempLabel", "Temperature: 0.99"};

    // Top-P slider
    juce::Slider topPSlider;
    juce::Label topPLabel{"topPLabel", "Top-P: 0.95"};

    // Sequence length slider
    juce::Slider seqLengthSlider;
    juce::Label seqLengthLabel{"seqLabel", "Seq Length: 512"};

    // Max generation length slider
    juce::Slider maxGenLengthSlider;
    juce::Label maxGenLabel{"maxGenLabel", "Max Gen: 512"};

    // Control buttons
    juce::TextButton runButton{"Generate"};
    juce::TextButton saveButton{"Save MIDI"};

    // Log display
    juce::TextEditor logEditor;

    // State
    juce::File midiFile;
    juce::File lastOutputFile;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginEditor)
};