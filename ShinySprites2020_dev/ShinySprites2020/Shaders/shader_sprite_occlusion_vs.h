

const char* shader_sprite_occlusion_vs = R"glsl(
    #version 420 core
    layout (location = 0) in vec3 attrPos;
    layout (location = 1) in vec3 attrNormal;
    layout (location = 2) in vec2 attrTexco;
    layout (location = 3) in vec4 attrSpriteSize;

    out vec2 TexcoOriginal;
    out float comOcclusion;

    uniform mat4 m4_World;
    uniform mat4 m4_View;
    uniform mat4 m4_Projection;
    uniform mat4 m4_WorldIT;
    uniform mat4 m4_ViewI;
    uniform mat4 m4_ViewProj; 
    uniform vec3 unifEyePos; 
    uniform vec3 unifLightPos;
    uniform float unifIsDepthOnlyPass;
    uniform float unifEpsilonZ;
    uniform float unifAspectRatio; 
    uniform float unifVisualizeOccluded; 
    uniform float Shininess = 30.0;

    layout(binding = 0) uniform sampler2D spriteImage;
    layout(binding = 1) uniform sampler2D sampler_depth;

    //
    //----> Default-initialized Uniforms
    //
    uniform vec2 CSTable[30] = { {1.0000, 0.0000} , {1.0000, 0.0000}
                , {1.0000, 0.0000} , {1.0000, 0.0000} , {1.0000, 0.0000}
                , {1.0000, 0.0000} , {0.4999, 0.8660} , {0.4383, 0.8987}
                , {0.5735, 0.8191} , {1.0000, 0.0000} , {1.0000, 0.0000}
                , {1.0000, 0.0000} , {1.0000, 0.0000} , {0.9396, 0.3420}
                , {1.0000, 0.0000} , {0.9396, 0.3420} , {1.0000, 0.0000}
                , {1.0000, 0.0000} , {1.0000, 0.0000} , {0.8829, 0.4694}
                , {0.8191, 0.5735} , {0.9396, 0.3420} , {1.0000, 0.0000}
                , {1.0000, 0.0000} , {1.0000, 0.0000} , {1.0000, 0.0000}
                , {1.0000, 0.0000} , {0.9848, 0.1736} , {1.0000, 0.0000}
                , {1.0000, 0.0000} 
    };


    vec4 computeSpriteCorner(vec2 dir2d, vec4 position, float scale, vec4 offset)
    {
        vec2 s;
        vec4 P;

        // we want the sparkles be 2d scaled so that their size isn't changed by perspective. 
        // CRUCIAL: must have correct window aspect ratio. 

        position /= position.w;  // ghv: perspective divide;

        s = vec2(1, unifAspectRatio) * ((scale * offset.y) + offset.x);
        P.xy = dir2d * s + position.xy;
        s = vec2(1, unifAspectRatio) * ((scale * offset.w) + offset.z);
        P.x = ( dir2d.y * s.x) + P.x;
        P.y = (-dir2d.x * s.y) + P.y;
        P.zw = position.zw;
        return P;
    }

    float OcclusionTest(vec4 hidePosClipSpace)
    {
        //==================================================================
        // Compare the zDepth of the shiny sprite to the zDepth of the 
        // corresponding point on the surface of the sphere.  
        // Return zero when the sphere occludes the shiny sprite; 
        // Return +1 if the shiny sprite is visible. 
        //==================================================================

        // Vertex Attribute attrPos.xyz in Object Space --> World Space --> Eye Space --> Clip Space:

        // vec4 spritePosObj = vec4(attrPos.xyz, 1.0); // the sprite's object-space position;
        // vec4 spritePosWorld = m4_World * vec4(attrPos.xyz, 1.0); // the sprite's world-space position; 
        // vec4 spritePosEye = m4_View * m4_World * vec4(attrPos.xyz, 1.0); // the sprite's eye-space position; 

        vec4 spritePosClip = m4_Projection * m4_View * m4_World * vec4(attrPos.xyz, 1.0); // the sprite's clip-space position; 

        // Use the perspective divide to transform the sprite position 
        // from Clip Space to NDC: 
        // (NDC coordinates take values on [-1, +1]).

        float xSpritePositionNDC = spritePosClip.x / spritePosClip.w; 
        float ySpritePositionNDC = spritePosClip.y / spritePosClip.w; 

        // then remap the NDC's [-1,+1] to [0,1] x [0,1] 
        // to get normalized texture coordinates: 

        float uSpritePosTexco = 0.5 + (xSpritePositionNDC / 2.0); 
        float vSpritePosTexco = 0.5 + (ySpritePositionNDC / 2.0);
        vec2 texcoSpritePos = vec2(uSpritePosTexco, vSpritePosTexco); 

        // Use these u,v normalized texture coordinates to sample
        // the depth texure. 
        // Recall that the Pass1 render enabled depth testing 
        // so that the sphere's depth values would be written 
        // to the frame buffer object's depth attachment 
        //  
        // The texture() function will return the depth of 
        // the sphere's surface at these <u,v> texture coordinates, 
        // termed "sphereDepth". 

        float sphereDepth = texture(sampler_depth, texcoSpritePos).r; 

        // sphereDepth returned from texture() takes values in [0, 1]. 
        // Convert sphereDepth to NDC by mapping 
        // the [0, 1] depth sample onto [-1, +1]. 
        // (NDC coordinates take values on [-1, +1]).

        float zSphereNDC =  2.0 * sphereDepth - 1.0;  

        // xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
        // xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

        // spritePosClip is the clip-space position OF THE SPRITE center; 
        // Use the perspective divide to transform the sprite position 
        // from Clip Space to NDC: 
        // (NDC coordinates take values on [-1, +1]).

        float zShineNDC = spritePosClip.z / spritePosClip.w;  

        float retval = 0.0;
        if ((zShineNDC - unifEpsilonZ) < zSphereNDC)
        {
            // The sprite is closer than the sphere, 
            // so the sprite is visible. Return +1: 
            retval = 1.0;
        }
        else
        {
            // The sprite is occluded by the sphere[s], 
            // so this sprite should be invisible. 
            // Return zero for "occluded": 
            retval = 0.0;
        }
        return retval;
    }

    void drawSprite(vec4 p_V_position, vec3 p_V_normal)
    {
        vec4 Po = p_V_position;
        vec4 Pw = m4_World * Po;
        vec3 Nw = normalize((m4_WorldIT * vec4(p_V_normal,0)).xyz);
        vec3 lightDir = normalize(unifLightPos - Pw.xyz);
        vec3 eyeDir = normalize(unifEyePos - Pw.xyz);   // normalize(m4_ViewI[3] - Pw); // D3D order

        //
        // get the second point
        //
        vec4 P2wv;
        P2wv.xyz = cross(lightDir, Nw) + Pw.xyz;
        P2wv.w = 0;

        //
        // specular vector
        //
        vec3 Hn = normalize(lightDir + eyeDir);

        //
        // compute the crystal reflection using only specular contribution: 
        //
        vec2 NLCosSin;
        int tabIndex;
        float NDotH;
        NLCosSin.x =    max(dot(Nw, Hn), 0);
        NLCosSin.y =    sqrt(1-(NLCosSin.x * NLCosSin.x));
        tabIndex =      int(max(NLCosSin.x * 30.0, 4));  // Use 30 for number of slices;
        NLCosSin =      NLCosSin * CSTable[tabIndex];
        NDotH =         NLCosSin.x + NLCosSin.y;
        float scale =   pow(max(NDotH, 0), Shininess);  // ghv: replacement for the asm "lit" opcode;

        //
        // Projections
        //
        vec4 projpos1_ClipSpace = (m4_ViewProj * Pw); // in homogeneous clip-space;
        vec4 projpos2 = (m4_ViewProj * P2wv); // in homogeneous clip-space;
        
        vec2 pos2d1_NDC = projpos1_ClipSpace.xy / projpos1_ClipSpace.ww; // the perspective divide yields normalized device coords;
        vec2 pos2d2_NDC = projpos2.xy / projpos2.ww; // the perspective divide yields normalized device coords;

        float MinWidth = 0.0; float MinHight = 0.0; float MaxWidth = 0.0; float MaxHight = 0.0;
        float actual_factor = 2.0; // must be < 3.0;



        if(unifIsDepthOnlyPass > 0.5)
        {
            actual_factor = 1.0; // Make the sprites smaller for the depth-only pass; 
        }

        MinWidth = attrSpriteSize.x * 0.02 * actual_factor; 
        MinHight = attrSpriteSize.y * 0.02 * actual_factor; 
        MaxWidth = attrSpriteSize.z * 0.06 * actual_factor; 
        MaxHight = attrSpriteSize.w * 0.06 * actual_factor;
      
        float minimumScale;  

        if(unifIsDepthOnlyPass > 0.5)
        {
            comOcclusion = 1.0; // Can't test occlusion in depth-only pass; 
            minimumScale = 0.0; 
        }
        else
        {
            comOcclusion = OcclusionTest(projpos1_ClipSpace);
            minimumScale = 0.05; 
        }

        if(scale > minimumScale)
        {
            vec2 dir2d_NDC  = normalize(pos2d2_NDC.xy - pos2d1_NDC.xy);  
            vec4 P11 = computeSpriteCorner(dir2d_NDC, projpos1_ClipSpace, scale, vec4(MinWidth, MaxWidth, MinHight, MaxHight)); 
            gl_Position = P11; 
        }
        else
        {
            gl_Position = vec4(0.0, 0.0, 0.0, 0.0);
        }
    }


    void main()
    {
        vec4 Po = vec4(attrPos.xyz, 1.0); // object-space position;
        drawSprite(Po, attrNormal.xyz); 
        TexcoOriginal = attrTexco;   
    }  
)glsl";


