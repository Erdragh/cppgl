#include <cppgl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <fstream>

// ------------------------------------------
// helper funcs and callbacks

void blit(const Texture2D& tex) {
    static Shader blit_shader("blit", "shader/quad.vs", "shader/blit.fs");
    blit_shader->bind();
    blit_shader->uniform("tex", tex, 0);
    Quad::draw();
    blit_shader->unbind();
}

void resize_callback(int w, int h) {
    Framebuffer::find("example_fbo")->resize(w, h);
}

void keyboard_callback(int key, int scancode, int action, int mods) {
    if (ImGui::GetIO().WantCaptureKeyboard) return;
    if (mods == GLFW_MOD_SHIFT && key == GLFW_KEY_R && action == GLFW_PRESS)
        reload_modified_shaders();
    if (key == GLFW_KEY_ENTER && action == GLFW_PRESS)
        Context::screenshot("screenshot.png");
}

void mouse_button_callback(int button, int action, int mods) {
    if (ImGui::GetIO().WantCaptureMouse) return;
}

void gui_callback(void) {
    ImGui::ShowMetricsWindow();
}

// ------------------------------------------
// main

int main(int argc, char** argv) {
    // init GL
    ContextParameters params;
    params.width = 1920;
    params.height = 1080;
    params.title = "CppGL example";
    //params.floating = GLFW_TRUE;
    //params.resizable = GLFW_FALSE;
    params.swap_interval = 1;
    Context::init(params); // TODO FIXME invalid query object msg
    Context::set_resize_callback(resize_callback);
    Context::set_keyboard_callback(keyboard_callback);
    Context::set_mouse_button_callback(mouse_button_callback);
    Context::set_gui_callback(gui_callback);

    // setup draw shader
    Shader("draw", "shader/draw.vs", "shader/draw.fs");

    // parse cmd line args
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "-w")
            Context::resize(std::stoi(argv[++i]), Context::resolution().y);
        else if (arg == "-h")
            Context::resize(Context::resolution().x, std::stoi(argv[++i]));
        else if (arg == "-pos") {
            current_camera()->pos.x = std::stof(argv[++i]);
            current_camera()->pos.y = std::stof(argv[++i]);
            current_camera()->pos.z = std::stof(argv[++i]);
        } else if (arg == "-dir") {
            current_camera()->dir.x = std::stof(argv[++i]);
            current_camera()->dir.y = std::stof(argv[++i]);
            current_camera()->dir.z = std::stof(argv[++i]);
        } else if (arg == "-fov")
            current_camera()->fov_degree = std::stof(argv[++i]);
        else {
            for (auto& mesh : load_meshes(argv[i], true))
                Drawelement(mesh->name, Shader::find("draw"), mesh);
        }
    }

    // setup fbo
    const glm::ivec2 res = Context::resolution();
    Framebuffer fbo = Framebuffer("example_fbo", res.x, res.y);
    fbo->attach_depthbuffer(Texture2D("example_fbo/depth", res.x, res.y, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT));
    fbo->attach_colorbuffer(Texture2D("example_fbo/col", res.x, res.y, GL_RGBA32F, GL_RGBA, GL_FLOAT));
    fbo->check();

    // run
    while (Context::running()) {
        // handle input
        glfwPollEvents();
        CameraImpl::default_input_handler(Context::frame_time());

        // update and reload shaders
        current_camera()->update();
        static uint32_t frame_counter = 0;
        if (frame_counter++ % 100 == 0)
            reload_modified_shaders();

        // render all drawelements into fbo
        fbo->bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        for (const auto& [key, drawelement] : Drawelement::map) {
            drawelement->bind();
            drawelement->draw();
            drawelement->unbind();
        }
        fbo->unbind();

        // draw
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        blit(fbo->color_textures[0]);

        // finish frame
        Context::swap_buffers();
    }
}
