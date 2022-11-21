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
int renderMode = 1;

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
void fillHalfTriangle(CanvasTriangle triangle, Material* material, float **depthBuffer, DrawingWindow &window) 
{
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
		t = swappedY ? 1 - t : t;
		CanvasPoint fromPoint = lerpCanvasPoint(triangle[2], triangle[0], t);
		CanvasPoint toPoint = lerpCanvasPoint(triangle[1], triangle[0], t);
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
			tx = swappedX ? 1 - tx : tx;
			CanvasPoint point = lerpCanvasPoint(fromPoint, toPoint, tx);
			
			float d = depthBuffer[i][j];
			double id = point.depth;
			if (id > d)
			{
				uint32_t col = colourToInt(vectorToColour(material->sampleAlbedo(point.texturePoint.x / id, point.texturePoint.y / id)));
				window.setPixelColour(i, j, col);
				depthBuffer[i][j] = id;
			}
		}
	}
}

void fillTriangle(CanvasTriangle triangle, Material* material, float **depthBuffer, DrawingWindow &window)
{
	triangle = sortTriangle(triangle);
	CanvasPoint center = centerPoint(triangle);
	bool centerRight = center.x > triangle[1].x;
	CanvasPoint left = centerRight ? triangle[1] : center;
	CanvasPoint right = centerRight ? center : triangle[1];
	CanvasTriangle triangleTop = CanvasTriangle(triangle[0], right, left);
	CanvasTriangle triangleBottom = CanvasTriangle(triangle[2], left, right);
	
	fillHalfTriangle(triangleTop, material, depthBuffer, window);
	fillHalfTriangle(triangleBottom, material, depthBuffer, window);
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

void wireframeDraw(DrawingWindow &window, std::vector<Model*> &models)
{
	window.clearPixels();
	camera.updateTransform();
	
	for (int m = 0; m < models.size(); m++)
	{
		Model& model = *models[m];
		for (int i = 0; i < model.triangles->size(); i++)
		{
			ModelTriangle tri = model.triangles->at(i);
			glm::vec3 mv1 = model.verts->at(tri.vertices[0]).pos;
			glm::vec3 mv2 = model.verts->at(tri.vertices[1]).pos;
			glm::vec3 mv3 = model.verts->at(tri.vertices[2]).pos;
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
}

void rasteriseDraw(DrawingWindow &window, float **depthBuffer, std::vector<Model*> &models)
{
	clearDepthBuffer(depthBuffer, window);
	window.clearPixels();

	camera.updateTransform();

	for (int m = 0; m < models.size(); m++)
	{
		Model& model = *models[m];
		for (int i = 0; i < model.triangles->size(); i++)
		{
			ModelTriangle tri = model.triangles->at(i);
			ModelVertex& mv1 = model.verts->at(tri.vertices[0]);
			ModelVertex& mv2 = model.verts->at(tri.vertices[1]);
			ModelVertex& mv3 = model.verts->at(tri.vertices[2]);
			glm::vec3 v1 = camera.getCanvasIntersectionPoint(glm::vec4(mv1.pos, 1));
			glm::vec3 v2 = camera.getCanvasIntersectionPoint(glm::vec4(mv2.pos, 1));
			glm::vec3 v3 = camera.getCanvasIntersectionPoint(glm::vec4(mv3.pos, 1));
			auto p1 = CanvasPoint(v1.x, v1.y, v1.z);
			p1.texturePoint = TexturePoint(mv1.texcoord.x / p1.depth, mv1.texcoord.y / p1.depth);
			p1.depth = 1 / p1.depth;
			auto p2 = CanvasPoint(v2.x, v2.y, v2.z);
			p2.texturePoint = TexturePoint(mv2.texcoord.x / p2.depth, mv2.texcoord.y / p2.depth);
			p2.depth = 1 / p2.depth;
			auto p3 = CanvasPoint(v3.x, v3.y, v3.z);
			p3.texturePoint = TexturePoint(mv3.texcoord.x / p3.depth, mv3.texcoord.y / p3.depth);
			p3.depth = 1 / p3.depth;
			CanvasTriangle triangle(p1,p2,p3);
			fillTriangle(triangle, model.material, depthBuffer, window);
		}
	}

	for (int i = 0; i < window.width; i++)
	{
		for (int j = 0; j < window.height; j++)
		{
			if (depthBuffer[i][j] == 0)
			{
				glm::vec3 rayDir = camera.getRayDirection(i, j);
				window.setPixelColour(i, j, colourToInt(vectorToColour(camera.environment->sampleEnvironment(rayDir))));
			}
		}	
	}
}


void traceDraw(DrawingWindow &window, std::vector<Model*> &models, std::vector<Light> &lights)
{
	window.clearPixels();

	camera.updateTransform();
	
	for (int i = 0; i < window.width; i++)
	{
		for (int j = 0; j < window.height; j++)
		{
			Colour col = camera.renderTraced(i, j, models, lights);
			uint32_t intCol = colourToInt(col);
			window.setPixelColour(i, j, intCol);
		}
		window.renderFrame();
	}
}

void traceDrawGouraud(DrawingWindow &window, std::vector<Model*> &models, std::vector<Light> &lights)
{
	window.clearPixels();

	camera.updateTransform();

	std::vector<std::vector<glm::vec3>> vertexColours = std::vector<std::vector<glm::vec3>>();
	camera.initialiseGouraud(models, lights, vertexColours);
	
	for (int i = 0; i < window.width; i++)
	{
		for (int j = 0; j < window.height; j++)
		{
			Colour col = camera.renderTracedGouraud(i, j, models, lights, vertexColours);
			uint32_t intCol = colourToInt(col);
			window.setPixelColour(i, j, intCol);
		}
	}
}

void createSoftLight(std::vector<Light> &lights, glm::vec3 centerPos, glm::vec3 colour, int segments, int heightSegments, float size, int shells = 1)
{
	int samples = segments * heightSegments * shells + 2;
	colour /= samples;
	float angleIncrement = 2 * M_PI / segments;
	// float heightCenter = (heightSegments) / 2.0f;
	for (int r = 0; r < shells; r++)
	{
		float radius = size * ((float)(r + 1) / shells);
		for (int i = 0; i < segments; i++)
		{
			float angle = i * angleIncrement;
			for (int j = 0; j < heightSegments; j++)
			{
				float tj = (float)(j + 1) / (heightSegments + 1);
				//float tj = 4.0f * (float)(j - heightCenter + 1) / (heightSegments + 1);
				float cosj = cos(tj * M_PI);
				float sinj = sin(tj * M_PI);
				glm::vec3 offset = glm::vec3(sin(angle) * sinj, cosj, cos(angle) * sinj);
				lights.emplace_back(centerPos + offset * radius, colour);
			}
		}
	}

	lights.emplace_back(centerPos + glm::vec3(0,size,0), colour);
	lights.emplace_back(centerPos + glm::vec3(0,-size,0), colour);
}

int main(int argc, char *argv[]) {
	DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
	SDL_Event event;

	std::unordered_map<std::string, Material*> *materials = new std::unordered_map<std::string, Material*>();

	Environment* environment = new Environment("/Users/smb/Desktop/Graphics-Coursework/src/dancing_hall_1k.ppm"); 

	loadMtl(*materials, "/Users/smb/Desktop/Graphics-Coursework/src/cornell-box.mtl");
	std::vector<Model*> *models = new std::vector<Model*>();
	loadObj(*models, "/Users/smb/Desktop/Graphics-Coursework/src/cornell-box.obj", *materials, 0.35f);
	std::vector<Light> lights = std::vector<Light>();
	//lights.push_back(Light(glm::vec3(0, 0.8f, 0), glm::vec3(10,10,10)));
	createSoftLight(lights, glm::vec3(0, 0.8f, 0), glm::vec3(10,10,10), 3, 2, 0.10f, 1);
	float angle = 0;
	auto cameraToWorld = matrixTRS(glm::vec3(0,0,3), glm::vec3(0,0,0));
	camera = Camera(200, cameraToWorld, window.width, window.height, environment);
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
	int frame = 0;
	
	while (true) {
		// We MUST poll for events - otherwise the window will freeze !
		if (window.pollForInputEvents(event)) handleEvent(event, window);
		if (!rendered)
		{
			angle += 0.01;
		}
		camera.cameraToWorld = matrixTRS(glm::vec3(0, sin(angle + 2) * 2, 3 + cos(angle) * 1.5f), glm::vec3(0,0,M_PI));
		camera.cameraToWorld = lookAt(camera.cameraToWorld, glm::vec3(0,0,0));

		if (renderMode == 0)
		{
			wireframeDraw(window, *models);
			rendered = false;
		}
		else if (renderMode == 1)
		{
			rasteriseDraw(window, depthBuffer, *models);
			rendered = false;
		}
		else if (renderMode == 2 && !rendered)
		{
			if (GouraudShading)
			{
				traceDrawGouraud(window, *models, lights);
			}
			else 
			{
				traceDraw(window, *models, lights);
			}

			//rendered = true;
			window.savePPM("/Users/smb/Desktop/Graphics-Coursework/output/" + std::to_string(frame) + ".ppm");
			frame++;
		}

		window.renderFrame();
	}

	for (auto& item: *materials)
	{
		delete item.second;
	}

	for (int i = 0; i < models->size(); i++)
	{
		delete models->at(i);
	}
	
	delete environment;
	delete materials;
	delete models;
	
	for (int i = 0; i < window.width; i++)
	{
		delete [] depthBuffer[i];
	}

	delete [] depthBuffer;
}
