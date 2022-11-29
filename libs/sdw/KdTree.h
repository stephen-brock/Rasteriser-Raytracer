#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <array>
#include "Photon.h"
#define K_NEIGHBOURS 20

struct Node
{
    glm::vec3 location{};
    Photon colour{};
    Node* leftChild{};
    Node* rightChild{};
    std::array<Photon, K_NEIGHBOURS> colCache{};
    std::array<float, K_NEIGHBOURS> dstCache{};
    Node();
    Node(glm::vec3 &location, Photon &colour);
    ~Node();
    Photon Search(glm::vec3 &location, float& closestDistance, int depth=0);
    void SearchKNeighbours(glm::vec3 &location, std::array<Photon, K_NEIGHBOURS> &colours, std::array<float, K_NEIGHBOURS> &sqrDistances, int depth = 0);
    void Insert(glm::vec3 &location, Photon &colour, int depth=0);
    void setMinimumDistance(std::array<Photon, K_NEIGHBOURS> &colours, std::array<float, K_NEIGHBOURS> &sqrDistances, Photon col, float sqrDst);
};

class KdTree
{
    public:
        KdTree();
        KdTree(std::vector<glm::vec3> &location, std::vector<Photon> &data);
        Photon Search(glm::vec3 &location, float &sqrDistance);
        std::array<Photon, K_NEIGHBOURS> SearchKNeighbours(glm::vec3 &location, std::array<float, K_NEIGHBOURS> &sqrDistances);
        ~KdTree();
    private:
        Node* root{};
};
