#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

namespace reamark {

// Studio OS design tokens (studio.css :root, the "Console+" dark theme). The chrome is fixed
// for every studio; only the accent is per-tenant (see setAccent()).
namespace Theme {
    // Backgrounds — --bg / --panel / --panel-2 / --line-strong
    inline juce::Colour bgBody()   { return juce::Colour(0xFF0B0D10); }
    inline juce::Colour bgCard()   { return juce::Colour(0xFF111419); }
    inline juce::Colour bgInput()  { return juce::Colour(0xFF171B21); }
    inline juce::Colour bgBorder() { return juce::Colour(0xFF3B4450); }

    // Accent — the studio's brand colour (--tide, default petrol #3fd9c8). Per-tenant:
    // fetched from GET {server}/api/studio and applied via setAccent().
    juce::Colour accent();
    juce::Colour accentHover();
    juce::Colour accentDim();
    void setAccent(juce::Colour c);

    // Text — --text / --dim / --mute
    inline juce::Colour text()      { return juce::Colour(0xFFF5F7FB); }
    inline juce::Colour textDim()   { return juce::Colour(0xFFADB4C2); }
    inline juce::Colour textMuted() { return juce::Colour(0xFF868E9D); }

    // Status (fixed semantics) — open=amber, resolved=green, error=red; favourite star=amber
    inline juce::Colour green()  { return juce::Colour(0xFF5FD39A); }
    inline juce::Colour amber()  { return juce::Colour(0xFFF0B45F); }
    inline juce::Colour red()    { return juce::Colour(0xFFF07A7A); }
    inline juce::Colour yellow() { return juce::Colour(0xFFF0B45F); }

    // Comment card backgrounds — --amber-bg / --green-bg, translucent
    inline juce::Colour cardOpen()   { return juce::Colour(0x802C2413); }
    inline juce::Colour cardSolved() { return juce::Colour(0x6014261D); }
}

class ReaMarkLookAndFeel : public juce::LookAndFeel_V4 {
public:
    ReaMarkLookAndFeel();

    // Re-apply the accent-dependent ColourIds after the tenant accent loads at runtime.
    void applyAccentColours();

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

} // namespace reamark
