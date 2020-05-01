
const char* shader_sphere_fs = R"glsl(
    #version 330 core
    layout(location = 0) out vec4 colorMiskatronic;

    in vec3 Normal;
    in vec3 Position;
    in vec2 Texco;

    uniform vec3 cameraPos;
    uniform samplerCube skybox;

    void main()
    {             
        vec3 I = normalize(Position - cameraPos);
        vec3 R = reflect(I, normalize(Normal));
        colorMiskatronic = vec4(texture(skybox, R).rgb, 1.0);
    }
)glsl";

