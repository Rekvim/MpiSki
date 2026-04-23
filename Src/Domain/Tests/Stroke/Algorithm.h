#pragma once

#include "Domain/Tests/Test.h"

namespace Domain::Tests::Stroke {
    class Algorithm : public Test
    {
        Q_OBJECT
    public:
        explicit Algorithm(QObject *parent = nullptr) : Test(parent) {}
        void run() override;

    signals:
        void results();
    };
}