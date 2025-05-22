#ifndef REGEXPATTERNS_H
#define REGEXPATTERNS_H

#pragma once

#include <QRegularExpression>

namespace RegexPatterns {
// только цифры 0–9
inline const QRegularExpression& digits() {
    static const QRegularExpression re(QStringLiteral("^[0-9]*$"));
    return re;
}

// цифры и дефис
inline const QRegularExpression& digitsHyphens() {
    static const QRegularExpression re(QStringLiteral("^[0-9-]*$"));
    return re;
}

// буквы рус и латин
inline const QRegularExpression& lettersHyphens() {
    static const QRegularExpression re(QStringLiteral("^[A-Za-zА-Яа-яЁё]*$"));
    return re;
}
// запрет символов
inline const QRegularExpression& noSpecialChars() {
    static const QRegularExpression re(QStringLiteral("^[^@!^\\/\\?\\*\\:\\;\\{\\}\\\\]*$"));
    return re;
}
}

#endif // REGEXPATTERNS_H
