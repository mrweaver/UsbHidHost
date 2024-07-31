/**
 * @file UsbHidBaseReport.h
 * @brief Defines the base class for USB HID reports with caching mechanism.
 */

#pragma once

#include <vector>
#include <cstdint>
#include <functional>

/**
 * @enum UsbHidDeviceType
 * @brief Enumeration of supported USB HID device types.
 */
enum class UsbHidDeviceType : uint8_t
{
    Keyboard,  ///< Keyboard device
    Mouse,     ///< Mouse device
    G20sPro,   ///< G20s Pro device
    Generic    ///< Generic HID device
};

/**
 * @class UsbHidBaseReport
 * @brief Base class for handling USB HID reports with caching.
 *
 * This template class provides a common interface and functionality
 * for processing different types of USB HID reports, including a caching
 * mechanism for raw report data.
 *
 * @tparam EventType The type of event this report generates.
 */
template <typename EventType, UsbHidDeviceType DeviceType>
class UsbHidBaseReport
{
public:
    /**
     * @typedef EventCallback
     * @brief Function type for event callbacks.
     */
    using EventCallback = std::function<void(const EventType&)>;

    /**
     * @brief Construct a new UsbHidBaseReport object.
     *
     * @param type The type of USB HID device this report represents.
     */
    explicit UsbHidBaseReport() : deviceType_(DeviceType) {}

    /**
     * @brief Destroy the UsbHidBaseReport object.
     */
    virtual ~UsbHidBaseReport() = default;

    /**
     * @brief Process raw report data from the USB HID device.
     *
     * This method should be implemented by derived classes to handle
     * device-specific report data processing.
     *
     * @param data Pointer to the raw report data.
     * @param length Length of the raw report data.
     */
    virtual void processReportData(const uint8_t* const data, int length) = 0;

    /**
     * @brief Get the current raw report data.
     *
     * @return std::vector<uint8_t> The raw report data.
     */
    std::vector<uint8_t> getRawReport() const
    {
        return rawReport_;
    }

    /**
     * @brief Get the type of the USB HID device.
     *
     * @return UsbHidDeviceType The type of the device.
     */
    UsbHidDeviceType getDeviceType() const { return deviceType_; }

    /**
     * @brief Register a callback function for report events.
     *
     * @param callback The callback function to register.
     */
    void registerCallback(EventCallback callback)
    {
        callbacks.push_back(std::move(callback));
    }

protected:
    UsbHidDeviceType deviceType_;     ///< The type of USB HID device this report represents.
    std::vector<uint8_t> rawReport_;  ///< Cached raw report data.

    /**
     * @brief Trigger the event for all registered callbacks.
     *
     * @param event The event to trigger.
     */
    void triggerEvent(const EventType& event)
    {
        for (const auto& callback : callbacks)
        {
            callback(event);
        }
    }

    /**
     * @brief Create an event based on the current report state.
     *
     * This method should be implemented by derived classes to create
     * device-specific events.
     *
     * @return EventType The created event.
     */
    virtual EventType createEvent() const = 0;

private:
    std::vector<EventCallback> callbacks;  ///< List of registered callback functions.
};