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

class BinaryWriter 
{
private:
	std::vector<char> buffer;
	size_t totalSize;

	void put(int byte)
	{
#ifdef _DEBUG
		size_t pos = target.tellp();
#endif

		char b = (char)byte;
		target.write(&b, 1);

#ifdef _DEBUG
		// this may happen when the underlying stream object is not in binary mode.
		if((size_t)target.tellp() != pos + 1)
			assert(false);
#endif
	}

public:
	std::ostream& target;

	BinaryWriter(std::ostream& target) : target(target) 
	{
	}

	void WriteInt64(int64_t value)
	{
		put((value >> 0) & 0xFF);
		put((value >> 8) & 0xFF);
		put((value >> 16) & 0xFF);
		put((value >> 24) & 0xFF);
		put((value >> 32) & 0xFF);
		put((value >> 40) & 0xFF);
		put((value >> 48) & 0xFF);
		put((value >> 56) & 0xFF);
	}

	void WriteInt32(int32_t value)
	{
		put((value >> 0) & 0xFF);
		put((value >> 8) & 0xFF);
		put((value >> 16) & 0xFF);
		put((value >> 24) & 0xFF);
	}

	void WriteColor(Pixel value)
	{
		WriteSingle(value.a);
		WriteSingle(value.r);
		WriteSingle(value.g);
		WriteSingle(value.b);
	}

	void WriteVector3(Vector3 value)
	{
		WriteSingle(value.x);
		WriteSingle(value.y);
		WriteSingle(value.z);
	}

	void WriteQuaternion(Quaternion value)
	{
		WriteSingle(value.i);
		WriteSingle(value.j);
		WriteSingle(value.k);
		WriteSingle(value.r);
	}

	void WriteMatrix(Matrix4x4 value)
	{
		for (int col = 0; col < 4; col++)
        {
            for (int row = 0; row < 4; row++)
            {
				WriteSingle(value[col][row]);
			}
		}
	}

	void WriteSingle(float value)
	{
		WriteInt32(*((int32_t*)&value));
	}

	void WriteBoolean(bool value)
	{
		put(value ? 1 : 0);
	}

	void WriteString(const std::string& value)
	{
		WriteInt32(value.size());
		target.write(value.data(), value.size());
	}

	void WriteBytes(const std::vector<unsigned char>& value)
	{
		target.write((const char*)value.data(), value.size());
	}
};
