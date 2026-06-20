#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "PluginProcessor.h"
#include "ReaMarkApi.h"
#include "ReaMarkModels.h"
#include "ReaMarkTheme.h"
#include "WaveformComponent.h"
#include "CommentListComponent.h"

class ReaMarkEditor : public juce::AudioProcessorEditor,
                      private juce::Timer {
public:
    explicit ReaMarkEditor(ReaMarkProcessor& processor);
    ~ReaMarkEditor() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    ReaMarkProcessor& processorRef;
    reamark::ReaMarkApi api;
    reamark::ReaMarkLookAndFeel reamarkLnf;

    // --- State ---
    bool loggedIn = false;
    juce::String errorMsg;

    reamark::Project currentProject;
    std::vector<reamark::AdminProject> adminProjects;
    std::vector<reamark::Comment> comments;
    juce::String activeShareLink;

    int selectedSongIdx = -1;
    int selectedVersionIdx = -1;

    // --- Login section ---
    juce::Label serverLabel   { {}, "Server" };
    juce::TextEditor serverInput;
    juce::Label userLabel     { {}, "User" };
    juce::TextEditor userInput;
    juce::Label passLabel     { {}, "Password" };
    juce::TextEditor passInput;
    juce::ToggleButton rememberCheck { "Remember me" };
    juce::TextButton loginBtn       { "Login" };
    juce::TextButton logoutBtn      { "Logout" };
    juce::Label loginStatusLabel;
    juce::Label loginErrorLabel;

    // --- Project section ---
    juce::Label projectLabel { {}, "Project" };
    juce::ComboBox projectCombo;

    // --- Song / Version section ---
    juce::ComboBox songCombo;
    juce::ComboBox versionCombo;
    juce::TextButton favouriteBtn { "" };
    juce::Label offsetLabel;
    juce::TextButton setOffsetBtn { "Set from Cursor" };
    juce::ToggleButton autoplayCheck { "Autoplay" };

    // --- Waveform ---
    reamark::WaveformComponent waveform;

    // --- New comment ---
    juce::TextEditor authorInput;
    juce::Label timecodeLabel;
    juce::TextEditor commentInput;
    juce::TextButton addCommentBtn { "Add" };

    // --- Comment list ---
    reamark::CommentListComponent commentList;

    // --- Error bar ---
    juce::Label errorLabel;

    // --- Separator helpers ---
    void drawSeparator(juce::Graphics& g, int y);

    // --- Section visibility ---
    bool isProjectLoaded() const { return !currentProject.songs.empty(); }

    // --- Actions ---
    void doLogin();
    void doLogout();
    void loadProject(const juce::String& shareLink);
    void onProjectSelected();
    void onSongSelected();
    void onVersionSelected();
    void loadComments();
    void loadPeaks();
    void doCreateComment();
    void doSetOffset();
    void doToggleFavourite();

    // --- Helpers ---
    void updateSongCombo();
    void updateVersionCombo();
    void updateOffsetDisplay();
    void updateTimecodeDisplay();
    void showError(const juce::String& msg);
    double getCurrentOffset() const;

    const reamark::Song* getSelectedSong() const;
    const reamark::Version* getSelectedVersion() const;

    // Seek: stores clicked timecode (absolute) when transport is stopped
    double manualSeekPos = -1.0;

    void seekTo(double relativeTimecode);

    // Timer for playhead updates
    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ReaMarkEditor)
};
