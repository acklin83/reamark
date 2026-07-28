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
    // Every endpoint lives under the Studio OS /rmc compat prefix on the studio host
    // (e.g. studio.example.com/rmc/admin/projects) — one place, so callers keep the
    // bare /admin and /api paths.
    auto url = juce::URL(serverUrl + "/rmc" + endpoint);

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

void ReaMarkApi::login(const juce::String& connectToken, LoginCallback callback) {
    threadPool.addJob([this, connectToken, cb = std::move(callback)]() {
        // The connect token IS the bearer — no login round-trip, no 24h expiry. Verify it
        // by listing projects (the first authenticated admin call); a 200 means this
        // instance accepts the token.
        setJwtToken(connectToken);
        auto resp = httpGet("/admin/projects");

        juce::MessageManager::callAsync([this, resp, connectToken, cb]() {
            if (resp.statusCode == 200) {
                cb(true, connectToken, {});
            } else {
                setJwtToken({});
                cb(false, {}, (resp.statusCode == 401 || resp.statusCode == 403)
                       ? "Connect token rejected"
                       : "Connection failed (HTTP " + juce::String(resp.statusCode) + ")");
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

void ReaMarkApi::loadComments(const juce::String& shareLink, const juce::String& versionId, CommentsCallback callback) {
    threadPool.addJob([this, shareLink, versionId, cb = std::move(callback)]() {
        auto resp = httpGet("/api/projects/" + shareLink + "/comments?version_id=" + versionId);

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

void ReaMarkApi::createComment(const juce::String& shareLink, const juce::String& versionId, double timecode,
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

void ReaMarkApi::replyToComment(const juce::String& shareLink, const juce::String& commentId,
                                 const juce::String& authorName, const juce::String& text, SimpleCallback callback) {
    threadPool.addJob([this, shareLink, commentId, authorName, text, cb = std::move(callback)]() {
        auto body = makeJsonObject({
            { "author_name", authorName },
            { "text", text }
        });

        auto resp = httpPost("/api/projects/" + shareLink + "/comments/" + commentId + "/reply", body);

        juce::MessageManager::callAsync([resp, cb]() {
            cb(resp.statusCode == 201, resp.statusCode != 201
                ? "Failed to reply (HTTP " + juce::String(resp.statusCode) + ")" : juce::String());
        });
    });
}

void ReaMarkApi::resolveComment(const juce::String& shareLink, const juce::String& commentId, SimpleCallback callback) {
    threadPool.addJob([this, shareLink, commentId, cb = std::move(callback)]() {
        auto resp = httpPatch("/api/projects/" + shareLink + "/comments/" + commentId + "/resolve");

        juce::MessageManager::callAsync([resp, cb]() {
            cb(resp.statusCode == 200, resp.statusCode != 200
                ? "Failed to resolve (HTTP " + juce::String(resp.statusCode) + ")" : juce::String());
        });
    });
}

void ReaMarkApi::updateComment(const juce::String& commentId, const juce::String& text, SimpleCallback callback) {
    threadPool.addJob([this, commentId, text, cb = std::move(callback)]() {
        auto body = makeJsonObject({
            { "text", text }
        });

        auto resp = httpPut("/admin/comments/" + commentId, body);

        juce::MessageManager::callAsync([resp, cb]() {
            cb(resp.statusCode == 200, resp.statusCode != 200
                ? "Failed to update comment (HTTP " + juce::String(resp.statusCode) + ")" : juce::String());
        });
    });
}

void ReaMarkApi::deleteComment(const juce::String& commentId, SimpleCallback callback) {
    threadPool.addJob([this, commentId, cb = std::move(callback)]() {
        auto resp = httpDelete("/admin/comments/" + commentId);

        juce::MessageManager::callAsync([resp, cb]() {
            bool ok = resp.statusCode == 200 || resp.statusCode == 204;
            cb(ok, !ok ? "Failed to delete comment (HTTP " + juce::String(resp.statusCode) + ")" : juce::String());
        });
    });
}

// ---------------------------------------------------------------------------
// Versions
// ---------------------------------------------------------------------------

void ReaMarkApi::toggleFavourite(const juce::String& versionId, FavouriteCallback callback) {
    threadPool.addJob([this, versionId, cb = std::move(callback)]() {
        auto resp = httpPatch("/admin/versions/" + versionId + "/favourite");

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

void ReaMarkApi::loadPeaks(const juce::String& versionId, PeaksCallback callback) {
    threadPool.addJob([this, versionId, cb = std::move(callback)]() {
        auto resp = httpGet("/api/versions/" + versionId + "/peaks");

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

// ---------------------------------------------------------------------------
// Branding (per-tenant accent)
// ---------------------------------------------------------------------------

void ReaMarkApi::fetchBranding(BrandingCallback callback) {
    threadPool.addJob([this, cb = std::move(callback)]() {
        // /api/studio is a native Studio OS public endpoint — no /rmc prefix, no auth. Build
        // the URL directly rather than via httpRequest (which prepends /rmc).
        auto url = juce::URL(serverUrl + "/api/studio");
        auto options = juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)
                           .withConnectionTimeoutMs(10000);

        int status = 0;
        juce::String body;
        if (auto stream = url.createInputStream(options)) {
            if (auto* ws = dynamic_cast<juce::WebInputStream*>(stream.get()))
                status = ws->getStatusCode();
            body = stream->readEntireStreamAsString();
        }

        bool ok = false;
        juce::Colour accent;
        if (status == 200) {
            auto hex = juce::JSON::parse(body).getProperty("accent", "").toString().trim();
            if (hex.startsWithChar('#')) hex = hex.substring(1);
            if (hex.length() == 6) {
                accent = juce::Colour((juce::uint32) 0xFF000000u | (juce::uint32) hex.getHexValue32());
                ok = true;
            }
        }

        juce::MessageManager::callAsync([ok, accent, cb]() { cb(ok, accent); });
    });
}

} // namespace reamark
