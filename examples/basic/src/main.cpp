// --- START OF FILE main.cpp ---
#include <stdio.h>
#include <string>
#include "UsbHostHid.h"
#include "esp_log.h"

static const char* TAG = "main";

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "USB HID Host Example");
    esp_log_level_set("USB_HOST", ESP_LOG_VERBOSE);

    UsbHostHid usbHostHid;

    // Initialize the USB Host HID instance
    esp_err_t ret = usbHostHid.init();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize USB Host HID: %s", esp_err_to_name(ret));
        return;
    }

    // Start processing HID events
    ret = usbHostHid.start();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to start USB Host HID: %s", esp_err_to_name(ret));
        return;
    }

    ESP_LOGI(TAG, "USB Host HID started");

    // G20sPro report
    // struct UsbHidG20sProEvent
    // {
    //     UsbHidDeviceType deviceType_;
    //     G20sProBtn button;
    //     bool pressed;
    //     int8_t mouseX;
    //     int8_t mouseY;

    //     UsbHidG20sProEvent() : deviceType_(UsbHidDeviceType::G20sPro), pressed(false), mouseX(0), mouseY(0) {}
    // };
    UsbHidG20sProReport* g20sProReport = usbHostHid.reportG20sPro();
    g20sProReport->registerCallback([](const UsbHidG20sProEvent event)
                                    {
                                        if(event.pressed)
                                            ESP_LOGI(TAG, "G20sPro event: button: %s | x: %03d y: %03d", UsbHidG20sProReport::buttonName(event.button).c_str(), event.mouseX, event.mouseY); });
    // Keyboard report
    UsbHidKeyboardReport* keyboardReport = usbHostHid.reportKeyboard();
    keyboardReport->registerCallback([](const UsbHidKeyboardEvent event)
                                     {
                                        if(event.keyCodes.size() == 0)
                                        return;

                                         std::string keys;
                                         for (auto keyCode : event.keyCodes)
                                         {
                                             keys += UsbHidKeyboardReport::getKeyName(keyCode);
                                         }
                                         ESP_LOGW(TAG, "Key: %s", keys.c_str());

                                         ESP_LOGW(TAG, "Modifiers: %s", UsbHidKeyboardReport::getModifierNames(event.modifiers).c_str()); });

    // Mouse report
    UsbHidMouseReport* mouseReport = usbHostHid.reportMouse();
    mouseReport->registerCallback([](const UsbHidMouseEvent event)
                                  { ESP_LOGI(TAG, "Mouse x: %03d y: %03d", event.x_delta, event.y_delta); });

    // Example: Keep the program running
    while (true)
    {
        // Read the USB reports

        vTaskDelay(pdMS_TO_TICKS(10));
    }

    // Stop processing HID eventsU
    usbHostHid.stop();

    // Deinitialize the USB Host HID instance
    usbHostHid.deinit();
}