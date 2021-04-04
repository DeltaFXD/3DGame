#include "Entity.h"

void Entity::Initialize(Level* level)
{
	this->level = level;
}

const XMFLOAT3 Entity::GetPosition()
{
	return position;
}

float Entity::GetX()
{
	return position.x;
}

float Entity::GetY()
{
	return position.y;
}

float Entity::GetZ()
{
	return position.z;
}

bool Entity::IsRemoved()
{
	return removed;
}

void Entity::Remove()
{
	removed = true;
}
