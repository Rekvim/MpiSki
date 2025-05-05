#include "ValidatorFactory.h"

QValidator* ValidatorFactory::create(Type type, QObject* parent)
{
    static const QHash<Type, QRegularExpression> patterns = {
        { Type::Digits,         QRegularExpression(QStringLiteral("^[0-9]*$")) },
        { Type::LettersRusLat,  QRegularExpression(QStringLiteral("^[A-Za-zА-Яа-яЁё]*$")) },
        { Type::NoSpecialChars, QRegularExpression(QStringLiteral("^[^@!^\\/\\?\\*\\:\\;\\{\\}\\\\]*$")) },
        { Type::DigitsHyphens,  QRegularExpression(QStringLiteral("^[0-9-]*$")) },
        { Type::LettersHyphens, QRegularExpression(QStringLiteral("^[A-Za-zА-Яа-яЁё-]*$")) }
    };

    auto it = patterns.find(type);
    if (it != patterns.end())
        return new QRegularExpressionValidator(*it, parent);

    return new QRegularExpressionValidator(
        QRegularExpression(QStringLiteral("^.*$")), parent);
}
