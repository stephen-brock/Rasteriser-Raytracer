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

#define WIDTH 400
#define HEIGHT 300

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

	return glm::vec3(x, y, z) * scale;
}

ModelTriangle triFromString(std::string s, std::vector<glm::vec3> &verts, Colour col)
{
	int xTo = getSubStringIndex(s, ' ', 2);
	int x = std::stoi(stringRange(s, 2, xTo)) - 1;
	int yTo = getSubStringIndex(s, ' ', xTo + 1);
	int y = std::stoi(stringRange(s, xTo + 1, yTo)) - 1;
	int z = std::stoi(stringRange(s, yTo + 1, s.length())) - 1;

	return ModelTriangle(verts[x], verts[y], verts[z], col);
}

std::string getMatNameFromString(std::string s)
{
	return stringRange(s, 6, s.length());
}

Colour getColourFromString(std::string s) 
{
	int xTo = getSubStringIndex(s, ' ', 3);
	float x = std::stof(stringRange(s, 3, xTo));
	int yTo = getSubStringIndex(s, ' ', xTo + 1);
	float y = std::stof(stringRange(s, xTo + 1, yTo));
	float z = std::stof(stringRange(s, yTo + 1, s.length()));

	return Colour(x * 255,y * 255,z * 255);
}

std::unordered_map<std::string, Colour> loadMtl(std::string path)
{
	std::unordered_map<std::string, Colour> materials;

	std::string line;
	std::ifstream file(path);

	while (getline(file, line))
	{
		if (line[0] == 'n')
		{
			std::string name = getMatNameFromString(line);
			getline(file, line);
			Colour col = getColourFromString(line);
			materials[name] = col;
		}
	}

	return materials;
}

std::vector<ModelTriangle> loadObj(std::string path, std::unordered_map<std::string, Colour> &materials, float scale) 
{
	std::string line;
	std::ifstream file(path);
	std::vector<ModelTriangle> triangles;
	std::vector<glm::vec3> verts;
	Colour currentColour = Colour(-1,-1,-1);
	while (getline(file, line))
	{
		if (line[0] == 'o') 
		{
			getline(file, line);
			std::string mat = getMatNameFromString(line);
			currentColour = materials[mat];
			while (getline(file, line) && line.length() > 0) 
			{
				if (line[0] == 'v')
				{
					verts.push_back(vec3FromString(line, scale));
				}
				else if (line[0] == 'f')
				{
					triangles.push_back(triFromString(line, verts, currentColour));
				}
			}
		}
	}

	file.close();

	return triangles;
}

glm::vec4 posFromMatrix(glm::mat4 mat)
{
	return glm::vec4(mat[0][3], mat[1][3], mat[2][3], 1);
}

glm::vec3 getCanvasIntersectionPoint(glm::mat4 viewMatrix, glm::vec4 vertexPosition, float focalLength, DrawingWindow &window)
{
	glm::vec4 cPos = vertexPosition * viewMatrix;
	cPos *= 230.0f;
	cPos.z = -cPos.z;
	float u = focalLength * cPos.x / fabs(cPos.z) + window.width / 2;
	float v = window.height / 2 - focalLength * cPos.y / fabs(cPos.z);

	return glm::vec3(u, v, cPos.z);
}

glm::mat4 xRotation(glm::mat4 mat, float angle)
{
	return glm::mat4(
		1,0,0,0,
		0,cos(angle),-sin(angle),0,
		0,sin(angle),cos(angle),0,
		0,0,0,1
	) * mat;
}

glm::mat4 yRotation(glm::mat4 mat, float angle)
{
	return glm::mat4(
		cos(angle),0,sin(angle),0,
		0,1,0,0,
		-sin(angle),0,cos(angle),0,
		0,0,0,1
	) * mat;
}

glm::mat4 zRotation(glm::mat4 mat, float angle)
{
	return glm::mat4(
		cos(angle),-sin(angle),0,0,
		sin(angle),cos(angle),0,0,
		0,0,1,0,
		0,0,0,1
	) * mat;
}

glm::mat4 move(glm::mat4 mat, glm::vec3 add)
{
	mat[0][3] += add.x;
	mat[1][3] += add.y;
	mat[2][3] += add.z;

	return mat;
}

glm::mat4 lookAt(glm::mat4 mat, glm::vec3 origin)
{
	glm::vec4 pos = posFromMatrix(mat);
	glm::vec3 fwd = glm::normalize((glm::vec3)pos - origin);
	glm::vec3 up = glm::vec3(0,1,0);
	glm::vec3 right = glm::cross(fwd, up);
	auto newMat = glm::mat4(
		right.x, up.x, fwd.x, pos.x,
		right.y, up.y, fwd.y, pos.y,
		right.z, up.z, fwd.z, pos.z,
		0,0,0,1
		);

	return newMat;
}

glm::mat4 matrixTRS(glm::vec3 pos, glm::vec3 eulerAngles)
{
	auto mat = glm::mat4(
		1,0,0,0,
		0,1,0,0,
		0,0,1,0,
		0,0,0,1
	);
	mat = xRotation(mat, eulerAngles.x);
	mat = yRotation(mat, eulerAngles.y);
	mat = zRotation(mat, eulerAngles.z);
	return move(mat, pos);
}

void handleEvent(SDL_Event event, DrawingWindow &window) {
	if (event.type == SDL_KEYDOWN) {
		if (event.key.keysym.sym == SDLK_LEFT) {
			CameraToWorld = move(CameraToWorld, glm::vec3(0.1, 0, 0));
		}
		else if (event.key.keysym.sym == SDLK_RIGHT) {
			CameraToWorld = move(CameraToWorld, glm::vec3(-0.1, 0, 0));
		}
		else if (event.key.keysym.sym == SDLK_UP) {
			CameraToWorld = move(CameraToWorld, glm::vec3(0, 0, -0.1));
		}
		else if (event.key.keysym.sym == SDLK_DOWN) {
			CameraToWorld = move(CameraToWorld, glm::vec3(0, 0, 0.1));
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

void draw(DrawingWindow &window, float **depthBuffer, std::vector<ModelTriangle> model)
{
	clearDepthBuffer(depthBuffer, window);
	window.clearPixels();

	float focalPoint = 400.0;

	glm::mat4 worldToCamera = glm::inverse(CameraToWorld);

	for (int i = 0; i < model.size(); i++)
	{
		ModelTriangle tri = model[i];
		glm::vec3 v1 = getCanvasIntersectionPoint(worldToCamera, glm::vec4(tri.vertices[0], 1), focalPoint, window);
		glm::vec3 v2 = getCanvasIntersectionPoint(worldToCamera, glm::vec4(tri.vertices[1], 1), focalPoint, window);
		glm::vec3 v3 = getCanvasIntersectionPoint(worldToCamera, glm::vec4(tri.vertices[2], 1), focalPoint, window);
		auto p1 = CanvasPoint(v1.x, v1.y, v1.z);
		auto p2 = CanvasPoint(v2.x, v2.y, v2.z);
		auto p3 = CanvasPoint(v3.x, v3.y, v3.z);
		CanvasTriangle triangle(p1,p2,p3);
		fillTriangle(triangle, tri.colour, depthBuffer, window);
	}
}

int main(int argc, char *argv[]) {
	DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
	SDL_Event event;

	
	std::unordered_map<std::string, Colour> materials = loadMtl("/Users/smb/Desktop/RedNoise/src/cornell-box.mtl");
	std::vector<ModelTriangle> model = loadObj("/Users/smb/Desktop/RedNoise/src/cornell-box.obj", materials, 0.35f);
	
	float angle = 0;
	CameraToWorld = matrixTRS(glm::vec3(0,0,3), glm::vec3(0,0,M_PI));

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
	
	while (true) {
		// We MUST poll for events - otherwise the window will freeze !
		if (window.pollForInputEvents(event)) handleEvent(event, window);
		angle += 0.01;
		CameraToWorld = matrixTRS(glm::vec3(sin(angle) * 3,0,cos(angle) * 3), glm::vec3(0,0,M_PI));
		CameraToWorld = lookAt(CameraToWorld, glm::vec3(0,0,0));

		draw(window, depthBuffer, model);
		// Need to render the frame at the end, or nothing actually gets shown on the screen !
		window.renderFrame();
	}
	
	for (int i = 0; i < window.width; i++)
	{
		delete [] depthBuffer[i];
	}

	delete [] depthBuffer;
}
