#include "DragonJumpFramework.h"
//#include <iostream>
#include <cassert>
#include <algorithm>
#include <cmath>
#include <random>
#include <functional>
#include <filesystem>
#include <string>
#include "wtypes.h"
#include "SpriteLoader.h"
#include "Drawable.h"
#include "Collidable.h"
#include "Platform.h"
#include "PlayerDoodle.h"
#include "Projectile.h"
#include "Monster.h"
#include "Hole.h"
#include "Button.h"
#include "Ability.h"
#include "AnimatedPlatform.h"
#include "TickableMonster.h"
#include "IntDisplayer.h"
#include "LifesDisplayer.h"
#include <regex>


DragonJumpFramework::DragonJumpFramework(bool bStartWithMenu, int sizeX, int sizeY, bool _bFullscreen) :
	screenSize{ sizeX, sizeY }, bFullscreen{ _bFullscreen }, bWithMenu{ bStartWithMenu }, bInMenu{ bStartWithMenu }, 
	ticksCount{ 0 }, tickFunction { &DragonJumpFramework::BeginPlay }, spriteScale{ 1.f }, doodleInputDirection{ 0 },
	jumpHeight{ 0.f }, inputImpulseAbs{ 25.f }, platformsInScreenWidth{ 5.5f }, lifesLeft{ 0 }, 
	distancePassed{ 0.f }, monstersKilledSinceAbilitySpawn{ 0 }
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
	return (!bCheckSides || IsOutOfSideBorder(pos, size) == 0) && 
		(bCheckUpper ? vertical < -1 : vertical == 0);
	return false;
}

int DragonJumpFramework::IsOutOfSideBorder(const Vector2Df& pos, const Vector2Df& size, bool yAxis) const
{
	if (!size.y)
		return 0;
	Vector2Df border{ pos };
	//template with shapes instead?
	if (size.x) {
		border = yAxis ? (border.y - size.y, border.y + size.y) : (border.x - size.x, border.y + size.x);
	}
	else {
		float radius{ sqrtf(size.y) };
		border = yAxis ? (border.y - radius, border.y + radius) : (border.x - radius, border.y + radius);
	}
	return (border.y >= 0.f) - (border.x <= yAxis ? screenSize.y : screenSize.x);
}

int DragonJumpFramework::DoesSpriteTouchBorder(const Vector2Df& pos, const Vector2Df& size, bool yAxis) const
{
	if (!size.y)
		return 0;
	Vector2Df border{ pos };
	//template with shapes instead?
	if (size.x) {
		border = yAxis ? (border.y - size.y, border.y + size.y) : (border.x - size.x, border.y + size.x);
	}
	else {
		float radius{ sqrtf(size.y) };
		border = yAxis ? (border.y - radius, border.y + radius) : (border.x - radius, border.y + radius);
	}
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
		playerDoodle = std::make_shared<PlayerDoodle>(*this, platfromSize.x, screenSize.y + platfromSize.y);
	else
		playerDoodle = std::make_shared<PlayerDoodle>(*this, screenSize.x >> 1, screenSize.y + platfromSize.y);
	if (!playerDoodle->IsActive()) {
		DJLog("playerDoodle init unsuccessful, aborting");
		return false;
	}
	/*if (bWithMenu) {
		if (!GetSprite(SpritePaths::startButton, tempSprite)) {
			DJLog("startButton sprite not found, aborting");
			return false;
		}
		if (!GetSprite(SpritePaths::startButtonPressed, tempSprite)) {
			DJLog("startButtonPressed sprite not found, aborting");
			return false;
		}
	}*/
	if (!CreateUI() && bWithMenu) {
		DJLog("UI creation unsuccessfull, aborting");
		return false;
	}
	jumpHeight = playerDoodle->GetCalculateJumpDistance();
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
	if (auto result{ loadedSprites.find(path) }; result != loadedSprites.end()) {
		outSprite = result->second;
		return true;
	}
	else {
		if (!std::filesystem::exists(path))
			return false;
		if (Sprite* possibleSprite{ createSprite(path.c_str()) }) {
			int tempX, tempY;
			getSpriteSize(possibleSprite, tempX, tempY);
			setSpriteSize(possibleSprite, static_cast<int>(tempX * spriteScale + 0.5f), static_cast<int>(tempY * spriteScale + 0.5f));
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
	std::uniform_real_distribution<float> dist(0.35f, 0.55f);
	std::uniform_int_distribution<int> prcnt(1, INT_MAX);
	while (lastPlatformPos.y > screenSize.y * -0.3f) {
		lastPlatformPos.y -= dist(rnd) * jumpHeight;
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
}

void DragonJumpFramework::SpawnStartingScene()
{
	Vector2Df platPos{ playerDoodle->GetPosition() };
	platPos.y -= jumpHeight * 0.8f;
	if (bInMenu) {
		SpawnPlatform(platPos, PlatformType::PT_Default);
		platPos.y -= jumpHeight * 0.8f;
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
		SpawnPlatform({ screenSize.x * 0.25f, platPos.y -= jumpHeight * 0.5f}, PlatformType::PT_Invisible);
		SpawnPlatform({ screenSize.x * 0.15f, platPos.y -= jumpHeight * 0.5f }, PlatformType::PT_Trampoline);
		SpawnMonster(
			*SpawnPlatform({ screenSize.x * 0.5f, platPos.y -= jumpHeight * 0.5f }, PlatformType::PT_SelfDestuct),
			MonsterType::MT_Hole);
		SpawnMonster(
			*SpawnPlatform({ screenSize.x * 0.75f, platPos.y -= jumpHeight * 0.5f }, PlatformType::PT_Trampoline), 
			MonsterType::MT_Movable);
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
	for (auto& tickable : tickablePlatforms)
		if (tickable->IsActive())
			tickable->ReceiveTick(deltaTime);
	for (auto& tickable : tickableMonsters)
		if (tickable->IsActive())
			tickable->ReceiveTick(deltaTime);
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
	if (int offset{ screenSize.x * playerDoodle->CheckOutOfSideBorder() }) {
		Vector2Df playerPos{ playerDoodle->GetPosition() };
		playerDoodle->SetPosition({ playerPos.x - offset, playerPos.y });
		DetectDispatchPlayerCollisions();
		playerDoodle->SetPosition(playerPos);
	}
}

void DragonJumpFramework::DetectDispatchPlayerCollisions()
{
	for (auto& ability : abilities) {
		Collider::DetectDispatchCollision(*ability, *playerDoodle);
	}
	for (auto& hole : holes) {
		Collider::DetectDispatchCollision(*hole, *playerDoodle);
	}
	for (auto& monster : tickableMonsters) {
		Collider::DetectDispatchCollision(*monster, *playerDoodle);
	}
	if (playerDoodle->IsStanding())
		return;
	for (auto& platform : platforms) {
		Collider::DetectDispatchCollision(*platform, *playerDoodle);
	}
	for (auto& platform : tickablePlatforms) {
		Collider::DetectDispatchCollision(*platform, *playerDoodle);
	}
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
	assert(deltaTime <= Projectile::GetMaxTickDeltaTimeStatic() && "deltaTime is too high, use SubstepProjectilesTicksAndCollisions");
	for (auto& projectile : projectiles) {
		if (projectile->IsActive()) {
			projectile->ReceiveTick(deltaTime);
		}
	}
	//vectorShared<Collidable> vec;
	//std::span<const std::shared_ptr<Collidable>>{vec};
	//Collider::DetectCollisions(std::span{ projectiles }, std::span{ holes });
	vectorShared<Vector2Df> test;
	//std::span<const std::shared_ptr<Collidable>>{vec};
	//Collider::DetectCollisions(std::span{ test }, std::span{ holes });
	/*for (auto& projectile : projectiles) {
		for (auto& hole : holes) {
			if (projectile->IsActive() && hole->IsActive() && projectile->DetectCollision(*hole)) {
				hole->ReceiveCollision(*projectile);
				projectile->ReceiveCollision(*hole);
				break;
			}
		}
		for (auto& monster : tickableMonsters) {
			if (projectile->IsActive() && monster->IsActive() && projectile->DetectCollision(*monster)) {
				monster->ReceiveCollision(*projectile);
				projectile->ReceiveCollision(*monster);
				break;
			}
		}
	}*/
}

bool DragonJumpFramework::MenuTick()
{
	unsigned int oldTicksCount{ ticksCount };
	ticksCount = getTickCount();
	float deltaTime{ static_cast<float>(ticksCount - oldTicksCount) * 0.001f };
	SubstepPlayerTickAndCollisions(deltaTime, false);
	drawTestBackground();
	//TODO
	for (auto& drawable : platforms)
		drawable->DrawIfActive(0.f);
	for (auto& drawable : tickablePlatforms)
		drawable->DrawIfActive(0.f);
	
	if (!playerDoodle->DrawIfActive(0.f)) {
		tickFunction = &DragonJumpFramework::BeginPlay;
	}
	for (auto& drawable : menuUIs)
		drawable->DrawIfActive(0.f);
	//std::for_each(menuUIs.begin(), menuUIs.end(), [](auto& p) { p.get()->DrawIfActive(0); });
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
	unsigned int oldTicksCount{ ticksCount };
	ticksCount = getTickCount();
	float deltaTime{ static_cast<float>(ticksCount - oldTicksCount) * 0.001f };

	SubstepWorldTicks(deltaTime);
	SubstepProjectilesTicksAndCollisions(deltaTime);
	SubstepPlayerTickAndCollisions(deltaTime, true);
	
	float cameraOffset{ 0.f };
	float playerPosY{ playerDoodle->GetPosition().y };
	if (float screenhalfY{ static_cast<float>(screenSize.y) * 0.5f }; playerDoodle->IsActive() && 
		playerPosY <= screenhalfY && playerDoodle->GetCurrentVelocity().y <= 0.f) {
		cameraOffset = screenhalfY - playerPosY + 0.5f;
		distancePassed += cameraOffset;
		lastPlatformPos.y += cameraOffset;
		TryAddEntities();
	}
	drawTestBackground();
	for (auto& drawable : holes)
		drawable->DrawIfActive(cameraOffset);
	for (auto& drawable : platforms)
		drawable->DrawIfActive(cameraOffset);
	for (auto& drawable : abilities)
		drawable->DrawIfActive(cameraOffset);
	for (auto& drawable : tickablePlatforms)
		drawable->DrawIfActive(cameraOffset);
	for (auto& drawable : tickableMonsters)
		drawable->DrawIfActive(cameraOffset);
	
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
	for (auto& drawable : projectiles)
		drawable->DrawIfActive(cameraOffset);

	if (bInMenu) {
		for (auto& drawable : menuUIs)
			drawable->DrawIfActive(cameraOffset);
	}
	else {
		for (auto& drawable : gametimeUIs)
			drawable->DrawIfActive(cameraOffset);
	}
	return false;
}

Platform* DragonJumpFramework::SpawnPlatform(Vector2Df pos, PlatformType type)
{
	assert(pos.x >= 0.f && pos.x <= screenSize.x && pos.y <= screenSize.y 
		&& "attempted spawn outside screen size");
	float platformHalfSize{ screenSize.x / platformsInScreenWidth * 0.5f };
	if (pos.x < platformHalfSize) {
		pos.x = platformHalfSize;
	}
	else {
		if (pos.x > screenSize.x - platformHalfSize)
			pos.x = screenSize.x - platformHalfSize;
	}
	if (type == PlatformType::PT_Default) {
		for (auto& platform : platforms) {
			if (!platform->IsActive() && platform->Reactivate(pos)) {
				return platform.get();
			}
		}
		if (auto t{ std::make_shared<Platform>(*this,pos) }; 
			t.get()->IsActive()) {
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
			t.get()->IsActive()) {
			tickablePlatforms.push_back(t);
			return t.get();
		}
	}
	return nullptr;
}

MonsterBase* DragonJumpFramework::SpawnMonster(const Platform& target, MonsterType type)
{
	//Platform* outPlatform{ nullptr };
	Vector2Df pos{ target.GetPosition() };
	assert(pos.x >= 0.f && pos.x <= screenSize.x && pos.y <= screenSize.y && "attempted spawn outside screen size");
	target.GetStandingPointY(pos.x, pos.y);
	if (type == MonsterType::MT_Hole) {
		for (auto& monster : holes) {
			if (!monster->IsActive() && monster->Reactivate(pos))
				return monster.get();
		}
		if (auto t{ std::make_shared<Hole>(*this,pos) };
			t.get()->IsActive()) {
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
			t.get()->IsActive()) {
			tickableMonsters.push_back(t);
			return t.get();
		}
	}
	return nullptr;
}

/*Monster* DragonJumpFramework::SpawnMonster(const Platform& target, MonsterType type)
{
	return nullptr;
}*/

Projectile* DragonJumpFramework::SpawnProjectile(const Vector2Df& target)
{
	for (auto& projectile : projectiles) {
		if (!projectile->IsActive() && projectile->Reactivate(target))
			return projectile.get();
	}
	if (auto t{ std::make_shared<Projectile>(*this, target) }; t.get()->IsActive()) {
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
	if (auto t{ std::make_shared<Ability>(*this, type, pos) }; t.get()->IsActive()) {
		abilities.push_back(t);
		monstersKilledSinceAbilitySpawn -= 5;
		return t.get();
	}
	return nullptr;
}

bool DragonJumpFramework::CreateUI()
{
	if (auto t{ std::make_shared<IntDisplayer>(
		*this, Vector2Df{screenSize.x, 0}) }; t.get()->IsActive()) {
		gametimeUIs.push_back(t);
	}
	if (auto t{ std::make_shared<LifesDisplayer>(
		*this, Vector2Df{0.f}) }; t.get()->IsActive()) {
		gametimeUIs.push_back(t);
	}
	if (!bWithMenu)
		return true;
	if (auto t{ std::make_shared<Button>(
		*this, Vector2Df{ screenSize.x * 0.6f, screenSize.y * 0.5f}, 
		SpritePaths::startButton) }; t.get()->IsActive()) {
		menuUIs.push_back(t);
		t->onClicked = std::bind(&DragonJumpFramework::OnStartButtonClicked, this);
		return true;
	}
	else {
		return false;
	}
	return true;
}

