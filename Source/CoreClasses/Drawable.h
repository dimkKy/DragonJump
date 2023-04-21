// by Dmitry Kolontay

#pragma once

#include "DragonJumpFramework.h"

//maybe should have gone with singleton after all
class Drawable
{
public:
	[[maybe_unused]] virtual bool DrawIfActive(float cameraVerticalOffset) { 
		position.y += cameraVerticalOffset; 
		return IsActive() ? DrawIfActive_Internal() : false; 
	};

	[[nodiscard]] virtual bool IsActive() { return bIsActive; };
	virtual Vector2Df GetPosition() const { return position; };

	virtual void SetPosition(const Vector2Df& p)
		{ position = p; }

	[[nodiscard]] virtual bool Reactivate(const Vector2Df& v) = 0;

	virtual void Deactivate() 
		{ bIsActive = false; };

	template<class TDrawable>
		requires std::is_base_of<Drawable, TDrawable>::value
	static void DispatchDrawcalls(float cameraVerticalOffset, std::vector<std::shared_ptr<TDrawable>>& vec) {
		for (auto& drawable : vec) {
			drawable->DrawIfActive(cameraVerticalOffset);
		}
	}
	template<typename FDrawable, typename...SDrawable>
		requires std::is_base_of<Drawable, FDrawable>::value && 
		((std::is_base_of<Drawable, SDrawable>::value) &&...)
	static void DispatchDrawcalls(float cameraVerticalOffset, std::vector<std::shared_ptr<FDrawable>>& vec, 
		std::vector<std::shared_ptr<SDrawable>>&...vecs) {
		for (auto& drawable : vec) {
			drawable->DrawIfActive(cameraVerticalOffset);
		}
		DispatchDrawcalls(cameraVerticalOffset,vecs...);
	}

protected:
	template<class...Args>
	Drawable(DragonJumpFramework& _framework, const Args&...args) : 
		framework{ _framework }, bIsActive{ true }, position{ args... } {};
	virtual bool DrawIfActive_Internal() = 0;
	DragonJumpFramework& framework;
	bool bIsActive;
	Vector2Df position;

	
};

