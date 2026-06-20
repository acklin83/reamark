#include "ReaMarkApi.h"

namespace reamark {

// Helper to build a JSON object from key-value pairs
static juce::String makeJsonObject(std::initializer_list<std::pair<juce::String, juce::var>> props) {
    auto* obj = new juce::DynamicObject();
    for (auto& p : props)
        obj->setProperty(p.first, p.second);
    return juce::JSON::toString(juce::var(obj));
}

ReaMarkApi::ReaMarkApi() {}

ReaMarkApi::~ReaMarkApi() {
    threadPool.removeAllJobs(true, 5000);
}

void ReaMarkApi::setServerUrl(const juce::String& url) {
    auto trimmed = url.trim().trimCharactersAtEnd("/");

    // Ensure the URL has a protocol prefix
    if (trimmed.isNotEmpty()
        && !trimmed.startsWithIgnoreCase("http://")
        && !trimmed.startsWithIgnoreCase("https://"))
    {
        trimmed = "https://" + trimmed;
    }

    serverUrl = trimmed;
}

juce::String ReaMarkApi::getServerUrl() const { return serverUrl; }

void ReaMarkApi::setJwtToken(const juce::String& token) { jwtToken = token; }
juce::String ReaMarkApi::getJwtToken() const { return jwtToken; }
bool ReaMarkApi::isLoggedIn() const { return jwtToken.isNotEmpty(); }

// ---------------------------------------------------------------------------
// HTTP helpers
// ---------------------------------------------------------------------------

ReaMarkApi::HttpResponse ReaMarkApi::httpRequest(const juce::String& method,
                                                   const juce::String& endpoint,
                                                   const juce::String& body) {
    HttpResponse result;
    auto url = juce::URL(serverUrl + endpoint);

    if (body.isNotEmpty() && (method == "POST" || method == "PUT" || method == "PATCH"))
        url = url.withPOSTData(body);

    juce::String headers = "Content-Type: application/json\r\n";
    if (jwtToken.isNotEmpty())
        headers += "Authorization: Bearer " + jwtToken + "\r\n";

    auto options = juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)
                       .withHttpRequestCmd(method)
                       .withConnectionTimeoutMs(10000)
                       .withExtraHeaders(headers);

    if (auto stream = url.createInputStream(options)) {
        if (auto* webStream = dynamic_cast<juce::WebInputStream*>(stream.get()))
            result.statusCode = webStream->getStatusCode();
        result.body = stream->readEntireStreamAsString();
    }

    return result;
}

ReaMarkApi::HttpResponse ReaMarkApi::httpGet(const juce::String& endpoint) {
    return httpRequest("GET", endpoint);
}

ReaMarkApi::HttpResponse ReaMarkApi::httpPost(const juce::String& endpoint, const juce::String& jsonBody) {
    return httpRequest("POST", endpoint, jsonBody);
}

ReaMarkApi::HttpResponse ReaMarkApi::httpPut(const juce::String& endpoint, const juce::String& jsonBody) {
    return httpRequest("PUT", endpoint, jsonBody);
}

ReaMarkApi::HttpResponse ReaMarkApi::httpPatch(const juce::String& endpoint, const juce::String& jsonBody) {
    return httpRequest("PATCH", endpoint, jsonBody);
}

ReaMarkApi::HttpResponse ReaMarkApi::httpDelete(const juce::String& endpoint) {
    return httpRequest("DELETE", endpoint);
}

// ---------------------------------------------------------------------------
// Auth
// ---------------------------------------------------------------------------

void ReaMarkApi::login(const juce::String& username, const juce::String& password, LoginCallback callback) {
    threadPool.addJob([this, username, password, cb = std::move(callback)]() {
        auto body = makeJsonObject({
            { "username", username },
            { "password", password }
        });

        auto resp = httpPost("/admin/auth/login", body);

        juce::MessageManager::callAsync([resp, cb]() {
            if (resp.statusCode == 200) {
                auto data = juce::JSON::parse(resp.body);
                auto token = data.getProperty("access_token", "").toString();
                if (token.isNotEmpty())
                    cb(true, token, {});
                else
                    cb(false, {}, "Invalid response");
            } else {
                cb(false, {}, "Login failed (HTTP " + juce::String(resp.statusCode) + ")");
            }
        });
    });
}

// ---------------------------------------------------------------------------
// Projects
// ---------------------------------------------------------------------------

void ReaMarkApi::loadProject(const juce::String& shareLink, ProjectCallback callback) {
    threadPool.addJob([this, shareLink, cb = std::move(callback)]() {
        auto resp = httpGet("/api/projects/" + shareLink);

        juce::MessageManager::callAsync([resp, cb]() {
            if (resp.statusCode == 200) {
                auto data = juce::JSON::parse(resp.body);
                cb(true, parseProject(data), {});
            } else {
                cb(false, {}, "Failed to load project (HTTP " + juce::String(resp.statusCode) + ")");
            }
        });
    });
}

void ReaMarkApi::loadAdminProjects(AdminProjectsCallback callback) {
    threadPool.addJob([this, cb = std::move(callback)]() {
        auto resp = httpGet("/admin/projects");

        juce::MessageManager::callAsync([resp, cb]() {
            if (resp.statusCode == 200) {
                auto data = juce::JSON::parse(resp.body);
                std::vector<AdminProject> projects;
                if (auto* arr = data.getArray())
                    for (auto& v : *arr)
                        projects.push_back(parseAdminProject(v));
                cb(true, projects, {});
            } else {
                cb(false, {}, "Failed to load projects (HTTP " + juce::String(resp.statusCode) + ")");
            }
        });
    });
}

// ---------------------------------------------------------------------------
// Comments
// ---------------------------------------------------------------------------

void ReaMarkApi::loadComments(const juce::String& shareLink, int versionId, CommentsCallback callback) {
    threadPool.addJob([this, shareLink, versionId, cb = std::move(callback)]() {
        auto resp = httpGet("/api/projects/" + shareLink + "/comments?version_id=" + juce::String(versionId));

        juce::MessageManager::callAsync([resp, cb]() {
            if (resp.statusCode == 200) {
                auto data = juce::JSON::parse(resp.body);
                std::vector<Comment> comments;
                if (auto* arr = data.getArray())
                    for (auto& v : *arr)
                        comments.push_back(parseComment(v));
                cb(true, comments, {});
            } else {
                cb(false, {}, "Failed to load comments (HTTP " + juce::String(resp.statusCode) + ")");
            }
        });
    });
}

void ReaMarkApi::createComment(const juce::String& shareLink, int versionId, double timecode,
                                const juce::String& authorName, const juce::String& text, SimpleCallback callback) {
    threadPool.addJob([this, shareLink, versionId, timecode, authorName, text, cb = std::move(callback)]() {
        auto body = makeJsonObject({
            { "version_id", versionId },
            { "timecode", timecode },
            { "author_name", authorName },
            { "text", text }
        });

        auto resp = httpPost("/api/projects/" + shareLink + "/comments", body);

        juce::MessageManager::callAsync([resp, cb]() {
            cb(resp.statusCode == 201, resp.statusCode != 201
                ? "Failed to create comment (HTTP " + juce::String(resp.statusCode) + ")" : juce::String());
        });
    });
}

void ReaMarkApi::replyToComment(const juce::String& shareLink, int commentId,
                                 const juce::String& authorName, const juce::String& text, SimpleCallback callback) {
    threadPool.addJob([this, shareLink, commentId, authorName, text, cb = std::move(callback)]() {
        auto body = makeJsonObject({
            { "author_name", authorName },
            { "text", text }
        });

        auto resp = httpPost("/api/projects/" + shareLink + "/comments/" + juce::String(commentId) + "/reply", body);

        juce::MessageManager::callAsync([resp, cb]() {
            cb(resp.statusCode == 201, resp.statusCode != 201
                ? "Failed to reply (HTTP " + juce::String(resp.statusCode) + ")" : juce::String());
        });
    });
}

void ReaMarkApi::resolveComment(const juce::String& shareLink, int commentId, SimpleCallback callback) {
    threadPool.addJob([this, shareLink, commentId, cb = std::move(callback)]() {
        auto resp = httpPatch("/api/projects/" + shareLink + "/comments/" + juce::String(commentId) + "/resolve");

        juce::MessageManager::callAsync([resp, cb]() {
            cb(resp.statusCode == 200, resp.statusCode != 200
                ? "Failed to resolve (HTTP " + juce::String(resp.statusCode) + ")" : juce::String());
        });
    });
}

void ReaMarkApi::updateComment(int commentId, const juce::String& text, SimpleCallback callback) {
    threadPool.addJob([this, commentId, text, cb = std::move(callback)]() {
        auto body = makeJsonObject({
            { "text", text }
        });

        auto resp = httpPut("/admin/comments/" + juce::String(commentId), body);

        juce::MessageManager::callAsync([resp, cb]() {
            cb(resp.statusCode == 200, resp.statusCode != 200
                ? "Failed to update comment (HTTP " + juce::String(resp.statusCode) + ")" : juce::String());
        });
    });
}

void ReaMarkApi::deleteComment(int commentId, SimpleCallback callback) {
    threadPool.addJob([this, commentId, cb = std::move(callback)]() {
        auto resp = httpDelete("/admin/comments/" + juce::String(commentId));

        juce::MessageManager::callAsync([resp, cb]() {
            bool ok = resp.statusCode == 200 || resp.statusCode == 204;
            cb(ok, !ok ? "Failed to delete comment (HTTP " + juce::String(resp.statusCode) + ")" : juce::String());
        });
    });
}

// ---------------------------------------------------------------------------
// Versions
// ---------------------------------------------------------------------------

void ReaMarkApi::toggleFavourite(int versionId, FavouriteCallback callback) {
    threadPool.addJob([this, versionId, cb = std::move(callback)]() {
        auto resp = httpPatch("/admin/versions/" + juce::String(versionId) + "/favourite");

        juce::MessageManager::callAsync([resp, cb]() {
            if (resp.statusCode == 200) {
                auto data = juce::JSON::parse(resp.body);
                cb(true, static_cast<bool>(data.getProperty("favourite", false)), {});
            } else {
                cb(false, false, "Failed to toggle favourite (HTTP " + juce::String(resp.statusCode) + ")");
            }
        });
    });
}

// ---------------------------------------------------------------------------
// Peaks
// ---------------------------------------------------------------------------

void ReaMarkApi::loadPeaks(int versionId, PeaksCallback callback) {
    threadPool.addJob([this, versionId, cb = std::move(callback)]() {
        auto resp = httpGet("/api/versions/" + juce::String(versionId) + "/peaks");

        juce::MessageManager::callAsync([resp, cb]() {
            if (resp.statusCode == 200) {
                auto data = juce::JSON::parse(resp.body);
                double duration = static_cast<double>(data.getProperty("duration", 0.0));
                std::vector<float> peaks;
                if (auto* arr = data.getProperty("peaks", juce::var()).getArray())
                    for (auto& v : *arr)
                        peaks.push_back(static_cast<float>(v));
                cb(true, peaks, duration, {});
            } else {
                cb(false, {}, 0.0, "Failed to load peaks (HTTP " + juce::String(resp.statusCode) + ")");
            }
        });
    });
}

} // namespace reamark
