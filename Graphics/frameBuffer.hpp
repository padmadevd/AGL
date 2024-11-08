#ifndef AGL_GRAPHICS_FRAMEBUFFER
#define AGL_GRAPHICS_FRAMEBUFFER

#include <Graphics/texture2d.hpp>

// FRAMEBUFFER --------------------------------------------------------------------------

struct FrameBuffer{

	public:
		FrameBuffer();
		void AttachDrawBufferColor(Texture2D *texture);
		void AttachReadBufferColor(Texture2D *texture);
		void BindDraw();
		void BindRead();
		void Free();
		~FrameBuffer();

	protected:
		unsigned int m_id;

		friend class BatchRenderer2D;
};

#endif