#pragma once

#include "UsbHidBaseReport.h"
#include <cstdint>
#include <unordered_map>
#include <string>

#include <esp_log.h>

enum class G20sProBtn : uint8_t
{
    Power,
    Mute,
    PgUp,
    PgDown,
    Left,
    Down,
    Up,
    Right,
    Enter,
    Back,
    Home,
    VolDown,
    Mic,
    VolUp,
    Prev,
    Play,
    Next,
    Num1,
    Num2,
    Num3,
    Num4,
    Num5,
    Num6,
    Num7,
    Num8,
    Num9,
    Num0,
    Backspace,
    App,
    MouseLeft,
    MouseRight,
    Unknown
};

struct ButtonCode
{
    uint8_t reportId;
    uint16_t code;  // up to 2 bytes
};

struct UsbHidG20sProEvent
{
    UsbHidDeviceType deviceType_;
    G20sProBtn button = G20sProBtn::Unknown;
    bool pressed      = false;
    int8_t mouseX     = 0;
    int8_t mouseY     = 0;

    UsbHidG20sProEvent() : deviceType_(UsbHidDeviceType::G20sPro), pressed(false), mouseX(0), mouseY(0) {}
};

class UsbHidG20sProReport : public UsbHidBaseReport<UsbHidG20sProEvent, UsbHidDeviceType::G20sPro>
{
public:
    UsbHidG20sProReport();
    void processReportData(const uint8_t* const data, int length) override;
    static std::string buttonName(G20sProBtn button);

protected:
private:
    UsbHidG20sProEvent createEvent() const override;

    static const std::unordered_map<G20sProBtn, ButtonCode> btnCodeMap;
    static const std::unordered_map<G20sProBtn, std::string> btnNames;

    struct ReportData
    {
        uint8_t reportId;
        union
        {
            struct
            {
                uint8_t buttons;
                int8_t x;
                int8_t y;
            } mouse;
            struct
            {
                uint8_t keyCode;
                uint8_t reserved[2];
            } button;
            uint8_t raw[3];
        } data;
    } __attribute__((packed));

    ReportData report_;
    G20sProBtn lastPressedButton;
    bool buttonPressed;
    int8_t mouseX;
    int8_t mouseY;

    void processMouseReport(const uint8_t* data, int length);
    void processButtonReport(const uint8_t* data, int length);
    static G20sProBtn buttonFromCode(uint8_t reportId, uint16_t code);
};