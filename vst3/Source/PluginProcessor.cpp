#include "PluginProcessor.h"
#include "PluginEditor.h"

MixnoteProcessor::MixnoteProcessor()
    : AudioProcessor(BusesProperties()
          .withInput("Input",  juce::AudioChannelSet::stereo(), true)
          .withOutput("Output", juce::AudioChannelSet::stereo(), true)) {
}

MixnoteProcessor::~MixnoteProcessor() {}

const juce::String MixnoteProcessor::getName() const { return "Mixnote"; }
bool MixnoteProcessor::acceptsMidi() const  { return false; }
bool MixnoteProcessor::producesMidi() const { return false; }
bool MixnoteProcessor::isMidiEffect() const { return false; }
double MixnoteProcessor::getTailLengthSeconds() const { return 0.0; }

int MixnoteProcessor::getNumPrograms()    { return 1; }
int MixnoteProcessor::getCurrentProgram() { return 0; }
void MixnoteProcessor::setCurrentProgram(int index) {
    juce::ignoreUnused(index);
}
const juce::String MixnoteProcessor::getProgramName(int index) {
    juce::ignoreUnused(index);
    return {};
}
void MixnoteProcessor::changeProgramName(int index, const juce::String& newName) {
    juce::ignoreUnused(index, newName);
}

void MixnoteProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    juce::ignoreUnused(sampleRate, samplesPerBlock);
}
void MixnoteProcessor::releaseResources() {}

void MixnoteProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) {
    // Pass audio through unchanged — this plugin is a comment/review tool, not an effect.
    juce::ignoreUnused(buffer);

    // Read transport info for the UI
    if (auto* playHead = getPlayHead()) {
        if (auto pos = playHead->getPosition()) {
            if (auto timeInSeconds = pos->getTimeInSeconds())
                transportPosition.store(*timeInSeconds);

            transportPlaying.store(pos->getIsPlaying());

            if (auto bpm = pos->getBpm())
                transportBpm.store(*bpm);
        }
    }
}

bool MixnoteProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* MixnoteProcessor::createEditor() {
    return new MixnoteEditor(*this);
}

// ---------------------------------------------------------------------------
// State persistence — save/restore plugin settings in the DAW session
// ---------------------------------------------------------------------------

void MixnoteProcessor::getStateInformation(juce::MemoryBlock& destData) {
    auto state = std::make_unique<juce::DynamicObject>();
    state->setProperty("serverUrl", serverUrl);
    state->setProperty("username", username);
    state->setProperty("authorName", authorName);
    state->setProperty("lastShareLink", lastShareLink);
    state->setProperty("rememberPassword", rememberPassword);
    state->setProperty("autoplayEnabled", autoplayEnabled);

    if (rememberPassword)
        state->setProperty("savedPassword", savedPassword);

    // Calibration offsets
    auto offsets = std::make_unique<juce::DynamicObject>();
    for (auto& [songId, offset] : calibrationOffsets)
        offsets->setProperty(juce::String(songId), offset);
    state->setProperty("calibrationOffsets", juce::var(offsets.release()));

    auto jsonStr = juce::JSON::toString(juce::var(state.release()));
    destData.append(jsonStr.toRawUTF8(), jsonStr.getNumBytesAsUTF8());
}

void MixnoteProcessor::setStateInformation(const void* data, int sizeInBytes) {
    auto jsonStr = juce::String::fromUTF8(static_cast<const char*>(data), sizeInBytes);
    auto state = juce::JSON::parse(jsonStr);

    serverUrl      = state.getProperty("serverUrl", "").toString();
    username       = state.getProperty("username", "").toString();
    authorName     = state.getProperty("authorName", "").toString();
    lastShareLink  = state.getProperty("lastShareLink", "").toString();
    rememberPassword = static_cast<bool>(state.getProperty("rememberPassword", false));
    autoplayEnabled  = static_cast<bool>(state.getProperty("autoplayEnabled", true));

    if (rememberPassword)
        savedPassword = state.getProperty("savedPassword", "").toString();

    // Calibration offsets
    auto offsetsVar = state.getProperty("calibrationOffsets", juce::var());
    if (auto* obj = offsetsVar.getDynamicObject()) {
        for (auto& prop : obj->getProperties()) {
            int songId = prop.name.toString().getIntValue();
            double offset = static_cast<double>(prop.value);
            calibrationOffsets[songId] = offset;
        }
    }
}

// Plugin instantiation
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new MixnoteProcessor();
}
