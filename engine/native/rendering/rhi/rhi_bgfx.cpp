module;

#include <vector>
#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <bx/math.h>

module rendering.rhi;

namespace draco::rhi
{
    struct Buffer
    {
        bgfx::VertexBufferHandle vbh;
    };

    struct Pipeline
    {
        bgfx::ProgramHandle program;
        uint64_t state;
    };

    static std::vector<Buffer> g_buffers;
    static std::vector<Pipeline> g_pipelines;

    void init(void* window_handle, uint16_t width, uint16_t height)
    {
        bgfx::Init init{};
        init.type = bgfx::RendererType::Count;

        init.resolution.width = width;
        init.resolution.height = height;
        init.resolution.reset = BGFX_RESET_VSYNC;

        bgfx::PlatformData pd{};
        pd.nwh = window_handle;
        bgfx::setPlatformData(pd);

        bgfx::init(init);

        bgfx::setDebug(BGFX_DEBUG_TEXT);
        bgfx::setViewClear(0,
            BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH,
            0x303030ff, 1.0f, 0);
    }

    ShaderHandle create_shader(const void* data, uint32_t size)
    {
        const bgfx::Memory* mem = bgfx::copy(data, size);
        return bgfx::createShader(mem).idx;
    }

    PipelineHandle create_pipeline(const PipelineDesc& desc)
    {
        bgfx::ShaderHandle vs{desc.vs};
        bgfx::ShaderHandle fs{desc.fs};

        bgfx::ProgramHandle prog = bgfx::createProgram(vs, fs, true);

        g_pipelines.push_back({prog, desc.state});
        return (PipelineHandle)(g_pipelines.size() - 1);
    }

    BufferHandle create_vertex_buffer(const void* data, uint32_t size)
    {
        bgfx::VertexLayout layout;
        layout.begin()
            .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
            .end();

        bgfx::VertexBufferHandle vbh = bgfx::createVertexBuffer(
            bgfx::copy(data, size),
            layout
        );

        g_buffers.push_back({vbh});
        return (BufferHandle)(g_buffers.size() - 1);
    }

    void submit(const RenderPacket& p, ViewID view)
    {
        Pipeline& pipe = g_pipelines[p.pipeline];
        Buffer& vb = g_buffers[p.vertex_buffer];

        bgfx::setTransform(p.model);
        bgfx::setVertexBuffer(0, vb.vbh);
        bgfx::setState(pipe.state);

        bgfx::submit(view, pipe.program);
    }

    void begin_frame()
    {
        // This is only for testing
        bgfx::setViewRect(0, 0, 0, 1280, 720); 

        float view[16]; 
        bx::mtxIdentity(view); 

        float proj[16]; 
        bx::mtxOrtho(
            proj,
            -1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 100.0f, 0.0f,
            false   // bgfx expects homogeneousNdc to be false
        );
        
        bgfx::setViewTransform(0, view, proj); 
        
        bgfx::touch(0);
    }

    void end_frame()
    {
        bgfx::frame();
    }
}