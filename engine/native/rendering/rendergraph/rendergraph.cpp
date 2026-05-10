module;

#include <print>

#include <bgfx/bgfx.h>

module rendering.rendergraph;

import rendering.rhi;

namespace draco::rendering::rendergraph {

    void RenderGraph::reset()
    {
        for (auto& p : m_passes)
            p.packets.clear();
    }

    Pass& RenderGraph::add_pass(const std::string& name)
    {
        m_passes.emplace_back();
        m_passes.back().name = name;
        return m_passes.back();
    }

    Pass* RenderGraph::get_pass(const std::string& name)
    {
        for (auto& p : m_passes)
        {
            if (p.name == name)
                return &p;
        }
        return nullptr;
    }

    void RenderGraph::execute()
    {
        for (auto& pass : m_passes)
        {
            rhi::apply_view(pass.view, {pass.framebuffer, 0, 0, pass.width, pass.height, pass.clear_flags, pass.clear_color});

            rhi::set_view_projection(pass.view, pass.view_mtx, pass.proj_mtx);

            if (pass.clear_flags)
            {
                bgfx::setViewClear(pass.view, pass.clear_flags, pass.clear_color);
            }

            for (auto& pkt : pass.packets)
            {
                rhi::submit(pkt, pass.view);
            }
        }
    }
}