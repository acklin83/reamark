#include "WaveformComponent.h"
#include "MixnoteTheme.h"

namespace mixnote {

WaveformComponent::WaveformComponent() {
    setMouseCursor(juce::MouseCursor::PointingHandCursor);
}

void WaveformComponent::setPeaks(const std::vector<float>& newPeaks, double newDuration) {
    peaks = newPeaks;
    duration = newDuration;
    repaint();
}

void WaveformComponent::setComments(const std::vector<Comment>& newComments) {
    comments = newComments;
    repaint();
}

void WaveformComponent::setPlayheadPosition(double seconds) {
    if (std::abs(playheadPos - seconds) > 0.01) {
        playheadPos = seconds;
        repaint();
    }
}

void WaveformComponent::setOffset(double offset) {
    calibrationOffset = offset;
}

double WaveformComponent::xToTimecode(float x) const {
    if (duration <= 0.0 || getWidth() <= 0) return 0.0;
    double ratio = juce::jlimit(0.0, 1.0, (double)x / (double)getWidth());
    return ratio * duration;
}

float WaveformComponent::timecodeToX(double tc) const {
    if (duration <= 0.0 || getWidth() <= 0) return 0.0f;
    return static_cast<float>((tc / duration) * getWidth());
}

void WaveformComponent::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();

    // Background
    g.setColour(Theme::bgInput());
    g.fillRoundedRectangle(bounds, 4.0f);

    if (peaks.empty() || duration <= 0.0) {
        g.setColour(Theme::textMuted());
        g.drawText("No waveform data", bounds, juce::Justification::centred);
        return;
    }

    float wfW = bounds.getWidth();
    float wfH = bounds.getHeight();
    float centreY = bounds.getY() + wfH * 0.5f;

    // Downsample peaks to pixel width
    int peakCount = static_cast<int>(peaks.size());
    int drawBars = juce::jmin(static_cast<int>(wfW), peakCount);
    if (drawBars <= 0) return;

    float barW = wfW / (float)drawBars;
    float samplesPerBar = (float)peakCount / (float)drawBars;

    // Draw waveform bars
    g.setColour(Theme::accent());
    for (int i = 0; i < drawBars; ++i) {
        int s = static_cast<int>(i * samplesPerBar);
        int e = static_cast<int>((i + 1) * samplesPerBar);
        float peak = 0.0f;
        for (int j = s; j < e && j < peakCount; ++j)
            peak = juce::jmax(peak, peaks[static_cast<size_t>(j)]);

        float h = peak * wfH * 0.45f;
        if (h > 0.5f) {
            float x = bounds.getX() + i * barW;
            g.fillRect(x, centreY - h, barW, h * 2.0f);
        }
    }

    // Comment markers
    for (size_t ci = 0; ci < comments.size(); ++ci) {
        auto& c = comments[ci];
        if (c.timecode >= 0.0 && c.timecode <= duration) {
            float mx = bounds.getX() + timecodeToX(c.timecode);
            auto markerCol = c.solved ? Theme::green() : Theme::amber();

            g.setColour(markerCol);
            g.drawLine(mx, bounds.getY(), mx, bounds.getBottom(), 2.0f);

            // Circle at top
            g.fillEllipse(mx - 4.0f, bounds.getY() + 1.0f, 8.0f, 8.0f);
        }
    }

    // Playhead (relative position = transport - calibration offset)
    double relPos = playheadPos - calibrationOffset;
    if (relPos >= 0.0 && relPos <= duration) {
        float px = bounds.getX() + timecodeToX(relPos);

        g.setColour(juce::Colours::white);
        g.drawLine(px, bounds.getY(), px, bounds.getBottom(), 2.0f);

        // Triangle at top
        juce::Path tri;
        tri.addTriangle(px - 5.0f, bounds.getY() - 6.0f,
                        px + 5.0f, bounds.getY() - 6.0f,
                        px,        bounds.getY());
        g.fillPath(tri);
    }

    // Tooltip for hovered comment
    if (hoveredCommentIdx >= 0 && hoveredCommentIdx < static_cast<int>(comments.size())) {
        auto& hc = comments[static_cast<size_t>(hoveredCommentIdx)];
        float mx = timecodeToX(hc.timecode);

        juce::Font font(juce::FontOptions(12.0f));
        g.setFont(font);

        auto tcStr = "@" + formatTimecode(hc.timecode) + "  " + hc.authorName;
        auto textStr = hc.text;
        if (textStr.length() > 60)
            textStr = textStr.substring(0, 57) + "...";

        juce::GlyphArrangement ga1, ga2;
        ga1.addLineOfText(font, tcStr, 0, 0);
        ga2.addLineOfText(font, textStr, 0, 0);
        float tipW = juce::jmax(ga1.getBoundingBox(0, -1, true).getWidth(), 
                                ga2.getBoundingBox(0, -1, true).getWidth()) + 16.0f;
        float tipH = 36.0f;
        float tipX = juce::jlimit(bounds.getX(), bounds.getRight() - tipW, mx - tipW * 0.5f);
        float tipY = bounds.getBottom() + 2.0f;

        g.setColour(Theme::bgCard());
        g.fillRoundedRectangle(tipX, tipY, tipW, tipH, 4.0f);
        g.setColour(Theme::bgBorder());
        g.drawRoundedRectangle(tipX, tipY, tipW, tipH, 4.0f, 1.0f);

        g.setColour(Theme::accent());
        g.drawText(tcStr, juce::Rectangle<float>(tipX + 8, tipY + 2, tipW - 16, 16),
                   juce::Justification::centredLeft, false);
        g.setColour(Theme::text());
        g.drawText(textStr, juce::Rectangle<float>(tipX + 8, tipY + 18, tipW - 16, 16),
                   juce::Justification::centredLeft, false);
    }
}

void WaveformComponent::mouseDown(const juce::MouseEvent& event) {
    if (duration <= 0.0 || peaks.empty()) return;

    double tc = xToTimecode(static_cast<float>(event.x));
    if (onSeek)
        onSeek(tc);
}

void WaveformComponent::mouseMove(const juce::MouseEvent& event) {
    int newHovered = -1;
    float mouseX = static_cast<float>(event.x);

    for (size_t i = 0; i < comments.size(); ++i) {
        float mx = timecodeToX(comments[i].timecode);
        if (std::abs(mouseX - mx) < 6.0f) {
            newHovered = static_cast<int>(i);
            break;
        }
    }

    if (newHovered != hoveredCommentIdx) {
        hoveredCommentIdx = newHovered;
        repaint();
    }
}

void WaveformComponent::mouseExit(const juce::MouseEvent& event) {
    juce::ignoreUnused(event);
    if (hoveredCommentIdx >= 0) {
        hoveredCommentIdx = -1;
        repaint();
    }
}

} // namespace mixnote
