//
// 
// ResourceTextures.cpp
//
//
#include "..\pch.h"
#include "ResourceTextures.h"
#include "..\resource.h"
//
//
//
unsigned int gvTextureFromResource()
{
    HMODULE hExe = GetModuleHandle(NULL); 
    HRSRC hResInfo = ::FindResource(hExe, MAKEINTRESOURCE(IDB_PNG4), L"PNG");
    HGLOBAL resourceHandle = ::LoadResource(NULL, hResInfo);
    unsigned char *resBytes = (unsigned char *)::LockResource(resourceHandle);

    // Need to know the size of the resource (the image): 

    DWORD bufLength = SizeofResource(hExe, hResInfo); 

    Texture* memTexture = TextureManager::getInstance()->createTextureFromMemory(
        "TextureResource", 
        resBytes,
        bufLength,
        GL_RGBA
    );

    glActiveTexture(GL_TEXTURE0); 
    glBindTexture(GL_TEXTURE_2D, memTexture->getTexId());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, memTexture->getWidth(), memTexture->getHeight());

    glTexSubImage2D(
        GL_TEXTURE_2D,  // target; 
        0,  // level;
        0,  // xOffset;
        0,  // yOffset; 
        memTexture->getWidth(),
        memTexture->getHeight(),
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        (const GLubyte*)memTexture->getImageData()
    );
    return memTexture->getTexId();
}
//
//
//
unsigned int gv_textureLoad_SkyCube()
{
    glActiveTexture(GL_TEXTURE0); 
    unsigned int tmpCubemapTextureID;
    glGenTextures(1, &tmpCubemapTextureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, tmpCubemapTextureID);

    std::vector<int> cubemapFaces; 
    cubemapFaces.push_back(IDB_PNG8); // Positive X;
    cubemapFaces.push_back(IDB_PNG5); // (negative x);
    cubemapFaces.push_back(IDB_PNG9); // Positive Y;
    cubemapFaces.push_back(IDB_PNG6); // (negative y);
    cubemapFaces.push_back(IDB_PNG7); // Positive Z;
    cubemapFaces.push_back(IDB_PNG10); // (negative z);

    HMODULE hExe = GetModuleHandle(NULL); 
    int width, height, nrChannels;
    unsigned char *data;

    for (GLuint i = 0; i < cubemapFaces.size(); i++)
    {
        HRSRC hResInfo = ::FindResource(hExe, MAKEINTRESOURCE(cubemapFaces[i]), L"PNG");

        HGLOBAL resourceHandle = ::LoadResource(NULL, hResInfo);
        
        unsigned char *resBytes = (unsigned char *)::LockResource(resourceHandle);
    
        DWORD bufLength = SizeofResource(hExe, hResInfo); 

        data = stbi_load_from_memory(
            resBytes,
            bufLength,
            &width, 
            &height, 
            &nrChannels, 
            0
        );

        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
            0, 
            GL_RGB, 
            width, 
            height, 
            0, 
            GL_RGB, 
            GL_UNSIGNED_BYTE, 
            data
        );
        
        stbi_image_free(data);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return tmpCubemapTextureID;
}
