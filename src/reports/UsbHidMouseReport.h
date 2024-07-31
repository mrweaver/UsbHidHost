// UsbHidMouseReport.h
#pragma once

#include "UsbHidBaseReport.h"
#include <cstdint>
#include <cstring>
#include <string>

#include <esp_log.h>

struct UsbHidMouseEvent
{
    UsbHidDeviceType deviceType_;
    uint8_t buttons;
    int8_t x_delta;
    int8_t y_delta;

    UsbHidMouseEvent() : deviceType_(UsbHidDeviceType::Mouse), buttons(0), x_delta(0), y_delta(0) {}
};

class UsbHidMouseReport : public UsbHidBaseReport<UsbHidMouseEvent, UsbHidDeviceType::Mouse>
{
public:
    UsbHidMouseReport() : UsbHidBaseReport() {}  // Is this correct?
    void processReportData(const uint8_t* const data, int length) override;

    bool isButtonPressed(int button) const;
    uint8_t getButtons() const;
    int8_t getXDelta() const;
    int8_t getYDelta() const;

protected:
    UsbHidMouseEvent createEvent() const override;

private:
    struct MouseReportData
    {
        union
        {
            struct
            {
                uint8_t left : 1;
                uint8_t right : 1;
                uint8_t middle : 1;
                uint8_t reserved : 5;
            };
            uint8_t val;
        } buttons;
        int8_t x_delta;
        int8_t y_delta;
    } __attribute__((packed));

    MouseReportData report_;
};