/*

    xpld project
    friol 2k20 (back to c++)

*/

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <iomanip>

#include "xpldMMU.h"
#include "koobra.h"
#include "koolibri.h"
#include "debugger.h"

// About Desktop OpenGL function loaders:
//  Modern desktop OpenGL doesn't have a standard portable header file to load OpenGL function pointers.
//  Helper libraries are often used for this purpose! Here we are supporting a few common ones (gl3w, glew, glad).
//  You may use another loader/header of your choice (glext, glLoadGen, etc.), or chose to manually implement your own.
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
#include <GL/gl3w.h>            // Initialize with gl3wInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
#include <GL/glew.h>            // Initialize with glewInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
#include <glad/glad.h>          // Initialize with gladLoadGL()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD2)
#include <glad/gl.h>            // Initialize with gladLoadGL(...) or gladLoaderLoadGL()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
#define GLFW_INCLUDE_NONE       // GLFW including OpenGL headers causes ambiguity or multiple definition errors.
#include <glbinding/Binding.h>  // Initialize with glbinding::Binding::initialize()
#include <glbinding/gl/gl.h>
using namespace gl;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
#define GLFW_INCLUDE_NONE       // GLFW including OpenGL headers causes ambiguity or multiple definition errors.
#include <glbinding/glbinding.h>// Initialize with glbinding::initialize()
#include <glbinding/gl/gl.h>
using namespace gl;
#else
#include IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#endif

// Include glfw3.h after our OpenGL definitions
#include <GLFW/glfw3.h>

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

void printRegistersInfo(xpldCPU* theCpu,xpldVideochip* theVdu)
{
    std::string tmpstr = "PC:";
    unsigned int pc = theCpu->getPC();
    std::stringstream stream;
    stream << "0x" << std::setfill('0') << std::setw(8) << std::hex << pc;
    tmpstr += stream.str();
    ImGui::Text(tmpstr.c_str());

    tmpstr = "SP:";
    unsigned int sp = theCpu->getSP();
    stream.str(""); stream.clear();
    stream << "0x" << std::setfill('0') << std::setw(8) << std::hex << sp;
    tmpstr += stream.str();
    ImGui::Text(tmpstr.c_str());

    tmpstr = "NVxxDIZC";
    ImGui::Text(tmpstr.c_str());
    ImGui::Text(theCpu->getFlagsRegister().c_str());

    if (ImGui::BeginTable("##table1", 4))
    {
        for (int row = 0; row < 4; row++)
        {
            ImGui::TableNextRow();
            for (int column = 0; column < 4; column++)
            {
                int reg = row + (column * 4);
                tmpstr = "R" + std::to_string(reg) + ":";
                unsigned int regval = theCpu->getRegister(reg);
                tmpstr += std::to_string(regval);
                ImGui::TableSetColumnIndex(column);
                ImGui::Text(tmpstr.c_str());
            }
        }
        ImGui::EndTable();
    }

    std::string vduControlReg(std::to_string(theVdu->getStatusRegister()));
    ImGui::Text(("0x2000000:"+vduControlReg).c_str());

    ImGui::Text(("Rasterline:" + std::to_string(theVdu->getRasterLine())).c_str());
}

void prepareGLTexture(xpldVideochip* vdu,GLuint& image_texture)
{
    // Create a OpenGL texture identifier
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

    // Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif

    int image_width = vdu->getCurModeBitmapWidth();
    int image_height = vdu->getCurModeBitmapHeight();
    unsigned char* image_data = vdu->getCurModeBitmap();

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
}

void renderToTexture(xpldVideochip* vdu, GLuint image_texture)
{
    glBindTexture(GL_TEXTURE_2D, image_texture);

    int image_width = vdu->getCurModeBitmapWidth();
    int image_height = vdu->getCurModeBitmapHeight();
    unsigned char* image_data = vdu->getCurModeBitmap();

    glTexSubImage2D(
        GL_TEXTURE_2D,
        0,
        0,
        0,
        image_width,
        image_height,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        image_data
    );

    ImGui::Image((void*)(intptr_t)image_texture, ImVec2(image_width, image_height));
}

void drawPaletteColors(xpldVideochip* vdu)
{
    ImGui::Text("Mode 0 palette colors:");
    for (int i = 0; i < 16; i++)
    {
        if ((i>0)&&(i!=8)) ImGui::SameLine();
        unsigned int palcol = vdu->getMode0Palette(i);
        ImGui::PushID(i);
        ImVec4 colf = ImVec4(((float)(palcol&0xff))/255.0f, ((float)((palcol>>8)&0xff)) / 255.0f, ((float)((palcol >> 16) & 0xff)) / 255.0f, 1.0f);
        //ImColor cl(colf);
        ImGui::PushStyleColor(ImGuiCol_Button, colf);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, colf);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, colf);
        ImGui::Button(" ");
        ImGui::PopStyleColor(3);
        ImGui::PopID();
    }

}

std::string utilPaddedHex(unsigned int n)
{
    std::string res = "";
    std::stringstream stream;
    stream << "0x" << std::setfill('0') << std::setw(4) << std::hex << n;
    res += stream.str();

    return res;
}

std::string byteToHex(unsigned char b)
{
    std::string res = "";
    std::stringstream stream;
    stream << std::hex << std::setfill('0') << std::setw(2) << (int)b;
    res += stream.str();
    return res;
}

void memoryViewer(xpldMMU* mmu)
{
    int mvRows = 10;
    int bytesPerRow = 10;
    unsigned int baseAddr = 0x500000;

    for (int r = 0;r < mvRows;r++)
    {
        std::string curRow = "";
        curRow += utilPaddedHex(baseAddr)+"  ";

        for (int c = 0;c < bytesPerRow;c++)
        {
            unsigned char b = mmu->read8(baseAddr + c);
            curRow += byteToHex(b)+" ";
        }

        ImGui::Text(curRow.c_str());

        baseAddr += bytesPerRow;
    }
}

int main(int, char**)
{
    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // Decide GL+GLSL versions
#ifdef __APPLE__
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1600,1000, "XPLD v0.2", NULL, NULL);
    if (window == NULL) return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Initialize OpenGL loader
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
    bool err = gl3wInit() != 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
    bool err = glewInit() != GLEW_OK;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
    bool err = gladLoadGL() == 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD2)
    bool err = gladLoadGL(glfwGetProcAddress) == 0; // glad2 recommend using the windowing library loader instead of the (optionally) bundled one.
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
    bool err = false;
    glbinding::Binding::initialize();
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
    bool err = false;
    glbinding::initialize([](const char* name) { return (glbinding::ProcAddress)glfwGetProcAddress(name); });
#else
    bool err = false; // If you use IMGUI_IMPL_OPENGL_LOADER_CUSTOM, your loader is likely to requires some form of initialization.
#endif
    if (err)
    {
        fprintf(stderr, "Failed to initialize OpenGL loader.\n");
        return 1;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\lucon.ttf", 18.0f);
    IM_ASSERT(font != NULL);

    //
    // load XPLD components
    //

    xpldVideochip* theVDU = new xpldVideochip();
    xpldMMU* theMmu = new xpldMMU(theVDU);
    xpldCPU* theCpu = new xpldCPU(theMmu);
    debugger* theDebugger = new debugger();

    unsigned char* biosPtr = theMmu->getBiosPtr();
    std::vector<std::string> disasmVector = theDebugger->disasmCode(biosPtr, 200);

    bool atStartup = true;
    bool isDebugWindowFocused = true;
    bool isRenderingWindowFocused = false;

    GLuint renderTexture;
    prepareGLTexture(theVDU,renderTexture);

    bool runToAddress = false;
    bool runCode = false;
    unsigned int targetAddress = 0;

    //

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    bool stepped = false;

    while (!glfwWindowShouldClose(window))
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();

        // keys for XPLD computer
        if (isRenderingWindowFocused)
        {
            for (int chr = 'A';chr <= 'Z';chr++)
            {
                if (ImGui::IsKeyPressed(chr))
                {
                    theMmu->setKeyPressed(chr+32);
                }
            }

            if (ImGui::IsKeyPressed(io.KeyMap[ImGuiKey_Space])) theMmu->setKeyPressed(32);
            if (ImGui::IsKeyPressed(io.KeyMap[ImGuiKey_Backspace])) theMmu->setKeyPressed(255);
            if (ImGui::IsKeyPressed('.')) theMmu->setKeyPressed(14+32);

            if (ImGui::IsKeyPressed(io.KeyMap[ImGuiKey_Enter]))
            {
                theMmu->setKeyPressed(13);
            }
        }

        // debug keys
        if (ImGui::IsKeyPressed('S'))
        {
            if (isDebugWindowFocused)
            {
                theCpu->stepOne();
                theVDU->stepOne();
                theVDU->renderFull();
                stepped = true;
            }
        }
        else if (ImGui::IsKeyPressed('R'))
        {
            if (isDebugWindowFocused)
            {
                runCode = true;
            }
        }
        else if (runToAddress||runCode)
        {
            int instrs = 0;
            bool goOut = false;

            while (!goOut)
            {
                theCpu->stepOne();
                theVDU->stepOne();

                unsigned int curPC = theCpu->getPC();
                if ((curPC == targetAddress)&&(runToAddress))
                {
                    runToAddress = false;
                    goOut = true;
                }
                else
                {
                    instrs++;
                }

                if (instrs >= 65536*4) goOut = true;
            }

            theVDU->renderFull();
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        bool show_demo_window = false;
        ImGui::ShowDemoWindow(&show_demo_window);

        // registers window
        {
            ImGui::Begin("Registers window");
            printRegistersInfo(theCpu,theVDU);
            ImGui::End();
        }

        // rendering window
        {
            ImGui::Begin("XPLD - Rendering");

            if (ImGui::IsWindowFocused())
            {
                isRenderingWindowFocused = true;
            }
            else isRenderingWindowFocused = false;

            renderToTexture(theVDU, renderTexture);
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }

        // control window
        {
            ImGui::Begin("Control window");
            if (ImGui::Button("Step"))
            {
                theCpu->stepOne();
                theVDU->stepOne();
                theVDU->renderFull();
                stepped = true;
            }
            if (ImGui::Button("Run"))
            {
                runCode = true;
            }

            drawPaletteColors(theVDU);

            if (ImGui::Button("Reset"))
            {
                theCpu->reset();
                theVDU->reset();
                runCode = false;
            }

            ImGui::End();
        }

        // memory viewer window
        {
            ImGui::Begin("Memory viewer window");
            memoryViewer(theMmu);
            ImGui::End();
        }

        // debug window
        {
            ImGui::Begin("Debugwindow");
            if (atStartup)
            {
                atStartup = false;
                ImGui::SetWindowFocus();
            }

            if (ImGui::IsWindowFocused())
            {
                isDebugWindowFocused = true;
            }
            else isDebugWindowFocused = false;
            
            for (int i = 0;i < disasmVector.size();i++)
            {
                unsigned int curaddr;
                std::stringstream ss;
                ss << std::hex << disasmVector[i].substr(2, 4);
                ss >> curaddr;
                unsigned int pc = theCpu->getPC();

                if (pc == curaddr)
                {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1));
                    ImGui::Selectable(disasmVector[i].c_str(), true);
                    ImGui::PopStyleColor();
                    if (stepped)
                    {
                        ImGui::SetScrollHere();
                        stepped = false;
                    }
                }
                else ImGui::Selectable(disasmVector[i].c_str(), false);

                std::string curElement = "CtxMenu" + std::to_string(i);
                if (ImGui::BeginPopupContextItem(curElement.c_str()))
                {
                    if (ImGui::Selectable("Run here"))
                    {
                        runToAddress = true;
                        targetAddress = curaddr;
                    }
                    ImGui::EndPopup();
                }
            }

            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup

    delete(theVDU);
    delete(theCpu);
    delete(theMmu);
    delete(theDebugger);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
