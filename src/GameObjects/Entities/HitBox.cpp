#include "HitBox.h"

HitBox::HitBox(float width, float height, float xOffset, float yOffset) 
{
	this->width = width + xOffset;
	this->height = height + yOffset;
	x = xOffset;
	y = yOffset;
	centerX = (this->width + x) / 2.0f;
	centerY = (this->height + y) / 2.0f;
}

bool HitBox::IsInside(float x, float y)
{
	if (x < this->x && y < this->y)
	{
		return false;
	}
	else if (x > width && y > height)
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool HitBox::IsInside(float x, float y, HitBox hitbox)
{
	float xt = x + hitbox.x;
	float yt = y + hitbox.y;
	float wt = x + hitbox.width;
	float ht = y + hitbox.height;

	if (((this->x < xt && xt < width) || (this->x < wt && wt < width)) && ((this->y < yt && yt < height) || (this->y < ht && ht < height)))
	{
		return true;
	}
	else if (((xt < this->x && x < wt) || (xt < width && width < wt)) && ((yt < this->y && this->y < ht) || (yt < height && height < ht)))
	{
		return true;
	}
	else
	{
		return false;
	}
}