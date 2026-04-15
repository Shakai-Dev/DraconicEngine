$input v_color0, v_texcoord0
#include <bgfx_shader.sh> 

SAMPLER2D(s_texColor, 0); 
uniform vec4 u_tint;

void main() { 
    // Sample the texture and multiply by vertex color and the tint
    vec4 texColor = texture2D(s_texColor, v_texcoord0);
    gl_FragColor = texColor * v_color0 * u_tint; 
}