#include "keySimulatorFunctions.h"
#include <Windows.h>

// Function to simulate a key press
void PressKey(WORD key) {
    INPUT input = { 0 };
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = key;

    // Simulate key press
    SendInput(1, &input, sizeof(INPUT));

    // Simulate key release
    input.ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(1, &input, sizeof(INPUT));
}

// Function to simulate multiple key presses at once
void PressMultipleKeys(const std::vector<WORD>& keys) {
    std::vector<INPUT> inputs(keys.size());
    for (size_t i = 0; i < keys.size(); ++i) {
        inputs[i].type = INPUT_KEYBOARD;
        inputs[i].ki.wVk = keys[i];
    }

    // Simulate key press for all keys
    SendInput(static_cast<UINT>(keys.size()), inputs.data(), sizeof(INPUT));

    // Simulate key release for all keys
    for (size_t i = 0; i < keys.size(); ++i) {
        inputs[i].ki.dwFlags = KEYEVENTF_KEYUP;
    }
    SendInput(static_cast<UINT>(keys.size()), inputs.data(), sizeof(INPUT));
}
