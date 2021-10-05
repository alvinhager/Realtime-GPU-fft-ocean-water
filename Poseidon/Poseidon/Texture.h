#pragma once

#include "stb_image.h"

#include <iostream>
#include <filesystem>
#include <sstream>
#include <fstream>
#include <gl/glew.h>

class Texture
{
public:
	Texture();
	Texture(bool readonly, int width, int height);
	Texture(const char* textureFilePath);
	~Texture();

	GLuint getID();

	void use();
	void deleteTexture();

private:
	int width;
	int height;
	int nrChannels;

	unsigned char* data;
	GLuint ID;

};


