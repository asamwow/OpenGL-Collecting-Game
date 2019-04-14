#ifndef __COLLECTABLE_H_
#define __COLLECTABLE_H_

#include "Shape.h"
#include "GameObject.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Collectable : public GameObject {

public:
  int moved;
  Collectable(std::shared_ptr<Shape> mesh, glm::vec3 position,
              glm::vec3 rotation, glm::vec3 scale, glm::vec3 velocity)
      : GameObject(mesh, position, rotation, scale, velocity) {
    moved = 0;
  }
  Collectable() { moved = -1; }
};

#endif // __GAMEOBJECT_H_
