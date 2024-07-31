/**
 * @file UsbHidKeyboardReport.h
 * @brief Defines the UsbHidKeyboardReport class for handling USB HID keyboard reports.
 */

#pragma once

#include "UsbHidBaseReport.h"
#include <cstdint>
#include <vector>
#include <string>
#include <unordered_map>

#include "esp_log.h"

/**
 * @struct UsbHidKeyboardEvent
 * @brief Represents a keyboard event with pressed keys and modifiers.
 */
struct UsbHidKeyboardEvent
{
    UsbHidDeviceType deviceType_;   ///< Type of the USB HID device
    uint8_t modifiers;              ///< Bitmask of active modifiers
    std::vector<uint8_t> keyCodes;  ///< List of pressed key codes

    UsbHidKeyboardEvent() : deviceType_(UsbHidDeviceType::Keyboard), modifiers(0), keyCodes() {}
};

/**
 * @class UsbHidKeyboardReport
 * @brief Handles processing and interpretation of USB HID keyboard reports.
 *
 * This class extends UsbHidBaseReport to provide specific functionality for keyboard devices.
 * It processes raw HID reports, tracks key states, and provides methods to query
 * the current keyboard state.
 */
class UsbHidKeyboardReport : public UsbHidBaseReport<UsbHidKeyboardEvent, UsbHidDeviceType::Keyboard>
{
public:
    /// Maximum number of keys that can be pressed simultaneously
    static constexpr int MAX_KEYS = 6;

    /**
     * @enum Modifier
     * @brief Enumeration of keyboard modifier keys.
     */
    enum class Modifier : uint8_t
    {
        LEFT_CTRL   = 1 << 0,
        LEFT_SHIFT  = 1 << 1,
        LEFT_ALT    = 1 << 2,
        LEFT_GUI    = 1 << 3,
        RIGHT_CTRL  = 1 << 4,
        RIGHT_SHIFT = 1 << 5,
        RIGHT_ALT   = 1 << 6,
        RIGHT_GUI   = 1 << 7
    };

    /**
     * @enum KeyCode
     * @brief Enumeration of keyboard key codes as per USB HID specification.
     */
    enum class KeyCode : uint8_t
    {
        KEY_NONE                 = 0x00,
        KEY_ERR_ROLLOVER         = 0x01,
        KEY_POST_FAIL            = 0x02,
        KEY_ERR_UNDEFINED        = 0x03,
        KEY_A                    = 0x04,
        KEY_B                    = 0x05,
        KEY_C                    = 0x06,
        KEY_D                    = 0x07,
        KEY_E                    = 0x08,
        KEY_F                    = 0x09,
        KEY_G                    = 0x0A,
        KEY_H                    = 0x0B,
        KEY_I                    = 0x0C,
        KEY_J                    = 0x0D,
        KEY_K                    = 0x0E,
        KEY_L                    = 0x0F,
        KEY_M                    = 0x10,
        KEY_N                    = 0x11,
        KEY_O                    = 0x12,
        KEY_P                    = 0x13,
        KEY_Q                    = 0x14,
        KEY_R                    = 0x15,
        KEY_S                    = 0x16,
        KEY_T                    = 0x17,
        KEY_U                    = 0x18,
        KEY_V                    = 0x19,
        KEY_W                    = 0x1A,
        KEY_X                    = 0x1B,
        KEY_Y                    = 0x1C,
        KEY_Z                    = 0x1D,
        KEY_1                    = 0x1E,
        KEY_2                    = 0x1F,
        KEY_3                    = 0x20,
        KEY_4                    = 0x21,
        KEY_5                    = 0x22,
        KEY_6                    = 0x23,
        KEY_7                    = 0x24,
        KEY_8                    = 0x25,
        KEY_9                    = 0x26,
        KEY_0                    = 0x27,
        KEY_ENTER                = 0x28,
        KEY_ESC                  = 0x29,
        KEY_BACKSPACE            = 0x2A,
        KEY_TAB                  = 0x2B,
        KEY_SPACE                = 0x2C,
        KEY_MINUS                = 0x2D,
        KEY_EQUAL                = 0x2E,
        KEY_BRACKET_LEFT         = 0x2F,
        KEY_BRACKET_RIGHT        = 0x30,
        KEY_BACKSLASH            = 0x31,
        KEY_HASHTAG              = 0x32,
        KEY_SEMICOLON            = 0x33,
        KEY_QUOTE                = 0x34,
        KEY_TILDE                = 0x35,
        KEY_COMMA                = 0x36,
        KEY_DOT                  = 0x37,
        KEY_SLASH                = 0x38,
        KEY_CAPS_LOCK            = 0x39,
        KEY_F1                   = 0x3A,
        KEY_F2                   = 0x3B,
        KEY_F3                   = 0x3C,
        KEY_F4                   = 0x3D,
        KEY_F5                   = 0x3E,
        KEY_F6                   = 0x3F,
        KEY_F7                   = 0x40,
        KEY_F8                   = 0x41,
        KEY_F9                   = 0x42,
        KEY_F10                  = 0x43,
        KEY_F11                  = 0x44,
        KEY_F12                  = 0x45,
        KEY_PRINTSCREEN          = 0x46,
        KEY_SCROLL_LOCK          = 0x47,
        KEY_PAUSE                = 0x48,
        KEY_INSERT               = 0x49,
        KEY_HOME                 = 0x4A,
        KEY_PAGE_UP              = 0x4B,
        KEY_DELETE               = 0x4C,
        KEY_END                  = 0x4D,
        KEY_PAGE_DOWN            = 0x4E,
        KEY_ARROW_RIGHT          = 0x4F,
        KEY_ARROW_LEFT           = 0x50,
        KEY_ARROW_DOWN           = 0x51,
        KEY_ARROW_UP             = 0x52,
        KEY_NUM_LOCK             = 0x53,
        KEYPAD_SLASH             = 0x54,
        KEYPAD_ASTERISK          = 0x55,
        KEYPAD_MINUS             = 0x56,
        KEYPAD_PLUS              = 0x57,
        KEYPAD_ENTER             = 0x58,
        KEYPAD_1                 = 0x59,
        KEYPAD_2                 = 0x5A,
        KEYPAD_3                 = 0x5B,
        KEYPAD_4                 = 0x5C,
        KEYPAD_5                 = 0x5D,
        KEYPAD_6                 = 0x5E,
        KEYPAD_7                 = 0x5F,
        KEYPAD_8                 = 0x60,
        KEYPAD_9                 = 0x61,
        KEYPAD_0                 = 0x62,
        KEYPAD_DOT               = 0x63,
        KEY_NON_US_HASH          = 0x64,
        KEY_APPLICATION          = 0x65,
        KEY_POWER                = 0x66,
        KEYPAD_EQUAL             = 0x67,
        KEY_F13                  = 0x68,
        KEY_F14                  = 0x69,
        KEY_F15                  = 0x6A,
        KEY_F16                  = 0x6B,
        KEY_F17                  = 0x6C,
        KEY_F18                  = 0x6D,
        KEY_F19                  = 0x6E,
        KEY_F20                  = 0x6F,
        KEY_F21                  = 0x70,
        KEY_F22                  = 0x71,
        KEY_F23                  = 0x72,
        KEY_F24                  = 0x73,
        KEY_EXECUTE              = 0x74,
        KEY_HELP                 = 0x75,
        KEY_MENU                 = 0x76,
        KEY_SELECT               = 0x77,
        KEY_STOP                 = 0x78,
        KEY_AGAIN                = 0x79,
        KEY_UNDO                 = 0x7A,
        KEY_CUT                  = 0x7B,
        KEY_COPY                 = 0x7C,
        KEY_PASTE                = 0x7D,
        KEY_FIND                 = 0x7E,
        KEY_MUTE                 = 0x7F,
        KEY_VOLUME_UP            = 0x80,
        KEY_VOLUME_DOWN          = 0x81,
        KEY_LOCKING_CAPS         = 0x82,
        KEY_LOCKING_NUM          = 0x83,
        KEY_LOCKING_SCROLL       = 0x84,
        KEYPAD_COMMA             = 0x85,
        KEYPAD_EQUAL_SIGN        = 0x86,
        KEY_INTERNATIONAL_1      = 0x87,
        KEY_INTERNATIONAL_2      = 0x88,
        KEY_INTERNATIONAL_3      = 0x89,
        KEY_INTERNATIONAL_4      = 0x8A,
        KEY_INTERNATIONAL_5      = 0x8B,
        KEY_INTERNATIONAL_6      = 0x8C,
        KEY_INTERNATIONAL_7      = 0x8D,
        KEY_INTERNATIONAL_8      = 0x8E,
        KEY_INTERNATIONAL_9      = 0x8F,
        KEY_LANG_1               = 0x90,
        KEY_LANG_2               = 0x91,
        KEY_LANG_3               = 0x92,
        KEY_LANG_4               = 0x93,
        KEY_LANG_5               = 0x94,
        KEY_LANG_6               = 0x95,
        KEY_LANG_7               = 0x96,
        KEY_LANG_8               = 0x97,
        KEY_LANG_9               = 0x98,
        KEY_ALTERNATE_ERASE      = 0x99,
        KEY_SYSREQ               = 0x9A,
        KEY_CANCEL               = 0x9B,
        KEY_CLEAR                = 0x9C,
        KEY_PRIOR                = 0x9D,
        KEY_RETURN               = 0x9E,
        KEY_SEPARATOR            = 0x9F,
        KEY_OUT                  = 0xA0,
        KEY_OPER                 = 0xA1,
        KEY_CLEAR_AGAIN          = 0xA2,
        KEY_CRSEL                = 0xA3,
        KEY_EXSEL                = 0xA4,  // A5 - AF Reserved
        KEYPAD_00                = 0xB0,
        KEYPAD_000               = 0xB1,
        THOUSANDS_SEPARATOR      = 0xB2,
        DECIMAL_SEPARATOR        = 0xB3,
        CURRENCY_UNIT            = 0xB4,
        CURRENCY_SUBUNIT         = 0xB5,
        KEYPAD_PARENTHESIS_LEFT  = 0xB6,
        KEYPAD_PARENTHESIS_RIGHT = 0xB7,
        KEYPAD_BRACE_LEFT        = 0xB8,
        KEYPAD_BRACE_RIGHT       = 0xB9,
        KEYPAD_TAB               = 0xBA,
        KEYPAD_BACKSPACE         = 0xBB,
        KEYPAD_A                 = 0xBC,
        KEYPAD_B                 = 0xBD,
        KEYPAD_C                 = 0xBE,
        KEYPAD_D                 = 0xBF,
        KEYPAD_E                 = 0xC0,
        KEYPAD_F                 = 0xC1,
        KEYPAD_XOR               = 0xC2,
        KEYPAD_CARET             = 0xC3,
        KEYPAD_PERCENT           = 0xC4,
        KEYPAD_LESS_THAN         = 0xC5,
        KEYPAD_GREATER_THAN      = 0xC6,
        KEYPAD_AMPERSAND         = 0xC7,
        KEYPAD_DOUBLE_AMPERSAND  = 0xC8,
        KEYPAD_PIPE              = 0xC9,
        KEYPAD_DOUBLE_PIPE       = 0xCA,
        KEYPAD_COLON             = 0xCB,
        KEYPAD_HASH              = 0xCC,
        KEYPAD_SPACE             = 0xCD,
        KEYPAD_AT                = 0xCE,
        KEYPAD_EXCLAMATION       = 0xCF,
        KEYPAD_MEMORY_STORE      = 0xD0,
        KEYPAD_MEMORY_RECALL     = 0xD1,
        KEYPAD_MEMORY_CLEAR      = 0xD2,
        KEYPAD_MEMORY_ADD        = 0xD3,
        KEYPAD_MEMORY_SUBTRACT   = 0xD4,
        KEYPAD_MEMORY_MULTIPLY   = 0xD5,
        KEYPAD_MEMORY_DIVIDE     = 0xD6,
        KEYPAD_PLUS_MINUS        = 0xD7,
        KEYPAD_CLEAR             = 0xD8,
        KEYPAD_CLEAR_ENTRY       = 0xD9,
        KEYPAD_BINARY            = 0xDA,
        KEYPAD_OCTAL             = 0xDB,
        KEYPAD_DECIMAL           = 0xDC,
        KEYPAD_HEXADECIMAL       = 0xDD,  // DE - DF Reserved
        KEY_LEFT_CTRL            = 0xE0,
        KEY_LEFT_SHIFT           = 0xE1,
        KEY_LEFT_ALT             = 0xE2,
        KEY_LEFT_GUI             = 0xE3,
        KEY_RIGHT_CTRL           = 0xE4,
        KEY_RIGHT_SHIFT          = 0xE5,
        KEY_RIGHT_ALT            = 0xE6,
        KEY_RIGHT_GUI            = 0xE7  // E8 - FFFF Reserved
    };

    /**
     * @brief Construct a new UsbHidKeyboardReport object.
     */
    UsbHidKeyboardReport();

    /**
     * @brief Process raw report data from the USB HID device.
     *
     * @param data Pointer to the raw report data.
     * @param length Length of the raw report data.
     */
    void processReportData(const uint8_t *const data, int length) override;

    /**
     * @brief Check if a specific modifier key is active.
     *
     * @param modifier The modifier to check.
     * @return true if the modifier is active, false otherwise.
     */
    bool isModifierActive(Modifier modifier) const;

    /**
     * @brief Get the current state of all modifier keys.
     *
     * @return uint8_t The modifier state as a bitmask.
     */
    uint8_t getModifiers() const;

    /**
     * @brief Get a list of currently pressed keys.
     *
     * @return std::vector<KeyCode> The list of pressed keys.
     */
    std::vector<KeyCode> getPressedKeys() const;

    /**
     * @brief Get the name of a specific key code.
     *
     * @param keyCode The key code to get the name for.
     * @return std::string The name of the key.
     */
    static std::string getKeyName(KeyCode keyCode);

    /**
     * @brief Get the name of a specific key code.
     *
     * @param keyCode The key code to get the name for.
     * @return std::string The name of the key.
     */
    static std::string getKeyName(uint8_t keyCode) { return getKeyName(static_cast<KeyCode>(keyCode)); }

    /**
     * @brief Get a space-separated string of modifier names from a bitmask.
     *
     * @param modifiers The modifier bitmask.
     * @return std::string The names of the modifiers.
     */
    static std::string getModifierNames(uint8_t modifiers);

    /**
     * @brief Get a comma-separated string of currently pressed key names.
     *
     * @return std::string The names of pressed keys.
     */
    std::string getPressedKeyNames() const;

    /**
     * @brief Get a space-separated string of currently active modifier names.
     *
     * @return std::string The names of active modifiers.
     */
    std::string getActiveModifierNames() const;

protected:
    /**
     * @brief Create a UsbHidKeyboardEvent based on the current report state.
     *
     * @return UsbHidKeyboardEvent The created event.
     */
    UsbHidKeyboardEvent createEvent() const override;

private:
    /**
     * @struct KeyboardReportData
     * @brief Structure representing the raw HID report data for a keyboard.
     *
     * @details The order of keycodes in array fields has no significance.
     *  Order determination is done by the host software comparing the contents of the previous report to the current report.
     *  If two or more keys are reported in one report, their order is indeterminate.
     */
    struct KeyboardReportData
    {
        union
        {
            struct
            {
                uint8_t left_ctrl : 1;
                uint8_t left_shift : 1;
                uint8_t left_alt : 1;
                uint8_t left_gui : 1;
                uint8_t right_ctrl : 1;
                uint8_t right_shift : 1;
                uint8_t right_alt : 1;
                uint8_t right_gui : 1;
            };
            uint8_t val;
        } modifier;
        uint8_t reserved;
        uint8_t key[MAX_KEYS];
    } __attribute__((packed));

    KeyboardReportData report_;  ///< The current keyboard report data

    /// Map of key codes to their string representations
    static const std::unordered_map<KeyCode, std::string> keyNameMap;
};