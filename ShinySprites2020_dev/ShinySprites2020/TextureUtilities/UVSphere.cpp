//
// 
// UVSphere.cpp
//
//
#include "..\pch.h"
#include "UVSphere.h"



UVSphere::UVSphere(float p_sphereRadius) :
        sphereRadius(p_sphereRadius)  // almost always use p_sphereRadius = 1.0f; 
    ,   sphereQuality(48.f) // using 85.f; use 128.f for zero artifacts;
{
    // Using lower quality on the spinning reflecting sphere
    // will increase rendering artifacts; 
    // But using quality of 128.f removes 
    // all artifacts. Which is awesome. 
    ;
}



//  
// The center of the sphere is implicitly at the origin; 
//  
void UVSphere::ComputeTextureCoordinates(
    float pThetaColatitude,
    float pLambdaLongitude,
    float pSRadius, 
    float uLocal, 
    float vLocal
)
{
    GLfloat eNmlx = 0.f; 
    GLfloat eNmly = 0.f; 
    GLfloat eNmlz = 0.f; 
    GLfloat posnx = 0.f; 
    GLfloat posny = 0.f; 
    GLfloat posnz = 0.f; 
    GLfloat texGlobalx = 0.f; 
    GLfloat texGlobaly = 0.f; 
    // Compute normal on the sphere: 
    eNmlx = sin(pThetaColatitude) * cos(pLambdaLongitude);           
    eNmlz = sin(pThetaColatitude) * sin(pLambdaLongitude);           
    eNmly = cos(pThetaColatitude);
    // Compute position based upon normal: 
    posnx = 0.f + pSRadius * eNmlx;        
    posnz = 0.f + pSRadius * eNmlz;        
    posny = 0.f + pSRadius * eNmly;
    //  
    //  Polar coordinates <polco_radius, polco_phi> 
    //  are used to identify points on the unit disk D: 
    //  0 <= polco_radius <= 1 and -pi < polco_phi <= +pi;
    //      
    float polco_phi = pLambdaLongitude - XM_PI; 
    float eta = 0.f;
    if (pThetaColatitude > XM_PI / 2.f)
    {
        eta = pThetaColatitude - XM_PI;  //  Replicate the hemisphere; 
        eta *= -1.f;
    }
    else
    {
        eta = pThetaColatitude; 
    }
    // Miscellaneous formulas for other kinds of projection: 
    // =================================================================
    // gold: float polco_radius = 0.5f * sin(eta / 4.f); // Modified Lambert Azimuthal Projection; 
    // platinum:  float polco_radius = sqrt(2.0) * sin(eta / 4.f); //  f * sqrt(2);
    // also interesting: float polco_radius = sin(eta / 4.f) / sqrt(2.0); //  f / sqrt(2);
    // trippy:  float polco_radius = sqrt(2.0) * sin(eta / 2.f); //  eta over 2 * sqrt(2);


    // undo 
    float polco_radius = 1.f * sin(eta / 2.f) / (float)sqrt(2.0); // Use this one!

    // float polco_radius = 0.5f * tan(eta / 4.f); // Modified Conformal Projection (Altered Stereographic); 

    float u_diskPolar = 0.5f + polco_radius * cos(polco_phi);  
    float v_diskPolar = 0.5f - polco_radius * sin(polco_phi);  
    texGlobalx = u_diskPolar; texGlobaly = v_diskPolar;
    //
    // Finally, write computed values out to vector of floats: 
    //
    sphereSurfacePositions.push_back(posnx); 
    sphereSurfacePositions.push_back(posny); 
    sphereSurfacePositions.push_back(posnz); 
    sphereSurfacePositions.push_back(eNmlx); 
    sphereSurfacePositions.push_back(eNmly); 
    sphereSurfacePositions.push_back(eNmlz); 
    sphereSurfacePositions.push_back(texGlobalx); 
    sphereSurfacePositions.push_back(texGlobaly); 
}
//
//
//
void UVSphere::CreateVertexBuffer()
{
    //========================================================================
    //   see  http://cse.csusb.edu/tongyu/courses/cs520/notes/texture.php
    //   which in turn quotes Paul Bourke. 
    //========================================================================
    GLuint          idxV = 0; // Hazard: stride is sizeof(GLuint); 
    GLuint          idxColat = 0;  //  Colatitude loop index; 
    GLuint          jdxLongit = 0; //  Longitude loop index;
    float           thetaColatitude = 0.f; // the present value of Colatitude;
    float           lambdaLongitude = 0.f; // the present value of Longitude;

    for (jdxLongit = 0; jdxLongit < sphereQuality; jdxLongit++)
    {
        for (idxColat = 0; idxColat < sphereQuality; idxColat++)  //  For Colatitude ranging from zero to pi: 
        {
            thetaColatitude = idxColat * XM_PI / (float)sphereQuality;  //  Colatitude;
            lambdaLongitude = jdxLongit * 2.f * XM_PI / (float)sphereQuality;   //  Longitude;
            this->ComputeTextureCoordinates( thetaColatitude, lambdaLongitude, sphereRadius, 0.f, 0.f );
            lambdaLongitude = (1 + jdxLongit) * 2.f * XM_PI / (float)sphereQuality;   //  Longitude;
            this->ComputeTextureCoordinates( thetaColatitude, lambdaLongitude, sphereRadius, 1.f, 0.f );
         
            thetaColatitude = (1 + idxColat) * XM_PI / (float)sphereQuality;  //  Colatitude; 
            lambdaLongitude = jdxLongit * 2.f * XM_PI / (float)sphereQuality;   //  Longitude;
            this->ComputeTextureCoordinates( thetaColatitude, lambdaLongitude, sphereRadius, 0.f, 1.f );
            lambdaLongitude = (1 + jdxLongit) * 2.f * XM_PI / (float)sphereQuality;   //  Longitude;
            this->ComputeTextureCoordinates( thetaColatitude, lambdaLongitude, sphereRadius, 1.f, 1.f );

            sphereSurfaceElements.push_back(idxV); 
            sphereSurfaceElements.push_back(idxV + 2); 
            sphereSurfaceElements.push_back(idxV + 1); 

            sphereSurfaceElements.push_back(idxV + 3); 
            sphereSurfaceElements.push_back(idxV + 1); 
            sphereSurfaceElements.push_back(idxV + 2); 

            idxV += 4;
        }
    }
}
