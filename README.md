# USB HID Host

## Overview

USB HID Host is a C++ library for ESP32 devices with USB capabilities, allowing them to act as HID (Human Interface Device) hosts. This library enables the use of various USB devices such as mice, keyboards, game controllers, and specifically supports the G20s Pro USB air remote.

## Features

- Support for multiple HID devices:
  - G20s Pro USB air remote
  - Standard keyboards
  - Standard mice
  - Generic HID devices
- Event-based architecture using FreeRTOS queues
- Easy integration with ESP32 projects
- Built on top of ESP-IDF's USB Host capabilities

## Key Components

- `UsbHostHid`: Main class for managing USB HID devices
- Device-specific report classes:
  - `UsbHidG20sProReport`
  - `UsbHidKeyboardReport`
  - `UsbHidMouseReport`
  - `UsbHidGenericReport`

## Usage

1. Initialize the USB HID Host:
   ```cpp
   UsbHostHid usbHost;
   usbHost.init();
   usbHost.start();
   ```

2. Access device-specific reports:
   ```cpp
   auto* keyboardReport = usbHost.reportKeyboard();
   auto* mouseReport = usbHost.reportMouse();
   auto* g20sProReport = usbHost.reportG20sPro();
   auto* genericReport = usbHost.reportGeneric();
   ```

3. (TODO) Register callbacks for HID events:
   ```cpp
   usbHost.registerHIDCallback([](const UsbHostHid::UsbHidEvent& event) {
       // Handle HID event
   });
   ```

## Dependencies

- ESP-IDF (version compatible with USB Host API)
- FreeRTOS

## Acknowledgments

This project builds upon the USB Host capabilities provided by Espressif's ESP-IDF. We've drawn inspiration and insights from the official ESP-IDF HID host example:

[esp-idf/examples/peripherals/usb/host/hid/main/hid_host_example.c](https://github.com/espressif/esp-idf/blob/master/examples/peripherals/usb/host/hid/main/hid_host_example.c)

## Contributing

Contributions to improve USB HID Host are welcome. Please feel free to submit issues and pull requests.


## License

MIT License

Copyright (c) 2024 Michael Weaver

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

## Contact

Michael Weaver (michael.robert.weaver@gmail.com)