#include <CanvasTriangle.h>
#include <DrawingWindow.h>
#include <glm/glm.hpp>
#include <CanvasPoint.h>
#include <CanvasTriangle.h>
#include <TextureMap.h>
#include <Colour.h>
#include <Utils.h>
#include <vector>
#include <iostream>
#include <string>
#include <unordered_map>
#include <ModelTriangle.h>

#include <Camera.h>
#include <MatrixUtils.h>
#include <ObjLoading.h>
#include <Light.h>

#define WIDTH 400
#define HEIGHT 300

const bool GouraudShading = false;

Camera camera;
int renderMode = 0;

uint32_t colourToInt(Colour colour) 
{
	return 255 << 24 | colour.red << 16 | colour.green << 8 | colour.blue;
}

void drawLine(CanvasPoint from, CanvasPoint to, Colour colour, DrawingWindow &window) 
{
	CanvasPoint diff = CanvasPoint(to.x - from.x, to.y - from.y);
	float numberOfSteps = fmax(fabs(diff.x), fabs(diff.y));
	CanvasPoint stepSize = CanvasPoint(diff.x / numberOfSteps, diff.y / numberOfSteps);
	for (int i = 0; i < numberOfSteps; i++)
	{
		CanvasPoint p = CanvasPoint(from.x + stepSize.x * i, from.y + stepSize.y * i);
		window.setPixelColour(p.x, p.y, colourToInt(colour));
	}
}

void strokeTriangle(CanvasTriangle triangle, Colour colour, DrawingWindow &window) 
{
	drawLine(triangle[0], triangle[1], colour, window);
	drawLine(triangle[1], triangle[2], colour, window);
	drawLine(triangle[2], triangle[0], colour, window);
}

CanvasPoint lerpCanvasPoint(CanvasPoint v1, CanvasPoint v2, float t)
{
	float x = v2.x * t + v1.x * (1 - t);
	float y = v2.y * t + v1.y * (1 - t);
	float z = v2.depth * t + v1.depth * (1 - t);

	CanvasPoint point = CanvasPoint(x, y, z);
	point.texturePoint.x = v2.texturePoint.x * t + v1.texturePoint.x * (1 - t);
	point.texturePoint.y = v2.texturePoint.y * t + v1.texturePoint.y * (1 - t);

	return point;
}

CanvasPoint centerPoint(CanvasTriangle sortedTri) 
{
	float yDiff = sortedTri[2].y - sortedTri[0].y;
	float t = (sortedTri[1].y - sortedTri[0].y) / yDiff;

	return lerpCanvasPoint(sortedTri[0], sortedTri[2], t);
}

CanvasTriangle sortTriangle(CanvasTriangle triangle) 
{
	if (triangle[2].y > triangle[1].y) 
	{
		std::swap(triangle[2], triangle[1]);
	}
	if (triangle[1].y > triangle[0].y) 
	{
		std::swap(triangle[1], triangle[0]);
	}
	if (triangle[2].y > triangle[1].y) 
	{
		std::swap(triangle[2], triangle[1]);
	}

	return triangle;
}

uint32_t sampleTexture(TexturePoint point, TextureMap &map)
{
	return map.pixels[round(point.y) * map.width + round(point.x)];
}
int clampToScreen(int x, int bound)
{
	return x < 0 ? 0 : (x >= bound ? bound - 1 : x);
}

//triangle[0] should be the non flat point
//clockwise indicies after
void fillHalfTriangle(CanvasTriangle triangle, Colour colour, float **depthBuffer, DrawingWindow &window) 
{
	uint32_t col = colourToInt(colour);

	int fromY = triangle[1].y;
	int toY = triangle[0].y;

	bool swappedY = toY < fromY;
	if (swappedY)
	{
		std::swap(fromY, toY);
	}
	
	int ySteps = toY - fromY;

	int clampedFromY = clampToScreen(fromY, window.height);
	int clampedToY = clampToScreen(toY, window.height);

	for (int j = clampedFromY; j <= clampedToY && clampedToY != clampedFromY; j++)
	{
		double t = (double)(j - fromY) / (double)(ySteps);
		CanvasPoint fromPoint = lerpCanvasPoint(triangle[2], triangle[0], swappedY ? 1 - t : t);
		CanvasPoint toPoint = lerpCanvasPoint(triangle[1], triangle[0], swappedY ? 1 - t : t);
		int fromX = fromPoint.x;
		int toX = toPoint.x;

		bool swappedX = toX < fromX;
		if (swappedX)
		{
			std::swap(fromX, toX);
		}

		int xSteps = toX - fromX;

		int clampedFromX = clampToScreen(fromX, window.width);
		int clampedToX = clampToScreen(toX, window.width);

		for (int i = clampedFromX; i <= clampedToX && clampedToX != clampedFromX; i++)
		{
			double tx = (double)(i - fromX) / (double)xSteps;
			CanvasPoint point = lerpCanvasPoint(fromPoint, toPoint, swappedX ? 1 - tx : tx);
			
			float d = depthBuffer[i][j];
			float id = 1 / point.depth;
			if (point.depth > 0 && id > d)
			{
				window.setPixelColour(i, j, col);
				depthBuffer[i][j] = id;
			}
		}
	}
}

void fillTriangle(CanvasTriangle triangle, Colour col, float **depthBuffer, DrawingWindow &window)
{
	triangle = sortTriangle(triangle);
	CanvasPoint center = centerPoint(triangle);
	bool centerRight = center.x > triangle[1].x;
	CanvasPoint left = centerRight ? triangle[1] : center;
	CanvasPoint right = centerRight ? center : triangle[1];
	CanvasTriangle triangleTop = CanvasTriangle(triangle[0], right, left);
	CanvasTriangle triangleBottom = CanvasTriangle(triangle[2], left, right);
	
	fillHalfTriangle(triangleTop, col, depthBuffer, window);
	fillHalfTriangle(triangleBottom, col, depthBuffer, window);
}

void handleEvent(SDL_Event event, DrawingWindow &window) {
	if (event.type == SDL_KEYDOWN) {
		if (event.key.keysym.sym == SDLK_0) 
		{
			renderMode = 0;
		}
		else if (event.key.keysym.sym == SDLK_1) 
		{
			renderMode = 1;
		}
		else if (event.key.keysym.sym == SDLK_2) 
		{
			renderMode = 2;
		}
	} else if (event.type == SDL_MOUSEBUTTONDOWN) {
		window.savePPM("output.ppm");
		window.saveBMP("output.bmp");
	}
}

void clearDepthBuffer(float **depthBuffer, DrawingWindow &window)
{
	for (int i = 0; i < window.width; i++)
	{
		for (int j = 0; j < window.height; j++)
		{
			depthBuffer[i][j] = 0;
		}
	}
}

void wireframeDraw(DrawingWindow &window, std::vector<ModelTriangle> &model, std::vector<ModelVertex> &verts)
{
	window.clearPixels();
	camera.updateTransform();
	
	for (int i = 0; i < model.size(); i++)
	{
		ModelTriangle tri = model[i];
		glm::vec3 mv1 = verts[tri.vertices[0]].pos;
		glm::vec3 mv2 = verts[tri.vertices[1]].pos;
		glm::vec3 mv3 = verts[tri.vertices[2]].pos;
		glm::vec3 v1 = camera.getCanvasIntersectionPoint(glm::vec4(mv1, 1));
		glm::vec3 v2 = camera.getCanvasIntersectionPoint(glm::vec4(mv2, 1));
		glm::vec3 v3 = camera.getCanvasIntersectionPoint(glm::vec4(mv3, 1));
		auto p1 = CanvasPoint(v1.x, v1.y, v1.z);
		auto p2 = CanvasPoint(v2.x, v2.y, v2.z);
		auto p3 = CanvasPoint(v3.x, v3.y, v3.z);
		CanvasTriangle triangle(p1,p2,p3);
		strokeTriangle(triangle, tri.colour, window);
	}
}

void rasteriseDraw(DrawingWindow &window, float **depthBuffer, std::vector<ModelTriangle> &model, std::vector<ModelVertex> &verts)
{
	clearDepthBuffer(depthBuffer, window);
	window.clearPixels();

	camera.updateTransform();

	for (int i = 0; i < model.size(); i++)
	{
		ModelTriangle tri = model[i];
		glm::vec3 mv1 = verts[tri.vertices[0]].pos;
		glm::vec3 mv2 = verts[tri.vertices[1]].pos;
		glm::vec3 mv3 = verts[tri.vertices[2]].pos;
		glm::vec3 v1 = camera.getCanvasIntersectionPoint(glm::vec4(mv1, 1));
		glm::vec3 v2 = camera.getCanvasIntersectionPoint(glm::vec4(mv2, 1));
		glm::vec3 v3 = camera.getCanvasIntersectionPoint(glm::vec4(mv3, 1));
		auto p1 = CanvasPoint(v1.x, v1.y, v1.z);
		auto p2 = CanvasPoint(v2.x, v2.y, v2.z);
		auto p3 = CanvasPoint(v3.x, v3.y, v3.z);
		CanvasTriangle triangle(p1,p2,p3);
		fillTriangle(triangle, tri.colour, depthBuffer, window);
	}
}


void traceDraw(DrawingWindow &window, std::vector<ModelTriangle> &model, std::vector<ModelVertex> &verts, std::vector<Light> &lights)
{
	window.clearPixels();

	camera.updateTransform();
	
	for (int i = 0; i < window.width; i++)
	{
		for (int j = 0; j < window.height; j++)
		{
			Colour col = camera.renderTraced(i, j, model, verts, lights);
			uint32_t intCol = colourToInt(col);
			window.setPixelColour(i, j, intCol);
		}
	}
}



void traceDrawGouraud(DrawingWindow &window, std::vector<ModelTriangle> &model, std::vector<ModelVertex> &verts, std::vector<Light> &lights)
{
	window.clearPixels();

	camera.updateTransform();

	std::vector<glm::vec3> vertexColours = std::vector<glm::vec3>();
	camera.initialiseGouraud(model, verts, lights, vertexColours);
	
	for (int i = 0; i < window.width; i++)
	{
		for (int j = 0; j < window.height; j++)
		{
			Colour col = camera.renderTracedGouraud(i, j, model, verts, lights, vertexColours);
			uint32_t intCol = colourToInt(col);
			window.setPixelColour(i, j, intCol);
		}
	}
}

int main(int argc, char *argv[]) {
	DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
	SDL_Event event;

	
	std::unordered_map<std::string, Colour> materials = std::unordered_map<std::string, Colour>();

	loadMtl(materials, "/Users/smb/Desktop/Graphics-Coursework/src/cornell-box.mtl");
	std::vector<ModelVertex> verts = std::vector<ModelVertex>();
	std::vector<ModelTriangle> model = std::vector<ModelTriangle>();
	loadObj(model, "/Users/smb/Desktop/Graphics-Coursework/src/cornell-box.obj", materials, verts, 0.35f);

	std::vector<Light> lights = std::vector<Light>();
	lights.push_back(Light(glm::vec3(0, 0.8f, 0), glm::vec3(10,10,10)));
	
	float angle = 0;
	auto cameraToWorld = matrixTRS(glm::vec3(0,0,3), glm::vec3(0,0,0));
	camera = Camera(200, cameraToWorld, window.width, window.height);
	float **depthBuffer;
	depthBuffer = new float *[window.width];
	for (int i = 0; i < window.width; i++)
	{
		depthBuffer[i] = new float[window.height];
		for (int j = 0; j < window.height; j++)
		{
			depthBuffer[i][j] = 0;
		}
	}

	bool rendered = false;
	
	while (true) {
		// We MUST poll for events - otherwise the window will freeze !
		if (window.pollForInputEvents(event)) handleEvent(event, window);
		//angle += 0.01;
		camera.cameraToWorld = matrixTRS(glm::vec3(sin(angle) * 3,0,cos(angle) * 3), glm::vec3(0,0,M_PI));
		camera.cameraToWorld = lookAt(camera.cameraToWorld, glm::vec3(0,0,0));

		if (renderMode == 0)
		{
			wireframeDraw(window, model, verts);
			rendered = false;
		}
		else if (renderMode == 1)
		{
			rasteriseDraw(window, depthBuffer, model, verts);
			rendered = false;
		}
		else if (renderMode == 2 && !rendered)
		{
			if (GouraudShading)
			{
				traceDrawGouraud(window, model, verts, lights);
			}
			else 
			{
				traceDraw(window, model, verts, lights);
			}
			rendered = true;
		}

		//Light debug

		window.renderFrame();
	}
	
	for (int i = 0; i < window.width; i++)
	{
		delete [] depthBuffer[i];
	}

	delete [] depthBuffer;
}
