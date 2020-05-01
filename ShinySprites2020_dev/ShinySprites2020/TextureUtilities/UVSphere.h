#ifndef UVSPHERE_H
#define UVSPHERE_H
//
//
// #include "..\pch.h"
//
//
//
class VHG_SphereSurfacePoint
{
public:
    unsigned int spriteIdx; 
    float X;
    float Y;
    float Z;
};
//
//
//
class UVSphere
{
public:
    UVSphere(float p_sphereRadius);

    void ComputeTextureCoordinates(
        float pThetaColatitude,
        float pLambdaLongitude,
        float pSRadius,
        float uLocal,
        float vLocal
    );

    void CreateVertexBuffer();

public:
    float                   sphereRadius;
    float                   sphereQuality; 
    std::vector<GLfloat>    sphereSurfacePositions;
    std::vector<GLuint>     sphereSurfaceElements;
};
#endif //UVSPHERE_H
