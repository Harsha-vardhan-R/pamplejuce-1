#include "PluginProcessor.h"
#include "PluginEditor.h"

PluginProcessor::PluginProcessor()
    : AudioProcessor(BusesProperties())
{
    try 
    {
        // Initialize Ort with compatible API version
        ortEnv = std::make_unique<Ort::Env>(ORT_LOGGING_LEVEL_WARNING, "moonbeam");
        
        // Load ONNX model at startup
        juce::File modelFile = juce::File::getCurrentWorkingDirectory().getChildFile("moonbeam_309M_pop.onnx");
        
        if (modelFile.existsAsFile())
        {
            Ort::SessionOptions sessionOptions;
            sessionOptions.SetIntraOpNumThreads(1);
            
            ortSession = std::make_unique<Ort::Session>(*ortEnv, modelFile.getFullPathName().toWideCharPointer(), sessionOptions);
            DBG("✓ ONNX model loaded successfully");
        }
        else
        {
            DBG("✗ ONNX model file not found: moonbeam_309M_pop.onnx");
        }
    }
    catch (const std::exception& e)
    {
        DBG("✗ Error initializing ONNX: " + juce::String(e.what()));
    }
}

PluginProcessor::~PluginProcessor()
{
    // unique_ptr automatically deletes ortSession and ortEnv
}

juce::AudioProcessorEditor* PluginProcessor::createEditor()
{
    return new PluginEditor(*this);
}

void PluginProcessor::runInference(const juce::File& midiFile, float temperature, float topP, int promptLen, int maxGenLen, std::function<void(const juce::String&)> logCallback)
{
    juce::ScopedLock lock(inferenceLock);
    
    if (!ortSession)
    {
        logCallback("ONNX model not loaded!");
        return;
    }
    
    if (!midiFile.existsAsFile())
    {
        logCallback("MIDI file does not exist!");
        return;
    }
    
    logCallback("Reading MIDI file: " + midiFile.getFullPathName());
    
    // --- MIDI to tokens (placeholder, replace with your actual tokenizer) ---
    std::vector<int64_t> inputTokens(promptLen, 1); // Dummy tokens
    size_t seqLen = inputTokens.size();
    
    // --- Prepare ONNX input ---
    std::vector<int64_t> inputShape = { 1, (int64_t)seqLen, 6 }; // Adjust shape as needed
    std::vector<int64_t> inputData(inputTokens); // Replace with actual tokenized data
    
    try
    {
        Ort::AllocatorWithDefaultOptions allocator;
        Ort::AllocatedStringPtr inputName = ortSession->GetInputNameAllocated(0, allocator);
        Ort::AllocatedStringPtr outputName = ortSession->GetOutputNameAllocated(0, allocator);
        
        Ort::MemoryInfo memInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
        Ort::Value inputTensor = Ort::Value::CreateTensor<int64_t>(memInfo, inputData.data(), inputData.size(), inputShape.data(), inputShape.size());
        
        logCallback("Running ONNX inference...");
        
        std::vector<Ort::Value> ortInputs;
        ortInputs.push_back(std::move(inputTensor));
        
        std::vector<const char*> inputNames = { inputName.get() };
        std::vector<const char*> outputNames = { outputName.get() };
        
        auto outputTensors = ortSession->Run(Ort::RunOptions{ nullptr }, inputNames.data(), ortInputs.data(), 1, outputNames.data(), 1);
        
        // --- Postprocess output (placeholder) ---
        logCallback("Inference complete. Output tensor shape: " + juce::String((int)outputTensors[0].GetTensorTypeAndShapeInfo().GetShape()[0]));
        logCallback("TODO: Convert output tokens to MIDI and save file.");
    }
    catch (const std::exception& e)
    {
        logCallback("✗ Inference error: " + juce::String(e.what()));
    }
}

// Entry point for JUCE plugin - REQUIRED!
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PluginProcessor();
}