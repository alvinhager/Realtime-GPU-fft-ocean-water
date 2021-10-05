#pragma once
#include "Texture.h"
float borderColor[] = { 1.0f, 1.0f, 0.0f, 1.0f };
Texture::Texture() = default;

Texture::Texture(bool readonly, int texture_width, int texture_height)
{
	glGenTextures(1, &this->ID);

	glBindTexture(GL_TEXTURE_2D, this->ID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	if (readonly)
	{
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, texture_width, texture_height);

		char* whitenoise_data = new char[texture_width * texture_height * 4]{};
		for (int i = 0; i < texture_width; i++) {
			for (int j = 0; j < texture_height; j++) {

				int rand_value = rand() % 255;

				whitenoise_data[((i * texture_width + j) * 4) + 0] = rand_value;
				whitenoise_data[((i * texture_width + j) * 4) + 1] = rand_value;
				whitenoise_data[((i * texture_width + j) * 4) + 2] = rand_value;
				whitenoise_data[((i * texture_width + j) * 4) + 3] = 255;
			}
		}
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texture_width, texture_height, GL_RGBA, GL_UNSIGNED_BYTE, whitenoise_data);
		delete[] whitenoise_data;

	}
	else {
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, texture_width, texture_height);
	}

	glBindTexture(GL_TEXTURE_2D, 0);

}


Texture::Texture(const char* textureFilePath) {

	glGenTextures(1, &this->ID);
	glBindTexture(GL_TEXTURE_2D, this->ID);

	// set the texture wrapping/filtering options (on the currently bound texture object)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// load and generate the texture
	data = stbi_load(textureFilePath, &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);

}

Texture::~Texture()
{

}

GLuint Texture::getID()
{
	return this->ID;
}

void Texture::use()
{
	glBindTexture(GL_TEXTURE_2D, this->ID);
}

void Texture::deleteTexture()
{
	glDeleteTextures(1, &this->ID);
}
