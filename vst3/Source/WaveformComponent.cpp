#include "WaveformComponent.h"
#include "ReaMarkTheme.h"

namespace reamark {

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
    g.fillRoundedRectangle(bounds, Theme::radiusSm);
    g.setColour(Theme::line());
    g.drawRoundedRectangle(bounds.reduced(0.5f), Theme::radiusSm, 1.0f);

    if (peaks.empty() || duration <= 0.0) {
        g.setColour(Theme::textMuted());
        g.drawText("No waveform data", bounds, juce::Justification::centred);
        return;
    }

    const float headStrip = 12.0f;                      // top strip for the marker pins —
    auto  wfBounds = bounds.withTrimmedTop(headStrip);   // keeps the coloured pins off the
    float wfW = wfBounds.getWidth();                     // (per-tenant) accent waveform
    float wfH = wfBounds.getHeight();
    float centreY = wfBounds.getY() + wfH * 0.5f;

    // Downsample peaks to pixel width
    int peakCount = static_cast<int>(peaks.size());
    int drawBars = juce::jmin(static_cast<int>(wfW), peakCount);
    if (drawBars <= 0) return;

    float barW = wfW / (float)drawBars;
    float samplesPerBar = (float)peakCount / (float)drawBars;

    // Balken wie im Web-Player: bis zur Abspielposition im Akzent, danach --wave-idle.
    double relPlay = playheadPos - calibrationOffset;
    float playX = (relPlay > 0.0 && duration > 0.0) ? bounds.getX() + timecodeToX(juce::jmin(relPlay, duration)) : -1.0f;
    for (int i = 0; i < drawBars; ++i) {
        int s = static_cast<int>(i * samplesPerBar);
        int e = static_cast<int>((i + 1) * samplesPerBar);
        float peak = 0.0f;
        for (int j = s; j < e && j < peakCount; ++j)
            peak = juce::jmax(peak, peaks[static_cast<size_t>(j)]);

        float h = peak * wfH * 0.45f;
        if (h > 0.5f) {
            float x = bounds.getX() + i * barW;
            g.setColour(x < playX ? Theme::accent() : Theme::waveIdle());
            g.fillRect(x, centreY - h, juce::jmax(1.0f, barW - (barW > 3.0f ? 1.0f : 0.0f)), h * 2.0f);
        }
    }

    // Bereiche: leise Fläche über den Balken, im dunklen Streifen eine Linie vom Kopf bis zum
    // Ende (wie Web und Script). Vor den Köpfen, damit die obenauf liegen.
    const float lineY = bounds.getY() + headStrip * 0.5f - 1.0f;
    for (auto& c : comments) {
        if (c.timecodeEnd > c.timecode && c.timecode <= duration) {
            float x1 = bounds.getX() + timecodeToX(c.timecode);
            float x2 = bounds.getX() + timecodeToX(juce::jmin(c.timecodeEnd, duration));
            auto col = c.solved ? Theme::green() : Theme::amber();
            g.setColour(col.withAlpha(0.10f));
            g.fillRect(x1, wfBounds.getY(), x2 - x1, wfH);
            g.setColour(col);
            g.drawLine(x1, lineY, x2, lineY, 2.0f);
        }
    }
    // Die gezogene Auswahl: der Bereich der nächsten Anmerkung.
    if (selA >= 0.0 && selE > selA) {
        float x1 = bounds.getX() + timecodeToX(selA);
        float x2 = bounds.getX() + timecodeToX(selE);
        g.setColour(Theme::accentDim());
        g.fillRect(x1, wfBounds.getY(), x2 - x1, wfH);
        g.setColour(Theme::accent());
        g.drawVerticalLine(juce::roundToInt(x1), wfBounds.getY(), wfBounds.getBottom());
        g.drawVerticalLine(juce::roundToInt(x2), wfBounds.getY(), wfBounds.getBottom());
    }

    // Comment markers
    for (size_t ci = 0; ci < comments.size(); ++ci) {
        auto& c = comments[ci];
        if (c.timecode >= 0.0 && c.timecode <= duration) {
            float mx = bounds.getX() + timecodeToX(c.timecode);
            auto markerCol = c.solved ? Theme::green() : Theme::amber();

            // A pin in the dark strip that points down at the exact spot. NO line through the
            // waveform (that read as a "break"), and the colour never touches the accent.
            float cy = bounds.getY() + headStrip * 0.5f - 1.0f;
            juce::Path pin;
            pin.addTriangle(mx - 3.0f, cy, mx + 3.0f, cy, mx, wfBounds.getY());
            g.setColour(markerCol);
            g.fillPath(pin);
            g.fillEllipse(mx - 4.0f, cy - 4.5f, 8.0f, 8.0f);
        }
    }

    // Playhead (relative position = transport - calibration offset)
    double relPos = playheadPos - calibrationOffset;
    if (relPos >= 0.0 && relPos <= duration) {
        float px = bounds.getX() + timecodeToX(relPos);

        g.setColour(juce::Colours::white);
        g.drawLine(px, wfBounds.getY(), px, wfBounds.getBottom(), 2.0f);

        // Triangle at top
        juce::Path tri;
        tri.addTriangle(px - 5.0f, wfBounds.getY() - 6.0f,
                        px + 5.0f, wfBounds.getY() - 6.0f,
                        px,        wfBounds.getY());
        g.fillPath(tri);
    }

    // Tooltip for hovered comment
    if (hoveredCommentIdx >= 0 && hoveredCommentIdx < static_cast<int>(comments.size())) {
        auto& hc = comments[static_cast<size_t>(hoveredCommentIdx)];
        float mx = timecodeToX(hc.timecode);

        juce::Font font(juce::FontOptions(12.0f));
        g.setFont(font);

        auto tcStr = formatTimeRange(hc.timecode, hc.timecodeEnd) + "  " + hc.authorName;
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

        g.setColour(Theme::bgPanel2());
        g.fillRoundedRectangle(tipX, tipY, tipW, tipH, 8.0f);
        g.setColour(Theme::bgBorder());
        g.drawRoundedRectangle(tipX, tipY, tipW, tipH, 8.0f, 1.0f);

        g.setColour(Theme::accent());
        g.drawText(tcStr, juce::Rectangle<float>(tipX + 8, tipY + 2, tipW - 16, 16),
                   juce::Justification::centredLeft, false);
        g.setColour(Theme::text());
        g.drawText(textStr, juce::Rectangle<float>(tipX + 8, tipY + 18, tipW - 16, 16),
                   juce::Justification::centredLeft, false);
    }
}

void WaveformComponent::clearRange() {
    selA = selE = -1.0;
    repaint();
}

// Klick springt (beim Loslassen), Ziehen ab 5 px markiert einen Bereich und springt nicht.
void WaveformComponent::mouseDown(const juce::MouseEvent& event) {
    dragStartX = static_cast<float>(event.x);
    dragging = false;
}

void WaveformComponent::mouseDrag(const juce::MouseEvent& event) {
    if (duration <= 0.0 || peaks.empty()) return;
    if (!dragging && std::abs(static_cast<float>(event.x) - dragStartX) < 5.0f) return;
    dragging = true;
    double t0 = xToTimecode(dragStartX), t1 = xToTimecode(static_cast<float>(event.x));
    selA = juce::jmin(t0, t1);
    selE = juce::jmax(t0, t1);
    repaint();
}

void WaveformComponent::mouseUp(const juce::MouseEvent& event) {
    if (duration <= 0.0 || peaks.empty()) return;
    if (dragging) {
        dragging = false;
        // Der Server speichert ganze Sekunden: was darin zusammenfällt, ist kein Bereich.
        if (std::floor(selE) <= std::floor(selA))
            selA = selE = -1.0;
        repaint();
        if (onRangeChanged) onRangeChanged(selA, selE);
        return;
    }
    if (onSeek)
        onSeek(xToTimecode(static_cast<float>(event.x)));
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

} // namespace reamark
