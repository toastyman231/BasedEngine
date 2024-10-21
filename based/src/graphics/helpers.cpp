#include "pch.h"
#include "graphics/helpers.h"

#include "glad/glad.h"

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

	bool ReadEntireFile(const std::string& filename, std::string& str)
	{
		std::ifstream f(filename);
		if (f) {
			std::ostringstream ss;
			ss << f.rdbuf();
			str = ss.str();
			return true;
		}
		return false;
	}
}
