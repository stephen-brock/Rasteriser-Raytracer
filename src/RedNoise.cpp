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

#define WIDTH 640
#define HEIGHT 480
#define SUPER_SAMPLE 1

const bool GouraudShading = false;
const glm::vec3 WindowPosition = glm::vec3(0,0.35f,1.2);

Camera camera;
int renderMode = 1;

uint32_t colourToInt(Colour colour) 
{
	if (colour.red < 0 || colour.green < 0 || colour.blue < 0)
	{
		std::cout << colour << std::endl;
	}
	return 255 << 24 | colour.red << 16 | colour.green << 8 | colour.blue;
}

CanvasPoint lerpCanvasPoint(CanvasPoint v1, CanvasPoint v2, double t)
{
	double x = v2.x * t + v1.x * (1 - t);
	double y = v2.y * t + v1.y * (1 - t);
	double z = v2.depth * t + v1.depth * (1 - t);

	CanvasPoint point = CanvasPoint(x, y, z);
	point.texturePoint.x = v2.texturePoint.x * t + v1.texturePoint.x * (1 - t);
	point.texturePoint.y = v2.texturePoint.y * t + v1.texturePoint.y * (1 - t);

	return point;
}

void drawLine(CanvasPoint from, CanvasPoint to, Colour colour, DrawingWindow &window) 
{
	if (from.x > to.x)
	{
		std::swap(from, to);
	}

	if (to.x < 0 || from.x >= window.width)
	{
		return;
	}
	double difference = to.x - from.x;
	if (from.x < 0)
	{
		double t = -from.x / difference;
		from = lerpCanvasPoint(from, to, t);
		from.x = 0;
	}

	if (to.x >= window.width)
	{
		double t = 1 - (to.x - window.width + 1) / difference;
		to = lerpCanvasPoint(from, to, t);
		to.x = window.width - 1;
	}
	if (from.y > to.y)
	{
		std::swap(from, to);
	}

	if (to.y < 0 || from.y >= window.height)
	{
		return;
	}

	difference = to.y - from.y;
	if (from.y < 0)
	{
		double t = -from.y / difference;
		from = lerpCanvasPoint(from, to, t);
		from.y = 0;
	}

	if (to.y >= window.height)
	{
		double t = 1 - (to.y - window.height + 1) / difference;
		to = lerpCanvasPoint(from, to, t);
		to.y = window.height - 1;
	}

	glm::vec2 diff = glm::vec2(to.x - from.x, to.y - from.y);
	float numberOfSteps = fmax(fabs(diff.x), fabs(diff.y));
	for (int i = 0; i < numberOfSteps; i++)
	{
		double t = (double)i / (numberOfSteps - 1);
		CanvasPoint p = lerpCanvasPoint(from, to, t);
		if (p.depth < 0)
		{
			continue;
		}
		window.setPixelColour(p.x, p.y, colourToInt(colour));
	}
}

void strokeTriangle(CanvasTriangle triangle, Colour colour, DrawingWindow &window) 
{
	drawLine(triangle[0], triangle[1], colour, window);
	drawLine(triangle[1], triangle[2], colour, window);
	drawLine(triangle[2], triangle[0], colour, window);
}

CanvasPoint centerPoint(CanvasTriangle sortedTri) 
{
	double yDiff = sortedTri[2].y - sortedTri[0].y;
	double t = (sortedTri[1].y - sortedTri[0].y) / yDiff;

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
			glm::vec3 mv1 = model.transformedVerts->at(tri.vertices[0]).pos;
			glm::vec3 mv2 = model.transformedVerts->at(tri.vertices[1]).pos;
			glm::vec3 mv3 = model.transformedVerts->at(tri.vertices[2]).pos;
			glm::vec3 v1 = camera.getCanvasIntersectionPoint(glm::vec4(mv1, 1));
			glm::vec3 v2 = camera.getCanvasIntersectionPoint(glm::vec4(mv2, 1));
			glm::vec3 v3 = camera.getCanvasIntersectionPoint(glm::vec4(mv3, 1));
			auto p1 = CanvasPoint(v1.x, v1.y, v1.z);
			auto p2 = CanvasPoint(v2.x, v2.y, v2.z);
			auto p3 = CanvasPoint(v3.x, v3.y, v3.z);
			CanvasTriangle triangle(p1,p2,p3);
			strokeTriangle(triangle, tri.colour, window);
			// auto col = colourToInt(tri.colour);
			// window.setPixelColour(v1.x, v1.y, col);
			// window.setPixelColour(v2.x, v2.y, col);
			// window.setPixelColour(v3.x, v3.y, col);
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
			ModelVertex& mv1 = model.transformedVerts->at(tri.vertices[0]);
			ModelVertex& mv2 = model.transformedVerts->at(tri.vertices[1]);
			ModelVertex& mv3 = model.transformedVerts->at(tri.vertices[2]);
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

void traceDraw(DrawingWindow &window, std::vector<Model*> &models, std::vector<Light> &lights, SDL_Event &event)
{
	window.clearPixels();

	camera.updateTransform();

	KdTree* photon_map = camera.renderPhotonMap(models, lights, 20000, 0.5f, WindowPosition, 7.0f, 0.1f);

	for (int i = 0; i < window.width; i++)
	{
		for (int j = 0; j < window.height; j++)
		{
			glm::vec3 sum = glm::vec3(0,0,0);
			for (int x = 0; x < SUPER_SAMPLE; x++)
			{
				float subX = x - SUPER_SAMPLE / 2.0f;
				
				for (int y = 0; y < SUPER_SAMPLE; y++)
				{
					float subY = y - SUPER_SAMPLE / 2.0f;
					
					sum += camera.renderTracedBaked(i + 0.5f + subX, j + 0.5f + subY, models, lights, photon_map);
				}
			}
			sum /= SUPER_SAMPLE * SUPER_SAMPLE;
			uint32_t intCol = colourToInt(vectorToColour(sum));
			window.setPixelColour(i, j, intCol);
		}
		if (window.pollForInputEvents(event)) handleEvent(event, window);
		window.renderFrame();
	}

	delete photon_map;
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
	for (int r = 0; r < shells; r++)
	{
		float radius = size * ((float)(r + 1) / shells);
		for (int i = 0; i < segments; i++)
		{
			float angle = i * angleIncrement;
			for (int j = 0; j < heightSegments; j++)
			{
				float tj = (float)(j + 1) / (heightSegments + 1);
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

void cameraAnimation(glm::vec3 &cameraPosition, glm::vec3 &lookAt, int frame)
{
	lookAt = glm::vec3(cos(frame * 0.05f) * 1.0f,0.1f,-.5f);
	cameraPosition = lookAt + glm::vec3(-cos(frame * 0.05f) * 1.0f, -0.1f,1.5f);
}

int main(int argc, char *argv[]) {
	DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
	SDL_Event event;

	std::unordered_map<std::string, Material*> *materials = new std::unordered_map<std::string, Material*>();

	Environment* environment = new Environment("./src/studio_small_03_1k.ppm", glm::vec3(.25f, .25f, .25f)); 

	loadMtl(*materials, "./src/scene.mtl");
	std::vector<Model*> *models = new std::vector<Model*>();
	// loadObjOld(*models, "./src/cornell-box.obj", *materials, 0.35f);
	loadObj(*models, "./src/scene.obj", *materials, 0.35f);

	std::vector<Light> lights = std::vector<Light>();
	// lights.push_back(Light(glm::vec3(0.0f, .5f, 4.0f), glm::vec3(20000,20000,20000)));
	createSoftLight(lights, glm::vec3(0.0f, .5f, 4.0f), glm::vec3(20000,20000,20000), 3, 2, 0.05f, 2);
	auto cameraToWorld = matrixTRS(glm::vec3(0.3,-0.25f,.5f), glm::vec3(0,0,M_PI));
	camera = Camera(5.0f, cameraToWorld, window.width, window.height, environment);

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
		for (int i = 0; i < models->size(); i++)
	{
		models->at(i)->TransformVerticies();
	}

	bool rendered = false;
	int frame = 0;
	
	while (true) {
		// We MUST poll for events - otherwise the window will freeze !
		if (window.pollForInputEvents(event)) handleEvent(event, window);
		// glm::vec3 orbit = glm::vec3(0,-.5f,-1.0f);
		glm::vec3 orbit = glm::vec3(0,0,0);
		glm::vec3 camPos = glm::vec3(0,0,0);
		cameraAnimation(camPos, orbit, frame);
		camera.cameraToWorld = matrixTRS(camPos, glm::vec3(0,0,M_PI));
		camera.cameraToWorld = lookAt(camera.cameraToWorld, orbit);
		
		frame++;

		lights.clear();
		createSoftLight(lights, WindowPosition + glm::vec3(-cos(frame * 0.05f) * 3.0f, 1.0f + sin(frame * 0.02f) * 0.5f, 5.0f + fabs(sin(frame * 0.0f))), glm::vec3(20000,20000,20000), 3, 2, 0.15f, 2);

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
		else if (renderMode == 2)
		{
			if (GouraudShading)
			{
				traceDrawGouraud(window, *models, lights);
			}
			else 
			{
				traceDraw(window, *models, lights, event);
			}

			rendered = true;
			window.savePPM("/Users/smb/Desktop/Graphics-Coursework/output/" + std::to_string(frame) + ".ppm");
			// frame++;
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
