# Licensing

ReaMark is licensed under the **GNU Affero General Public License v3.0**
(AGPLv3). The full text is in [`LICENSE`](LICENSE).

## Why AGPLv3

The VST3 plugin (`vst3/`) is built with [JUCE](https://juce.com). JUCE's free
licensing option is **AGPLv3** — the only no-cost path that allows
distributing the plugin as open source. Licensing the whole project under
AGPLv3 keeps everything consistent and compliant.

AGPLv3 also fits a self-hosted, network-facing tool: anyone who runs a modified
ReaMark as a network service must make their modified source available to its
users (AGPLv3 §13). If you self-host the unmodified release, there is nothing
extra to do.

## Bundled / fetched components

| Component | License | Notes |
|-----------|---------|-------|
| JUCE 8 | AGPLv3 (free tier) | Fetched by CMake when building the VST3 plugin. Paid JUCE licenses are available if you need to ship a closed-source build. |
| Steinberg VST3 SDK | MIT | Provided via JUCE; permissive. |
| FastAPI, SQLAlchemy, Wavesurfer.js, TailwindCSS, etc. | MIT / BSD-style | Permissive, AGPLv3-compatible. |

The JUCE splash screen is enabled (`JUCE_DISPLAY_SPLASH_SCREEN=1`) as required
by the free JUCE license.

## Commercial / closed-source use

If you want to ship a closed-source product based on ReaMark's plugin, you need
a paid JUCE license (see <https://juce.com/get-juce/>). The web application
itself (backend + frontend) contains no JUCE code.
