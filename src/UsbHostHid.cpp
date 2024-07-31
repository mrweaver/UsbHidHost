// --- START OF FILE UsbHostHid.cpp ---
#include "UsbHostHid.h"

/**
 * @brief HID Protocol string names
 */
static const std::vector<std::string> HID_PROTO_NAMES = {
    "NONE",
    "Keyboard",
    "Mouse"};

UsbHostHid::UsbHostHid()
    : hidProcessorTaskHandle(nullptr),
      usbLibTaskHandle(nullptr)
{
    eventQueue = xQueueCreate(EVENT_QUEUE_SIZE, sizeof(UsbHidEvent));
    if (eventQueue == nullptr)
    {
        ESP_LOGE(TAG, "Failed to create USB event queue");
    }
}

UsbHostHid::~UsbHostHid()
{
    deinit();
    if (eventQueue != nullptr)
    {
        vQueueDelete(eventQueue);
    }
}

esp_err_t UsbHostHid::init()
{
    // Create the USB lib task
    BaseType_t taskCreated = xTaskCreate(
        usbLibTask,
        "usb_events",
        USB_TASK_STACK_SIZE,
        xTaskGetCurrentTaskHandle(),
        USB_TASK_PRIORITY,
        &usbLibTaskHandle);

    if (taskCreated != pdPASS)
    {
        ESP_LOGE(TAG, "Failed to create USB lib task");
        return ESP_FAIL;
    }

    // Wait for notification from usb_lib_task to proceed
    if (ulTaskNotifyTake(pdFALSE, 1000) == 0)
    {
        ESP_LOGE(TAG, "USB lib task failed to start");
        vTaskDelete(usbLibTaskHandle);
        return ESP_FAIL;
    }

    // Install HID host driver
    const hid_host_driver_config_t driverConfig = {
        .create_background_task = true,
        .task_priority          = HID_PROCESSOR_TASK_PRIORITY,
        .stack_size             = USB_TASK_STACK_SIZE,
        .core_id                = tskNO_AFFINITY,
        .callback               = hidHostDeviceCallback,
        .callback_arg           = this};

    ESP_ERROR_CHECK(hid_host_install(&driverConfig));

    ESP_LOGI(TAG, "Waiting for HID devices to connect...");

    return ESP_OK;
}

esp_err_t UsbHostHid::deinit()
{
    if (hidProcessorTaskHandle != nullptr)
    {
        vTaskDelete(hidProcessorTaskHandle);
        hidProcessorTaskHandle = nullptr;
    }

    if (usbLibTaskHandle != nullptr)
    {
        // Wait for the USB lib task to finish
        while (eTaskGetState(usbLibTaskHandle) != eDeleted)
        {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
        usbLibTaskHandle = nullptr;
    }

    if (eventQueue != nullptr)
    {
        vQueueDelete(eventQueue);
        eventQueue = nullptr;
    }

    esp_err_t ret = hid_host_uninstall();
    if (ret != ESP_OK)
    {
        ESP_LOGW(TAG, "Failed to uninstall HID host driver: %s", esp_err_to_name(ret));
    }

    return ret;
}

esp_err_t UsbHostHid::start()
{
    BaseType_t task_created = xTaskCreate(
        hidEventProcessorTaskTrampoline,
        "hidEventProcessor",
        USB_TASK_STACK_SIZE,
        this,
        HID_PROCESSOR_TASK_PRIORITY,
        &hidProcessorTaskHandle);

    if (task_created != pdPASS)
    {
        ESP_LOGE(TAG, "Failed to create HID event processor task");
        return ESP_FAIL;
    }

    // Force connected devices to be re-enumerated

    return ESP_OK;
}

esp_err_t UsbHostHid::stop()
{
    if (hidProcessorTaskHandle != nullptr)
    {
        vTaskDelete(hidProcessorTaskHandle);
        hidProcessorTaskHandle = nullptr;
    }
    return ESP_OK;
}

/**
 * @brief Start USB Host install and handle common USB host library events while app pin not low
 *
 */
void UsbHostHid::usbLibTask(void* pvParameters)
{
    const usb_host_config_t host_config = {
        .skip_phy_setup = false,
        .intr_flags     = ESP_INTR_FLAG_LEVEL1,
        .enum_filter_cb = usbEnumerationFilterCallback,
    };

    esp_err_t ret = usb_host_install(&host_config);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to install USB host: %s", esp_err_to_name(ret));
    }

    TaskHandle_t xTask = static_cast<TaskHandle_t>(pvParameters);
    xTaskNotifyGive(xTask);

    ESP_LOGI(TAG, "USB host installed");

    while (true)
    {
        uint32_t event_flags;
        esp_err_t ret = usb_host_lib_handle_events(portMAX_DELAY, &event_flags);

        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "USB host event handling failed: %s", esp_err_to_name(ret));
        }
        else
        {
            ESP_LOGI(TAG, "USB host event flags: 0x%" PRIx32, event_flags);
        }

        // Check info of USB Host Library
        usb_host_lib_info_t info;
        ESP_ERROR_CHECK(usb_host_lib_info(&info));
        ESP_LOGI(TAG, "USB Host Library Info: Devices: %d, Clients: %d", info.num_devices, info.num_clients);

        // ESP_LOGI(TAG, "USB host event handling: %lu", event_flags);

        // ret = usb_host_lib_handle_events(portMAX_DELAY, &event_flags);
        // if (ret != ESP_OK)
        // {
        //     ESP_LOGE(TAG, "USB host event handling failed: %s", esp_err_to_name(ret));
        //     break;
        // }

        // Handle USB events if needed
        if (event_flags & USB_HOST_LIB_EVENT_FLAGS_NO_CLIENTS)
        {
            usb_host_device_free_all();
            ESP_LOGW(TAG, "No more clients");
            break;
        }
        if (event_flags & USB_HOST_LIB_EVENT_FLAGS_ALL_FREE)
        {
            ESP_LOGW(TAG, "USB Device disconnected");
        }

        // vTaskDelay(1)  // Yield to other tasks
    }

    ESP_LOGI(TAG, "USB shutdown");
    vTaskDelay(10);  // Short delay to allow clients clean-up

    ret = usb_host_uninstall();
    if (ret != ESP_OK)
    {
        ESP_LOGW(TAG, "Failed to uninstall USB host: %s", esp_err_to_name(ret));
    }

    vTaskDelete(NULL);
}

bool UsbHostHid::usbEnumerationFilterCallback(const usb_device_desc_t* dev_desc, uint8_t* bConfigurationValue)
{
    ESP_LOGI(TAG, "USB Device Enumeration:");
    ESP_LOGI(TAG, "  Descriptor Length: %d", dev_desc->bLength);
    ESP_LOGI(TAG, "  Descriptor Type: 0x%02X", dev_desc->bDescriptorType);
    ESP_LOGI(TAG, "  USB Version: %d.%02d", dev_desc->bcdUSB >> 8, dev_desc->bcdUSB & 0xFF);
    ESP_LOGI(TAG, "  Device Class: 0x%02X", dev_desc->bDeviceClass);
    ESP_LOGI(TAG, "  Device Subclass: 0x%02X", dev_desc->bDeviceSubClass);
    ESP_LOGI(TAG, "  Device Protocol: 0x%02X", dev_desc->bDeviceProtocol);
    ESP_LOGI(TAG, "  Max Packet Size (EP0): %d", dev_desc->bMaxPacketSize0);
    ESP_LOGI(TAG, "  Vendor ID: 0x%04X", dev_desc->idVendor);
    ESP_LOGI(TAG, "  Product ID: 0x%04X", dev_desc->idProduct);
    ESP_LOGI(TAG, "  Device Version: %d.%02d", dev_desc->bcdDevice >> 8, dev_desc->bcdDevice & 0xFF);
    ESP_LOGI(TAG, "  Manufacturer String Index: %d", dev_desc->iManufacturer);
    ESP_LOGI(TAG, "  Product String Index: %d", dev_desc->iProduct);
    ESP_LOGI(TAG, "  Serial Number String Index: %d", dev_desc->iSerialNumber);
    ESP_LOGI(TAG, "  Num Configurations: %d", dev_desc->bNumConfigurations);

    // Check for specific devices if needed
    if (dev_desc->idVendor == 0x0C40 && dev_desc->idProduct == 0x7A1C)
    {
        ESP_LOGI(TAG, "  G20s Pro detected!");
        // You could set a specific configuration for this device if needed
        // *bConfigurationValue = ...;
    }

    // Use the first configuration by default
    *bConfigurationValue = 1;

    // Return true to allow enumeration for all devices
    return true;
}

void UsbHostHid::hidEventProcessorTaskTrampoline(void* arg)
{
    static_cast<UsbHostHid*>(arg)->hidEventProcessorTask();
}

void UsbHostHid::hidEventProcessorTask()
{
    UsbHidEvent event;
    BaseType_t xResult;
    constexpr TickType_t xTicksToWait = portMAX_DELAY;

    while (1)
    {
        // Wait indefinitely for an event to be posted to the queue.
        xResult = xQueueReceive(eventQueue, &event, xTicksToWait);

        if (xResult == pdPASS)
        {
            // Event received, process it.
            auto [deviceHandle, eventType, eventData] = event;
            handleHidHostEvent(deviceHandle, eventType, eventData);
        }
    }
}

void UsbHostHid::hidHostDeviceCallback(hid_host_device_handle_t hid_device_handle,
                                       const hid_host_driver_event_t event,
                                       void* arg)
{
    hid_host_dev_params_t dev_params;
    ESP_ERROR_CHECK(hid_host_device_get_params(hid_device_handle, &dev_params));

    hid_host_dev_info_t dev_info;
    ESP_ERROR_CHECK(hid_host_get_device_info(hid_device_handle, &dev_info));

    ESP_LOGI(TAG, "Device Info - VID: 0x%04x, PID: 0x%04x, Manufacturer: %s, Product: %s, Serial: %s",
             dev_info.VID, dev_info.PID, (char*)dev_info.iManufacturer, (char*)dev_info.iProduct, (char*)dev_info.iSerialNumber);

    ESP_LOGI(TAG, "Device Params - Address: %d, Interface: %d, SubClass: %d, Protocol: %d",
             dev_params.addr, dev_params.iface_num, dev_params.sub_class, dev_params.proto);

    UsbHostHid* self = static_cast<UsbHostHid*>(arg);

    UsbHidEvent e(hid_device_handle, event, arg);
    self->addEventToQueue(e);
}

void UsbHostHid::hidHostInterfaceCallback(hid_host_device_handle_t hid_device_handle,
                                          const hid_host_interface_event_t event,
                                          void* arg)
{
    uint8_t data[64]   = {0};
    size_t data_length = 0;
    hid_host_dev_params_t dev_params;
    ESP_ERROR_CHECK(hid_host_device_get_params(hid_device_handle, &dev_params));

    hid_host_dev_info_t dev_info;
    ESP_ERROR_CHECK(hid_host_get_device_info(hid_device_handle, &dev_info));

    UsbHostHid& self = *static_cast<UsbHostHid*>(arg);

    switch (event)
    {
    case HID_HOST_INTERFACE_EVENT_INPUT_REPORT:
        ESP_ERROR_CHECK(hid_host_device_get_raw_input_report_data(hid_device_handle,
                                                                  data,
                                                                  sizeof(data),
                                                                  &data_length));

        if (dev_info.VID == 0x0C40 && dev_info.PID == 0x7A1C)  // G20s Pro
        {
            self.g20sProReport.processReportData(data, data_length);
        }
        else if (HID_SUBCLASS_BOOT_INTERFACE == dev_params.sub_class)
        {
            if (HID_PROTOCOL_KEYBOARD == dev_params.proto)
            {
                self.keyboardReport.processReportData(data, data_length);
            }
            else if (HID_PROTOCOL_MOUSE == dev_params.proto)
            {
                self.mouseReport.processReportData(data, data_length);
            }
            else
            {
                // Handle other boot interface devices if needed
                ESP_LOGW(TAG, "Unhandled boot interface device");
            }
        }
        else
        {
            // Handle other generic devices if needed
            ESP_LOGW(TAG, "Unhandled generic HID device");
        }
        break;

    case HID_HOST_INTERFACE_EVENT_DISCONNECTED:
        ESP_LOGW(TAG, "HID Device, protocol '%s' DISCONNECTED",
                 HID_PROTO_NAMES[dev_params.proto].c_str());
        ESP_ERROR_CHECK(hid_host_device_close(hid_device_handle));
        break;

    case HID_HOST_INTERFACE_EVENT_TRANSFER_ERROR:
        ESP_LOGW(TAG, "HID Device, protocol '%s' TRANSFER_ERROR",
                 HID_PROTO_NAMES[dev_params.proto].c_str());
        break;

    default:
        ESP_LOGE(TAG, "HID Device, protocol '%s' Unhandled event",
                 HID_PROTO_NAMES[dev_params.proto].c_str());
        break;
    }
}

/**
 * @brief USB HID Host Device event
 *
 * @param[in] hid_device_handle  HID Device handle
 * @param[in] event              HID Host Device event
 * @param[in] arg                Pointer to arguments, does not used
 */
void UsbHostHid::handleHidHostEvent(hid_host_device_handle_t hid_device_handle,
                                    const hid_host_driver_event_t event,
                                    void* arg)
{
    hid_host_dev_params_t dev_params;
    esp_err_t err = hid_host_device_get_params(hid_device_handle, &dev_params);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to get device parameters: %s", esp_err_to_name(err));
        return;
    }

    switch (event)
    {
    case HID_HOST_DRIVER_EVENT_CONNECTED:
    {
        ESP_LOGI(TAG, "HID Device, protocol '%s' CONNECTED",
                 HID_PROTO_NAMES[dev_params.proto].c_str());

        // Handle connected device
        const hid_host_device_config_t dev_config = {
            .callback     = hidHostInterfaceCallback,
            .callback_arg = arg};

        err = hid_host_device_open(hid_device_handle, &dev_config);
        if (err == ESP_OK)
        {
            // Device opened successfully
            if (HID_SUBCLASS_BOOT_INTERFACE == dev_params.sub_class)
            {
                err = hid_class_request_set_protocol(hid_device_handle, HID_REPORT_PROTOCOL_BOOT);
                if (err != ESP_OK)
                {
                    ESP_LOGE(TAG, "Failed to set boot protocol: %s", esp_err_to_name(err));
                    break;
                }

                if (HID_PROTOCOL_KEYBOARD == dev_params.proto)
                {
                    err = hid_class_request_set_idle(hid_device_handle, 0, 0);
                    if (err != ESP_OK)
                    {
                        ESP_LOGE(TAG, "Failed to set idle: %s", esp_err_to_name(err));
                        break;
                    }
                }
            }

            err = hid_host_device_start(hid_device_handle);
            if (err != ESP_OK)
            {
                ESP_LOGE(TAG, "Failed to start HID device: %s", esp_err_to_name(err));
            }
            else
            {
                ESP_LOGI(TAG, "HID device started successfully");
            }
        }
        else
        {
            ESP_LOGE(TAG, "Failed to open HID device: %s", esp_err_to_name(err));
        }
        break;
    }

    default:
        ESP_LOGW(TAG, "HID Device, protocol '%s' Unhandled event: %d",
                 HID_PROTO_NAMES[dev_params.proto].c_str(), event);
        break;
    }
}

void UsbHostHid::addEventToQueue(const UsbHidEvent& event)
{
    if (xQueueSend(eventQueue, &event, 0) != pdTRUE)
    {
        ESP_LOGW(TAG, "Failed to add event to queue");
    }
    else
    {
        ESP_LOGI(TAG, "Event added to queue");
    }
}
