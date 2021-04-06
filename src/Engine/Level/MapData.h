#ifndef MapData_h__
#define MapData_h__

struct MapData
{
	float height;
	unsigned char flags;
};

enum MapDataFlags : unsigned char
{
	SOLID = 0x01,
	PENETRABLE = 0x02,
	OUT_OF_MAP = 0x04,
	UNUSED = 0xF8
};

#endif // !MapData_h__