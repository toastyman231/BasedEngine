#pragma once

#include "log.h"
#include "glad/glad.h"

namespace based::graphics
{
	void CheckGLError();
}

#ifndef BASED_CONFIG_RELEASE
#define BASED_CHECK_GL_ERROR based::graphics::CheckGLError();
#else
#define BASED_CHECK_GL_ERROR (void)0
#endif