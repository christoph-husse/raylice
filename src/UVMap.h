// ======================================================================== //
// Copyright 2013 Christoph Husse                                           //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

#include "Math.h"
#include "PathSegment.h"

#include <vector>
#include <cmath>
#include <boost/noncopyable.hpp>

struct Rect
{
	int left, top, width, height;

	Rect(int left, int top, int width, int height) : left(left), top(top), width(width), height(height) {}
	Rect() : left(0), top(0), width(0), height(0) {}
};


template<class TUVEntry>
class UVMap : protected boost::noncopyable
{
private:
	int height;
	int heightShift;
	int heightMask;
	int width;
	int widthShift;
	int widthMask;

protected:
	std::vector<TUVEntry> entries;

public:
	typedef TUVEntry texel_type;

	int GetWidth() const { return width; }
	int GetHeight() const { return height; }

	UVMap()
	{
		Initialize(1, 1);
	}

	UVMap(int width, int height)
	{
		Initialize(width, height);
	}
	
	void Initialize(int width, int height)
	{
		this->width = width;
		this->widthShift = (int)(std::log(width) / std::log(2));
		this->widthMask = width - 1;

		this->height = height;
		this->heightShift = (int)(std::log(height) / std::log(2));
		this->heightMask = height - 1;

		if((width < 1) || ((1 << widthShift) != width) || (height < 1) || ((1 << heightShift) != height))
			throw std::invalid_argument("Must be power of two!");

		entries = std::vector<TUVEntry>(width * height);
	}

	TUVEntry& operator()(const PathSegment& segment)
	{
		return operator()(*segment.GetTriangleAtImpact(), segment.GetImpact());
	}

	TUVEntry& operator()(const Triangle& triangle, Vector3 hitPoint)
	{
		Vector3 uv = triangle.WorldToUv(hitPoint);

		return operator()(uv.x, uv.y);
	}

	TUVEntry& operator()(int x, int y)
	{
		int i = (x & widthMask) + ((y & heightMask) << widthShift);

#ifndef _DEBUG
		return entries[i];
#else
		return entries.at(i);
#endif
	}

	typename std::vector<TUVEntry>::iterator begin() { return entries.begin(); }
	typename std::vector<TUVEntry>::iterator end() { return entries.end(); }

	TUVEntry& operator()(float x, float y)
	{
		return this->operator()((int)(x * width), (int)(y * height));
	}
};

template<class TUVEntry>
class UVMapNPOT : protected boost::noncopyable
{
private:
	int height;
	int width;

protected:
	std::vector<TUVEntry> entries;

public:
	typedef TUVEntry texel_type;

	int GetWidth() const { return width; }
	int GetHeight() const { return height; }

	UVMapNPOT()
	{
		Initialize(1, 1);
	}

	UVMapNPOT(int width, int height)
	{
		Initialize(width, height);
	}
	
	void Initialize(int width, int height)
	{
		this->width = width;
		this->height = height;

		if((width < 1) || (height < 1))
			throw std::invalid_argument("Must be at least 1x1!");

		entries = std::vector<TUVEntry>(width * height);
	}

	Vector3 MakeUV(const PathSegment& segment) const { return MakeUV(*segment.GetTriangleAtImpact(), segment.GetImpact()); }
	Vector3 MakeUV(const Triangle& triangle, Vector3 hitPoint) const { return triangle.WorldToUv(hitPoint); }
	Vector3 UVToOffset(Vector3 uv) const
	{ 
		int x = ((int)Math::Sign(uv.x)) * (((int)std::abs(uv.x * width)) % width);
		int y = ((int)Math::Sign(uv.y)) * (((int)std::abs(uv.y * height)) % height);

		if(x < 0) x = width + x;
		if(y < 0) y = height + y;

		return Vector3(x, y, 0);
	}


	TUVEntry& operator()(const PathSegment& segment)
	{
		Vector3 uv = MakeUV(segment);
		return operator()(uv.x, uv.y);
	}

	TUVEntry& operator()(const Triangle& triangle, Vector3 hitPoint)
	{
		Vector3 uv = MakeUV(triangle, hitPoint);
		return operator()(uv);
	}

	Vector3 OffsetToUV(int x, int y) const
	{ 
		return Vector3(x / (float)width, y / (float)height, 0);
	}

	TUVEntry& operator()(int x, int y)
	{
		x = ((int)Math::Sign(x)) * (((int)std::abs(x)) % width);
		y = ((int)Math::Sign(y)) * (((int)std::abs(y)) % height);

		if(x < 0) x = width + x;
		if(y < 0) y = height + y;

		int i = (int)(x + y * width);

#ifndef _DEBUG
		return entries[i];
#else
		return entries.at(i);
#endif
	}

	TUVEntry& operator()(Vector3 uv)
	{
		Vector3 offset = UVToOffset(uv);
		return this->operator()((int)offset.x, (int)offset.y);
	}

	TUVEntry operator()(const PathSegment& segment) const
	{
		return const_cast<UVMapNPOT<TUVEntry>*>(this)->operator()(*segment.GetTriangleAtImpact(), segment.GetImpact());
	}

	TUVEntry operator()(const Triangle& triangle, Vector3 hitPoint) const
	{
		return const_cast<UVMapNPOT<TUVEntry>*>(this)->operator()(triangle, hitPoint);
	}

	TUVEntry operator()(int x, int y) const
	{
		return const_cast<UVMapNPOT<TUVEntry>*>(this)->operator()(x, y);
	}

	TUVEntry operator()(Vector3 uv) const
	{
		return const_cast<UVMapNPOT<TUVEntry>*>(this)->operator()(uv);
	}

	typename std::vector<TUVEntry>::iterator begin() { return entries.begin(); }
	typename std::vector<TUVEntry>::iterator end() { return entries.end(); }
};

template<class TUVMap>
void TraverseImage(TUVMap& map, std::function<void (int x, int y)> callback) 
{
	for(int x = 0, i = 0; x < map.GetWidth(); x++)
	{
		for(int y = 0; y < map.GetHeight(); y++)
		{
			callback(x, y);
		} 
	}
}
