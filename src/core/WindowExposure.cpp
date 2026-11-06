// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

#include "WindowExposure.h"

#include <QEvent>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(lcExposure, "crossparty.window", QtInfoMsg)

WindowExposure::WindowExposure(QObject *parent)
    : QObject(parent)
{
}

QWindow *WindowExposure::window() const
{
    return m_window;
}

void WindowExposure::setWindow(QWindow *window)
{
    if (m_window == window) {
        return;
    }
    if (m_window) {
        m_window->removeEventFilter(this);
    }
    m_window = window;
    if (m_window) {
        m_window->installEventFilter(this);
    }
    Q_EMIT windowChanged();
    update();
}

bool WindowExposure::isExposed() const
{
    return m_exposed;
}

bool WindowExposure::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_window && (event->type() == QEvent::Expose || event->type() == QEvent::Show || event->type() == QEvent::Hide)) {
        update();
    }
    return QObject::eventFilter(watched, event);
}

void WindowExposure::update()
{
    const bool exposed = m_window && m_window->isExposed();
    if (exposed != m_exposed) {
        m_exposed = exposed;
        qCDebug(lcExposure) << "Window exposed:" << exposed;
        Q_EMIT exposedChanged();
    }
}
