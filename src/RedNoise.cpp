#include <CanvasTriangle.h>
#include <DrawingWindow.h>
#include <glm/glm.hpp>
#include <Utils.h>
#include <fstream>
#include <vector>

#define WIDTH 320
#define HEIGHT 240

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

void draw(DrawingWindow &window, std::vector<std::vector<glm::vec3>> &pixels) {
	window.clearPixels();
	for (size_t y = 0; y < window.height; y++) {
		for (size_t x = 0; x < window.width; x++) {
			glm::vec3 col = pixels[y][x];
			float red = col.x;
			float green = col.y;
			float blue = col.z;
			uint32_t colour = (255 << 24) + (int(red) << 16) + (int(green) << 8) + int(blue);
			window.setPixelColour(x, y, colour);
		}
	}
}

void handleEvent(SDL_Event event, DrawingWindow &window) {
	if (event.type == SDL_KEYDOWN) {
		if (event.key.keysym.sym == SDLK_LEFT) std::cout << "LEFT" << std::endl;
		else if (event.key.keysym.sym == SDLK_RIGHT) std::cout << "RIGHT" << std::endl;
		else if (event.key.keysym.sym == SDLK_UP) std::cout << "UP" << std::endl;
		else if (event.key.keysym.sym == SDLK_DOWN) std::cout << "DOWN" << std::endl;
	} else if (event.type == SDL_MOUSEBUTTONDOWN) {
		window.savePPM("output.ppm");
		window.saveBMP("output.bmp");
	}
}

int main(int argc, char *argv[]) {
	DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
	SDL_Event event;
	std::vector<std::vector<glm::vec3>> pixels;
	std::vector<float> green = interpolateSingleFloats(0, 255, window.height);
	for (int j = 0; j < window.height; j++)
	{
		glm::vec3 from = glm::vec3(255, green[j], 0);
		glm::vec3 to = glm::vec3(0, green[j], 255 - green[j]);
		pixels.push_back(interpolateThreeElementValues(from, to, window.width));
	}
	
	while (true) {
		// We MUST poll for events - otherwise the window will freeze !
		if (window.pollForInputEvents(event)) handleEvent(event, window);
		draw(window, pixels);
		// Need to render the frame at the end, or nothing actually gets shown on the screen !
		window.renderFrame();
	}
}
