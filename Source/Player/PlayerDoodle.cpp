#include "PlayerDoodle.h"
#include "DragonJumpFramework.h"
#include "SteppableOn.h"
#include "Platform.h"
#include "Monster.h"
#include "SpriteLoader.h"
#include "Hole.h"
#include "Ability.h"
#include <cassert>

PlayerDoodle::PlayerDoodle(DragonJumpFramework& _framework, const Vector2Df& pos) :
	Drawable(_framework, pos), Tickable(0.f, jumpingVelocity * 2.f), 
	sprites{ framework.GetNumberedSprites(SpritePaths::doodle) }
{
	if (sprites.size() < 6) {
		bIsActive = false;
		sprites.clear();
		DJLog("player doodle: sprites loading failed");
		return;
	}
	/*for (int i{ 0 }; i < 5; ++i) {
		sprites.at(i).offset.x *= (i % 2)? 0.66f : 0.33f;
		sprites.at(i).offset.y *= 0.5f;
	}*/
	SetOffsets();
	sprites.at(4).offset *= 0.5f;

	sprites.at(5).offset.x *= 0.55f;
	sprites.at(5).offset.y *= 0.5f;
	collisionInfo.size = sprites.at(5).offset;

	if (sprites.size() > 6) {
		sprites.at(6).offset.x *= 0.5f;
		sprites.at(6).offset.y *= 1.65f;
	}
	if (sprites.size() > 7) {
		sprites.at(7).offset.x *= 0.5f;
		sprites.at(7).offset.y *= -1.65f;
	}
}
bool PlayerDoodle::DrawIfActive(float cameraVerticalOffset)
{
	position.y += cameraVerticalOffset;
	return DrawIfActive_Internal();
}

bool PlayerDoodle::Reactivate(const Vector2Df& positionX_velocityY)
{
	bIsActive = true;
	if (Collidable::IsActive()) {
		bIsAiming = false;
		position.x = positionX_velocityY.x;
		position.y = framework.GetSize().y + collisionInfo.size.y;
		if (positionX_velocityY.y == 0.f)
			currentVelocity.y = jumpingVelocity * 1.5f;
		else
			currentVelocity.y = positionX_velocityY.y;
		currentVelocity.x = 0.f;
		knockoutAnimationTimeLeft = -1.f;
		standingTimeLeft = -1.f;
		onJumpDelegate = nullptr;
		controllingHole = nullptr;
		additionalForces.clear();
	}
	return bIsActive;
}

bool PlayerDoodle::DrawIfActive_Internal()
{
	if (framework.IsOutOfSideBorder(position, framework.GetSpriteSize(
		sprites.at(mainSpriteToDraw).sprite)) > 0 && currentVelocity.y >= -0.f) {
		//going to fail
		return false;
	}
	//calculate spries

	ProcessDraw();
	if (int additionalDraw{ framework.DoesSpriteTouchBorder(position,
		framework.GetSpriteSize(sprites.at(mainSpriteToDraw).sprite))}) {
		float posX{ position.x };
		position.x -= framework.GetSize().x * additionalDraw;
		ProcessDraw();
		position.x = posX;
	}
	return true;
}

void PlayerDoodle::ProcessDraw()
{
	if (controllingHole) {
		float scale{ fallingAnimationTimeLeft / fallingAnimationDuration };
		Vector2D size;
		Sprite* sprite{ sprites.at(mainSpriteToDraw).sprite.get() };
		getSpriteSize(sprite, size.x, size.y);
		setSpriteSize(sprite, static_cast<int>(size.x * scale + 0.5f), static_cast<int>(size.y * scale + 0.5f));
		sprites.at(mainSpriteToDraw).Draw(position + sprites.at(mainSpriteToDraw).offset * (1.f - scale));
		setSpriteSize(sprite, size.x, size.y);
		return;
	}
	sprites.at(mainSpriteToDraw).Draw(position);
	if (mainSpriteToDraw > 3 /*&& sprites.size() > 6*/) {
		//draw mouth
		//sprites.at(6).Draw({ position.x, position.y });
	}
}

void PlayerDoodle::ReceiveTick(float deltaTime)
{
	position += (currentVelocity * deltaTime);
	position.x = std::abs(std::fmodf(position.x, 
		static_cast<float>(framework.GetSize().x)));
	//
	if (controllingHole) {
		fallingAnimationTimeLeft -= deltaTime;
		Vector2Df resultingForce{ controllingHole->GetPosition() - position};
		currentVelocity = resultingForce / fallingAnimationTimeLeft;
		/*float distSquared{ resultingForce.LengthSquared() };
		if (distSquared < collisionSize.x * collisionSize.y * 0.11f) {
			//resultingForce.TurnThisToUnit();
			currentVelocity = resultingForce * sqrtf(distSquared) / fallingAnimationTimeLeft;
			//position = controllingHole->GetPosition();
		}
		else {
			resultingForce.TurnThisToUnit();
			resultingForce *= std::abs(gravityForce * deltaTime * controllingHole->GetForceMultiplier() / distSquared);
			currentVelocity += resultingForce;
			DampCurrentVelocity(deltaTime, minVelocityAndForce);
		}
		fallingAnimationTimeLeft -= deltaTime;*/
		return;
	}
	if (standingTimeLeft > -0.f) {
		standingTimeLeft -= deltaTime;
		if (standingTimeLeft <= +0.f) {
			deltaTime = -standingTimeLeft;
			StopStanding();
		}
		else
			return;
	}
	if (abilityDurationLeft >= 0.f)
		abilityDurationLeft -= deltaTime;
	//
	Vector2Df resultingForce{ 0.f, gravityForce * deltaTime * mass };	
	for (auto iterator{ additionalForces.begin() }; iterator != additionalForces.end(); ) {
		if ((*iterator).second <= deltaTime) {
			resultingForce += (*iterator).first * (*iterator).second;
			additionalForces.erase(iterator++);
		}
		else {
			resultingForce += (*iterator).first * deltaTime;
			(*iterator).second -= deltaTime;
			++iterator;
		}
	}
	currentVelocity += resultingForce / mass;
	DampCurrentVelocity(deltaTime, resultingForce.LengthSquared());
	return;
}

CollisionChannel PlayerDoodle::GetCollisionChannel() const
{
	return CollisionChannel::CC_Player;
}

bool PlayerDoodle::GetCollisionResponse(CollisionChannel channel) const
{
	return channel == CollisionChannel::CC_Monster || (channel == CollisionChannel::CC_Platform && !IsStanding());
}

void PlayerDoodle::ReceiveCollision(CollidableBase& other)
{
	if (Ability* ability{ dynamic_cast<Ability*>(&other) }) {
		/*if (abilityDurationLeft > 0.f) {
			//TODO
			if (ability->GetAbilityType() == currentActiveAbility) {
				abilityDurationLeft += ability->GetAbilityDuration() * 0.5f;
				ability->Deactivate();
			}
		}
		else {
			abilityDurationLeft = ability->GetAbilityDuration();
			ability->Deactivate();
		}*/
		return;
	}
	if (!controllingHole) {
		if (Hole* hole{ dynamic_cast<Hole*>(&other) }) {
			controllingHole = hole;
			fallingAnimationTimeLeft = fallingAnimationDuration;
			currentVelocity *= 0.5f;
			StartDying();
			return;
		}
	}
	if (!onJumpDelegate) {
		if (SteppableOnBase* steppable{ dynamic_cast<SteppableOnBase*>(&other) }) {
			if (steppable->CanBeSteppedOn() && currentVelocity.y >= 0 &&
				position.y + collisionInfo.size.y * 0.5f <= other.GetPosition().y &&
				steppable->GetStandingPointY(position.x, position.y)) {
				DJLog("start standing");
				standingTimeLeft = standingTime;
				for (auto it{ additionalForces.begin() }; it != additionalForces.end(); ++it) {
					(*it).first.y = 0.f;
					currentVelocity = 0.f;
				}
				return;
			}
		}
		return;
	}
	/*if (abilityDurationLeft <= 0.f) {
		if (Monster<SShape>*steppable{ dynamic_cast<Monster<SShape>*>(&other) }) {

		}
	}
		Monster<SShape>*steppable{ dynamic_cast<Monster<SShape>*>(&other) }) {
		//knockout
		currentVelocity.y = jumpingVelocity * -0.75f;
		additionalForces.clear();
		knockoutAnimationTimeLeft = knockoutAnimationDuration;
		StartDying();
	}*/
}

void PlayerDoodle::AddImpulse(const Vector2Df& force, float duration)
{ 
	if (bIsActive && standingTimeLeft <= -0.f) {
		if (duration > 0.f)
			additionalForces.push_back({ force, duration });
		else
			currentVelocity += force;
	}
}


void PlayerDoodle::DampCurrentVelocity(float deltaTime, float forceLengthSquared)
{
	float lengthSquared{ currentVelocity.LengthSquared() };
	if (lengthSquared < minVelocityAndForce && forceLengthSquared < minVelocityAndForce) {
		currentVelocity = 0.f;
	}
	else {
		if (lengthSquared >= maxVelocityLengthSquared) {
			DJLog("max velocity reached");
			currentVelocity *= std::sqrtf(maxVelocityLengthSquared / lengthSquared);
		}
		additionalForces.push_back({ dampingMultiplier * currentVelocity * lengthSquared, deltaTime });
	}
}

void PlayerDoodle::StopStanding()
{
	DJLog("stop standing");
	currentVelocity.y = jumpingVelocity;
	AddImpulse({ 0.f, jumpImpulseMultiplier * mass }, 0.5f);
	standingTimeLeft = -1.f;
	std::invoke(onJumpDelegate, *this);
	onJumpDelegate = nullptr;
}

void PlayerDoodle::StartDying()
{
	//reset all
	bIsActive = false;
	onJumpDelegate = nullptr;
	standingTimeLeft = -1.f;
	bIsAiming = false;
}

bool PlayerDoodle::StartShooting()
{
	bIsAiming = true;
	return true;
}

bool PlayerDoodle::StopShooting(Vector2D& outTargetPosition)
{
	if (!bIsAiming || (Vector2Df{ outTargetPosition } - GetMouthGlobalPos()).
		LengthSquared() < collisionInfo.size.LengthSquared())
		return false;
	if (outTargetPosition.y >= position.y) {
		//if target below
		outTargetPosition.y = rand() % static_cast<int>(position.y * 0.75f + 0.5f);
	}
	bIsAiming = false;
	return true;
}


float PlayerDoodle::GetCalculateJumpDistance() const
{
	return std::abs((jumpingVelocity * jumpingVelocity - jumpImpulseMultiplier) / gravityForce * 0.75f) - 5.f;
}

