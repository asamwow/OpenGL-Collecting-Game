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

#include "Collectable.h"
#include "GLSL.h"
#include "GameObject.h"
#include "MatrixStack.h"
#include "Program.h"
#include "Shape.h"
#include "WindowManager.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader/tiny_obj_loader.h>

// value_ptr for glm
//#include <glm/gtc/matrix_transform.hpp>
//#include <glm/gtc/type_ptr.hpp>
//#include <gtc/matrix_transform.hpp>
//#include <gtc/type_ptr.hpp>

#ifdef __APPLE__
#include "MacIncludeGL.h"
#endif
#ifdef __linux__
#include "LinuxIncludeGL.h"
#endif

#include "Camera.h"

using namespace std;
using namespace glm;

#define PI 3.14f

// game settings
#define BOARD_SIZE 10
#define PLAYER_SPEED 2.0f
#define COLLECTABLE_SPAWN_DELAY 2.0f

class Application : public EventCallbacks
{

public:
  WindowManager *windowManager = nullptr;

  // Our shader program
  std::shared_ptr<Program> prog;

  // meshes
  shared_ptr<Shape> collectableMesh;
  shared_ptr<Shape> groundPlane;
  shared_ptr<Shape> playerMesh;

  // pointers to objects in the scene
  vector<GameObject *> renderObjects;

  GameObject player;
  Camera playerView;

  // for each board space, objects player collects
  Collectable collectables[BOARD_SIZE][BOARD_SIZE];
  int collectablesCount = 0;

  GameObject ground;

  int score = 0;
  float colorTime = 0;

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
                   int mods)
  {
    if (key == GLFW_KEY_W && action == GLFW_PRESS)
    {
      playerView.w = 1;
    }
    if (key == GLFW_KEY_W && action == GLFW_RELEASE)
    {
      playerView.w = 0;
    }
    if (key == GLFW_KEY_S && action == GLFW_PRESS)
    {
      playerView.s = 1;
    }
    if (key == GLFW_KEY_S && action == GLFW_RELEASE)
    {
      playerView.s = 0;
    }
    if (key == GLFW_KEY_A && action == GLFW_PRESS)
    {
      playerView.a = 1;
    }
    if (key == GLFW_KEY_A && action == GLFW_RELEASE)
    {
      playerView.a = 0;
    }
    if (key == GLFW_KEY_D && action == GLFW_PRESS)
    {
      playerView.d = 1;
    }
    if (key == GLFW_KEY_D && action == GLFW_RELEASE)
    {
      playerView.d = 0;
    }
    if (key == GLFW_KEY_R && action == GLFW_PRESS)
    {
      playerView.r = 1;
    }
    if (key == GLFW_KEY_R && action == GLFW_RELEASE)
    {
      playerView.r = 0;
    }
    if (key == GLFW_KEY_F && action == GLFW_PRESS)
    {
      playerView.f = 1;
    }
    if (key == GLFW_KEY_F && action == GLFW_RELEASE)
    {
      playerView.f = 0;
    }

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
      glfwSetWindowShouldClose(window, GL_TRUE);
    }
    if (key == GLFW_KEY_Z && action == GLFW_PRESS)
    {
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    if (key == GLFW_KEY_Z && action == GLFW_RELEASE)
    {
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    vec3 playerVelocity = vec3(0, 0, 0);
    if (key == GLFW_KEY_UP && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
      playerVelocity.z -= PLAYER_SPEED;
    }
    if (key == GLFW_KEY_DOWN &&
        (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
      playerVelocity.z += PLAYER_SPEED;
    }
    if (key == GLFW_KEY_RIGHT &&
        (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
      playerVelocity.x += PLAYER_SPEED;
    }
    if (key == GLFW_KEY_LEFT &&
        (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
      playerVelocity.x -= PLAYER_SPEED;
    }
    player.velocity = playerVelocity;
  }

  void
  mouseCallback(GLFWwindow *window, int button, int action, int mods)
  {
    double posX, posY;

    if (action == GLFW_PRESS)
    {
      glfwGetCursorPos(window, &posX, &posY);
      cout << "Pos X " << posX << " Pos Y " << posY << endl;
    }
  }

  void resizeCallback(GLFWwindow *window, int width, int height)
  {
    glViewport(0, 0, width, height);
  }

  void init(const std::string &resourceDirectory)
  {
    GLSL::checkVersion();

    // Set background color.
    glClearColor(.12f, .34f, .56f, 1.0f);
    // Enable z-buffer test.
    glEnable(GL_DEPTH_TEST);

    glfwSetInputMode(windowManager->getHandle(), 
                      GLFW_CURSOR,
                      GLFW_CURSOR_DISABLED);

    // Initialize the GLSL program.
    prog = make_shared<Program>();
    prog->setVerbose(true);
    prog->setShaderNames(resourceDirectory + "/simple_vert.glsl",
                         resourceDirectory + "/simple_frag.glsl");
    prog->init();
    prog->addUniform("P");
    prog->addUniform("V");
    prog->addUniform("M");
    prog->addUniform("collected");
    prog->addUniform("colorTime");
    prog->addAttribute("vertPos");
    prog->addAttribute("vertNor");
  }

  void initGeom(const std::string &resourceDirectory)
  {
    // load obj file for collectable
    vector<tinyobj::shape_t> TOshapes;
    vector<tinyobj::material_t> objMaterials;
    string errStr;
    // load in the mesh and make the shape(s)
    bool rc =
        tinyobj::LoadObj(TOshapes, objMaterials, errStr,
                         (resourceDirectory + "/bunny.obj").c_str());
    if (!rc)
    {
      cerr << errStr << endl;
    }
    else
    {
      collectableMesh = make_shared<Shape>();
      collectableMesh->createShape(TOshapes[0]);
      collectableMesh->measure();
      collectableMesh->init();
    }
    gMin.x = collectableMesh->min.x;
    gMin.y = collectableMesh->min.y;

    // create mesh for collectable
    // collectableMesh = make_shared<Shape>();
    // collectableMesh->createShape(0);
    // collectableMesh->measure();
    // collectableMesh->init();

    // ground mesh and gameobject
    groundPlane = make_shared<Shape>();
    groundPlane->createShape(1);
    groundPlane->measure();
    groundPlane->init();
    ground = GameObject(
        groundPlane, vec3(0, 0, -6), vec3(0, 0, 0),
        vec3(0.5f * BOARD_SIZE, 0.5f * BOARD_SIZE, 0.5f * BOARD_SIZE),
        vec3(0, 0, 0), vec3(0, 0, 0), 0);
    renderObjects.push_back(&ground);

    // player mesh and gameobject
    playerMesh = make_shared<Shape>();
    playerMesh->createShape(0);
    playerMesh->measure();
    playerMesh->init();
    player = GameObject(playerMesh, vec3(0, 20, -6), vec3(0, PI / 4, 0),
                        vec3(0.5f, 0.7f, 0.3f), vec3(0, 0, 0),
                        vec3(0.5f, 0.5f, 0.5f), 0);
    //renderObjects.push_back(&player);
  }

  void renderGameObject(shared_ptr<MatrixStack> Projection,
                        shared_ptr<MatrixStack> View,
                        shared_ptr<MatrixStack> Model, GameObject *renderObject,
                        float deltaTime)
  {
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
    glUniform1i(prog->getUniform("collected"), renderObject->collected);
    glUniform1f(prog->getUniform("colorTime"), cos(colorTime));
    renderObject->mesh->draw(prog);
    Model->popMatrix();
    Model->popMatrix();
  }

  void render(float deltaTime)
  {
    colorTime += 10* deltaTime;
    player.position = playerView.position;
    // Get current frame buffer size.
    int width, height;
    double mousex, mousey;
    glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
    glViewport(0, 0, width, height);

    glfwGetCursorPos(windowManager->getHandle(), &mousex, &mousey);

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
    Projection->perspective(0.45f, aspect, 0.01f, 100.0f);

    // View is identity - for now
    View->pushMatrix();
    //View->translate(vec3(0, -3, -16));
    //View->rotate(PI / 4, vec3(1, 0, 0));
    View->multMatrix(playerView.process(deltaTime, mousex, mousey, width, height));
    glfwSetCursorPos(windowManager->getHandle(), width / 4.0, height / 4.0);

    // Draw a stack of cubes with indiviudal transforms
    prog->bind();
    glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE,
                       value_ptr(Projection->topMatrix()));
    glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE,
                       value_ptr(View->topMatrix()));

    // render gameobjects in renderObjects
    for (unsigned int i = 0; i < renderObjects.size(); i++)
    {
      GameObject *renderObject = renderObjects[i];
      renderGameObject(Projection, View, Model, renderObject, deltaTime);
    }

    // render collectables seperately to avoid render calls on empty spaces
    for (int x = 0; x < BOARD_SIZE; x++)
    {
      for (int y = 0; y < BOARD_SIZE; y++)
      {
        Collectable *renderObject = &collectables[x][y];
        if (renderObject->moved == -1)
        {
          continue;
        }
        renderGameObject(Projection, View, Model, renderObject, deltaTime);
      }
    }

    // clean up
    prog->unbind();
    Projection->popMatrix();
    View->popMatrix();
  }

  void CreateCollectable()
  {
    if (collectablesCount >= 10)
    {
      return;
    }
    collectablesCount++;
    int x = rand() % BOARD_SIZE;
    int z = rand() % BOARD_SIZE;
    collectables[x][z] = Collectable(
        collectableMesh, vec3(0, 0, 0), vec3(0, PI / 8, 0),
        vec3(0.15f, 0.15f, 0.25f), vec3(0, 0, 0), vec3(0.5f, 0.5f, 0.5f),
        0);
    // renderObjects.push_back(&collectables[x][z]);
    PositionCollectable(&collectables[x][z], x, z);
  }

  void PositionCollectable(Collectable *collectable, int x, int z)
  {
    collectable->position =
        vec3(x - BOARD_SIZE / 2 + 0.5f, 0, z - 6 - BOARD_SIZE / 2 + 0.5f);
  }

  /// only used once, but made a function to break out of double loop
  /// returns false if no path available
  int MoveCollectableRandom(int x, int y)
  {
    // keep track of wich directions we have checked
    int dirsFree[] = {1, 1, 1, 1};
    const vec2 dirs[] = {vec2(-1, 0), vec2(0, 1), vec2(1, 0), vec2(0, -1)};
    if (x == 0)
    {
      dirsFree[0] = 0;
    }
    else if (x == BOARD_SIZE - 1)
    {
      dirsFree[2] = 0;
    }
    if (y == 0)
    {
      dirsFree[3] = 0;
    }
    else if (y == BOARD_SIZE - 1)
    {
      dirsFree[1] = 0;
    }

    // keep finding directions untill exhausted all options
    int randomDir = -1;
    int neighbor_x = -1;
    int neighbor_y = -1;
    while (dirsFree[0] != 0 || dirsFree[1] != 0 || dirsFree[2] != 0 ||
           dirsFree[3] != 0)
    {
      int randomCheck;
      do
      {
        randomCheck = rand() % 4;
      } while (dirsFree[randomCheck] == 0);
      neighbor_x = x + dirs[randomCheck].x;
      neighbor_y = y + dirs[randomCheck].y;
      if (collectables[neighbor_x][neighbor_y].moved != -1)
      {
        dirsFree[randomCheck] = 0;
      }
      else
      {
        randomDir = randomCheck;
        break;
      }
    }
    // no directions available
    if (randomDir == -1)
    {
      return 0;
    }

    // swap cell with adjacent
    Collectable temp = collectables[x][y];
    collectables[x][y] = collectables[neighbor_x][neighbor_y];
    collectables[neighbor_x][neighbor_y] = temp;
    collectables[neighbor_x][neighbor_y].velocity =
        vec3(dirs[randomDir].x, 0, dirs[randomDir].y) / COLLECTABLE_SPAWN_DELAY;
    collectables[neighbor_x][neighbor_y].rotation.y =
        -randomDir * PI / 2 + 9 * PI / 16;
    collectables[neighbor_x][neighbor_y].moved = 1;
    return 1;
  }

  /// this function handles the spawning and moving of collectables
  void CollectableStep()
  {
    CreateCollectable();

    // align all collectables
    for (int x = 0; x < BOARD_SIZE; x++)
    {
      for (int y = 0; y < BOARD_SIZE; y++)
      {
        if (collectables[x][y].moved != -1)
        {
          if (collectables[x][y].moved == 2)
          {
            collectables[x][y].velocity = vec3(0, 0, 0);
          }
          else
          {
            collectables[x][y].moved = 0;
            PositionCollectable(&collectables[x][y], x, y);
          }
        }
      }
    }

    // move collectables randomly
    for (int x = 0; x < BOARD_SIZE; x++)
    {
      for (int y = 0; y < BOARD_SIZE; y++)
      {
        if (collectables[x][y].moved == 0)
        {
          MoveCollectableRandom(x, y);
        }
      }
    }
  }

  /// for each collectable, check if its extent is within the player's extent
  void CheckCollisions()
  {
    //vec3 playerStart = player.position - player.extent;
    //vec3 playerEnd = player.position + player.extent;
    player.position = -playerView.position;
    vec3 playerStart = player.position - player.extent;
    vec3 playerEnd = player.position + player.extent;
    for (int x = 0; x < BOARD_SIZE; x++)
    {
      for (int y = 0; y < BOARD_SIZE; y++)
      {
        if (collectables[x][y].moved != -1)
        {
          vec3 colStart =
              collectables[x][y].position - collectables[x][y].extent;
          vec3 colEnd = collectables[x][y].position + collectables[x][y].extent;
          if (colStart.x > playerEnd.x)
          {
            continue;
          }
          if (colEnd.x < playerStart.x)
          {
            continue;
          }
          if (colStart.y > playerEnd.y)
          {
            continue;
          }
          if (colEnd.y < playerStart.y)
          {
            continue;
          }
          if (colStart.z > playerEnd.z)
          {
            continue;
          }
          if (colEnd.z < playerStart.z)
          {
            continue;
          }
          collectables[x][y].velocity = vec3(0, 0, 0);
          if (collectables[x][y].moved != 2)
          {
            score += 100;
            collectables[x][y].moved = 2;
            collectables[x][y].collected = 1;
            printf("Hit! Your score is %i\n", score);
          }
        }
      }
    }
  }
};

int main(int argc, char *argv[])
{
  std::string resourceDir = "../resources";

  if (argc >= 2)
  {
    resourceDir = argv[1];
  }

  Application *application = new Application();

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

  // setup timers
  auto lastTime = std::chrono::high_resolution_clock::now();
  float timeSinceLastCollectable = 0;

  // Loop until the user closes the window.
  while (!glfwWindowShouldClose(windowManager->getHandle()))
  {

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
    if (timeSinceLastCollectable > COLLECTABLE_SPAWN_DELAY)
    {
      timeSinceLastCollectable = 0.0f;
      application->CollectableStep();
    }
    application->CheckCollisions();

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
