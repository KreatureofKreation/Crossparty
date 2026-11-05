// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

#pragma once

#include <QByteArray>
#include <QObject>
#include <QTimer>
#include <QVariantMap>

// Asks the desktop's Secret portal for this app's secret. KDE keeps it in
// KWallet. Inside Flatpak this needs no extra permission, and the secret is
// tied to our app ID, so other apps cannot get it. See
// docs/security.md#sec-009.
class SecretPortal : public QObject
{
    Q_OBJECT

public:
    explicit SecretPortal(QObject *parent = nullptr);
    ~SecretPortal() override;

    void requestSecret();

private:
    void fail(const QString &reason);
    void closeReadEnd();
    QByteArray readSecret() const;

    int m_readEnd = -1;
    QTimer m_timeout;

Q_SIGNALS:
    void secretReady(const QByteArray &secret);
    void secretUnavailable(const QString &reason);

private Q_SLOTS:
    void onResponse(uint response, const QVariantMap &results);
};
