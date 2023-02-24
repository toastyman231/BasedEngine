#include "graphics/helpers.h"

namespace based::graphics
{
	void CheckGLError()
	{
		GLenum error = glGetError();
		bool shouldAssert = error != GL_NO_ERROR;
		while (error != GL_NO_ERROR)
		{
			std::string errorstr;
			switch (error)
			{
			case GL_INVALID_OPERATION: errorstr = "INVALID_OPERATION"; break;
			case GL_INVALID_ENUM: errorstr = "GL_INVALID_ENUM"; break;
			case GL_INVALID_VALUE: errorstr = "GL_INVALID_VALUE"; break;
			case GL_OUT_OF_MEMORY: errorstr = "GL_OUT_OF_MEMORY"; break;
			case GL_INVALID_FRAMEBUFFER_OPERATION: errorstr = "GL_INVALID_FRAMEBUFFER_OPERATION"; break;
			default: errorstr = std::to_string(error);
			}

			BASED_ERROR("OpenGL Error: {}", errorstr.c_str());

			error = glGetError();
			BASED_ASSERT(!shouldAssert, "OpenGL Error!");
		}
		
	}

	/**
	 * \brief Finds the next power of two for an integer.
	 * \param v The integer to find the next power of two for.
	 * \return The next power of two of v.
	 *
	 * This code is directly copy/pasted from:
	 * http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
	*/
	unsigned int NextPowerOfTwo(unsigned int v)
	{
		v--;
		v |= v >> 1;
		v |= v >> 2;
		v |= v >> 4;
		v |= v >> 8;
		v |= v >> 16;
		v++;
		return v;
	}
}
