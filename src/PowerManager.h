#pragma once

#define XPOWERS_CHIP_AXP2101
#include "XPowersLib.h"

class PowerManager {

    public:
        PowerManager();
        ~PowerManager();

        bool setup();
        void loop();

        bool isModemPowered();
        bool modemPower(bool enable);

        XPowersPMU* getPMU() { return this->PMU; }

        void enableDeepSleep();
        void disableDeepSleep();

    private:
        XPowersPMU* PMU = nullptr;
};