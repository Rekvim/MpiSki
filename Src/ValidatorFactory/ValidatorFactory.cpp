#include "ValidatorFactory.h"
#include "RegexPatterns.h"

QValidator* ValidatorFactory::create(Type type, QObject* parent)
{
    static const QHash<Type, QRegularExpression> patterns = {
        { Type::Digits,         QRegularExpression(RegexPatterns::digits()) },
        { Type::DigitsHyphens,  QRegularExpression(RegexPatterns::digitsHyphens()) },
        { Type::LettersHyphens, QRegularExpression(RegexPatterns::lettersHyphens()) },
        { Type::NoSpecialChars, QRegularExpression(RegexPatterns::noSpecialChars()) },

    };

    auto it = patterns.find(type);
    if (it != patterns.end())
        return new QRegularExpressionValidator(*it, parent);

    return new QRegularExpressionValidator(
        QRegularExpression(QStringLiteral("^.*$")), parent);
}
