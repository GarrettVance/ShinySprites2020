

const char* shader_sprite_occlusion_fs = R"glsl(
    #version 420 core
    layout(location=0) out vec4 colorOut;

    in vec2 TexcoOriginal;
    in float comOcclusion; // holds +1 for visible, zero for occluded. 

    uniform float unifVisualizeOccluded; 

    uniform float unifIsDepthOnlyPass;

    layout(binding = 0) uniform sampler2D spriteImage;
    layout(binding = 1) uniform sampler2D sampler_depth;

    void main()
    {
        colorOut = texture(spriteImage, TexcoOriginal.xy);  

        if(comOcclusion < 0.5)
        {
            // Fragment is occluded by sphere[s]. 

            if(unifVisualizeOccluded > 0.5)
            {
                // Render any hidden fragments as solid red 
                // to help visualize the occlusion test:  

                colorOut = vec4(0.5, 0.0, 1.0, 1.0);
            }
            else
            {
                // Don't allow occluded fragments to be rendered: 

                discard; 
            }
        }
    }
)glsl";


