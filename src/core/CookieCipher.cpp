// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

#include "CookieCipher.h"

#include <openssl/evp.h>
#include <openssl/kdf.h>
#include <openssl/rand.h>

#include <memory>

// Layout of an encrypted file:
//
//     "CPV1" | nonce (12 bytes) | ciphertext | tag (16 bytes)
//
// "CPV1" names the format and version. It is also fed to GCM as additional
// authenticated data, so it cannot be changed without decryption failing. A
// new random nonce is made for every save; GCM is only safe if a nonce is
// never reused with the same key.

namespace
{
QByteArray formatHeader()
{
    return QByteArrayLiteral("CPV1");
}

QByteArray keyPurpose()
{
    return QByteArrayLiteral("crossparty cookie encryption key v1");
}

constexpr int keyLength = 32;
constexpr int nonceLength = 12;
constexpr int tagLength = 16;

struct CipherContextDeleter {
    void operator()(EVP_CIPHER_CTX *context) const
    {
        EVP_CIPHER_CTX_free(context);
    }
};
using CipherContext = std::unique_ptr<EVP_CIPHER_CTX, CipherContextDeleter>;

struct KeyContextDeleter {
    void operator()(EVP_PKEY_CTX *context) const
    {
        EVP_PKEY_CTX_free(context);
    }
};
using KeyContext = std::unique_ptr<EVP_PKEY_CTX, KeyContextDeleter>;

// OpenSSL works on unsigned bytes; QByteArray holds char.
const unsigned char *bytes(const QByteArray &data)
{
    return reinterpret_cast<const unsigned char *>(data.constData());
}

unsigned char *writableBytes(QByteArray &data)
{
    return reinterpret_cast<unsigned char *>(data.data());
}
}

QByteArray CookieCipher::deriveKey(const QByteArray &portalSecret)
{
    if (portalSecret.isEmpty()) {
        return {};
    }

    const QByteArray purpose = keyPurpose();
    const KeyContext context(EVP_PKEY_CTX_new_id(EVP_PKEY_HKDF, nullptr));
    QByteArray key(keyLength, '\0');
    size_t length = keyLength;
    const bool derived = context && EVP_PKEY_derive_init(context.get()) > 0 && EVP_PKEY_CTX_set_hkdf_md(context.get(), EVP_sha256()) > 0
        && EVP_PKEY_CTX_set1_hkdf_key(context.get(), bytes(portalSecret), static_cast<int>(portalSecret.size())) > 0
        && EVP_PKEY_CTX_add1_hkdf_info(context.get(), bytes(purpose), static_cast<int>(purpose.size())) > 0
        && EVP_PKEY_derive(context.get(), writableBytes(key), &length) > 0 && length == keyLength;

    return derived ? key : QByteArray();
}

std::optional<QByteArray> CookieCipher::encrypt(const QByteArray &plaintext, const QByteArray &key)
{
    if (key.size() != keyLength) {
        return std::nullopt;
    }

    const QByteArray header = formatHeader();
    QByteArray nonce(nonceLength, '\0');
    if (RAND_bytes(writableBytes(nonce), nonceLength) != 1) {
        return std::nullopt;
    }

    const CipherContext context(EVP_CIPHER_CTX_new());
    QByteArray ciphertext(plaintext.size(), '\0');
    QByteArray tag(tagLength, '\0');
    int written = 0;
    int finalWritten = 0;
    const bool encrypted = context && EVP_EncryptInit_ex(context.get(), EVP_aes_256_gcm(), nullptr, bytes(key), bytes(nonce)) == 1
        && EVP_EncryptUpdate(context.get(), nullptr, &written, bytes(header), static_cast<int>(header.size())) == 1
        && EVP_EncryptUpdate(context.get(), writableBytes(ciphertext), &written, bytes(plaintext), static_cast<int>(plaintext.size())) == 1
        && EVP_EncryptFinal_ex(context.get(), writableBytes(ciphertext) + written, &finalWritten) == 1
        && EVP_CIPHER_CTX_ctrl(context.get(), EVP_CTRL_GCM_GET_TAG, tagLength, tag.data()) == 1;

    if (!encrypted) {
        return std::nullopt;
    }
    QByteArray result = header;
    result.append(nonce).append(ciphertext).append(tag);
    return result;
}

std::optional<QByteArray> CookieCipher::decrypt(const QByteArray &encrypted, const QByteArray &key)
{
    const QByteArray header = formatHeader();
    const qsizetype overhead = header.size() + nonceLength + tagLength;
    if (key.size() != keyLength || encrypted.size() < overhead || !encrypted.startsWith(header)) {
        return std::nullopt;
    }

    const QByteArray nonce = encrypted.mid(header.size(), nonceLength);
    const QByteArray ciphertext = encrypted.mid(header.size() + nonceLength, encrypted.size() - overhead);
    QByteArray tag = encrypted.right(tagLength);

    const CipherContext context(EVP_CIPHER_CTX_new());
    QByteArray plaintext(ciphertext.size(), '\0');
    int written = 0;
    int finalWritten = 0;
    // The final step checks the tag. It fails if the key is wrong or any byte
    // of the header, nonce, ciphertext or tag has changed.
    const bool decrypted = context && EVP_DecryptInit_ex(context.get(), EVP_aes_256_gcm(), nullptr, bytes(key), bytes(nonce)) == 1
        && EVP_DecryptUpdate(context.get(), nullptr, &written, bytes(header), static_cast<int>(header.size())) == 1
        && EVP_DecryptUpdate(context.get(), writableBytes(plaintext), &written, bytes(ciphertext), static_cast<int>(ciphertext.size())) == 1
        && EVP_CIPHER_CTX_ctrl(context.get(), EVP_CTRL_GCM_SET_TAG, tagLength, tag.data()) == 1
        && EVP_DecryptFinal_ex(context.get(), writableBytes(plaintext) + written, &finalWritten) == 1;

    if (!decrypted) {
        return std::nullopt;
    }
    return plaintext;
}
