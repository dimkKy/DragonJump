#pragma once
#include "Drawable.h"
#include "Tickable.h"
#include "Collidable.h"
#include <memory>
#include <list>
#include <functional>

class SteppableOnBase;

class Hole;
enum class AbilityType;

class PlayerDoodle : public Tickable, public Collidable<RectangleShape>
{
public:
	PlayerDoodle() = delete;
	PlayerDoodle(DragonJumpFramework& _framework) = delete;
	PlayerDoodle(DragonJumpFramework& _framework, const Vector2Df& pos);

	[[nodiscard]] virtual bool DrawIfActive(float cameraVerticalOffset) override;
	virtual bool Reactivate(const Vector2Df& positionX_velocityY) override;

	virtual void ReceiveTick(float deltaTime) override;
	virtual float GetMaxTickDeltaTime() const override { return 0.02f; };

	virtual CollisionChannel GetCollisionChannel() const override;
	virtual bool GetCollisionResponse(CollisionChannel channel) const override;
	virtual void ReceiveCollision(CollidableBase& other);

	virtual void AddImpulse(const Vector2Df& force, float duration = -0.f);
	bool StartShooting();
	bool StopShooting(Vector2D& outTargetPosition);
	Vector2Df GetMouthGlobalPos() const 
		{ return position; };
	bool IsStanding() const 
		{ return standingTimeLeft >= 0.f && onJumpDelegate; };
	void SetPosition(const Vector2Df& inPosition) { position = inPosition; };
	float GetCalculateJumpDistance() const;
	//float GetJumpingVelocity() const { return jumpingVelocity; };
protected:
	[[nodiscard]] virtual bool DrawIfActive_Internal() override;
	void ProcessDraw();
	void DampCurrentVelocity(float deltaTime, float forceLengthSquared);
	//void TryStartStanding(SteppableOnBase& other);
	void StopStanding();
	void StartDying();
	
	template<int index = 0, int maxIndex = 5>
	void SetOffsets();

	std::vector<SpriteInfo> sprites;
	int mainSpriteToDraw{ 0 };
	float mass = 75.f;
	float gravityForce = 300.f;
	float jumpImpulseMultiplier = -100.f;
	float jumpingVelocity = -300.f;
	std::list<std::pair<Vector2Df, float>> additionalForces;
	float minVelocityAndForce = 0.5f;
	float maxVelocityLengthSquared = 350000.f;
	Vector2Df dampingMultiplier = { -0.03f, -0.00001f };

	float standingTime = 0.06f;
	float standingTimeLeft = -1.f;
	//pointer to SteppableOnBase?
	std::function<void(PlayerDoodle&)> onJumpDelegate = nullptr;

	bool bIsAiming = false;

	float fallingAnimationDuration = 3.5f;
	float fallingAnimationTimeLeft = -1.f;
	const Hole* controllingHole = nullptr;

	float abilityDurationLeft = -1.f;

	float knockoutAnimationDuration = 1.f;
	float knockoutAnimationTimeLeft = -1.f;
	std::list< std::shared_ptr<Sprite>> knockoutSprites;
};

template<int index, int maxIndex>
inline void PlayerDoodle::SetOffsets()
{
	static_assert(index >= 0 && maxIndex >= 0, "indexes must be positive");
	if constexpr (index < maxIndex) {
		if constexpr (index % 2) {
			sprites.at(index).offset.x *= 0.66f;
		}
		else {
			sprites.at(index).offset.x *= 0.33f;
		}
		sprites.at(index).offset.y *= 0.5f;
		SetOffsets<index + 1, maxIndex>();
	}
}