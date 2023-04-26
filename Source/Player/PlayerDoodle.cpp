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
	sprites{ framework.GetNumberedSprites(SpritePaths::doodle) },
	knockoutSprites{ framework.GetNumberedSprites(SpritePaths::knockoutStars) },
	jetSprites{ framework.GetNumberedSprites(SpritePaths::jetpack) }
{
	if (sprites.size() < 8 || knockoutSprites.size() < 1 || 
		jetSprites.size() < 2 || jetSprites.size() % 2 == 1) {
		bIsActive = false;
		sprites.clear();
		knockoutSprites.clear();
		jetSprites.clear();
		DJLog("player doodle: sprites loading failed");
		return;
	}
	SetOffsets();
	sprites.at(4).offset *= 0.5f;
	sprites.at(5).offset *= 0.5f;

	collisionInfo.halfSize = sprites.at(5).offset;

	sprites.at(6).offset.x *= 0.5f;
	sprites.at(6).offset.y *= 1.65f;
	sprites.at(7).offset.x *= 0.5f;
	sprites.at(7).offset.y *= 0.45f;

	for (auto& sprite : knockoutSprites) {
		sprite.offset.x *= 0.5f;
	}
	for (auto i{ 0 }; i < jetSprites.size(); ++i) {
		if (i % 2) {
			jetSprites.at(i).offset.x *= 1.2f;
		}
		else {
			jetSprites.at(i).offset.x *= -0.2f;
		}
		jetSprites.at(i).offset.y *= 0.3f;
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
		knockoutAnimTime = -1.f;
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
	//
	if (!bIsActive) {
		knockoutAnimTime = std::fmodf(knockoutAnimTime,
			knockoutAnimDuration);
		knockoutSprites.at(static_cast<int>(
			knockoutAnimTime * knockoutSprites.size() / knockoutAnimDuration)).Draw(position);
		return spriteToDraw;
	}

	if (bIsAiming) {
		if (framework.GetMousePosition().y <= position.y) {
			sprites.at(6).Draw(position);
		}
		else {
			sprites.at(7).Draw(position);
		}
	}
	if (auto r{ activeAbilities.find(AbilityType::AT_Jet) }; r != activeAbilities.end()) {
		if (r->second > 0.f) {
			float animTime{ std::fmodf(r->second, Ability::jetAnimDuration) };
			int frames{ static_cast<int>(jetSprites.size()) / 2 };
			int jetSpriteToDraw{ 2 * static_cast<int>(animTime * frames / Ability::jetAnimDuration)
			+ (spriteToDraw % 2) };
			jetSprites.at(jetSpriteToDraw).Draw(position);
		}
		else {

		}
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
	//
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
	//
	if (knockoutAnimTime >= 0.f) {
		knockoutAnimTime += deltaTime;
		return;
	}
	//
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
		}
	}
	if (activeAbilities.size() < 1) {
		if (MonsterBase* mosnter{ dynamic_cast<MonsterBase*>(&other) }) {
			//knockout
			velocity.y = jumpingVelocity * -0.5f;
			activeForces.clear();
			knockoutAnimTime = 0.f;
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
