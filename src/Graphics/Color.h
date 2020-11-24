#pragma once
typedef unsigned char BYTE;

class Color
{
public:
	Color();
	Color(unsigned int color);
	Color(BYTE r, BYTE g, BYTE b);
	Color(BYTE r, BYTE g, BYTE b, BYTE a);
	Color(const Color& src);

	Color& operator=(const Color& src);
	bool operator==(const Color& rhs) const;
	bool operator!=(const Color& rhs) const;

	constexpr BYTE GetRed() const;
	void SetRed(BYTE r);

	constexpr BYTE GetGreen() const;
	void SetGreen(BYTE g);

	constexpr BYTE GetBlue() const;
	void SetBlue(BYTE b);

	constexpr BYTE GetAlpha() const;
	void SetAlpha(BYTE a);
private:
	union
	{
		BYTE rgba[4];
		unsigned int color;
	};
};