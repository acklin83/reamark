#include "PluginProcessor.h"
#include "PluginEditor.h"

// Per-user global settings (server + connect token + display name). Kept out of the DAW
// project state on purpose: a shared .rpp must never carry the studio's connect token.
static juce::PropertiesFile& globalSettings() {
    static juce::PropertiesFile props(
        juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
            .getChildFile("Studio OS").getChildFile("MixClient.settings"),
        juce::PropertiesFile::Options());
    return props;
}

ReaMarkProcessor::ReaMarkProcessor()
    : AudioProcessor(BusesProperties()
          .withInput("Input",  juce::AudioChannelSet::stereo(), true)
          .withOutput("Output", juce::AudioChannelSet::stereo(), true)) {
    auto& s = globalSettings();
    serverUrl    = s.getValue("serverUrl");
    connectToken = s.getValue("connectToken");
    authorName   = s.getValue("authorName");
}

void ReaMarkProcessor::saveGlobalSettings() {
    auto& s = globalSettings();
    s.setValue("serverUrl",    serverUrl);
    s.setValue("connectToken", connectToken);
    s.setValue("authorName",   authorName);
    s.saveIfNeeded();
}

ReaMarkProcessor::~ReaMarkProcessor() {}

const juce::String ReaMarkProcessor::getName() const { return "Mix Notes"; }
bool ReaMarkProcessor::acceptsMidi() const  { return false; }
bool ReaMarkProcessor::producesMidi() const { return false; }
bool ReaMarkProcessor::isMidiEffect() const { return false; }
double ReaMarkProcessor::getTailLengthSeconds() const { return 0.0; }

int ReaMarkProcessor::getNumPrograms()    { return 1; }
int ReaMarkProcessor::getCurrentProgram() { return 0; }
void ReaMarkProcessor::setCurrentProgram(int index) {
    juce::ignoreUnused(index);
}
const juce::String ReaMarkProcessor::getProgramName(int index) {
    juce::ignoreUnused(index);
    return {};
}
void ReaMarkProcessor::changeProgramName(int index, const juce::String& newName) {
    juce::ignoreUnused(index, newName);
}

void ReaMarkProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    juce::ignoreUnused(sampleRate, samplesPerBlock);
}
void ReaMarkProcessor::releaseResources() {}

void ReaMarkProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) {
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

bool ReaMarkProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* ReaMarkProcessor::createEditor() {
    return new ReaMarkEditor(*this);
}

// ---------------------------------------------------------------------------
// State persistence — save/restore plugin settings in the DAW session
// ---------------------------------------------------------------------------

void ReaMarkProcessor::getStateInformation(juce::MemoryBlock& destData) {
    auto state = std::make_unique<juce::DynamicObject>();
    // server / connectToken / authorName live in globalSettings(), not here.
    state->setProperty("lastShareLink", lastShareLink);
    state->setProperty("autoplayEnabled", autoplayEnabled);

    // Calibration offsets
    auto offsets = std::make_unique<juce::DynamicObject>();
    for (auto& [songId, offset] : calibrationOffsets)
        offsets->setProperty(songId, offset);
    state->setProperty("calibrationOffsets", juce::var(offsets.release()));

    auto jsonStr = juce::JSON::toString(juce::var(state.release()));
    destData.append(jsonStr.toRawUTF8(), jsonStr.getNumBytesAsUTF8());
}

void ReaMarkProcessor::setStateInformation(const void* data, int sizeInBytes) {
    auto jsonStr = juce::String::fromUTF8(static_cast<const char*>(data), sizeInBytes);
    auto state = juce::JSON::parse(jsonStr);

    // server / connectToken / authorName come from globalSettings() (loaded in the ctor).
    lastShareLink  = state.getProperty("lastShareLink", "").toString();
    autoplayEnabled  = static_cast<bool>(state.getProperty("autoplayEnabled", true));

    // Calibration offsets
    auto offsetsVar = state.getProperty("calibrationOffsets", juce::var());
    if (auto* obj = offsetsVar.getDynamicObject()) {
        for (auto& prop : obj->getProperties()) {
            juce::String songId = prop.name.toString();
            double offset = static_cast<double>(prop.value);
            calibrationOffsets[songId] = offset;
        }
    }
}

// Plugin instantiation
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new ReaMarkProcessor();
}
