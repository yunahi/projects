#include "SceneviewScene.h"
#include "GL/glew.h"
#include <QGLWidget>
#include "Camera.h"
#include <iostream>


#include "Settings.h"
#include "SupportCanvas3D.h"
#include "ResourceLoader.h"
#include "gl/shaders/CS123Shader.h"


#include "shapes/Shape.h"
#include "shapes/cube.h"
#include "shapes/cylinder.h"
#include "shapes/cloth.h"


using namespace CS123::GL;


SceneviewScene::SceneviewScene()
{
    // TODO: [SCENEVIEW] Set up anything you need for your Sceneview scene here...
    loadPhongShader();
    loadWireframeShader();
    loadNormalsShader();
    loadNormalsArrowShader();

}

SceneviewScene::~SceneviewScene()
{
}

void SceneviewScene::loadPhongShader() {
    std::string vertexSource = ResourceLoader::loadResourceFileToString(":/shaders/default.vert");
    std::string fragmentSource = ResourceLoader::loadResourceFileToString(":/shaders/default.frag");
    m_phongShader = std::make_unique<CS123Shader>(vertexSource, fragmentSource);
}

void SceneviewScene::loadWireframeShader() {
    std::string vertexSource = ResourceLoader::loadResourceFileToString(":/shaders/wireframe.vert");
    std::string fragmentSource = ResourceLoader::loadResourceFileToString(":/shaders/wireframe.frag");
    m_wireframeShader = std::make_unique<Shader>(vertexSource, fragmentSource);
}

void SceneviewScene::loadNormalsShader() {
    std::string vertexSource = ResourceLoader::loadResourceFileToString(":/shaders/normals.vert");
    std::string geometrySource = ResourceLoader::loadResourceFileToString(":/shaders/normals.gsh");
    std::string fragmentSource = ResourceLoader::loadResourceFileToString(":/shaders/normals.frag");
    m_normalsShader = std::make_unique<Shader>(vertexSource, geometrySource, fragmentSource);
}

void SceneviewScene::loadNormalsArrowShader() {
    std::string vertexSource = ResourceLoader::loadResourceFileToString(":/shaders/normalsArrow.vert");
    std::string geometrySource = ResourceLoader::loadResourceFileToString(":/shaders/normalsArrow.gsh");
    std::string fragmentSource = ResourceLoader::loadResourceFileToString(":/shaders/normalsArrow.frag");
    m_normalsArrowShader = std::make_unique<Shader>(vertexSource, geometrySource, fragmentSource);
}

void SceneviewScene::render(SupportCanvas3D *context) {
    setClearColor();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_phongShader->bind();
    setSceneUniforms(context);
    setLights();
    renderGeometry();
    glBindTexture(GL_TEXTURE_2D, 0);
    m_phongShader->unbind();




}


void SceneviewScene::setSceneUniforms(SupportCanvas3D *context) {
    Camera *camera = context->getCamera();
    m_phongShader->setUniform("useLighting", settings.useLighting);
    m_phongShader->setUniform("useArrowOffsets", false);
    m_phongShader->setUniform("p" , camera->getProjectionMatrix());
    m_phongShader->setUniform("v", camera->getViewMatrix());

}

void SceneviewScene::setMatrixUniforms(Shader *shader, SupportCanvas3D *context) {
    shader->setUniform("p", context->getCamera()->getProjectionMatrix());
    shader->setUniform("v", context->getCamera()->getViewMatrix());
}

void SceneviewScene::setLights()
{
    // TODO: [SCENEVIEW] Fill this in...
    //
    // Set up the lighting for your scene using m_phongShader.
    // The lighting information will most likely be stored in CS123SceneLightData structures.
    //

        for (int i = 0; i < m_lightList.size();i++){
            m_phongShader->setLight(m_lightList.at(i));
        }

}

void SceneviewScene::renderGeometry() {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // TODO: [SCENEVIEW] Fill this in...
    // You shouldn't need to write *any* OpenGL in this class!
    //
    //
    // This is where you should render the geometry of the scene. Use what you
    // know about OpenGL and leverage your Shapes classes to get the job done.
    //


    for (int i = 0; i < m_primitiveList.size(); i++){

        m_material.clear();
        m_material.cDiffuse.r = m_primitiveList[i].material.cDiffuse.r * m_global.kd;
        m_material.cDiffuse.g = m_primitiveList[i].material.cDiffuse.g * m_global.kd;
        m_material.cDiffuse.b = m_primitiveList[i].material.cDiffuse.b * m_global.kd;

        m_material.cAmbient.r = m_primitiveList[i].material.cAmbient.r * m_global.ka;
        m_material.cAmbient.g = m_primitiveList[i].material.cAmbient.g * m_global.ka;
        m_material.cAmbient.b = m_primitiveList[i].material.cAmbient.b * m_global.ka;
        m_phongShader->applyMaterial(m_material);

        m_phongShader->setUniform("m",m_primitiveList[i].transformation);

        if (m_shapes.find(m_primitiveList[i].type) != m_shapes.end())
            m_shapes.find(m_primitiveList[i].type)->second->draw();
    }



    m_material.clear();
    m_material.cAmbient.r = 1.f;
    m_material.cDiffuse.r = 1.f;
    m_material.cSpecular.r = m_material.cSpecular.g = m_material.cSpecular.b = 1;
    m_material.shininess = 64;
    m_phongShader->applyMaterial(m_material);
    m_phongShader->setUniform("m", glm::mat4(1.0f));
    m_cloth->draw();

}

void SceneviewScene::settingsChanged() {
    // TODO: [SCENEVIEW] Fill this in if applicable.



    m_shapes.clear();
    //adaptive level of detail
    int arbitaryParam = std::max(4,int(200 * 1.f/m_primitiveList.size()));

    for (int i = 0; i < m_primitiveList.size(); i++){

        if (m_shapes.find(m_primitiveList[i].type) == m_shapes.end()){

        switch (m_primitiveList[i].type) {
                case PrimitiveType::PRIMITIVE_CUBE:
                    m_shapes.insert({m_primitiveList[i].type,std::make_unique<Cube>(arbitaryParam)});
                    break;
                case PrimitiveType::PRIMITIVE_CYLINDER:
                    m_shapes.insert({m_primitiveList[i].type,std::make_unique<Cylinder>(arbitaryParam,arbitaryParam)});
                     break;
                default:
                    break;
            }
        }
    }
    m_cloth = std::make_unique<Cloth>(0);

}

void SceneviewScene::updateScene(){
    m_cloth->update();
}

