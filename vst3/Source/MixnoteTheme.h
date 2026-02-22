#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

namespace mixnote {

// Colours matching Mixnote website dark theme
namespace Theme {
    // Backgrounds (4-level hierarchy)
    inline juce::Colour bgBody()   { return juce::Colour(0xFF0F0F0F); }
    inline juce::Colour bgCard()   { return juce::Colour(0xFF1A1A1A); }
    inline juce::Colour bgInput()  { return juce::Colour(0xFF2A2A2A); }
    inline juce::Colour bgBorder() { return juce::Colour(0xFF3A3A3A); }

    // Accent (Indigo)
    inline juce::Colour accent()      { return juce::Colour(0xFF6366F1); }
    inline juce::Colour accentHover() { return juce::Colour(0xFF5558E8); }
    inline juce::Colour accentDim()   { return juce::Colour(0xFF6366F1).withAlpha(0.25f); }

    // Text
    inline juce::Colour text()      { return juce::Colour(0xFFE5E7EB); }
    inline juce::Colour textDim()   { return juce::Colour(0xFF9CA3AF); }
    inline juce::Colour textMuted() { return juce::Colour(0xFF6B7280); }

    // Status
    inline juce::Colour green()  { return juce::Colour(0xFF4ADE80); }
    inline juce::Colour amber()  { return juce::Colour(0xFFF59E0B); }
    inline juce::Colour red()    { return juce::Colour(0xFFEF4444); }
    inline juce::Colour yellow() { return juce::Colour(0xFFFBBF24); }

    // Comment card backgrounds
    inline juce::Colour cardOpen()   { return juce::Colour(0x801E2333); }
    inline juce::Colour cardSolved() { return juce::Colour(0x601A2A1A); }
}

class MixnoteLookAndFeel : public juce::LookAndFeel_V4 {
public:
    MixnoteLookAndFeel();

    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
                              const juce::Colour& backgroundColour,
                              bool shouldDrawButtonAsHighlighted,
                              bool shouldDrawButtonAsDown) override;

    void drawTextEditorOutline(juce::Graphics& g, int width, int height,
                               juce::TextEditor& editor) override;

    void drawComboBox(juce::Graphics& g, int width, int height,
                      bool isButtonDown, int buttonX, int buttonY,
                      int buttonW, int buttonH,
                      juce::ComboBox& box) override;

    void drawPopupMenuBackground(juce::Graphics& g, int width, int height) override;

    void drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
                           bool isSeparator, bool isActive, bool isHighlighted,
                           bool isTicked, bool hasSubMenu,
                           const juce::String& text, const juce::String& shortcutKeyText,
                           const juce::Drawable* icon, const juce::Colour* textColour) override;

    void drawScrollbar(juce::Graphics& g, juce::ScrollBar& scrollbar,
                       int x, int y, int width, int height,
                       bool isScrollbarVertical, int thumbStartPosition,
                       int thumbSize, bool isMouseOver,
                       bool isMouseDown) override;

    void drawLabel(juce::Graphics& g, juce::Label& label) override;
};

} // namespace mixnote
