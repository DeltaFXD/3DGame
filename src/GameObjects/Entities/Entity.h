#pragma once
#include <DirectXMath.h>
#include "GameObjects/GameObject.h"

using namespace DirectX;

class Level;

class Entity
{
public:
	void Initialize(Level* level);
	const XMFLOAT3 GetPosition();
	float GetX();
	float GetY();
	float GetZ();
	bool IsRemoved();

	virtual void Update() = 0;
	virtual void Render() = 0;
protected:
	Level* level = nullptr;
	XMFLOAT3 position;
	bool removed = false;

	void Remove();
};