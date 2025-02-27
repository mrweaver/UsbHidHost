#include "UsbHidG20sProReport.h"
#include <sstream>
#include <iomanip>

UsbHidG20sProReport::UsbHidG20sProReport()
{
    // Initialize the report vector
    rawReport_.clear();
}

const std::unordered_map<G20sProBtn, ButtonCode> UsbHidG20sProReport::btnCodeMap = {
    {G20sProBtn::Power, {0x05, 0x01}},
    {G20sProBtn::Mute, {0x04, 0xE2}},
    {G20sProBtn::PgUp, {0x01, 0x4B}},
    {G20sProBtn::PgDown, {0x01, 0x4E}},
    {G20sProBtn::Left, {0x01, 0x50}},
    {G20sProBtn::Down, {0x01, 0x51}},
    {G20sProBtn::Up, {0x01, 0x52}},
    {G20sProBtn::Right, {0x01, 0x4F}},
    {G20sProBtn::Enter, {0x01, 0x28}},
    {G20sProBtn::Back, {0x04, 0x24}},
    {G20sProBtn::Home, {0x04, 0x23}},
    {G20sProBtn::VolDown, {0x04, 0xEA}},
    {G20sProBtn::Mic, {0x04, 0xCF}},
    {G20sProBtn::VolUp, {0x04, 0xE9}},
    {G20sProBtn::Prev, {0x04, 0xB6}},
    {G20sProBtn::Play, {0x04, 0xCD}},
    {G20sProBtn::Next, {0x04, 0xB5}},
    {G20sProBtn::Num1, {0x01, 0x1E}},
    {G20sProBtn::Num2, {0x01, 0x1F}},
    {G20sProBtn::Num3, {0x01, 0x20}},
    {G20sProBtn::Num4, {0x01, 0x21}},
    {G20sProBtn::Num5, {0x01, 0x22}},
    {G20sProBtn::Num6, {0x01, 0x23}},
    {G20sProBtn::Num7, {0x01, 0x24}},
    {G20sProBtn::Num8, {0x01, 0x25}},
    {G20sProBtn::Num9, {0x01, 0x26}},
    {G20sProBtn::Num0, {0x01, 0x27}},
    {G20sProBtn::Backspace, {0x01, 0x2A}},
    {G20sProBtn::App, {0x01, 0x65}},
    {G20sProBtn::Unknown, {0x00, 0x00}},
};

const std::unordered_map<G20sProBtn, std::string> UsbHidG20sProReport::btnNames = {
    {G20sProBtn::Power, "Power"},
    {G20sProBtn::Mute, "Mute"},
    {G20sProBtn::PgUp, "PgUp"},
    {G20sProBtn::PgDown, "PgDown"},
    {G20sProBtn::Left, "Left"},
    {G20sProBtn::Down, "Down"},
    {G20sProBtn::Up, "Up"},
    {G20sProBtn::Right, "Right"},
    {G20sProBtn::Enter, "Enter"},
    {G20sProBtn::Back, "Back"},
    {G20sProBtn::Home, "Home"},
    {G20sProBtn::VolDown, "VolDown"},
    {G20sProBtn::Mic, "Mic"},
    {G20sProBtn::VolUp, "VolUp"},
    {G20sProBtn::Prev, "Prev"},
    {G20sProBtn::Play, "Play"},
    {G20sProBtn::Next, "Next"},
    {G20sProBtn::Num1, "1"},
    {G20sProBtn::Num2, "2"},
    {G20sProBtn::Num3, "3"},
    {G20sProBtn::Num4, "4"},
    {G20sProBtn::Num5, "5"},
    {G20sProBtn::Num6, "6"},
    {G20sProBtn::Num7, "7"},
    {G20sProBtn::Num8, "8"},
    {G20sProBtn::Num9, "9"},
    {G20sProBtn::Num0, "0"},
    {G20sProBtn::Backspace, "Backspace"},
    {G20sProBtn::App, "App"},
    {G20sProBtn::MouseLeft, "MouseLeft"},
    {G20sProBtn::MouseRight, "MouseRight"},
    {G20sProBtn::Unknown, "Unknown"}};

void UsbHidG20sProReport::processReportData(const uint8_t* const data, int length)
{
    // Copy the incoming data to our internal report vector
    // Q: How to enforce this in a derived class?
    rawReport_.assign(data, data + length);

    if (length < 1)
    {
        ESP_LOGE("G20sProReport", "Invalid report length: %d", length);
        return;
    }

    if (length == 4)
    {
        processMouseReport(data, length);
    }
    else if ((length == 8 && data[0] == 0x01) ||
             (length == 3 && data[0] == 0x04) ||
             (length == 2 && data[0] == 0x05))
    {
        processButtonReport(data, length);
    }
    else
    {
        ESP_LOGW("G20sProReport", "Unknown report type. Length: %d, First byte: 0x%02X", length, data[0]);
    }

    triggerEvent(createEvent());
}

void UsbHidG20sProReport::processMouseReport(const uint8_t* data, int length)
{
    report_.reportId           = 0x00;  // Assume 0x00 for mouse reports
    report_.data.mouse.buttons = data[0];
    report_.data.mouse.x       = static_cast<int8_t>(data[1]);
    report_.data.mouse.y       = static_cast<int8_t>(data[2]);

    mouseX = report_.data.mouse.x;
    mouseY = report_.data.mouse.y;

    buttonPressed = (report_.data.mouse.buttons != 0);
    if (buttonPressed)
    {
        if (report_.data.mouse.buttons & 0x01)
        {
            lastPressedButton = G20sProBtn::MouseLeft;
        }
        else if (report_.data.mouse.buttons & 0x02)
        {
            lastPressedButton = G20sProBtn::MouseRight;
        }
    }
}

void UsbHidG20sProReport::processButtonReport(const uint8_t* data, int length)
{
    report_.reportId            = data[0];
    report_.data.button.keyCode = 0;
    uint8_t nonZeroCount        = 0;

    for (int i = 1; i < length; ++i)
    {
        if (data[i] != 0)
        {
            if (nonZeroCount < 2)
            {
                report_.data.button.keyCode |= data[i] << (8 * nonZeroCount);
                ++nonZeroCount;
            }
            else
            {
                // Error handling for more than 2 non-zero bytes
                ESP_LOGE("G20sProReport", "Error: More than 2 non-zero bytes in keyCode");
                return;
            }
        }
    }

    ESP_LOGI("G20sProReport", "reportId=0x%02X, keyCode=0x%04X", report_.reportId, report_.data.button.keyCode);

    buttonPressed = (report_.data.button.keyCode != 0);
    if (buttonPressed)
    {
        lastPressedButton = buttonFromCode(report_.reportId, report_.data.button.keyCode);
    }
}

UsbHidG20sProEvent UsbHidG20sProReport::createEvent() const
{
    UsbHidG20sProEvent event;
    event.button  = lastPressedButton;
    event.pressed = buttonPressed;
    event.mouseX  = mouseX;
    event.mouseY  = mouseY;
    return event;
}

G20sProBtn UsbHidG20sProReport::buttonFromCode(uint8_t reportId, uint16_t code)
{
    for (const auto& mapping : btnCodeMap)
    {
        if (mapping.second.reportId == reportId && mapping.second.code == code)
        {
            return mapping.first;
        }
    }

    ESP_LOGW("G20sProReport", "Unknown button code: reportId=0x%02X, code=0x%04X", reportId, code);
    return G20sProBtn::Unknown;
}

std::string UsbHidG20sProReport::buttonName(G20sProBtn button)
{
    auto it = btnNames.find(button);
    if (it != btnNames.end())
    {
        return it->second;
    }

    return "Unknown";
}