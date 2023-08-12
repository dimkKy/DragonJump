// by Dmitry Kolontay

#include "IntDisplayer.h"
#include "SpriteLoader.h"
#include <string>
#include <assert.h>

IntDisplayer::IntDisplayer(DragonJumpFramework& _framework, const Vector2Df& position) : 
	Drawable(_framework, position), valueGetter{nullptr},
	digits { framework.GetNumberedSprites(SpritePaths::digitNums) }
{
	if (digits.size() != 10 || position.x <= 0.f || position.y >= framework.GetSize().y) {
		bIsActive = false;
	}
	assert(bIsActive && "Drawable init unsuccessfull");
}

bool IntDisplayer::DrawIfActive_Internal ()
{
	if (!valueGetter)
		return false;
	int valueToDisplay{ std::invoke(valueGetter)};
	float offset{ 0.f };
	while (valueToDisplay != 0) {
		int lastDigit{ valueToDisplay % 10 };
		valueToDisplay /= 10;
		digits.at(lastDigit).Draw(position.x - offset, position.y + digits.at(lastDigit).offset.y);
		offset += digits.at(lastDigit).offset.x;
	}
	return true;
}


bool IntDisplayer::Reactivate(const Vector2Df& pos) &
{
	position = pos;
	bIsActive = digits.size() == 10 && position.x <= 0.f && position.y >= framework.GetSize().y;
	return bIsActive;
}

float IntDisplayer::GetWidgetHeight() const
{
	float height{ 0.f };
	for (auto& digit : digits) {
		if (digit.offset.x > height) {
			height = digit.offset.x;
		}
	}
	return height * 1.1f;
}
