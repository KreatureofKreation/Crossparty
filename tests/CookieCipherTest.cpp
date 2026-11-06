// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

#include "CookieCipher.h"

#include <QTest>

// The saved sign-in is only as safe as this encryption, so it is tested for
// the ways it could quietly go wrong: a reused nonce, a wrong key accepted, or
// a changed file read back as if nothing happened.
class CookieCipherTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void derivesAFixedLengthKey();
    void refusesAnEmptySecret();
    void roundTrips_data();
    void roundTrips();
    void usesAFreshNonceEverySave();
    void rejectsTheWrongKey();
    void rejectsAnyChangedByte();
    void rejectsShortOrForeignData_data();
    void rejectsShortOrForeignData();
    void refusesABadKeyLength();
};

namespace
{
QByteArray testKey()
{
    return CookieCipher::deriveKey(QByteArrayLiteral("a secret the portal might send"));
}

// The tests check has_value() first; this reads the value without a second
// check the analyser cannot see.
QByteArray valueOf(const std::optional<QByteArray> &result)
{
    return result.value_or(QByteArrayLiteral("<no value>"));
}
}

void CookieCipherTest::derivesAFixedLengthKey()
{
    const QByteArray key = testKey();
    QCOMPARE(key.size(), 32);
    QCOMPARE(CookieCipher::deriveKey(QByteArrayLiteral("a secret the portal might send")), key);
    QVERIFY(CookieCipher::deriveKey(QByteArrayLiteral("a different secret")) != key);
    // The key is derived, not the secret itself.
    QVERIFY(!key.contains(QByteArrayLiteral("secret")));
}

void CookieCipherTest::refusesAnEmptySecret()
{
    QVERIFY(CookieCipher::deriveKey(QByteArray()).isEmpty());
}

void CookieCipherTest::roundTrips_data()
{
    QTest::addColumn<QByteArray>("plaintext");

    QTest::newRow("cookies") << QByteArrayLiteral("crossparty-cookies 1\nTVNQQXV0aD12YWx1ZQ==");
    QTest::newRow("empty") << QByteArray();
    QTest::newRow("binary") << QByteArray("\0\1\2\377", 4);
    QTest::newRow("large") << QByteArray(100000, 'x');
}

void CookieCipherTest::roundTrips()
{
    QFETCH(QByteArray, plaintext);

    const auto encrypted = CookieCipher::encrypt(plaintext, testKey());
    QVERIFY(encrypted.has_value());
    if (!plaintext.isEmpty()) {
        QVERIFY(!valueOf(encrypted).contains(plaintext));
    }
    const auto decrypted = CookieCipher::decrypt(valueOf(encrypted), testKey());
    QVERIFY(decrypted.has_value());
    QCOMPARE(valueOf(decrypted), plaintext);
}

void CookieCipherTest::usesAFreshNonceEverySave()
{
    const QByteArray plaintext = QByteArrayLiteral("same input");
    const auto first = CookieCipher::encrypt(plaintext, testKey());
    const auto second = CookieCipher::encrypt(plaintext, testKey());
    QVERIFY(first.has_value() && second.has_value());
    QVERIFY(valueOf(first) != valueOf(second));
    // Bytes 4 to 15 are the nonce.
    QVERIFY(valueOf(first).mid(4, 12) != valueOf(second).mid(4, 12));
}

void CookieCipherTest::rejectsTheWrongKey()
{
    const auto encrypted = CookieCipher::encrypt(QByteArrayLiteral("sign-in"), testKey());
    QVERIFY(encrypted.has_value());
    QVERIFY(!CookieCipher::decrypt(valueOf(encrypted), CookieCipher::deriveKey(QByteArrayLiteral("another wallet"))));
}

void CookieCipherTest::rejectsAnyChangedByte()
{
    const auto encrypted = CookieCipher::encrypt(QByteArrayLiteral("sign-in cookies"), testKey());
    QVERIFY(encrypted.has_value());
    const QByteArray original = valueOf(encrypted);

    // Every byte: header, nonce, ciphertext and tag.
    for (qsizetype index = 0; index < original.size(); ++index) {
        QByteArray changed = original;
        changed[index] = static_cast<char>(changed.at(index) ^ 0x01);
        QVERIFY2(!CookieCipher::decrypt(changed, testKey()), qPrintable(QStringLiteral("byte %1").arg(index)));
    }
}

void CookieCipherTest::rejectsShortOrForeignData_data()
{
    QTest::addColumn<QByteArray>("data");

    QTest::newRow("empty") << QByteArray();
    QTest::newRow("header only") << QByteArrayLiteral("CPV1");
    QTest::newRow("one byte short") << QByteArray("CPV1").append(QByteArray(27, '\0'));
    QTest::newRow("plain text file") << QByteArrayLiteral("crossparty-cookies 1\nTVNQQXV0aD12YWx1ZQ==");
}

void CookieCipherTest::rejectsShortOrForeignData()
{
    QFETCH(QByteArray, data);
    QVERIFY(!CookieCipher::decrypt(data, testKey()));
}

void CookieCipherTest::refusesABadKeyLength()
{
    QVERIFY(!CookieCipher::encrypt(QByteArrayLiteral("x"), QByteArray(16, 'k')));
    QVERIFY(!CookieCipher::encrypt(QByteArrayLiteral("x"), QByteArray()));
    QVERIFY(!CookieCipher::decrypt(QByteArray(64, '\0'), QByteArray(16, 'k')));
}

QTEST_GUILESS_MAIN(CookieCipherTest)

#include "CookieCipherTest.moc"
