// by Dmitry Kolontay

#include "IntDisplayer.h"
#include "SpriteLoader.h"
#include <string>
#include <assert.h>

IntDisplayer::IntDisplayer(DragonJumpFramework& _framework, const Vector2Df& position) : 
	Drawable(_framework, position), digits{framework.GetNumberedSprites(SpritePaths::digitNums)}
{
	if (digits.size() != 10 || position.x <= 0.f || position.y >= framework.GetSize().y) {
		bIsActive = false;
	}
	assert(bIsActive && "Drawable init unsuccessfull");
}

bool IntDisplayer::DrawIfActive_Internal ()
{
	int score{ static_cast<int>(framework.GetScore() +0.5f) };
	float offset{ 0.f };
	while (score != 0) {
		int lastDigit{ score % 10 };
		score /= 10;
		digits.at(lastDigit).Draw(position.x - offset, position.y);
		offset += digits.at(lastDigit).offset.x;
	}
	return true;
}


bool IntDisplayer::Reactivate(const Vector2Df& pos)
{
	position = pos;
	bIsActive = digits.size() == 10 && position.x <= 0.f && position.y >= framework.GetSize().y;
	return bIsActive;
}
