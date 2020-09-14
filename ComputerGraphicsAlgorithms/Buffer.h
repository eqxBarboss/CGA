#pragma once

#include "framework.h"

namespace cga
{

class Buffer
{
public:
	COLORREF* data;

	Buffer(int aWidth, int aHeight, COLORREF initialColor)
		: width(aWidth),
		height(aHeight)
	{
		totalPixels = width * height;
		data = static_cast<COLORREF*>(calloc(totalPixels, sizeof(COLORREF)));
	}

	~Buffer()
	{
		free(data);
	}

	inline int GetWidth()
	{
		return width;
	}

	inline int GetHeight()
	{
		return height;
	}

	// TODO: this is invalid due to bytes order, fix
	inline void ClearWithColor(COLORREF color)
	{
		memset(data, color, totalPixels * sizeof(COLORREF));
	}

	inline void SetPixel(int x, int y, COLORREF color)
	{
		data[y * width + x] = color;
	}

private:
	int width, height;
	int totalPixels;
};

}
