#ifndef IMSAI_H
#define IMSAI_H
//
//
//
#include "pch.h"
//
//
//
class UVSphere; // forward declaration; 

class DevCamera; // forward declaration;
//
//
//
enum class RENDER_PASS
{
    PASS_1_DEPTH_ONLY,
    PASS_2_COLOR
};
//
//
//



enum class  SPHERE_TYPE
{
    SPHERE_CENTRAL, 
    SPHERE_SATELLITE
};



class Imsai
{
public:
    Imsai();

    ~Imsai();

    float getGLFWAspectRatio();

    void gvFindRotationalExtrema(float &p_minimum, float &p_maximum); 

    std::string loadShaderSourceFile(const char *filename); 

    unsigned int gvCreateProgramFromVSFS(const char* vs, const char* fs); 

    void gvCreateRTT2N3055();

    void coverSphereWithShines(SPHERE_TYPE sphType, unsigned int idxSprite, float gawgai, float kawkai, float dawdek);

    void generateRandomShineLocations();

    void init_VAO_VBO_Sphere(); 

    void init_VAO_VBO_Sprite(); 

    void init_VAO_VBO_FullScreenQuad(); 

    void init_VAO_VBO_Skybox(); 

    void drawTwoSpheres(glm::mat4 const& uModel); 

    void drawSpritesPass(RENDER_PASS renderPassType, glm::mat4 const& uModel);

    void drawSkyboxSky();

    void draw_fbo_depth_attachment_miniature(); 

    void RenderScene(unsigned int p_loopCount);

public:
    int                 mainWindowWidth;
    int                 mainWindowHeight;
    GLFWwindow          *theGLFWwindow;     

    float               clipZNear;
    float               clipZFar;
    float               occlusionTestEpsilonZ; 

    float const         devCamera_camera_InitialZ; 

    float const         depthVisualizer_ortho_edge; 

    glm::mat4           m4Model; 

    glm::mat4           m4Projection;

    glm::vec3           lightPosition;

    GLuint              rtt2N3055_fbo;
    GLuint              rtt2N3055_color2;
    GLuint              rtt2N3055_depth;

    UVSphere            *sphV;

    DevCamera           *dCamera;

    float const         sphere_Central_Radius; // Use 1.000f;
    float const         sphere_Satellite_Radius; // Use 0.500f;
    float const         sphere_Surface_Shim; 
    float const         sphere_Equatorial_Limit; 


    unsigned int            numShines;
    std::vector<GLfloat>    shinePositions;
    std::vector<GLuint>     shineElements;


    unsigned int        sphereVAO; 
    unsigned int        sphereVBO; 
    unsigned int        sphereEBO;
    unsigned int        sphereProgramId;

    unsigned int        spriteVAO1; 
    unsigned int        spriteVBO1; 
    unsigned int        spriteEBO1;
    unsigned int        spriteOcclusionTestProgramId;  


    unsigned int        fullScreenQuadVAO;   
    unsigned int        fullScreenQuadVBO; 
    unsigned int        fullScreenQuadEBO; 
    unsigned int        glsl_fbo_Depth_Attachment;

    unsigned int        skyboxVAO; 
    unsigned int        skyboxVBO;
    unsigned int        skyboxProgramId;


    bool                optSpheresUseWireframe;
    bool                optShowSkybox;
    bool                optVisualizeOccluded;

    unsigned int        textureIdMandalorian; 
    unsigned int        textureIdShine; 
    unsigned int        textureIdSkybox; 

};
#endif // IMSAI_H
