#pragma once
#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>
#include <juce_graphics/juce_graphics.h>   // juce::Colour (per-tenant accent)
#include "ReaMarkModels.h"
#include <functional>

namespace reamark {

// Async API client for the ReaMark backend.
// All requests run on a background thread; callbacks are invoked on the message thread.
class ReaMarkApi {
public:
    ReaMarkApi();
    ~ReaMarkApi();

    void setServerUrl(const juce::String& url);
    juce::String getServerUrl() const;

    void setJwtToken(const juce::String& token);
    juce::String getJwtToken() const;
    bool isLoggedIn() const;

    // --- Auth ---
    using LoginCallback = std::function<void(bool success, const juce::String& token, const juce::String& error)>;
    // Connect with a per-instance connect token (static bearer). Verifies it against the server.
    void login(const juce::String& connectToken, LoginCallback callback);

    // --- Projects ---
    using ProjectCallback = std::function<void(bool success, const Project& project, const juce::String& error)>;
    void loadProject(const juce::String& shareLink, ProjectCallback callback);

    using AdminProjectsCallback = std::function<void(bool success, const std::vector<AdminProject>& projects, const juce::String& error)>;
    void loadAdminProjects(AdminProjectsCallback callback);

    // --- Comments ---
    using CommentsCallback = std::function<void(bool success, const std::vector<Comment>& comments, const juce::String& error)>;
    void loadComments(const juce::String& shareLink, const juce::String& versionId, CommentsCallback callback);

    using SimpleCallback = std::function<void(bool success, const juce::String& error)>;
    void createComment(const juce::String& shareLink, const juce::String& versionId, double timecode,
                       const juce::String& authorName, const juce::String& text, SimpleCallback callback);

    void replyToComment(const juce::String& shareLink, const juce::String& commentId,
                        const juce::String& authorName, const juce::String& text, SimpleCallback callback);

    void resolveComment(const juce::String& shareLink, const juce::String& commentId, SimpleCallback callback);

    void updateComment(const juce::String& commentId, const juce::String& text, SimpleCallback callback);

    void deleteComment(const juce::String& commentId, SimpleCallback callback);

    // --- Versions ---
    using FavouriteCallback = std::function<void(bool success, bool isFavourite, const juce::String& error)>;
    void toggleFavourite(const juce::String& versionId, FavouriteCallback callback);

    // --- Peaks ---
    using PeaksCallback = std::function<void(bool success, const std::vector<float>& peaks, double duration, const juce::String& error)>;
    void loadPeaks(const juce::String& versionId, PeaksCallback callback);

    // --- Branding (per-tenant accent from GET {server}/api/studio; native, no /rmc, no auth) ---
    using BrandingCallback = std::function<void(bool success, juce::Colour accent)>;
    void fetchBranding(BrandingCallback callback);

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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ReaMarkApi)
};

} // namespace reamark
