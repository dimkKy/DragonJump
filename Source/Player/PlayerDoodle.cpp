// by Dmitry Kolontay

#include "PlayerDoodle.h"
#include "DragonJumpFramework.h"
#include "SteppableOn.h"
#include "Platform.h"
#include "Monster.h"
#include "SpriteLoader.h"
#include "Hole.h"
#include "Ability.h"
#include <cassert>

const float PlayerDoodle::jumpHeight = PlayerDoodle::CalculateJumpHeight();

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
	SetOffsets();
	sprites.at(4).offset *= 0.5f;
	sprites.at(5).offset *= 0.5f;
	collisionInfo.halfSize = sprites.at(5).offset;

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
	if (controllingHole && fallingAnimationTimeLeft <= 0.f) {
		return false;
	}
	position.y += cameraVerticalOffset;
	return DrawIfActive_Internal();
}


bool PlayerDoodle::Reactivate(const Vector2Df& positionX_velocityY)
{
	bIsActive = true;
	if (Collidable::IsActive()) {
		bIsAiming = false;
		position.x = positionX_velocityY.x;
		position.y = framework.GetSize().y + collisionInfo.halfSize.y;
		if (positionX_velocityY.y == 0.f)
			velocity.y = jumpingVelocity * 1.5f;
		else
			velocity.y = positionX_velocityY.y;
		velocity.x = 0.f;
		knockoutAnimationTimeLeft = -1.f;
		fallingAnimationTimeLeft = -1.f;
		lastFaceDirection = false;
		standingTimeLeft = -1.f;
		standingOn = nullptr;
		controllingHole = nullptr;
		activeForces.clear();
		activeAbilities.clear();
	}
	return bIsActive;
}

bool PlayerDoodle::DrawIfActive_Internal()
{
	if (framework.IsOutOfSideBorder(position,
		sprites.at(lastFaceDirection).sprite, true) > 0 && velocity.y >= -0.f) {
		//going to fail
		return false;
	}
	int spriteDrawn{ ProcessDraw() };
	if (int additionalDraw{ 
		framework.DoesSpriteTouchBorder(position,sprites.at(spriteDrawn).sprite) }) {
		float posX{ position.x };
		position.x -= framework.GetSize().x * additionalDraw;
		ProcessDraw();
		position.x = posX;
	}
	return true;
}

int PlayerDoodle::ProcessDraw()
{
	if (controllingHole) {
		float scale{ fallingAnimationTimeLeft / fallingAnimationDuration };
		int spriteToDraw{ 0 };
		if (controllingHole->GetPosition().x > position.x) {
			spriteToDraw = 1;
		}
		SpriteInfo& spriteInfo{ sprites.at(spriteToDraw) };
		Vector2D size{ framework.GetSpriteSize(spriteInfo.sprite) };
		setSpriteSize(spriteInfo.sprite.get(), static_cast<int>(size.x * scale + 0.5f), static_cast<int>(size.y * scale + 0.5f));
		spriteInfo.Draw(position + spriteInfo.offset * (1.f - scale));
		setSpriteSize(spriteInfo.sprite.get(), size.x, size.y);
		return spriteToDraw;
	}

	if (!bIsActive) {

		//knockout
	}
	//ugly
	int spriteToDraw{ static_cast<int>(lastFaceDirection) };
	if (bIsAiming) {
		spriteToDraw = 4 + static_cast<bool>(standingOn);
		//?
	}
	else {
		if (std::abs(velocity.x) > framework.inputImpulseAbs) {
			lastFaceDirection = (velocity.x > 0.f);
		}
		spriteToDraw = 2 * static_cast<bool>(standingOn) +
			lastFaceDirection;
	}
	sprites.at(spriteToDraw).Draw(position);
	if (bIsAiming) {
		//draw mouth
		//sprites.at(6).Draw({ position.x, position.y });
	}
	return spriteToDraw;
}

void PlayerDoodle::ReceiveTick(float deltaTime)
{
	position += (velocity * deltaTime);
	position.x = std::fmodf(position.x, 
		static_cast<float>(framework.GetSize().x));
	if (position.x < 0.f) {
		position.x += framework.GetSize().x;
	}
	//
	if (controllingHole) {
		fallingAnimationTimeLeft -= deltaTime;
		Vector2Df resultingForce{ controllingHole->GetPosition() - position};
		velocity = resultingForce / fallingAnimationTimeLeft;
		/*float distSquared{ resultingForce.LengthSquared() };
		if (distSquared < collisionSize.x * collisionSize.y * 0.11f) {
			//resultingForce.TurnThisToUnit();
			velocity = resultingForce * sqrtf(distSquared) / fallingAnimationTimeLeft;
			//position = controllingHole->GetPosition();
		}
		else {
			resultingForce.TurnThisToUnit();
			resultingForce *= std::abs(gravityForce * deltaTime * controllingHole->GetForceMultiplier() / distSquared);
			velocity += resultingForce;
			DampCurrentVelocity(deltaTime, minVelocityAndForce);
		}
		fallingAnimationTimeLeft -= deltaTime;*/
		return;
	}
	if (standingTimeLeft > -0.f) {
		standingTimeLeft -= deltaTime;
		if (standingTimeLeft > +0.f) {
			return;	
		}
		DJLog("stop standing");
		deltaTime = -standingTimeLeft;
		velocity.y = jumpingVelocity;
		AddImpulse({ 0.f, jumpImpulse * mass }, jumpImpulseDuration);
		standingTimeLeft = -1.f;
		//std::invoke(onJumpDelegate, *this);
		standingOn->OnJumpFrom(*this);
		if (Platform * platform{ dynamic_cast<Platform*>(standingOn) }) {
			framework.IncreaseJumpsCounter();
		}
		standingOn = nullptr;
	}

	for (auto it{ activeAbilities.begin() }; it != activeAbilities.end(); ) {
		if ((*it).second <= deltaTime) {
			Ability::OnAbilityTick(*this, (*it).first, (*it).second);
			activeAbilities.erase(it++);
		}
		else {
			Ability::OnAbilityTick(*this, (*it).first, deltaTime);
			(*it).second -= deltaTime;
			++it;
		}
	}
	//
	Vector2Df resultingForce{ 0.f, gravityForce * deltaTime * mass };	
	for (auto it{ activeForces.begin() }; it != activeForces.end(); ) {
		if ((*it).second <= deltaTime) {
			resultingForce += (*it).first * (*it).second;
			activeForces.erase(it++);
		}
		else {
			resultingForce += (*it).first * deltaTime;
			(*it).second -= deltaTime;
			++it;
		}
	}
	velocity += resultingForce / mass;
	DampCurrentVelocity(deltaTime, resultingForce.LengthSquared());
	return;
}

CollisionChannel PlayerDoodle::GetCollisionChannel() const
{
	return CollisionChannel::CC_Player;
}

bool PlayerDoodle::GetCollisionResponse(CollisionChannel channel) const
{
	return channel == CollisionChannel::CC_Monster || 
		(channel == CollisionChannel::CC_Platform && !IsStanding());
}

void PlayerDoodle::ReceiveCollision(CollidableBase& other)
{
	if (Ability* ability{ dynamic_cast<Ability*>(&other) }) {
		if (auto result{ activeAbilities.find(ability->GetAbilityType()) };
			result != activeAbilities.end()) {
			if (result->second < 0.f) {
				result->second = ability->GetAbilityDuration(result->first);	
			}
			else {
				result->second += ability->GetAbilityDuration(result->first) * 0.5f;
			}
		}
		else {
			activeAbilities.emplace(ability->GetAbilityType(), ability->GetAbilityDuration());
		}
		ability->Deactivate();
		return;
	}
	//can be improved
	if (!controllingHole && activeAbilities.size() < 1) {
		if (Hole* hole{ dynamic_cast<Hole*>(&other) }) {
			controllingHole = hole;
			fallingAnimationTimeLeft = fallingAnimationDuration;
			velocity *= 0.5f;
			StartDying();
			return;
		}
	}
	if (!standingOn) {
		if (SteppableOnBase* steppable{ dynamic_cast<SteppableOnBase*>(&other) }) {
			if (steppable->CanBeSteppedOn() && velocity.y >= 0 &&
				position.y + collisionInfo.halfSize.y <= other.GetPosition().y) {
				DJLog("start standing");
				standingTimeLeft = standingTime;
				for (auto it{ activeForces.begin() }; it != activeForces.end(); ++it) {
					(*it).first.y = 0.f;
					velocity = 0.f;
				}
				position.y = steppable->GetStandingPointY(position.x) - sprites.at(2).offset.y;
				standingOn = steppable;
			}
			return;
		}
	}
	if (activeAbilities.size() < 1) {
		if (MonsterBase* mosnter{ dynamic_cast<MonsterBase*>(&other) }) {
			//knockout
			velocity.y = jumpingVelocity * -0.75f;
			activeForces.clear();
			knockoutAnimationTimeLeft = knockoutAnimationDuration;
			StartDying();
		}
	}
}

void PlayerDoodle::AddImpulse(const Vector2Df& force, float duration)
{ 
	if (bIsActive && standingTimeLeft <= -0.f) {
		if (duration > 0.f)
			activeForces.push_back({ force, duration });
		else
			velocity += force;
	}
}


void PlayerDoodle::DampCurrentVelocity(float deltaTime, float forceLengthSquared)
{
	float lengthSquared{ velocity.LengthSquared() };
	if (lengthSquared < minVelocityAndForce && forceLengthSquared < minVelocityAndForce) {
		velocity = 0.f;
	}
	else {
		if (lengthSquared >= maxVelocityLengthSquared) {
			DJLog("max velocity reached");
			velocity *= std::sqrtf(maxVelocityLengthSquared / lengthSquared);
		}
		activeForces.push_back({ dampingMultiplier * velocity * lengthSquared, deltaTime });
	}
}

void PlayerDoodle::StartDying()
{
	//reset all
	bIsActive = false;
	standingOn = nullptr;
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
	if (!bIsAiming)
		return false;
	bIsAiming = false;
	if ((Vector2Df{ outTargetPosition } - GetMouthGlobalPos()).
		LengthSquared() < collisionInfo.halfSize.LengthSquared()){
		return false;
	}
#ifdef ORIGINAL_SHOOTING
	if (outTargetPosition.y >= position.y) {
		//if target below
		outTargetPosition.y = rand() % static_cast<int>(position.y * 0.75f + 0.5f);
	}
#endif
	return true;
}

//bug?: should be above
constexpr float PlayerDoodle::DampInstSpeed(float speed, float dampingMp)
{
	return speed * dampingMp * speed * speed / mass;
}

constexpr float PlayerDoodle::CalculateJumpHeight()
{
	constexpr float dampedVelocity{ jumpingVelocity + 
		DampInstSpeed(jumpingVelocity, dampingMultiplier.y) };
	constexpr float distanceFirst{ 
		(dampedVelocity + (jumpImpulse + gravityForce) * jumpImpulseDuration * 0.5f) 
		* jumpImpulseDuration };

	constexpr float speedAfterJumpImpulse{ jumpingVelocity + 
		(jumpImpulse + gravityForce) * jumpImpulseDuration };
	constexpr float dampedSecondVelocity{ speedAfterJumpImpulse + 
		DampInstSpeed(speedAfterJumpImpulse, dampingMultiplier.y) };

	if (speedAfterJumpImpulse < 0.f) {
		constexpr float distanceSecond{ 
			dampedSecondVelocity * dampedSecondVelocity / (2.f * gravityForce) };
		return -1.f * (distanceFirst - distanceSecond);
	}
	else {
		throw std::logic_error("speedAfterJumpImpulse must be negative");
	}
}
