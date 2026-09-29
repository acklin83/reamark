#include "ReaMarkTheme.h"

namespace reamark {

// --- Per-tenant accent ------------------------------------------------------
// Standard ist das Petrol des Themes Console+ (--tide #11ffe5). Ein Studio mit eigener Farbe
// setzt sie per Theme::setAccent(), sobald GET /api/studio antwortet. Der alte Standard
// #3fd9c8 zählt NICHT als eigene Farbe (dieselbe Regel wie frontend/src/app/brand.js).
namespace Theme {
    static juce::Colour g_accent { 0xFF11FFE5 };
    juce::Colour accent()      { return g_accent; }
    juce::Colour accentHover() { return g_accent.brighter(0.06f); }
    juce::Colour accentDim()   { return g_accent.withAlpha(0.10f); }
    juce::Colour accentLine()  { return g_accent.withAlpha(0.30f); }
    void setAccent(juce::Colour c) {
        g_accent = (c.getARGB() == 0xFF3FD9C8) ? juce::Colour(0xFF11FFE5) : c;
    }
}

namespace {
    juce::String rolleVon(juce::Button& b, juce::Colour hintergrund) {
        auto r = b.getProperties()["rolle"].toString();
        if (r.isNotEmpty()) return r;
        if (hintergrund == reamark::Theme::accent()) return "primaer";
        if (hintergrund.getAlpha() == 0) return "link";
        return "neben";
    }
}

ReaMarkLookAndFeel::ReaMarkLookAndFeel() {
    // Window / general
    setColour(juce::ResizableWindow::backgroundColourId, Theme::bgBody());

    // TextEditor (.inp: --sunken, Rahmen --line-strong)
    setColour(juce::TextEditor::backgroundColourId,    Theme::bgInput());
    setColour(juce::TextEditor::textColourId,          Theme::text());
    setColour(juce::TextEditor::outlineColourId,       Theme::bgBorder());
    setColour(juce::CaretComponent::caretColourId,     Theme::text());

    // TextButton
    setColour(juce::TextButton::textColourOnId,   Theme::text());
    setColour(juce::TextButton::textColourOffId,  Theme::text());

    // ComboBox
    setColour(juce::ComboBox::backgroundColourId,  Theme::bgInput());
    setColour(juce::ComboBox::textColourId,        Theme::text());
    setColour(juce::ComboBox::outlineColourId,     Theme::bgBorder());
    setColour(juce::ComboBox::arrowColourId,       Theme::textDim());

    // PopupMenu
    setColour(juce::PopupMenu::backgroundColourId,         Theme::bgCard());
    setColour(juce::PopupMenu::textColourId,               Theme::text());
    setColour(juce::PopupMenu::highlightedTextColourId,    Theme::text());

    // ScrollBar
    setColour(juce::ScrollBar::thumbColourId,      Theme::bgBorder());
    setColour(juce::ScrollBar::trackColourId,      Theme::bgBody());

    // Label
    setColour(juce::Label::textColourId, Theme::text());

    // ToggleButton (Checkbox)
    setColour(juce::ToggleButton::textColourId, Theme::text());
    setColour(juce::ToggleButton::tickDisabledColourId, Theme::textMuted());

    applyAccentColours();
}

void ReaMarkLookAndFeel::applyAccentColours() {
    setColour(juce::TextEditor::focusedOutlineColourId,       Theme::accent());
    setColour(juce::TextEditor::highlightColourId,            Theme::accent().withAlpha(0.25f));
    setColour(juce::TextButton::buttonColourId,               Theme::accent());
    setColour(juce::PopupMenu::highlightedBackgroundColourId, Theme::accentDim());
    setColour(juce::ToggleButton::tickColourId,               Theme::accent());
}

void ReaMarkLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                               const juce::Colour& backgroundColour,
                                               bool shouldDrawButtonAsHighlighted,
                                               bool shouldDrawButtonAsDown) {
    auto bounds = button.getLocalBounds().toFloat().reduced(0.5f);
    auto rolle = rolleVon(button, backgroundColour);
    const float gedrueckt = shouldDrawButtonAsDown ? 1.0f : 0.0f;   // .btn:active: 1 px tiefer
    bounds.translate(0.0f, gedrueckt);

    if (rolle == "link")
        return;
    if (rolle == "ansicht") {
        // Filter wie .ansicht im Web: nur die gewählte trägt Fläche und Kante.
        if (button.getProperties()["an"]) {
            g.setColour(Theme::accentDim());
            g.fillRoundedRectangle(bounds, Theme::radiusBtn);
            g.setColour(Theme::accentLine());
            g.drawRoundedRectangle(bounds.reduced(0.5f), Theme::radiusBtn, 1.0f);
        }
        return;
    }
    if (rolle == "pille") {
        // @1:23 wie im Web: Petrol-Text auf --tide-bg, Kante --tide-line, ganz rund.
        g.setColour(Theme::accentDim());
        g.fillRoundedRectangle(bounds, bounds.getHeight() * 0.5f);
        g.setColour(Theme::accentLine());
        g.drawRoundedRectangle(bounds.reduced(0.5f), bounds.getHeight() * 0.5f, 1.0f);
        return;
    }
    if (rolle == "primaer") {
        g.setColour(shouldDrawButtonAsHighlighted ? Theme::accentHover() : Theme::accent());
        g.fillRoundedRectangle(bounds, Theme::radiusBtn);
        return;
    }
    // neben (.btn.ghost): --panel-2, Hover --panel-3, Rahmen --line-strong
    g.setColour(shouldDrawButtonAsHighlighted ? Theme::bgPanel3() : Theme::bgPanel2());
    g.fillRoundedRectangle(bounds, Theme::radiusBtn);
    g.setColour(Theme::bgBorder());
    g.drawRoundedRectangle(bounds.reduced(0.5f), Theme::radiusBtn, 1.0f);
}

void ReaMarkLookAndFeel::drawButtonText(juce::Graphics& g, juce::TextButton& button,
                                         bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) {
    auto rolle = rolleVon(button, button.findColour(juce::TextButton::buttonColourId));
    // Auf dem gefüllten Petrol steht dunkler Text (--on-tide), sonst die Farbe des Knopfs.
    auto farbe = rolle == "primaer" ? Theme::onAccent()
               : button.findColour(button.getToggleState() ? juce::TextButton::textColourOnId
                                                           : juce::TextButton::textColourOffId);
    if (rolle == "ansicht")
        farbe = (button.getProperties()["an"] || shouldDrawButtonAsHighlighted) ? Theme::text() : Theme::textDim();
    if (rolle == "link" && shouldDrawButtonAsHighlighted)
        farbe = Theme::text();
    if (!button.isEnabled())
        farbe = farbe.withMultipliedAlpha(0.45f);
    g.setFont(getTextButtonFont(button, button.getHeight()));
    g.setColour(farbe);
    auto area = button.getLocalBounds().reduced(rolle == "link" ? 0 : 6, 0);
    if (shouldDrawButtonAsDown) area.translate(0, 1);
    g.drawFittedText(button.getButtonText(), area,
                     rolle == "link" ? juce::Justification::centredLeft : juce::Justification::centred, 1);
}

juce::Font ReaMarkLookAndFeel::getTextButtonFont(juce::TextButton& button, int buttonHeight) {
    auto rolle = rolleVon(button, button.findColour(juce::TextButton::buttonColourId));
    if (rolle == "pille")   // Zeitmarken in Mono wie .timecode
        return juce::Font(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 12.0f, juce::Font::plain));
    if (rolle == "link")    // .cmt-act: 11 px
        return juce::Font(juce::FontOptions(11.5f));
    return juce::Font(juce::FontOptions(juce::jmin(13.0f, (float)buttonHeight * 0.55f), juce::Font::bold));
}

void ReaMarkLookAndFeel::fillTextEditorBackground(juce::Graphics& g, int width, int height,
                                                   juce::TextEditor& editor) {
    g.setColour(editor.findColour(juce::TextEditor::backgroundColourId));
    g.fillRoundedRectangle(0.0f, 0.0f, (float)width, (float)height, Theme::radiusSm);
}

void ReaMarkLookAndFeel::drawTextEditorOutline(juce::Graphics& g, int width, int height,
                                                juce::TextEditor& editor) {
    auto bounds = juce::Rectangle<float>(0, 0, (float)width, (float)height);
    const bool fokus = editor.hasKeyboardFocus(true);
    g.setColour(fokus ? Theme::accent() : Theme::bgBorder());
    g.drawRoundedRectangle(bounds.reduced(fokus ? 1.0f : 0.5f), Theme::radiusSm, fokus ? 2.0f : 1.0f);
}

void ReaMarkLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height,
                                       bool isButtonDown, int, int, int, int,
                                       juce::ComboBox& box) {
    juce::ignoreUnused(isButtonDown, box);
    auto bounds = juce::Rectangle<float>(0, 0, (float)width, (float)height);
    g.setColour(Theme::bgInput());
    g.fillRoundedRectangle(bounds, Theme::radiusSm);
    g.setColour(Theme::bgBorder());
    g.drawRoundedRectangle(bounds.reduced(0.5f), Theme::radiusSm, 1.0f);

    // Arrow — a small centred caret. The old triangle spanned the full 16px zone, which
    // read as too wide/flat; an 8×5 caret sits better next to the text.
    auto cx = (float)width - 12.0f;
    auto cy = (float)height * 0.5f;
    juce::Path arrow;
    arrow.addTriangle(cx - 4.0f, cy - 2.5f,
                      cx + 4.0f, cy - 2.5f,
                      cx,        cy + 2.5f);
    g.setColour(Theme::textDim());
    g.fillPath(arrow);
}

void ReaMarkLookAndFeel::drawPopupMenuBackground(juce::Graphics& g, int width, int height) {
    g.setColour(Theme::bgPanel2());
    g.fillRoundedRectangle(0.0f, 0.0f, (float)width, (float)height, 10.0f);
    g.setColour(Theme::bgBorder());
    g.drawRoundedRectangle(0.5f, 0.5f, (float)width - 1.0f, (float)height - 1.0f, 10.0f, 1.0f);
}

void ReaMarkLookAndFeel::drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
                                            bool isSeparator, bool isActive, bool isHighlighted,
                                            bool isTicked, bool, const juce::String& text,
                                            const juce::String&, const juce::Drawable*,
                                            const juce::Colour*) {
    if (isSeparator) {
        g.setColour(Theme::bgBorder());
        g.fillRect(area.reduced(4, 0).withHeight(1));
        return;
    }

    if (isHighlighted && isActive) {
        g.setColour(Theme::accentDim());
        g.fillRoundedRectangle(area.reduced(3, 1).toFloat(), 6.0f);
    }

    g.setColour(isActive ? Theme::text() : Theme::textMuted());
    g.drawFittedText(text, area.reduced(8, 0), juce::Justification::centredLeft, 1);

    if (isTicked) {
        auto tickArea = area.withTrimmedLeft(area.getWidth() - area.getHeight()).reduced(6);
        g.setColour(Theme::accent());
        g.fillEllipse(tickArea.toFloat());
    }
}

void ReaMarkLookAndFeel::drawScrollbar(juce::Graphics& g, juce::ScrollBar&,
                                        int x, int y, int width, int height,
                                        bool isScrollbarVertical, int thumbStartPosition,
                                        int thumbSize, bool isMouseOver, bool isMouseDown) {
    g.setColour(Theme::bgBody());
    g.fillRect(x, y, width, height);

    auto thumbColour = isMouseDown ? Theme::textDim()
                     : isMouseOver ? Theme::textMuted()
                     : Theme::bgBorder();
    g.setColour(thumbColour);

    if (isScrollbarVertical)
        g.fillRoundedRectangle((float)x + 1, (float)thumbStartPosition, (float)width - 2, (float)thumbSize, 3.0f);
    else
        g.fillRoundedRectangle((float)thumbStartPosition, (float)y + 1, (float)thumbSize, (float)height - 2, 3.0f);
}

void ReaMarkLookAndFeel::drawLabel(juce::Graphics& g, juce::Label& label) {
    g.fillAll(label.findColour(juce::Label::backgroundColourId));

    auto textArea = juce::BorderSize<int>(label.getBorderSize()).subtractedFrom(label.getLocalBounds());
    g.setColour(label.findColour(juce::Label::textColourId));
    g.setFont(label.getFont());
    g.drawFittedText(label.getText(), textArea, label.getJustificationType(),
                     juce::jmax(1, (int)((float)textArea.getHeight() / label.getFont().getHeight())),
                     label.getMinimumHorizontalScale());
}

} // namespace reamark
