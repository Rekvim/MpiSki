#pragma once

#include <QComboBox>

enum class StrokeMovement : int
{
    Linear,
    Rotary
};

enum class SafePosition : int
{
    NormallyClosed,
    NormallyOpen
};

enum class StuffingBoxSeal : int
{
    PTFE,
    Graphite
};

enum class DriveType : int
{
    SpringDiaphragm,
    SingleSided,
    DoubleActing
};

enum class PositionerType : int
{
    Intelligent,
    EPP,
    PP,
    IPConverter
};

enum class ToolNumber : int
{
    A,
    B,
    C,
    D
};

namespace ValveEnums
{
template<typename Enum>
Enum fromComboIndex(int index, Enum defaultValue)
{
    if (index < 0) {
        return defaultValue;
    }
    return static_cast<Enum>(index);
}

inline StrokeMovement strokeMovementFromCombo(const QComboBox* combo)
{
    return fromComboIndex(combo ? combo->currentIndex() : -1,
                          StrokeMovement::Linear);
}

inline SafePosition safePositionFromCombo(const QComboBox* combo)
{
    return fromComboIndex(combo ? combo->currentIndex() : -1,
                          SafePosition::NormallyClosed);
}

inline DriveType driveTypeFromCombo(const QComboBox* combo)
{
    return fromComboIndex(combo ? combo->currentIndex() : -1,
                          DriveType::SpringDiaphragm);
}

inline StuffingBoxSeal stuffingBoxSealFromCombo(const QComboBox* combo)
{
    return fromComboIndex(combo ? combo->currentIndex() : -1,
                          StuffingBoxSeal::PTFE);
}

inline PositionerType positionerTypeFromCombo(const QComboBox* combo)
{
    return fromComboIndex(combo ? combo->currentIndex() : -1,
                          PositionerType::Intelligent);
}

inline ToolNumber toolNumberFromCombo(const QComboBox* combo)
{
    return fromComboIndex(combo ? combo->currentIndex() : -1,
                          ToolNumber::A);
}

inline QString StuffingBoxSealToString(StuffingBoxSeal seal)
{
    switch (seal) {
    case StuffingBoxSeal::PTFE:
        return QStringLiteral("PTFE");

    case StuffingBoxSeal::Graphite:
        return QStringLiteral("Graphite");
    }

    return QString();
}
}