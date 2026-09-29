#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "ReaMarkModels.h"
#include "ReaMarkTheme.h"

namespace reamark {

// Die Versionen eines Songs als Chips, wie auf der Mix-Notes-Seite von Studio OS (.mx-ver):
// „v1  v2 1  v3 2 ★". Die Zahl der offenen Anmerkungen steht in Amber, der Stern an der
// Lieblingsfassung, die gewählte Version trägt Petrol-Fläche und -Kante. Bricht um, wenn der
// Platz nicht reicht (heightFor), statt Chips abzuschneiden.
class VersionChips : public juce::Component {
public:
    std::function<void(int)> onSelect;

    void setVersions(const std::vector<Version>& vs, int selected) {
        versions = vs;
        sel = selected;
        layoutChips(getWidth());
        repaint();
    }

    int heightFor(int width) {
        auto rows = layoutChips(width);
        return rows * chipH + juce::jmax(0, rows - 1) * gap;
    }

    void resized() override { layoutChips(getWidth()); }

    void paint(juce::Graphics& g) override {
        for (size_t i = 0; i < versions.size() && i < rects.size(); ++i) {
            const auto& v = versions[i];
            auto r = rects[i].toFloat().reduced(0.5f);
            const bool on = (int)i == sel, hover = (int)i == hov;
            if (on) {
                g.setColour(Theme::accentDim());
                g.fillRoundedRectangle(r, radius);
                g.setColour(Theme::accentLine());
            } else {
                g.setColour(hover ? Theme::bgBorder() : Theme::line());
            }
            g.drawRoundedRectangle(r, radius, 1.0f);

            float x = r.getX() + padX;
            auto teil = [&](const juce::String& t, juce::Colour c) {
                g.setColour(c);
                g.setFont(font());
                float w = textW(t);
                g.drawText(t, juce::Rectangle<float>(x, r.getY(), w + 1.0f, r.getHeight()),
                           juce::Justification::centredLeft, false);
                x += w + innerGap;
            };
            teil("v" + juce::String(v.versionNumber), on || hover ? Theme::text() : Theme::textDim());
            if (v.openCount > 0) teil(juce::String(v.openCount), Theme::amber());
            if (v.favourite)     teil(juce::String::fromUTF8("\xe2\x98\x85"), Theme::amber());
        }
    }

    void mouseMove(const juce::MouseEvent& e) override { setHover(indexAt(e.getPosition())); }
    void mouseExit(const juce::MouseEvent&) override { setHover(-1); }
    void mouseUp(const juce::MouseEvent& e) override {
        auto i = indexAt(e.getPosition());
        if (i >= 0 && onSelect) onSelect(i);
    }

private:
    static constexpr int chipH = 24, gap = 4, padX = 7;
    static constexpr float radius = 6.0f, innerGap = 4.0f;
    std::vector<Version> versions;
    std::vector<juce::Rectangle<int>> rects;
    int sel = -1, hov = -1;

    static juce::Font font() {
        return juce::Font(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 11.5f, juce::Font::bold));
    }
    static float textW(const juce::String& t) { return juce::GlyphArrangement::getStringWidth(font(), t); }

    int chipW(const Version& v) const {
        float w = textW("v" + juce::String(v.versionNumber));
        if (v.openCount > 0) w += innerGap + textW(juce::String(v.openCount));
        if (v.favourite)     w += innerGap + textW(juce::String::fromUTF8("\xe2\x98\x85"));
        return (int)std::ceil(w) + 2 * padX;
    }

    // Legt die Chips von links nach rechts und bricht um; gibt die Zahl der Zeilen zurück.
    int layoutChips(int width) {
        rects.clear();
        if (versions.empty()) return 1;
        int x = 0, y = 0, rows = 1;
        for (auto& v : versions) {
            int w = chipW(v);
            if (x > 0 && width > 0 && x + w > width) { x = 0; y += chipH + gap; ++rows; }
            rects.emplace_back(x, y, w, chipH);
            x += w + gap;
        }
        return rows;
    }

    int indexAt(juce::Point<int> p) const {
        for (size_t i = 0; i < rects.size(); ++i)
            if (rects[i].contains(p)) return (int)i;
        return -1;
    }

    void setHover(int i) {
        if (i == hov) return;
        hov = i;
        setMouseCursor(i >= 0 ? juce::MouseCursor::PointingHandCursor : juce::MouseCursor::NormalCursor);
        repaint();
    }
};

} // namespace reamark
