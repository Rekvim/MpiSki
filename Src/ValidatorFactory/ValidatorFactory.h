#ifndef VALIDATORFACTORY_H
#define VALIDATORFACTORY_H

#pragma once

#include <QValidator>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QHash>

class ValidatorFactory
{
public:
    enum class Type {
        Digits,         // только цифры
        LettersRusLat,  // только буквы (латиница + кириллица)
        NoSpecialChars, // запрет символов
        DigitsHyphens,  // цифры и дефис
        LettersHyphens  // буквы и дефис
    };

    static QValidator* create(Type type, QObject* parent = nullptr);

private:
    ValidatorFactory() = delete;
};


#endif // VALIDATORFACTORY_H
