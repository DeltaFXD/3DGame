#include "Color.h"

Color::Color() : color(0)
{

}

Color::Color(unsigned int color) : color(color)
{

}

Color::Color(BYTE r, BYTE g, BYTE b) : Color(r, g, b, 0)
{

}

Color::Color(BYTE r, BYTE g, BYTE b, BYTE a)
{
	rgba[0] = r;
	rgba[1] = g;
	rgba[2] = b;
	rgba[3] = a;
}

Color::Color(const Color& src) : color(src.color)
{
	
}

Color& Color::operator=(const Color& src)
{
	color = src.color;
	return *this;
}

bool Color::operator==(const Color& rhs) const
{
	return (color == rhs.color);
}

bool Color::operator!=(const Color& rhs) const
{
	return !(*this == rhs);
}

constexpr BYTE Color::GetRed() const
{
	return rgba[0];
}

void Color::SetRed(BYTE r)
{
	rgba[0] = r;
}

constexpr BYTE Color::GetGreen() const
{
	return rgba[1];
}

void Color::SetGreen(BYTE g)
{
	rgba[1] = g;
}

constexpr BYTE Color::GetBlue() const
{
	return rgba[2];
}

void Color::SetBlue(BYTE b)
{
	rgba[2] = g;
}

constexpr BYTE Color::GetAlpha() const
{
	return rgba[3];
}

void Color::SetAlpha(BYTE a)
{
	rgba[3] = a;
}