//
//
// ShinySprites2020.cpp : This file contains the 'main' function.
//
//
#include "pch.h"
#include "TextureUtilities\UVSphere.h"
#include "TextureUtilities\ResourceTextures.h"
#include "Imsai.h"





#include "DevCamera.h"  // TODO: remove camera files and add quaternion trackball mouse handler;

static Imsai* theImsai = nullptr;  // static ==> has internal linkage;




bool firstMouse = true; 


float deltaTime = 0.0f; 



// 
float lastFrame = 0.0f; // TODO: remove; 





void Imsai::gvCreateRTT2N3055()
{
    // Create a framebuffer object (fbo) for RTT (render-to-texture). 
    // The rtt2N3055 fbo has both an RGB color attachment as well as a depth attachment. 
    // cf Sparkles Lorach 2007 for DirectX D3D10. 

    glGenFramebuffers(1, &rtt2N3055_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, rtt2N3055_fbo);

    //xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    //          Create the color attachment: 
    // Can be implemented as a Texture or as Renderbuffer object. 
    //xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

    glGenTextures(1, &rtt2N3055_color2);
    glBindTexture(GL_TEXTURE_2D, rtt2N3055_color2);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGB,   // <------------------------------------------ GL_RGB.
        theImsai->mainWindowWidth, 
        theImsai->mainWindowHeight,  
        0,
        GL_RGB,   // <------------------------------------------ GL_RGB. 
        GL_UNSIGNED_BYTE,  // <--------------------------------- GL_UNSIGNED_BYTE. 
        0
    );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    //xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    //          Create the depth attachment: 
    // Can be implemented as a Texture or as Renderbuffer object. 
    //xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

    glGenTextures(1, &rtt2N3055_depth);
    glBindTexture(GL_TEXTURE_2D, rtt2N3055_depth);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_DEPTH_COMPONENT32F,   // <-------------------------GL_DEPTH_COMPONENT32F. 
        theImsai->mainWindowWidth, 
        theImsai->mainWindowHeight,  
        0,
        GL_DEPTH_COMPONENT,   // <--------------------------- GL_DEPTH_COMPONENT. 
        GL_FLOAT,  // <-------------------------------------- GL_FLOAT. 
        0
    );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    //xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    //          Attach color and depth to fbo: 
    //xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

    glFramebufferTexture(
        GL_FRAMEBUFFER, 
        GL_COLOR_ATTACHMENT2,  // <----------------------------------- GL_COLOR_ATTACHMENT2.
        rtt2N3055_color2, 
        0
    );
    
    glFramebufferTexture( 
        GL_FRAMEBUFFER, 
        GL_DEPTH_ATTACHMENT,  // <------------------------------------ GL_DEPTH_ATTACHMENT. 
        rtt2N3055_depth, 
        0 
    );

    // Set the list of draw buffers:  

    GLenum listOfDrawBuffers[1] = { GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(1, listOfDrawBuffers);

     
     
    //      Note that when I call glCopyTexImage2D() to copy the Depth Buffer 
    //      I preface that call with glReadBuffer(GL_BACK)  
    //      to indicate the source of the copy: copy from the back buffer; 
    //
    //  Quote from https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glReadBuffer.xhtml
    //
    //        glReadBuffer specifies a color buffer as the source for subsequent ...
    //       calls to ..., ...,  glCopyTexImage2D, .... 
    // 

    //xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    //          Check for completeness and unbind: 
    //xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

    GLenum fboStatus = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER);
    _ASSERTE(fboStatus == GL_FRAMEBUFFER_COMPLETE);


    glBindFramebuffer(GL_FRAMEBUFFER, 0); // CRUCIAL: don't forget to unbind fbo.
}








void GLAPIENTRY callback_for_glDebugMessageCallback(GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    const GLchar* message,
    const void* userParam)
{
    static unsigned int errCount = 0;

    if (++errCount > 21)
    {
        return;
    }

    std::string comparedString(message); 

    size_t foundAt = comparedString.find("will use VIDEO memory as the source for buffer object operations.");  

    if (type == GL_DEBUG_TYPE_OTHER && foundAt != std::string::npos)
    {
        //  fprintf(stderr, "____0x8251_buffer_object_operations___\n"); 
        return; 
    }

    fprintf(stderr, "\nGL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
        (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
        type, severity, message);
}
//
//
//














void quit(GLFWwindow *wd) // TODO: remove this unused function;
{
    glfwDestroyWindow(wd); 
    glfwTerminate(); 
    exit(0);
}














void kbd(GLFWwindow* wd, int key, int scancode, int action, int mods)
{
    if (action == GLFW_RELEASE) { // function is called first on GLFW_PRESS.
        return;
    }

    switch (key) {

    case GLFW_KEY_ESCAPE:
    case 'Q':
        glfwSetWindowShouldClose(theImsai->theGLFWwindow, true);
        // quit(wd); // TODO: revert this or delete target function;
        // glfwDestroyWindow(wd); glfwTerminate(); exit(0);
        break;


    case GLFW_KEY_F2: // toggle the skybox: show the sky above and all around the scene: 
        theImsai->optShowSkybox ^= true;
        break;


    case GLFW_KEY_F3: // toggle wireframe rasterization: 
        theImsai->optSpheresUseWireframe ^= true;
        break;


    case GLFW_KEY_F4: // toggle the visualization of occluded fragments: 
        theImsai->optVisualizeOccluded ^= true;
        break;








    case GLFW_KEY_W: // dolly forward;
        theImsai->dCamera->ProcessKeyboard(CAM_DIRECTION::CAM_FORWARD, deltaTime);
        break;

    case GLFW_KEY_S: // dolly back away from model;
        theImsai->dCamera->ProcessKeyboard(CAM_DIRECTION::CAM_BACK, deltaTime);
        break;



    case GLFW_KEY_LEFT: // traverse left;
        if ((mods & GLFW_MOD_CONTROL) == GLFW_MOD_CONTROL)
        {
            theImsai->dCamera->yawCamera(-1, deltaTime); 
        }
        else
        {
            theImsai->dCamera->ProcessKeyboard(CAM_DIRECTION::CAM_LEFT, deltaTime);
        }
        break;




    case GLFW_KEY_RIGHT: // traverse to the right;
        if ((mods & GLFW_MOD_CONTROL) == GLFW_MOD_CONTROL)
        {
            theImsai->dCamera->yawCamera(1, deltaTime); 
        }
        else
        {
            theImsai->dCamera->ProcessKeyboard(CAM_DIRECTION::CAM_RIGHT, deltaTime);
        }
        break;

    default:
        break;
    }
}

















void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}








void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    theImsai->dCamera->ProcessMouseScroll((float)yoffset);
}








glm::mat4 calcAngleFromFrameIdx(unsigned int p_frame_idx)
{
    // Set the angular velocity in powerOfTwo: 

    unsigned int powerOfTwo = 512;  // Slow <==> 4096; Fast <==> 256;

    unsigned int quarter = powerOfTwo / 4;
    unsigned int properIdx = p_frame_idx % powerOfTwo; 
    float properFraction = (float)properIdx / powerOfTwo; // takes values in [0.f, 0.9999999f];
    float theta = properFraction * 2.f * XM_PI;

    glm::mat4 yAxisRotation = glm::rotate(
        glm::mat4(1.0f),
        theta,
        glm::vec3(0.f, 1.f, 0.f)
    );

    return yAxisRotation;
}
//
//
//
void Imsai::RenderScene(unsigned int p_loopCount)
{
    glm::mat4 sphereYAxisRotation = calcAngleFromFrameIdx(p_loopCount); 

    //xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    //          Update the projection matrix: 
    //xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

    m4Projection = glm::perspective( // TODO: move this call;
        theImsai->dCamera->m_fovy,
        (float)mainWindowWidth / (float)mainWindowHeight,
        clipZNear, 
        clipZFar
    );

    //xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    // Render Pass1:
    //xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

    glBindFramebuffer(GL_FRAMEBUFFER, rtt2N3055_fbo);  
    {
        glViewport(0, 0, mainWindowWidth, mainWindowHeight);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS);
        glDisable(GL_BLEND);
        drawTwoSpheres(sphereYAxisRotation);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);  

    //xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    // Render Pass2:
    //xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

    draw_fbo_depth_attachment_miniature(); // Visualize the depth buffer; 

    drawSkyboxSky();

    if (optSpheresUseWireframe) 
    {
        // glPolygonMode option for the two spheres: 
        // ================================================================= 
        // Use wireframe (aka GL_LINE) mode to show a visual proof 
        // of the occlusion test's correctness. 
        //  
        // Wireframe mode makes the space BEHIND the two spheres visible. 
        // 
        // But when a shiny sprite's orbit carries it behind a sphere, 
        // the sprite goes invisible, being occluded as if the sphere were solid. 
        // 
        // This proves that the correct depth data were captured in the 
        // fbo's depth attachment and proves that those data are correctly 
        // accessed by the glsl sampler in the sprite vertex shader. 

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    drawTwoSpheres(sphereYAxisRotation);


    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    drawSpritesPass(RENDER_PASS::PASS_2_COLOR, sphereYAxisRotation);
    glDisable(GL_BLEND); 
}
//
//
//
int main(int argc, char *argv[])
{
    theImsai = new Imsai();

#ifdef GHV_OPTION_USE_GHV_VIEW_PROJ
    theImsai->CalcDefaultViewMatrix();
    theImsai->CalcDefaultProjectionMatrix();
#endif

    static unsigned int gvLoopCount = 0;


    if (!glfwInit()) // ghv: glfwInit() should be matched with glfwTerminate below...
    {
        exit(1);
    }

    //  Restrict application code to modern OpenGL API from the core profile: 
    //  cf   https://open.gl/context
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);


    // glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    // ghv: Context is created along with the window: 
    theImsai->theGLFWwindow = glfwCreateWindow(
        theImsai->mainWindowWidth, 
        theImsai->mainWindowHeight, 
        "[G. Vance / Miskatronic]...ShinySprites 2020", 
        nullptr, 
        nullptr
    );

    std::cout << "\nGLFW Context Major: " 
        << glfwGetWindowAttrib(
            theImsai->theGLFWwindow, 
            GLFW_CONTEXT_VERSION_MAJOR) 
        << std::endl;
    std::cout << "GLFW Context Minor: " 
        << glfwGetWindowAttrib(
            theImsai->theGLFWwindow, 
            GLFW_CONTEXT_VERSION_MINOR) 
        << std::endl;

    if (!theImsai->theGLFWwindow) 
    {
        glfwTerminate();
        exit(1);
    }

    glfwMakeContextCurrent(theImsai->theGLFWwindow);

    glfwSetFramebufferSizeCallback(
        theImsai->theGLFWwindow, 
        framebuffer_size_callback
    );




    // not glfwSetCursorPosCallback(theImsai->theGLFWwindow, mouse_callback);

    glfwSetScrollCallback(theImsai->theGLFWwindow, scroll_callback);


    //  Make application hard to use, except for easy 360-degree panning: 
    glfwSetInputMode(theImsai->theGLFWwindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


    GLenum retvalGLEWInit = glewInit();

    if (GLEW_OK != retvalGLEWInit) 
    {
        std::cerr << "Error: " 
            << glewGetErrorString(retvalGLEWInit) 
            << std::endl;
    }
    std::cout << "GLEW version " 
        << glewGetString(GLEW_VERSION) 
        << std::endl;


    // Initialize GLEW before registering debug message callback: 

    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(callback_for_glDebugMessageCallback, 0);


    // TODO:  glfwSetWindowSizeCallback(theImsai->theGLFWwindow, reshape); // TODO; 

    glfwSetWindowCloseCallback(theImsai->theGLFWwindow, quit);
    glfwSetKeyCallback(theImsai->theGLFWwindow, kbd); 


    glClearColor(0.2f, 0.2f, 0.2f, 1.0f); // choose a nice clear color;


    theImsai->init_VAO_VBO_Sphere(); 
    theImsai->init_VAO_VBO_Sprite(); 
    theImsai->init_VAO_VBO_FullScreenQuad(); 
    theImsai->init_VAO_VBO_Skybox(); 



    theImsai->gvCreateRTT2N3055(); 



    // Create OpenGL textures from win32 resources: 

    theImsai->textureIdShine = gvTextureFromResource(); 
    theImsai->textureIdSkybox = gv_textureLoad_SkyCube();


    do {
        gvLoopCount++;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        float currentFrame = (float)glfwGetTime();  // TODO:  float/double mis-match; 
        deltaTime = currentFrame - lastFrame; // time elapsed from prior frame to this;
        lastFrame = currentFrame;


        theImsai->RenderScene(gvLoopCount);

        glfwSwapBuffers(theImsai->theGLFWwindow);

        glfwPollEvents(); 

    } while (!glfwWindowShouldClose(theImsai->theGLFWwindow));

    // TODO:  memory leak? cleanup VAOs, VBOs, etc...

    if (theImsai->sphV) // TODO: move into class destructor; 
    {
        delete theImsai->sphV;
        theImsai->sphV = nullptr;
    }

    if (theImsai)
    {
        delete theImsai;
        theImsai = nullptr;
    }

    glfwTerminate(); // ghv: counterpart to glfwInit(); 
    return 0;
}









