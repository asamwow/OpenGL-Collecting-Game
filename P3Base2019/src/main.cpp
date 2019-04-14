/*
 * Program 3 base code - includes modifications to shape and initGeom in
 * preparation to load multi shape objects CPE 471 Cal Poly Z. Wood + S. Sueda +
 * I. Dunn
 */

#include <chrono>
#include <glad/glad.h>
#include <iostream>
#include <stdlib.h>
#include <time.h>

#include "GLSL.h"
#include "GameObject.h"
#include "MatrixStack.h"
#include "Program.h"
#include "Shape.h"
#include "WindowManager.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader/tiny_obj_loader.h>

// value_ptr for glm
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;
using namespace glm;

#define PI 3.14f

// game settings
#define BOARD_SIZE 10
#define PLAYER_SPEED 2.0f

class Application : public EventCallbacks {

public:
  WindowManager *windowManager = nullptr;

  // Our shader program
  std::shared_ptr<Program> prog;

  // meshes
  shared_ptr<Shape> collectableMesh;
  shared_ptr<Shape> groundPlane;
  shared_ptr<Shape> playerMesh;

  // pointers to objects in the scene
  vector<GameObject*> renderObjects;

  // the object that the player controls
  GameObject player;

  // for each board space, objects player collects
  GameObject collectables[BOARD_SIZE][BOARD_SIZE];
  int collectablesCount = 0;

  GameObject ground;

  // Contains vertex information for OpenGL
  GLuint VertexArrayID;

  // Data necessary to give our triangle to OpenGL
  GLuint VertexBufferID;

  // Used to find delta time
  time_t currentTime;

  // example data that might be useful when trying to compute bounds on
  // multi-shape
  vec3 gMin;

  void keyCallback(GLFWwindow *window, int key, int scancode, int action,
                   int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
      glfwSetWindowShouldClose(window, GL_TRUE);
    }
    if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    if (key == GLFW_KEY_Z && action == GLFW_RELEASE) {
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    // rotate for debuging
    if (key == GLFW_KEY_R && action == GLFW_PRESS) {
      player.rotation.y += PI / 16;
    }
    vec3 playerVelocity = vec3(0, 0, 0);
    if (key == GLFW_KEY_UP && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
      playerVelocity.z -= PLAYER_SPEED;
    }
    if (key == GLFW_KEY_DOWN &&
        (action == GLFW_PRESS || action == GLFW_REPEAT)) {
      playerVelocity.z += PLAYER_SPEED;
    }
    if (key == GLFW_KEY_RIGHT &&
        (action == GLFW_PRESS || action == GLFW_REPEAT)) {
      playerVelocity.x += PLAYER_SPEED;
    }
    if (key == GLFW_KEY_LEFT &&
        (action == GLFW_PRESS || action == GLFW_REPEAT)) {
      playerVelocity.x -= PLAYER_SPEED;
    }
    player.velocity = playerVelocity;
  }

  void mouseCallback(GLFWwindow *window, int button, int action, int mods) {
    double posX, posY;

    if (action == GLFW_PRESS) {
      glfwGetCursorPos(window, &posX, &posY);
      cout << "Pos X " << posX << " Pos Y " << posY << endl;
    }
  }

  void resizeCallback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
  }

  void init(const std::string &resourceDirectory) {
    GLSL::checkVersion();

    // Set background color.
    glClearColor(.12f, .34f, .56f, 1.0f);
    // Enable z-buffer test.
    glEnable(GL_DEPTH_TEST);

    // Initialize the GLSL program.
    prog = make_shared<Program>();
    prog->setVerbose(true);
    prog->setShaderNames(resourceDirectory + "/simple_vert.glsl",
                         resourceDirectory + "/simple_frag.glsl");
    prog->init();
    prog->addUniform("P");
    prog->addUniform("V");
    prog->addUniform("M");
    prog->addAttribute("vertPos");
    prog->addAttribute("vertNor");
  }

  void initGeom(const std::string &resourceDirectory) {
    // EXAMPLE new set up to read one shape from one obj file - convert to read
    // several
    // Initialize mesh
    // Load geometry
    // Some obj files contain material information.We'll ignore them for this
    // assignment.
    // vector<tinyobj::shape_t> TOshapes;
    // vector<tinyobj::material_t> objMaterials;
    // string errStr;
    // // load in the mesh and make the shape(s)
    // bool rc =
    //     tinyobj::LoadObj(TOshapes, objMaterials, errStr,
    //                      (resourceDirectory + "/SmoothSphere.obj").c_str());
    // if (!rc) {
    //   cerr << errStr << endl;
    // } else {
    //   collectable = make_shared<Shape>();
    //   collectable->createShape(TOshapes[0]);
    //   collectable->measure();
    //   collectable->init();
    // }
    // gMin.x = collectable->min.x;
    // gMin.y = collectable->min.y;

    // ground mesh and gameobject
    groundPlane = make_shared<Shape>();
    groundPlane->createShape(1);
    groundPlane->measure();
    groundPlane->init();
    ground = GameObject(
        groundPlane, vec3(0, 0, -6), vec3(0, 0, 0),
        vec3(0.5f * BOARD_SIZE, 0.5f * BOARD_SIZE, 0.5f * BOARD_SIZE),
        vec3(0, 0, 0));
    renderObjects.push_back(&ground);

    // player mesh and gameobject
    playerMesh = make_shared<Shape>();
    playerMesh->createShape(0);
    playerMesh->measure();
    playerMesh->init();
    player = GameObject(playerMesh, vec3(0, 0, -6),
                                       vec3(0, PI / 4, 0),
                                       vec3(0.5f, 0.7f, 0.3f), vec3(0, 0, 0));
    renderObjects.push_back(&player);

    // collectable mesh only
    collectableMesh = make_shared<Shape>();
    collectableMesh->createShape(0);
    collectableMesh->measure();
    collectableMesh->init();
  }

  void render(float deltaTime) {
    // Get current frame buffer size.
    int width, height;
    glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
    glViewport(0, 0, width, height);

    // Clear framebuffer.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Use the matrix stack for Lab 6
    float aspect = width / (float)height;

    // Create the matrix stacks - please leave these alone for now
    auto Projection = make_shared<MatrixStack>();
    auto View = make_shared<MatrixStack>();
    auto Model = make_shared<MatrixStack>();

    // Apply perspective projection.
    Projection->pushMatrix();
    // Projection->perspective(45.0f, aspect, 0.01f, 100.0f);
    Projection->perspective(0.45f, aspect, 0.01f, 100.0f);

    // View is identity - for now
    View->pushMatrix();
    View->translate(vec3(0, -3, -16));
    View->rotate(PI / 4, vec3(1, 0, 0));

    // Draw a stack of cubes with indiviudal transforms
    prog->bind();
    glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE,
                       value_ptr(Projection->topMatrix()));
    glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE,
                       value_ptr(View->topMatrix()));

    for (unsigned int i = 0; i < renderObjects.size(); i++) {
      GameObject *renderObject = renderObjects[i];
      Model->pushMatrix();
      Model->loadIdentity();
      renderObject->position =
          renderObject->position + (renderObject->velocity * deltaTime);
      Model->translate(renderObject->position);
      Model->rotate(renderObject->rotation.x, vec3(1, 0, 0));
      Model->rotate(renderObject->rotation.z, vec3(0, 0, 1));
      Model->rotate(renderObject->rotation.y, vec3(0, 1, 0));
      Model->pushMatrix();
      Model->scale(renderObject->scale);
      glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE,
                         value_ptr(Model->topMatrix()));
      renderObject->mesh->draw(prog);
      Model->popMatrix();
      Model->popMatrix();
    }

    // // draw mesh
    // Model->pushMatrix();
    // Model->loadIdentity();
    // //"global" translate
    // Model->translate(vec3(0, 0, -6));
    // cube_rot_y += PI/4*deltaTime;
    // Model->rotate(cube_rot_y, vec3(0, 1, 0));
    // Model->rotate(PI/8, vec3(1, 0, 0));
    // Model->pushMatrix();
    // Model->scale(vec3(0.5, 0.5, 0.5));
    // glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE,
    // value_ptr(Model->topMatrix())); collectable->draw(prog);
    // Model->popMatrix();
    // Model->popMatrix();

    prog->unbind();

    // Pop matrix stacks.
    Projection->popMatrix();
    View->popMatrix();
  }

  void CreateCollectable() {
    if (collectablesCount >= BOARD_SIZE * BOARD_SIZE) {
      return;
    }
    int x = rand() % BOARD_SIZE;
    int z = rand() % BOARD_SIZE;
    collectables[x][z] = GameObject(
        collectableMesh, vec3(0, 0, 0),
        vec3(0, -PI / 8, 0), vec3(0.25f, 0.25f, 0.25f), vec3(0, 0, 0));
    renderObjects.push_back(&collectables[x][z]);
    PositionCollectable(&collectables[x][z], x, z);
  }

  void PositionCollectable(GameObject* collectable, int x, int z) {
    collectable->position = vec3(x - BOARD_SIZE/2, 0, z - 6 - BOARD_SIZE/2 );
  }
};

int main(int argc, char *argv[]) {
  // Where the resources are loaded from
  std::string resourceDir = "../resources";

  if (argc >= 2) {
    resourceDir = argv[1];
  }

  Application *application = new Application();

  // Your main will always include a similar set up to establish your window
  // and GL context, etc.

  WindowManager *windowManager = new WindowManager();
  windowManager->init(640, 480);
  windowManager->setEventCallbacks(application);
  application->windowManager = windowManager;

  // This is the code that will likely change program to program as you
  // may need to initialize or set up different data and state

  application->init(resourceDir);
  application->initGeom(resourceDir);

  // set random seed
  srand(time(NULL));

  application->CreateCollectable();

  // set current time initially
  auto lastTime = std::chrono::high_resolution_clock::now();

  float timeSinceLastCollectable = 0;

  // Loop until the user closes the window.
  while (!glfwWindowShouldClose(windowManager->getHandle())) {

    // save current time for next frame
    auto nextLastTime = std::chrono::high_resolution_clock::now();

    // get time since last frame
    float deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(
                          std::chrono::high_resolution_clock::now() - lastTime)
                          .count();
    // convert to seconds
    deltaTime = deltaTime * 0.000001f;

    // handle collectables
    timeSinceLastCollectable += deltaTime;
    if (timeSinceLastCollectable > 1.0f) {
      timeSinceLastCollectable = 0.0f;
      application->CreateCollectable();
    }

    // Render scene.
    application->render(deltaTime);

    // Swap front and back buffers.
    glfwSwapBuffers(windowManager->getHandle());
    // Poll for and process events.
    glfwPollEvents();

    // set time for next frame
    lastTime = nextLastTime;
  }

  // Quit program.
  windowManager->shutdown();
  return 0;
}
