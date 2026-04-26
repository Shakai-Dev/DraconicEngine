import std;

#include <SDL3/SDL.h>

import core.io.filesystem;
import core.io.image_loader;

import rendering.rhi;
import rendering.rhi.vertex;

int main(int argc, char* argv[])
{
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::println("SDL init failed: {}", SDL_GetError());
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "Draconic Engine",
        1280, 720,
        SDL_WINDOW_RESIZABLE
    );

    if (!window) {
        std::println("Failed to create window: {}", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    const char* driver = SDL_GetCurrentVideoDriver();
    std::println("Driver: {}", driver ? driver : "Unknown");

    void* nwh = nullptr; // Native window handle
    void* ndt = nullptr; // Native display type

#if defined(__linux__)

    SDL_PropertiesID props = SDL_GetWindowProperties(window);

    if (driver && std::string_view(driver) == "x11")
    {
        ndt = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_X11_DISPLAY_POINTER, nullptr); // Get the X11 display pointer
        nwh = (void*)(uintptr_t)SDL_GetNumberProperty(props, SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0); // Get the X11 window number and cast it to a pointer
    }
    else if (driver && std::string_view(driver) == "wayland")
    {
        ndt = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WAYLAND_DISPLAY_POINTER, nullptr); // Get the Wayland display pointer
        nwh = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER, nullptr); // Get the Wayland surface pointer
    }
    else {
        std::println("Unsupported video driver: {}", driver);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }
    
#endif

    if (!nwh) {
        std::println("Failed to get native window handle");
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    if(!ndt) {
        std::println("Failed to get native display type");
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    // Init the RHI with the native window handle and initial size
    if (!draco::rhi::init(ndt, nwh, 1280, 720)) {
        std::println("Failed to initialize RHI");
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    draco::rhi::TexturedVertex quad[] = {
        { -0.5f,  0.5f, 0.0f, 0xffffffff, 0.0f, 0.0f },
        {  0.5f,  0.5f, 0.0f, 0xffffffff, 1.0f, 0.0f },
        {  0.5f, -0.5f, 0.0f, 0xffffffff, 1.0f, 1.0f },
        { -0.5f, -0.5f, 0.0f, 0xffffffff, 0.0f, 1.0f } 
    };

    uint16_t indices[] = {
        0, 1, 2, 
        2, 3, 0
    };

    auto vbh = draco::rhi::create_vertex_buffer(quad, sizeof(quad));
    auto ibh = draco::rhi::create_index_buffer(indices, sizeof(indices));

    auto img = draco::core::io::image_loader::load_image("test.png");
    draco::rhi::TextureHandle tex = draco::rhi::InvalidTexture;
    
    if (img.is_valid) {
        tex = draco::rhi::create_texture(img.pixels.data(), img.width, img.height);
    } else {
        std::println("Failed to load texture!");
    }

    auto s_texColor = draco::rhi::create_uniform("s_texColor", draco::rhi::UniformType::Sampler);

    // Load the vertex & fragment shaders
    auto vs_data = draco::core::io::filesystem::load_binary("vs_triangle.bin");
    auto fs_data = draco::core::io::filesystem::load_binary("fs_triangle.bin");

    // If the path is empty, return an error
    if (vs_data.empty() || fs_data.empty()) {
        std::println("Failed to load shaders");
        std::println("Working dir: {}", std::filesystem::current_path().string());
        draco::rhi::shutdown();
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    auto vsh = draco::rhi::create_shader(vs_data.data(), (uint32_t)vs_data.size());
    auto fsh = draco::rhi::create_shader(fs_data.data(), (uint32_t)fs_data.size());
    
    auto pipeline = draco::rhi::create_pipeline({
    vsh, 
    fsh, 
    draco::rhi::PipelineState::WriteRGB | 
    draco::rhi::PipelineState::WriteAlpha | 
    draco::rhi::PipelineState::MSAA
    });

    // Create a uniform for a color tint
    auto u_tint = draco::rhi::create_uniform("u_tint", draco::rhi::UniformType::Vec4);

    // Create a uniform for an offset
    auto u_offset = draco::rhi::create_uniform("u_offset", draco::rhi::UniformType::Vec4);

    auto offscreen_fb = draco::rhi::create_framebuffer(1280, 720, draco::rhi::TextureFormat::RGBA8);

    bool running = true;

    auto layout_desc = draco::rhi::get_textured_vertex_layout();
    auto layout_handle = draco::rhi::create_vertex_layout(layout_desc);

    auto point_sampler = draco::rhi::create_sampler(false, true);

    float time = 0.0f;
    while (running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
                running = false;
        }

        int w, h;
        SDL_GetWindowSize(window, &w, &h);
        draco::rhi::resize(uint16_t(w), uint16_t(h));

        time += 0.01f;

        draco::rhi::begin_frame();

        float view[16];
        draco::rhi::identity_matrix(view);

        float proj[16];
        draco::rhi::identity_matrix(proj);
        // Basic Ortho: Right=1.6, Left=-1.6, Top=0.9, Bottom=-0.9 (16:9 Aspect)
        proj[0] = 1.0f / 1.6f;
        proj[5] = 1.0f / 0.9f;
        proj[10] = 1.0f / (100.0f - 0.0f);
        proj[14] = -0.0f / (100.0f - 0.0f);

        // Upload Camera Data (Global Uniforms)
        draco::rhi::set_view_projection(0, view, proj);
        draco::rhi::set_view_projection(1, view, proj);

        draco::rhi::set_view_framebuffer(0, offscreen_fb);

        float tint[4] = { 1.0f, 0.5f, 0.2f, 1.0f };
        float offset[4] = { std::sin(time), std::cos(time), 0.0f, 0.0f };

        draco::rhi::set_uniform(u_tint, tint);
        draco::rhi::set_uniform(u_offset, offset);

        draco::rhi::RenderPacket packet{};
        packet.vertex_buffer = vbh;
        packet.index_buffer  = ibh;
        packet.pipeline      = pipeline;
        packet.texture_handle = tex;
        packet.uniform_handle = s_texColor; 
        packet.texture_unit   = 0;
        
        draco::rhi::identity_matrix(packet.model);
        draco::rhi::submit(packet, 0);

        draco::rhi::set_view_framebuffer(1, draco::rhi::InvalidFramebuffer);

        uint16_t scissorW = w / 2;
        uint16_t scissorH = h / 2;
        draco::rhi::set_scissor((w - scissorW) / 2, (h - scissorH) / 2, scissorW, scissorH);

        draco::rhi::RenderPacket display_packet{};
        display_packet.vertex_buffer = vbh;
        display_packet.index_buffer  = ibh;
        display_packet.pipeline      = pipeline;
        display_packet.texture_handle = draco::rhi::get_framebuffer_texture(offscreen_fb);
        display_packet.uniform_handle = s_texColor;
        display_packet.texture_unit   = 0;

        draco::rhi::identity_matrix(display_packet.model);
        draco::rhi::submit(display_packet, 1);

        draco::rhi::end_frame();

        draco::rhi::process_deletions();
    }

    draco::rhi::shutdown();

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
