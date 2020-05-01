#pragma once

#include <unordered_map>

class Texture;

typedef unsigned int GLuint;

class TextureManager
{
public:
	~TextureManager();

	/** Create a texture from file with specified format */
	Texture* createTexture(const char* name, GLuint format);


    
    Texture* createTextureFromMemory(
        const char * name,   
        unsigned char* buffer, 
        unsigned int bufLength,
        GLuint format
    ); // ghv; 



	/** Free a texture no longer needed by name */
	void destroyTexture(const char* name);

	/** Free a texture no longer needed by handle */
	void destroyTexture(Texture* texture);

#ifdef GHV_OPTION_WRITE_STB
	/** Saves a .png texture image data to disk */
	bool saveTexture(const Texture* texture);
#endif

	static TextureManager* getInstance()
	{
		static TextureManager* inst = new TextureManager();

		return inst;
	}

private:
	TextureManager();

	std::unordered_map<const char*, Texture*> m_textureList;

	Texture* getTexture(const char* name);

	int getNumComponent(GLuint format);
};