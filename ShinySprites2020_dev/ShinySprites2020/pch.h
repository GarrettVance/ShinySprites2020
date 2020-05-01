#ifndef PCH_H
#define PCH_H
//  
//  ghv: must include glew.h before glut.h: 
//  
#include "windows.h"
#define GLEW_STATIC
#include "GL\glew.h"
#include "GLFW\glfw3.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

// #include <iostream>
#include <chrono>
#include <vector>
#include <algorithm>


#include "TextureUtilities/MeshData.h"
#include "TextureUtilities/Texture.h"
#include "TextureUtilities/TextureManager.h"
#include "TextureUtilities/stb_image.h"


#define GHV_OPTION_DISABLE_CRAZY_MOUSE
#undef GHV_OPTION_USE_GHV_VIEW_PROJ
#undef GHV_OPTION_VISUAL_NORMALS
#define GHV_OPTION_USE_PBUFFER
#undef GHV_OPTION_ENABLE_SPRITES
#undef GHV_OPTION_SHOW_MINI_DEPTH

GLfloat const XM_PI = 3.1415926535f; // man this better be const or won't ever link...


// for TextureManager: 
#define SAFE_DELETE(x) { if (x) { delete x; x = nullptr; } }
#define SAFE_DELETE_ARRAY(x) { if (x) delete[] x; x = nullptr; }

#endif //PCH_H
