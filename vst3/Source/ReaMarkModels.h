#pragma once
#include <juce_core/juce_core.h>
#include <vector>

namespace reamark {

struct Reply {
    int id = 0;
    juce::String authorName;
    juce::String text;
    juce::String createdAt;
};

struct Comment {
    int id = 0;
    int versionId = 0;
    double timecode = 0.0;
    juce::String authorName;
    juce::String text;
    bool solved = false;
    juce::String createdAt;
    std::vector<Reply> replies;
};

struct Version {
    int id = 0;
    int versionNumber = 0;
    juce::String label;
    bool favourite = false;
};

struct Song {
    int id = 0;
    juce::String title;
    int position = 0;
    std::vector<Version> versions;
};

struct Project {
    int id = 0;
    juce::String title;
    juce::String shareLink;
    std::vector<Song> songs;
};

struct AdminProject {
    int id = 0;
    juce::String title;
    juce::String shareLink;
};

// --- JSON parsing helpers ---

inline Reply parseReply(const juce::var& v) {
    Reply r;
    r.id = static_cast<int>(v.getProperty("id", 0));
    r.authorName = v.getProperty("author_name", "").toString();
    r.text = v.getProperty("text", "").toString();
    r.createdAt = v.getProperty("created_at", "").toString();
    return r;
}

inline Comment parseComment(const juce::var& v) {
    Comment c;
    c.id = static_cast<int>(v.getProperty("id", 0));
    c.versionId = static_cast<int>(v.getProperty("version_id", 0));
    c.timecode = static_cast<double>(v.getProperty("timecode", 0.0));
    c.authorName = v.getProperty("author_name", "").toString();
    c.text = v.getProperty("text", "").toString();
    c.solved = static_cast<bool>(v.getProperty("solved", false));
    c.createdAt = v.getProperty("created_at", "").toString();

    if (auto* arr = v.getProperty("replies", juce::var()).getArray())
        for (auto& rv : *arr)
            c.replies.push_back(parseReply(rv));

    return c;
}

inline Version parseVersion(const juce::var& v) {
    Version ver;
    ver.id = static_cast<int>(v.getProperty("id", 0));
    ver.versionNumber = static_cast<int>(v.getProperty("version_number", 0));
    ver.label = v.getProperty("label", "").toString();
    ver.favourite = static_cast<bool>(v.getProperty("favourite", false));
    return ver;
}

inline Song parseSong(const juce::var& v) {
    Song s;
    s.id = static_cast<int>(v.getProperty("id", 0));
    s.title = v.getProperty("title", "").toString();
    s.position = static_cast<int>(v.getProperty("position", 0));

    if (auto* arr = v.getProperty("versions", juce::var()).getArray())
        for (auto& vv : *arr)
            s.versions.push_back(parseVersion(vv));

    return s;
}

inline Project parseProject(const juce::var& v) {
    Project p;
    p.id = static_cast<int>(v.getProperty("id", 0));
    p.title = v.getProperty("title", "").toString();
    p.shareLink = v.getProperty("share_link", "").toString();

    if (auto* arr = v.getProperty("songs", juce::var()).getArray())
        for (auto& sv : *arr)
            p.songs.push_back(parseSong(sv));

    return p;
}

inline AdminProject parseAdminProject(const juce::var& v) {
    AdminProject p;
    p.id = static_cast<int>(v.getProperty("id", 0));
    p.title = v.getProperty("title", "").toString();
    p.shareLink = v.getProperty("share_link", "").toString();
    return p;
}

// --- Formatting ---

inline juce::String formatTimecode(double seconds) {
    if (seconds < 0.0) seconds = 0.0;
    int mins = static_cast<int>(seconds / 60.0);
    double secs = seconds - mins * 60.0;
    return juce::String::formatted("%02d:%05.2f", mins, secs);
}

inline juce::String extractShareCode(const juce::String& input) {
    // Extract UUID from full URL or return as-is
    int lastSlash = input.lastIndexOfChar('/');
    if (lastSlash >= 0)
        return input.substring(lastSlash + 1);
    return input;
}

} // namespace reamark
