// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

#include "MicrophoneMute.h"

#include "MicrophoneStreams.h"

#include <QGuiApplication>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(lcMicrophone, "crossparty.microphone", QtInfoMsg)

// PipeWire speaks the PulseAudio protocol, and the app already has the
// PulseAudio socket for party chat (SEC-012), so this needs no new
// permission. The PulseAudio library runs on Qt's own event loop through its
// GLib adapter: no extra thread, and every callback below arrives on the
// main thread.

namespace
{
// Running inside Flatpak is the only way PipeWire can vouch for which stream
// is ours (MicrophoneStreams::appIdProperty).
QString flatpakAppId()
{
    return qEnvironmentVariable("FLATPAK_ID");
}

void finish(pa_operation *operation)
{
    if (operation) {
        pa_operation_unref(operation);
    }
}
}

MicrophoneMute::MicrophoneMute(QObject *parent)
    : QObject(parent)
    , m_mainloop(pa_glib_mainloop_new(nullptr))
    , m_context(pa_context_new(pa_glib_mainloop_get_api(m_mainloop), "Crossparty"))
    , m_ourAppId(flatpakAppId())
{
    if (m_ourAppId.isEmpty()) {
        qCWarning(lcMicrophone) << "Not running in Flatpak; the mute cannot tell which stream is ours, so it is off";
        return;
    }
    pa_context_set_state_callback(m_context, &MicrophoneMute::onContextState, this);
    if (pa_context_connect(m_context, nullptr, PA_CONTEXT_NOFAIL, nullptr) < 0) {
        qCWarning(lcMicrophone) << "Could not reach the sound server";
    }
}

MicrophoneMute::~MicrophoneMute()
{
    pa_context_set_state_callback(m_context, nullptr, nullptr);
    pa_context_set_subscribe_callback(m_context, nullptr, nullptr);
    pa_context_disconnect(m_context);
    pa_context_unref(m_context);
    pa_glib_mainloop_free(m_mainloop);
}

bool MicrophoneMute::isMuted() const
{
    return m_muted;
}

void MicrophoneMute::setMuted(bool muted)
{
    if (muted == m_muted) {
        return;
    }
    m_muted = muted;
    for (auto stream = m_streams.cbegin(); stream != m_streams.cend(); ++stream) {
        setStreamMuted(stream.key(), m_muted);
    }
    qCInfo(lcMicrophone) << (m_muted ? "Microphone muted" : "Microphone on");
    Q_EMIT mutedChanged();
}

void MicrophoneMute::toggle()
{
    setMuted(!m_muted);
}

bool MicrophoneMute::isAvailable() const
{
    return m_available;
}

void MicrophoneMute::setAvailable(bool available)
{
    if (available != m_available) {
        m_available = available;
        Q_EMIT availableChanged();
    }
}

void MicrophoneMute::onContextState(pa_context *context, void *self)
{
    auto *mute = static_cast<MicrophoneMute *>(self);
    switch (pa_context_get_state(context)) {
    case PA_CONTEXT_READY:
        // Watch recording streams come and go, then look at the ones that
        // already exist.
        pa_context_set_subscribe_callback(context, &MicrophoneMute::onStreamEvent, self);
        finish(pa_context_subscribe(context, PA_SUBSCRIPTION_MASK_SOURCE_OUTPUT, nullptr, nullptr));
        finish(pa_context_get_source_output_info_list(context, &MicrophoneMute::onStreamInfo, self));
        mute->setAvailable(true);
        return;
    case PA_CONTEXT_FAILED:
    case PA_CONTEXT_TERMINATED:
        mute->m_streams.clear();
        mute->setAvailable(false);
        return;
    default:
        return;
    }
}

void MicrophoneMute::onStreamEvent(pa_context *context, pa_subscription_event_type_t eventType, uint32_t index, void *self)
{
    auto *mute = static_cast<MicrophoneMute *>(self);
    const unsigned int kind = eventType & PA_SUBSCRIPTION_EVENT_TYPE_MASK;
    if (kind == PA_SUBSCRIPTION_EVENT_REMOVE) {
        mute->m_streams.remove(index);
        return;
    }
    finish(pa_context_get_source_output_info(context, index, &MicrophoneMute::onStreamInfo, self));
}

void MicrophoneMute::onStreamInfo(pa_context *context, const pa_source_output_info *info, int isLast, void *self)
{
    Q_UNUSED(context)
    if (isLast == 0 && info) {
        static_cast<MicrophoneMute *>(self)->handleStream(*info);
    }
}

void MicrophoneMute::handleStream(const pa_source_output_info &info)
{
    const QString streamAppId = QString::fromUtf8(pa_proplist_gets(info.proplist, MicrophoneStreams::appIdProperty));
    if (!MicrophoneStreams::isOurs(streamAppId, m_ourAppId)) {
        return;
    }

    const bool isNew = !m_streams.contains(info.index);
    const bool streamIsMuted = info.mute != 0;
    m_streams.insert(info.index, streamIsMuted);

    switch (MicrophoneStreams::decide(isNew, streamIsMuted, m_muted)) {
    case MicrophoneStreams::MuteAction::None:
        return;
    case MicrophoneStreams::MuteAction::ApplyChoice:
        setStreamMuted(info.index, m_muted);
        return;
    case MicrophoneStreams::MuteAction::AdoptStreamState:
        m_muted = streamIsMuted;
        qCInfo(lcMicrophone) << "Mute changed outside the app; following it";
        Q_EMIT mutedChanged();
        return;
    }
}

void MicrophoneMute::setStreamMuted(uint32_t index, bool muted)
{
    m_streams.insert(index, muted);
    finish(pa_context_set_source_output_mute(m_context, index, muted ? 1 : 0, nullptr, nullptr));
}
