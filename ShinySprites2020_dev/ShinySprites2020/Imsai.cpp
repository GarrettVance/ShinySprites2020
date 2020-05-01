//
// 
// Imsai.cpp
//
//
#include "Imsai.h"
#include "TextureUtilities\UVSphere.h"
#include "DevCamera.h"  // TODO: remove camera files and add quaternion trackball mouse handler;


#include "Shaders\shader_sphere_vs.h"
#include "Shaders\shader_sphere_fs.h"
#include "Shaders\shader_sprite_occlusion_vs.h"
#include "Shaders\shader_sprite_occlusion_fs.h"
#include "Shaders\shader_skybox_vs.h"
#include "Shaders\shader_skybox_fs.h"



Imsai::Imsai() :   
        clipZNear(1.0f)         // use clipZNear = 1.00f; 
    ,   clipZFar(100.f)         // use clipZFar = 100.f;
    ,   occlusionTestEpsilonZ(0.01f) // use +0.01f; 
    ,   devCamera_camera_InitialZ(8.0f) // cameraPosition determines the origin of view-space coordinates; 
    ,   lightPosition(glm::vec3(4.0f, 4.0f, 4.0f))
    ,   sphere_Surface_Shim(0.015f) // use 0.015f; 
    ,   sphere_Central_Radius(1.000f) // Use 1.000f;
    ,   sphere_Satellite_Radius(0.500f) // Use 0.500f;
    ,   sphere_Equatorial_Limit(0.65f)  // Use 0.55f or 0.65f; 
    ,   numShines(166)  // use 148...166; 
    ,   mainWindowWidth(900)
    ,   mainWindowHeight(900)
    ,   depthVisualizer_ortho_edge(1.0f)
    ,   optSpheresUseWireframe(true)
    ,   optShowSkybox(true)
    ,   optVisualizeOccluded(false)
    ,   spriteOcclusionTestProgramId(0)
{
    sphV = new UVSphere(sphere_Central_Radius);

    dCamera = new DevCamera(glm::vec3(0.0f, 0.0f, devCamera_camera_InitialZ));
}






Imsai::~Imsai()
{
    // TODO : 
    /*
    if (sphereProgram)
    {
        delete sphereProgram;
        sphereProgram = nullptr;
    }

    if (skyboxProgram)
    {
        delete skyboxProgram;
        skyboxProgram = nullptr;
    }
    */
}



void Imsai::gvFindRotationalExtrema(float &p_perihelion, float &p_aphelion)
{
    glm::mat4 m4View = dCamera->GetViewMatrix();

    glm::vec4 worldSpaceAphelion = glm::vec4(0.f, 0.f, -2.0f, 1.0f);
    glm::vec4 worldSpacePerihelion = glm::vec4(0.f, 0.f, +2.0f, 1.0f);

    glm::vec4 clipSpaceAphelion = m4Projection * m4View * worldSpaceAphelion; 
    glm::vec4 clipSpacePerihelion = m4Projection * m4View * worldSpacePerihelion; 

    // Use the perspective divide to transform the point's position
    // from Clip Space to NDC: 
    // (NDC coordinates take values on [-1, +1]).

    float ndcSpaceAphelion = clipSpaceAphelion.z / clipSpaceAphelion.w; 
    float ndcSpacePerihelion = clipSpacePerihelion.z / clipSpacePerihelion.w; 

    p_perihelion = ndcSpacePerihelion; 
    p_aphelion = ndcSpaceAphelion; 
}



float Imsai::getGLFWAspectRatio()
{
    int windowWidth = 0;
    int windowHeight = 0; 
    glfwGetWindowSize(theGLFWwindow, &windowWidth, &windowHeight);
    return float(windowWidth) / windowHeight;
}









std::string Imsai::loadShaderSourceFile(const char *p_filename)
{
    std::ifstream inStream(p_filename, std::ios::in | std::ios::binary); // Trust me: use std::ios::binary;

    //
    // Even though the intention is to read an ASCII text file, 
    // leave the std::ios::binary argument alone. See Hans Passant's remark on stackexchange: 
    //
    // https://stackoverflow.com/questions/27055771/using-seekg-in-text-mode
    //

    if (inStream)
    {
        std::string shaderSource;

        inStream.seekg(0, std::ios::end);

        shaderSource.resize((size_t)inStream.tellg());

        inStream.seekg(0, std::ios::beg);

        inStream.read(&shaderSource[0], shaderSource.size());

        inStream.close();

        return(shaderSource);
    }
    throw(errno);
}







void getInfoLog(GLuint shader, std::string type)
{
    GLint success;
    GLchar infoLog[1024];
    if (type != "PROGRAM")
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "Shader compilation failed: " << type << "\n" << infoLog << "\n" << std::endl;
            __debugbreak(); 
        }
    }
    else
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "Shader link failed: " << type << "\n" << infoLog << "\n" << std::endl;
            __debugbreak(); 
        }
    }
}










    
unsigned int Imsai::gvCreateProgramFromVSFS(const char* vs, const char* fs)
{
    // std::string vertexShaderText = loadShaderSourceFile(vertexShaderPath.c_str()); 
    // std::string fragmentShaderText = loadShaderSourceFile(fragmentShaderPath.c_str());


    //xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    //
    //  https://stackoverflow.com/questions/6047527/how-to-pass-an-stdstring-to-glshadersource
    //
    // The return value of std::string::c_str() is a pointer value (i.e., an address) 
    // to a static string array held inside the data-structures of the std::string object. 
    //
    // Since the return value is just a temporary r-value (i.e., it's just a number stored in a CPU register), 
    // it is not an l-value and therefore it does not have a memory address; 
    // Moreover, you cannot use the & operator to "take its address" and cast to a pointer-to-pointer 
    // (There's nothing to take the address of). 
    // 
    //
    // Instead you first must save the return pointer value in a memory address. 
    // Memory-locations are l-values, and can have the address-of operator applied to them. 
    // 
    // Keep in mind that the pointer value returned is a temporary, 
    // meaning that if you do any operations on the std::string object, it could invalidate the pointer. 
    // 
    //xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

    unsigned int vertexShaderId = glCreateShader(GL_VERTEX_SHADER);


    // const char   *memoryLocationVertexShaderText = vertexShaderText.c_str(); // Must create an l-value;
    // glShaderSource(vertexShaderId, 1, &memoryLocationVertexShaderText, NULL); // Fine: no problem taking the address of an l-value;

    glShaderSource(vertexShaderId, 1, &vs, NULL); // Fine: no problem taking the address of an l-value;

    glCompileShader(vertexShaderId);
    getInfoLog(vertexShaderId, "VERTEX");








    unsigned int fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

    // const char   *memoryLocationFragmentShaderText = fragmentShaderText.c_str();
    // glShaderSource(fragmentShaderId, 1, &memoryLocationFragmentShaderText, NULL);


    glShaderSource(fragmentShaderId, 1, &fs, NULL);

    glCompileShader(fragmentShaderId);
    getInfoLog(fragmentShaderId, "FRAGMENT");


    unsigned int programId = glCreateProgram();
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);
    glLinkProgram(programId);
    getInfoLog(programId, "PROGRAM");

    glDeleteShader(vertexShaderId);
    glDeleteShader(fragmentShaderId);

    return programId;
}



void Imsai::init_VAO_VBO_Sphere()
{
    _ASSERTE(sphV);
    sphV->CreateVertexBuffer();

    glGenVertexArrays(1, &sphereVAO);
    glBindVertexArray(sphereVAO);

    glGenBuffers(1, &sphereVBO);
    glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);

    // Calculate total required allocation in Bytes: 
    GLuint totalBytesCombo = (GLuint)(sphV->sphereSurfacePositions.size() * sizeof(float));
    glBufferData(GL_ARRAY_BUFFER, totalBytesCombo, &(sphV->sphereSurfacePositions[0]), GL_STATIC_DRAW);

    glGenBuffers(1, &sphereEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);

    // Calculate the total required allocation in Bytes: 
    // cardinality_of_GLuints * sizeof(GLuint): 
    GLuint totalBytesElement = (GLuint)(sphV->sphereSurfaceElements.size() * sizeof(GLuint));
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, totalBytesElement, &(sphV->sphereSurfaceElements[0]), GL_STATIC_DRAW);

    // Vertex Byte Stride formerly was eight floats wide (<x,y,z>, <nx,ny,nz>, <u,v>): 
    // but is only six floats wide if i hide texture coordinates u,v: 

    glEnableVertexAttribArray(0); // x,y,z;
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1); // nx, ny, nz;
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));

    glEnableVertexAttribArray(2); // u,v;
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

    sphereProgramId = gvCreateProgramFromVSFS(shader_sphere_vs, shader_sphere_fs);
}








void Imsai::init_VAO_VBO_Sprite()
{
    generateRandomShineLocations();

    glGenVertexArrays(1, &spriteVAO1);
    glBindVertexArray(spriteVAO1);

    glGenBuffers(1, &spriteVBO1);
    glBindBuffer(GL_ARRAY_BUFFER, spriteVBO1);

    // Calculate total required allocation in Bytes: 
    GLuint totalBytesSpriteCombo = (GLuint)(shinePositions.size() * sizeof(float));
    glBufferData(GL_ARRAY_BUFFER, totalBytesSpriteCombo, &(shinePositions[0]), GL_STATIC_DRAW);

    glGenBuffers(1, &spriteEBO1);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, spriteEBO1);

    // Calculate the total required allocation in Bytes: 
    // cardinality_of_GLuints * sizeof(GLuint): 
    GLuint totalBytesElement = (GLuint)(shineElements.size() * sizeof(GLuint));
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, totalBytesElement, &(shineElements[0]), GL_STATIC_DRAW);

    // Vertex Byte Stride is twelve floats wide:
    // (<x,y,z>, <nx,ny,nz>, <u,v>, <min,min,max,max>): 
    size_t byteStride = 12 * sizeof(float);

    glEnableVertexAttribArray(0); // x,y,z;
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, byteStride, (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, byteStride, (void*)(3 * sizeof(float)));

    glEnableVertexAttribArray(2); // u,v;
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, byteStride, (void*)(6 * sizeof(float)));

    glEnableVertexAttribArray(3); // minSz, minSz, maxSz, maxSz;
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, byteStride, (void*)(8 * sizeof(float)));

    spriteOcclusionTestProgramId = gvCreateProgramFromVSFS(shader_sprite_occlusion_vs, shader_sprite_occlusion_fs); 
}








void Imsai::init_VAO_VBO_FullScreenQuad()
{
    glGenVertexArrays(1, &fullScreenQuadVAO);
    glBindVertexArray(fullScreenQuadVAO);

    glGenBuffers(1, &fullScreenQuadVBO);
    glBindBuffer(GL_ARRAY_BUFFER, fullScreenQuadVBO);

    // Calculate total required allocation in Bytes: 
    GLuint totalBytesVerts = (GLuint)(g0_fullScreenQuadVertices.size() * sizeof(float));
    glBufferData(GL_ARRAY_BUFFER, totalBytesVerts, &(g0_fullScreenQuadVertices[0]), GL_STATIC_DRAW);

    glGenBuffers(1, &fullScreenQuadEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fullScreenQuadEBO);

    // Calculate the total required allocation in Bytes: 
    // cardinality_of_GLuints * sizeof(GLuint): 
    GLuint totalBytesElement = (GLuint)(g0_fullScreenQuadElements.size() * sizeof(GLuint));
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, totalBytesElement, &(g0_fullScreenQuadElements[0]), GL_STATIC_DRAW);

    //      
    //  Vertex Shader: 
    //      
    const char* vertexProgramSource = R"glsl(
        #version 150 core

        in vec3 position3f;  // vertex attribute;
        in vec2 attTexCo2f;  // vertex attribute;

        out vec2 varyingTexCo2f;  // varying variable; 

        uniform mat4 unifWorldMatrix;
        uniform mat4 unifViewMatrix;
        uniform mat4 unifProjMatrix;

        void main()
        {
            varyingTexCo2f = attTexCo2f; 
            gl_Position = unifProjMatrix * unifViewMatrix * unifWorldMatrix * vec4(position3f.xyz, 1.0);
        }
    )glsl";
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexProgramSource, NULL);
    glCompileShader(vertexShader);
    GLint statusVertexShader;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &statusVertexShader);
    _ASSERTE(statusVertexShader == GL_TRUE);

    //      
    //  Pixel Shader: 
    //      
    const char* fragmentProgramSource = R"glsl(
        #version 150 core

        in vec2 varyingTexCo2f;  // varying variable; 
        uniform float unifPerihelion;
        uniform float unifAphelion;
        uniform sampler2D  unif_depth_tex; 
        out vec4 outColor;

        float sub_hue_to_rgb(float p, float q, float t)
        {
            float tt = t;
            if (tt < 0.0) tt += 1.0;
            if (tt > 1.0) tt -= 1.0;
            if (tt < 1.0/6.0) return p + (q - p) * 6.0 * tt;
            if (tt < 1.0/2.0) return q;
            if (tt < 2.0/3.0) return p + (q - p) * (2.0/3.0 - tt) * 6.0;
            return p;
        }

        vec4 color_HSL_to_RGB(float h, float s, float l)
        {
            float r = 0.f;
            float g = 0.f;
            float b = 0.f;
            if(s == 0.0)
            {
                r = g = b = l; // achromatic
            }
            else
            {
                float q;
                if (l < 0.5)
                { 
                    q = l * (1.0 + s);
                } 
                else
                { 
                    q = l + s - l * s;
                }
                float p = 2.0 * l - q;
                r = sub_hue_to_rgb(p, q, h + 1.0/3.0);
                g = sub_hue_to_rgb(p, q, h);
                b = sub_hue_to_rgb(p, q, h - 1.0/3.0);
            }
            return vec4(r, g, b, 1.f);
        }

        void main()
        {
            float depthBuffer = texture(unif_depth_tex, varyingTexCo2f).r;  

            float ndcSpaceDepth = 2.0 * depthBuffer - 1.0; 

            //
            // The uniforms unifPerihelion and unifAphelion are values 
            // of depth in NDC space. 
            //
            // Map the perihelion and aphelion depth values so that 
            // the interval [perihelion, aphelion] fills the entire [0,1] interval: 
            //
            //
            // Test data: 
            //      float unifPerihelion = 0.6835;  // NDC space;
            //      float unifAphelion = 0.8181;  // NDC space;
            //

            float dZeroToOne = (ndcSpaceDepth - unifPerihelion) / (unifAphelion - unifPerihelion); 

            if(ndcSpaceDepth > unifAphelion)
            {
                dZeroToOne = 1.0;
            }
            else if (ndcSpaceDepth < unifPerihelion)
            {
                dZeroToOne = 0.0;
            }

            outColor = color_HSL_to_RGB(dZeroToOne, dZeroToOne, dZeroToOne); 
        }
    )glsl";
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentProgramSource, NULL);
    glCompileShader(fragmentShader);
    GLint statusFragmentShader;
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &statusFragmentShader);
    _ASSERTE(statusFragmentShader == GL_TRUE);


    glsl_fbo_Depth_Attachment = glCreateProgram();
    glAttachShader(glsl_fbo_Depth_Attachment, vertexShader);
    glAttachShader(glsl_fbo_Depth_Attachment, fragmentShader);

    glBindFragDataLocation(glsl_fbo_Depth_Attachment, 0, "outColor");

    glLinkProgram(glsl_fbo_Depth_Attachment);
    glUseProgram(glsl_fbo_Depth_Attachment);

    {
        // Vertex Byte Stride is five floats wide: <x,y,z>,  <u,v>. 
        size_t vertexByteStride = 5 * sizeof(float);
        GLint nComponents = 0;


        GLint attrLoc_Position = glGetAttribLocation(glsl_fbo_Depth_Attachment, "position3f");
        nComponents = 3; // <x,y,z>;
        // Within each vertex, the offset of <x,y,z> data is zero;
        glVertexAttribPointer(attrLoc_Position, nComponents, GL_FLOAT, GL_FALSE, vertexByteStride, 0);
        glEnableVertexAttribArray(attrLoc_Position);


        GLint attrLoc_Texco = glGetAttribLocation(glsl_fbo_Depth_Attachment, "attTexCo2f");
        nComponents = 2; // <tex_s, tex_t>;
        // Within each vertex, the offset of <tex_s, tex_t> data is three floats wide;
        void * texCo2fOffset = (void *)(3 * sizeof(float));
        glVertexAttribPointer(attrLoc_Texco, nComponents, GL_FLOAT, GL_FALSE, vertexByteStride, texCo2fOffset);
        glEnableVertexAttribArray(attrLoc_Texco);
    }
}











void Imsai::init_VAO_VBO_Skybox()
{
    glGenVertexArrays(1, &skyboxVAO);
    glBindVertexArray(skyboxVAO);

    glGenBuffers(1, &skyboxVBO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(g0_skyboxSkyVertices), &g0_skyboxSkyVertices, GL_STATIC_DRAW);

    // Vertex Byte Stride is just three floats wide <x,y,z> for position: 
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    skyboxProgramId = gvCreateProgramFromVSFS(shader_skybox_vs, shader_skybox_fs); 
}


















void Imsai::drawTwoSpheres(glm::mat4 const& uModel)
{
    glUseProgram(sphereProgramId);   

    glm::mat4 m4View = dCamera->GetViewMatrix();


    glUniformMatrix4fv(glGetUniformLocation(sphereProgramId, "view"), 1, GL_FALSE, &m4View[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(sphereProgramId, "projection"), 1, GL_FALSE, &m4Projection[0][0]);
    glUniform3fv(glGetUniformLocation(sphereProgramId, "cameraPos"), 1, &dCamera->m_cameraPos[0]);



    glBindVertexArray(sphereVAO);

    glActiveTexture(GL_TEXTURE0); // bind to texture unit zero;
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureIdSkybox);

    {
        // Draw the central (major) sphere: 
    
        glUniformMatrix4fv(glGetUniformLocation(sphereProgramId, "model"), 1, GL_FALSE, &uModel[0][0]);

        glm::mat4 m4worldInverseTranspose = glm::transpose(glm::inverse(uModel));
        glUniformMatrix4fv(glGetUniformLocation(sphereProgramId, "m4_WorldIT"), 1, GL_FALSE, &m4worldInverseTranspose[0][0]);


        glDrawElements(GL_TRIANGLES, (GLuint)(sphV->sphereSurfaceElements.size()), GL_UNSIGNED_INT, NULL);
    }

    {
        // Draw the orbiting satellite (minor) sphere: 
        glm::mat4 satelliteScaling = glm::scale(
            glm::mat4(1.0f),
            glm::vec3(sphere_Satellite_Radius)
        );

        glm::mat4 satelliteXOffset = glm::translate(
            glm::mat4(1.0f),
            glm::vec3(sphere_Satellite_Radius + sphere_Central_Radius, 0.f, 0.f)
        );

        glm::mat4 satelliteModelMatrix = uModel * satelliteXOffset * satelliteScaling;
        glm::mat4 satWorldInverseTranspose = glm::transpose(glm::inverse(satelliteModelMatrix));

        glUniformMatrix4fv(glGetUniformLocation(sphereProgramId, "model"), 1, GL_FALSE, &satelliteModelMatrix[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(sphereProgramId, "m4_WorldIT"), 1, GL_FALSE, &satWorldInverseTranspose[0][0]);

        glDrawElements(GL_TRIANGLES, (GLuint)(sphV->sphereSurfaceElements.size()), GL_UNSIGNED_INT, NULL);
    }

    glBindVertexArray(0);  // unbind the vertex array;
}







void Imsai::drawSpritesPass(RENDER_PASS renderPassType, glm::mat4 const& uModel)
{
    _ASSERTE(
        (renderPassType == RENDER_PASS::PASS_1_DEPTH_ONLY) || 
        (renderPassType == RENDER_PASS::PASS_2_COLOR)
    );

    // In Pass2 mode, the sprite vertex shader 
    // will apply the occlusion test. 
    // The occlusion test result is then 
    // passed to the sprite's fragment shader. 


    glm::mat4 m4View = dCamera->GetViewMatrix();



    glm::mat4 worldInverseTranspose = glm::transpose(glm::inverse(uModel));
    glm::mat4 inverseView = glm::inverse(m4View);
    glm::mat4 viewProjectionMatrix = m4Projection * m4View;

    glUseProgram(spriteOcclusionTestProgramId);   

    glUniformMatrix4fv(glGetUniformLocation(spriteOcclusionTestProgramId, "m4_World"), 1, GL_FALSE, &uModel[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(spriteOcclusionTestProgramId, "m4_View"), 1, GL_FALSE, &m4View[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(spriteOcclusionTestProgramId, "m4_Projection"), 1, GL_FALSE, &m4Projection[0][0]);


    glUniformMatrix4fv(glGetUniformLocation(spriteOcclusionTestProgramId, "m4_WorldIT"), 1, GL_FALSE, &worldInverseTranspose[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(spriteOcclusionTestProgramId, "m4_ViewI"), 1, GL_FALSE, &inverseView[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(spriteOcclusionTestProgramId, "m4_ViewProj"), 1, GL_FALSE, &viewProjectionMatrix[0][0]);

    glUniform3fv(glGetUniformLocation(spriteOcclusionTestProgramId, "unifEyePos"), 1, &dCamera->m_cameraPos[0]); 
    glUniform3fv(glGetUniformLocation(spriteOcclusionTestProgramId, "unifLightPos"), 1, &lightPosition[0]); 

    glUniform1f(
        glGetUniformLocation(spriteOcclusionTestProgramId, "unifIsDepthOnlyPass"),
        (renderPassType == RENDER_PASS::PASS_1_DEPTH_ONLY) ? 1.0f : 0.0f
    );
    glUniform1f(glGetUniformLocation(spriteOcclusionTestProgramId, "unifEpsilonZ"), occlusionTestEpsilonZ); 
    glUniform1f(glGetUniformLocation(spriteOcclusionTestProgramId, "unifAspectRatio"), getGLFWAspectRatio()); 

    float floatBool = (optVisualizeOccluded == true) ? 1.0f : 0.0f; 
    glUniform1f(glGetUniformLocation(spriteOcclusionTestProgramId, "unifVisualizeOccluded"), floatBool); 


    // remove the following lines if using GLSL layout(binding=1) inside the shader; 
    // glUniform1i(glGetUniformLocation(spriteOcclusionTestProgramId, "spriteImage"), 0); 
    // glUniform1i(glGetUniformLocation(spriteOcclusionTestProgramId, "sampler_depth"), 1); 


    if (renderPassType == RENDER_PASS::PASS_2_COLOR)
    {
        glEnable(GL_BLEND); glBlendFunc(GL_SRC_COLOR, GL_ONE); // cf Sparkles Lorach 2007; 

        //$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
        // re: OpenGL Depth Test: 
        //
        // quoting from 
        // https://www.khronos.org/opengl/wiki/Common_Mistakes#Disable_depth_test_and_allow_depth_writes
        //
        // Disable depth test and allow depth writes
        // -----------------------------------------
        // In some cases, you might want to disable depth testing 
        // and still allow the depth buffer [to be] updated while you are 
        // rendering your objects. 
        // 
        // It turns out that if you disable depth testing (glDisable(GL_DEPTH_TEST)), 
        // GL also disables writes to the depth buffer. 
        //
        // The correct solution is to tell GL to ignore the depth test results 
        // with glDepthFunc(GL_ALWAYS). 
        // -----------------------------------------

        glDepthFunc(GL_ALWAYS);  // Looks good as of 20200418 at 23:38.

        //$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
   
        
        glDisable(GL_CULL_FACE); // TODO: remove; 
    }

    glBindVertexArray(spriteVAO1);

    glActiveTexture(GL_TEXTURE1); // bind my depth texture to texture unit 1;
    glBindTexture(GL_TEXTURE_2D, rtt2N3055_depth); // bind my depth texture to texture unit 1;

    glActiveTexture(GL_TEXTURE0); // bind the Shine image to texture unit zero;
    glBindTexture(GL_TEXTURE_2D, textureIdShine);

    glDrawElements(GL_TRIANGLES, (GLuint)(shineElements.size()), GL_UNSIGNED_INT, NULL);

    glBindVertexArray(0);  // unbind the vertex array;


    if (renderPassType == RENDER_PASS::PASS_2_COLOR)
    {
        glDisable(GL_CULL_FACE);  // TODO: remove; 

        // undo:  glDisable(GL_BLEND); 

        // TODO: revert depth test back to GL_LESS
    }
}







void Imsai::drawSkyboxSky()
{
    if (optShowSkybox)
    {
        glDepthFunc(GL_LEQUAL);

        glUseProgram(skyboxProgramId); 

        glm::mat4 m4View = dCamera->GetViewMatrix();

        glm::mat4 nonTranslatedViewMatrix = glm::mat4(
            glm::mat3(
                m4View
            )
        ); // remove translation from the view matrix
    
        glUniformMatrix4fv(glGetUniformLocation(skyboxProgramId, "view"), 1, GL_FALSE, &nonTranslatedViewMatrix[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(skyboxProgramId, "projection"), 1, GL_FALSE, &m4Projection[0][0]);

    
        glBindVertexArray(skyboxVAO);

        glActiveTexture(GL_TEXTURE0); // bind to texture unit zero;
        glBindTexture(GL_TEXTURE_CUBE_MAP, textureIdSkybox);

        glDrawArrays(GL_TRIANGLES, 0, 36);

        glBindVertexArray(0); // unbind the vertex array;

        glDepthFunc(GL_LESS); // revert depth function; 
    }
}





    
void Imsai::draw_fbo_depth_attachment_miniature()
{
    //
    // Diagnostic visualization of depth values. 
    // =================================================================
    // Render Pass1 enables the Depth Test in order that 
    // the sphere fragment program will write depth values 
    // to my fbo's depth attachment. 
    // 
    // The draw_fbo_depth_attachment() method runs a fragment program 
    // whose sampler object retrieves depth values from the fbo depth
    // attachment. 
    //
    // The scalar depth is interpreted as hue to generate an HSL 
    // (hue, saturation, luminance) color value. 
    // 

    // In order to better visualize the depth buffer, 
    // it is helpful to compute the extrema for the 
    // region of interest. 
    // 


    //  
    // Although the scene presents two spheres (Central and Satellite), 
    // the spheres rotate as a fixed single object. 
    // 
    // The vertical rotation axis coincides with the world-space z-axis
    // and passes through the center of the central sphere. 
    //
    // Considering the two spheres as a single rotating object, 
    // the part of this object that achieves the maximum 
    // and also the minimum distance from the camera is a point 
    // on the surface of the satellite sphere's equator opposite to 
    // where it touches the central sphere. Call this point the 
    // extremal point. 
    // 
    // (The extremal point is located at azimuth = 0.f and 
    // latitude = 0.f using the spherical coordinates described 
    // in generateRandomShineLocations()). 
    // 
    //
    // Denote the maximum and minimum distances from the camera 
    // by aphelion and perihelion. 
    // 

    float perihelion = 0.f; 
    float aphelion = 0.f; 
    gvFindRotationalExtrema(perihelion, aphelion); 


    glUseProgram(glsl_fbo_Depth_Attachment);

    glm::mat4  modelMatrix = glm::mat4(1.f);

    glm::mat4 tmpProjection = glm::ortho(
        -depthVisualizer_ortho_edge, 
        +depthVisualizer_ortho_edge, 
        +depthVisualizer_ortho_edge, 
        -depthVisualizer_ortho_edge, 
        clipZNear, 
        clipZFar
    );

    glm::vec3 fixed_cameraPosition = glm::vec3(0.f, 0.f, devCamera_camera_InitialZ);
    glm::vec3 fixed_lookAt = glm::vec3(0.f, 0.f, -5.0f); // TODO: remove hard-coded numerics; 
    glm::vec3 fixed_upVector = glm::vec3(0.f, 1.f, 0.f);
    glm::mat4 viewMatrix = glm::lookAtRH(fixed_cameraPosition, fixed_lookAt, fixed_upVector);

    glUniformMatrix4fv(glGetUniformLocation(glsl_fbo_Depth_Attachment, "unifWorldMatrix"), 1, GL_FALSE, &modelMatrix[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(glsl_fbo_Depth_Attachment, "unifViewMatrix"), 1, GL_FALSE, &viewMatrix[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(glsl_fbo_Depth_Attachment, "unifProjMatrix"), 1, GL_FALSE, &tmpProjection[0][0]);

    glUniform1f(glGetUniformLocation(glsl_fbo_Depth_Attachment, "unifPerihelion"), perihelion); 
    glUniform1f(glGetUniformLocation(glsl_fbo_Depth_Attachment, "unifAphelion"), aphelion); 


    glBindVertexArray(fullScreenQuadVAO);

    glActiveTexture(GL_TEXTURE0); // bind the depth texture image to texture unit zero;
    glBindTexture(GL_TEXTURE_2D, rtt2N3055_depth);

    glViewport(0, 0, mainWindowWidth / 4, mainWindowHeight / 4); // render to a miniature viewport; 
    glDrawElements(GL_TRIANGLES, (GLuint)(g0_fullScreenQuadElements.size()), GL_UNSIGNED_INT, NULL);
    glViewport(0, 0, mainWindowWidth, mainWindowHeight); // restore usual viewport;

    glBindVertexArray(0);  // unbind the vertex array;
}





//
//
//
void Imsai::coverSphereWithShines(SPHERE_TYPE sphType, unsigned int idxSprite, float parx, float pary, float parz)
{
    float pox = 0.f; float poy = 0.f; float poz = 0.f;

    if (sphType == SPHERE_TYPE::SPHERE_CENTRAL)
    {
        // Need to scale <x,y,z> by the sphere radius, then add fixed shim amount; 

        pox = parx * sphere_Central_Radius + sphere_Surface_Shim; 
        poy = pary * sphere_Central_Radius + sphere_Surface_Shim; 
        poz = parz * sphere_Central_Radius + sphere_Surface_Shim;
    }
    else if (sphType == SPHERE_TYPE::SPHERE_SATELLITE)
    {
        // For the satellite, need to first add the x-axis offset, 
        // which is just the sum of the two sphere radii.
        // Then, scale <x,y,z> by the sphere radius, 
        // and finally add the fixed shim amount; 

        float x_offset = sphere_Central_Radius + sphere_Satellite_Radius;

        pox = x_offset + parx * sphere_Satellite_Radius + sphere_Surface_Shim; 
        poy = pary * sphere_Satellite_Radius + sphere_Surface_Shim; 
        poz = parz * sphere_Satellite_Radius + sphere_Surface_Shim;
    }
    else
    {
        throw;
    }

    // Synthesize vertex for the lower-right corner of the sprite: 
    shinePositions.push_back(pox); shinePositions.push_back(poy); shinePositions.push_back(poz); // position;
    shinePositions.push_back(parx); shinePositions.push_back(pary); shinePositions.push_back(parz); // normal;
    shinePositions.push_back(1.f); shinePositions.push_back(1.f); // ...........................................texture coords;
    shinePositions.push_back(1.f); shinePositions.push_back(-1.f); shinePositions.push_back(1.f); shinePositions.push_back(-1.f); 

    // Synthesize vertex for the lower-left corner of the sprite: 
    shinePositions.push_back(pox); shinePositions.push_back(poy); shinePositions.push_back(poz); 
    shinePositions.push_back(parx); shinePositions.push_back(pary); shinePositions.push_back(parz); 
    shinePositions.push_back(0.f); shinePositions.push_back(1.f); 
    shinePositions.push_back(-1.f); shinePositions.push_back(-1.f); shinePositions.push_back(-1.f); shinePositions.push_back(-1.f); 

    // Synthesize vertex for the upper-left corner of the sprite: 
    shinePositions.push_back(pox); shinePositions.push_back(poy); shinePositions.push_back(poz); 
    shinePositions.push_back(parx); shinePositions.push_back(pary); shinePositions.push_back(parz); 
    shinePositions.push_back(0.f); shinePositions.push_back(0.f); 
    shinePositions.push_back(-1.f); shinePositions.push_back(+1.f); shinePositions.push_back(-1.f); shinePositions.push_back(+1.f); 

    // Synthesize vertex for the upper-right corner of the sprite: 
    shinePositions.push_back(pox); shinePositions.push_back(poy); shinePositions.push_back(poz); 
    shinePositions.push_back(parx); shinePositions.push_back(pary); shinePositions.push_back(parz); 
    shinePositions.push_back(1.f); shinePositions.push_back(0.f); 
    shinePositions.push_back(1.f); shinePositions.push_back(+1.f); shinePositions.push_back(1.f); shinePositions.push_back(+1.f); 


    // two triangles: 

    int kOne = 4 * idxSprite;

    shineElements.push_back(kOne + 1); shineElements.push_back(kOne + 2); shineElements.push_back(kOne + 0);

    shineElements.push_back(kOne + 3); shineElements.push_back(kOne + 0); shineElements.push_back(kOne + 2);
}
//
//
//
float randomFraction()
{
    int aRandomInt = rand(); 
    int intAndInt = aRandomInt & 255; 
    float properFraction = intAndInt / 255.f; 
    return properFraction;
}

void Imsai::generateRandomShineLocations()
{
    srand((unsigned int)glfwGetTime());  

    std::vector<VHG_SphereSurfacePoint> randomSurfacePoint;
    VHG_SphereSurfacePoint tmp; 

    for (unsigned int idxShine = 0; idxShine < numShines; idxShine++)
    {
        // Use spherical coordinates to position sprites "randomly"
        // about the surface of each sphere. 
        // 
        // sprites near the poles look bad, 
        // so the elevations have been limited to those 
        // of equatorial latitude. 
        // 
        float elevation = sphere_Equatorial_Limit * (randomFraction() - 0.5f) * XM_PI;
        float azimuth = randomFraction() * 2 * XM_PI;

        tmp.spriteIdx = idxShine; 
        tmp.X = cos(elevation) * cos(azimuth);  // x
        tmp.Z = cos(elevation) * sin(azimuth);  // z
        tmp.Y = sin(elevation);  // y
        randomSurfacePoint.push_back(tmp);
    }

    std::for_each(std::begin(randomSurfacePoint), std::end(randomSurfacePoint), [this](VHG_SphereSurfacePoint const& surface) {
        this->coverSphereWithShines(SPHERE_TYPE::SPHERE_CENTRAL, surface.spriteIdx, surface.X, surface.Y, surface.Z);
    }); 

    std::for_each(std::begin(randomSurfacePoint), std::end(randomSurfacePoint), [this](VHG_SphereSurfacePoint const& surface) {
        this->coverSphereWithShines(SPHERE_TYPE::SPHERE_SATELLITE, numShines + surface.spriteIdx, surface.X, surface.Y, surface.Z);
    }); 
}









