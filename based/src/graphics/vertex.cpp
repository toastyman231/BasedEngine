#include "pch.h"
#include "graphics/vertex.h"

#include "glad/glad.h"

namespace based::graphics
{
	const uint32_t RawVertexBuffer::GLTypeByte = GL_BYTE;
	const uint32_t RawVertexBuffer::GLTypeUByte = GL_UNSIGNED_BYTE;
	const uint32_t RawVertexBuffer::GLTypeShort = GL_SHORT;
	const uint32_t RawVertexBuffer::GLTypeUShort = GL_UNSIGNED_SHORT;
	const uint32_t RawVertexBuffer::GLTypeInt = GL_INT;
	const uint32_t RawVertexBuffer::GLTypeUInt = GL_UNSIGNED_INT;
	const uint32_t RawVertexBuffer::GLTypeFloat = GL_FLOAT;
	const uint32_t RawVertexBuffer::GLTypeDouble = GL_DOUBLE;

	RawVertexBuffer::RawVertexBuffer()
	{
		glGenBuffers(1, &mVbo); BASED_CHECK_GL_ERROR;
	}

	RawVertexBuffer::~RawVertexBuffer()
	{
		BASED_TRACE("Deleting vertex buffer {}", mVbo);
		glDeleteBuffers(1, &mVbo); BASED_CHECK_GL_ERROR;
	}

	void RawVertexBuffer::SetLayout(const std::vector<uint32_t>& layout)
	{
		mLayout = layout;
		mStride = 0;
		for (auto& count : layout)
		{
			mStride += count;
		}
	}

	void RawVertexBuffer::Upload(bool dynamic)
	{
		PROFILE_FUNCTION();
		glBindBuffer(GL_ARRAY_BUFFER, mVbo); BASED_CHECK_GL_ERROR;
		glBufferData(GL_ARRAY_BUFFER, mSize, mData, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW); BASED_CHECK_GL_ERROR;
		glBindBuffer(GL_ARRAY_BUFFER, 0); BASED_CHECK_GL_ERROR;
		mIsUploaded = true;
	}

	void RawVertexBuffer::Bind()
	{
		glBindBuffer(GL_ARRAY_BUFFER, mVbo); BASED_CHECK_GL_ERROR;
	}

	void RawVertexBuffer::Unbind()
	{
		glBindBuffer(GL_ARRAY_BUFFER, 0); BASED_CHECK_GL_ERROR;
	}

	VertexArray::VertexArray()
		: mVao(0)
		, mEbo(0)
		, mAttributeCount(0)
		, mVertexCount(0)
		, mElementCount(0)
		, mIsValid(false)
	{
		glGenVertexArrays(1, &mVao); BASED_CHECK_GL_ERROR;
	}

	VertexArray::~VertexArray()
	{
		mVbos.clear();
		glDeleteVertexArrays(1, &mVao); BASED_CHECK_GL_ERROR;
		glDeleteBuffers(1, &mEbo); BASED_CHECK_GL_ERROR;
	}

	void VertexArray::PushBuffer(std::unique_ptr<RawVertexBuffer> vbo)
	{
		PROFILE_FUNCTION();
		BASED_ASSERT(!vbo->GetLayout().empty(), "VertexBuffer has no layout defined");
		if (!vbo->GetLayout().empty())
		{
			mVbos.push_back(std::move(vbo));
			mVertexCount = (uint32_t)mVbos[0]->GetVertexCount();
		}
	}

	void VertexArray::SetElements(const std::vector<uint32_t>& elements)
	{
		PROFILE_FUNCTION();
		mElementCount = (uint32_t)elements.size();
		glBindVertexArray(mVao); BASED_CHECK_GL_ERROR;
		glGenBuffers(1, &mEbo); BASED_CHECK_GL_ERROR;
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEbo); BASED_CHECK_GL_ERROR;
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, elements.size() * sizeof(uint32_t), elements.data(), GL_STATIC_DRAW); BASED_CHECK_GL_ERROR;
		glBindVertexArray(0); BASED_CHECK_GL_ERROR;
	}

	void VertexArray::ClearElements()
	{
		mVbos.clear();
	}

	void VertexArray::Upload()
	{
		PROFILE_FUNCTION();
		glBindVertexArray(mVao); BASED_CHECK_GL_ERROR;
		uint32_t attributeCount = 0;
		for (auto& vbo : mVbos)
		{
			if (!vbo->IsUploaded())
			{
				vbo->Upload(false);
			} 
			vbo->Bind();
			uint32_t offset = 0;
			for (uint32_t count : vbo->GetLayout())
			{
				glEnableVertexAttribArray(attributeCount); BASED_CHECK_GL_ERROR;
				if (vbo->GetGLType() == RawVertexBuffer::GLTypeInt)
				{
					glVertexAttribIPointer(attributeCount, count, static_cast<GLenum>(vbo->GetGLType()), 
						vbo->GetStride(), (void*)(intptr_t)offset);
					BASED_CHECK_GL_ERROR;
				} else
				{
					glVertexAttribPointer(attributeCount, count, static_cast<GLenum>(vbo->GetGLType()), 
						GL_FALSE, vbo->GetStride(), (void*)(intptr_t)offset);
					BASED_CHECK_GL_ERROR;
				}
				if (vbo->IsInstanced()) glVertexAttribDivisor(attributeCount, 1); BASED_CHECK_GL_ERROR;

				attributeCount++;
				offset += (count * vbo->GetTypeSize());
			}
			vbo->Unbind();
		}
		glBindVertexArray(0); BASED_CHECK_GL_ERROR;
		mIsValid = true;
	}

	void VertexArray::Bind()
	{
		glBindVertexArray(mVao); BASED_CHECK_GL_ERROR;
	}

	void VertexArray::Unbind()
	{
		glBindVertexArray(0); BASED_CHECK_GL_ERROR;
	}
}