idf_component_register(
    SRCS 
        "src/UsbHidHost.cpp"
        "src/reports/UsbHidG20sProReport.cpp"
        "src/reports/UsbHidGenericReport.cpp"
        "src/reports/UsbHidKeyboardReport.cpp"
        "src/reports/UsbHidMouseReport.cpp"
        "src/usb/hid_host.c"
    INCLUDE_DIRS 
        "include"
        "src/reports"
        "src/usb"
    REQUIRES 
        freertos
        esp_common
        # esp_log
        # usb
)
