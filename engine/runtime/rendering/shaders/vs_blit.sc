$output v_texcoord0

#include <bgfx_shader.sh>

void main()
{
    // Generate vertex coordinates from index: 0, 1, 2
    // Vertex 0: (-1.0, -1.0) -> Bottom left
    // Vertex 1: ( 3.0, -1.0) -> Oversized right
    // Vertex 2: (-1.0,  3.0) -> Oversized top
    float x = -1.0 + float((gl_VertexID & 1) << 2);
    float y = -1.0 + float((gl_VertexID >> 1) << 2);

    gl_Position = vec4(x, y, 0.0, 1.0);

    // Map screen clip space positions seamlessly to [0.0, 1.0] texture coordinates
    v_texcoord0 = vec2(x, y) * vec2(0.5, -0.5) + vec2(0.5, 0.5);
}