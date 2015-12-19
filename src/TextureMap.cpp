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


#include "stdafx.h"
#include "OpenGLWindow.h"

#include <ImfRgbaFile.h>
#include <ImfArray.h>


TextureMap::TextureMap(int width, int height, int channelCount) : UVMapNPOT<Texel>(width, height), channelCount(channelCount)
{
	if((channelCount != 1) && (channelCount != 3) && (channelCount != 4))
		throw std::invalid_argument("A texture may have either 1, 3 or 4 channels.");
}

void TextureMap::SetBytesRGBA(const std::vector<unsigned char>& bytes)
{
	if(GetWidth() * GetHeight() * channelCount != bytes.size())
		throw std::invalid_argument("Raw texture bytes must match the texture's internal buffer exactly in size.");

	for(int i = 0, j = 0, count = GetWidth() * GetHeight(); i < count; i++)
	{
		int r = 0, g = 0, b = 0, a = 255;

		if(channelCount == 3)
		{
			r = bytes[j++];
			g = bytes[j++];
			b = bytes[j++];
		}
		else if(channelCount == 4)
		{
			r = bytes[j++];
			g = bytes[j++];
			b = bytes[j++];
			a = bytes[j++];
		}
		else  if(channelCount == 1)
		{
			a = bytes[j++];
		}

		entries[i] = Texel(r, g, b, a);
	}
}

void RenderBuffer::SaveToEXR(std::string fileName)
{
	Imf::Array2D<Imf::Rgba> pixels(GetHeight(), GetWidth());

	for (int y = 0; y < GetHeight(); y++) 
	{
		for (int x = 0; x < GetWidth(); x++) 
		{
			const Pixel p = this->operator()(x, y);

			pixels[y][x] = Imf::Rgba(p.r, p.g, p.b);
		}
	}

	Imf::RgbaOutputFile file(fileName.c_str(), GetWidth(), GetHeight(), Imf::WRITE_RGBA);
	file.setFrameBuffer(&pixels[0][0], 1, GetWidth());
	file.writePixels(GetHeight());
}

void RenderBuffer::BlitToOpenGL(Rect rect) const
{
	std::vector<unsigned char> pixels(rect.width * rect.height * 3);
	int i = 0;

	for(int y = rect.top; y < rect.top + rect.height; y++)
	{
		for(int x = rect.left; x < rect.left + rect.width; x++)
		{
			const Pixel p = this->operator()(x, y);
			pixels[i++] = Math::Clamp(p.r, 0.0f, 1.0f) * 255;
			pixels[i++] = Math::Clamp(p.g, 0.0f, 1.0f) * 255;
			pixels[i++] = Math::Clamp(p.b, 0.0f, 1.0f) * 255;
		} 
	}

	glTexSubImage2D(GL_TEXTURE_2D, 0, rect.left, rect.top, rect.width, rect.height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
}

float TextureMap::ExtractAlpha(const Texel& pixel) const
{
	if(channelCount == 3)
		return pixel.r / 255.0f;
	else
		return pixel.a / 255.0f;
}

float TextureMap::ExtractHeight(const Texel& pixel) const
{
	if(channelCount == 1)
		return pixel.a / 255.0f;
	else
		return pixel.r / 255.0f;
}

Pixel TextureMap::Interpolated(const Triangle& triangle, Vector3 w) const
{
	const TextureMap& tex = *this;
	Vector3 uv = MakeUV(triangle, w);

	// bilinear filtering (Wikipedia)
	float u = uv.x * GetWidth() - 0.5;
	float v = uv.y * GetHeight() - 0.5;
	int x = std::floor(u);
	int y = std::floor(v);
	double u_ratio = u - x;
	double v_ratio = v - y;
	double u_opposite = 1 - u_ratio;
	double v_opposite = 1 - v_ratio;
	return ((Pixel)tex(x, y) * u_opposite + (Pixel)tex(x+1, y) * u_ratio) * v_opposite + 
				((Pixel)tex(x,y+1) * u_opposite + (Pixel)tex(x+1,y+1) * u_ratio) * v_ratio;
}
