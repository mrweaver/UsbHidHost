/**
 * @brief G20S Pro Remote Buttons
 * 
 * @details
 * | Button  | Description | Report ID | Key Code |
 * | ------- | ----------- | --------- | -------- |
 * | Power   | Power       | 0x05      | 0x01     |
 * | Mute    | Mute        | 0x05      | 0xE2     |
 * | Pg+     | Page up     | 0x01      | 0x4B     |
 * | Pg-     | Page down   | 0x01      | 0x4E     |
 * | ArrowL  | Arrow left  | 0x01      | 0x50     |
 * | ArrowDn | Arrow down  | 0x01      | 0x51     |
 * | ArrowUp | Arrow up    | 0x01      | 0x52     |
 * | ArrowR  | Arrow right | 0x01      | 0x4F     |
 * | Enter   | Enter       | 0x01      | 0x28     |
 * | Back    | Back        | 0x04      | 0x24 02  |
 * | Home    | Home        | 0x04      | 0x23 02  |
 * | Vol-    | Volume down | 0x04      | 0xEA     |
 * | Mic     | Microphone  | 0x04      | 0xCF     |
 * | Vol+    | Volume up   | 0x04      | 0xE9     |
 * | Prev    | Previous    | 0x04      | 0xB6     |
 * | Play    | Play/Pause  | 0x04      | 0xCD     |
 * | Next    | Next        | 0x04      | 0xB5     |
 * | 1       | 1           | 0x01      | 0x1E     |
 * | 2       | 2           | 0x01      | 0x1F     |
 * | 3       | 3           | 0x01      | 0x20     |
 * | 4       | 4           | 0x01      | 0x21     |
 * | 5       | 5           | 0x01      | 0x22     |
 * | 6       | 6           | 0x01      | 0x23     |
 * | 7       | 7           | 0x01      | 0x24     |
 * | 8       | 8           | 0x01      | 0x25     |
 * | 9       | 9           | 0x01      | 0x26     |
 * | 0       | 0           | 0x01      | 0x27     |
 * | Backsp  | Backspace   | 0x01      | 0x2A     |
 * | App     | Application | 0x01      | 0x65     |
 */


#pragma once

// Does this need to be in the build flags?
//  if < 100 rededfine CONFIG_USB_HOST_RESET_RECOVERY_MS=100 for G20sPro
#if CONFIG_USB_HOST_RESET_RECOVERY_MS < 100
#define CONFIG_USB_HOST_RESET_RECOVERY_MS 100
#endif

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