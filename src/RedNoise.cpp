#include <CanvasTriangle.h>
#include <DrawingWindow.h>
#include <glm/glm.hpp>
#include <CanvasPoint.h>
#include <CanvasTriangle.h>
#include <TextureMap.h>
#include <Colour.h>
#include <Utils.h>
#include <fstream>
#include <vector>

#define WIDTH 320
#define HEIGHT 240

uint32_t colourToInt(Colour colour) 
{
	return 255 << 24 | colour.blue << 16 | colour.green << 8 | colour.red;
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

	CanvasPoint point = CanvasPoint(x, y);
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

//triangle[0] should be the non flat point
//clockwise indicies after
void fillHalfTriangle(CanvasTriangle triangle, uint32_t colour, DrawingWindow &window) 
{
	float baseHeight = triangle[1].y;
	float yDiff = triangle[0].y - baseHeight;
	float ySteps = ceil(fabs(yDiff));

	float fromDiff = triangle[0].x - triangle[2].x;
	float toDiff = triangle[0].x - triangle[1].x;

	float stepDir = yDiff > 0 ? 1 : -1;

	for (int j = 0; j < ySteps; j++)
	{
		float t = (float)(j) / (float)(ySteps);
		float y = baseHeight + j * stepDir;
		float fromX = triangle[2].x + fromDiff * t;
		float toX = triangle[1].x + toDiff * t;
		float xSteps = ceil(fabs(toX - fromX));
		float xDir = toX - fromX > 0 ? 1 : -1;
		
		for (int i = 0; i < xSteps; i++)
		{
			float x = fromX + i * xDir;
			window.setPixelColour(x, y, colour);
		}
	}
}

void fillTriangle(CanvasTriangle triangle, Colour colour, DrawingWindow &window)
{
	triangle = sortTriangle(triangle);
	CanvasPoint center = centerPoint(triangle);
	bool centerRight = center.x > triangle[1].x;
	CanvasPoint left = centerRight ? triangle[1] : center;
	CanvasPoint right = centerRight ? center : triangle[1];
	CanvasTriangle triangleTop = CanvasTriangle(triangle[0], right, left);
	CanvasTriangle triangleBottom = CanvasTriangle(triangle[2], left, right);
	
	uint32_t col = colourToInt(colour);
	fillHalfTriangle(triangleTop, col, window);
	fillHalfTriangle(triangleBottom, col, window);
}

std::vector<glm::vec3> interpolateThreeElementValues(glm::vec3 from, glm::vec3 to, int numberOfValues)
{
	std::vector<glm::vec3> v;
	for (int i = 0; i < numberOfValues; i++)
	{
		glm::vec3 value = from + (to - from) * ((float)i / (numberOfValues - 1));
		v.push_back(value);
	}

	return v;
}

std::vector<float> interpolateSingleFloats(float from, float to, int numberOfValues) 
{
	std::vector<float> v;
	for (int i = 0; i < numberOfValues; i++)
	{
		float value = from + (to - from) * ((float)i / (numberOfValues - 1));
		v.push_back(value);
	}

	return v;
}

void handleEvent(SDL_Event event, DrawingWindow &window) {
	if (event.type == SDL_KEYDOWN) {
		if (event.key.keysym.sym == SDLK_LEFT) std::cout << "LEFT" << std::endl;
		else if (event.key.keysym.sym == SDLK_RIGHT) std::cout << "RIGHT" << std::endl;
		else if (event.key.keysym.sym == SDLK_UP) std::cout << "UP" << std::endl;
		else if (event.key.keysym.sym == SDLK_DOWN) std::cout << "DOWN" << std::endl;
		else if (event.key.keysym.sym == SDLK_u) 
		{
			CanvasPoint v0 = CanvasPoint(random() % window.width, random() % window.height);
			CanvasPoint v1 = CanvasPoint(random() % window.width, random() % window.height);
			CanvasPoint v2 = CanvasPoint(random() % window.width, random() % window.height);
			CanvasTriangle tri = CanvasTriangle(v0, v1, v2);

			fillTriangle(tri, Colour(random() % 255,random() % 255,random() % 255), window);
			strokeTriangle(tri, Colour(255,255,255), window);
		} 
	} else if (event.type == SDL_MOUSEBUTTONDOWN) {
		window.savePPM("output.ppm");
		window.saveBMP("output.bmp");
	}
}

int main(int argc, char *argv[]) {
	DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
	SDL_Event event;
	while (true) {
		// We MUST poll for events - otherwise the window will freeze !
		if (window.pollForInputEvents(event)) handleEvent(event, window);
		
		// Need to render the frame at the end, or nothing actually gets shown on the screen !
		window.renderFrame();
	}
}
