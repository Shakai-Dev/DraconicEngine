module;

#include <vector>
#include <array>
#include <string>
#include <cstdint>
#include <functional>
#include <cstring>
#include <algorithm>

export module rendering.rendergraph;

import rendering.rhi;

export namespace draco::rendering::rendergraph {

    struct Pass {
        std::string name;

        rhi::ViewID view;
        rhi::FramebufferHandle framebuffer;

        std::vector<rhi::RenderPacket> packets;

        float view_mtx[16];
        float proj_mtx[16];

        uint16_t width = 0;
        uint16_t height = 0;

        uint32_t clear_flags = 0;
        uint32_t clear_color = 0;
    };

    class RenderGraph {
    public:
        void reset();

        Pass& add_pass(const std::string& name);

        Pass* get_pass(const std::string& name);

        void execute();
    private:
        std::vector<Pass> m_passes;
    };
}
