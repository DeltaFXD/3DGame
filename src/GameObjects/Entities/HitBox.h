#pragma once

class HitBox
{
public:
	HitBox(float width, float height, float xOffset, float yOffset);
	bool IsInside(float x, float y);
	bool IsInside(float x, float y, HitBox hitbox);
private:
	float width;
	float height;
	float x;
	float y;
	float centerX;
	float centerY;
};