#include "graphics/mesh.h"
#include "graphics/helpers.h"
#include "glad/glad.h"

namespace based::graphics
{
	Mesh::Mesh(float* vertexArray, uint32_t vertexCount, uint32_t dimensions)
		: mVertexCount(vertexCount)
		, mEbo(0)
		, mElementCount(0)
	{
		glGenVertexArrays(1, &mVao); BASED_CHECK_GL_ERROR;
		glBindVertexArray(mVao); BASED_CHECK_GL_ERROR;

		glGenBuffers(1, &mPositionVbo); BASED_CHECK_GL_ERROR;
		glBindBuffer(GL_ARRAY_BUFFER, mPositionVbo); BASED_CHECK_GL_ERROR;
		glBufferData(GL_ARRAY_BUFFER, vertexCount * dimensions * sizeof(float), vertexArray, GL_STATIC_DRAW);
		BASED_CHECK_GL_ERROR;

		glEnableVertexAttribArray(0); BASED_CHECK_GL_ERROR;
		glVertexAttribPointer(0, dimensions, GL_FLOAT, GL_FALSE, 0, 0); BASED_CHECK_GL_ERROR;
		glDisableVertexAttribArray(0); BASED_CHECK_GL_ERROR;
		glBindBuffer(GL_ARRAY_BUFFER, 0); BASED_CHECK_GL_ERROR;

		glBindVertexArray(0); BASED_CHECK_GL_ERROR;
	}

	Mesh::Mesh(float* vertexArray, uint32_t vertexCount, uint32_t dimensions, uint32_t* elementArray, uint32_t elementCount)
		: Mesh(vertexArray, vertexCount, dimensions)
	{
		mElementCount = elementCount;
		glBindVertexArray(mVao); BASED_CHECK_GL_ERROR;

		glGenBuffers(1, &mEbo); BASED_CHECK_GL_ERROR;
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEbo); BASED_CHECK_GL_ERROR;
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, elementCount * sizeof(uint32_t), elementArray, GL_STATIC_DRAW);
		BASED_CHECK_GL_ERROR;

		glBindVertexArray(0); BASED_CHECK_GL_ERROR;
	}

	Mesh::Mesh(float* vertexArray, uint32_t vertexCount, uint32_t dimensions, float* texCoords, uint32_t* elementArray, 
		uint32_t elementCount)
		: Mesh(vertexArray, vertexCount, dimensions, elementArray, elementCount)
	{
		glBindVertexArray(mVao); BASED_CHECK_GL_ERROR;

		glGenBuffers(1, &mTexCoordsVbo); BASED_CHECK_GL_ERROR;
		glBindBuffer(GL_ARRAY_BUFFER, mTexCoordsVbo); BASED_CHECK_GL_ERROR;
		glBufferData(GL_ARRAY_BUFFER, vertexCount * 2 * sizeof(float), texCoords, GL_STATIC_DRAW);
		BASED_CHECK_GL_ERROR;

		glEnableVertexAttribArray(1); BASED_CHECK_GL_ERROR;
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0); BASED_CHECK_GL_ERROR;
		glBindBuffer(GL_ARRAY_BUFFER, 0); BASED_CHECK_GL_ERROR;

		glBindVertexArray(0); BASED_CHECK_GL_ERROR;
	}

	Mesh::~Mesh()
	{
		glDeleteBuffers(1, &mPositionVbo); BASED_CHECK_GL_ERROR;
		if (mEbo != 0)
		{
			glDeleteBuffers(1, &mEbo); BASED_CHECK_GL_ERROR;
		}
		glDeleteVertexArrays(1, &mVao); BASED_CHECK_GL_ERROR;
	}

	void Mesh::Bind()
	{
		glBindVertexArray(mVao); BASED_CHECK_GL_ERROR;
		glEnableVertexAttribArray(0); BASED_CHECK_GL_ERROR;
	}

	void Mesh::Unbind()
	{
		glDisableVertexAttribArray(0); BASED_CHECK_GL_ERROR;
		glBindVertexArray(0); BASED_CHECK_GL_ERROR;
	}
}