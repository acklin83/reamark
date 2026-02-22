#include "CommentListComponent.h"
#include "MixnoteTheme.h"

namespace mixnote {

// ===========================================================================
// CommentCard
// ===========================================================================

CommentCard::CommentCard() {
    addAndMakeVisible(timecodeBtn);
    addAndMakeVisible(authorLabel);
    addAndMakeVisible(textLabel);
    addAndMakeVisible(replyBtn);

    // Hidden by default
    addChildComponent(resolveBtn);
    addChildComponent(editBtn);
    addChildComponent(deleteBtn);
    addChildComponent(replyInput);
    addChildComponent(replySendBtn);
    addChildComponent(editInput);
    addChildComponent(editSaveBtn);
    addChildComponent(editCancelBtn);

    // Styling
    timecodeBtn.setColour(juce::TextButton::buttonColourId, Theme::bgBorder());
    timecodeBtn.setColour(juce::TextButton::textColourOnId, Theme::accent());
    timecodeBtn.setColour(juce::TextButton::textColourOffId, Theme::accent());

    for (auto* btn : { &resolveBtn, &editBtn, &replyBtn }) {
        btn->setColour(juce::TextButton::buttonColourId, Theme::bgInput());
        btn->setColour(juce::TextButton::textColourOnId, Theme::textDim());
        btn->setColour(juce::TextButton::textColourOffId, Theme::textDim());
    }
    deleteBtn.setColour(juce::TextButton::buttonColourId, Theme::bgInput());
    deleteBtn.setColour(juce::TextButton::textColourOnId, Theme::red());
    deleteBtn.setColour(juce::TextButton::textColourOffId, Theme::red());

    replyBtn.setColour(juce::TextButton::textColourOnId, Theme::accent());
    replyBtn.setColour(juce::TextButton::textColourOffId, Theme::accent());

    replySendBtn.setColour(juce::TextButton::buttonColourId, Theme::bgInput());
    editSaveBtn.setColour(juce::TextButton::buttonColourId, Theme::bgInput());
    editCancelBtn.setColour(juce::TextButton::buttonColourId, Theme::bgInput());

    authorLabel.setColour(juce::Label::textColourId, Theme::text());
    textLabel.setColour(juce::Label::textColourId, Theme::text());
    textLabel.setMinimumHorizontalScale(1.0f);

    replyInput.setColour(juce::TextEditor::backgroundColourId, Theme::bgInput());
    replyInput.setColour(juce::TextEditor::textColourId, Theme::text());
    editInput.setColour(juce::TextEditor::backgroundColourId, Theme::bgInput());
    editInput.setColour(juce::TextEditor::textColourId, Theme::text());
    editInput.setMultiLine(true);

    // Actions
    timecodeBtn.onClick = [this]() {
        if (onTimecodeClick) onTimecodeClick(comment.timecode);
    };
    resolveBtn.onClick = [this]() {
        if (onResolve) onResolve(comment.id);
    };
    deleteBtn.onClick = [this]() {
        if (onDelete) onDelete(comment.id);
    };
    editBtn.onClick = [this]() {
        editOpen = !editOpen;
        if (editOpen) {
            editInput.setText(comment.text);
            replyOpen = false;
        }
        resized();
    };
    replyBtn.onClick = [this]() {
        replyOpen = !replyOpen;
        if (replyOpen) {
            replyInput.clear();
            editOpen = false;
        }
        resized();
    };
    replySendBtn.onClick = [this]() {
        auto text = replyInput.getText().trim();
        if (text.isNotEmpty() && onReply) {
            onReply(comment.id, text);
            replyOpen = false;
            resized();
        }
    };
    editSaveBtn.onClick = [this]() {
        auto text = editInput.getText().trim();
        if (text.isNotEmpty() && onEdit) {
            onEdit(comment.id, text);
            editOpen = false;
            resized();
        }
    };
    editCancelBtn.onClick = [this]() {
        editOpen = false;
        resized();
    };
}

void CommentCard::setComment(const Comment& c, bool isAdminUser) {
    comment = c;
    admin = isAdminUser;

    timecodeBtn.setButtonText("@" + formatTimecode(c.timecode));
    authorLabel.setText(c.authorName, juce::dontSendNotification);
    textLabel.setText(c.text, juce::dontSendNotification);

    if (c.solved) {
        timecodeBtn.setColour(juce::TextButton::textColourOnId, Theme::textMuted());
        timecodeBtn.setColour(juce::TextButton::textColourOffId, Theme::textMuted());
        textLabel.setColour(juce::Label::textColourId, Theme::textMuted());
        resolveBtn.setColour(juce::TextButton::textColourOnId, Theme::green());
        resolveBtn.setColour(juce::TextButton::textColourOffId, Theme::green());
    } else {
        timecodeBtn.setColour(juce::TextButton::textColourOnId, Theme::accent());
        timecodeBtn.setColour(juce::TextButton::textColourOffId, Theme::accent());
        textLabel.setColour(juce::Label::textColourId, Theme::text());
        resolveBtn.setColour(juce::TextButton::textColourOnId, Theme::textDim());
        resolveBtn.setColour(juce::TextButton::textColourOffId, Theme::textDim());
    }

    resolveBtn.setVisible(admin);
    editBtn.setVisible(admin);
    deleteBtn.setVisible(admin);

    rebuildReplyDisplays();
    editOpen = false;
    replyOpen = false;
    resized();
}

void CommentCard::rebuildReplyDisplays() {
    replyDisplays.clear();
    for (auto& r : comment.replies) {
        ReplyDisplay rd;
        rd.textLabel = std::make_unique<juce::Label>("", r.text);
        rd.textLabel->setColour(juce::Label::textColourId, Theme::text());
        rd.textLabel->setFont(juce::FontOptions(13.0f));
        addAndMakeVisible(*rd.textLabel);

        rd.authorLabel = std::make_unique<juce::Label>("", "  -- " + r.authorName);
        rd.authorLabel->setColour(juce::Label::textColourId, Theme::textMuted());
        rd.authorLabel->setFont(juce::FontOptions(12.0f));
        addAndMakeVisible(*rd.authorLabel);

        replyDisplays.push_back(std::move(rd));
    }
}

int CommentCard::getDesiredHeight() const {
    int h = 8;  // top padding
    h += 24;    // header row (timecode + author + buttons)
    h += 4;     // spacing
    h += 20;    // text

    // Replies
    for (size_t i = 0; i < replyDisplays.size(); ++i)
        h += 34;  // reply text + author

    h += 24;    // reply button row

    if (replyOpen)
        h += 30;  // reply input

    if (editOpen)
        h += 60;  // edit input + buttons

    h += 8;  // bottom padding
    return h;
}

void CommentCard::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat().reduced(1.0f);
    auto bgCol = comment.solved ? Theme::cardSolved() : Theme::cardOpen();
    g.setColour(bgCol);
    g.fillRoundedRectangle(bounds, 4.0f);
}

void CommentCard::resized() {
    auto area = getLocalBounds().reduced(8);
    int rowH = 24;

    // Header: [timecode] [author]                [Done] [Edit] [Delete]
    auto header = area.removeFromTop(rowH);
    timecodeBtn.setBounds(header.removeFromLeft(90));
    header.removeFromLeft(4);

    if (admin) {
        deleteBtn.setBounds(header.removeFromRight(55));
        header.removeFromRight(2);
        editBtn.setBounds(header.removeFromRight(45));
        header.removeFromRight(2);
        resolveBtn.setBounds(header.removeFromRight(50));
        header.removeFromRight(4);
    }
    authorLabel.setBounds(header);

    area.removeFromTop(4);

    // Text (or edit input)
    if (editOpen) {
        editInput.setVisible(true);
        editSaveBtn.setVisible(true);
        editCancelBtn.setVisible(true);
        textLabel.setVisible(false);

        editInput.setBounds(area.removeFromTop(30));
        area.removeFromTop(2);
        auto editBtnRow = area.removeFromTop(22);
        editSaveBtn.setBounds(editBtnRow.removeFromLeft(50));
        editBtnRow.removeFromLeft(4);
        editCancelBtn.setBounds(editBtnRow.removeFromLeft(60));
    } else {
        editInput.setVisible(false);
        editSaveBtn.setVisible(false);
        editCancelBtn.setVisible(false);
        textLabel.setVisible(true);

        textLabel.setBounds(area.removeFromTop(20));
    }

    // Replies
    for (auto& rd : replyDisplays) {
        area.removeFromTop(4);
        auto replyArea = area.removeFromTop(30).withTrimmedLeft(12);
        rd.textLabel->setBounds(replyArea.removeFromTop(16));
        rd.authorLabel->setBounds(replyArea);
    }

    area.removeFromTop(4);

    // Reply button (right-aligned)
    auto replyRow = area.removeFromTop(22);
    replyBtn.setBounds(replyRow.removeFromRight(55));

    if (!admin) {
        // Show status text for non-admin
        juce::ignoreUnused(replyRow);
        // Status rendered in paint() or could add a label
    }

    // Reply input
    if (replyOpen) {
        replyInput.setVisible(true);
        replySendBtn.setVisible(true);
        area.removeFromTop(4);
        auto inputRow = area.removeFromTop(24).withTrimmedLeft(12);
        replySendBtn.setBounds(inputRow.removeFromRight(50));
        inputRow.removeFromRight(4);
        replyInput.setBounds(inputRow);
    } else {
        replyInput.setVisible(false);
        replySendBtn.setVisible(false);
    }
}

void CommentCard::setExpanded(bool reply, bool edit) {
    replyOpen = reply;
    editOpen = edit;
    resized();
}

// ===========================================================================
// CommentListComponent
// ===========================================================================

CommentListComponent::CommentListComponent() {
    addAndMakeVisible(allBtn);
    addAndMakeVisible(openBtn);
    addAndMakeVisible(doneBtn);
    addAndMakeVisible(refreshBtn);
    addAndMakeVisible(viewport);

    contentComponent = std::make_unique<juce::Component>();
    viewport.setViewedComponent(contentComponent.get(), false);
    viewport.setScrollBarsShown(true, false);

    // Filter button styling
    for (auto* btn : { &allBtn, &openBtn, &doneBtn, &refreshBtn }) {
        btn->setColour(juce::TextButton::buttonColourId, Theme::bgInput());
        btn->setColour(juce::TextButton::textColourOnId, Theme::text());
        btn->setColour(juce::TextButton::textColourOffId, Theme::textDim());
    }

    allBtn.onClick = [this]() { setFilterMode(FilterMode::All); rebuild(); };
    openBtn.onClick = [this]() { setFilterMode(FilterMode::Open); rebuild(); };
    doneBtn.onClick = [this]() { setFilterMode(FilterMode::Done); rebuild(); };
    refreshBtn.onClick = [this]() { if (onRefresh) onRefresh(); };
}

int CommentListComponent::getOpenCount() const {
    int count = 0;
    for (auto& c : allComments)
        if (!c.solved) ++count;
    return count;
}

int CommentListComponent::getResolvedCount() const {
    int count = 0;
    for (auto& c : allComments)
        if (c.solved) ++count;
    return count;
}

void CommentListComponent::setComments(const std::vector<Comment>& comments, bool adminMode) {
    allComments = comments;
    isAdmin = adminMode;
    rebuild();
}

void CommentListComponent::setFilterMode(FilterMode mode) {
    filterMode = mode;

    // Highlight active filter
    auto activeCol = Theme::accent();
    auto inactiveCol = Theme::bgInput();

    allBtn.setColour(juce::TextButton::buttonColourId,  filterMode == All  ? activeCol : inactiveCol);
    openBtn.setColour(juce::TextButton::buttonColourId, filterMode == Open ? activeCol : inactiveCol);
    doneBtn.setColour(juce::TextButton::buttonColourId, filterMode == Done ? activeCol : inactiveCol);
}

void CommentListComponent::rebuild() {
    cards.clear();
    contentComponent->removeAllChildren();

    // Update button labels with counts
    allBtn.setButtonText("All (" + juce::String(getTotalCount()) + ")");
    openBtn.setButtonText("Open (" + juce::String(getOpenCount()) + ")");
    doneBtn.setButtonText("Done (" + juce::String(getResolvedCount()) + ")");

    for (auto& c : allComments) {
        bool show = (filterMode == All)
                 || (filterMode == Open && !c.solved)
                 || (filterMode == Done && c.solved);

        if (!show) continue;

        auto card = std::make_unique<CommentCard>();
        card->setComment(c, isAdmin);

        // Forward callbacks
        card->onTimecodeClick = onTimecodeClick;
        card->onResolve = onResolve;
        card->onDelete = onDelete;
        card->onEdit = onEdit;
        card->onReply = onReply;

        contentComponent->addAndMakeVisible(*card);
        cards.push_back(std::move(card));
    }

    layoutCards();
}

void CommentListComponent::layoutCards() {
    int w = viewport.getWidth() - viewport.getScrollBarThickness();
    if (w <= 0) w = getWidth() - 12;
    int y = 4;

    for (auto& card : cards) {
        int h = card->getDesiredHeight();
        card->setBounds(0, y, w, h);
        y += h + 4;
    }

    contentComponent->setSize(w, y);
}

void CommentListComponent::resized() {
    auto area = getLocalBounds();

    // Filter buttons row
    auto filterRow = area.removeFromTop(28);
    int btnW = 70;
    allBtn.setBounds(filterRow.removeFromLeft(btnW));
    filterRow.removeFromLeft(4);
    openBtn.setBounds(filterRow.removeFromLeft(btnW));
    filterRow.removeFromLeft(4);
    doneBtn.setBounds(filterRow.removeFromLeft(btnW));
    filterRow.removeFromLeft(4);
    refreshBtn.setBounds(filterRow.removeFromLeft(65));

    area.removeFromTop(4);
    viewport.setBounds(area);
    layoutCards();
}

} // namespace mixnote
