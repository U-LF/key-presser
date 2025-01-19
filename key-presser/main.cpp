#include "Libraries.h"
#include "keySimulatorFunctions.h"
#include "MacroManager.h"

int main() {

    //SetConsoleOutputCP(CP_UTF8);

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

    // Customizing ImGui style for a modern look
    ImGuiStyle& style = ImGui::GetStyle();
    style.FrameRounding = 5.0f;
    style.GrabRounding = 5.0f;
    style.WindowRounding = 10.0f;
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.15f, 1.0f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.2f, 0.3f, 0.5f, 1.0f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.3f, 0.4f, 0.7f, 1.0f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.1f, 0.2f, 0.4f, 1.0f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.2f, 0.2f, 0.3f, 1.0f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.3f, 0.3f, 0.5f, 1.0f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.2f, 0.3f, 0.5f, 1.0f);
    style.Colors[ImGuiCol_Header] = ImVec4(0.3f, 0.4f, 0.6f, 1.0f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.4f, 0.5f, 0.7f, 1.0f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.2f, 0.3f, 0.5f, 1.0f);

    // State variables for the GUI
    static int selectedKeyIndex = 0;
    static const char* keyOptions[] = {
        "A", "B", "C", "D", "E", "F", "G", "H", "I",
        "J", "K", "L", "M", "N", "O", "P", "Q", "R",
        "S", "T", "U", "V", "W", "X", "Y", "Z",
        "Space", "Enter", "Tab", "Backspace", "Esc",
        "Left Mouse Button", "Right Mouse Button",
        "Num 0", "Num 1", "Num 2", "Num 3", "Num 4",
        "Num 5", "Num 6", "Num 7", "Num 8", "Num 9"
    };
    static WORD virtualKeys[] = {
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I',
        'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R',
        'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
        VK_SPACE, VK_RETURN, VK_TAB, VK_BACK, VK_ESCAPE,
        VK_LBUTTON, VK_RBUTTON, VK_NUMPAD0, VK_NUMPAD1,
        VK_NUMPAD2, VK_NUMPAD3, VK_NUMPAD4, VK_NUMPAD5,
        VK_NUMPAD6, VK_NUMPAD7, VK_NUMPAD8, VK_NUMPAD9
    };

    static std::vector<WORD> keysToSimulate;
    static int delay = 1;
    static bool running = false;
    static bool randomizerEnabled = false;
    static int sizeIndex = 1;
    static const char* sizeOptions[] = { "S", "M", "L", "XL" };
    static ImVec2 sizes[] = { ImVec2(400, 400), ImVec2(500, 415), ImVec2(600, 430), ImVec2(700, 445) };
    static std::vector<Macro> loadedMacros;
    static int selectedMacroIndex = -1;

    std::thread keyThread;

    std::mutex stopMutex;
    std::condition_variable stopCondition;

    auto simulateKeys = [&]() {
        std::random_device rd;
        std::mt19937 gen(rd());

        while (running) {
            PressMultipleKeys(keysToSimulate);

            int currentDelay = delay * 1000;
            if (randomizerEnabled) {
                std::uniform_int_distribution<int> dist(10, 100);
                int randomPercentage = dist(gen);
                currentDelay = delay * (randomPercentage / 100.0f) * 1000;
            }

            std::unique_lock<std::mutex> lock(stopMutex);
            if (stopCondition.wait_for(lock, std::chrono::milliseconds(currentDelay), [&]() { return !running; })) {
                break;
            }
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

        ImGui::Begin("Key Presser", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

        // Dropdown menu for key selection
        //ImGui::Text("\u2794 Select key to add:");
        ImGui::Text("*** Select key to add ***");
        ImGui::Combo("Key", &selectedKeyIndex, keyOptions, IM_ARRAYSIZE(keyOptions));

        // Add key button
        if (ImGui::Button("Add Key")) {
            WORD key = virtualKeys[selectedKeyIndex];
            keysToSimulate.push_back(key);
        }

        // Show the list of selected keys and add/remove option
        //ImGui::Text("\u2794 Keys to simulate:");
        ImGui::Text("-> Keys to simulate:");
        for (size_t i = 0; i < keysToSimulate.size(); ++i) {
            ImGui::Text("%c", keysToSimulate[i]);
            ImGui::SameLine();
            if (ImGui::Button(("Remove##" + std::to_string(i)).c_str())) {
                keysToSimulate.erase(keysToSimulate.begin() + i);
            }
        }

        // Dropdown menu for window size
        //ImGui::Text("\u2794 Select window size:");
        ImGui::Text("-> Select window size:");
        if (ImGui::Combo("Size", &sizeIndex, sizeOptions, IM_ARRAYSIZE(sizeOptions))) {
            ImGui::SetNextWindowSize(sizes[sizeIndex], ImGuiCond_Always);
        }

        // Input for delay
        ImGui::InputInt("Delay (seconds)", &delay);
        if (delay < 1) delay = 1;

        // Randomizer toggle
        //ImGui::Checkbox("\u2728 Enable Randomizer", &randomizerEnabled);
        ImGui::Checkbox("Enable Randomizer :)", &randomizerEnabled);

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
                {
                    std::lock_guard<std::mutex> lock(stopMutex);
                    running = false;
                }
                stopCondition.notify_all(); // Notify the thread to stop
                if (keyThread.joinable()) keyThread.join();
            }
        }

        ImGui::End();

        // Render ImGui
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    running = false;
    if (keyThread.joinable()) {
        {
            std::lock_guard<std::mutex> lock(stopMutex);
            running = false;
        }
        stopCondition.notify_all();
        keyThread.join();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}