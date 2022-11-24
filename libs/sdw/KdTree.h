#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <array>
#define K_NEIGHBOURS 16

struct Node
{
    glm::vec3 location{};
    glm::vec3 colour{};
    Node* leftChild{};
    Node* rightChild{};
    Node();
    Node(glm::vec3 &location, glm::vec3 &colour);
    ~Node();
    glm::vec3 Search(glm::vec3 &location, float& closestDistance, int depth=0);
    void SearchKNeighbours(glm::vec3 &location, std::array<glm::vec3, K_NEIGHBOURS> &colours, std::array<float, K_NEIGHBOURS> &sqrDistances, int depth = 0);
    void Insert(glm::vec3 &location, glm::vec3 &colour, int depth=0);
    void setMinimumDistance(std::array<glm::vec3, K_NEIGHBOURS> &colours, std::array<float, K_NEIGHBOURS> &sqrDistances, glm::vec3 col, float sqrDst);
};

class KdTree
{
    public:
        KdTree();
        KdTree(std::vector<glm::vec3> &location, std::vector<glm::vec3> &data);
        glm::vec3 Search(glm::vec3 &location, float &sqrDistance);
        std::array<glm::vec3, K_NEIGHBOURS> SearchKNeighbours(glm::vec3 &location, std::array<float, K_NEIGHBOURS> &sqrDistances);
        ~KdTree();
    private:
        Node* root{};
};
