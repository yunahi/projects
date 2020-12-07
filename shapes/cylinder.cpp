#include "cylinder.h"
#include <iostream>
#include <math.h>


Cylinder::Cylinder()
{
    std::cout<<"not here"<<std::endl;
}

Cylinder::Cylinder(int param1, int param2) :
    SpunShape(std::max(1,param1),std::max(3,param2),0)

{
    buildVertexData();
    buildVAO();
}

Cylinder::~Cylinder()
{
}


void Cylinder::buildVertexData(){

    m_vertexData.empty();
    spin(topCap,topCapNormal,-1);
    spin(bottomCap,bottomCapNormal,1);
    spin(wrap,wrapNormal,1);

}

glm::vec3 Cylinder::topCap(float p1, float p2, float p3, float param1, float param2,float param3){
    float ang = 4*acos(0.0)/param2;
    float rad = 0.5;
    return glm::vec3(rad * cos(ang*p2) * p1/param1,
                     0.5,
                     rad*sin(ang*p2)* p1/param1);
}
glm::vec3 Cylinder::topCapNormal( float p1, float p2, float p3, float param1, float param2,float param3){
    return glm::vec3(0,1,0);
}
glm::vec3 Cylinder::bottomCap(float p1, float p2, float p3, float param1, float param2,float param3){
    float ang = 4*acos(0.0)/param2;
    float rad = 0.5;
    return glm::vec3(rad * cos(ang*p2) * p1/param1,
                     -0.5,
                     rad*sin(ang*p2)* p1/param1);
}
glm::vec3 Cylinder::bottomCapNormal(float p1, float p2, float p3, float param1, float param2,float param3){
    return glm::vec3(0,-1,0);
}
glm::vec3 Cylinder::wrap(float p1, float p2, float p3, float param1, float param2,float param3){
    float ang = 4*acos(0.0)/param2;
    float rad = 0.5;
    return glm::vec3(rad * cos(ang*p2),
                     -0.5 + p1/param1,
                     rad*sin(ang*p2));
}
glm::vec3 Cylinder::wrapNormal(float p1, float p2, float p3, float param1, float param2,float param3){
    float ang = 4*acos(0.0)/param2;
    float rad = 0.5;
    return glm::vec3(cos(ang*p2),
                     0,
                     sin(ang*p2));
}


