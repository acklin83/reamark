#pragma once
#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>
#include "MixnoteModels.h"
#include <functional>

namespace mixnote {

// Async API client for the Mixnote backend.
// All requests run on a background thread; callbacks are invoked on the message thread.
class MixnoteApi {
public:
    MixnoteApi();
    ~MixnoteApi();

    void setServerUrl(const juce::String& url);
    juce::String getServerUrl() const;

    void setJwtToken(const juce::String& token);
    juce::String getJwtToken() const;
    bool isLoggedIn() const;

    // --- Auth ---
    using LoginCallback = std::function<void(bool success, const juce::String& token, const juce::String& error)>;
    void login(const juce::String& username, const juce::String& password, LoginCallback callback);

    // --- Projects ---
    using ProjectCallback = std::function<void(bool success, const Project& project, const juce::String& error)>;
    void loadProject(const juce::String& shareLink, ProjectCallback callback);

    using AdminProjectsCallback = std::function<void(bool success, const std::vector<AdminProject>& projects, const juce::String& error)>;
    void loadAdminProjects(AdminProjectsCallback callback);

    // --- Comments ---
    using CommentsCallback = std::function<void(bool success, const std::vector<Comment>& comments, const juce::String& error)>;
    void loadComments(const juce::String& shareLink, int versionId, CommentsCallback callback);

    using SimpleCallback = std::function<void(bool success, const juce::String& error)>;
    void createComment(const juce::String& shareLink, int versionId, double timecode,
                       const juce::String& authorName, const juce::String& text, SimpleCallback callback);

    void replyToComment(const juce::String& shareLink, int commentId,
                        const juce::String& authorName, const juce::String& text, SimpleCallback callback);

    void resolveComment(const juce::String& shareLink, int commentId, SimpleCallback callback);

    void updateComment(int commentId, const juce::String& text, SimpleCallback callback);

    void deleteComment(int commentId, SimpleCallback callback);

    // --- Versions ---
    using FavouriteCallback = std::function<void(bool success, bool isFavourite, const juce::String& error)>;
    void toggleFavourite(int versionId, FavouriteCallback callback);

    // --- Peaks ---
    using PeaksCallback = std::function<void(bool success, const std::vector<float>& peaks, double duration, const juce::String& error)>;
    void loadPeaks(int versionId, PeaksCallback callback);

private:
    juce::String serverUrl;
    juce::String jwtToken;
    juce::ThreadPool threadPool { 2 };

    // Internal HTTP helpers
    struct HttpResponse {
        int statusCode = 0;
        juce::String body;
    };

    HttpResponse httpGet(const juce::String& endpoint);
    HttpResponse httpPost(const juce::String& endpoint, const juce::String& jsonBody);
    HttpResponse httpPut(const juce::String& endpoint, const juce::String& jsonBody);
    HttpResponse httpPatch(const juce::String& endpoint, const juce::String& jsonBody = {});
    HttpResponse httpDelete(const juce::String& endpoint);
    HttpResponse httpRequest(const juce::String& method, const juce::String& endpoint, const juce::String& body = {});

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MixnoteApi)
};

} // namespace mixnote
