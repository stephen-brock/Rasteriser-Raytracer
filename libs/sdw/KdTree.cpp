#include "KdTree.h"

Node::Node() = default;

Node::Node(glm::vec3 &location, Photon &colour)
{
    this->location = location;
    this->colour = colour;
}

Photon Node::Search(glm::vec3 &location, float &closestDistance, int depth)
{
    int axis = depth % 3;
    Node* nextNode = location[axis] > this->location[axis] ? rightChild : leftChild;
    glm::vec3 dir = this->location - location;
    Photon col = colour;
    closestDistance = glm::dot(dir, dir);
    
    if (nextNode != nullptr)
    {
        float childDistance = -1;
        Photon childCol = nextNode->Search(location, childDistance, depth + 1);

        if (childDistance < closestDistance)
        {
            closestDistance = childDistance;
            col = childCol;
        }
    }

    Node* otherNode = location[axis] > this->location[axis] ? leftChild : rightChild;
    if (otherNode != nullptr)
    {
        glm::vec3 otherDir = location - this->location;
        //does hypersphere cross the hyperplane
        if (otherDir[axis] * otherDir[axis] <= closestDistance)
        {
            float childDistance = -1;
            Photon childCol = otherNode->Search(location, childDistance, depth + 1);

            if (childDistance < closestDistance)
            {
                closestDistance = childDistance;
                col = childCol;
            }
        }
    }

    return col;
}

void Node::SearchKNeighbours(glm::vec3 &location, std::array<Photon, K_NEIGHBOURS> &colours, std::array<float, K_NEIGHBOURS> &sqrDistances, int depth)
{
    int axis = depth % 3;
    glm::vec3 dir = location - this->location;
    float dirValue = dir[axis];
    Node* nextNode = dirValue > 0 ? rightChild : leftChild;
    Node* otherNode = dirValue > 0 ? leftChild : rightChild;
    Photon col = colour;
    
    if (nextNode != nullptr)
    {
        nextNode->SearchKNeighbours(location, colours, sqrDistances, depth + 1);
    }

    float thisDistance = glm::dot(dir, dir);
    setMinimumDistance(colours, sqrDistances, col, thisDistance);
    if (otherNode != nullptr && dirValue * dirValue <= sqrDistances[K_NEIGHBOURS - 1])
    {
        otherNode->SearchKNeighbours(location, colours, sqrDistances, depth + 1);
    }
}

void Node::setMinimumDistance(std::array<Photon, K_NEIGHBOURS> &colours, std::array<float, K_NEIGHBOURS> &sqrDistances, Photon col, float sqrDst)
{
    int replacedIndex = -1;
    for (int i = 0; i < K_NEIGHBOURS && replacedIndex == -1; i++)
    {
        if (sqrDst < sqrDistances[i])
        {
            replacedIndex = i;
        }
    }

    if (replacedIndex != -1)
    {
        for (int i = K_NEIGHBOURS - 1; i > replacedIndex; i--)
        {
            std::swap(colours[i], colours[i - 1]);
            std::swap(sqrDistances[i], sqrDistances[i - 1]);
        }

        colours[replacedIndex] = col;
        sqrDistances[replacedIndex] = sqrDst;
    }
}

Node::~Node()
{
    delete leftChild;
    delete rightChild;
}

void Node::Insert(glm::vec3 &location, Photon &colour, int depth)
{
    int axis = depth % 3;
    float dir = location[axis] - this->location[axis];
    if (dir < 0)
    {
        if (leftChild == nullptr)
        {
            leftChild = new Node(location, colour);
        }
        else 
        {
            leftChild->Insert(location, colour, depth+1);
        }
    }
    else 
    {
        if (rightChild == nullptr)
        {
            rightChild = new Node(location, colour);
        }
        else 
        {
            rightChild->Insert(location, colour, depth+1);
        }
    }
}

KdTree::KdTree() = default;

KdTree::KdTree(std::vector<glm::vec3> &location, std::vector<Photon> &data)
{
    root = new Node(location[0], data[0]);
    for (int i = 1; i < data.size(); i++)
    {
        root->Insert(location[i], data[i]);
    }
}

Photon KdTree::Search(glm::vec3 &location, float &sqrDistance)
{
    return root->Search(location, sqrDistance);
}

std::array<Photon, K_NEIGHBOURS> KdTree::SearchKNeighbours(glm::vec3 &location, std::array<float, K_NEIGHBOURS> &sqrDistances)
{
    std::array<Photon, K_NEIGHBOURS> colours = std::array<Photon, K_NEIGHBOURS>();
    for (int i = 0; i < K_NEIGHBOURS; i++)
    {
        sqrDistances[i] = 10000000;
    }
    
    root->SearchKNeighbours(location, colours, sqrDistances);
    return colours;
}

KdTree::~KdTree()
{
    delete root;
}