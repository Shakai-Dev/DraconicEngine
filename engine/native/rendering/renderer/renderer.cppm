module;

#include <array>

#include <bx/math.h>

export module rendering.renderer;

import core.stdtypes;
import rendering.rhi;
import rendering.rendergraph;
import rendering.quad_renderer;

import scene;

export namespace draco::rendering::renderer {

    struct Camera {
        std::array<f32, 3> position {0.0f, 0.0f, 0.0f};
        std::array<f32, 3> target {0.0f, 0.0f, 0.0f};
        std::array<f32, 3> up {0.0f, 1.0f, 0.0f};
        f32 fov = 60.0f;
        f32 near_plane = 0.1f;
        f32 far_plane = 1000.0f;
    };

    struct SceneContext {
        u16 screen_width = 0;
        u16 screen_height = 0;
        Camera main_camera;

        draco::rendering::rendergraph::RenderGraph graph;
    };

    inline SceneContext g_ctx;

    void init(u16 width, u16 height);
    void resize(u16 width, u16 height);

    void begin_frame(const Camera& cam);

    void submit_entity(draco::rendering::rhi::RenderPacket& packet, u16 view);

    void end_frame();

    void render_scene(const draco::scene::Scene& scene);

    void submit_ui(draco::rendering::quad_renderer::QuadRenderer& quad_renderer);

    rendergraph::RenderGraph& get_graph();
}
