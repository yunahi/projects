#include "cloth.h"
#include <iostream>
#include <QCoreApplication>
#include <QtConcurrent/QtConcurrent>
#include <QThread>

Cloth::Cloth()
{
}

Cloth::Cloth(int param1) :
    m_param1(param1)
{

    buildVertexData();
    initialVertex();
    setVertex();
    buildVAO();
}

Cloth::~Cloth()
{
}


void Cloth::buildVertexData(){
    m_vertexData.empty();
}

void Cloth::initialVertex(){
    int dimension = 10;
    m_position.clear();
    m_velocity.clear();
    for (int row = 0; row < dimension; row ++){
        for (int col = 0; col < dimension; col++){
            float x = (float(col)/(dimension - 1)) - 0.5;
            float y = 0.5 - (float(row)/(dimension - 1));
            m_position.push_back(glm::vec3(x,y,0));
            m_velocity.push_back(glm::vec3(0,0,0));
        }
    }
}

void Cloth::setVertex(){
    int dimension = 10;
    m_vertexData.clear();
    glm::vec3 frontNormal(0,0,-1);
    glm::vec3 backNormal(0,0,1);

    for (int row = 0; row < dimension - 1; row ++){
        for (int col = 0; col < dimension - 1; col++ ){
            glm::vec3 topLeft = m_position.at(row * dimension + col);
            glm::vec3 topRight = m_position.at(row * dimension + (col + 1));
            glm::vec3 bottomLeft = m_position.at((row + 1) * dimension + col);
            glm::vec3 bottomRight = m_position.at((row + 1) * dimension + (col + 1));
            setVertexHelper(bottomLeft,topRight,topLeft,frontNormal);
            setVertexHelper(bottomRight,topRight,bottomLeft,frontNormal);
            setVertexHelper(bottomLeft,topLeft,topRight,backNormal);
            setVertexHelper(bottomRight,bottomLeft,topRight,backNormal);
        }
    }
}

void Cloth::setVertexHelper(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 normal){


    insertVec3(m_vertexData,p1);
    insertVec3(m_vertexData,normal);

    insertVec3(m_vertexData,p2);
    insertVec3(m_vertexData,normal);

    insertVec3(m_vertexData,p3);
    insertVec3(m_vertexData,normal);

}

void Cloth::update(){
    updateVertex();
    setVertex();
    buildVAO();
}

std::vector<std::vector<glm::vec3>> Cloth::updateVertexHelper(int row,std::vector<glm::vec3> position,std::vector<glm::vec3> velocity){
    std::vector<std::vector<glm::vec3>> result;
    std::vector<glm::vec3> nextVelocity;
    std::vector<glm::vec3> nextPosition;

    int dimension = 10;
    int particleMass = 3;
    float step = 0.005;
    for (int col = 0; col < dimension; col++){
        glm::vec3 acceleration = netForce(row,col,position,velocity)/(particleMass*0.1f);
        nextVelocity.push_back(velocity[row*dimension+col] + acceleration * step);
        nextPosition.push_back(position[row*dimension+col] + nextVelocity[col] * step);
    }
    result.push_back(nextPosition);
    result.push_back(nextVelocity);

    return result;
}

void Cloth::updateVertex(){
    int dimension = 10;
    int particleMass = 3;
    float step = 0.005;

    std::vector<QFuture<std::vector<std::vector<glm::vec3>>>> threads;

    for (int row = 0; row < dimension; row++){
        threads.push_back(QtConcurrent::run(updateVertexHelper,row,m_position,m_velocity));
    }

    for (int i = 0; i < threads.size();i++){
        std::vector<std::vector<glm::vec3>>ret = threads[i].result();
        m_nextPosition.insert(m_nextPosition.end(), ret[0].begin(), ret[0].end());
        m_nextVelocity.insert(m_nextVelocity.end(), ret[1].begin(), ret[1].end());

    }
    m_nextVelocity[0] = glm::vec3(0,0,0);
    m_nextPosition[0] = glm::vec3(-0.5,0.5,0);
    m_nextVelocity[dimension - 1] = glm::vec3(0,0,0);
    m_nextPosition[dimension - 1] = glm::vec3(0.5,0.5,0);
    //transfer and clear
    m_position = m_nextPosition;
    m_velocity = m_nextVelocity;
    m_nextPosition.clear();
    m_nextVelocity.clear();


}

glm::vec3 Cloth::netForce(int row, int col,
                          std::vector<glm::vec3> position,std::vector<glm::vec3> velocity){
    int dimension = 10;
    glm::vec3 netForce(0,0,0);


    float rs =  (1.f/(dimension - 1));
    float rsh = (sqrt(2)/(dimension - 1));
    float rb= (2.f/(dimension - 1));

    netForce += structuralForce(row,col,30,rs,position);
    netForce += shearForce(row,col, 30,rsh,position);
    netForce += bendForce(row,col, 30,rb,position);
    netForce += gravityForce();
    netForce += windForce();
    netForce += dampingForce(velocity[row * dimension + col]);

    return netForce;
}

bool Cloth::validIndex(int row, int col){
    int dimension = 10;
    if (row >=dimension)
        return false;
    if (row < 0)
        return false;
    if (col >= dimension)
        return false;
    if (col < 0)
        return false;
    return true;
}

glm::vec3 Cloth::springForce(glm::vec3 p, glm::vec3 q, float stiffness, float restLength){
    glm::vec3 e = p - q;
    float length = glm::length(e);
    return glm::vec3(((stiffness)  *100* (restLength - length) * 1.f/length) * e);
}

glm::vec3 Cloth::dampingForce(glm::vec3 velocity){
    float damping = 1;
    return damping * 0.1f * velocity;
}

glm::vec3 Cloth::gravityForce(){
    float particleMass = 3;
    float g = -9.8;
    return glm::vec3 (0,(particleMass*0.1) * g,0);
}

glm::vec3 Cloth::windForce(){
    float windAngle = 0;
    float windVelocity = 5;
    float radian = windAngle*M_PI/180.f;
    float unknownVal = 0.12 * 0.5 * 1.2 * pow(windVelocity,2);

    return glm::vec3(unknownVal * sin(radian), 0, unknownVal * cos(radian));
}

glm::vec3 Cloth::structuralForce(int row, int col,
                                 float stiffness, float restLength,
                                 std::vector<glm::vec3> position){
    int dimension = 10;
    glm::vec3 force(0,0,0);
    if (validIndex(row + 1, col))
        force += springForce(position[row*dimension+col],position[(row + 1) * dimension + (col)],
                                stiffness,restLength);
    if (validIndex(row - 1, col))
        force += springForce(position[row*dimension+col],position[(row - 1) * dimension + (col)],
                                stiffness,restLength);
    if (validIndex(row, col + 1))
        force += springForce(position[row*dimension+col],position[(row) * dimension + (col + 1)],
                                stiffness,restLength);
    if (validIndex(row, col - 1))
        force += springForce(position[row*dimension+col],position[(row) * dimension + (col - 1)],
                                stiffness,restLength);
    return force;
}

glm::vec3 Cloth::shearForce(int row, int col, float stiffness, float restLength,
                            std::vector<glm::vec3> position){
    int dimension = 10;
    glm::vec3 force(0,0,0);

    if (validIndex(row + 1, col + 1))
        force += springForce(position[row*dimension+col],position[(row + 1)*dimension+(col + 1)],
                                stiffness,restLength);
    if (validIndex(row + 1, col - 1))
        force += springForce(position[row*dimension+col],position[(row + 1)*dimension+(col - 1)],
                                stiffness,restLength);
    if (validIndex(row - 1, col + 1))
        force += springForce(position[row*dimension+col],position[(row - 1)*dimension+(col + 1)],
                                stiffness,restLength);
    if (validIndex(row - 1, col - 1))
        force += springForce(position[row*dimension+col],position[(row - 1)*dimension+(col - 1)],
                                stiffness,restLength);
    return force;
}

glm::vec3 Cloth::bendForce(int row, int col, float stiffness, float restLength,
                           std::vector<glm::vec3> position){
    int dimension = 10;
    glm::vec3 force(0,0,0);

    if (validIndex(row + 2, col))
        force += springForce(position[row*dimension+col],position[(row + 2)*dimension+(col)],
                                stiffness,restLength);
    if (validIndex(row - 2, col))
        force += springForce(position[row*dimension+col],position[(row - 2)*dimension+(col)],
                                stiffness,restLength);
    if (validIndex(row, col + 2))
        force += springForce(position[row*dimension+col],position[(row)*dimension+(col + 2)],
                                stiffness,restLength);
    if (validIndex(row, col - 2))
        force += springForce(position[row*dimension+col],position[(row)*dimension+(col - 2)],
                                stiffness,restLength);
    return force;

}



