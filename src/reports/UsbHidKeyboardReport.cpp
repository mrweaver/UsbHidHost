/**
 * @file UsbHidKeyboardReport.cpp
 * @brief Implementation of the UsbHidKeyboardReport class for handling USB HID keyboard reports.
 */

#include "UsbHidKeyboardReport.h"
#include <cstring>
#include <sstream>

/**
 * @brief Construct a new UsbHidKeyboardReport object.
 *
 * Initializes the report as a USB HID Keyboard device.
 */
UsbHidKeyboardReport::UsbHidKeyboardReport()
{
    // Initialize the report_ data
    std::memset(&report_, 0, sizeof(KeyboardReportData));
    rawReport_.clear();
}

/**
 * @brief Process the raw report data received from the USB HID device.
 *
 * @param data Pointer to the raw report data.
 * @param length Length of the raw report data.
 */
void UsbHidKeyboardReport::processReportData(const uint8_t *const data, int length)
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
    ESP_LOGI("KeyboardReport", "Raw data: %s", raw.c_str());

    if (length < sizeof(KeyboardReportData))
    {
        ESP_LOGW("KeyboardReport", "Invalid report_ data length: %d", length);
        std::memset(&report_, 0, sizeof(KeyboardReportData));
        return;
    }

    KeyboardReportData newReport;
    std::memcpy(&newReport, data, sizeof(KeyboardReportData));

    if (memcmp(&newReport, &report_, sizeof(KeyboardReportData)) != 0)
    {
        report_ = newReport;

        triggerEvent(createEvent());
    }
}

/**
 * @brief Create a UsbHidKeyboardEvent based on the current report_ state.
 *
 * @return UsbHidKeyboardEvent The created event.
 */
UsbHidKeyboardEvent UsbHidKeyboardReport::createEvent() const
{
    UsbHidKeyboardEvent event;
    event.modifiers = report_.modifier.val;

    event.keyCodes.reserve(MAX_KEYS);
    for (int i = 0; i < MAX_KEYS; ++i)
    {
        if (report_.key[i] != static_cast<uint8_t>(KeyCode::KEY_NONE))
        {
            event.keyCodes.push_back(report_.key[i]);
            ESP_LOGI("KeyboardReport", "Size %d | Key: 0x%02X", event.keyCodes.size(), event.keyCodes.back());
            // event.keyCodes[i] = report_.key[i];
        }
    }
    return event;
}

/**
 * @brief Check if a specific modifier key is active.
 *
 * @param modifier The modifier to check.
 * @return true if the modifier is active, false otherwise.
 */
bool UsbHidKeyboardReport::isModifierActive(Modifier modifier) const
{
    return (report_.modifier.val & static_cast<uint8_t>(modifier)) != 0;
}

/**
 * @brief Get the current state of all modifier keys.
 *
 * @return uint8_t The modifier state as a bitmask.
 */
uint8_t UsbHidKeyboardReport::getModifiers() const
{
    return report_.modifier.val;
}

/**
 * @brief Get a list of currently pressed keys.
 *
 * @return std::vector<UsbHidKeyboardReport::KeyCode> The list of pressed keys.
 */
std::vector<UsbHidKeyboardReport::KeyCode> UsbHidKeyboardReport::getPressedKeys() const
{
    std::vector<KeyCode> pressedKeys;
    for (int i = 0; i < MAX_KEYS; ++i)
    {
        if (report_.key[i] != static_cast<uint8_t>(KeyCode::KEY_NONE))
        {
            pressedKeys.push_back(static_cast<KeyCode>(report_.key[i]));
        }
    }
    return pressedKeys;
}

/**
 * @brief Get the name of a specific key code.
 *
 * @param keyCode The key code to get the name for.
 * @return std::string The name of the key.
 */
std::string UsbHidKeyboardReport::getKeyName(KeyCode keyCode)
{
    auto it = keyNameMap.find(keyCode);
    return it != keyNameMap.end() ? it->second : "Unknown";
}

/**
 * @brief Get a comma-separated string of currently pressed key names.
 *
 * @return std::string The names of pressed keys.
 */
std::string UsbHidKeyboardReport::getPressedKeyNames() const
{
    std::stringstream ss;
    auto pressedKeys = getPressedKeys();
    for (const auto &key : pressedKeys)
    {
        if (!ss.str().empty()) ss << ", ";
        ss << getKeyName(key);
    }
    return ss.str().empty() ? "NONE" : ss.str();
}

/**
 * @brief Get a space-separated string of currently active modifier names.
 *
 * @return std::string The names of active modifiers.
 */
std::string UsbHidKeyboardReport::getActiveModifierNames() const
{
    return getModifierNames(report_.modifier.val);
}

/**
 * @brief Get a space-separated string of modifier names from a bitmask.
 *
 * @param modifiers The modifier bitmask.
 * @return std::string The names of the modifiers.
 */
std::string UsbHidKeyboardReport::getModifierNames(uint8_t modifiers)
{
    std::stringstream ss;
    if (modifiers & static_cast<uint8_t>(Modifier::LEFT_CTRL)) ss << "LEFT_CTRL ";
    if (modifiers & static_cast<uint8_t>(Modifier::LEFT_SHIFT)) ss << "LEFT_SHIFT ";
    if (modifiers & static_cast<uint8_t>(Modifier::LEFT_ALT)) ss << "LEFT_ALT ";
    if (modifiers & static_cast<uint8_t>(Modifier::LEFT_GUI)) ss << "LEFT_GUI ";
    if (modifiers & static_cast<uint8_t>(Modifier::RIGHT_CTRL)) ss << "RIGHT_CTRL ";
    if (modifiers & static_cast<uint8_t>(Modifier::RIGHT_SHIFT)) ss << "RIGHT_SHIFT ";
    if (modifiers & static_cast<uint8_t>(Modifier::RIGHT_ALT)) ss << "RIGHT_ALT ";
    if (modifiers & static_cast<uint8_t>(Modifier::RIGHT_GUI)) ss << "RIGHT_GUI ";

    std::string result = ss.str();
    return result.empty() ? "NONE" : result;
}

// Key name map definition
const std::unordered_map<UsbHidKeyboardReport::KeyCode, std::string> UsbHidKeyboardReport::keyNameMap = {
    {KeyCode::KEY_NONE, "NONE"},
    {KeyCode::KEY_ERR_ROLLOVER, "ERR_ROLLOVER"},
    {KeyCode::KEY_POST_FAIL, "POST_FAIL"},
    {KeyCode::KEY_ERR_UNDEFINED, "ERR_UNDEFINED"},
    {KeyCode::KEY_A, "A"},
    {KeyCode::KEY_B, "B"},
    {KeyCode::KEY_C, "C"},
    {KeyCode::KEY_D, "D"},
    {KeyCode::KEY_E, "E"},
    {KeyCode::KEY_F, "F"},
    {KeyCode::KEY_G, "G"},
    {KeyCode::KEY_H, "H"},
    {KeyCode::KEY_I, "I"},
    {KeyCode::KEY_J, "J"},
    {KeyCode::KEY_K, "K"},
    {KeyCode::KEY_L, "L"},
    {KeyCode::KEY_M, "M"},
    {KeyCode::KEY_N, "N"},
    {KeyCode::KEY_O, "O"},
    {KeyCode::KEY_P, "P"},
    {KeyCode::KEY_Q, "Q"},
    {KeyCode::KEY_R, "R"},
    {KeyCode::KEY_S, "S"},
    {KeyCode::KEY_T, "T"},
    {KeyCode::KEY_U, "U"},
    {KeyCode::KEY_V, "V"},
    {KeyCode::KEY_W, "W"},
    {KeyCode::KEY_X, "X"},
    {KeyCode::KEY_Y, "Y"},
    {KeyCode::KEY_Z, "Z"},
    {KeyCode::KEY_1, "1"},
    {KeyCode::KEY_2, "2"},
    {KeyCode::KEY_3, "3"},
    {KeyCode::KEY_4, "4"},
    {KeyCode::KEY_5, "5"},
    {KeyCode::KEY_6, "6"},
    {KeyCode::KEY_7, "7"},
    {KeyCode::KEY_8, "8"},
    {KeyCode::KEY_9, "9"},
    {KeyCode::KEY_0, "0"},
    {KeyCode::KEY_ENTER, "ENTER"},
    {KeyCode::KEY_ESC, "ESC"},
    {KeyCode::KEY_BACKSPACE, "BACKSPACE"},
    {KeyCode::KEY_TAB, "TAB"},
    {KeyCode::KEY_SPACE, "SPACE"},
    {KeyCode::KEY_MINUS, "MINUS"},
    {KeyCode::KEY_EQUAL, "EQUAL"},
    {KeyCode::KEY_BRACKET_LEFT, "BRACKET_LEFT"},
    {KeyCode::KEY_BRACKET_RIGHT, "BRACKET_RIGHT"},
    {KeyCode::KEY_BACKSLASH, "BACKSLASH"},
    {KeyCode::KEY_HASHTAG, "HASHTAG"},
    {KeyCode::KEY_SEMICOLON, "SEMICOLON"},
    {KeyCode::KEY_QUOTE, "QUOTE"},
    {KeyCode::KEY_TILDE, "TILDE"},
    {KeyCode::KEY_COMMA, "COMMA"},
    {KeyCode::KEY_DOT, "DOT"},
    {KeyCode::KEY_SLASH, "SLASH"},
    {KeyCode::KEY_CAPS_LOCK, "CAPS_LOCK"},
    {KeyCode::KEY_F1, "F1"},
    {KeyCode::KEY_F2, "F2"},
    {KeyCode::KEY_F3, "F3"},
    {KeyCode::KEY_F4, "F4"},
    {KeyCode::KEY_F5, "F5"},
    {KeyCode::KEY_F6, "F6"},
    {KeyCode::KEY_F7, "F7"},
    {KeyCode::KEY_F8, "F8"},
    {KeyCode::KEY_F9, "F9"},
    {KeyCode::KEY_F10, "F10"},
    {KeyCode::KEY_F11, "F11"},
    {KeyCode::KEY_F12, "F12"},
    {KeyCode::KEY_PRINTSCREEN, "PRINTSCREEN"},
    {KeyCode::KEY_SCROLL_LOCK, "SCROLL_LOCK"},
    {KeyCode::KEY_PAUSE, "PAUSE"},
    {KeyCode::KEY_INSERT, "INSERT"},
    {KeyCode::KEY_HOME, "HOME"},
    {KeyCode::KEY_PAGE_UP, "PAGE_UP"},
    {KeyCode::KEY_DELETE, "DELETE"},
    {KeyCode::KEY_END, "END"},
    {KeyCode::KEY_PAGE_DOWN, "PAGE_DOWN"},
    {KeyCode::KEY_ARROW_RIGHT, "ARROW_RIGHT"},
    {KeyCode::KEY_ARROW_LEFT, "ARROW_LEFT"},
    {KeyCode::KEY_ARROW_DOWN, "ARROW_DOWN"},
    {KeyCode::KEY_ARROW_UP, "ARROW_UP"},
    {KeyCode::KEY_NUM_LOCK, "NUM_LOCK"},
    {KeyCode::KEYPAD_SLASH, "KEYPAD_SLASH"},
    {KeyCode::KEYPAD_ASTERISK, "KEYPAD_ASTERISK"},
    {KeyCode::KEYPAD_MINUS, "KEYPAD_MINUS"},
    {KeyCode::KEYPAD_PLUS, "KEYPAD_PLUS"},
    {KeyCode::KEYPAD_ENTER, "KEYPAD_ENTER"},
    {KeyCode::KEYPAD_1, "KEYPAD_1"},
    {KeyCode::KEYPAD_2, "KEYPAD_2"},
    {KeyCode::KEYPAD_3, "KEYPAD_3"},
    {KeyCode::KEYPAD_4, "KEYPAD_4"},
    {KeyCode::KEYPAD_5, "KEYPAD_5"},
    {KeyCode::KEYPAD_6, "KEYPAD_6"},
    {KeyCode::KEYPAD_7, "KEYPAD_7"},
    {KeyCode::KEYPAD_8, "KEYPAD_8"},
    {KeyCode::KEYPAD_9, "KEYPAD_9"},
    {KeyCode::KEYPAD_0, "KEYPAD_0"},
    {KeyCode::KEYPAD_DOT, "KEYPAD_DOT"},
    {KeyCode::KEY_NON_US_HASH, "NON_US_HASH"},
    {KeyCode::KEY_APPLICATION, "APPLICATION"},
    {KeyCode::KEY_POWER, "POWER"},
    {KeyCode::KEYPAD_EQUAL, "KEYPAD_EQUAL"},
    {KeyCode::KEY_F13, "F13"},
    {KeyCode::KEY_F14, "F14"},
    {KeyCode::KEY_F15, "F15"},
    {KeyCode::KEY_F16, "F16"},
    {KeyCode::KEY_F17, "F17"},
    {KeyCode::KEY_F18, "F18"},
    {KeyCode::KEY_F19, "F19"},
    {KeyCode::KEY_F20, "F20"},
    {KeyCode::KEY_F21, "F21"},
    {KeyCode::KEY_F22, "F22"},
    {KeyCode::KEY_F23, "F23"},
    {KeyCode::KEY_F24, "F24"},
    {KeyCode::KEY_EXECUTE, "EXECUTE"},
    {KeyCode::KEY_HELP, "HELP"},
    {KeyCode::KEY_MENU, "MENU"},
    {KeyCode::KEY_SELECT, "SELECT"},
    {KeyCode::KEY_STOP, "STOP"},
    {KeyCode::KEY_AGAIN, "AGAIN"},
    {KeyCode::KEY_UNDO, "UNDO"},
    {KeyCode::KEY_CUT, "CUT"},
    {KeyCode::KEY_COPY, "COPY"},
    {KeyCode::KEY_PASTE, "PASTE"},
    {KeyCode::KEY_FIND, "FIND"},
    {KeyCode::KEY_MUTE, "MUTE"},
    {KeyCode::KEY_VOLUME_UP, "VOLUME_UP"},
    {KeyCode::KEY_VOLUME_DOWN, "VOLUME_DOWN"},
    {KeyCode::KEY_LOCKING_CAPS, "LOCKING_CAPS"},
    {KeyCode::KEY_LOCKING_NUM, "LOCKING_NUM"},
    {KeyCode::KEY_LOCKING_SCROLL, "LOCKING_SCROLL"},
    {KeyCode::KEYPAD_COMMA, "KEYPAD_COMMA"},
    {KeyCode::KEYPAD_EQUAL_SIGN, "KEYPAD_EQUAL_SIGN"},
    {KeyCode::KEY_INTERNATIONAL_1, "INTERNATIONAL_1"},
    {KeyCode::KEY_INTERNATIONAL_2, "INTERNATIONAL_2"},
    {KeyCode::KEY_INTERNATIONAL_3, "INTERNATIONAL_3"},
    {KeyCode::KEY_INTERNATIONAL_4, "INTERNATIONAL_4"},
    {KeyCode::KEY_INTERNATIONAL_5, "INTERNATIONAL_5"},
    {KeyCode::KEY_INTERNATIONAL_6, "INTERNATIONAL_6"},
    {KeyCode::KEY_INTERNATIONAL_7, "INTERNATIONAL_7"},
    {KeyCode::KEY_INTERNATIONAL_8, "INTERNATIONAL_8"},
    {KeyCode::KEY_INTERNATIONAL_9, "INTERNATIONAL_9"},
    {KeyCode::KEY_LANG_1, "LANG_1"},
    {KeyCode::KEY_LANG_2, "LANG_2"},
    {KeyCode::KEY_LANG_3, "LANG_3"},
    {KeyCode::KEY_LANG_4, "LANG_4"},
    {KeyCode::KEY_LANG_5, "LANG_5"},
    {KeyCode::KEY_LANG_6, "LANG_6"},
    {KeyCode::KEY_LANG_7, "LANG_7"},
    {KeyCode::KEY_LANG_8, "LANG_8"},
    {KeyCode::KEY_LANG_9, "LANG_9"},
    {KeyCode::KEY_ALTERNATE_ERASE, "ALTERNATE_ERASE"},
    {KeyCode::KEY_SYSREQ, "SYSREQ"},
    {KeyCode::KEY_CANCEL, "CANCEL"},
    {KeyCode::KEY_CLEAR, "CLEAR"},
    {KeyCode::KEY_PRIOR, "PRIOR"},
    {KeyCode::KEY_RETURN, "RETURN"},
    {KeyCode::KEY_SEPARATOR, "SEPARATOR"},
    {KeyCode::KEY_OUT, "OUT"},
    {KeyCode::KEY_OPER, "OPER"},
    {KeyCode::KEY_CLEAR_AGAIN, "CLEAR_AGAIN"},
    {KeyCode::KEY_CRSEL, "CRSEL"},
    {KeyCode::KEY_EXSEL, "EXSEL"},
    {KeyCode::KEYPAD_00, "KEYPAD_00"},
    {KeyCode::KEYPAD_000, "KEYPAD_000"},
    {KeyCode::THOUSANDS_SEPARATOR, "THOUSANDS_SEPARATOR"},
    {KeyCode::DECIMAL_SEPARATOR, "DECIMAL_SEPARATOR"},
    {KeyCode::CURRENCY_UNIT, "CURRENCY_UNIT"},
    {KeyCode::CURRENCY_SUBUNIT, "CURRENCY_SUBUNIT"},
    {KeyCode::KEYPAD_PARENTHESIS_LEFT, "KEYPAD_PARENTHESIS_LEFT"},
    {KeyCode::KEYPAD_PARENTHESIS_RIGHT, "KEYPAD_PARENTHESIS_RIGHT"},
    {KeyCode::KEYPAD_BRACE_LEFT, "KEYPAD_BRACE_LEFT"},
    {KeyCode::KEYPAD_BRACE_RIGHT, "KEYPAD_BRACE_RIGHT"},
    {KeyCode::KEYPAD_TAB, "KEYPAD_TAB"},
    {KeyCode::KEYPAD_BACKSPACE, "KEYPAD_BACKSPACE"},
    {KeyCode::KEYPAD_A, "KEYPAD_A"},
    {KeyCode::KEYPAD_B, "KEYPAD_B"},
    {KeyCode::KEYPAD_C, "KEYPAD_C"},
    {KeyCode::KEYPAD_D, "KEYPAD_D"},
    {KeyCode::KEYPAD_E, "KEYPAD_E"},
    {KeyCode::KEYPAD_F, "KEYPAD_F"},
    {KeyCode::KEYPAD_XOR, "KEYPAD_XOR"},
    {KeyCode::KEYPAD_CARET, "KEYPAD_CARET"},
    {KeyCode::KEYPAD_PERCENT, "KEYPAD_PERCENT"},
    {KeyCode::KEYPAD_LESS_THAN, "KEYPAD_LESS_THAN"},
    {KeyCode::KEYPAD_GREATER_THAN, "KEYPAD_GREATER_THAN"},
    {KeyCode::KEYPAD_AMPERSAND, "KEYPAD_AMPERSAND"},
    {KeyCode::KEYPAD_DOUBLE_AMPERSAND, "KEYPAD_DOUBLE_AMPERSAND"},
    {KeyCode::KEYPAD_PIPE, "KEYPAD_PIPE"},
    {KeyCode::KEYPAD_DOUBLE_PIPE, "KEYPAD_DOUBLE_PIPE"},
    {KeyCode::KEYPAD_COLON, "KEYPAD_COLON"},
    {KeyCode::KEYPAD_HASH, "KEYPAD_HASH"},
    {KeyCode::KEYPAD_SPACE, "KEYPAD_SPACE"},
    {KeyCode::KEYPAD_AT, "KEYPAD_AT"},
    {KeyCode::KEYPAD_EXCLAMATION, "KEYPAD_EXCLAMATION"},
    {KeyCode::KEYPAD_MEMORY_STORE, "KEYPAD_MEMORY_STORE"},
    {KeyCode::KEYPAD_MEMORY_RECALL, "KEYPAD_MEMORY_RECALL"},
    {KeyCode::KEYPAD_MEMORY_CLEAR, "KEYPAD_MEMORY_CLEAR"},
    {KeyCode::KEYPAD_MEMORY_ADD, "KEYPAD_MEMORY_ADD"},
    {KeyCode::KEYPAD_MEMORY_SUBTRACT, "KEYPAD_MEMORY_SUBTRACT"},
    {KeyCode::KEYPAD_MEMORY_MULTIPLY, "KEYPAD_MEMORY_MULTIPLY"},
    {KeyCode::KEYPAD_MEMORY_DIVIDE, "KEYPAD_MEMORY_DIVIDE"},
    {KeyCode::KEYPAD_PLUS_MINUS, "KEYPAD_PLUS_MINUS"},
    {KeyCode::KEYPAD_CLEAR, "KEYPAD_CLEAR"},
    {KeyCode::KEYPAD_CLEAR_ENTRY, "KEYPAD_CLEAR_ENTRY"},
    {KeyCode::KEYPAD_BINARY, "KEYPAD_BINARY"},
    {KeyCode::KEYPAD_OCTAL, "KEYPAD_OCTAL"},
    {KeyCode::KEYPAD_DECIMAL, "KEYPAD_DECIMAL"},
    {KeyCode::KEYPAD_HEXADECIMAL, "KEYPAD_HEXADECIMAL"},
    {KeyCode::KEY_LEFT_CTRL, "LEFT_CTRL"},
    {KeyCode::KEY_LEFT_SHIFT, "LEFT_SHIFT"},
    {KeyCode::KEY_LEFT_ALT, "LEFT_ALT"},
    {KeyCode::KEY_LEFT_GUI, "LEFT_GUI"},
    {KeyCode::KEY_RIGHT_CTRL, "RIGHT_CTRL"},
    {KeyCode::KEY_RIGHT_SHIFT, "RIGHT_SHIFT"},
    {KeyCode::KEY_RIGHT_ALT, "RIGHT_ALT"},
    {KeyCode::KEY_RIGHT_GUI, "RIGHT_GUI"}};