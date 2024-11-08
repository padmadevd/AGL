#include <BatchRenderer/batchRenderer.hpp>
#include <Base/window.hpp>
#include <Base/Events/eventHandler.hpp>
#include <Base/Events/eventDispatcher.hpp>

#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <string>

extern EventDispatcher agl_eventDispatcher;

const char *vertShaderCode = 
"\n"
"layout (location=0) in vec2 vPos;\n"
"layout (location=1) in vec4 vColor;\n"
"layout (location=2) in vec2 vTC;\n"
"layout (location=3) in float vTexUnit;\n"
"\n"
"out vec4 iFragColor;\n"
"out vec2 iFragTC;\n"
"out float iFragTexUnit;\n"
"\n"
"uniform mat4 proj;\n"
"uniform mat4 view;\n"
"\n"
"void main(){\n"
"\t\n"
"\tgl_Position = proj*view*vec4(vPos.x, vPos.y, 0.0f, 1.0f);\n"
"\tiFragColor = vColor;\n"
"\tiFragTC = vTC;\n"
"\tiFragTexUnit = vTexUnit;\n"
"}";

const char *fragShaderCode = 
"precision mediump float;\n"
"\n"
"in vec4 iFragColor;\n"
"in vec2 iFragTC;\n"
"\n"
"in float iFragTexUnit;\n"
"\n"
"uniform sampler2D uTexture0;\n"
"uniform sampler2D uTexture1;\n"
"uniform sampler2D uTexture2;\n"
"uniform sampler2D uTexture3;\n"
"uniform sampler2D uTexture4;\n"
"uniform sampler2D uTexture5;\n"
"uniform sampler2D uTexture6;\n"
"uniform sampler2D uTexture7;\n"
"uniform sampler2D uTexture8;\n"
"uniform sampler2D uTexture9;\n"
"uniform sampler2D uTexture10;\n"
"uniform sampler2D uTexture11;\n"
"uniform sampler2D uTexture12;\n"
"uniform sampler2D uTexture13;\n"
"uniform sampler2D uTexture14;\n"
"uniform sampler2D uTexture15;\n"
"\n"
"vec4 Texture(vec2 fragTC){\n"
"\t\n"
"\tif(int(iFragTexUnit) == -1)\n"
"\t\treturn vec4(1.f, 1.f, 1.f, 1.f);\n"
"\tif(int(iFragTexUnit) == 0)\n"
"\t\treturn texture(uTexture0, fragTC);\n"
"\tif(int(iFragTexUnit) == 1)\n"
"\t\treturn texture(uTexture1, fragTC);\n"
"\tif(int(iFragTexUnit) == 2)\n"
"\t\treturn texture(uTexture2, fragTC);\n"
"\tif(int(iFragTexUnit) == 3)\n"
"\t\treturn texture(uTexture3, fragTC);\n"
"\tif(int(iFragTexUnit) == 4)\n"
"\t\treturn texture(uTexture4, fragTC);\n"
"\tif(int(iFragTexUnit) == 5)\n"
"\t\treturn texture(uTexture5, fragTC);\n"
"\tif(int(iFragTexUnit) == 6)\n"
"\t\treturn texture(uTexture6, fragTC);\n"
"\tif(int(iFragTexUnit) == 7)\n"
"\t\treturn texture(uTexture7, fragTC);\n"
"\tif(int(iFragTexUnit) == 8)\n"
"\t\treturn texture(uTexture8, fragTC);\n"
"\tif(int(iFragTexUnit) == 9)\n"
"\t\treturn texture(uTexture9, fragTC);\n"
"\tif(int(iFragTexUnit) == 10)\n"
"\t\treturn texture(uTexture10, fragTC);\n"
"\tif(int(iFragTexUnit) == 11)\n"
"\t\treturn texture(uTexture11, fragTC);\n"
"\tif(int(iFragTexUnit) == 12)\n"
"\t\treturn texture(uTexture12, fragTC);\n"
"\tif(int(iFragTexUnit) == 13)\n"
"\t\treturn texture(uTexture13, fragTC);\n"
"\tif(int(iFragTexUnit) == 14)\n"
"\t\treturn texture(uTexture14, fragTC);\n"
"\tif(int(iFragTexUnit) == 15)\n"
"\t\treturn texture(uTexture15, fragTC);\n"
"}\n"
"\n"
"out vec4 outFragColor;\n"
"\n"
"void main(){\n"
"\t\n"
"\tvec4 color = Texture(iFragTC);\n"
"\toutFragColor = color*iFragColor;\n"
"}";

#ifndef BATCHES_PER_DRAW
 #define BATCHES_PER_DRAW 10
#endif

BatchRenderer2D::BatchRenderer2D(glm::vec2 winSize, uint32_t batchCount, uint32_t vertexPerBatch, uint32_t indexPerBatch, uint32_t texPerBatch)
{

	m_windowSize = winSize;
	
	for(int i = 0; i < batchCount; i++)
		m_batches.push_back(new Batch(vertexPerBatch, indexPerBatch, texPerBatch));
	m_batchN = 0;

	m_shader = new Shader();
	m_shader->FromString((std::string(glsl_header)+std::string(vertShaderCode)).c_str(), (std::string(glsl_header)+std::string(fragShaderCode)).c_str());
	//m_shader.FromFile("vert.vs", "frag.fs");
	
	m_shader->Use();
	char uniformName[100];
	for(int i = 0; i < texPerBatch; i++){
		sprintf(uniformName, "uTexture%d", i);
		m_shader->Set1i(uniformName, i);
	}

	int FB;
	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &FB);
	m_defaultDrawFB = new FrameBuffer;
	m_defaultDrawFB->m_id = FB;

	glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &FB);
	m_defaultReadFB = new FrameBuffer;
	m_defaultReadFB->m_id = FB;

	m_flipY = false;
	m_customShader = false;
	m_beginDraw = false;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendEquationSeparate(GL_FUNC_ADD, GL_MAX);
	glViewport(0, 0, winSize.x, winSize.y);

	agl_eventDispatcher.Register("AGL_EVENT_WINDOW_RESIZE", new EventHandler(std::bind(&OnWindowResize, this, std::placeholders::_1, std::placeholders::_2), nullptr, "AGL_BATCHRENDERER_RESIZE_HANDLER"));
}

BatchRenderer2D::~BatchRenderer2D(){

	for(int i = 0; i < m_batches.size(); i++){
		delete m_batches[i];
	}

	delete m_shader;
	delete m_defaultReadFB;
	delete m_defaultDrawFB;
}

void BatchRenderer2D::OnWindowResize(const Event* event, void *userData){

	const EventWindowResize *w_event = (const EventWindowResize *)event;
	m_windowSize.x = w_event->m_width_px;
	m_windowSize.y = w_event->m_height_px;
	glViewport(0, 0, m_windowSize.x, m_windowSize.y);
}

void BatchRenderer2D::SetFlipY(bool flip){
	m_flipY = flip;
}

FrameBuffer* BatchRenderer2D::GetDefaultDrawFB(){
	return m_defaultDrawFB;
}

FrameBuffer* BatchRenderer2D::GetDefaultReadFB(){
	return m_defaultReadFB;
}

void BatchRenderer2D::BeginDrawCustomShader(Camera2D _cam, Shader *shader){

	if(m_beginDraw)
		return;

	m_beginDraw = true;
	m_customShader = true;

	m_camera = _cam;

	float winWidth = m_windowSize.x;
	float winHeight = m_windowSize.y;

	float camWidth = _cam.m_width;
	float camHeight = _cam.m_height;

	if(camWidth/camHeight < winWidth/winHeight)
		camHeight = camWidth*winHeight/winWidth;
	else
		camWidth = camHeight*winWidth/winHeight;

	m_camera.m_width = camWidth;
	m_camera.m_height = camHeight;

	shader->Use();

	m_viewMatrix = m_camera.ViewMatrix();
	shader->SetMatrix4("view", glm::value_ptr(m_viewMatrix));

	if(m_flipY)
        m_projMatrix = glm::orthoNO(-m_camera.m_width/2.f, -m_camera.m_width/2.f+m_camera.m_width, -m_camera.m_height/2.f, -m_camera.m_height/2.f+m_camera.m_height, 0.f, 100.f);
	else
		m_projMatrix = glm::orthoNO(-m_camera.m_width/2.f, -m_camera.m_width/2.f+m_camera.m_width, -m_camera.m_height/2.f+m_camera.m_height, -m_camera.m_height/2.f, 0.f, 100.f);
    
	shader->SetMatrix4("proj", glm::value_ptr(m_projMatrix));
}

void BatchRenderer2D::EndDrawCustomShader(){

	if(!m_beginDraw || !m_customShader)
		return;

	m_beginDraw = false;
	m_customShader = false;

	for (int b = 0; b < m_batchN; b++)
		m_batches[b]->SubmitDraw();
}

void BatchRenderer2D::BeginDraw(Camera2D cam){

	if(m_beginDraw)
		return;

	m_beginDraw = true;

	m_camera = cam;

	float winWidth = m_windowSize.x;
	float winHeight = m_windowSize.y;

	float camWidth = cam.m_width;
	float camHeight = cam.m_height;

	if(camWidth/camHeight < winWidth/winHeight)
		camHeight = camWidth*winHeight/winWidth;
	else
		camWidth = camHeight*winWidth/winHeight;

	m_camera.m_width = camWidth;
	m_camera.m_height = camHeight;

	m_shader->Use();

	m_viewMatrix = m_camera.ViewMatrix();
	m_shader->SetMatrix4("view", glm::value_ptr(m_viewMatrix));

	if(m_flipY)
        m_projMatrix = glm::orthoNO(-m_camera.m_width/2.f, -m_camera.m_width/2.f+m_camera.m_width, -m_camera.m_height/2.f, -m_camera.m_height/2.f+m_camera.m_height, 0.f, 100.f);
	else
		m_projMatrix = glm::orthoNO(-m_camera.m_width/2.f, -m_camera.m_width/2.f+m_camera.m_width, -m_camera.m_height/2.f+m_camera.m_height, -m_camera.m_height/2.f, 0.f, 100.f);

	m_shader->SetMatrix4("proj", glm::value_ptr(m_projMatrix));
}

void BatchRenderer2D::EndDraw(){
	
	if(!m_beginDraw || m_customShader)
		return;

	m_beginDraw = false;

	for (int b = 0; b < m_batchN; b++)
	{
		m_batches[b]->SubmitDraw();
	}
}

void BatchRenderer2D::ClearColor(glm::vec4 color){

	glClearColor(color.x, color.y, color.z, color.w);
}

void BatchRenderer2D::Clear(){

	glClear(GL_COLOR_BUFFER_BIT);
}

void BatchRenderer2D::AddTri(Vertex *tri, Texture2D *tex){

	int b;
	for(b = 0; b < m_batchN; b++){

		if(m_batches[b]->AddTri(tri, tex)){
			return;
		}
	}

	if(b >= BATCHES_PER_DRAW){

		for(int i = 0; i < m_batchN; i++){
			m_batches[i]->SubmitDraw();
		}
		b = 0;
		m_batchN = 0;
	}

	m_batchN += 1;
	m_batches[b]->AddTri(tri, tex);
}

void BatchRenderer2D::AddQuad(Vertex *quad, Texture2D *tex){

	int b;
	for(b = 0; b < m_batchN; b++){

		if(m_batches[b]->AddQuad(quad, tex)){
			return;
		}
	}

	if(b >= BATCHES_PER_DRAW){

		for(int i = 0; i < m_batchN; i++){
			m_batches[i]->SubmitDraw();
		}
		b = 0;
		m_batchN = 0;
	}

	m_batchN += 1;
	m_batches[b]->AddQuad(quad, tex);
}

void BatchRenderer2D::DrawTriangle(glm::vec2 v1, glm::vec2 v2, glm::vec2 v3, glm::u8vec4 color){

	Vertex tri[3];
	tri[0].x = v1.x;
	tri[0].y = v1.y;

	tri[1].x = v2.x;
	tri[1].y = v2.y;

	tri[2].x = v3.x;
	tri[2].y = v3.y;

	for(int i = 0; i < 3; i++){

		tri[i].r = color.r/255.f;
		tri[i].g = color.g/255.f;
		tri[i].b = color.b/255.f;
		tri[i].a = color.a/255.f;

		tri[i].s = 0;
		tri[i].t = 0;
	}

	AddTri(tri, nullptr);
}

void BatchRenderer2D::DrawTriangleStrip(glm::vec2 *points, int pointCount, glm::u8vec4 color){

	if (pointCount >= 3)
	{
		Vertex tri[3];
		for(int t = 0; t < 3; t++){
			tri[t].r = color.x/255.f;
			tri[t].g = color.y/255.f;
			tri[t].b = color.z/255.f;
			tri[t].a = color.w/255.f;
			tri[t].s = 0;
			tri[t].t = 0;
		}
		for (int i = 2; i < pointCount; i++)
		{
		    if ((i%2) == 0)
		    {
		    	tri[0].x = points[i].x;
		    	tri[0].y = points[i].y;
		    	tri[1].x = points[i-2].x;
		    	tri[1].y = points[i-2].y;
		    	tri[2].x = points[i-1].x;
		    	tri[2].y = points[i-1].y;
		    }
		    else
		    {
		    	tri[0].x = points[i].x;
		    	tri[0].y = points[i].y;
		    	tri[1].x = points[i-1].x;
		    	tri[1].y = points[i-1].y;
		    	tri[2].x = points[i-2].x;
		    	tri[2].y = points[i-2].y;
		    }

		    AddTri(tri, nullptr);
		}
	}
}

void BatchRenderer2D::DrawTriangleFan(glm::vec2 *points, int pointCount, glm::u8vec4 color){

	if (pointCount >= 3)
    {
        Vertex quad[4];
		for(int i = 0; i < 4; i++){
			quad[i].r = color.r/255.f;
			quad[i].g = color.g/255.f;
			quad[i].b = color.b/255.f;
			quad[i].a = color.a/255.f;
			quad[i].s = 0;
			quad[i].t = 0;
		}

        for (int i = 1; i < pointCount - 1; i++)
        {	
			quad[0].x = points[0].x;
			quad[0].y = points[0].y;

			quad[1].x = points[i].x;
			quad[1].y = points[i].y;

			quad[2].x = points[i + 1].x;
			quad[2].y = points[i + 1].y;

			quad[3].x = points[i + 1].x;
			quad[3].y = points[i + 1].y;

			AddQuad(quad, nullptr);
        }
    }
}

void BatchRenderer2D::DrawPixel(glm::vec2 position, glm::u8vec4 color){

	Vertex quad[4];
	quad[0].x = position.x;
	quad[0].y = position.y;

	quad[1].x = position.x+1;
	quad[1].y = position.y;

	quad[2].x = position.x+1;
	quad[2].y = position.y+1;

	quad[3].x = position.x;
	quad[3].y = position.y+1;

	for(int i = 0; i < 4; i++){

		quad[i].r = color.r/255.f;
		quad[i].g = color.g/255.f;
		quad[i].b = color.b/255.f;
		quad[i].a = color.a/255.f;

		quad[i].s = 0;
		quad[i].t = 0;
	}

	AddQuad(quad, nullptr);
}

void BatchRenderer2D::DrawLine(glm::vec2 startPos, glm::vec2 endPos, float thick, glm::u8vec4 color){

	glm::vec2 delta = { endPos.x - startPos.x, endPos.y - startPos.y };
    float length = sqrtf(delta.x*delta.x + delta.y*delta.y);

    if ((length > 0) && (thick > 0))
    {
        float scale = thick/(2*length);

        glm::vec2 radius = { -scale*delta.y, scale*delta.x };
        glm::vec2 strip[4] = {
            { startPos.x - radius.x, startPos.y - radius.y },
            { startPos.x + radius.x, startPos.y + radius.y },
            { endPos.x - radius.x, endPos.y - radius.y },
            { endPos.x + radius.x, endPos.y + radius.y }
        };

        DrawTriangleStrip(strip, 4, color);
    }
}

// Cubic easing in-out
// NOTE: Used by DrawLineBezier() only
static float EaseCubicInOut(float t, float b, float c, float d)
{
    if ((t /= 0.5f*d) < 1) return 0.5f*c*t*t*t + b;

    t -= 2;

    return 0.5f*c*(t*t*t + 2.0f) + b;
}

void BatchRenderer2D::DrawLineBezier(glm::vec2 startPos, glm::vec2 endPos, int segments, float thick, glm::u8vec4 color){

	if(segments < 4)
		segments = 4;

	glm::vec2 previous = startPos;
    glm::vec2 current;

    glm::vec2 points[2*segments + 2];

    for (int i = 1; i <= segments; i++)
    {
        // Cubic easing in-out
        // NOTE: Easing is calculated only for y position value
        current.y = EaseCubicInOut((float)i, startPos.y, endPos.y - startPos.y, (float)segments);
        current.x = previous.x + (endPos.x - startPos.x)/(float)segments;

        float dy = current.y - previous.y;
        float dx = current.x - previous.x;
        float size = 0.5f*thick/sqrtf(dx*dx+dy*dy);

        if (i == 1)
        {
            points[0].x = previous.x + dy*size;
            points[0].y = previous.y - dx*size;
            points[1].x = previous.x - dy*size;
            points[1].y = previous.y + dx*size;
        }

        points[2*i + 1].x = current.x - dy*size;
        points[2*i + 1].y = current.y + dx*size;
        points[2*i].x = current.x + dy*size;
        points[2*i].y = current.y - dx*size;

        previous = current;
    }

    DrawTriangleStrip(points, 2*segments + 2, color);
}

void BatchRenderer2D::DrawRectangle(glm::vec4 rec, glm::vec2 origin, float rotation, glm::u8vec4 color){

	glm::vec2 topLeft;
    glm::vec2 topRight;
    glm::vec2 bottomLeft;
    glm::vec2 bottomRight;

	float width = rec.z;
	float height = rec.w;
	
    // Only calculate rotation if needed
    if (rotation == 0.0f)
    {
        float x = rec.x - origin.x;
        float y = rec.y - origin.y;
        topLeft = (glm::vec2){ x, y };
        topRight = (glm::vec2){ x + width, y };
        bottomLeft = (glm::vec2){ x, y + height };
        bottomRight = (glm::vec2){ x + width, y + height };
    }
    else
    {
        float sinRotation = sinf(glm::radians(rotation));
        float cosRotation = cosf(glm::radians(rotation));
        float x = rec.x;
        float y = rec.y;
        float dx = -origin.x;
        float dy = -origin.y;

        topLeft.x = x + dx*cosRotation - dy*sinRotation;
        topLeft.y = y + dx*sinRotation + dy*cosRotation;

        topRight.x = x + (dx + width)*cosRotation - dy*sinRotation;
        topRight.y = y + (dx + width)*sinRotation + dy*cosRotation;

        bottomLeft.x = x + dx*cosRotation - (dy + height)*sinRotation;
        bottomLeft.y = y + dx*sinRotation + (dy + height)*cosRotation;

        bottomRight.x = x + (dx + width)*cosRotation - (dy + height)*sinRotation;
        bottomRight.y = y + (dx + width)*sinRotation + (dy + height)*cosRotation;
    }

	Vertex quad[4];
	for(int i = 0; i < 4; i++){
		quad[i].r = color.r/255.f;
		quad[i].g = color.g/255.f;
		quad[i].b = color.b/255.f;
		quad[i].a = color.a/255.f;
		quad[i].s = 0;
		quad[i].t = 0;
	}

	quad[0].x = topLeft.x;
	quad[0].y = topLeft.y;

	quad[1].x = bottomLeft.x;
	quad[1].y = bottomLeft.y;

	quad[2].x = bottomRight.x;
	quad[2].y = bottomRight.y;

	quad[3].x = topRight.x;
	quad[3].y = topRight.y;

    AddQuad(quad, nullptr);
}

void BatchRenderer2D::DrawPoly(glm::vec2 center, int sides, float radius, float rotation, glm::u8vec4 color){

	if (sides < 3) sides = 3;
    float centralAngle = glm::radians(rotation);
    float angleStep = glm::radians(360.0f/(float)sides);

	Vertex tri[3];
	for(int i = 0; i < 3; i++){
		tri[i].r = color.r/255.f;
		tri[i].g = color.g/255.f;
		tri[i].b = color.b/255.f;
		tri[i].a = color.a/255.f;
		tri[i].s = 0;
		tri[i].t = 0;
	}
	
    for (int i = 0; i < sides; i++)
    {		
		tri[0].x = center.x;
		tri[0].y = center.y;

		tri[1].x = center.x + cosf(centralAngle + angleStep)*radius;
		tri[1].y = center.y + sinf(centralAngle + angleStep)*radius;

		tri[2].x = center.x + cosf(centralAngle)*radius;
		tri[2].y = center.y + sinf(centralAngle)*radius;

		AddTri(tri, nullptr);
        centralAngle += angleStep;
    }
}

void BatchRenderer2D::DrawPolyLines(glm::vec2 center, int sides, float radius, float rotation, float lineThick, glm::u8vec4 color){

	if (sides < 3) sides = 3;
    float centralAngle = glm::radians(rotation);
    float exteriorAngle = glm::radians(360.0f/(float)sides);
    float innerRadius = radius - (lineThick*cosf(glm::radians(exteriorAngle/2.0f)));

	Vertex quad[4];
	for(int i = 0; i < 4; i++){
		quad[i].r = color.r/255.f;
		quad[i].g = color.g/255.f;
		quad[i].b = color.b/255.f;
		quad[i].a = color.a/255.f;
		quad[i].s = 0;
		quad[i].t = 0;
	}

    for (int i = 0; i < sides; i++)
    {

        float nextAngle = centralAngle + exteriorAngle;

		quad[0].x = center.x + cosf(centralAngle)*radius;
		quad[0].y = center.y + sinf(centralAngle)*radius;

		quad[1].x = center.x + cosf(centralAngle)*innerRadius;
		quad[1].y = center.y + sinf(centralAngle)*innerRadius;

		quad[2].x = center.x + cosf(nextAngle)*innerRadius;
		quad[2].y = center.y + sinf(nextAngle)*innerRadius;

		quad[3].x = center.x + cosf(nextAngle)*radius;
		quad[3].y = center.y + sinf(nextAngle)*radius;

		AddQuad(quad, nullptr);

        centralAngle = nextAngle;
    }
}

void BatchRenderer2D::DrawCircleSector(glm::vec2 center, float radius, float startAngle, float endAngle, int segments, glm::u8vec4 color){

	if (radius <= 0.0f) radius = 0.1f;  // Avoid div by zero

    // Function expects (endAngle > startAngle)
    if (endAngle < startAngle)
    {
        // Swap values
        float tmp = startAngle;
        startAngle = endAngle;
        endAngle = tmp;
    }

    int minSegments = (int)ceilf((endAngle - startAngle)/90);

    if (segments < minSegments)
    {
        // Calculate the maximum angle between segments based on the error rate (usually 0.5f)
        float th = acosf(2*powf(1 - 0.5f/radius, 2) - 1);
        segments = (int)((endAngle - startAngle)*ceilf(2*glm::pi<double>()/th)/360);

        if (segments <= 0) segments = minSegments;
    }

    float stepLength = (endAngle - startAngle)/(float)segments;
    float angle = startAngle;

	Vertex tri[3];
	for(int i = 0; i < 3; i++){
		tri[i].r = color.r/255.f;
		tri[i].g = color.g/255.f;
		tri[i].b = color.b/255.f;
		tri[i].a = color.a/255.f;
		tri[i].s = 0;
		tri[i].t = 0;
	}

    for (int i = 0; i < segments; i++)
    {

		tri[0].x = center.x;
		tri[0].y = center.y;

		tri[1].x = center.x + cosf(glm::radians(angle + stepLength))*radius;
		tri[1].y = center.y + sinf(glm::radians(angle + stepLength))*radius;

		tri[2].x = center.x + cosf(glm::radians(angle))*radius;
		tri[2].y = center.y + sinf(glm::radians(angle))*radius;

		AddTri(tri, nullptr);

        angle += stepLength;
    }
}

void BatchRenderer2D::DrawCircle(glm::vec2 center, float radius, glm::u8vec4 color){

	DrawCircleSector(center, radius, 0, 360, 36, color);
}

void BatchRenderer2D::DrawRing(glm::vec2 center, float innerRadius, float outerRadius, float startAngle, float endAngle, int segments, glm::u8vec4 color){

	if (startAngle == endAngle) return;

    // Function expects (outerRadius > innerRadius)
    if (outerRadius < innerRadius)
    {
        float tmp = outerRadius;
        outerRadius = innerRadius;
        innerRadius = tmp;

        if (outerRadius <= 0.0f) outerRadius = 0.1f;
    }

    // Function expects (endAngle > startAngle)
    if (endAngle < startAngle)
    {
        // Swap values
        float tmp = startAngle;
        startAngle = endAngle;
        endAngle = tmp;
    }

    int minSegments = (int)ceilf((endAngle - startAngle)/90);

    if (segments < minSegments)
    {
        // Calculate the maximum angle between segments based on the error rate (usually 0.5f)
        float th = acosf(2*powf(1 - .5f/outerRadius, 2) - 1);
        segments = (int)((endAngle - startAngle)*ceilf(2*glm::pi<double>()/th)/360);

        if (segments <= 0) segments = minSegments;
    }

    // Not a ring
    if (innerRadius <= 0.0f)
    {
        DrawCircleSector(center, outerRadius, startAngle, endAngle, segments, color);
        return;
    }

    float stepLength = (endAngle - startAngle)/(float)segments;
    float angle = startAngle;

	Vertex quad[4];
	for(int i = 0; i < 4; i++){
		quad[i].r = color.r/255.f;
		quad[i].g = color.g/255.f;
		quad[i].b = color.b/255.f;
		quad[i].a = color.a/255.f;
		quad[i].s = 0;
		quad[i].t = 0;
	}

    for (int i = 0; i < segments; i++)
    {
		
		quad[0].x = center.x + cosf(glm::radians(angle))*outerRadius;
		quad[0].y = center.y + sinf(glm::radians(angle))*outerRadius;

		quad[1].x = center.x + cosf(glm::radians(angle))*innerRadius;
		quad[1].y = center.y + sinf(glm::radians(angle))*innerRadius;

		quad[2].x = center.x + cosf(glm::radians(angle + stepLength))*innerRadius;
		quad[2].y = center.y + sinf(glm::radians(angle + stepLength))*innerRadius;
		
		quad[3].x = center.x + cosf(glm::radians(angle + stepLength))*outerRadius;
		quad[3].y = center.y + sinf(glm::radians(angle + stepLength))*outerRadius;

		AddQuad(quad, nullptr);

        angle += stepLength;
    }
}

void BatchRenderer2D::DrawCircleLines(glm::vec2 center, float radius, float lineThick, glm::u8vec4 color){

	DrawRing(center, radius-lineThick/2.f, radius+lineThick/2.f, 0, 360, 36, color);
}

void BatchRenderer2D::DrawConvexPolygon(glm::vec2 *points, int pointCount, glm::vec2 origin, float rotation, glm::u8vec4 color){

    glm::vec2 *_points = new glm::vec2[pointCount+2];
    glm::vec2 sum{0, 0};

    glm::mat4 trans(1.f);
    trans = glm::rotate(trans, glm::radians(rotation), glm::vec3(0.f, 0.f, 1.f));
    trans = glm::translate(trans, {origin.x, origin.y, 0.f});

    for(int i = 1; i <= pointCount; i++){

        glm::vec4 point(points[i-1], 0.f, 0.f);
        _points[i] = point*trans;
        sum += _points[i];
    }
    _points[0] = sum*(1.f/pointCount);
    _points[pointCount+1] = _points[1];
    DrawTriangleFan(_points, pointCount+2, color);
}

#ifndef SMOOTH_CIRCLE_ERROR_RATE
    #define SMOOTH_CIRCLE_ERROR_RATE    0.5f      // Circle error rate
#endif
#ifndef SPLINE_SEGMENT_DIVISIONS
    #define SPLINE_SEGMENT_DIVISIONS      24      // Spline segment divisions
#endif

// Draw spline: linear, minimum 2 points
void BatchRenderer2D::DrawSplineLinear(glm::vec2 *points, int pointCount, float thick, glm::u8vec4 color)
{
    glm::vec2 delta;
    float length = 0.0f;
    float scale = 0.0f;
    
    for (int i = 0; i < pointCount - 1; i++)
    {
        delta = (glm::vec2){ points[i + 1].x - points[i].x, points[i + 1].y - points[i].y };
        length = sqrtf(delta.x*delta.x + delta.y*delta.y);

        if (length > 0) scale = thick/(2*length);

        glm::vec2 radius = { -scale*delta.y, scale*delta.x };
        glm::vec2 strip[4] = {
            { points[i].x - radius.x, points[i].y - radius.y },
            { points[i].x + radius.x, points[i].y + radius.y },
            { points[i + 1].x - radius.x, points[i + 1].y - radius.y },
            { points[i + 1].x + radius.x, points[i + 1].y + radius.y }
        };

        DrawTriangleStrip(strip, 4, color);
    }
}

// Draw spline: B-Spline, minimum 4 points
void BatchRenderer2D::DrawSplineBasis(glm::vec2 *points, int pointCount, float thick, glm::u8vec4 color)
{
    if (pointCount < 4) return;

    float a[4] = { 0 };
    float b[4] = { 0 };
    float dy = 0.0f;
    float dx = 0.0f;
    float size = 0.0f;

    glm::vec2 currentPoint;
    glm::vec2 nextPoint;
    glm::vec2 vertices[2*SPLINE_SEGMENT_DIVISIONS + 2];

    for (int i = 0; i < (pointCount - 3); i++)
    {
        float t = 0.0f;
        glm::vec2 p1 = points[i], p2 = points[i + 1], p3 = points[i + 2], p4 = points[i + 3];

        a[0] = (-p1.x + 3.0f*p2.x - 3.0f*p3.x + p4.x)/6.0f;
        a[1] = (3.0f*p1.x - 6.0f*p2.x + 3.0f*p3.x)/6.0f;
        a[2] = (-3.0f*p1.x + 3.0f*p3.x)/6.0f;
        a[3] = (p1.x + 4.0f*p2.x + p3.x)/6.0f;

        b[0] = (-p1.y + 3.0f*p2.y - 3.0f*p3.y + p4.y)/6.0f;
        b[1] = (3.0f*p1.y - 6.0f*p2.y + 3.0f*p3.y)/6.0f;
        b[2] = (-3.0f*p1.y + 3.0f*p3.y)/6.0f;
        b[3] = (p1.y + 4.0f*p2.y + p3.y)/6.0f;

        currentPoint.x = a[3];
        currentPoint.y = b[3];

        if (i == 0) DrawCircle(currentPoint, thick/2.0f, color);   // Draw init line circle-cap

        if (i > 0)
        {
            vertices[0].x = currentPoint.x + dy*size;
            vertices[0].y = currentPoint.y - dx*size;
            vertices[1].x = currentPoint.x - dy*size;
            vertices[1].y = currentPoint.y + dx*size;
        }

        for (int j = 1; j <= SPLINE_SEGMENT_DIVISIONS; j++)
        {
            t = ((float)j)/((float)SPLINE_SEGMENT_DIVISIONS);

            nextPoint.x = a[3] + t*(a[2] + t*(a[1] + t*a[0]));
            nextPoint.y = b[3] + t*(b[2] + t*(b[1] + t*b[0]));

            dy = nextPoint.y - currentPoint.y;
            dx = nextPoint.x - currentPoint.x;
            size = 0.5f*thick/sqrtf(dx*dx+dy*dy);

            if ((i == 0) && (j == 1))
            {
                vertices[0].x = currentPoint.x + dy*size;
                vertices[0].y = currentPoint.y - dx*size;
                vertices[1].x = currentPoint.x - dy*size;
                vertices[1].y = currentPoint.y + dx*size;
            }

            vertices[2*j + 1].x = nextPoint.x - dy*size;
            vertices[2*j + 1].y = nextPoint.y + dx*size;
            vertices[2*j].x = nextPoint.x + dy*size;
            vertices[2*j].y = nextPoint.y - dx*size;

            currentPoint = nextPoint;
        }

        DrawTriangleStrip(vertices, 2*SPLINE_SEGMENT_DIVISIONS + 2, color);
    }

    DrawCircle(currentPoint, thick/2.0f, color);   // Draw end line circle-cap
}

// Draw spline: Catmull-Rom, minimum 4 points
void BatchRenderer2D::DrawSplineCatmullRom(glm::vec2 *points, int pointCount, float thick, glm::u8vec4 color)
{
    if (pointCount < 4) return;

    float dy = 0.0f;
    float dx = 0.0f;
    float size = 0.0f;

    glm::vec2 currentPoint = points[1];
    glm::vec2 nextPoint;
    glm::vec2 vertices[2*SPLINE_SEGMENT_DIVISIONS + 2];

    DrawCircle(currentPoint, thick/2.0f, color);   // Draw init line circle-cap

    for (int i = 0; i < (pointCount - 3); i++)
    {
        float t = 0.0f;
        glm::vec2 p1 = points[i], p2 = points[i + 1], p3 = points[i + 2], p4 = points[i + 3];

        if (i > 0)
        {
            vertices[0].x = currentPoint.x + dy*size;
            vertices[0].y = currentPoint.y - dx*size;
            vertices[1].x = currentPoint.x - dy*size;
            vertices[1].y = currentPoint.y + dx*size;
        }

        for (int j = 1; j <= SPLINE_SEGMENT_DIVISIONS; j++)
        {
            t = ((float)j)/((float)SPLINE_SEGMENT_DIVISIONS);

            float q0 = (-1.0f*t*t*t) + (2.0f*t*t) + (-1.0f*t);
            float q1 = (3.0f*t*t*t) + (-5.0f*t*t) + 2.0f;
            float q2 = (-3.0f*t*t*t) + (4.0f*t*t) + t;
            float q3 = t*t*t - t*t;

            nextPoint.x = 0.5f*((p1.x*q0) + (p2.x*q1) + (p3.x*q2) + (p4.x*q3));
            nextPoint.y = 0.5f*((p1.y*q0) + (p2.y*q1) + (p3.y*q2) + (p4.y*q3));

            dy = nextPoint.y - currentPoint.y;
            dx = nextPoint.x - currentPoint.x;
            size = (0.5f*thick)/sqrtf(dx*dx + dy*dy);

            if ((i == 0) && (j == 1))
            {
                vertices[0].x = currentPoint.x + dy*size;
                vertices[0].y = currentPoint.y - dx*size;
                vertices[1].x = currentPoint.x - dy*size;
                vertices[1].y = currentPoint.y + dx*size;
            }

            vertices[2*j + 1].x = nextPoint.x - dy*size;
            vertices[2*j + 1].y = nextPoint.y + dx*size;
            vertices[2*j].x = nextPoint.x + dy*size;
            vertices[2*j].y = nextPoint.y - dx*size;

            currentPoint = nextPoint;
        }

        DrawTriangleStrip(vertices, 2*SPLINE_SEGMENT_DIVISIONS + 2, color);
    }

    DrawCircle(currentPoint, thick/2.0f, color);   // Draw end line circle-cap
}

// Draw spline: Quadratic Bezier, minimum 3 points (1 control point): [p1, c2, p3, c4...]
void BatchRenderer2D::DrawSplineBezierQuadratic(glm::vec2 *points, int pointCount, float thick, glm::u8vec4 color)
{
    if (pointCount < 3) return;
    
    for (int i = 0; i < pointCount - 2; i++)
    {
        DrawSplineSegmentBezierQuadratic(points[i], points[i + 1], points[i + 2], thick, color);
    }
}

// Draw spline: Cubic Bezier, minimum 4 points (2 control points): [p1, c2, c3, p4, c5, c6...]
void BatchRenderer2D::DrawSplineBezierCubic(glm::vec2 *points, int pointCount, float thick, glm::u8vec4 color)
{
    if (pointCount < 4) return;
    
    for (int i = 0; i < pointCount - 3; i++)
    {
        DrawSplineSegmentBezierCubic(points[i], points[i + 1], points[i + 2], points[i + 3], thick, color);
    }
}

// Draw spline segment: Linear, 2 points
void BatchRenderer2D::DrawSplineSegmentLinear(glm::vec2 p1, glm::vec2 p2, float thick, glm::u8vec4 color)
{
    // NOTE: For the linear spline we don't use subdivisions, just a single quad
    
    glm::vec2 delta = { p2.x - p1.x, p2.y - p1.y };
    float length = sqrtf(delta.x*delta.x + delta.y*delta.y);

    if ((length > 0) && (thick > 0))
    {
        float scale = thick/(2*length);

        glm::vec2 radius = { -scale*delta.y, scale*delta.x };
        glm::vec2 strip[4] = {
            { p1.x - radius.x, p1.y - radius.y },
            { p1.x + radius.x, p1.y + radius.y },
            { p2.x - radius.x, p2.y - radius.y },
            { p2.x + radius.x, p2.y + radius.y }
        };

        DrawTriangleStrip(strip, 4, color);
    }
}

// Draw spline segment: B-Spline, 4 points
void BatchRenderer2D::DrawSplineSegmentBasis(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 p4, float thick, glm::u8vec4 color)
{
    const float step = 1.0f/SPLINE_SEGMENT_DIVISIONS;

    glm::vec2 currentPoint;
    glm::vec2 nextPoint;
    float t = 0.0f;
    
    glm::vec2 points[2*SPLINE_SEGMENT_DIVISIONS + 2];
    
    float a[4] = { 0 };
    float b[4] = { 0 };

    a[0] = (-p1.x + 3*p2.x - 3*p3.x + p4.x)/6.0f;
    a[1] = (3*p1.x - 6*p2.x + 3*p3.x)/6.0f;
    a[2] = (-3*p1.x + 3*p3.x)/6.0f;
    a[3] = (p1.x + 4*p2.x + p3.x)/6.0f;

    b[0] = (-p1.y + 3*p2.y - 3*p3.y + p4.y)/6.0f;
    b[1] = (3*p1.y - 6*p2.y + 3*p3.y)/6.0f;
    b[2] = (-3*p1.y + 3*p3.y)/6.0f;
    b[3] = (p1.y + 4*p2.y + p3.y)/6.0f;

    currentPoint.x = a[3];
    currentPoint.y = b[3];

    for (int i = 0; i <= SPLINE_SEGMENT_DIVISIONS; i++)
    {
        t = step*(float)i;

        nextPoint.x = a[3] + t*(a[2] + t*(a[1] + t*a[0]));
        nextPoint.y = b[3] + t*(b[2] + t*(b[1] + t*b[0]));

        float dy = nextPoint.y - currentPoint.y;
        float dx = nextPoint.x - currentPoint.x;
        float size = (0.5f*thick)/sqrtf(dx*dx + dy*dy);

        if (i == 1)
        {
            points[0].x = currentPoint.x + dy*size;
            points[0].y = currentPoint.y - dx*size;
            points[1].x = currentPoint.x - dy*size;
            points[1].y = currentPoint.y + dx*size;
        }

        points[2*i + 1].x = nextPoint.x - dy*size;
        points[2*i + 1].y = nextPoint.y + dx*size;
        points[2*i].x = nextPoint.x + dy*size;
        points[2*i].y = nextPoint.y - dx*size;

        currentPoint = nextPoint;
    }

    DrawTriangleStrip(points, 2*SPLINE_SEGMENT_DIVISIONS+2, color);
}

// Draw spline segment: Catmull-Rom, 4 points
void BatchRenderer2D::DrawSplineSegmentCatmullRom(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 p4, float thick, glm::u8vec4 color)
{
    const float step = 1.0f/SPLINE_SEGMENT_DIVISIONS;

    glm::vec2 currentPoint = p1;
    glm::vec2 nextPoint;
    float t = 0.0f;
    
    glm::vec2 points[2*SPLINE_SEGMENT_DIVISIONS + 2];

    for (int i = 0; i <= SPLINE_SEGMENT_DIVISIONS; i++)
    {
        t = step*(float)i;

        float q0 = (-1*t*t*t) + (2*t*t) + (-1*t);
        float q1 = (3*t*t*t) + (-5*t*t) + 2;
        float q2 = (-3*t*t*t) + (4*t*t) + t;
        float q3 = t*t*t - t*t;

        nextPoint.x = 0.5f*((p1.x*q0) + (p2.x*q1) + (p3.x*q2) + (p4.x*q3));
        nextPoint.y = 0.5f*((p1.y*q0) + (p2.y*q1) + (p3.y*q2) + (p4.y*q3));

        float dy = nextPoint.y - currentPoint.y;
        float dx = nextPoint.x - currentPoint.x;
        float size = (0.5f*thick)/sqrtf(dx*dx + dy*dy);

        if (i == 1)
        {
            points[0].x = currentPoint.x + dy*size;
            points[0].y = currentPoint.y - dx*size;
            points[1].x = currentPoint.x - dy*size;
            points[1].y = currentPoint.y + dx*size;
        }

        points[2*i + 1].x = nextPoint.x - dy*size;
        points[2*i + 1].y = nextPoint.y + dx*size;
        points[2*i].x = nextPoint.x + dy*size;
        points[2*i].y = nextPoint.y - dx*size;

        currentPoint = nextPoint;
    }

    DrawTriangleStrip(points, 2*SPLINE_SEGMENT_DIVISIONS + 2, color);
}

// Draw spline segment: Quadratic Bezier, 2 points, 1 control point
void BatchRenderer2D::DrawSplineSegmentBezierQuadratic(glm::vec2 p1, glm::vec2 c2, glm::vec2 p3, float thick, glm::u8vec4 color)
{
    const float step = 1.0f/SPLINE_SEGMENT_DIVISIONS;

    glm::vec2 previous = p1;
    glm::vec2 current;
    float t = 0.0f;

    glm::vec2 points[2*SPLINE_SEGMENT_DIVISIONS + 2];

    for (int i = 1; i <= SPLINE_SEGMENT_DIVISIONS; i++)
    {
        t = step*(float)i;

        float a = powf(1.0f - t, 2);
        float b = 2.0f*(1.0f - t)*t;
        float c = powf(t, 2);

        // NOTE: The easing functions aren't suitable here because they don't take a control point
        current.y = a*p1.y + b*c2.y + c*p3.y;
        current.x = a*p1.x + b*c2.x + c*p3.x;

        float dy = current.y - previous.y;
        float dx = current.x - previous.x;
        float size = 0.5f*thick/sqrtf(dx*dx+dy*dy);

        if (i == 1)
        {
            points[0].x = previous.x + dy*size;
            points[0].y = previous.y - dx*size;
            points[1].x = previous.x - dy*size;
            points[1].y = previous.y + dx*size;
        }

        points[2*i + 1].x = current.x - dy*size;
        points[2*i + 1].y = current.y + dx*size;
        points[2*i].x = current.x + dy*size;
        points[2*i].y = current.y - dx*size;

        previous = current;
    }

    DrawTriangleStrip(points, 2*SPLINE_SEGMENT_DIVISIONS + 2, color);
}

// Draw spline segment: Cubic Bezier, 2 points, 2 control points
void BatchRenderer2D::DrawSplineSegmentBezierCubic(glm::vec2 p1, glm::vec2 c2, glm::vec2 c3, glm::vec2 p4, float thick, glm::u8vec4 color)
{
    const float step = 1.0f/SPLINE_SEGMENT_DIVISIONS;

    glm::vec2 previous = p1;
    glm::vec2 current;
    float t = 0.0f;

    glm::vec2 points[2*SPLINE_SEGMENT_DIVISIONS + 2];

    for (int i = 1; i <= SPLINE_SEGMENT_DIVISIONS; i++)
    {
        t = step*(float)i;

        float a = powf(1.0f - t, 3);
        float b = 3.0f*powf(1.0f - t, 2)*t;
        float c = 3.0f*(1.0f - t)*powf(t, 2);
        float d = powf(t, 3);

        current.y = a*p1.y + b*c2.y + c*c3.y + d*p4.y;
        current.x = a*p1.x + b*c2.x + c*c3.x + d*p4.x;

        float dy = current.y - previous.y;
        float dx = current.x - previous.x;
        float size = 0.5f*thick/sqrtf(dx*dx+dy*dy);

        if (i == 1)
        {
            points[0].x = previous.x + dy*size;
            points[0].y = previous.y - dx*size;
            points[1].x = previous.x - dy*size;
            points[1].y = previous.y + dx*size;
        }

        points[2*i + 1].x = current.x - dy*size;
        points[2*i + 1].y = current.y + dx*size;
        points[2*i].x = current.x + dy*size;
        points[2*i].y = current.y - dx*size;

        previous = current;
    }

    DrawTriangleStrip(points, 2*SPLINE_SEGMENT_DIVISIONS + 2, color);
}

// // Get spline point for a given t [0.0f .. 1.0f], Linear
// glm::vec2 GetSplinePointLinear(glm::vec2 startPos, glm::vec2 endPos, float t)
// {
//     glm::vec2 point;

//     point.x = startPos.x*(1.0f - t) + endPos.x*t;
//     point.y = startPos.y*(1.0f - t) + endPos.y*t;

//     return point;
// }

// // Get spline point for a given t [0.0f .. 1.0f], B-Spline
// glm::vec2 GetSplinePointBasis(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 p4, float t)
// {
//     glm::vec2 point;

//     float a[4] = { 0 };
//     float b[4] = { 0 };

//     a[0] = (-p1.x + 3*p2.x - 3*p3.x + p4.x)/6.0f;
//     a[1] = (3*p1.x - 6*p2.x + 3*p3.x)/6.0f;
//     a[2] = (-3*p1.x + 3*p3.x)/6.0f;
//     a[3] = (p1.x + 4*p2.x + p3.x)/6.0f;

//     b[0] = (-p1.y + 3*p2.y - 3*p3.y + p4.y)/6.0f;
//     b[1] = (3*p1.y - 6*p2.y + 3*p3.y)/6.0f;
//     b[2] = (-3*p1.y + 3*p3.y)/6.0f;
//     b[3] = (p1.y + 4*p2.y + p3.y)/6.0f;

//     point.x = a[3] + t*(a[2] + t*(a[1] + t*a[0]));
//     point.y = b[3] + t*(b[2] + t*(b[1] + t*b[0]));

//     return point;
// }

// // Get spline point for a given t [0.0f .. 1.0f], Catmull-Rom
// glm::vec2 GetSplinePointCatmullRom(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 p4, float t)
// {
//     glm::vec2 point;

//     float q0 = (-1*t*t*t) + (2*t*t) + (-1*t);
//     float q1 = (3*t*t*t) + (-5*t*t) + 2;
//     float q2 = (-3*t*t*t) + (4*t*t) + t;
//     float q3 = t*t*t - t*t;

//     point.x = 0.5f*((p1.x*q0) + (p2.x*q1) + (p3.x*q2) + (p4.x*q3));
//     point.y = 0.5f*((p1.y*q0) + (p2.y*q1) + (p3.y*q2) + (p4.y*q3));

//     return point;
// }

// // Get spline point for a given t [0.0f .. 1.0f], Quadratic Bezier
// glm::vec2 GetSplinePointBezierQuad(glm::vec2 startPos, glm::vec2 controlPos, glm::vec2 endPos, float t)
// {
//     glm::vec2 point;

//     float a = powf(1.0f - t, 2);
//     float b = 2.0f*(1.0f - t)*t;
//     float c = powf(t, 2);

//     point.y = a*startPos.y + b*controlPos.y + c*endPos.y;
//     point.x = a*startPos.x + b*controlPos.x + c*endPos.x;

//     return point;
// }

// // Get spline point for a given t [0.0f .. 1.0f], Cubic Bezier
// glm::vec2 GetSplinePointBezierCubic(glm::vec2 startPos, glm::vec2 startControlPos, glm::vec2 endControlPos, glm::vec2 endPos, float t)
// {
//     glm::vec2 point;

//     float a = powf(1.0f - t, 3);
//     float b = 3.0f*powf(1.0f - t, 2)*t;
//     float c = 3.0f*(1.0f - t)*powf(t, 2);
//     float d = powf(t, 3);

//     point.y = a*startPos.y + b*startControlPos.y + c*endControlPos.y + d*endPos.y;
//     point.x = a*startPos.x + b*startControlPos.x + c*endControlPos.x + d*endPos.x;

//     return point;
// }

void BatchRenderer2D::DrawTexture2D(Texture2D *texture, glm::ivec4 map, glm::vec4 proj, glm::vec2 origin, float rotation, glm::u8vec4 color){

	glm::vec2 quad[4];
	quad[0] = {-origin.x       , -origin.y};
	quad[1] = {-origin.x+proj.z, -origin.y};
	quad[2] = {-origin.x+proj.z, -origin.y+proj.w};
	quad[3] = {-origin.x       , -origin.y+proj.w};

	for(int i = 0; i < 4; i++){

		glm::mat4 trans(1.0f);
		trans = glm::rotate(trans, glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));
		trans = glm::translate(trans, glm::vec3(proj.x, proj.y, 0.0f));
		quad[i] = trans*glm::vec4(quad[i], 1.f, 1.f);
	}

	Vertex quadVert[4];
	for(int i = 0; i < 4; i++){
		quadVert[i].x = quad[i].x;
		quadVert[i].y = quad[i].y;
		quadVert[i].r = color.r/255.f;
		quadVert[i].g = color.g/255.f;
		quadVert[i].b = color.b/255.f;
		quadVert[i].a = color.a/255.f;
	}

	// TEXTURE CO-ORDINATES
	quad[0] = { map.x/texture->m_width        ,  map.y/texture->m_height};
	quad[1] = {(map.x+map.z)/texture->m_width ,  map.y/texture->m_height};
	quad[2] = {(map.x+map.z)/texture->m_width , (map.y+map.w)/texture->m_height};
	quad[3] = { map.x/texture->m_width        , (map.y+map.w)/texture->m_height};

	for(int i = 0; i < 4; i++){
		quadVert[i].s = quad[i].x;
		quadVert[i].t = quad[i].y;
	}

	AddQuad(quadVert, texture);
}

void BatchRenderer2D::DrawText(Font *font, const char16_t *str, glm::vec2 posBL, glm::vec2 origin, float rotation, glm::u8vec4 color){

	glm::vec2 pen = posBL;

	int i = 0;
	while(str[i]){
		
		GlyphMetric metric = font->m_metrics[str[i]-u' '];

		glm::vec4 map = metric.bitmapTC;

		glm::vec4 proj = metric.bitmapTC;
		proj.x = pen.x;
		proj.y = pen.y;

		DrawTexture2D(font->m_bitmap, map, proj, origin, rotation, color);

		pen.x += metric.advance;
		i++;
	}
}

void BatchRenderer2D::DrawText(Font *font, const char *str, glm::vec2 posBL, glm::vec2 origin, float rotation, glm::u8vec4 color){

	int len = strlen(str);

	char16_t strUni[len+1];
	for(int i = 0; i < len; i++)
		strUni[i] = str[i];
	strUni[len] = 0;

	DrawText(font, strUni, posBL, origin, rotation, color);
}