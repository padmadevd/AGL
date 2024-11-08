
INCLUDE_FOLDER := -IC:\Users\username\programming\cpplibs\include -I.
LIB_FOLDER := -LBuild
LIBS := -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lfreetype -lglad -lopengl32 -lgdi32 -lkernel32

Build/test.exe: Build/test.o Build/agl.o Build/audio.o Build/eventDispatcher.o Build/gl.o Build/input.o Build/window.o\
		Build/batch.o Build/aglShader.o Build/image.o Build/batchRenderer.o Build/camera2d.o Build/font.o Build/frameBuffer.o Build/shader.o\
		Build/texture2d.o Build/particles.o Build/polygon2d.o Build/raylibImage.o

	g++ -o Build/test.exe  Build/test.o Build/agl.o Build/audio.o Build/eventDispatcher.o Build/gl.o Build/input.o Build/window.o\
		Build/aglShader.o Build/image.o Build/batch.o Build/batchRenderer.o Build/camera2d.o Build/font.o Build/frameBuffer.o Build/shader.o\
		Build/texture2d.o Build/particles.o Build/polygon2d.o Build/raylibImage.o $(INCLUDE_FOLDER) $(LIB_FOLDER) $(LIBS)

Build/test.o: test.cpp
	g++ -c -o Build/test.o test.cpp $(INCLUDE_FOLDER)

Build/agl.o: AGL/agl.cpp
	g++ -c -o Build/agl.o AGL/agl.cpp $(INCLUDE_FOLDER)

Build/eventDispatcher.o: Base/Events/eventDispatcher.cpp
	g++ -c -o Build/eventDispatcher.o Base/Events/eventDispatcher.cpp $(INCLUDE_FOLDER)

Build/window.o: Base/window.cpp
	g++ -c -o Build/window.o Base/window.cpp $(INCLUDE_FOLDER)

Build/gl.o: Base/gl.cpp
	g++ -c -o Build/gl.o Base/gl.cpp $(INCLUDE_FOLDER)

Build/input.o: Base/input.cpp
	g++ -c -o Build/input.o Base/input.cpp $(INCLUDE_FOLDER)

Build/audio.o: Audio/audio.cpp
	g++ -c -o Build/Audio.o Audio/audio.cpp $(INCLUDE_FOLDER)

Build/aglShader.o: Graphics/aglShader.cpp
	g++ -c -o Build/aglShader.o Graphics/aglShader.cpp $(INCLUDE_FOLDER)

Build/image.o: Graphics/image.cpp
	g++ -c -o Build/image.o Graphics/image.cpp $(INCLUDE_FOLDER)

Build/batch.o: BatchRenderer/batch.cpp
	g++ -c -o Build/batch.o BatchRenderer/batch.cpp $(INCLUDE_FOLDER)

Build/batchRenderer.o: BatchRenderer/batchRenderer.cpp
	g++ -c -o Build/batchRenderer.o BatchRenderer/batchRenderer.cpp $(INCLUDE_FOLDER)

Build/shader.o: Graphics/shader.cpp
	g++ -c -o Build/shader.o Graphics/shader.cpp $(INCLUDE_FOLDER)

Build/font.o: Graphics/font.cpp
	g++ -c -o Build/font.o Graphics/font.cpp $(INCLUDE_FOLDER)

Build/texture2d.o: Graphics/texture2d.cpp
	g++ -c -o Build/texture2d.o Graphics/texture2d.cpp $(INCLUDE_FOLDER)

Build/camera2d.o: Graphics/camera2d.cpp
	g++ -c -o Build/camera2d.o Graphics/camera2d.cpp $(INCLUDE_FOLDER)

Build/frameBuffer.o: Graphics/frameBuffer.cpp
	g++ -c -o Build/frameBuffer.o Graphics/frameBuffer.cpp $(INCLUDE_FOLDER)

Build/particles.o: ParticleSystem/particles.cpp
	g++ -c -o Build/particles.o ParticleSystem/particles.cpp $(INCLUDE_FOLDER)

Build/polygon2d.o: Physics/polygon2d.cpp
	g++ -c -o Build/polygon2d.o Physics/polygon2d.cpp $(INCLUDE_FOLDER)

Build/raylibImage.o: Vendor/raylib/image.cpp
	g++ -c -o Build/raylibImage.o Vendor/raylib/image.cpp $(INCLUDE_FOLDER)

run: Build/test.exe
	.\Build\test.exe
