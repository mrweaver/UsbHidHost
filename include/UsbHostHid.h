#pragma once

#include <variant>
#include <vector>
#include <map>
#include <cstdint>
#include <cstring>
#include <string>
#include <algorithm>
#include <optional>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"

#include "usb/usb_host.h"
#include "usb/hid_host.h"

#include "reports/UsbHidG20sProReport.h"
#include "reports/UsbHidKeyboardReport.h"
#include "reports/UsbHidMouseReport.h"
#include "reports/UsbHidGenericReport.h"

class UsbHostHid
{
public:
    struct UsbHidEvent
    {
        hid_host_device_handle_t deviceHandle;
        hid_host_driver_event_t driverEvent;
        void* eventData;
    };

    struct Report
    {
        hid_host_device_handle_t deviceHandle;
        std::vector<uint8_t> reportData;
    };

    UsbHostHid();
    ~UsbHostHid();

    esp_err_t init();
    esp_err_t deinit();
    esp_err_t start();
    esp_err_t stop();

    // Register a callback to use USB HID device events (e.g. LVGL input device)
    void registerHIDCallback(std::function<void(const UsbHidEvent&)> callback) {};  // TODO

    // Reporters
    UsbHidG20sProReport* reportG20sPro() { return &g20sProReport; }
    UsbHidKeyboardReport* reportKeyboard() { return &keyboardReport; }
    UsbHidMouseReport* reportMouse() { return &mouseReport; }
    UsbHidGenericReport* reportGeneric() { return &genericReport; }

private:
    static constexpr const char* TAG                         = "UsbHostHid";
    static constexpr size_t EVENT_QUEUE_SIZE                 = 10;
    static constexpr size_t USB_TASK_STACK_SIZE              = 8192;
    static constexpr UBaseType_t USB_TASK_PRIORITY           = 2;
    static constexpr UBaseType_t HID_PROCESSOR_TASK_PRIORITY = 5;

    UsbHidG20sProReport g20sProReport;
    UsbHidKeyboardReport keyboardReport;
    UsbHidMouseReport mouseReport;
    UsbHidGenericReport genericReport;

    QueueHandle_t eventQueue;  // FreeRTOS queue for incoming USB events

    TaskHandle_t hidProcessorTaskHandle;
    TaskHandle_t usbLibTaskHandle;
    std::vector<hid_host_device_handle_t> connectedDevices;

    static void usbLibTask(void* pvParameters);

    static void hidEventProcessorTaskTrampoline(void* arg);
    void hidEventProcessorTask();

    static void hidHostDeviceCallback(hid_host_device_handle_t hid_device_handle,
                                      const hid_host_driver_event_t event,
                                      void* arg);

    static void hidHostInterfaceCallback(hid_host_device_handle_t hid_device_handle,
                                         const hid_host_interface_event_t event,
                                         void* arg);

    void handleHidHostEvent(hid_host_device_handle_t hid_device_handle,
                            const hid_host_driver_event_t event,
                            void* arg);

    void addEventToQueue(const UsbHidEvent& event);

    static bool usbEnumerationFilterCallback(const usb_device_desc_t *dev_desc, uint8_t *bConfigurationValue);
};