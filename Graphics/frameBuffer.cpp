#include <Graphics/frameBuffer.hpp>

// FRAMEBUFFER ------------------------------------------------------------------------------

// struct FrameBuffer{

// 	public:
// 		FrameBuffer();
// 		void AttachDrawBufferColor(Texture2D *texture);
// 		void AttachReadBufferColor(Texture2D *texture);
// 		void BindDraw();
// 		void BindRead();
// 		void Free();
// 		~FrameBuffer();

// 	protected:
// 		unsigned int id;
// };

FrameBuffer::FrameBuffer(){
	glGenFramebuffers(1, &m_id);
}

void FrameBuffer::AttachDrawBufferColor(Texture2D *texture){
	glBindTexture(GL_TEXTURE_2D, texture->m_id);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_id);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture->m_id, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void FrameBuffer::AttachReadBufferColor(Texture2D *texture){
	glBindTexture(GL_TEXTURE_2D, texture->m_id);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_id);
	glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture->m_id, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void FrameBuffer::BindDraw(){
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_id);
}

void FrameBuffer::BindRead(){
	glBindFramebuffer(GL_READ_FRAMEBUFFER,m_id);
}

void FrameBuffer::Free(){
	glDeleteFramebuffers(1, &m_id);
}

FrameBuffer::~FrameBuffer(){
	Free();
}
