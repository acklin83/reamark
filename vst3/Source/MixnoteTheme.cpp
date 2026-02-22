#include "MixnoteTheme.h"

namespace mixnote {

MixnoteLookAndFeel::MixnoteLookAndFeel() {
    // Window / general
    setColour(juce::ResizableWindow::backgroundColourId, Theme::bgBody());

    // TextEditor
    setColour(juce::TextEditor::backgroundColourId,    Theme::bgInput());
    setColour(juce::TextEditor::textColourId,          Theme::text());
    setColour(juce::TextEditor::outlineColourId,       Theme::bgBorder());
    setColour(juce::TextEditor::focusedOutlineColourId, Theme::accent());
    setColour(juce::TextEditor::highlightColourId,     Theme::accentDim());
    setColour(juce::CaretComponent::caretColourId,     Theme::text());

    // TextButton
    setColour(juce::TextButton::buttonColourId,   Theme::accent());
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
    setColour(juce::PopupMenu::highlightedBackgroundColourId, Theme::accent());
    setColour(juce::PopupMenu::highlightedTextColourId,    Theme::text());

    // ScrollBar
    setColour(juce::ScrollBar::thumbColourId,      Theme::bgBorder());
    setColour(juce::ScrollBar::trackColourId,      Theme::bgBody());

    // Label
    setColour(juce::Label::textColourId, Theme::text());

    // ToggleButton (Checkbox)
    setColour(juce::ToggleButton::textColourId, Theme::text());
    setColour(juce::ToggleButton::tickColourId, Theme::accent());
    setColour(juce::ToggleButton::tickDisabledColourId, Theme::textMuted());
}

void MixnoteLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                               const juce::Colour& backgroundColour,
                                               bool shouldDrawButtonAsHighlighted,
                                               bool shouldDrawButtonAsDown) {
    juce::ignoreUnused(button);
    auto bounds = button.getLocalBounds().toFloat().reduced(0.5f);
    auto baseColour = backgroundColour;

    if (shouldDrawButtonAsDown)
        baseColour = baseColour.darker(0.2f);
    else if (shouldDrawButtonAsHighlighted)
        baseColour = baseColour.brighter(0.1f);

    g.setColour(baseColour);
    g.fillRoundedRectangle(bounds, 4.0f);
}

void MixnoteLookAndFeel::drawTextEditorOutline(juce::Graphics& g, int width, int height,
                                                juce::TextEditor& editor) {
    auto bounds = juce::Rectangle<float>(0, 0, (float)width, (float)height);
    auto colour = editor.hasKeyboardFocus(true)
                    ? Theme::accent()
                    : Theme::bgBorder();
    g.setColour(colour);
    g.drawRoundedRectangle(bounds.reduced(0.5f), 4.0f, 1.0f);
}

void MixnoteLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height,
                                       bool isButtonDown, int, int, int, int,
                                       juce::ComboBox& box) {
    juce::ignoreUnused(isButtonDown, box);
    auto bounds = juce::Rectangle<float>(0, 0, (float)width, (float)height);
    g.setColour(Theme::bgInput());
    g.fillRoundedRectangle(bounds, 4.0f);
    g.setColour(Theme::bgBorder());
    g.drawRoundedRectangle(bounds.reduced(0.5f), 4.0f, 1.0f);

    // Arrow
    auto arrowZone = juce::Rectangle<float>((float)width - 20.0f, 0, 16.0f, (float)height);
    juce::Path arrow;
    arrow.addTriangle(arrowZone.getX(), arrowZone.getCentreY() - 3.0f,
                      arrowZone.getRight(), arrowZone.getCentreY() - 3.0f,
                      arrowZone.getCentreX(), arrowZone.getCentreY() + 3.0f);
    g.setColour(Theme::textDim());
    g.fillPath(arrow);
}

void MixnoteLookAndFeel::drawPopupMenuBackground(juce::Graphics& g, int width, int height) {
    g.setColour(Theme::bgCard());
    g.fillRoundedRectangle(0.0f, 0.0f, (float)width, (float)height, 4.0f);
    g.setColour(Theme::bgBorder());
    g.drawRoundedRectangle(0.5f, 0.5f, (float)width - 1.0f, (float)height - 1.0f, 4.0f, 1.0f);
}

void MixnoteLookAndFeel::drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
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
        g.setColour(Theme::accent());
        g.fillRoundedRectangle(area.reduced(2).toFloat(), 3.0f);
    }

    g.setColour(isActive ? Theme::text() : Theme::textMuted());
    g.drawFittedText(text, area.reduced(8, 0), juce::Justification::centredLeft, 1);

    if (isTicked) {
        auto tickArea = area.withTrimmedLeft(area.getWidth() - area.getHeight()).reduced(6);
        g.setColour(Theme::accent());
        g.fillEllipse(tickArea.toFloat());
    }
}

void MixnoteLookAndFeel::drawScrollbar(juce::Graphics& g, juce::ScrollBar&,
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

void MixnoteLookAndFeel::drawLabel(juce::Graphics& g, juce::Label& label) {
    g.fillAll(label.findColour(juce::Label::backgroundColourId));

    auto textArea = juce::BorderSize<int>(label.getBorderSize()).subtractedFrom(label.getLocalBounds());
    g.setColour(label.findColour(juce::Label::textColourId));
    g.setFont(label.getFont());
    g.drawFittedText(label.getText(), textArea, label.getJustificationType(),
                     juce::jmax(1, (int)((float)textArea.getHeight() / label.getFont().getHeight())),
                     label.getMinimumHorizontalScale());
}

} // namespace mixnote
