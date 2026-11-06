// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

#pragma once

#include <QObject>
#include <QPointer>
#include <QQmlEngine>
#include <QWindow>

// Tells QML whether a window is actually being drawn on screen. Wayland never
// tells an app that its window was minimised, so Window.Minimized is never
// set; KDE does, however, stop drawing a minimised window, and Qt reports that
// as the window no longer being "exposed".
class WindowExposure : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QWindow *window READ window WRITE setWindow NOTIFY windowChanged)
    Q_PROPERTY(bool exposed READ isExposed NOTIFY exposedChanged)

public:
    explicit WindowExposure(QObject *parent = nullptr);

    QWindow *window() const;
    void setWindow(QWindow *window);
    bool isExposed() const;

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void update();

    QPointer<QWindow> m_window;
    bool m_exposed = false;

Q_SIGNALS:
    void windowChanged();
    void exposedChanged();
};
