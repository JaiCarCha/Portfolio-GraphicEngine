#ifndef DRAWABLE_OBJECT_H
#define DRAWABLE_OBJECT_H

#include <vector>
#include "Shader.h"
#include "Transformation.h"

class DrawableObject
{
public:
	Transformation transformation;
	virtual void Draw(Shader* shader) = 0;
};

#endif DRAWABLE_OBJECT_H
