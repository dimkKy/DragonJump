#pragma once
#include "Drawable.h"

class Clickable : public virtual Drawable
{
public:
	[[nodiscard]] virtual bool OnMouseButtonDown(const Vector2D& clickPos) = 0;
	[[nodiscard]] virtual bool OnMouseButtonUp(const Vector2D& clickPos) = 0;
};

