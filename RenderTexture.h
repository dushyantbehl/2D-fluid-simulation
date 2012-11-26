//Source : http://troylawlor.com/tutorial-fbo.html
#pragma once
#ifndef _RENDERTEXTURE_H_
#define _RENDERTEXTURE_H_


#include <GL/glew.h>
#include <GL/glut.h>

class RenderTexture {
	private:
		//  we don't want to allow copies to be made
		RenderTexture&  operator = (const RenderTexture& other) {}
		RenderTexture(const RenderTexture& other) {}

	protected:
		GLuint texID;
		GLuint fbo;
		GLuint width;
		GLuint height;

	public:
		RenderTexture(GLuint p_width, GLuint p_height, GLuint format, GLuint internal_format);
		~RenderTexture();
		void startRender();
		void finishRender();
		void bind(GLuint unit=0);
		int textureID();
};


#endif
