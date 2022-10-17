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
#include <iostream>
#include <string>
#include <fstream>
#include <ModelTriangle.h>
#include <unordered_map>

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

uint32_t sampleTexture(TexturePoint point, TextureMap &map)
{
	return map.pixels[round(point.y) * map.width + round(point.x)];
}

//triangle[0] should be the non flat point
//clockwise indicies after
void fillHalfTriangle(CanvasTriangle triangle, TextureMap &map, DrawingWindow &window) 
{
	float baseHeight = triangle[1].y;
	float yDiff = triangle[0].y - baseHeight;
	float ySteps = ceil(fabs(yDiff));

	for (int j = 0; j < ySteps; j++)
	{
		float t = (float)(j) / (float)(ySteps);
		CanvasPoint fromPoint = lerpCanvasPoint(triangle[2], triangle[0], t);
		CanvasPoint toPoint = lerpCanvasPoint(triangle[1], triangle[0], t);
		float xSteps = ceil(fabs(toPoint.x - fromPoint.x)); 
		
		for (int i = 0; i < xSteps; i++)
		{
			CanvasPoint point = lerpCanvasPoint(fromPoint, toPoint, (float)(i) / (float)(xSteps));
			uint32_t col = sampleTexture(point.texturePoint, map);
			window.setPixelColour(round(point.x), round(point.y), col);
		}
	}
}

void fillTriangle(CanvasTriangle triangle, TextureMap &map, DrawingWindow &window)
{
	triangle = sortTriangle(triangle);
	CanvasPoint center = centerPoint(triangle);
	bool centerRight = center.x > triangle[1].x;
	CanvasPoint left = centerRight ? triangle[1] : center;
	CanvasPoint right = centerRight ? center : triangle[1];
	CanvasTriangle triangleTop = CanvasTriangle(triangle[0], right, left);
	CanvasTriangle triangleBottom = CanvasTriangle(triangle[2], left, right);
	
	fillHalfTriangle(triangleTop, map, window);
	fillHalfTriangle(triangleBottom, map, window);
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

int getSubStringIndex(std::string s, char delimiter, int startIndex)
{
	for (int i = startIndex; i < s.length(); i++)
	{
		if (s[i] == delimiter) 
		{
			return i;
		}
	}

	return -1;
}

std::string stringRange(std::string s, int from, int to)
{
	std::string sub = "";
	for (int i = from; i < to; i++)
	{
		sub += s[i];
	}
	
	return sub;
}

glm::vec3 vec3FromString(std::string s, float scale)
{
	int xTo = getSubStringIndex(s, ' ', 2);
	float x = std::stof(stringRange(s, 2, xTo));
	int yTo = getSubStringIndex(s, ' ', xTo + 1);
	float y = std::stof(stringRange(s, xTo + 1, yTo));
	float z = std::stof(stringRange(s, yTo + 1, s.length()));

	return glm::vec3(x * scale, y * scale, z * scale);
}

ModelTriangle triFromString(std::string s, std::vector<glm::vec3> &verts)
{
	int xTo = getSubStringIndex(s, ' ', 2);
	int x = std::stoi(stringRange(s, 2, xTo)) - 1;
	int yTo = getSubStringIndex(s, ' ', xTo + 1);
	int y = std::stoi(stringRange(s, xTo + 1, yTo)) - 1;
	int z = std::stoi(stringRange(s, yTo + 1, s.length())) - 1;
	return ModelTriangle(verts[x], verts[y], verts[z], Colour(255,255,255));
}

std::unordered_map<std::string, Colour> loadMtl(std::string path)
{
	std::unordered_map<std::string, Colour> materials();
	
	std::string line;
	std::ifstream file(path);


	return materials;
}

std::vector<ModelTriangle> loadObj(std::string path, float scale) 
{
	std::string line;
	std::ifstream file(path);
	std::vector<ModelTriangle> triangles;
	std::vector<glm::vec3> verts;
	while (getline(file, line))
	{
		if (line[0] == 'o') 
		{
			while (getline(file, line) && line.length() > 0) 
			{
				if (line[0] == 'v')
				{
					verts.push_back(vec3FromString(line, scale));
				}
				else if (line[0] == 'f')
				{
					triangles.push_back(triFromString(line, verts));
				}
			}
		}
	}

	file.close();

	return triangles;
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
			v0.texturePoint = TexturePoint(v0.x, v0.y);
			CanvasPoint v1 = CanvasPoint(random() % window.width, random() % window.height);
			v1.texturePoint = TexturePoint(v1.x, v1.y);
			CanvasPoint v2 = CanvasPoint(random() % window.width, random() % window.height);
			v2.texturePoint = TexturePoint(v2.x, v2.y);
			CanvasTriangle tri = CanvasTriangle(v0, v1, v2);

			TextureMap map = TextureMap("/Users/smb/Desktop/RedNoise/src/texture.ppm");

			fillTriangle(tri, map, window);
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

	CanvasPoint v0 = CanvasPoint(160, 10);
	v0.texturePoint = TexturePoint(195, 5);
	CanvasPoint v1 = CanvasPoint(300, 230);
	v1.texturePoint = TexturePoint(395, 380);
	CanvasPoint v2 = CanvasPoint(10, 150);
	v2.texturePoint = TexturePoint(65, 330);
	CanvasTriangle tri = CanvasTriangle(v0, v1, v2);

	TextureMap map = TextureMap("/Users/smb/Desktop/RedNoise/src/texture.ppm");

	fillTriangle(tri, map, window);
	strokeTriangle(tri, Colour(255,255,255), window);
	
	std::vector<ModelTriangle> model = loadObj("/Users/smb/Desktop/RedNoise/src/cornell-box.obj", 0.35f);
	
	
	std::cout << model[0] << std::endl;
	

	while (true) {
		// We MUST poll for events - otherwise the window will freeze !
		if (window.pollForInputEvents(event)) handleEvent(event, window);
		
		// Need to render the frame at the end, or nothing actually gets shown on the screen !
		window.renderFrame();
	}
}
