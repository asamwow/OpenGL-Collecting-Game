#ifndef __GAMEOBJECT_H_
#define __GAMEOBJECT_H_

#include "Shape.h"
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

class GameObject {

public:
  GameObject();
  GameObject(std::shared_ptr<Shape> mesh, glm::vec3 position,
             glm::vec3 rotation, glm::vec3 scale, glm::vec3 velocity,
             glm::vec3 extent);
  std::shared_ptr<Shape> mesh;
  glm::vec3 position;
  glm::vec3 rotation;
  glm::vec3 scale;
  glm::vec3 velocity;
  glm::vec3 extent; // for bbox
};

#endif // __GAMEOBJECT_H_
