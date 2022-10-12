#ifndef TRANSFORMATION_H
#define TRANSFORMATION_H

#include "glm/glm.hpp"

struct Transformation
{
    glm::vec3 translation = glm::vec3(0.f);
    glm::vec3 rotation = glm::vec3(0.f);
    glm::vec3 scale = glm::vec3(1.f);
};

#endif TRANSFORMATION_H

