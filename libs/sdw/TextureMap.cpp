#include "TextureMap.h"

const float UVScale = 20;

TextureMap::TextureMap() = default;
TextureMap::TextureMap(const std::string &filename) {
	std::ifstream inputStream(filename, std::ifstream::binary);
	std::string nextLine;
	// Get the "P6" magic number
	std::getline(inputStream, nextLine);
	// Read the width and height line
	std::getline(inputStream, nextLine);
	// Skip over any comment lines !
	while (nextLine.at(0) == '#') std::getline(inputStream, nextLine);
	auto widthAndHeight = split(nextLine, ' ');
	if (widthAndHeight.size() != 2)
		throw std::invalid_argument("Failed to parse width and height line, line was `" + nextLine + "`");

	width = std::stoi(widthAndHeight[0]);
	height = std::stoi(widthAndHeight[1]);
	// Read the max value (which we assume is 255)
	std::getline(inputStream, nextLine);

	pixels.resize(width * height);
	for (size_t i = 0; i < width * height; i++) {
		int red = inputStream.get();
		int green = inputStream.get();
		int blue = inputStream.get();
		pixels[i] = ((255 << 24) + (red << 16) + (green << 8) + (blue));
	}
	inputStream.close();
}

uint32_t TextureMap::sample(float u, float v)
{
    u = u < 0 ? 1 - fmod(-u * UVScale, 1) : fmod(u * UVScale, 1);
    v = v < 0 ? 1 - fmod(-v * UVScale, 1) : fmod(v * UVScale, 1);
	return pixels[floor(v * height) * width + floor(u * width)];
}

std::ostream &operator<<(std::ostream &os, const TextureMap &map) {
	os << "(" << map.width << " x " << map.height << ")";
	return os;
}
