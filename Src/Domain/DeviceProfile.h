#pragma once

namespace Domain {

enum class ValveType {
    Control,        // CVT  — регулирующая арматура (только аналоговый сигнал)
    Shutoff,        // SOVT — отсечная арматура (только DI/DO)
    ShutoffControl  // SACVT — запорно-регулирующая (аналог + DI/DO)
};

enum class TestLevel {
    Basic,   // Б — базовый
    Complex  // К — комплексный
};

struct DeviceProfile {
    ValveType valveType = ValveType::Control;
    TestLevel testLevel = TestLevel::Basic;

    bool pressure1    = false;
    bool pressure2    = false;
    bool pressure3    = false;
    bool outputSignal = false; // output_4_20_mA — обратная связь

    bool do1 = false;
    bool do2 = false;
    bool do3 = false;
    bool do4 = false;

    bool hasShutoff() const {
        return valveType == ValveType::Shutoff
            || valveType == ValveType::ShutoffControl;
    }
    bool hasControl() const {
        return valveType == ValveType::Control
            || valveType == ValveType::ShutoffControl;
    }
    bool hasImitSwitch() const { return do1 || do2 || do3 || do4; }
    bool isComplex()    const { return testLevel == TestLevel::Complex; }
};

} // namespace Domain
