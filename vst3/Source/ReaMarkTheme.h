#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

namespace reamark {

// Studio OS design tokens (studio.css :root, the "Console+" dark theme). The chrome is fixed
// for every studio; only the accent is per-tenant (see setAccent()).
namespace Theme {
    // Studio OS design tokens, Theme „Console+" (studio.css :root, Stand 29.09.2026). Die
    // Grundfarben sind für jedes Studio gleich; nur der Akzent kommt vom Studio (setAccent()).
    inline juce::Colour bgBody()   { return juce::Colour(0xFF020304); }  // --bg
    inline juce::Colour bgStage()  { return juce::Colour(0xFF0A0E16); }  // --stage
    inline juce::Colour bgCard()   { return juce::Colour(0xFF121A27); }  // --panel
    inline juce::Colour bgPanel2() { return juce::Colour(0xFF1A2439); }  // --panel-2 (Nebenknopf)
    inline juce::Colour bgPanel3() { return juce::Colour(0xFF243149); }  // --panel-3 (Nebenknopf, Hover)
    inline juce::Colour bgInput()  { return juce::Colour(0xFF020203); }  // --sunken (Eingabefelder)
    inline juce::Colour line()     { return juce::Colour(0xFF1E2836); }  // --line
    inline juce::Colour bgBorder() { return juce::Colour(0xFF5F7BA0); }  // --line-strong
    inline juce::Colour waveIdle() { return juce::Colour(0xFF5E6777); }  // --wave-idle

    // Akzent: die Farbe des Studios. Steht dort der Standard (#3fd9c8), gilt das Petrol des
    // Themes (--tide #11ffe5), genau wie im Web (frontend/src/app/brand.js).
    juce::Colour accent();
    juce::Colour accentHover();
    juce::Colour accentDim();   // --tide-bg: Fläche des Gewählten
    juce::Colour accentLine();  // --tide-line: Kante des Gewählten
    void setAccent(juce::Colour c);
    inline juce::Colour onAccent() { return bgBody(); }                 // --on-tide

    // Text — --text / --dim / --mute
    inline juce::Colour text()      { return juce::Colour(0xFFF6F8FC); }
    inline juce::Colour textDim()   { return juce::Colour(0xFFAEB6C4); }
    inline juce::Colour textMuted() { return juce::Colour(0xFF98A0AF); }

    // Status (feste Bedeutung): offen = Amber, erledigt = Grün, Fehler = Rot, Stern = Amber
    inline juce::Colour green()  { return juce::Colour(0xFF33F596); }
    inline juce::Colour amber()  { return juce::Colour(0xFFFFB246); }
    inline juce::Colour red()    { return juce::Colour(0xFFF28E8E); }
    inline juce::Colour yellow() { return amber(); }

    // Kommentar-Karten — --amber-bg / --green-bg
    inline juce::Colour cardOpen()   { return juce::Colour(0xFF281D0E); }
    inline juce::Colour cardSolved() { return juce::Colour(0xFF09271A); }

    // Radien — --radius-sm (Felder, Karten) und Knöpfe (8 wie .btn)
    constexpr float radiusSm  = 9.0f;
    constexpr float radiusBtn = 8.0f;
}

// Rolle eines Knopfs wie in Studio OS (studio.css .btn): „primaer" Petrol gefüllt (die eine
// Hauptaktion), „neben" dunkel mit Rahmen (.btn.ghost), „link" nur Text (.cmt-act), „pille" die
// Zeitmarke @1:23. Ohne Angabe: Akzentfarbe = primaer, durchsichtig = link, sonst neben.
inline void setRolle(juce::Component& c, const char* rolle) { c.getProperties().set("rolle", rolle); }

class ReaMarkLookAndFeel : public juce::LookAndFeel_V4 {
public:
    ReaMarkLookAndFeel();

    // Re-apply the accent-dependent ColourIds after the tenant accent loads at runtime.
    void applyAccentColours();

    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
                              const juce::Colour& backgroundColour,
                              bool shouldDrawButtonAsHighlighted,
                              bool shouldDrawButtonAsDown) override;

    void drawButtonText(juce::Graphics& g, juce::TextButton& button,
                        bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

    juce::Font getTextButtonFont(juce::TextButton& button, int buttonHeight) override;

    void fillTextEditorBackground(juce::Graphics& g, int width, int height,
                                  juce::TextEditor& editor) override;

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
