#ifndef CLOTH_H
#define CLOTH_H

#include "Shape.h"

class Cloth : public Shape
{
public:
    Cloth();
    Cloth(int param1);
    ~Cloth();

private:
    int m_param1;
    std::vector<glm::vec3> m_position;
    std::vector<glm::vec3> m_velocity;
    std::vector<glm::vec3> m_nextPosition;
    std::vector<glm::vec3> m_nextVelocity;
    void update() override;

protected:
    void buildVertexData() override;
    void initialVertex();
    void setVertex();
    void setVertexHelper(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 normal);
    void updateVertex();
    static std::vector<std::vector<glm::vec3>> updateVertexHelper(int row,std::vector<glm::vec3> position,std::vector<glm::vec3> velocity);

    static glm::vec3 netForce(int row, int col,
                       std::vector<glm::vec3> position,std::vector<glm::vec3> velocity);
    static bool validIndex(int row, int col);
    static glm::vec3 springForce(glm::vec3 p, glm::vec3 q, float stiffness, float restLength);
    static glm::vec3 dampingForce(glm::vec3 velocity);
    static glm::vec3 gravityForce();
    static glm::vec3 windForce();
    static glm::vec3 structuralForce(int row, int col,
                              float stiffness, float restLength,
                              std::vector<glm::vec3> position);
    static glm::vec3 shearForce(int row, int col,
                         float stiffness, float restLength,
                         std::vector<glm::vec3> position);
    static glm::vec3 bendForce(int row, int col,
                        float stiffness, float restLength,
                        std::vector<glm::vec3> position);


};

#endif // CUBE_H
