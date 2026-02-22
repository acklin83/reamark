#pragma once
#include <JuceHeader.h>
#include "MixnoteModels.h"
#include <vector>

namespace mixnote {

class WaveformComponent : public juce::Component {
public:
    WaveformComponent();

    void setPeaks(const std::vector<float>& peaks, double duration);
    void setComments(const std::vector<Comment>& comments);
    void setPlayheadPosition(double seconds);  // relative to song start
    void setOffset(double offset);

    // Callback when user clicks on waveform (timecode relative to song start)
    std::function<void(double timecode)> onSeek;

    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseMove(const juce::MouseEvent& event) override;
    void mouseExit(const juce::MouseEvent& event) override;

private:
    std::vector<float> peaks;
    double duration = 0.0;
    std::vector<Comment> comments;
    double playheadPos = 0.0;
    double calibrationOffset = 0.0;

    int hoveredCommentIdx = -1;  // index into comments vector, -1 = none

    double xToTimecode(float x) const;
    float timecodeToX(double tc) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformComponent)
};

} // namespace mixnote
