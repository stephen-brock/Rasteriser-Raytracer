#include "KdTree.h"

Node::Node() = default;

Node::Node(glm::vec3 &location, glm::vec3 &colour)
{
    this->location = location;
    this->colour = colour;
}

glm::vec3 Node::Search(glm::vec3 &location, float &closestDistance, int depth)
{
    int axis = depth % 3;
    Node* nextNode = location[axis] > this->location[axis] ? rightChild : leftChild;
    glm::vec3 dir = this->location - location;
    glm::vec3 col = colour;
    closestDistance = glm::dot(dir, dir);
    
    if (nextNode != nullptr)
    {
        float childDistance = -1;
        glm::vec3 childCol = nextNode->Search(location, childDistance, depth + 1);

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
            glm::vec3 childCol = otherNode->Search(location, childDistance, depth + 1);

            if (childDistance < closestDistance)
            {
                closestDistance = childDistance;
                col = childCol;
            }
        }
    }

    return col;
}

void Node::SearchKNeighbours(glm::vec3 &location, std::array<glm::vec3, K_NEIGHBOURS> &colours, std::array<float, K_NEIGHBOURS> &sqrDistances, int depth)
{
    int axis = depth % 3;
    Node* nextNode = location[axis] > this->location[axis] ? rightChild : leftChild;
    glm::vec3 dir = this->location - location;
    glm::vec3 col = colour;
    float thisDistance = glm::dot(dir, dir);
    setMinimumDistance(colours, sqrDistances, col, thisDistance);
    
    if (nextNode != nullptr)
    {
        std::array<glm::vec3, K_NEIGHBOURS> nextCols;
        std::array<float, K_NEIGHBOURS> nextDsts;
        for (int i = 0; i < K_NEIGHBOURS; i++)
        {
            nextDsts[i] = 10000000;
        }
        nextNode->SearchKNeighbours(location, nextCols, nextDsts, depth + 1);
        for (int i = 0; i < K_NEIGHBOURS; i++)
        {
            setMinimumDistance(colours, sqrDistances, nextCols[i], nextDsts[i]);
        }
        
    }

    Node* otherNode = location[axis] > this->location[axis] ? leftChild : rightChild;
    if (otherNode != nullptr)
    {
        glm::vec3 otherDir = location - this->location;
        // does hypersphere cross the hyperplane
        if (otherDir[axis] * otherDir[axis] <= sqrDistances[K_NEIGHBOURS - 1])
        {
            std::array<glm::vec3, K_NEIGHBOURS> nextCols;
            std::array<float, K_NEIGHBOURS> nextDsts;
            for (int i = 0; i < K_NEIGHBOURS; i++)
            {
                nextDsts[i] = 10000000;
            }
            otherNode->SearchKNeighbours(location, nextCols, nextDsts, depth + 1);
            for (int i = 0; i < K_NEIGHBOURS; i++)
            {
                setMinimumDistance(colours, sqrDistances, nextCols[i], nextDsts[i]);
            }
        }
    }
}

void Node::setMinimumDistance(std::array<glm::vec3, K_NEIGHBOURS> &colours, std::array<float, K_NEIGHBOURS> &sqrDistances, glm::vec3 col, float sqrDst)
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

void Node::Insert(glm::vec3 &location, glm::vec3 &colour, int depth)
{
    int axis = depth % 3;
    int value = location[axis];
    int thisValue = location[axis];
    // std::cout << depth << std::endl;
    if (value > thisValue)
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

KdTree::KdTree(std::vector<glm::vec3> &location, std::vector<glm::vec3> &data)
{
    root = new Node(location[0], data[0]);
    std::vector<float> test = std::vector<float>();
    test.push_back(1);
    test.push_back(2);
    test.push_back(4);
    for (int i = 1; i < data.size(); i++)
    {
        root->Insert(location[i], data[i]);
    }
}
glm::vec3 KdTree::Search(glm::vec3 &location, float &sqrDistance)
{
    return root->Search(location, sqrDistance);
}

std::array<glm::vec3, K_NEIGHBOURS> KdTree::SearchKNeighbours(glm::vec3 &location, std::array<float, K_NEIGHBOURS> &sqrDistances)
{
    std::array<glm::vec3, K_NEIGHBOURS> colours = std::array<glm::vec3, K_NEIGHBOURS>();
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