#pragma once
#include <fstream>
#include <unordered_map>
#include <fstream>
#include <vector>
#include <iostream>
#include <string>
#include <glm/glm.hpp>
#include "ModelTriangle.h"
#include "ModelVertex.h"

int getSubStringIndex(std::string s, char delimiter, int startIndex);

std::string stringRange(std::string s, int from, int to);

ModelVertex vertFromString(std::string s, float scale);

ModelTriangle triFromString(std::string s, std::vector<ModelVertex> &verts, Colour col);

std::string getMatNameFromString(std::string s);

Colour getColourFromString(std::string s);

std::unordered_map<std::string, Colour> loadMtl(std::string path);

std::vector<ModelTriangle> loadObj(std::string path, std::unordered_map<std::string, Colour> &materials, std::vector<ModelVertex> &verts, float scale);