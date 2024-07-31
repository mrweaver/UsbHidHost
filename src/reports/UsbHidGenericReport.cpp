/**
 * @file UsbHidGenericReport.cpp
 * @brief Implements the UsbHidGenericReport class for handling USB HID generic reports.
 */

#include "UsbHidGenericReport.h"
#include <stdexcept>

UsbHidGenericReport::UsbHidGenericReport()
{
    // Initialize the report vector
    rawReport_.clear();
}

void UsbHidGenericReport::processReportData(const uint8_t* const data, int length)
{
    // Copy the incoming data to our internal report vector
    rawReport_.assign(data, data + length);
}

size_t UsbHidGenericReport::getReportSize() const
{
    // Return the size of the current report
    return rawReport_.size();
}

uint8_t UsbHidGenericReport::getByte(size_t index) const
{
    // Check if the index is within bounds
    if (index < rawReport_.size())
    {
        return rawReport_[index];
    }
    // Return a default value if the index is out of bounds
    // Consider logging this error if appropriate for your application
    return 0;
}

std::vector<uint8_t> UsbHidGenericReport::getReportData() const
{
    // Return a copy of the current report data
    return rawReport_;
}

UsbHidGenericEvent UsbHidGenericReport::createEvent() const
{
    // Create and return a new event based on the current report data
    UsbHidGenericEvent event;
    event.data = rawReport_;
    return event;
}