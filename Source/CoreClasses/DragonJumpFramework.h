// by Dmitry Kolontay

#pragma once

#include <map>
#include <memory>
#include <vector>
#include "Framework.h"
#include <random>

class Drawable;
class Platform;
class TickablePlatform;
class PlayerDoodle;
class Projectile;
class MonsterBase;
class TickableMonster;
class Hole;
class Ability;
class Button;

enum class PlatformType;
enum class MonsterType;
enum class AbilityType;

#ifdef NDEBUG
#define DJLog(...) ((void)0)
#else
#include <iostream>
template<typename... Args>
void DebugMsg(Args&&... args)
{
    (std::cout << ... << args) << '\n';
}
#define DJLog(...) DebugMsg(__VA_ARGS__)
#endif

struct Vector2D {
	constexpr Vector2D() : x{ 0 }, y{ 0 } {};
	constexpr Vector2D(const int& _x, const int& _y) : 
		x{ _x }, y{ _y } {};
	int x;
	int y;
	constexpr int LengthSquared() const
		{ return x * x + y * y; };

	bool IsPointInsideRectangle(const Vector2D& leftUpper, const Vector2D& rightLower) const

		{ return x >= leftUpper.x && x <= leftUpper.x && 
			y >= leftUpper.y && y <= rightLower.y; }

	explicit operator bool() const
		{ return x != 0 && y != 0; }

	Vector2D& operator=(const Vector2D& other) {
		if (this == &other)
			return *this;
		this->x = other.x;
		this->y = other.y;
		return *this;
	}
	Vector2D& operator+=(const Vector2D& other) {
		this->x += other.x;
		this->y += other.y;
		return *this;
	}
	Vector2D& operator-=(const Vector2D& other) {
		this->x -= other.x;
		this->y -= other.y;
		return *this;
	}
	Vector2D& operator*=(const int& other) {
		this->x *= other;
		this->y *= other;
		return *this;
	}
	Vector2D& operator/=(const int& other) {
		this->x /= other;
		this->y /= other;
		return *this;
	}
	Vector2D operator+(const Vector2D& other) const {
		return Vector2D{ this->x + other.x, this->y + other.y };
	}
	Vector2D operator-(const Vector2D& other) const {
		return Vector2D{ this->x - other.x, this->y - other.y };
	}
	Vector2D operator*(const int& other) const {
		return Vector2D{ this->x * other, this->y * other };
	}
	Vector2D operator/(const int& other) const {
		return Vector2D{ this->x / other, this->y / other };
	}
	Vector2D operator-(const int& other) {
		return Vector2D{ this->x - other, this->y - other };
	}
	Vector2D operator+(const int& other) {
		return Vector2D{ this->x + other, this->y + other };
	}
	Vector2D operator>>(const int& other) const {
		return Vector2D{ this->x >> other, this->y >> other };
	}
};

struct Vector2Df {
	constexpr Vector2Df() :
		x{ 0.f }, y{ 0.f } {};

	Vector2Df(const float& _x) : 
		x{ _x }, y{ _x } {};

	Vector2Df(const int& _x) : 
		x{ static_cast<float>(_x) }, y{ static_cast<float>(_x) } {};

	constexpr Vector2Df(const float& _x, const float& _y) :
		x{ _x }, y{ _y } {};

	Vector2Df(const int& _x, const int& _y) : 
		x{ static_cast<float>(_x) }, y{ static_cast<float>(_y) } {};

	Vector2Df(const Vector2Df& other) : 
		x{ other.x }, y{ other.y } {};

	Vector2Df(const Vector2D& other) : 
		x{ static_cast<float>(other.x) }, y{ static_cast<float>(other.y) } {};

	float x;
	float y;

	constexpr float LengthSquared() const { return x * x + y * y; };

	explicit operator bool() const
		{ return x != 0.f && y != 0.f; }

	void TurnThisToUnit() {
		float length{ sqrtf(LengthSquared()) };
		x /= length;
		y /= length;
	}
	bool IsPointInsideRectangle(const Vector2Df& leftUpper, const Vector2Df& rightLower) const 
		{ return x >= leftUpper.x && x <= rightLower.x && y >= leftUpper.y && y <= rightLower.y; }

	Vector2Df ToUnit() const
		{ return *this / sqrtf(LengthSquared()); }

	Vector2D ToInt() const 
		{ return { static_cast<int>(x + 0.5f),  static_cast<int>(y + 0.5f) }; };

	Vector2Df& operator=(const Vector2Df& other) {
		if (this == &other)
			return *this;
		this->x = other.x;
		this->y = other.y;
		return *this;
	}
	Vector2Df& operator+=(const Vector2Df& other) {
		this->x += other.x;
		this->y += other.y;
		return *this;
	}
	Vector2Df& operator-=(const Vector2Df& other) {
		this->x -= other.x;
		this->y -= other.y;
		return *this;
	}
	Vector2Df& operator*=(const float& other) {
		this->x *= other;
		this->y *= other;
		return *this;
	}
	Vector2Df& operator*=(const Vector2Df& other) {
		this->x *= other.x;
		this->y *= other.y;
		return *this;
	}
	Vector2Df& operator/=(const float& other) {
		this->x /= other;
		this->y /= other;
		return *this;
	}
	Vector2Df operator*(const Vector2Df& other) const {
		return Vector2Df{ this->x * other.x, this->y * other.y };
	}
	Vector2Df operator+(const Vector2Df& other) const {
		return Vector2Df{ this->x + other.x, this->y + other.y };
	}
	Vector2Df& operator=(const Vector2D& other) {
		//no self guard
		this->x = static_cast<float>(other.x);
		this->y = static_cast<float>(other.y);
		return *this;
	}
	Vector2Df operator-(const Vector2Df& other) const {
		return Vector2Df{ this->x - other.x, this->y - other.y };
	}
	Vector2Df operator*(const float& other) const {
		return Vector2Df{ this->x * other, this->y * other };
	}
	Vector2Df operator/(const float& other) const {
		return Vector2Df{ this->x / other, this->y / other };
	}
	Vector2Df operator-(const float& other) {
		return Vector2Df{ this->x - other, this->y - other };
	}
	Vector2Df operator+(const float& other) {
		return Vector2Df{ this->x + other, this->y + other };
	}

	Vector2Df operator*(const int& other) const {
		return Vector2Df{ this->x * static_cast<float>(other), this->y * static_cast<float>(other) };
	}
	Vector2Df operator/(const int& other) const {
		return Vector2Df{ this->x / static_cast<float>(other), this->y / static_cast<float>(other) };
	}
	Vector2Df operator-(const int& other) {
		return Vector2Df{ this->x - static_cast<float>(other), this->y - static_cast<float>(other) };
	}
	Vector2Df operator+(const int& other) {
		return Vector2Df{ this->x + static_cast<float>(other), this->y + static_cast<float>(other) };
	}

};

struct SpriteInfo {
	SpriteInfo() : 
		sprite{ nullptr }, offset{ 0.f, 0.f } {};

	SpriteInfo(const std::shared_ptr<Sprite>& _sprite) : 
		sprite{ _sprite }, offset{ 0, 0 } {};

	SpriteInfo(const std::shared_ptr<Sprite>& _sprite, const float& x, const float& y) : 
		sprite{ _sprite }, offset{ x, y } {};

	SpriteInfo(const std::shared_ptr<Sprite>& _sprite, const int& x, const int& y) : 
		sprite{ _sprite }, offset{ x, y } {};

	SpriteInfo(const std::shared_ptr<Sprite>& _sprite, const Vector2Df& vector) : 
		sprite{ _sprite }, offset{ vector } {};

	SpriteInfo(const std::shared_ptr<Sprite>& _sprite, const Vector2D& vector) : 
		sprite{ _sprite }, offset{ static_cast<float>(vector.x), static_cast<float>(vector.y) } {};

	std::shared_ptr<Sprite> sprite;

	Vector2Df offset;

	void Draw(const Vector2Df& vector) const {
		drawSprite(sprite.get(), static_cast<int>(vector.x - offset.x + 0.5f), static_cast<int>(vector.y - offset.y + 0.5f));
	};

	void Draw(const float& posX, const float& posY) const {
		drawSprite(sprite.get(), static_cast<int>(posX - offset.x + 0.5f), static_cast<int>(posY - offset.y + 0.5f));
	};

	void Draw(const Vector2D& vector) const {
		drawSprite(sprite.get(), vector.x - static_cast<int>(offset.x + 0.5f), vector.y - static_cast<int>(offset.y + 0.5f));
	};

	void Draw(const int& posX, const int& posY) const {
		drawSprite(sprite.get(), posX - static_cast<int>(offset.x + 0.5f), posY - static_cast<int>(offset.y + 0.5f));
	};

	void Reset() {
		sprite.reset();
		offset = 0.f;
	}

	explicit operator bool() const
		{ return sprite.get() && offset; }
};

struct PlayerState {
	PlayerState() :
		lifesLeft{ lifes }, distancePassed{ 0.f }, 
		jumps{ 0 } {};
	int lifesLeft;
	float distancePassed;
	int jumps;

	void Reset() {
		lifesLeft = lifes;
		distancePassed = 0.f;
		jumps = 0;
	}
protected:
#ifdef NDEBUG
	static constexpr int lifes = 5;
#else
	static constexpr int lifes = 2;
#endif
};

template<class TDrawable>
using vectorShared = std::vector<std::shared_ptr<TDrawable>>;

//maybe should have gone with singleton after all
class DragonJumpFramework : public Framework
{
public:
	DragonJumpFramework(bool bStartWithMenu = true, int sizeX = 450, int sizeY = 600, bool bFullscreen = false);
	[[nodiscard]] Vector2D GetSize() const { return screenSize; };
	[[nodiscard]] int GetDoodleInputDirection() const { return doodleInputDirection; };
	//
	[[nodiscard]] bool GetSprite(const std::string& path, std::shared_ptr<Sprite>& outSprite);
	[[nodiscard]] bool GetSprite(const char* path, std::shared_ptr<Sprite>& outSprite)
		{ return GetSprite(std::string{ path }, outSprite); };

	[[nodiscard]] Vector2D GetSpriteSize(const std::shared_ptr<Sprite>& sprite) const;
	void GetSpriteSize(const std::shared_ptr<Sprite>& sprite, Vector2D& vector) const;
	void GetSpriteSize(const std::shared_ptr<Sprite>& sprite, Vector2Df& vector) const;

	[[nodiscard]] SpriteInfo GetSpriteInfo(const std::string& path);
	[[nodiscard]] SpriteInfo GetSpriteInfo(const char* path)
		{ return GetSpriteInfo(std::string{ path }); };

	[[nodiscard]] bool GetSpriteInfo(const std::string& path, SpriteInfo& spriteInfo);
	[[nodiscard]] bool GetSpriteInfo(const char* path, SpriteInfo& spriteInfo)
		{ return GetSpriteInfo(std::string{ path }, spriteInfo); };

	[[nodiscard]] std::vector<SpriteInfo> GetNumberedSprites(const std::string& pathBase, int startI = 0);
	[[nodiscard]] std::vector<SpriteInfo> GetNumberedSprites(const char* pathBase, int startI = 0)
		{ return GetNumberedSprites(std::string{ pathBase }, startI); };

	[[maybe_unused]] bool GetNumberedSprites(const std::string& pathBase, std::vector<SpriteInfo>& vec, int startI = 0);
	[[maybe_unused]] bool GetNumberedSprites(const char* pathBase, std::vector<SpriteInfo>& vec, int startI = 0)
		{ return GetNumberedSprites(std::string{ pathBase }, vec, startI); };
	//
	[[nodiscard]] PlayerDoodle* GetPlayerDoodle() { return playerDoodle.get(); };
	[[nodiscard]] Vector2D GetMousePosition() const { return mousePosition; }
	//
	[[nodiscard]] int GetLifesLeft() const 
		{ return playerState.lifesLeft; };
	[[nodiscard]] int GetDistance() const 
		{ return static_cast<int>(playerState.distancePassed); };
	[[nodiscard]] int GetJumps();

	void IncreaseMonstersKilledCounter() { ++monstersKilledSinceAbilitySpawn; };
	void IncreaseJumpsCounter() { ++playerState.jumps; };
	//
	[[nodiscard]] bool IsInScreenArea(const Vector2Df& pos, const Vector2Df& size,
		bool checkSides = true, bool checkUpper = false) const;
	[[nodiscard]] bool IsInScreenArea(const Vector2Df& pos, const std::shared_ptr<Sprite>& sprite,
		bool checkSides = true, bool checkUpper = false) const
		{ return IsInScreenArea(pos, Vector2Df{ GetSpriteSize(sprite) }, checkSides, checkUpper); }
	
	[[nodiscard]] int IsOutOfSideBorder(const Vector2Df& pos, const Vector2Df& size, bool yAxis = false) const;
	[[nodiscard]] int IsOutOfSideBorder(const Vector2Df& pos, const std::shared_ptr<Sprite>& sprite, bool yAxis = false) const
		{ return IsOutOfSideBorder(pos, Vector2Df{ GetSpriteSize(sprite) }, yAxis); }

	[[nodiscard]] int DoesSpriteTouchBorder(const Vector2Df& pos, const Vector2Df& size, bool yAxis = false) const;
	[[nodiscard]] int DoesSpriteTouchBorder(const Vector2Df& pos, const std::shared_ptr<Sprite>& sprite, bool yAxis = false) const
		{ return DoesSpriteTouchBorder(pos, Vector2Df{ GetSpriteSize(sprite) }, yAxis); }
	
	static constexpr float inputImpulseAbs = 25.f;
	static constexpr float platformsInScreenWidth = 5.5f;
	static constexpr Vector2D minimalScreenSize = { 300, 400 };
protected:
	virtual void PreInit(int& width, int& height, bool& fullscreen) override;
	[[nodiscard]] virtual bool Init() override;
	[[nodiscard]] virtual bool Tick() override 
		{ return std::invoke(this->tickFunction, this); };
	virtual const char* GetTitle() override 
		{ return "dimkKy"; };
	virtual void Close() override {};
	//
	[[nodiscard]] bool MenuTick();
	[[nodiscard]] bool BeginPlay();
	[[nodiscard]] bool GameplayTick();
	[[nodiscard]] float GetDeltaTime();
	//
	virtual void onMouseMove(int x, int y, int xRelative, int yRelative) override;
	virtual void onMouseButtonClick(FRMouseButton button, bool bReleased) override;
	virtual void onKeyPressed(FRKey key) override;
	virtual void onKeyReleased(FRKey key) override;
	//
	[[nodiscard]] bool CreateUI();
	void SpawnStartingScene();
	void TryAddEntities();
	void OnStartButtonClicked();
	[[nodiscard]] PlatformType GetPlatformType(int seed);
	[[nodiscard]] MonsterType GetMonsterType(int seed);
	//
	[[maybe_unused]] Platform* SpawnPlatform(Vector2Df pos, PlatformType type);
	[[maybe_unused]] Ability* SpawnAbility(const Platform& target, AbilityType type, int cost = abilityCost);
	//
	[[maybe_unused]] MonsterBase* SpawnMonster(const Platform& target, MonsterType type);
	[[maybe_unused]] Projectile* SpawnProjectile(const Vector2Df& target);

	
	void SubstepWorldTicks(float deltaTime);
	void DispatchWorldTicks(float deltaTime);
	void SubstepProjectilesTicksAndCollisions(float deltaTime);
	void DispatchProjectilesTicksAndCollisions(float deltaTime);
	void SubstepPlayerTickAndCollisions(float deltaTime, bool dispatchInput);
	void DispatchPlayerTickAndCollisions(float deltaTime, bool bDispatchInput);
	void DetectDispatchPlayerCollisions();

	Vector2D screenSize;
	bool bFullscreen;
	bool bWithMenu;
	bool bInMenu;
	unsigned int ticksCount;
	bool (DragonJumpFramework::* tickFunction)(void);
	float spriteScale;

	PlayerState playerState;
	int doodleInputDirection;
	int monstersKilledSinceAbilitySpawn;
	int lastDisplayedJumps;

	static constexpr int abilityCost = 5;

	template<class TDrawable>
		requires std::is_base_of<Drawable, TDrawable>::value
	static void ClearAll(std::vector<std::shared_ptr<TDrawable>>& vec) {
		vec.clear();
	}
	template<typename FDrawable, typename...SDrawable>
		requires std::is_base_of<Drawable, FDrawable>::value &&
	((std::is_base_of<Drawable, SDrawable>::value) &&...)
		static void ClearAll(std::vector<std::shared_ptr<FDrawable>>& vec,
			std::vector<std::shared_ptr<SDrawable>>&...vecs) {
		vec.clear();
		ClearAll(vecs...);
	}
	
	//---------
	std::map<std::string, std::shared_ptr<Sprite>> loadedSprites;
	std::shared_ptr<PlayerDoodle> playerDoodle;
	Vector2Df lastPlatformPos;
	vectorShared<Hole> holes;
	vectorShared<TickableMonster> tickableMonsters;
	vectorShared<Drawable> menuUIs;
	vectorShared<Drawable> gametimeUIs;
	vectorShared<Platform> platforms;
	vectorShared<TickablePlatform> tickablePlatforms;
	vectorShared<Ability> abilities;
	vectorShared<Projectile> projectiles;
	Vector2D mousePosition;
};

