#pragma once
#include <JuceHeader.h>

class MixnoteProcessor : public juce::AudioProcessor {
public:
    MixnoteProcessor();
    ~MixnoteProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // Transport info for the editor
    double getTransportPositionSeconds() const { return transportPosition.load(); }
    bool isTransportPlaying() const { return transportPlaying.load(); }
    double getTransportBpm() const { return transportBpm.load(); }

    // Persistent settings
    juce::String serverUrl;
    juce::String username;
    juce::String authorName;
    juce::String savedPassword;
    juce::String lastShareLink;
    bool rememberPassword = false;
    bool autoplayEnabled = true;

    // Per-song calibration offsets (songId -> offset seconds)
    std::map<int, double> calibrationOffsets;

private:
    std::atomic<double> transportPosition { 0.0 };
    std::atomic<bool> transportPlaying { false };
    std::atomic<double> transportBpm { 120.0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MixnoteProcessor)
};
