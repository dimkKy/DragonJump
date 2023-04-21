#include "DragonJumpFramework.h"
#include <iostream>
#include <cassert>
#include <algorithm>
#include <cmath>
#include <random>
#include <functional>
#include <filesystem>
#include <string>
#include <regex>
#include "wtypes.h"
#include "SpriteLoader.h"
#include "Platform.h"
#include "PlayerDoodle.h"
#include "Projectile.h"
#include "Hole.h"
#include "Button.h"
#include "Ability.h"
#include "AnimatedPlatform.h"
#include "TickableMonster.h"
#include "IntDisplayer.h"
#include "LifesDisplayer.h"
#include "Collider.h"

DragonJumpFramework::DragonJumpFramework(bool bStartWithMenu, int sizeX, int sizeY, bool _bFullscreen) :
	screenSize{ sizeX, sizeY }, bFullscreen{ _bFullscreen }, bWithMenu{ bStartWithMenu }, bInMenu{ bStartWithMenu }, 
	ticksCount{ 0 }, tickFunction { &DragonJumpFramework::BeginPlay }, spriteScale{ 1.f }, doodleInputDirection{ 0 },
	inputImpulseAbs{ 25.f }, platformsInScreenWidth{ 5.5f }, lifesLeft{ 0 }, distancePassed{ 0.f }, 
	monstersKilledSinceAbilitySpawn{ 0 }
{
	if (!bFullscreen) {
		if (sizeX < 300)
			screenSize.x = 300;
		if (sizeY < 400)
			screenSize.y = 400;
	}
}

bool DragonJumpFramework::IsInScreenArea(const Vector2Df& pos, const Vector2Df& size, 
	bool bCheckSides, bool bCheckUpper) const
{
	int vertical{ IsOutOfSideBorder(pos, size, true) };
	bool isInVertical{ (!bCheckSides || IsOutOfSideBorder(pos, size) == 0) };
	bool test2{ (bCheckUpper ? vertical == 0 : vertical < 1) };
	return (!bCheckSides || IsOutOfSideBorder(pos, size) == 0) && 
		(bCheckUpper ? vertical == 0 : vertical < 1);
}

int DragonJumpFramework::IsOutOfSideBorder(const Vector2Df& pos, const Vector2Df& size, bool yAxis) const
{
	assert(size.x >= 0.f && size.y >= 0.f && "size must be positive");
	Vector2Df border{};
	if (yAxis) {
		border.x = pos.y - size.y * 0.5f;
		border.y = pos.y + size.y * 0.5f;
	}
	else {
		border.x = pos.x - size.x * 0.5f;
		border.y = pos.x + size.x * 0.5f;
	}
	return static_cast<bool>(border.y >= 0.f) - 
		static_cast<bool>(border.x <= (yAxis ? screenSize.y : screenSize.x));
}

int DragonJumpFramework::DoesSpriteTouchBorder(const Vector2Df& pos, const Vector2Df& size, bool yAxis) const
{
	assert(size.x >= 0.f && size.y >= 0.f && "size must be positive");
	Vector2Df border{ pos };
	//template with shapes instead?
	border = yAxis ? (border.y - size.y, border.y + size.y) : (border.x - size.x, border.y + size.x);
	float screenB{ yAxis ? static_cast<float>(screenSize.y) : static_cast<float>(screenSize.x) };
	return (border.x < 0.f && border.y > 0.f) - (border.x < screenB && border.y > screenB);
}


void DragonJumpFramework::PreInit(int& width, int& height, bool& fullscreen)
{
	DJLog("preinit");
	if (bFullscreen) {
		const HWND desktopHandle{ GetDesktopWindow() };
		RECT desktopSize;
		GetWindowRect(desktopHandle, &desktopSize);
		width = desktopSize.right;
		height = desktopSize.bottom;
	}
	else {
		width = screenSize.x;
		height = screenSize.y;
	}
	fullscreen = bFullscreen;
}

bool DragonJumpFramework::Init()
{
	getScreenSize(screenSize.x, screenSize.y);
	std::shared_ptr<Sprite> tempSprite;
	if (!GetSprite(SpritePaths::defaultPlatform, tempSprite)) {
		DJLog("defaultPlatform sprite not found, aborting");
		return false;
	}
	Vector2D platfromSize{ GetSpriteSize(tempSprite) };
	float desiredSizeX{ screenSize.x / platformsInScreenWidth };
	spriteScale = desiredSizeX / platfromSize.x;
	setSpriteSize(tempSprite.get(), static_cast<int>(desiredSizeX + 0.5f), static_cast<int>(platfromSize.y * spriteScale + 0.5f));
	if (!GetSprite(SpritePaths::projectile, tempSprite)) {
		DJLog("projectile sprite not found, aborting");
		return false;
	}
	if(bWithMenu)
		playerDoodle = std::make_shared<PlayerDoodle>(*this, Vector2Df{ platfromSize.x, screenSize.y + platfromSize.y });
	else
		playerDoodle = std::make_shared<PlayerDoodle>(*this, Vector2Df{ screenSize.x >> 1, screenSize.y + platfromSize.y });
	if (!playerDoodle->IsActive()) {
		DJLog("playerDoodle init unsuccessful, aborting");
		return false;
	}
	if (!CreateUI() && bWithMenu) {
		DJLog("UI creation unsuccessfull, aborting");
		return false;
	}
	srand(static_cast<unsigned int>(time(NULL)));
	return true;
}

void DragonJumpFramework::onMouseMove(int x, int y, int xRelative, int yRelative)
{
	mousePosition.x = x;
	mousePosition.y = y;
}

void DragonJumpFramework::onMouseButtonClick(FRMouseButton button, bool bReleased)
{
	if (button != FRMouseButton::LEFT)
		return;
	for (auto& drawable : menuUIs) {
		if (Button* button{ dynamic_cast<Button*>(drawable.get()) }) {
			if (button->OnMouseButtonClick(mousePosition, bReleased))
				return;
		}
	}
	if (tickFunction == &DragonJumpFramework::GameplayTick) {
		if (bReleased) {
			Vector2D targetPos{ mousePosition };
			if (playerDoodle->StopShooting(targetPos)) {
				SpawnProjectile(targetPos);
			}
		}
		else
			playerDoodle->StartShooting();
	}
}

void DragonJumpFramework::onKeyPressed(FRKey key)
{
	switch (key) {
	case FRKey::RIGHT:
		doodleInputDirection += 1;
		break;
	case FRKey::LEFT:
		doodleInputDirection -= 1;
		break;
	default:
		break;
	}
}

void DragonJumpFramework::onKeyReleased(FRKey key)
{
	switch (key) {
	case FRKey::RIGHT:
		doodleInputDirection -= 1;
		break;
	case FRKey::LEFT:
		doodleInputDirection += 1;
		break;
	default:
		break;
	}
}

bool DragonJumpFramework::GetSprite(const std::string& path, std::shared_ptr<Sprite>& outSprite) 
{
	if (auto result{ loadedSprites.find(path) }; 
		result != loadedSprites.end()) {
		outSprite = result->second;
		return true;
	}
	else {
		if (!std::filesystem::exists(path))
			return false;
		if (Sprite* possibleSprite{ createSprite(path.c_str()) }) {
			int tempX, tempY;
			getSpriteSize(possibleSprite, tempX, tempY);
			setSpriteSize(possibleSprite, static_cast<int>(tempX * spriteScale + 0.5f), 
				static_cast<int>(tempY * spriteScale + 0.5f));
			std::shared_ptr<Sprite> shared{ possibleSprite, destroySprite };
			loadedSprites.insert({ path, shared });
			outSprite =  shared;
			return true;
		}
		else {
			return false;
		}
	}
}

Vector2D DragonJumpFramework::GetSpriteSize(const std::shared_ptr<Sprite>& sprite) const
{
	Vector2D out;
	getSpriteSize(sprite.get(), out.x, out.y);
	return out;
}

void DragonJumpFramework::GetSpriteSize(const std::shared_ptr<Sprite>& sprite, Vector2D& vector) const
{
	getSpriteSize(sprite.get(), vector.x, vector.y);
}
void DragonJumpFramework::GetSpriteSize(const std::shared_ptr<Sprite>& sprite, Vector2Df& vector) const
{
	vector = GetSpriteSize(sprite);
}

SpriteInfo DragonJumpFramework::GetSpriteInfo(const std::string& path)
{
	SpriteInfo out;
	if (GetSprite(path, out.sprite)) {
		GetSpriteSize(out.sprite, out.offset);
	}
	return out;
}

bool DragonJumpFramework::GetSpriteInfo(const std::string& path, SpriteInfo& spriteInfo)
{
	if (GetSprite(path, spriteInfo.sprite)) {
		GetSpriteSize(spriteInfo.sprite, spriteInfo.offset);
		return true;
	}
	else {
		return false;
	}
}

std::vector<SpriteInfo> DragonJumpFramework::GetNumberedSprites(const std::string& pathBase)
{
	std::vector<SpriteInfo> out;
	const std::regex fileFormatDelim{ "\\.[^\\.]*?$" };
	std::smatch match;
	std::regex_search(pathBase, match, fileFormatDelim);
	for (auto i{ 0 }; ; ++i) {
		std::string path{ pathBase };	
		if (SpriteInfo info{ GetSpriteInfo(path.insert(match.position(), std::to_string(i)))})
			out.push_back(info);
		else
			break;
	}
	return out;
}

bool DragonJumpFramework::GetNumberedSprites(const std::string& pathBase, std::vector<SpriteInfo>& vec, int index)
{
	const std::regex fileFormatDelim{ "\\.[^\\.]*?$" };
	std::smatch match;
	std::regex_search(pathBase, match, fileFormatDelim);
	vec.clear();
	for (; ; ++index) {
		std::string path{ pathBase };
		if (SpriteInfo info{ GetSpriteInfo(path.insert(match.position(), std::to_string(index))) }) {
			vec.push_back(info);
		}		
		else {
			break;
		}
	}
	return vec.size();
}

void DragonJumpFramework::TryAddEntities()
{
	std::random_device seed;
	std::mt19937 rnd(seed());
	std::uniform_real_distribution<float> dist(0.55f, 0.85f);
	std::uniform_int_distribution<int> prcnt(1, INT_MAX);
	while (lastPlatformPos.y > screenSize.y * -0.3f) {
		lastPlatformPos.y -= dist(rnd) * PlayerDoodle::jumpHeight;
		lastPlatformPos.x = std::fmodf(lastPlatformPos.x + dist(rnd) * screenSize.x, 
			static_cast<float>(screenSize.x));
		Platform* newP{ SpawnPlatform(lastPlatformPos, PlatformType::PT_Default) };
		if (!newP)
			return;
		int randint{ prcnt(rnd) };
		if (auto type{ GetPlatformType(randint) }; type != PlatformType::PT_COUNT) {
			SpawnPlatform({ std::fmodf(lastPlatformPos.x + dist(rnd) * screenSize.x, 
				static_cast<float>(screenSize.x)), lastPlatformPos.y }, type);
		}
		if (monstersKilledSinceAbilitySpawn > 5) {
			if (randint % 2 == 0)
				SpawnAbility(*newP, AbilityType::AT_Jet);
			else
				SpawnAbility(*newP, AbilityType::AT_Shield);
			return;
		}
		if (MonsterType type{ GetMonsterType(randint) }; type != MonsterType::MT_COUNT) {
			SpawnMonster(*newP, type);
		}
	}
}

PlatformType DragonJumpFramework::GetPlatformType(int seed)
{
	if (seed % 3 == 0) {
		return PlatformType::PT_Weak;
	}
	if (seed % 5 == 0) {
		return PlatformType::PT_Trampoline;
	}
	if (seed % 6 == 0 || seed % 23 == 0) {
		return PlatformType::PT_OneOff;
	}
	if (seed % 7 == 0 || seed % 29 == 0) {
		return PlatformType::PT_Invisible;
	}
	if (seed % 11 == 0 || seed % 37 == 0) {
		return PlatformType::PT_SelfDestuct;
	}
	return PlatformType::PT_COUNT;
}

MonsterType DragonJumpFramework::GetMonsterType(int seed)
{
	if (seed % 10 == 0 || seed % 31 == 0) {
		return MonsterType::MT_Hole;
	}
	if (seed % 14 == 0 || seed % 41 == 0) {
		return MonsterType::MT_Static;
	}
	if (seed % 17 == 0 || seed % 43 == 0) {
		return MonsterType::MT_Movable;
	}
	return MonsterType::MT_COUNT;
}

void DragonJumpFramework::SpawnStartingScene()
{
	Vector2Df platPos{ playerDoodle->GetPosition() };
	platPos.y -= PlayerDoodle::jumpHeight;
	if (bInMenu) {
		SpawnPlatform(platPos, PlatformType::PT_Default);
		platPos.y -= PlayerDoodle::jumpHeight;
		SpawnPlatform(platPos, PlatformType::PT_Weak);
	}
	else {
		int platformNumber{ static_cast<int>(platformsInScreenWidth) };
		float platformWidth{ screenSize.x / platformsInScreenWidth };
		float spawnPositionX{ (screenSize.x - ( platformNumber - 1 ) * platformWidth) * 0.5f };
		for (auto i{ 0 }; i < platformNumber; i++) {
			SpawnPlatform({ spawnPositionX, platPos.y }, PlatformType::PT_Default);
			spawnPositionX += platformWidth;
		}
		SpawnAbility(*platforms.back().get(), AbilityType::AT_Jet);
		SpawnPlatform({ screenSize.x * 0.25f, platPos.y -= PlayerDoodle::jumpHeight * 0.5f}, PlatformType::PT_Invisible);
		SpawnPlatform({ screenSize.x * 0.15f, platPos.y -= PlayerDoodle::jumpHeight * 0.5f }, PlatformType::PT_Trampoline);
		/*SpawnMonster(
			*SpawnPlatform({ screenSize.x * 0.5f, platPos.y -= PlayerDoodle::jumpHeight * 0.5f }, PlatformType::PT_SelfDestuct),
			MonsterType::MT_Hole);
		SpawnMonster(
			*SpawnPlatform({ screenSize.x * 0.75f, platPos.y -= PlayerDoodle::jumpHeight * 0.5f }, PlatformType::PT_Trampoline),
			MonsterType::MT_Movable);*/
		lastPlatformPos.y = platPos.y;
		TryAddEntities();
	}
}

void DragonJumpFramework::OnStartButtonClicked()
{
	DJLog("start");
	bInMenu = false;
	tickFunction = &DragonJumpFramework::BeginPlay;
	for (auto& drawable : platforms)
		drawable->Deactivate();
	for (auto& drawable : tickablePlatforms)
		drawable->Deactivate();
}

void DragonJumpFramework::SubstepWorldTicks(float deltaTime)
{
	float maxDeltaTime{ TickablePlatform::GetMaxTickDeltaTime() };
	if (TickableMonster::GetMaxTickDeltaTime() < maxDeltaTime)
		maxDeltaTime = TickableMonster::GetMaxTickDeltaTime();
	while (deltaTime > maxDeltaTime) {
		deltaTime -= maxDeltaTime;
		DispatchWorldTicks(maxDeltaTime);
	}
	DispatchWorldTicks(deltaTime);
}

void DragonJumpFramework::DispatchWorldTicks(float deltaTime)
{
	Tickable::DispatchTicks(deltaTime, 
		tickablePlatforms, tickableMonsters);
}

void DragonJumpFramework::SubstepPlayerTickAndCollisions(float deltaTime, bool dispatchInput)
{
	float maxDeltaTime{ playerDoodle->GetMaxTickDeltaTime() };
	while (deltaTime > maxDeltaTime) {
		deltaTime -= maxDeltaTime;
		DispatchPlayerTickAndCollisions(maxDeltaTime, dispatchInput);
		dispatchInput = false;
	}
	DispatchPlayerTickAndCollisions(deltaTime, dispatchInput);
}

void DragonJumpFramework::DispatchPlayerTickAndCollisions(float deltaTime, bool bDispatchInput)
{
	assert(deltaTime <= playerDoodle->GetMaxTickDeltaTime() && "deltaTime is too high, use SubstepPlayerTickAndCollisions");
	if (bDispatchInput) {
		if (doodleInputDirection > 0)
			playerDoodle->AddImpulse({ inputImpulseAbs, 0.f }, 0);
		if (doodleInputDirection < 0)
			playerDoodle->AddImpulse({ -inputImpulseAbs, 0.f }, 0);
	}
	playerDoodle->ReceiveTick(deltaTime);
	DetectDispatchPlayerCollisions();
	playerDoodle->GetCollisionInfo().halfSize.x;
	if (int offset{ DoesSpriteTouchBorder(playerDoodle->GetPosition(), 
		playerDoodle->GetCollisionInfo().halfSize) }) {
		Vector2Df pos{ playerDoodle->GetPosition() };
		playerDoodle->SetPosition({ pos.x - offset * screenSize.x, pos.y });
		DetectDispatchPlayerCollisions();
		playerDoodle->SetPosition(pos);
	}
}

void DragonJumpFramework::DetectDispatchPlayerCollisions()
{
	Collider::DispatchCollisions(*playerDoodle, 
		abilities, holes, tickableMonsters, platforms, tickablePlatforms);
}

void DragonJumpFramework::SubstepProjectilesTicksAndCollisions(float deltaTime)
{
	float maxDeltaTime{ Projectile::GetMaxTickDeltaTimeStatic() };
	while (deltaTime > maxDeltaTime) {
		deltaTime -= maxDeltaTime;
		DispatchProjectilesTicksAndCollisions(maxDeltaTime);
	}
	DispatchProjectilesTicksAndCollisions(deltaTime);
}

void DragonJumpFramework::DispatchProjectilesTicksAndCollisions(float deltaTime)
{
	assert(deltaTime <= Projectile::GetMaxTickDeltaTimeStatic() 
		&& "deltaTime is too high, use SubstepProjectilesTicksAndCollisions");
	Tickable::DispatchTicks(deltaTime, projectiles);
	Collider::DispatchCollisions(projectiles, holes);
	Collider::DispatchCollisions(projectiles, tickableMonsters);
}

bool DragonJumpFramework::MenuTick()
{
	float deltaTime{ GetDeltaTime() };
	SubstepWorldTicks(deltaTime);
	SubstepPlayerTickAndCollisions(deltaTime, false);
	drawTestBackground();
	Drawable::DispatchDrawcalls(0.f, platforms, tickablePlatforms);
	if (!playerDoodle->DrawIfActive(0.f)) {
		tickFunction = &DragonJumpFramework::BeginPlay;
	}
	Drawable::DispatchDrawcalls(0.f, menuUIs);
	return false;
}

bool DragonJumpFramework::BeginPlay()
{
	//clean up
	if (bInMenu) {
		playerDoodle->Reactivate({ screenSize.x / platformsInScreenWidth, 0.f });
	}
	else {
		playerDoodle->Reactivate({ screenSize.x * 0.5f, 0.f });
	}
	lastPlatformPos.y = static_cast<float>(screenSize.y);
#ifdef NDEBUG
	lifesLeft = 4;
#else
	lifesLeft = 2;
#endif
	distancePassed = 0.f;
	//clean up done
	SpawnStartingScene();
	tickFunction = bInMenu ? &DragonJumpFramework::MenuTick : &DragonJumpFramework::GameplayTick;
	ticksCount = getTickCount();
	return false;
}

bool DragonJumpFramework::GameplayTick()
{
	float deltaTime{ GetDeltaTime() };
	SubstepWorldTicks(deltaTime);
	SubstepProjectilesTicksAndCollisions(deltaTime);
	SubstepPlayerTickAndCollisions(deltaTime, true);
	float cameraOffset{ 0.f };
	float playerPosY{ playerDoodle->GetPosition().y };
	if (float screenhalfY{ static_cast<float>(screenSize.y) * 0.5f }; playerDoodle->IsActive() && 
		playerPosY <= screenhalfY && playerDoodle->GetVelocity().y <= 0.f) {
		cameraOffset = screenhalfY - playerPosY + 0.5f;
		distancePassed += cameraOffset;
		lastPlatformPos.y += cameraOffset;
		TryAddEntities();
	}
	drawTestBackground();
	Drawable::DispatchDrawcalls(cameraOffset, holes, 
		platforms, abilities, tickablePlatforms, tickableMonsters);
	if (!playerDoodle->DrawIfActive(cameraOffset)) {
		if (lifesLeft > 0 && playerDoodle->IsActive()) {
			lifesLeft -= 1;
			playerDoodle->Reactivate({ playerDoodle->GetPosition().x, 0.f });
		}
		else {
			playerDoodle->Deactivate();
		}
		if (!playerDoodle->IsActive()) {
			//end gameplay
			bInMenu = bWithMenu;
			tickFunction = &DragonJumpFramework::BeginPlay;
			platforms.clear();
			holes.clear();
			tickableMonsters.clear();
			tickablePlatforms.clear();
			projectiles.clear();
			abilities.clear();	
		}
	}
	if (bInMenu) {
		Drawable::DispatchDrawcalls(cameraOffset, projectiles, menuUIs);
	}
	else {
		Drawable::DispatchDrawcalls(cameraOffset, projectiles, gametimeUIs);
	}
	return false;
}

float DragonJumpFramework::GetDeltaTime()
{
	unsigned int oldTicksCount{ ticksCount };
	ticksCount = getTickCount();
	return static_cast<float>(ticksCount - oldTicksCount) * 0.001f;
}

Platform* DragonJumpFramework::SpawnPlatform(Vector2Df pos, PlatformType type)
{
	assert(pos.x >= 0.f && pos.x <= screenSize.x && pos.y <= screenSize.y 
		&& "attempted spawn outside screen size");
	float platformHalfSize{ screenSize.x / platformsInScreenWidth * 0.5f };
	pos.x = std::clamp(pos.x, platformHalfSize, screenSize.x - platformHalfSize);
	if (type == PlatformType::PT_Default) {
		for (auto& platform : platforms) {
			if (!platform->IsActive() && platform->Reactivate(pos)) {
				return platform.get();
			}
		}
		if (auto t{ std::make_shared<Platform>(*this,pos) }; 
			t->IsActive()) {
			platforms.push_back(t);
			return t.get();
		}
	}
	if (type > PlatformType::PT_Default && type < PlatformType::PT_Vertical) {
		for (auto& platform : tickablePlatforms) {
			if (platform->GetPlatformType() == type &&
				!platform->IsActive() && platform->Reactivate(pos)) {
				return platform.get();
			}
		}
		if (auto t{ std::make_shared<AnimatedPlatform>(*this, pos, type) };
			t->IsActive()) {
			tickablePlatforms.push_back(t);
			return t.get();
		}
	}
	return nullptr;
}

MonsterBase* DragonJumpFramework::SpawnMonster(const Platform& target, MonsterType type)
{
	Vector2Df pos{ target.GetPosition() };
	assert(pos.x >= 0.f && pos.x <= screenSize.x && 
		pos.y <= screenSize.y && "attempted spawn outside screen size");
	//target.GetStandingPointY(pos.x, pos.y);
	if (type == MonsterType::MT_Hole) {
		for (auto& monster : holes) {
			if (!monster->IsActive() && monster->Reactivate(pos)) {
				return monster.get();
			}
		}
		if (auto t{ std::make_shared<Hole>(*this,pos) }; t->IsActive()) {
			holes.push_back(t);
			return t.get();
		}
	}
	if (type < MonsterType::MT_COUNT) {
		for (auto& monster : tickableMonsters) {
			if (monster->GetMonsterType() == type && 
				!monster->IsActive() && monster->Reactivate(pos))
				return monster.get();
		}
		if (auto t{ std::make_shared<TickableMonster>(*this, pos, type) };
			t->IsActive()) {
			tickableMonsters.push_back(t);
			return t.get();
		}
	}
	return nullptr;
}

Projectile* DragonJumpFramework::SpawnProjectile(const Vector2Df& target)
{
	for (auto& projectile : projectiles) {
		if (!projectile->IsActive() && projectile->Reactivate(target))
			return projectile.get();
	}
	if (auto t{ std::make_shared<Projectile>(*this, target) }; t->IsActive()) {
		projectiles.push_back(t);
		return t.get();
	}
	return nullptr;
}

Ability* DragonJumpFramework::SpawnAbility(const Platform& platform, AbilityType type)
{
	assert(type != AbilityType::AT_COUNT && "SpawnAbility reported wrong ability type");
	Vector2Df pos{};
	for (auto& ability : abilities) {
		if (ability->GetAbilityType() == type && ability->Reactivate(pos)) {
			monstersKilledSinceAbilitySpawn -= 5;
			return ability.get();
		}
	}
	if (auto t{ std::make_shared<Ability>(*this, type, pos) }; t->IsActive()) {
		abilities.push_back(t);
		monstersKilledSinceAbilitySpawn -= 5;
		return t.get();
	}
	return nullptr;
}

bool DragonJumpFramework::CreateUI()
{
	if (auto t{ std::make_shared<IntDisplayer>(
		*this, Vector2Df{screenSize.x, 0}) }; t->IsActive()) {
		gametimeUIs.push_back(t);
	}
	if (auto t{ std::make_shared<LifesDisplayer>(
		*this, Vector2Df{0.f}) }; t->IsActive()) {
		gametimeUIs.push_back(t);
	}
	if (!bWithMenu)
		return true;
	if (auto t{ std::make_shared<Button>(
		*this, Vector2Df{ screenSize.x * 0.6f, screenSize.y * 0.5f}, 
		SpritePaths::startButton) }; t->IsActive()) {
		menuUIs.push_back(t);
		t->onClicked = std::bind(&DragonJumpFramework::OnStartButtonClicked, this);
		return true;
	}
	else {
		return false;
	}
	return true;
}

