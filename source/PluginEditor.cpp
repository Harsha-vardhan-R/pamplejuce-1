#include "PluginEditor.h"

PluginEditor::PluginEditor(PluginProcessor& p) 
    : AudioProcessorEditor(&p), processor(p)
{
    setSize(600, 500);

    // MIDI file button
    addAndMakeVisible(chooseMidiButton);
    chooseMidiButton.addListener(this);

    addAndMakeVisible(midiFileLabel);
    midiFileLabel.setColour(juce::Label::textColourId, juce::Colours::lightgreen);

    // Temperature slider
    addAndMakeVisible(temperatureSlider);
    temperatureSlider.setRange(0.0, 2.0, 0.01);
    temperatureSlider.setValue(0.99);
    temperatureSlider.addListener(this);
    addAndMakeVisible(temperatureLabel);

    // Top-P slider
    addAndMakeVisible(topPSlider);
    topPSlider.setRange(0.0, 1.0, 0.01);
    topPSlider.setValue(0.95);
    topPSlider.addListener(this);
    addAndMakeVisible(topPLabel);

    // Seq Length slider
    addAndMakeVisible(seqLengthSlider);
    seqLengthSlider.setRange(1, 1024, 1);
    seqLengthSlider.setValue(512);
    seqLengthSlider.addListener(this);
    addAndMakeVisible(seqLengthLabel);

    // Max Gen Length slider
    addAndMakeVisible(maxGenLengthSlider);
    maxGenLengthSlider.setRange(1, 1024, 1);
    maxGenLengthSlider.setValue(512);
    maxGenLengthSlider.addListener(this);
    addAndMakeVisible(maxGenLabel);

    // Generate button
    addAndMakeVisible(runButton);
    runButton.addListener(this);
    runButton.setEnabled(false);

    // Save button
    addAndMakeVisible(saveButton);
    saveButton.addListener(this);
    saveButton.setEnabled(false);

    // Log display
    addAndMakeVisible(logEditor);
    logEditor.setMultiLine(true);
    logEditor.setReadOnly(true);
    logEditor.setColour(juce::TextEditor::backgroundColourId, juce::Colours::black);
    logEditor.setColour(juce::TextEditor::textColourId, juce::Colours::green);
    logEditor.setColour(juce::TextEditor::outlineColourId, juce::Colours::darkgrey);
}

PluginEditor::~PluginEditor() {}

void PluginEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey);
}

void PluginEditor::resized()
{
    auto r = getLocalBounds().reduced(10);

    // File section
    auto fileRow = r.removeFromTop(35);
    chooseMidiButton.setBounds(fileRow.removeFromLeft(100));
    fileRow.removeFromLeft(10);
    midiFileLabel.setBounds(fileRow.removeFromLeft(200));

    r.removeFromTop(10);

    // Sliders section
    auto tempRow = r.removeFromTop(30);
    temperatureLabel.setBounds(tempRow.removeFromLeft(150));
    tempRow.removeFromLeft(5);
    temperatureSlider.setBounds(tempRow.removeFromLeft(150));

    auto topPRow = r.removeFromTop(30);
    topPLabel.setBounds(topPRow.removeFromLeft(150));
    topPRow.removeFromLeft(5);
    topPSlider.setBounds(topPRow.removeFromLeft(150));

    auto seqRow = r.removeFromTop(30);
    seqLengthLabel.setBounds(seqRow.removeFromLeft(150));
    seqRow.removeFromLeft(5);
    seqLengthSlider.setBounds(seqRow.removeFromLeft(150));

    auto maxGenRow = r.removeFromTop(30);
    maxGenLabel.setBounds(maxGenRow.removeFromLeft(150));
    maxGenRow.removeFromLeft(5);
    maxGenLengthSlider.setBounds(maxGenRow.removeFromLeft(150));

    r.removeFromTop(10);

    // Buttons
    auto buttonRow = r.removeFromTop(35);
    runButton.setBounds(buttonRow.removeFromLeft(100));
    buttonRow.removeFromLeft(10);
    saveButton.setBounds(buttonRow.removeFromLeft(100));

    r.removeFromTop(5);

    // Log
    logEditor.setBounds(r);
}

void PluginEditor::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &temperatureSlider)
    {
        temperatureLabel.setText(
            "Temperature: " + juce::String(temperatureSlider.getValue(), 2),
            juce::dontSendNotification
        );
    }
    else if (slider == &topPSlider)
    {
        topPLabel.setText(
            "Top-P: " + juce::String(topPSlider.getValue(), 2),
            juce::dontSendNotification
        );
    }
    else if (slider == &seqLengthSlider)
    {
        seqLengthLabel.setText(
            "Seq Length: " + juce::String((int)seqLengthSlider.getValue()),
            juce::dontSendNotification
        );
    }
    else if (slider == &maxGenLengthSlider)
    {
        maxGenLabel.setText(
            "Max Gen: " + juce::String((int)maxGenLengthSlider.getValue()),
            juce::dontSendNotification
        );
    }
}

void PluginEditor::buttonClicked(juce::Button* b)
{
    if (b == &chooseMidiButton)
    {
        juce::FileChooser chooser("Select MIDI file...", {}, "*.mid;*.midi");
        chooser.launchAsync(juce::FileBrowserComponent::openMode, [this](const juce::FileChooser& fc) {
            if (fc.getResults().size() > 0)
            {
                midiFile = fc.getResult();
                midiFileLabel.setText(midiFile.getFileName(), juce::dontSendNotification);
                runButton.setEnabled(true);
                appendLog("✓ Loaded: " + midiFile.getFileName());
            }
        });
    }
    else if (b == &runButton)
    {
        if (!midiFile.existsAsFile())
        {
            appendLog("✗ Please load a MIDI file first!");
            return;
        }

        float temperature = (float)temperatureSlider.getValue();
        float topP = (float)topPSlider.getValue();
        int seqLen = (int)seqLengthSlider.getValue();
        int maxGen = (int)maxGenLengthSlider.getValue();

        appendLog("━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
        appendLog("Starting generation...");
        appendLog("Temperature: " + juce::String(temperature, 2));
        appendLog("Top-P: " + juce::String(topP, 2));
        appendLog("Seq Length: " + juce::String(seqLen));
        appendLog("Max Gen: " + juce::String(maxGen));
        appendLog("━━━━━━━━━━━━━━━━━━━━━━━━━━━━");

        runButton.setEnabled(false);

        std::thread([this, temperature, topP, seqLen, maxGen]() {
            processor.runInference(midiFile, temperature, topP, seqLen, maxGen,
                [this](const juce::String& msg) { appendLog(msg); });
            
            // After generation completes, save output file
            lastOutputFile = midiFile.getParentDirectory().getChildFile(
                midiFile.getFileNameWithoutExtension() + "_generated.mid"
            );
            
            juce::MessageManager::callAsync([this]() {
                saveButton.setEnabled(true);
                appendLog("✓ Generation complete! Click 'Save MIDI' to save.");
            });
        }).detach();
    }
    else if (b == &saveButton)
    {
        if (lastOutputFile == juce::File())
        {
            appendLog("✗ No generated file available to save.");
            return;
        }

        juce::FileChooser chooser("Save generated MIDI...", {}, "*.mid");
        chooser.launchAsync(juce::FileBrowserComponent::saveMode, [this](const juce::FileChooser& fc) {
            if (fc.getResults().size() > 0)
            {
                juce::File destination = fc.getResult().withFileExtension("mid");
                if (lastOutputFile.copyFileTo(destination))
                {
                    appendLog("✓ Saved: " + destination.getFullPathName());
                }
                else
                {
                    appendLog("✗ Failed to save file.");
                }
            }
        });
    }
}

void PluginEditor::appendLog(const juce::String& line)
{
    juce::MessageManager::callAsync([this, line]() {
        logEditor.moveCaretToEnd();
        logEditor.insertTextAtCaret(line + "\n");
        logEditor.moveCaretToEnd();
    });
}