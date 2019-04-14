#include "GameObject.h"

GameObject::GameObject() {}

GameObject::GameObject(std::shared_ptr<Shape> mesh, glm::vec3 position,
                       glm::vec3 rotation, glm::vec3 scale,
                       glm::vec3 velocity) {
  this->mesh = mesh;
  this->position = position;
  this->rotation = rotation;
  this->scale = scale;
  this->velocity = velocity;
}

// GameObject::GameObject(const GameObject &gameObject)
//     : GameObject(gameObject.mesh, gameObject.position, gameObject.rotation,
//                  GameObject.scale, gameObject.velocity) {}
