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

class PlayerDoodle : public Collidable<RectangleShape>, public Tickable
{
public:
	PlayerDoodle(DragonJumpFramework& _framework) = delete;
	PlayerDoodle(DragonJumpFramework& _framework, const Vector2Df& pos);

	virtual void AddImpulse(const Vector2Df& force, float duration = -0.f);
	bool StartShooting();
	bool StopShooting(Vector2D& outTargetPosition);
	Vector2Df GetMouthGlobalPos() const
		{ return position; };
	bool IsStanding() const
		{ return standingTimeLeft >= 0.f && standingOn; };

	[[nodiscard]] virtual bool DrawIfActive(float cameraVerticalOffset) override;
	virtual bool Reactivate(const Vector2Df& positionX_velocityY) override;

	virtual void ReceiveTick(float deltaTime) override;
	virtual float GetMaxTickDeltaTime() const override { return 0.02f; };

	virtual CollisionChannel GetCollisionChannel() const override;
	virtual bool GetCollisionResponse(CollisionChannel channel) const override;
	virtual void ReceiveCollision(CollidableBase& other);

	void SetPosition(const Vector2Df& inPosition) { position = inPosition; };

	static const float jumpHeight;
protected:
	[[nodiscard]] virtual bool DrawIfActive_Internal() override;
	[[maybe_unused]] int ProcessDraw();
	void DampCurrentVelocity(float deltaTime, float forceLengthSquared);
	void StartDying();
	std::vector<SpriteInfo> sprites;
	bool lastFaceDirection = false;
	float standingTimeLeft = -1.f;
	std::list<std::pair<Vector2Df, float>> activeForces;

	//std::function<void(PlayerDoodle&)> onJumpDelegate = nullptr;
	SteppableOnBase* standingOn = nullptr;
	bool bIsAiming = false;
	
	float fallingAnimationTimeLeft = -1.f;
	const Hole* controllingHole = nullptr;

	float knockoutAnimationTimeLeft = -1.f;
	std::list< std::shared_ptr<Sprite>> knockoutSprites;
	std::map<AbilityType, float> activeAbilities;

	//--------------
	template<int index = 0, int maxIndex = 4>
	void SetOffsets();
	constexpr static float CalculateJumpHeight();
	constexpr static float DampInstSpeed(float speed, float dampingMp);

	static constexpr float knockoutAnimationDuration = 1.f;
	static constexpr float fallingAnimationDuration = 3.5f;
	static constexpr float mass = 75.f;
	static constexpr float gravityForce = 400.f;
	static constexpr float jumpImpulse = -100.f;
	static constexpr float jumpImpulseDuration = 0.5f;
	static constexpr float jumpingVelocity = -350.f;
	static constexpr float minVelocityAndForce = 0.5f;
	static constexpr float maxVelocityLengthSquared = 350000.f;
	static constexpr Vector2Df dampingMultiplier = { -0.025f, -0.0001f };
	static constexpr float standingTime = 0.06f;
};

template<int index, int maxIndex>
inline void PlayerDoodle::SetOffsets()
{
	static_assert(index >= 0 && maxIndex >= 0, "indexes must be positive");
	if constexpr (index < maxIndex) {
		if constexpr (index % 2) {
			sprites.at(index).offset.x *= 0.33f;
		}
		else {
			sprites.at(index).offset.x *= 0.66f;
		}
		sprites.at(index).offset.y *= 0.5f;
		SetOffsets<index + 1, maxIndex>();
	}
}