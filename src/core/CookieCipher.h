// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

#pragma once

#include <QByteArray>

#include <optional>

// Encrypts the saved cookies with AES-256-GCM, which also detects any change
// to the encrypted file. See docs/security.md#sec-009.
namespace CookieCipher
{
// Turns the secret from the Secret portal into a 32-byte key for this one
// purpose (HKDF with SHA-256). Returns an empty array if the secret is empty
// or the key cannot be made.
QByteArray deriveKey(const QByteArray &portalSecret);

// Returns the encrypted data, or nothing if encryption failed.
std::optional<QByteArray> encrypt(const QByteArray &plaintext, const QByteArray &key);

// Returns the original data, or nothing if the key is wrong or the data has
// been changed, cut short or was never ours.
std::optional<QByteArray> decrypt(const QByteArray &encrypted, const QByteArray &key);
}
