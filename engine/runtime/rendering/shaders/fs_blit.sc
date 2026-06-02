$input v_texcoord0

#include <bgfx_shader.sh>

SAMPLER2D(s_sceneTex, 0);

void main()
{
    // Sample the offscreen pass color maps directly
    vec3 sceneColor = texture2D(s_sceneTex, v_texcoord0).rgb;

    // This is where future post-processing scripts will be dropped in 

    gl_FragColor = vec4(sceneColor, 1.0);
}