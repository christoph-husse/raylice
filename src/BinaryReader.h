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


class BinaryReader 
{
private:
	std::vector<char> buffer;
	size_t totalSize;

	int get()
	{
#ifdef _DEBUG
		size_t pos = source.tellg();
#endif
		char b;
		source.read(&b, 1);

#ifdef _DEBUG
		// this may happen when the underlying stream object is not in binary mode.
		assert((size_t)source.tellg() == pos + 1);
#endif

		return (unsigned char)b;
	}

public:
	std::istream& source;

	BinaryReader(std::istream& source) : source(source) 
	{
		auto pos = source.tellg();
		source.seekg(0, std::ios_base::end);
		totalSize = source.tellg();
		source.seekg(pos, std::ios_base::beg);
	}

	void ReadSingleArray(std::vector<float>& buffer, int elementsToRead)
	{
		if(elementsToRead < buffer.size())
			throw std::overflow_error("Buffer not large enough to hold all requested data!");

		static_assert(sizeof(float) == 4, "Float needs to be 4 bytes for this operation to succeed.");

		source.read((char*)&buffer[0], elementsToRead * 4);
	}

	int64_t ReadInt64()
	{
		auto a = (((int64_t)get()) << 0);
		auto b = (((int64_t)get()) << 8);
		auto c = (((int64_t)get()) << 16);
		auto d = (((int64_t)get()) << 24);
		auto e = (((int64_t)get()) << 32);
		auto f = (((int64_t)get()) << 40);
		auto g = (((int64_t)get()) << 48);
		auto h = (((int64_t)get()) << 56);
		return a | b | c | d | e | f | g | h;
	}

	int32_t ReadInt32()
	{
		auto a = (((int32_t)get()) << 0);
		auto b = (((int32_t)get()) << 8);
		auto c = (((int32_t)get()) << 16);
		auto d = (((int32_t)get()) << 24);
		return a | b | c | d;
	}

	Pixel ReadColor()
	{
		Pixel res;
		res.a = ReadSingle();
		res.r = ReadSingle();
		res.g = ReadSingle();
		res.b = ReadSingle();
		return res;
	}

	Vector3 ReadVector3()
	{
		Vector3 res;
		res.x = ReadSingle();
		res.y = ReadSingle();
		res.z = ReadSingle();
		return res;
	}

	Quaternion ReadQuaternion()
	{
		Quaternion res;
		res.i = ReadSingle();
		res.j = ReadSingle();
		res.k = ReadSingle();
		res.r = ReadSingle();
		return res;
	}

	Matrix4x4 ReadMatrix()
	{
		Matrix4x4 res;
		for (int col = 0; col < 4; col++)
        {
            for (int row = 0; row < 4; row++)
            {
				res[col][row] = ReadSingle();
			}
		}
		return res;
	}

	float ReadSingle()
	{
		int32_t bits = ReadInt32();
		return *((float*)&bits);
	}

	bool ReadBoolean()
	{
		return get() != 0;
	}

	std::string ReadString()
	{
		auto len = ReadInt32();
		buffer.resize(len);
		if(source.read(buffer.data(), len).fail())
			throw std::invalid_argument("Unexpected end of stream!");

		return std::string(buffer.data(), len);
	}

	void ReadBytes(size_t count, std::vector<unsigned char>& buffer)
	{
		// safety switch to prevent allocation of unresonable amount of memory...
		if(totalSize - source.tellg() < count)
			throw std::invalid_argument("End of stream reached during read operation.");

		buffer.resize(count);
		source.read((char*)buffer.data(), count);
	}
};
