#pragma once
#include <JuceHeader.h>
#include "MixnoteModels.h"
#include <vector>
#include <functional>

namespace mixnote {

// A single comment card (header, text, replies, action buttons)
class CommentCard : public juce::Component {
public:
    CommentCard();

    void setComment(const Comment& comment, bool isAdmin);
    void setExpanded(bool replyOpen, bool editOpen);
    int getDesiredHeight() const;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Callbacks
    std::function<void(double timecode)> onTimecodeClick;
    std::function<void(int commentId)> onResolve;
    std::function<void(int commentId)> onDelete;
    std::function<void(int commentId, const juce::String& text)> onEdit;
    std::function<void(int commentId, const juce::String& text)> onReply;

private:
    Comment comment;
    bool admin = false;
    bool replyOpen = false;
    bool editOpen = false;

    juce::TextButton timecodeBtn;
    juce::Label authorLabel;
    juce::Label textLabel;
    juce::TextButton resolveBtn { "Done" };
    juce::TextButton editBtn    { "Edit" };
    juce::TextButton deleteBtn  { "Delete" };
    juce::TextButton replyBtn   { "Reply" };

    // Reply input
    juce::TextEditor replyInput;
    juce::TextButton replySendBtn { "Send" };

    // Edit input
    juce::TextEditor editInput;
    juce::TextButton editSaveBtn   { "Save" };
    juce::TextButton editCancelBtn { "Cancel" };

    // Reply labels (rendered inline)
    struct ReplyDisplay {
        std::unique_ptr<juce::Label> textLabel;
        std::unique_ptr<juce::Label> authorLabel;
    };
    std::vector<ReplyDisplay> replyDisplays;

    void rebuildReplyDisplays();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CommentCard)
};

// ---------------------------------------------------------------------------

enum FilterMode { All = 0, Open = 1, Done = 2 };

class CommentListComponent : public juce::Component {
public:
    CommentListComponent();

    void setComments(const std::vector<Comment>& comments, bool isAdmin);
    void setFilterMode(FilterMode mode);
    FilterMode getFilterMode() const { return filterMode; }

    // Counts
    int getTotalCount() const { return static_cast<int>(allComments.size()); }
    int getOpenCount() const;
    int getResolvedCount() const;

    // Callbacks (forwarded from cards)
    std::function<void(double timecode)> onTimecodeClick;
    std::function<void(int commentId)> onResolve;
    std::function<void(int commentId)> onDelete;
    std::function<void(int commentId, const juce::String& text)> onEdit;
    std::function<void(int commentId, const juce::String& text)> onReply;
    std::function<void()> onRefresh;

    void resized() override;

private:
    FilterMode filterMode = FilterMode::All;
    std::vector<Comment> allComments;
    bool isAdmin = false;

    // Filter buttons
    juce::TextButton allBtn      { "All" };
    juce::TextButton openBtn     { "Open" };
    juce::TextButton doneBtn     { "Done" };
    juce::TextButton refreshBtn  { "Refresh" };

    // Scrollable area
    juce::Viewport viewport;
    std::unique_ptr<juce::Component> contentComponent;
    std::vector<std::unique_ptr<CommentCard>> cards;

    void rebuild();
    void layoutCards();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CommentListComponent)
};

} // namespace mixnote
