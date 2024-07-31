/**
 * @file UsbHidGenericReport.hp
 * @brief Defines the UsbHidGenericReport class for handling USB HID generic reports.
 */

#pragma once

#include "UsbHidBaseReport.h"
#include <vector>
#include <cstdint>

/**
 * @struct UsbHidGenericEvent
 * @brief Represents a generic HID event with raw data.
 */
struct UsbHidGenericEvent
{
    UsbHidDeviceType deviceType_;  ///< Type of the USB HID device
    std::vector<uint8_t> data;     ///< Raw data from the HID report

    /**
     * @brief Construct a new UsbHidGenericEvent object.
     */
    UsbHidGenericEvent() : deviceType_(UsbHidDeviceType::Generic) {}
};

/**
 * @class UsbHidGenericReport
 * @brief Handles processing and interpretation of USB HID generic reports.
 *
 * This class extends UsbHidBaseReport to provide functionality for generic HID devices.
 * It processes raw HID reports and provides methods to access the report data.
 */
class UsbHidGenericReport : public UsbHidBaseReport<UsbHidGenericEvent, UsbHidDeviceType::Generic>
{
public:
    /**
     * @brief Construct a new UsbHidGenericReport object.
     */
    UsbHidGenericReport();

    /**
     * @brief Process raw report data from the USB HID device.
     *
     * @param data Pointer to the raw report data.
     * @param length Length of the raw report data.
     */
    void processReportData(const uint8_t* const data, int length) override;

    /**
     * @brief Get the size of the current report.
     *
     * @return size_t The size of the report in bytes.
     */
    size_t getReportSize() const;

    /**
     * @brief Get a specific byte from the report.
     *
     * @param index The index of the byte to retrieve.
     * @return uint8_t The byte at the specified index.
     * @throw std::out_of_range If the index is out of bounds.
     */
    uint8_t getByte(size_t index) const;

    /**
     * @brief Get a copy of the entire report data.
     *
     * @return std::vector<uint8_t> A copy of the report data.
     */
    std::vector<uint8_t> getReportData() const;

protected:
    /**
     * @brief Create a UsbHidGenericEvent based on the current report state.
     *
     * @return UsbHidGenericEvent The created event.
     */
    UsbHidGenericEvent createEvent() const override;

private:
};