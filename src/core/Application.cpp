#include "glad/glad.h"
#include "../common/imgui-style.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl2.h"
#include "../common/functions.h"
#include "Application.h"


using namespace cpp_ge;

namespace cpp_ge::core
{
    Application::Application() {
        InitSDL();
        InitWindow();
        InitOpenGl();
        InitImGui();


        glClearColor(0.2, 0.2, 0.2, 1.0);
    }

    Application::~Application() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();

        SDL_GL_DeleteContext(rendering_context.gl_context);
        SDL_DestroyWindow(rendering_context.window_handle);
        SDL_Quit();

        std::cout << "\n- - -\n" << "[" << currentTime(std::chrono::system_clock::now()) << "] " << "Quit\n";
    }

    void Application::Run() {
        while (app_state.running)
        {
            HandleEvents();
            render();
        }

    }

    void Application::InitOpenGl() {
        rendering_context.gl_context = SDL_GL_CreateContext(rendering_context.window_handle);
        if (rendering_context.gl_context == nullptr)
        {
            std::cerr << "[ERROR] Failed to create a GL context: "
                      << SDL_GetError() << std::endl;
            throw EXIT_FAILURE;
        }
        SDL_GL_MakeCurrent(rendering_context.window_handle, rendering_context.gl_context);

        // enable VSync
        SDL_GL_SetSwapInterval(1);

        if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
        {
            std::cerr << "[ERROR] Couldn't initialize glad" << std::endl;
            throw EXIT_FAILURE;
        }
        else
        {
            std::cout << "[INFO] glad initialized" << std::endl;
        }

        std::cout << "[INFO] OpenGL renderer: "
                  << glGetString(GL_RENDERER)
                  << std::endl;

        std::cout << "[INFO] OpenGL version: "
                  << glGetString(GL_VERSION)
                  << std::endl;

        std::cout << "[INFO] OpenGL vendor: "
                  << glGetString(GL_VENDOR)
                  << std::endl;

        std::cout << "[INFO] OpenGL shading language version: "
                  << glGetString(GL_SHADING_LANGUAGE_VERSION)
                  << std::endl;

        int nNumExtensions;
        glGetIntegerv(GL_NUM_EXTENSIONS, &nNumExtensions);

        for(int i = 0; i < nNumExtensions; i++)
        {
            std::cout << "[INFO] OpenGL extension: "
                      << glGetStringi(GL_EXTENSIONS, i)
                      << std::endl;
        }

        // apparently, that shows maximum supported version
        std::cout << "[INFO] OpenGL from glad: "
                  << GLVersion.major
                  << "."
                  << GLVersion.minor
                  << std::endl;


        glViewport(0, 0, app_state.window_width, app_state.window_height);
    }

    void Application::InitImGui() const {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;

        setImGuiStyle();

        ImGui_ImplSDL2_InitForOpenGL(rendering_context.window_handle, rendering_context.gl_context);
        ImGui_ImplOpenGL3_Init(rendering_context.glsl_version.c_str());
    }

    void Application::InitWindow() {
        auto window_flags = (SDL_WindowFlags)(
                SDL_WINDOW_OPENGL
                | SDL_WINDOW_RESIZABLE
                | SDL_WINDOW_ALLOW_HIGHDPI
        );
        rendering_context.window_handle = SDL_CreateWindow(
                "Dear ImGui SDL",
                SDL_WINDOWPOS_CENTERED,
                SDL_WINDOWPOS_CENTERED,
                defWindowWidth,
                defWindowHeight,
                window_flags
        );

        SDL_SetWindowMinimumSize(rendering_context.window_handle, 500, 300);
    }

    void Application::InitSDL() {
        int result = 0;

        result = result | SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC);

        result = result | SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        result = result | SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
        result = result | SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

        result = result | SDL_GL_SetAttribute(
                SDL_GL_CONTEXT_PROFILE_MASK,
                SDL_GL_CONTEXT_PROFILE_CORE
        );

#ifdef __APPLE__
        // GL 3.2 Core + GLSL 150
        rendering_context.glsl_version = "#version 150";
        result = result | SDL_GL_SetAttribute( // required on Mac OS
            SDL_GL_CONTEXT_FLAGS,
            SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG
            );
        result = result | SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        result = result | SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
#elif __linux__
        // GL 3.2 Core + GLSL 150
        rendering_context.glsl_version = "#version 150";
        result = result | SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
        result = result | SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        result = result | SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
#elif _WIN32
        // GL 3.0 + GLSL 130
        rendering_context.glsl_version = "#version 130";
        result = result | SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
        result = result | SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        result = result | SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#endif

        if (result != 0)
        {
            std::cerr << "[ERROR] Failed to initialize SDL: "
                      << SDL_GetError() << std::endl;
            throw EXIT_FAILURE;
        }
    }

    void Application::HandleEvents() {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);

            switch (event.type)
            {
                case SDL_QUIT:
                    app_state.running = false;
                    break;

                case SDL_WINDOWEVENT:
                    switch (event.window.event)
                    {
                        case SDL_WINDOWEVENT_RESIZED:
                            app_state.window_width = event.window.data1;
                            app_state.window_height = event.window.data2;
                            glViewport(0, 0, app_state.window_width, app_state.window_height);
                            break;
                    }
                    break;

                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym)
                    {
                        case SDLK_ESCAPE:
                            app_state.running = false;
                            break;
                        case SDLK_f:
                            if (event.key.keysym.mod & KMOD_CTRL)
                            {
                                if (SDL_GetWindowFlags(rendering_context.window_handle) & SDL_WINDOW_FULLSCREEN_DESKTOP)
                                {
                                    SDL_SetWindowFullscreen(rendering_context.window_handle, 0);
                                }
                                else
                                {
                                    SDL_SetWindowFullscreen(rendering_context.window_handle, SDL_WINDOW_FULLSCREEN_DESKTOP);
                                }
                            }
                            break;
                        case SDLK_v:
                            if (event.key.keysym.mod & KMOD_CTRL)
                            {
                                SDL_GL_SetSwapInterval(!SDL_GL_GetSwapInterval());
                            }
                            break;
                        case SDLK_k:
                            if(input_context.controller != nullptr)
                            {
                                SDL_GameControllerClose(input_context.controller);
                                input_context.controller = nullptr;
                                std::cout << "[INFO] Controller disconnected" << std::endl;
                            }
                            else
                            {
                                std::cout << "[ERROR] No controller connected" << std::endl;
                            }
                            break;
                        case SDLK_h:
                        {
                            if(input_context.controller == nullptr) { break; }
                            input_context.joystick = SDL_GameControllerGetJoystick(input_context.controller);
                            if(input_context.joystick == nullptr) { break; }

                            int res = SDL_JoystickRumble(input_context.joystick, 0xFFFF, 0xFFFF, 1000);
                            std::cout << "[INFO] Rumble result: " << res << std::endl;
                            break;
                        }
                        case SDLK_j:
                            if(input_context.controller == nullptr)
                            {
                                SDL_JoystickUpdate();
                                int joyStickCount = SDL_NumJoysticks();
                                if(joyStickCount != 0)
                                {
                                    input_context.controller = SDL_GameControllerOpen(0);
                                    if(input_context.controller != nullptr)
                                    {
                                        std::cout << "[INFO] Controller connected" << std::endl;
                                    }
                                    else
                                    {
                                        std::cout << "[ERROR] Controller not connected" << std::endl;
                                        std::cout << SDL_GetError() << std::endl;
                                    }
                                }
                                else
                                {
                                    std::cout << "[ERROR] No controller connected" << std::endl;
                                }
                            }
                            else
                            {
                                std::cout << "[ERROR] Controller already connected" << std::endl;
                            }
                            break;
                        case SDLK_r:
                            log();
                            break;
                    }
                    break;
            }
        }
    }

    void Application::render() {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        // start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(rendering_context.window_handle);
        ImGui::NewFrame();

        // standard demo window
        if (app_state.show_demo_window) {
            ImGui::ShowDemoWindow(&app_state.show_demo_window);
        }

        // a window is defined by Begin/End pair
        {
            int sdl_width, sdl_height, controls_width;
            // get the window size as a base for calculating widgets geometry
            SDL_GetWindowSize(rendering_context.window_handle, &sdl_width, &sdl_height);
            controls_width = sdl_width;
            // make controls widget width to be 1/3 of the main window width
            if ((controls_width /= 3) < 300) { controls_width = 300; }

            // position the controls widget in the top-right corner with some margin
            ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
            // here we set the calculated width and also make the height to
            // be the height of the main window also with some margin
            ImGui::SetNextWindowSize(
                    ImVec2(float(controls_width), float(sdl_height)),
                    ImGuiCond_Always
            );
            // create a window and append into it

            ImGui::Begin("Controls", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

            ImGui::Dummy(ImVec2(0.0f, 1.0f));
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f), "Time");
            ImGui::Text("%s", currentTime(std::chrono::system_clock::now()).c_str());


            ImGui::Dummy(ImVec2(0.0f, 3.0f));
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f), "Platform");
            ImGui::Text("%s", SDL_GetPlatform());
            ImGui::Text("CPU cores: %d", SDL_GetCPUCount());
            ImGui::Text("RAM: %.2f GB", (double)SDL_GetSystemRAM() / 1024.0f);

            ImGui::Dummy(ImVec2(0.0f, 3.0f));
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f), "Application");
            ImGui::Text("Main window width: %d", sdl_width);
            ImGui::Text("Main window height: %d", sdl_height);

            ImGui::Dummy(ImVec2(0.0f, 3.0f));
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f), "SDL");

            ImGui::Dummy(ImVec2(0.0f, 10.0f));
            ImGui::Separator();
            ImGui::Dummy(ImVec2(0.0f, 10.0f));

            // buttons and most other widgets return true when clicked/edited/activated
            if (ImGui::Button("Counter button"))
            {
                std::cout << "counter button clicked\n";
                app_state.counter++;
                if (app_state.counter == 9) { ImGui::OpenPopup("Easter egg"); }
            }
            ImGui::SameLine();
            ImGui::Text("counter = %d", app_state.counter);

            if (ImGui::BeginPopupModal("Easter egg", nullptr))
            {
                ImGui::Text("Ho-ho, you found me!");
                if (ImGui::Button("Buy Ultimate Orb")) { ImGui::CloseCurrentPopup(); }
                ImGui::EndPopup();
            }

            ImGui::Dummy(ImVec2(0.0f, 15.0f));
            if (!app_state.show_demo_window)
            {
                if (ImGui::Button("Open standard demo"))
                {
                    app_state.show_demo_window = true;
                }
            }

            ImGui::Checkbox("show a custom window", &app_state.show_another_window);
            if (app_state.show_another_window)
            {
                ImGui::SetNextWindowSize(
                        ImVec2(400.0f, 350.0f),
                        ImGuiCond_FirstUseEver // after first launch it will use values from imgui.ini
                );
                // the window will have a closing button that will clear the bool variable
                ImGui::Begin("A custom window", &app_state.show_another_window);

                ImGui::Dummy(ImVec2(0.0f, 1.0f));
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f), "Files in the current folder");

                //ImGui::TextColored(ImVec4(128 / 255.0f, 128 / 255.0f, 128 / 255.0f, 1.0f), "%s", currentPath.string().data());
                ImGui::Dummy(ImVec2(0.0f, 0.5f));

                // static int currentFile = 0;
                // ImVec2 windowSize = ImGui::GetWindowSize();
                // ImGui::PushItemWidth(windowSize.x - 15);
                // ImGui::ListBox(
                //     "",
                //     &currentFile,
                //     vector_getter,
                //     &files,
                //     static_cast<int>(files.size())
                //     );

                ImGui::Dummy(ImVec2(0.0f, 1.0f));
                if (ImGui::Button("Close"))
                {
                    std::cout << "close button clicked\n";
                    app_state.show_another_window = false;
                }

                ImGui::End();
            }

            ImGui::End();
        }

        // rendering
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        SDL_GL_SwapWindow(rendering_context.window_handle);
    }


}