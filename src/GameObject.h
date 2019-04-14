#ifndef __GAMEOBJECT_H_
#define __GAMEOBJECT_H_

#include "Shape.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class GameObject {

public:
  /* GameObject(const GameObject& gameObject); */
  GameObject();
  GameObject(std::shared_ptr<Shape> mesh, glm::vec3 position,
             glm::vec3 rotation, glm::vec3 scale, glm::vec3 velocity);
  std::shared_ptr<Shape> mesh;
  glm::vec3 position;
  glm::vec3 rotation;
  glm::vec3 scale;
  glm::vec3 velocity;
};

#endif // __GAMEOBJECT_H_
