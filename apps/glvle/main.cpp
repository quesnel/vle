/*
 * This file is part of VLE, a framework for multi-modeling, simulation
 * and analysis of complex dynamical systems.
 * https://www.vle-project.org
 *
 * Copyright (c) 2003-2018 Gauthier Quesnel <gauthier.quesnel@inra.fr>
 * Copyright (c) 2003-2018 ULCO http://www.univ-littoral.fr
 * Copyright (c) 2007-2018 INRA http://www.inra.fr
 *
 * See the AUTHORS or Authors.txt file for copyright owners and
 * contributors
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <cstdio>

// About OpenGL function loaders: modern OpenGL doesn't have a standard header
// file and requires individual function pointers to be loaded manually. Helper
// libraries are often used for this purpose! Here we are supporting a few
// common ones: gl3w, glew, glad. You may use another loader/header of your
// choice (glext, glLoadGen, etc.), or chose to manually implement your own.
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
#include <GL/gl3w.h> // Initialize with gl3wInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
#include <GL/glew.h> // Initialize with glewInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
#include <glad/glad.h> // Initialize with gladLoadGL()
#else
#include IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#endif

// Include glfw3.h after our OpenGL definitions
#include <GLFW/glfw3.h>

#include "glvle.hpp"

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to
// maximize ease of testing and compatibility with old VS compilers. To link
// with VS2010-era libraries, VS2015+ requires linking with
// legacy_stdio_definitions.lib, which we do using this pragma. Your own
// project should not be affected, as you are likely to link with a newer
// binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) &&                                \
  !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

static void
glfw_error_callback(int error, const char* description)
{
    fprintf(
      stderr,
      "Glfw Error %d: %s\nTry to use declare environment variables before "
      "running glvle. For example:\n$ MESA_GLSL_VERSION_OVERRIDE=450 env "
      "MESA_GL_VERSION_OVERRIDE=4.5COMPAT ./glvle\n",
      error,
      description);
}

enum class gv_error_type
{
    success,
    help,
    short_argument_error,
    long_argument_error,
    open_package_error
};

struct command_line_status
{
    command_line_status(gv_error_type status_)
      : status(status_)
    {}

    command_line_status(char *package_, int optind_)
      : package(package_)
      , optind(optind_)
    {}

    char *package = nullptr;
    int optind = -1;
    gv_error_type status = gv_error_type::success;
};

// Parse command line option
// glvle [options...] [vpz files...]
// - `-P[ ]str`             short option to open a package.
// - --package[:|=]str]     long option to open a package.
// - [vpz files...]         list of vpz file to open after in the package.
//
// Use the package option to open a package directly when starting glvle.
static command_line_status
parse_argument(int argc, char* argv[])
{
    const char* short_package = "-P";
    const char* long_package = "--package";
    const auto short_package_sz = std::strlen(short_package);
    const auto long_package_sz = std::strlen(long_package);
    command_line_status status;
    char* package = nullptr;
    int optind = argc;
    int i = 1;

    while (i < optind) {
        const auto len = strlen(argv[i]);

        if (std::strncmp(argv[i], short_package, short_package_sz) == 0) {
            if (len > short_package_sz) {
                package = argv[i] + short_package_sz;
            } else {
                if (i + 1 < argc) {
                    ++i;
                    package = argv[i];
                } else {
                    return command_line_status(
                      gv_error_type::short_argument_error);
                }
            }
        } else if (std::strcmp(argv[i], long_package, long_package_sz) == 0) {
            if (len > long_package_sz && (argv[i][long_package_sz] == '=' ||
                                          argv[i][long_package_sz] == ':')) {
                package = argv[i] + long_package_sz;
            } else {
                if (i + 1 < argc) {
                    ++i;
                    package = argv[i];
                } else {
                    return command_line_status(
                        gv_error_type::long_argument_error);
                }
            }
        } else {
            --optind;
            if (i < optind)
                std::swap(argv[i], argv[optind]);
        }

        ++i;
    }

    return command_line_status(package, optind);
}

int
main(int argc, char* argv[])
{
    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

        // Decide GL+GLSL versions
#if __APPLE__
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+
    // only glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // 3.0+ only
#endif

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(
      1280, 720, "Dear ImGui GLFW+OpenGL3 example", NULL, NULL);
    if (window == NULL)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Initialize OpenGL loader
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
    bool err = gl3wInit() != 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
    bool err = glewInit() != GLEW_OK;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
    bool err = gladLoadGL() == 0;
#else
    bool err = false; // If you use IMGUI_IMPL_OPENGL_LOADER_CUSTOM, your loader
                      // is likely to requires some form of initialization.
#endif
    if (err) {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return 1;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable
    // Keyboard Controls io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    // // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsClassic();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can
    // also load multiple fonts and use ImGui::PushFont()/PopFont() to select
    // them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if
    // you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please
    // handle those errors in your application (e.g. use an assertion, or
    // display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and
    // stored into a texture when calling
    // ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame
    // below will call.
    // - Read 'misc/fonts/README.txt' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a
    // string literal you need to write a double backslash \\ !
    // io.Fonts->AddFontDefault();
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    // ImFont* font =
    // io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f,
    // NULL, io.Fonts->GetGlyphRangesJapanese()); IM_ASSERT(font != NULL);

    vle::glvle::Glvle gv;

    auto args = parse_argument(argc, argc);
    if (args.status == gv_error_type::success) {
        if (args.package) {
            try {
                gv.open(".", package, false);
            } catch (const std::exceptio& e) {
                fprintf(
                  stderr, "Fail to open package %s: %s\n", package, e.what());
                args.status = gv_error_type::open_package_error;
            }
        }
    }

    switch (args.status) {
    case gv_error_type::success:
        break;
    case gv_error_type::help:
        puts("glvle [options...] [vpz files...]\n\n"
             "Options:\n"
             "- -P[ ]str               short option to open a package.\n"
             "- --package[:|=]str]     long option to open a package.\n"
             "- [vpz files...]         list of vpz file to open after in the package.\n");
        return 0;
    case gv_error_type::short_argument_error:
        fprint(stderr, "Short argument error\n");
        return 1;
    case gv_error_type::long_argument_error:
        fprint(stderr, "Long argument error\n");
        return 2;
    case gv_error_type::open_package_error:
        fprintf(stderr, "Fail to open package %s\n", args.package);
        return 3;
    case gv_error_type::bag_cli_package:
        fprintf(stderr, "%s is not a package\n", args.package);
        return 4;
    };

    // For all other argument in command line (from the args.optind index),
    // search vpz files and try to open it in the experiment directory of the
    // package.
    if (args.package) {
        for (; args.optind != argc; ++args.optind) {
            auto dot = rindex(argv[args.optind], '.');

            if (dot) {
                if (strcmp(dot, ".vpz") == 0) {
                    auto p =
                        gv.package().getDir(vle::utils::Package::PKG_SOURCE);
                    p /= "exp";
                    p /= optind;

                    if (p.exists()) {
                        auto& vpz = gv.vpz_files[p.string()];
                        vpz.open(f.path.string());
                    } else {
                        fprintf(stderr, "File %s does not exist\n",
                                p.string().c_str());
                    }
                } else {
                    fprintf(stderr, "Can not open a non vpz file (%s)\n",
                            argv[args.optind]);
                }
            } else {
                fprintf(stderr, "Can not open a non vpz file (%s)\n",
                        argv[args.optind]);
            }
        }
    }

    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags
        // to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data
        // to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard
        // input data to your main application. Generally you may always pass
        // all inputs to dear imgui, and hide them from your application based
        // on those two flags.
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        show_app_menubar(gv);

        if (gv.show_package_window)
            gv.show_package_window = gv.package.show(gv);

        if (gv.show_log_window)
            gv.log_w.show(gv);

        {
            auto it = gv.txt_files.begin();

            while (it != gv.txt_files.end()) {
                if (!it->second.show())
                    it = gv.txt_files.erase(it);
                else
                    ++it;
            }
        }

        {
            auto it = gv.vpz_files.begin();

            while (it != gv.vpz_files.end()) {
                if (!it->second.show())
                    it = gv.vpz_files.erase(it);
                else
                    ++it;
            }
        }

        // 1. Show the big demo window (Most of the sample code is in
        // ImGui::ShowDemoWindow()! You can browse its code to learn more about
        // Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End
        // pair to created a named window.
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!"); // Create a window called "Hello,
                                           // world!" and append into it.

            ImGui::Text(
              "This is some useful text."); // Display some text (you can use a
                                            // format strings too)
            ImGui::Checkbox("Demo Window",
                            &show_demo_window); // Edit bools storing our
                                                // window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::SliderFloat(
              "float",
              &f,
              0.0f,
              1.0f); // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3(
              "clear color",
              (float*)&clear_color); // Edit 3 floats representing a color

            if (ImGui::Button(
                  "Button")) // Buttons return true when clicked (most widgets
                             // return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                        1000.0f / ImGui::GetIO().Framerate,
                        ImGui::GetIO().Framerate);
            ImGui::End();
        }

        // 3. Show another simple window.
        if (show_another_window) {
            ImGui::Begin(
              "Another Window",
              &show_another_window); // Pass a pointer to our bool variable
                                     // (the window will have a closing button
                                     // that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwMakeContextCurrent(window);
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(
          clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwMakeContextCurrent(window);
        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
