// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

#pragma once

#include <QHash>
#include <QObject>
#include <QQmlEngine>

#include <pulse/context.h>
#include <pulse/glib-mainloop.h>
#include <pulse/introspect.h>
#include <pulse/subscribe.h>

#include <cstdint>

// Mutes the microphone itself, not the page's sound. QtWebEngine can only
// silence what a page plays, and the app never scripts Microsoft's page, so
// it mutes its own recording stream in PipeWire instead. Microsoft's page
// then simply hears silence. See docs/security.md#sec-014.
class MicrophoneMute : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(bool muted READ isMuted WRITE setMuted NOTIFY mutedChanged)
    // False when the sound server cannot be reached, or the app is not
    // running in Flatpak and so cannot tell which stream is its own.
    Q_PROPERTY(bool available READ isAvailable NOTIFY availableChanged)

public:
    explicit MicrophoneMute(QObject *parent = nullptr);
    ~MicrophoneMute() override;

    bool isMuted() const;
    void setMuted(bool muted);
    Q_INVOKABLE void toggle();

    bool isAvailable() const;

Q_SIGNALS:
    void mutedChanged();
    void availableChanged();

private:
    static void onContextState(pa_context *context, void *self);
    static void onStreamEvent(pa_context *context, pa_subscription_event_type_t eventType, uint32_t index, void *self);
    static void onStreamInfo(pa_context *context, const pa_source_output_info *info, int isLast, void *self);
    static void onMuteApplied(pa_context *context, int success, void *self);

    void handleStream(const pa_source_output_info &info);
    void setStreamMuted(uint32_t index, bool muted);
    void setAvailable(bool available);

    pa_glib_mainloop *m_mainloop;
    pa_context *m_context;
    QString m_ourAppId;
    // Our streams, by PipeWire's index, with their last known mute state.
    QHash<uint32_t, bool> m_streams;
    bool m_muted = false;
    bool m_available = false;
    // Mute changes sent to the sound server and not yet confirmed. While any
    // are in flight, a stream report may still show the old state, and must
    // not be mistaken for the user changing it elsewhere.
    int m_pendingMuteChanges = 0;
};
