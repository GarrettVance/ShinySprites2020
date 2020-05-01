
const char* shader_sphere_vs = R"glsl(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec3 aNormal;
    layout (location = 2) in vec2 aTexco;

    out vec3 Normal;
    out vec3 Position;
    out vec2 Texco;

    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;
    uniform mat4 m4_WorldIT;

    void main()
    {
        Normal = mat3(m4_WorldIT) * aNormal;

        Position = vec3(model * vec4(aPos, 1.0));

        Texco = aTexco;
       
        gl_Position = projection * view * model * vec4(aPos.xyz, 1.0);
    }
)glsl";



