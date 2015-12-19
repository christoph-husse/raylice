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

#include <assert.h>

struct WeightedPixel;

inline float checkExcept(float v)
{
	assert(!(v != v));
	return v;
}

struct Pixel
{
	float a, r, g, b;

	Pixel() : a(1), r(0), g(0), b(0) {}

	Pixel(float r, float g, float b) : a(1), r(checkExcept(r)), g(checkExcept(g)), b(checkExcept(b)) {}
	Pixel(float r, float g, float b, float a) : a(checkExcept(a)), r(checkExcept(r)), g(checkExcept(g)), b(checkExcept(b)) {}

	explicit Pixel(const WeightedPixel& src);

	float GetLuminance() const { return ( 0.2126 * r ) + ( 0.7152 * g ) + ( 0.0722 * b ); }

	void SetRGB(int r, int g, int b)
	{
		SetRGBA(r, g, b, 255);
	}

	void SetRGBA(int r, int g, int b, int a)
	{
		this->a = a / 255.0f;
		this->r = r / 255.0f;
		this->g = g / 255.0f;
		this->b = b / 255.0f;
	}

	Pixel& operator +=(const Pixel& other)
	{
		checkExcept(a += other.a);
		checkExcept(r += other.r);
		checkExcept(g += other.g);
		checkExcept(b += other.b);
		return *this;
	}

	Pixel& operator *=(float scalar)
	{
		checkExcept(a *= scalar);
		checkExcept(r *= scalar);
		checkExcept(g *= scalar);
		checkExcept(b *= scalar);
		return *this;
	}

	Pixel& operator *=(const Pixel& pixel)
	{
		checkExcept(a *= pixel.a);
		checkExcept(r *= pixel.r);
		checkExcept(g *= pixel.g);
		checkExcept(b *= pixel.b);
		return *this;
	}

	Pixel& operator /=(float scalar)
	{
		checkExcept(a /= scalar);
		checkExcept(r /= scalar);
		checkExcept(g /= scalar);
		checkExcept(b /= scalar);
		return *this;
	}

	bool IsBlack() const
	{
		return (r < 5/255.f) && (g < 5/255.f) && (b < 5/255.f);
	}
};

inline Pixel operator *(float scalar, const Pixel& p)
{
	return Pixel(p.r * scalar, p.g * scalar, p.b * scalar, p.a * scalar);
} 

inline Pixel operator *(const Pixel& p, float scalar)
{
	return Pixel(p.r * scalar, p.g * scalar, p.b * scalar, p.a * scalar);
} 

inline Pixel operator +(const Pixel& a, const Pixel& b)
{
	return Pixel(a.r + b.r, a.g + b.g, a.b + b.b, a.a + b.a);
} 

inline Pixel operator -(const Pixel& a, const Pixel& b)
{
	return Pixel(a.r - b.r, a.g - b.g, a.b - b.b, a.a - b.a);
} 

inline Pixel operator /(const Pixel& p, float scalar)
{
	return Pixel(p.r / scalar, p.g / scalar, p.b / scalar, p.a / scalar);
} 

inline Pixel operator *(const Pixel& a, const Pixel& b)
{
	Pixel res = a;
	return res *= b;
}

struct Texel
{
	unsigned char a, r, g, b;
	Texel(const Pixel& pixel) : a(pixel.a * 255.0f), r(pixel.r * 255.0f), g(pixel.g * 255.0f), b(pixel.b * 255.0f) { }
	Texel() : a(0), r(0), g(0), b(0) { }
	Texel(unsigned char r, unsigned char g, unsigned char b, unsigned char a) : a(a), r(r), g(g), b(b) { }
	operator Pixel() 
	{
		return Pixel(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
	}
};

struct WeightedPixel
{
	float weight;
	Pixel color;

	WeightedPixel() : weight(0), color(0,0,0) {} 
	WeightedPixel(float weight) : weight(checkExcept(weight)), color(0,0,0) {} 
	WeightedPixel(float weight, Pixel color) : weight(checkExcept(weight)), color(color) {} 

	WeightedPixel& operator +=(const WeightedPixel& other)
	{
		color += other.color;
		checkExcept(weight += other.weight);
		return *this;
	}
};

inline WeightedPixel operator +(const WeightedPixel& a, const WeightedPixel& b)
{
	return WeightedPixel(a.weight + b.weight, a.color + b.color);
}
