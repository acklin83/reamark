#include "PluginEditor.h"

using namespace reamark;

ReaMarkEditor::ReaMarkEditor(ReaMarkProcessor& p)
    : AudioProcessorEditor(p), processorRef(p) {
    setLookAndFeel(&reamarkLnf);
    setSize(420, 700);
    setResizable(true, true);
    setResizeLimits(420, 300, 9999, 9999);

    // Restore state from processor
    api.setServerUrl(processorRef.serverUrl);
    serverInput.setText(processorRef.serverUrl);
    userInput.setText(processorRef.username);
    authorInput.setText(processorRef.authorName.isNotEmpty() ? processorRef.authorName : processorRef.username);
    passInput.setText(processorRef.savedPassword);
    rememberCheck.setToggleState(processorRef.rememberPassword, juce::dontSendNotification);
    autoplayCheck.setToggleState(processorRef.autoplayEnabled, juce::dontSendNotification);

    // --- Login section ---
    for (auto* lbl : { &serverLabel, &userLabel, &passLabel }) {
        lbl->setColour(juce::Label::textColourId, Theme::textDim());
        addAndMakeVisible(lbl);
    }
    passInput.setPasswordCharacter('*');
    for (auto* editor : { &serverInput, &userInput, &passInput }) {
        addAndMakeVisible(editor);
    }
    addAndMakeVisible(rememberCheck);
    addAndMakeVisible(loginBtn);
    addChildComponent(logoutBtn);
    addChildComponent(loginStatusLabel);
    addAndMakeVisible(loginErrorLabel);

    loginBtn.onClick = [this]() { doLogin(); };
    logoutBtn.onClick = [this]() { doLogout(); };
    logoutBtn.setColour(juce::TextButton::buttonColourId, Theme::bgInput());

    loginStatusLabel.setColour(juce::Label::textColourId, Theme::green());
    loginErrorLabel.setColour(juce::Label::textColourId, Theme::red());

    // --- Project section ---
    projectLabel.setColour(juce::Label::textColourId, Theme::textDim());
    addChildComponent(projectLabel);
    addChildComponent(projectCombo);
    projectCombo.onChange = [this]() { onProjectSelected(); };

    // --- Song / Version ---
    addChildComponent(songCombo);
    addChildComponent(versionCombo);
    addChildComponent(favouriteBtn);
    addChildComponent(offsetLabel);
    addChildComponent(setOffsetBtn);
    addChildComponent(autoplayCheck);

    songCombo.onChange = [this]() { onSongSelected(); };
    versionCombo.onChange = [this]() { onVersionSelected(); };

    favouriteBtn.setColour(juce::TextButton::buttonColourId, Theme::bgInput());
    favouriteBtn.onClick = [this]() { doToggleFavourite(); };

    offsetLabel.setColour(juce::Label::textColourId, Theme::textMuted());
    setOffsetBtn.setColour(juce::TextButton::buttonColourId, Theme::bgInput());
    setOffsetBtn.setColour(juce::TextButton::textColourOffId, Theme::textDim());
    setOffsetBtn.onClick = [this]() { doSetOffset(); };

    autoplayCheck.onStateChange = [this]() {
        processorRef.autoplayEnabled = autoplayCheck.getToggleState();
    };

    // --- Waveform ---
    addChildComponent(waveform);
    waveform.onSeek = [this](double timecode) {
        seekTo(timecode);
    };

    // --- New comment ---
    addChildComponent(authorInput);
    addChildComponent(timecodeLabel);
    addChildComponent(commentInput);
    addChildComponent(addCommentBtn);

    timecodeLabel.setColour(juce::Label::textColourId, Theme::accent());
    commentInput.setMultiLine(true);
    commentInput.setReturnKeyStartsNewLine(true);
    addCommentBtn.onClick = [this]() { doCreateComment(); };

    // --- Comment list ---
    addChildComponent(commentList);
    commentList.onTimecodeClick = [this](double timecode) {
        seekTo(timecode);
    };
    commentList.onResolve = [this](int commentId) {
        if (!loggedIn || activeShareLink.isEmpty()) return;
        api.resolveComment(activeShareLink, commentId, [this](bool ok, const juce::String& err) {
            if (ok) loadComments();
            else showError(err);
        });
    };
    commentList.onDelete = [this](int commentId) {
        if (!loggedIn) return;
        api.deleteComment(commentId, [this](bool ok, const juce::String& err) {
            if (ok) loadComments();
            else showError(err);
        });
    };
    commentList.onEdit = [this](int commentId, const juce::String& text) {
        if (!loggedIn) return;
        api.updateComment(commentId, text, [this](bool ok, const juce::String& err) {
            if (ok) loadComments();
            else showError(err);
        });
    };
    commentList.onReply = [this](int commentId, const juce::String& text) {
        if (activeShareLink.isEmpty()) return;
        api.replyToComment(activeShareLink, commentId, authorInput.getText().trim(), text,
            [this](bool ok, const juce::String& err) {
                if (ok) loadComments();
                else showError(err);
            });
    };
    commentList.onRefresh = [this]() {
        if (activeShareLink.isNotEmpty())
            loadComments();
    };

    // --- Error bar ---
    errorLabel.setColour(juce::Label::textColourId, Theme::red());
    addAndMakeVisible(errorLabel);

    // Start timer for playhead updates (30 fps)
    startTimerHz(30);
}

ReaMarkEditor::~ReaMarkEditor() {
    setLookAndFeel(nullptr);
    stopTimer();
}

// ---------------------------------------------------------------------------
// Layout
// ---------------------------------------------------------------------------

void ReaMarkEditor::paint(juce::Graphics& g) {
    g.fillAll(Theme::bgBody());
}

void ReaMarkEditor::resized() {
    auto area = getLocalBounds().reduced(12);
    int rowH = 26;
    int spacing = 6;
    int labelW = 80;

    if (!loggedIn) {
        // --- Login form ---
        serverLabel.setVisible(true);  userLabel.setVisible(true);  passLabel.setVisible(true);
        serverInput.setVisible(true);  userInput.setVisible(true);  passInput.setVisible(true);
        rememberCheck.setVisible(true); loginBtn.setVisible(true);
        logoutBtn.setVisible(false); loginStatusLabel.setVisible(false);

        auto row1 = area.removeFromTop(rowH);
        serverLabel.setBounds(row1.removeFromLeft(labelW));
        serverInput.setBounds(row1);
        area.removeFromTop(spacing);

        auto row2 = area.removeFromTop(rowH);
        userLabel.setBounds(row2.removeFromLeft(labelW));
        userInput.setBounds(row2);
        area.removeFromTop(spacing);

        auto row3 = area.removeFromTop(rowH);
        passLabel.setBounds(row3.removeFromLeft(labelW));
        passInput.setBounds(row3);
        area.removeFromTop(spacing);

        auto row4 = area.removeFromTop(rowH);
        rememberCheck.setBounds(row4.removeFromLeft(130));
        row4.removeFromLeft(4);
        loginBtn.setBounds(row4.removeFromLeft(70));
        area.removeFromTop(spacing);

        loginErrorLabel.setBounds(area.removeFromTop(rowH));

        // Hide project sections
        projectLabel.setVisible(false); projectCombo.setVisible(false);
        songCombo.setVisible(false); versionCombo.setVisible(false);
        favouriteBtn.setVisible(false); offsetLabel.setVisible(false);
        setOffsetBtn.setVisible(false); autoplayCheck.setVisible(false);
        waveform.setVisible(false);
        authorInput.setVisible(false); timecodeLabel.setVisible(false);
        commentInput.setVisible(false); addCommentBtn.setVisible(false);
        commentList.setVisible(false);
    } else {
        // --- Logged in header ---
        serverLabel.setVisible(false); userLabel.setVisible(false); passLabel.setVisible(false);
        serverInput.setVisible(false); userInput.setVisible(false); passInput.setVisible(false);
        rememberCheck.setVisible(false); loginBtn.setVisible(false);
        logoutBtn.setVisible(true); loginStatusLabel.setVisible(true);
        loginErrorLabel.setVisible(false);

        auto headerRow = area.removeFromTop(rowH);
        loginStatusLabel.setBounds(headerRow.removeFromLeft(200));
        logoutBtn.setBounds(headerRow.removeFromRight(65));

        area.removeFromTop(spacing);

        // --- Project dropdown ---
        projectLabel.setVisible(true); projectCombo.setVisible(true);
        auto projRow = area.removeFromTop(rowH);
        projectLabel.setBounds(projRow.removeFromLeft(labelW));
        projectCombo.setBounds(projRow);

        area.removeFromTop(spacing);

        if (isProjectLoaded()) {
            // --- Song / Version ---
            songCombo.setVisible(true); versionCombo.setVisible(true);
            favouriteBtn.setVisible(loggedIn);
            offsetLabel.setVisible(true); setOffsetBtn.setVisible(true);
            autoplayCheck.setVisible(true);

            auto svRow = area.removeFromTop(rowH);
            int songW = static_cast<int>(svRow.getWidth() * 0.55f);
            songCombo.setBounds(svRow.removeFromLeft(songW));
            svRow.removeFromLeft(4);
            if (loggedIn) {
                favouriteBtn.setBounds(svRow.removeFromRight(30));
                svRow.removeFromRight(4);
            }
            versionCombo.setBounds(svRow);

            area.removeFromTop(spacing);

            // Offset + autoplay row
            auto offsetRow = area.removeFromTop(rowH);
            offsetLabel.setBounds(offsetRow.removeFromLeft(140));
            setOffsetBtn.setBounds(offsetRow.removeFromLeft(110));
            offsetRow.removeFromLeft(8);
            autoplayCheck.setBounds(offsetRow.removeFromRight(90));

            area.removeFromTop(spacing);

            // --- Waveform ---
            waveform.setVisible(true);
            waveform.setBounds(area.removeFromTop(56));

            area.removeFromTop(spacing);

            // --- New comment ---
            authorInput.setVisible(true); timecodeLabel.setVisible(true);
            commentInput.setVisible(true); addCommentBtn.setVisible(true);

            auto authorRow = area.removeFromTop(rowH);
            authorInput.setBounds(authorRow.removeFromLeft(120));
            authorRow.removeFromLeft(8);
            timecodeLabel.setBounds(authorRow);

            area.removeFromTop(4);
            auto commentRow = area.removeFromTop(46);
            addCommentBtn.setBounds(commentRow.removeFromRight(60));
            commentRow.removeFromRight(4);
            commentInput.setBounds(commentRow);

            area.removeFromTop(spacing);

            // --- Comment list (fills remaining space) ---
            commentList.setVisible(true);
            commentList.setBounds(area);
        } else {
            songCombo.setVisible(false); versionCombo.setVisible(false);
            favouriteBtn.setVisible(false); offsetLabel.setVisible(false);
            setOffsetBtn.setVisible(false); autoplayCheck.setVisible(false);
            waveform.setVisible(false);
            authorInput.setVisible(false); timecodeLabel.setVisible(false);
            commentInput.setVisible(false); addCommentBtn.setVisible(false);
            commentList.setVisible(false);
        }
    }

    // Error label at very bottom
    errorLabel.setBounds(getLocalBounds().removeFromBottom(20).reduced(12, 0));
}

// ---------------------------------------------------------------------------
// Login / Logout
// ---------------------------------------------------------------------------

void ReaMarkEditor::doLogin() {
    auto server = serverInput.getText().trim();
    auto user = userInput.getText().trim();
    auto pass = passInput.getText();

    if (server.isEmpty() || user.isEmpty()) {
        loginErrorLabel.setText("Server and User required", juce::dontSendNotification);
        return;
    }

    api.setServerUrl(server);
    loginErrorLabel.setText("", juce::dontSendNotification);

    api.login(user, pass, [this, server, user, pass](bool ok, const juce::String& token, const juce::String& err) {
        if (ok) {
            api.setJwtToken(token);
            loggedIn = true;

            // Save to processor
            processorRef.serverUrl = server;
            processorRef.username = user;
            if (processorRef.authorName.isEmpty())
                processorRef.authorName = user;
            processorRef.rememberPassword = rememberCheck.getToggleState();
            if (processorRef.rememberPassword)
                processorRef.savedPassword = pass;

            loginStatusLabel.setText(">> " + user + "  " + server, juce::dontSendNotification);
            authorInput.setText(processorRef.authorName);

            // Load admin projects
            api.loadAdminProjects([this](bool projectsOk, const std::vector<AdminProject>& projects, const juce::String& projectsErr) {
                if (projectsOk) {
                    adminProjects = projects;
                    projectCombo.clear();
                    for (size_t i = 0; i < projects.size(); ++i)
                        projectCombo.addItem(projects[i].title, static_cast<int>(i + 1));

                    // Restore last selected project
                    if (processorRef.lastShareLink.isNotEmpty()) {
                        for (size_t i = 0; i < projects.size(); ++i) {
                            if (projects[i].shareLink == processorRef.lastShareLink) {
                                projectCombo.setSelectedId(static_cast<int>(i + 1), juce::dontSendNotification);
                                loadProject(projects[i].shareLink);
                                break;
                            }
                        }
                    }
                } else {
                    showError(projectsErr);
                }
            });

            resized();
        } else {
            loginErrorLabel.setText(err, juce::dontSendNotification);
        }
    });
}

void ReaMarkEditor::doLogout() {
    loggedIn = false;
    api.setJwtToken({});
    currentProject = {};
    adminProjects.clear();
    comments.clear();
    activeShareLink = {};
    selectedSongIdx = -1;
    selectedVersionIdx = -1;
    errorMsg = {};

    if (!rememberCheck.getToggleState())
        passInput.setText({});

    resized();
}

// ---------------------------------------------------------------------------
// Project loading
// ---------------------------------------------------------------------------

void ReaMarkEditor::loadProject(const juce::String& shareLink) {
    auto code = extractShareCode(shareLink);
    showError({});

    api.loadProject(code, [this, code](bool ok, const Project& project, const juce::String& err) {
        if (ok) {
            currentProject = project;
            activeShareLink = code;
            processorRef.lastShareLink = code;

            selectedSongIdx = currentProject.songs.empty() ? -1 : 0;
            selectedVersionIdx = -1;

            updateSongCombo();

            if (selectedSongIdx >= 0) {
                // Auto-select favourite version or latest
                auto& versions = currentProject.songs[static_cast<size_t>(selectedSongIdx)].versions;
                selectedVersionIdx = versions.empty() ? -1 : static_cast<int>(versions.size()) - 1;
                for (int i = 0; i < static_cast<int>(versions.size()); ++i) {
                    if (versions[static_cast<size_t>(i)].favourite) {
                        selectedVersionIdx = i;
                        break;
                    }
                }
                updateVersionCombo();
                loadComments();
                loadPeaks();
            }

            updateOffsetDisplay();
            resized();
        } else {
            showError(err);
        }
    });
}

void ReaMarkEditor::onProjectSelected() {
    int idx = projectCombo.getSelectedId() - 1;
    if (idx >= 0 && idx < static_cast<int>(adminProjects.size()))
        loadProject(adminProjects[static_cast<size_t>(idx)].shareLink);
}

void ReaMarkEditor::onSongSelected() {
    selectedSongIdx = songCombo.getSelectedId() - 1;
    auto* song = getSelectedSong();
    if (song) {
        auto& versions = song->versions;
        selectedVersionIdx = versions.empty() ? -1 : static_cast<int>(versions.size()) - 1;
        for (int i = 0; i < static_cast<int>(versions.size()); ++i) {
            if (versions[static_cast<size_t>(i)].favourite) {
                selectedVersionIdx = i;
                break;
            }
        }
    } else {
        selectedVersionIdx = -1;
    }
    updateVersionCombo();
    updateOffsetDisplay();
    loadComments();
    loadPeaks();
}

void ReaMarkEditor::onVersionSelected() {
    selectedVersionIdx = versionCombo.getSelectedId() - 1;
    loadComments();
    loadPeaks();
    updateOffsetDisplay();
}

// ---------------------------------------------------------------------------
// Comments
// ---------------------------------------------------------------------------

void ReaMarkEditor::loadComments() {
    auto* ver = getSelectedVersion();
    if (!ver || activeShareLink.isEmpty()) {
        comments.clear();
        commentList.setComments({}, loggedIn);
        waveform.setComments({});
        return;
    }

    api.loadComments(activeShareLink, ver->id, [this](bool ok, const std::vector<Comment>& coms, const juce::String& err) {
        if (ok) {
            comments = coms;
            commentList.setComments(comments, loggedIn);
            waveform.setComments(comments);
        } else {
            showError(err);
        }
    });
}

void ReaMarkEditor::loadPeaks() {
    auto* ver = getSelectedVersion();
    if (!ver) {
        waveform.setPeaks({}, 0.0);
        return;
    }

    api.loadPeaks(ver->id, [this](bool ok, const std::vector<float>& peaks, double duration, const juce::String& err) {
        if (ok) {
            waveform.setPeaks(peaks, duration);
            waveform.setOffset(getCurrentOffset());
        } else {
            waveform.setPeaks({}, 0.0);
        }
    });
}

void ReaMarkEditor::doCreateComment() {
    auto text = commentInput.getText().trim();
    auto author = authorInput.getText().trim();
    if (text.isEmpty() || author.isEmpty()) return;

    auto* ver = getSelectedVersion();
    if (!ver || activeShareLink.isEmpty()) return;

    double transportPos = processorRef.getTransportPositionSeconds();
    double offset = getCurrentOffset();
    double relativeTC = juce::jmax(0.0, transportPos - offset);

    processorRef.authorName = author;

    api.createComment(activeShareLink, ver->id, relativeTC, author, text,
        [this](bool ok, const juce::String& err) {
            if (ok) {
                commentInput.clear();
                loadComments();
            } else {
                showError(err);
            }
        });
}

// ---------------------------------------------------------------------------
// Calibration offset
// ---------------------------------------------------------------------------

void ReaMarkEditor::doSetOffset() {
    auto* song = getSelectedSong();
    if (!song) return;

    double transportPos = processorRef.getTransportPositionSeconds();
    processorRef.calibrationOffsets[song->id] = transportPos;
    updateOffsetDisplay();
    waveform.setOffset(transportPos);
}

double ReaMarkEditor::getCurrentOffset() const {
    auto* song = getSelectedSong();
    if (!song) return 0.0;
    auto it = processorRef.calibrationOffsets.find(song->id);
    return it != processorRef.calibrationOffsets.end() ? it->second : 0.0;
}

void ReaMarkEditor::doToggleFavourite() {
    auto* ver = getSelectedVersion();
    if (!ver || !loggedIn) return;

    api.toggleFavourite(ver->id, [this](bool ok, bool isFav, const juce::String& err) {
        juce::ignoreUnused(err);
        if (ok) {
            auto* song = getSelectedSong();
            if (song) {
                for (auto& version : const_cast<Song*>(song)->versions)
                    version.favourite = false;
                if (selectedVersionIdx >= 0 && selectedVersionIdx < static_cast<int>(song->versions.size()))
                    const_cast<Song*>(song)->versions[static_cast<size_t>(selectedVersionIdx)].favourite = isFav;
            }
            updateVersionCombo();
        }
    });
}

// ---------------------------------------------------------------------------
// UI update helpers
// ---------------------------------------------------------------------------

void ReaMarkEditor::updateSongCombo() {
    songCombo.clear();
    for (size_t i = 0; i < currentProject.songs.size(); ++i)
        songCombo.addItem(currentProject.songs[i].title, static_cast<int>(i + 1));

    if (selectedSongIdx >= 0 && selectedSongIdx < static_cast<int>(currentProject.songs.size()))
        songCombo.setSelectedId(selectedSongIdx + 1, juce::dontSendNotification);
}

void ReaMarkEditor::updateVersionCombo() {
    versionCombo.clear();
    auto* song = getSelectedSong();
    if (!song) return;

    for (size_t i = 0; i < song->versions.size(); ++i) {
        auto& v = song->versions[i];
        juce::String label = "v" + juce::String(v.versionNumber);
        if (v.label.isNotEmpty())
            label += " - " + v.label;
        if (v.favourite)
            label += " \xe2\x98\x85";
        versionCombo.addItem(label, static_cast<int>(i + 1));
    }

    if (selectedVersionIdx >= 0 && selectedVersionIdx < static_cast<int>(song->versions.size()))
        versionCombo.setSelectedId(selectedVersionIdx + 1, juce::dontSendNotification);

    // Update favourite button
    auto* ver = getSelectedVersion();
    if (ver) {
        favouriteBtn.setButtonText(ver->favourite ? "\xe2\x98\x85" : "\xe2\x98\x86");
        favouriteBtn.setColour(juce::TextButton::textColourOffId,
                               ver->favourite ? Theme::yellow() : Theme::textMuted());
    }
}

void ReaMarkEditor::updateOffsetDisplay() {
    double offset = getCurrentOffset();
    offsetLabel.setText("Offset: " + formatTimecode(offset) + (offset == 0.0 ? "  (!)" : ""),
                        juce::dontSendNotification);
    if (offset == 0.0)
        offsetLabel.setColour(juce::Label::textColourId, Theme::amber());
    else
        offsetLabel.setColour(juce::Label::textColourId, Theme::textMuted());
}

void ReaMarkEditor::updateTimecodeDisplay() {
    double transportPos = processorRef.getTransportPositionSeconds();
    double offset = getCurrentOffset();
    double relativeTC = juce::jmax(0.0, transportPos - offset);
    timecodeLabel.setText("@" + formatTimecode(relativeTC), juce::dontSendNotification);
}

void ReaMarkEditor::showError(const juce::String& msg) {
    errorMsg = msg;
    errorLabel.setText(msg, juce::dontSendNotification);
}

const Song* ReaMarkEditor::getSelectedSong() const {
    if (selectedSongIdx >= 0 && selectedSongIdx < static_cast<int>(currentProject.songs.size()))
        return &currentProject.songs[static_cast<size_t>(selectedSongIdx)];
    return nullptr;
}

const Version* ReaMarkEditor::getSelectedVersion() const {
    auto* song = getSelectedSong();
    if (!song) return nullptr;
    if (selectedVersionIdx >= 0 && selectedVersionIdx < static_cast<int>(song->versions.size()))
        return &song->versions[static_cast<size_t>(selectedVersionIdx)];
    return nullptr;
}

// ---------------------------------------------------------------------------
// Seek — move waveform cursor to a timecode position
// ---------------------------------------------------------------------------

void ReaMarkEditor::seekTo(double relativeTimecode) {
    double offset = getCurrentOffset();
    manualSeekPos = relativeTimecode + offset;

    // Update waveform playhead immediately
    waveform.setPlayheadPosition(manualSeekPos);

    // Update timecode display with the clicked position
    timecodeLabel.setText("@" + formatTimecode(relativeTimecode), juce::dontSendNotification);
}

// ---------------------------------------------------------------------------
// Timer — update playhead + timecode display
// ---------------------------------------------------------------------------

void ReaMarkEditor::timerCallback() {
    bool playing = processorRef.isTransportPlaying();
    double pos = processorRef.getTransportPositionSeconds();

    if (playing) {
        // Transport is running — follow DAW cursor, clear manual seek
        manualSeekPos = -1.0;
        waveform.setPlayheadPosition(pos);
        updateTimecodeDisplay();
    } else if (manualSeekPos < 0.0) {
        // Stopped, no manual seek — show transport position
        waveform.setPlayheadPosition(pos);
        updateTimecodeDisplay();
    }
    // else: stopped + manual seek active — keep showing the clicked position
}

// ---------------------------------------------------------------------------
// Separator
// ---------------------------------------------------------------------------

void ReaMarkEditor::drawSeparator(juce::Graphics& g, int y) {
    g.setColour(Theme::bgBorder());
    g.fillRect(12, y, getWidth() - 24, 1);
}
