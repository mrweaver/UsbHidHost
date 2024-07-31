// UsbHidMouseReport.cpp
#include "UsbHidMouseReport.h"

void UsbHidMouseReport::processReportData(const uint8_t *const data, int length)
{
    rawReport_.assign(data, data + length);

    // Print raw data in hexadecimal format
    std::string raw;
    for (int i = 0; i < length; ++i)
    {
        char buffer[3];                                                // Each byte to hex requires 2 characters + null terminator
        sprintf(buffer, "%02X", static_cast<unsigned char>(data[i]));  // Convert byte to hex
        raw += buffer;
        raw += " ";  // Add space between bytes for readability
    }
    ESP_LOGI("MouseReport", "Raw data: %s", raw.c_str());

    if (length >= sizeof(MouseReportData))
    {
        MouseReportData newReport;
        std::memcpy(&newReport, data, sizeof(MouseReportData));

        if (memcmp(&newReport, &report_, sizeof(MouseReportData)) != 0)
        {
            report_ = newReport;
            triggerEvent(createEvent());
        }
    }
    else
    {
        // Handle error: report data is too short
        std::memset(&report_, 0, sizeof(MouseReportData));
    }
}

bool UsbHidMouseReport::isButtonPressed(int button) const
{
    switch (button)
    {
    case 0:  // LEFT
        return report_.buttons.left;
    case 1:  // RIGHT
        return report_.buttons.right;
    case 2:  // MIDDLE
        return report_.buttons.middle;
    default:
        return false;
    }
}

uint8_t UsbHidMouseReport::getButtons() const
{
    return report_.buttons.val;
}

int8_t UsbHidMouseReport::getXDelta() const
{
    return report_.x_delta;
}

int8_t UsbHidMouseReport::getYDelta() const
{
    return report_.y_delta;
}

UsbHidMouseEvent UsbHidMouseReport::createEvent() const
{
    UsbHidMouseEvent event;
    event.buttons = report_.buttons.val;
    event.x_delta = report_.x_delta;
    event.y_delta = report_.y_delta;
    return event;
}