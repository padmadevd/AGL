#ifndef AGL_BATCHRENDERER
#define AGL_BATCHRENDERER

#include <Graphics/texture2d.hpp>
#include <Graphics/shader.hpp>
#include <Graphics/font.hpp>
#include <Graphics/camera2d.hpp>
#include <Graphics/frameBuffer.hpp>
#include <BatchRenderer/batch.hpp>
#include <Base/Events/event.hpp>

// BATCH RENDERER ------------------------------------------------------------------------

class BatchRenderer2D{

    public:

        BatchRenderer2D(glm::vec2 winSize, uint32_t batchCount, uint32_t vertexPerBatch, uint32_t indexPerBatch, uint32_t texPerBatch);

        void SetFlipY(bool flip);

        FrameBuffer* GetDefaultDrawFB();
        FrameBuffer* GetDefaultReadFB();

        void BeginDrawCustomShader(Camera2D cam, Shader *shader);
        void EndDrawCustomShader();

        void BeginDraw(Camera2D cam);
        void EndDraw();

        void ClearColor(glm::vec4 color);
        void Clear();

        void AddTri(Vertex *tri, Texture2D *tex);
        void AddQuad(Vertex *quad, Texture2D *tex);

        void DrawTriangle(glm::vec2 v1, glm::vec2 v2, glm::vec2 v3, glm::u8vec4 color);
        void DrawTriangleStrip(glm::vec2 *points, int pointCount, glm::u8vec4 color);
        void DrawTriangleFan(glm::vec2 *points, int pointCount, glm::u8vec4 color);

        void DrawPixel(glm::vec2 position, glm::u8vec4 color);
        void DrawLine(glm::vec2 startPos, glm::vec2 endPos, float thick, glm::u8vec4 color);
        void DrawLineBezier(glm::vec2 startPos, glm::vec2 endPos, int segments, float thick, glm::u8vec4 color);

        void DrawRectangle(glm::vec4 rec, glm::vec2 origin, float rotation, glm::u8vec4 color); 
        // void DrawRectangleLines(glm::vec4 rec, float lineThick, glm::u8vec4 color);    

        // void DrawRectangleRounded(glm::vec4 rec, float roundness, int segments, glm::u8vec2 color);
        // void DrawRectangleRoundedLines(glm::vec4 rec, float roundness, int segments, float lineThick, glm::u8vec2 color);
        
        void DrawCircleSector(glm::vec2 center, float radius, float startAngle, float endAngle, int segments, glm::u8vec4 color);
        void DrawCircle(glm::vec2 center, float radius, glm::u8vec4 color);
        void DrawRing(glm::vec2 center, float innerRadius, float outerRadius, float startAngle, float endAngle, int segments, glm::u8vec4 color);
        void DrawCircleLines(glm::vec2 center, float radius, float lineThick, glm::u8vec4 color);

        void DrawPoly(glm::vec2 center, int sides, float radius, float rotation, glm::u8vec4 color);
        void DrawPolyLines(glm::vec2 center, int sides, float radius, float rotation, float lineThick, glm::u8vec4 color);

        void DrawConvexPolygon(glm::vec2 *points, int pointCount, glm::vec2 origin, float rotation, glm::u8vec4 color);
 
        // Splines drawing functions
        void DrawSplineLinear(glm::vec2 *points, int pointCount, float thick, glm::u8vec4 color);                  // Draw spline: Linear, minimum 2 points
        void DrawSplineBasis(glm::vec2 *points, int pointCount, float thick, glm::u8vec4 color);                   // Draw spline: B-Spline, minimum 4 points
        void DrawSplineCatmullRom(glm::vec2 *points, int pointCount, float thick, glm::u8vec4 color);              // Draw spline: Catmull-Rom, minimum 4 points
        void DrawSplineBezierQuadratic(glm::vec2 *points, int pointCount, float thick, glm::u8vec4 color);         // Draw spline: Quadratic Bezier, minimum 3 points (1 control point): [p1, c2, p3, c4...]
        void DrawSplineBezierCubic(glm::vec2 *points, int pointCount, float thick, glm::u8vec4 color);             // Draw spline: Cubic Bezier, minimum 4 points (2 control points): [p1, c2, c3, p4, c5, c6...]
        void DrawSplineSegmentLinear(glm::vec2 p1, glm::vec2 p2, float thick, glm::u8vec4 color);                    // Draw spline segment: Linear, 2 points
        void DrawSplineSegmentBasis(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 p4, float thick, glm::u8vec4 color); // Draw spline segment: B-Spline, 4 points
        void DrawSplineSegmentCatmullRom(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 p4, float thick, glm::u8vec4 color); // Draw spline segment: Catmull-Rom, 4 points
        void DrawSplineSegmentBezierQuadratic(glm::vec2 p1, glm::vec2 c2, glm::vec2 p3, float thick, glm::u8vec4 color); // Draw spline segment: Quadratic Bezier, 2 points, 1 control point
        void DrawSplineSegmentBezierCubic(glm::vec2 p1, glm::vec2 c2, glm::vec2 c3, glm::vec2 p4, float thick, glm::u8vec4 color); // Draw spline segment: Cubic Bezier, 2 points, 2 control points

        void DrawTexture2D(Texture2D *texture, glm::ivec4 map, glm::vec4 proj, glm::vec2 origin, float rotation, glm::u8vec4 color);

        void DrawText(Font *font, const char16_t *str, glm::vec2 posBL, glm::vec2 origin, float rotation, glm::u8vec4 color);
        void DrawText(Font *font, const char *str, glm::vec2 posBL, glm::vec2 origin, float rotation, glm::u8vec4 color);

        void Free();
        ~BatchRenderer2D();

    protected:

        void OnWindowResize(const Event* event, void *userData);

    protected:

        std::vector<Batch*> m_batches;
        uint32_t m_batchN;

        Camera2D m_camera;
        glm::vec2 m_windowSize;

        Shader *m_shader;
        glm::mat4 m_viewMatrix;
        glm::mat4 m_projMatrix;

        bool m_flipY;
        bool m_customShader;
        bool m_beginDraw;

        FrameBuffer *m_defaultDrawFB;
        FrameBuffer *m_defaultReadFB;
};

#endif