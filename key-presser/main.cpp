#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <windows.h>
#include <iostream>
#include <thread>
#include <vector>
#include <fstream>
#include <sstream>
#include "keySimulatorFunctions.h"
#include "MacroManager.h"

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Create OpenGL context
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Key Simulator", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // State variables for the GUI
    static int selectedKeyIndex = 0;
    static const char* keyOptions[] = {
        "A", "B", "C", "D", "E", "F", "G", "H", "I",
        "J", "K", "L", "M", "N", "O", "P", "Q", "R",
        "S", "T", "U", "V", "W", "X", "Y", "Z",
        "Space", "Enter", "Tab", "Backspace", "Esc"
    };
    static WORD virtualKeys[] = {
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I',
        'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R',
        'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
        VK_SPACE, VK_RETURN, VK_TAB, VK_BACK, VK_ESCAPE
    };

    static std::vector<WORD> keysToSimulate;
    static int delay = 1;
    static bool running = false;
    static int sizeIndex = 1;
    static const char* sizeOptions[] = { "Small", "Medium", "Large", "Excel" };
    static ImVec2 sizes[] = { ImVec2(400, 400), ImVec2(500, 415), ImVec2(600, 430), ImVec2(700, 445) };
    static std::vector<Macro> loadedMacros;
    static int selectedMacroIndex = -1;

    std::thread keyThread;

    auto simulateKeys = [&]() {
        while (running) {
            PressMultipleKeys(keysToSimulate);
            Sleep(delay * 1000);
        }
        };

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Start the ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Adjust the scaling based on the window size
        ImVec2 windowSize = ImGui::GetIO().DisplaySize;
        float scaleFactor = windowSize.x / 800.0f;
        ImGui::GetIO().FontGlobalScale = scaleFactor;

        // Center the window's content
        ImVec2 windowCenter = ImVec2(windowSize.x * 0.5f, windowSize.y * 0.5f);
        ImVec2 windowContentSize = sizes[sizeIndex];
        ImVec2 contentPos = ImVec2(windowCenter.x - windowContentSize.x * 0.5f, windowCenter.y - windowContentSize.y * 0.5f);

        // Build the GUI with centered content
        ImGui::SetNextWindowPos(contentPos, ImGuiCond_Always);
        ImGui::SetNextWindowSize(sizes[sizeIndex], ImGuiCond_Always);

        ImGui::Begin("Key Simulator", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

        // Title
        ImGui::Text("Simulate key presses with a delay.");

        // Dropdown menu for key selection
        ImGui::Text("Select key to add:");
        ImGui::Combo("Key", &selectedKeyIndex, keyOptions, IM_ARRAYSIZE(keyOptions));

        // Add key button
        if (ImGui::Button("Add Key")) {
            WORD key = virtualKeys[selectedKeyIndex];
            keysToSimulate.push_back(key);
        }

        // Show the list of selected keys and add/remove option
        ImGui::Text("Keys to simulate:");
        for (size_t i = 0; i < keysToSimulate.size(); ++i) {
            ImGui::Text("%c", keysToSimulate[i]);
            ImGui::SameLine();
            if (ImGui::Button(("Remove##" + std::to_string(i)).c_str())) {
                keysToSimulate.erase(keysToSimulate.begin() + i);
            }
        }

        // Dropdown menu for window size
        ImGui::Text("Select window size:");
        if (ImGui::Combo("Size", &sizeIndex, sizeOptions, IM_ARRAYSIZE(sizeOptions))) {
            ImGui::SetNextWindowSize(sizes[sizeIndex], ImGuiCond_Always);
        }

        // Input for delay
        ImGui::InputInt("Delay (seconds)", &delay);
        if (delay < 1) delay = 1;

        // Input for macro name and saving
        static char macroName[128] = "";
        ImGui::InputText("Macro Name", macroName, IM_ARRAYSIZE(macroName));
        if (ImGui::Button("Save Macro")) {
            Macro newMacro;
            newMacro.name = macroName;
            newMacro.keys = keysToSimulate;
            newMacro.delay = delay;
            SaveMacro(newMacro);
            loadedMacros = LoadMacros(); // Reload the saved macros
            memset(macroName, 0, sizeof(macroName)); // Clear input field
        }

        // Dropdown menu to load macros
        ImGui::Text("Select Macro to Load:");
        loadedMacros = LoadMacros();
        std::vector<const char*> macroNames;
        for (const auto& macro : loadedMacros) {
            macroNames.push_back(macro.name.c_str());
        }

        if (ImGui::Combo("Load Macro", &selectedMacroIndex, macroNames.data(), static_cast<int>(macroNames.size()))) {
            if (selectedMacroIndex != -1) {
                keysToSimulate = loadedMacros[selectedMacroIndex].keys;
                delay = loadedMacros[selectedMacroIndex].delay;
            }
        }

        // Start/Stop button
        if (!running) {
            if (ImGui::Button("Start Macro")) {
                running = true;
                keyThread = std::thread(simulateKeys);
            }
        }
        else {
            if (ImGui::Button("Stop Macro")) {
                running = false;
                if (keyThread.joinable()) keyThread.join();
            }
        }

        ImGui::End();

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    running = false;
    if (keyThread.joinable()) keyThread.join();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
