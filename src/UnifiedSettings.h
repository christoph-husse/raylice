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


enum class EPropertyType
{
	Integer,
	Float,
	Texture2D,
	String,
	Boolean,
	Vector3,
	Pixel,
	Matrix,
};

enum class EUnifiedFormatFlag
{
	None = 0,
	Switch = 1,
};

class UnifiedProperty
{
protected:
	bool hasValue;

private:
	friend class UnifiedSettings;

	EPropertyType type;
	std::string title;
	std::string description;

	static void Skip(BinaryReader& reader);

protected:
	virtual void DeserializeFormatInternal(BinaryReader& reader) = 0;

public:
	virtual ~UnifiedProperty() { } 
	UnifiedProperty(EPropertyType type, std::string title, std::string description = std::string()) : hasValue(false), type(type), title(title), description(description) { } 

	EPropertyType GetType() const { return type; }
	std::string GetTitle() const { return title; }
	std::string GetDescription() const { return description; }

	virtual std::string GetString() const { throw std::bad_cast("This property is not a string!"); }
	virtual void SetString(std::string value) { throw std::bad_cast("This property is not a string!"); }
		
	virtual int64_t GetInteger() const { throw std::bad_cast("This property is not an integer!"); }
	virtual void SetInteger(int64_t value) { throw std::bad_cast("This property is not an integer!"); }

	virtual float GetSingle() const { throw std::bad_cast("This property is not a float!"); }
	virtual void SetSingle(float value) { throw std::bad_cast("This property is not a float!"); }

	virtual bool GetBoolean() const { throw std::bad_cast("This property is not a bool!"); }
	virtual void SetBoolean(bool value) { throw std::bad_cast("This property is not a bool!"); }

	virtual Pixel GetColor() const { throw std::bad_cast("This property is not a color!"); }
	virtual void SetColor(Pixel value) { throw std::bad_cast("This property is not a color!"); }

	virtual Vector3 GetVector3() const { throw std::bad_cast("This property is not a vector!"); }
	virtual void SetVector3(Vector3 value) { throw std::bad_cast("This property is not a vector!"); }

	virtual Matrix4x4 GetMatrix() const { throw std::bad_cast("This property is not a matrix!"); }
	virtual void SetMatrix(Matrix4x4 value) { throw std::bad_cast("This property is not a matrix!"); }

	virtual std::shared_ptr<TextureMap> GetTexture2D() const { throw std::bad_cast("This property is not a 2D texture!"); }

	virtual void SerializeFormat(BinaryWriter& writer) const
	{
		writer.WriteInt32((int)type);
		writer.WriteString(title);
		writer.WriteString(description);
	}

	static std::shared_ptr<UnifiedProperty> DeserializeFormat(BinaryReader& reader);

	virtual void SerializeData(BinaryWriter& writer) const
	{
		writer.WriteInt32((int)type);
		writer.WriteBoolean(hasValue);
	}

	virtual void DeserializeData(BinaryReader& reader)
	{
		if((EPropertyType)reader.ReadInt32() != type)
			throw std::invalid_argument("Deserialized property does not match type!");

		hasValue = reader.ReadBoolean();
	}

	virtual void ClearData()
	{
		hasValue = false;
	}
};

class UnifiedIntegerProperty : public UnifiedProperty
{
private:
	int64_t integerMin;
	int64_t integerMax;
	int64_t integerDefault;
	int64_t integer;

protected:
	virtual void DeserializeFormatInternal(BinaryReader& reader) override
	{
		integerDefault = reader.ReadInt64();
		integerMin = reader.ReadInt64();
		integerMax = reader.ReadInt64();
	}

public:

	UnifiedIntegerProperty(
			std::string title = std::string(), 
			int64_t default = 0, 
			int64_t min = std::numeric_limits<int64_t>::min(),
			int64_t max = std::numeric_limits<int64_t>::max(),
			std::string description = std::string()) 
		: 
		UnifiedProperty(EPropertyType::Integer, title, description), 
		integerMin(min),
		integerMax(std::max(min, max)),
		integerDefault(Math::Clamp(default, integerMin, integerMax)), 
		integer(integerDefault)
	{
	}

	virtual int64_t GetInteger() const override
	{
		return hasValue ? integer : integerDefault;
	}

	virtual void SetInteger(int64_t value) override
	{
		hasValue = true;
		integer = Math::Clamp(value, integerMin, integerMax);
	}

	virtual void SerializeFormat(BinaryWriter& writer) const override
	{
		UnifiedProperty::SerializeFormat(writer);

		writer.WriteInt64(integerDefault);
		writer.WriteInt64(integerMin);
		writer.WriteInt64(integerMax);
	}

	virtual void SerializeData(BinaryWriter& writer) const override
	{
		UnifiedProperty::SerializeData(writer);
		writer.WriteInt64(integer);
	}

	virtual void DeserializeData(BinaryReader& reader) override
	{
		UnifiedProperty::DeserializeData(reader);
		integer = reader.ReadInt64();
	}
};

class UnifiedFloatProperty : public UnifiedProperty
{
private:
	float singleMin;
	float singleMax;
	float singleDefault;
	float single;

protected:
	virtual void DeserializeFormatInternal(BinaryReader& reader) override
	{
		singleDefault = reader.ReadSingle();
		singleMin = reader.ReadSingle();
		singleMax = reader.ReadSingle();
	}

public:

	UnifiedFloatProperty(
			std::string title = std::string(), 
			float default = 0, 
			float min = -std::numeric_limits<float>::infinity(),
			float max = std::numeric_limits<float>::infinity(), 
			std::string description = std::string()) 
		: 
		UnifiedProperty(EPropertyType::Float, title, description), 
		singleMin(min),
		singleMax(std::max(min, max)),
		singleDefault(Math::Clamp(default, singleMin, singleMax)), 
		single(singleDefault)
	{
	}

	virtual float GetSingle() const override
	{
		return hasValue ? single : singleDefault;
	}

	virtual void SetSingle(float value) override
	{
		hasValue = true;
		single = Math::Clamp(value, singleMin, singleMax);
	}

	virtual void SerializeFormat(BinaryWriter& writer) const override
	{
		UnifiedProperty::SerializeFormat(writer);

		writer.WriteSingle(singleDefault);
		writer.WriteSingle(singleMin);
		writer.WriteSingle(singleMax);
	}

	virtual void SerializeData(BinaryWriter& writer) const override
	{
		UnifiedProperty::SerializeData(writer);
		writer.WriteSingle(single);
	}

	virtual void DeserializeData(BinaryReader& reader) override
	{
		UnifiedProperty::DeserializeData(reader);
		single = reader.ReadSingle();
	}
};

class UnifiedStringProperty : public UnifiedProperty
{
private:
	std::string string;
	std::string stringDefault;

protected:
	virtual void DeserializeFormatInternal(BinaryReader& reader) override
	{
		stringDefault = reader.ReadString();
	}

public:

	UnifiedStringProperty(std::string title = std::string(), std::string stringDefault = std::string(), std::string description = std::string()) : 
		UnifiedProperty(EPropertyType::String, title, description), 
		stringDefault(stringDefault), 
		string(stringDefault)
	{
	}

	virtual std::string GetString() const override
	{
		return hasValue ? string : stringDefault;
	}

	virtual void SetString(std::string value) override
	{
		hasValue = true;
		string = value;
	}

	virtual void SerializeFormat(BinaryWriter& writer) const override
	{
		UnifiedProperty::SerializeFormat(writer);
		writer.WriteString(stringDefault);
	}

	virtual void SerializeData(BinaryWriter& writer) const override
	{
		UnifiedProperty::SerializeData(writer);
		writer.WriteString(string);
	}

	virtual void DeserializeData(BinaryReader& reader) override
	{
		UnifiedProperty::DeserializeData(reader);
		string = reader.ReadString();
	}
};

class UnifiedBooleanProperty : public UnifiedProperty
{
private:
	bool boolean;
	bool booleanDefault;

protected:
	virtual void DeserializeFormatInternal(BinaryReader& reader) override
	{
		booleanDefault = reader.ReadBoolean();
	}

public:

	UnifiedBooleanProperty(std::string title = std::string(), bool default = false, std::string description = std::string()) : 
		UnifiedProperty(EPropertyType::Boolean, title, description), 
		booleanDefault(default), 
		boolean(booleanDefault)
	{
	}

	virtual bool GetBoolean() const override
	{
		return hasValue ? boolean : booleanDefault;
	}

	virtual void SetBoolean(bool value) override
	{
		hasValue = true;
		boolean = value;
	}

	virtual void SerializeFormat(BinaryWriter& writer) const override
	{
		UnifiedProperty::SerializeFormat(writer);
		writer.WriteBoolean(booleanDefault);
	}

	virtual void SerializeData(BinaryWriter& writer) const override
	{
		UnifiedProperty::SerializeData(writer);
		writer.WriteBoolean(boolean);
	}

	virtual void DeserializeData(BinaryReader& reader) override
	{
		UnifiedProperty::DeserializeData(reader);
		boolean = reader.ReadBoolean();
	}
};

class UnifiedVector3Property : public UnifiedProperty
{
private:
	Vector3 value;
	Vector3 valueDefault;

protected:
	virtual void DeserializeFormatInternal(BinaryReader& reader) override
	{
		valueDefault = reader.ReadVector3();
	}

public:

	UnifiedVector3Property(std::string title = std::string(), Vector3 default = Vector3(0,0,0), std::string description = std::string()) : 
		UnifiedProperty(EPropertyType::Vector3, title, description), 
		valueDefault(default), 
		value(valueDefault)
	{
	}

	virtual Vector3 GetVector3() const override
	{
		return hasValue ? value : valueDefault;
	}

	virtual void SetVector3(Vector3 value) override
	{
		hasValue = true;
		value = value;
	}

	virtual void SerializeFormat(BinaryWriter& writer) const override
	{
		UnifiedProperty::SerializeFormat(writer);
		writer.WriteVector3(valueDefault);
	}

	virtual void SerializeData(BinaryWriter& writer) const override
	{
		UnifiedProperty::SerializeData(writer);
		writer.WriteVector3(value);
	}

	virtual void DeserializeData(BinaryReader& reader) override
	{
		UnifiedProperty::DeserializeData(reader);
		value = reader.ReadVector3();
	}
};

class UnifiedPixelProperty : public UnifiedProperty
{
private:
	Pixel value;
	Pixel valueDefault;

protected:
	virtual void DeserializeFormatInternal(BinaryReader& reader) override
	{
		valueDefault = reader.ReadColor();
	}

public:

	UnifiedPixelProperty(std::string title = std::string(), Pixel default = Pixel(), std::string description = std::string()) : 
		UnifiedProperty(EPropertyType::Pixel, title, description), 
		valueDefault(default), 
		value(valueDefault)
	{
	}

	virtual Pixel GetColor() const override
	{
		return hasValue ? value : valueDefault;
	}

	virtual void SetColor(Pixel value) override
	{
		hasValue = true;
		value = value;
	}

	virtual void SerializeFormat(BinaryWriter& writer) const override
	{
		UnifiedProperty::SerializeFormat(writer);
		writer.WriteColor(valueDefault);
	}

	virtual void SerializeData(BinaryWriter& writer) const override
	{
		UnifiedProperty::SerializeData(writer);
		writer.WriteColor(value);
	}

	virtual void DeserializeData(BinaryReader& reader) override
	{
		UnifiedProperty::DeserializeData(reader);
		value = reader.ReadColor();
	}
};

class UnifiedMatrixProperty : public UnifiedProperty
{
private:
	Matrix4x4 value;
	Matrix4x4 valueDefault;

protected:
	virtual void DeserializeFormatInternal(BinaryReader& reader) override
	{
		valueDefault = reader.ReadMatrix();
	}

public:

	UnifiedMatrixProperty(std::string title = std::string(), Matrix4x4 default = Matrix4x4::identity(), std::string description = std::string()) : 
		UnifiedProperty(EPropertyType::Matrix, title, description), 
		valueDefault(default), 
		value(valueDefault)
	{
	}

	virtual Matrix4x4 GetMatrix() const override
	{
		return hasValue ? value : valueDefault;
	}

	virtual void SetMatrix(Matrix4x4 value) override
	{
		hasValue = true;
		value = value;
	}

	virtual void SerializeFormat(BinaryWriter& writer) const override
	{
		UnifiedProperty::SerializeFormat(writer);
		writer.WriteMatrix(valueDefault);
	}

	virtual void SerializeData(BinaryWriter& writer) const override
	{
		UnifiedProperty::SerializeData(writer);
		writer.WriteMatrix(value);
	}

	virtual void DeserializeData(BinaryReader& reader) override
	{
		UnifiedProperty::DeserializeData(reader);
		value = reader.ReadMatrix();
	}
};

class UnifiedTexture2DProperty : public UnifiedProperty
{
private:
	int64_t id;
	float xOffset, yOffset, xScale, yScale;

protected:
	virtual void DeserializeFormatInternal(BinaryReader& reader) override
	{
	}

public:

	UnifiedTexture2DProperty(std::string title = std::string(), std::string description = std::string()) : 
		UnifiedProperty(EPropertyType::Texture2D, title, description), 
		id(-1), 
		xOffset(0), yOffset(0), xScale(1), yScale(1)
	{
	}

	int64_t GetTextureId() const { return id; }

	virtual std::shared_ptr<TextureMap> GetTexture2D() const override
	{
		return nullptr;
	}

	virtual void SerializeFormat(BinaryWriter& writer) const override
	{
		UnifiedProperty::SerializeFormat(writer);
	}

	virtual void SerializeData(BinaryWriter& writer) const override
	{
		UnifiedProperty::SerializeData(writer);
		writer.WriteInt64(id);
		writer.WriteSingle(xOffset);
		writer.WriteSingle(yOffset);
		writer.WriteSingle(xScale);
		writer.WriteSingle(yScale);
	}

	virtual void DeserializeData(BinaryReader& reader) override
	{
		UnifiedProperty::DeserializeData(reader);
		id = reader.ReadInt64();
		xOffset = reader.ReadSingle();
		yOffset = reader.ReadSingle();
		xScale = reader.ReadSingle();
		yScale = reader.ReadSingle();
	}
};

class UnifiedSettings
{
private:
	std::unordered_map<std::string, std::shared_ptr<UnifiedSettings>> childFormats;
	std::unordered_multimap<std::string, std::shared_ptr<UnifiedSettings>> childInstances;
	std::unordered_map<std::string, std::shared_ptr<UnifiedProperty>> properties;
	std::string title;
	std::string description;
	int64_t flags;

	template<class TEnum>
	void CheckEnumClass()
	{
		static_assert(std::is_enum<TEnum>::value && !std::is_convertible<E, int>::value>, "Only enum classes can be passed as template argument here!");
	}

	static void Skip(BinaryReader& reader)
	{
		UnifiedSettings().DeserializeData(reader);
	}

	void EnumTextures(std::vector<std::shared_ptr<UnifiedTexture2DProperty>>& list)
	{
		for(auto& prop : properties)
		{
			if(prop.second->GetType() == EPropertyType::Texture2D)
				list.push_back(std::dynamic_pointer_cast<UnifiedTexture2DProperty>(prop.second));
		}

		for(auto& child : childInstances)
		{
			child.second->EnumTextures(list);
		}
	}

public:

	std::string GetDescription() const { return description; }
	std::string GetTitle() const { return title; }

	UnifiedSettings(std::string title = std::string(), std::string description = std::string()) : title(title), description(description), flags(0)
	{
	}

	std::vector<std::shared_ptr<UnifiedTexture2DProperty>> EnumTextures()
	{
		std::vector<std::shared_ptr<UnifiedTexture2DProperty>> list;
		EnumTextures(list);
		return list;
	}

	void ClearFlag(EUnifiedFormatFlag flag)
	{
		flags &= ~((int64_t)flag);
	}

	void SetFlag(EUnifiedFormatFlag flag)
	{
		flags |= (int64_t)flag;
	}

	void AddIntegerProperty(std::string name, std::string title = std::string(), int64_t default = 0, int64_t min = std::numeric_limits<int64_t>::min(), int64_t max = std::numeric_limits<int64_t>::max(), std::string description = std::string())
		{ properties.insert(std::make_pair(name, std::make_shared<UnifiedIntegerProperty>(title, default, min, max, description))); }

	void AddSingleProperty(std::string name, std::string title = std::string(), float default = 0, float min = -std::numeric_limits<float>::max(), float max = std::numeric_limits<float>::min(), std::string description = std::string())
		{ properties.insert(std::make_pair(name, std::make_shared<UnifiedFloatProperty>(title, default, min, max, description))); }

	void AddVector3Property(std::string name, std::string title = std::string(), Vector3 default = Vector3(0, 0, 0), std::string description = std::string()) 
		{ properties.insert(std::make_pair(name, std::make_shared<UnifiedVector3Property>(title, default, description))); }

	void AddColorProperty(std::string name, std::string title = std::string(), Pixel default = Pixel(), std::string description = std::string()) 
		{ properties.insert(std::make_pair(name, std::make_shared<UnifiedPixelProperty>(title, default, description))); }

	void AddMatrixProperty(std::string name, std::string title = std::string(), Matrix4x4 default = Matrix4x4::identity(), std::string description = std::string()) 
		{ properties.insert(std::make_pair(name, std::make_shared<UnifiedMatrixProperty>(title, default, description))); }

	void AddTexture2DProperty(std::string name, std::string title = std::string(), std::string description = std::string()) 
		{ properties.insert(std::make_pair(name, std::make_shared<UnifiedTexture2DProperty>(title, description))); }

	void AddBooleanProperty(std::string name, std::string title = std::string(), bool default = false, std::string description = std::string()) 
		{ properties.insert(std::make_pair(name, std::make_shared<UnifiedBooleanProperty>(title, default, description))); }

	void AddStringProperty(std::string name, std::string title = std::string(), std::string default = std::string(), std::string description = std::string()) 
		{ properties.insert(std::make_pair(name, std::make_shared<UnifiedStringProperty>(title, default, description))); }

	std::shared_ptr<UnifiedProperty> Get(std::string name) const
	{
		auto it = properties.find(name);
		if(it == properties.end())
			throw std::invalid_argument("Unknown property \"" + name + "\".");

		return it->second;
	}

	bool HasFlag(EUnifiedFormatFlag flag) const { return (flags & (int64_t)flag) != 0; }

	float GetSingle(std::string name) const { return Get(name)->GetSingle(); }
	Vector3 GetVector3(std::string name) const { return Get(name)->GetVector3(); }
	Matrix4x4 GetMatrix(std::string name) const { return Get(name)->GetMatrix(); }
	Pixel GetColor(std::string name) const { return Get(name)->GetColor(); }
	bool GetBoolean(std::string name) const { return Get(name)->GetBoolean(); }
	std::string GetString(std::string name) const { return Get(name)->GetString(); }
	std::shared_ptr<TextureMap> GetTexture2D(std::string name) const { return Get(name)->GetTexture2D(); }

	void SetSingle(std::string name, float value) const { Get(name)->SetSingle(value); }
	void SetVector3(std::string name, Vector3 value) const { Get(name)->SetVector3(value); }
	void SetMatrix(std::string name, Matrix4x4 value) const { Get(name)->SetMatrix(value); }
	void SetColor(std::string name, Pixel value) const { Get(name)->SetColor(value); }
	void SetBoolean(std::string name, bool value) const { Get(name)->SetBoolean(value); }
	void SetString(std::string name, std::string value) const { Get(name)->SetString(value); }
	
	std::shared_ptr<UnifiedSettings> GetChild(std::string name) const
	{
		auto it = childFormats.find(name);
		if(it == childFormats.end())
			throw std::invalid_argument("Unknown child \"" + name + "\".");

		return it->second;
	}

	std::shared_ptr<UnifiedSettings> TryGetInstancePath(std::string name) const
	{
		auto it = childInstances.find(name);
		if(it == childInstances.end())
			return nullptr;

		return it->second;
	}

	template<typename... TArgs>
	std::shared_ptr<UnifiedSettings> TryGetInstancePath(std::string name, TArgs&&... args) const
	{
		auto node = TryGetInstancePath(name);
		if(!node)
			return nullptr;

		return node->TryGetInstancePath(args...);
	}

	std::shared_ptr<UnifiedSettings> AddChildFormat(
		std::string name, 
		std::string title = std::string(), 
		std::string description = std::string())
	{
		auto child = std::make_shared<UnifiedSettings>(title, description);
		childFormats.insert(std::make_pair(name, child));
		return child;
	}

	std::shared_ptr<UnifiedSettings> AddChildInstance(std::string name)
	{
		auto it = childFormats.find(name);
		if(it == childFormats.end())
			throw std::invalid_argument("A child format named \"" + name + "\" does not exist!");

		auto child = it->second->Clone();
		childInstances.insert(std::make_pair(name, child));
		return child;
	}

	void Inherit(std::shared_ptr<UnifiedSettings> existingFormat)
	{
		std::stringstream format;
		existingFormat->SerializeFormat(BinaryWriter(format));
		format.seekg(std::ios_base::beg, 0);
		
		*this = *DeserializeFormat(BinaryReader(format));
	}

	void ClearData()
	{
		childInstances.clear();
		for(auto& prop : properties) prop.second->ClearData();
	}

	std::vector<std::shared_ptr<UnifiedSettings>> GetChildInstances(std::string name)
	{
		std::vector<std::shared_ptr<UnifiedSettings>> res;
		for(auto& e : childInstances)
		{
			if(e.first == name)
				res.push_back(e.second);
		}

		return res;
	}

	void SerializeFormat(BinaryWriter& writer) const
	{
		writer.WriteString("RAYLICE - UNIFIED SETTINGS FORMAT");
		writer.WriteInt32(0x1000); // version

		writer.WriteString(title);
		writer.WriteString(description);
		writer.WriteInt64(flags);

		// serialize property specifications
		writer.WriteInt32(properties.size());
		for(const auto& e : properties)
		{
			writer.WriteString(e.first);
			e.second->SerializeFormat(writer);
		}

		// serialize child specifications
		writer.WriteInt32(childFormats.size());
		for(const auto& e : childFormats)
		{
			writer.WriteString(e.first);
			e.second->SerializeFormat(writer);
		}
	}

	static std::shared_ptr<UnifiedSettings> DeserializeFormat(BinaryReader& reader)
	{
		if(reader.ReadString() != "RAYLICE - UNIFIED SETTINGS FORMAT")
			throw std::invalid_argument("Format stream is corrupt!");

		if(reader.ReadInt32() != 0x1000)
			throw std::invalid_argument("Unsupported version.");

		auto title = reader.ReadString();
		auto description = reader.ReadString();
		auto res = std::make_shared<UnifiedSettings>(title, description);

		res->flags = reader.ReadInt64();

		// serialize property specifications	
		int propCount = reader.ReadInt32();
		for(int i = 0; i < propCount; i++)
		{
			auto name = reader.ReadString();
			auto format = UnifiedProperty::DeserializeFormat(reader);

			res->properties.insert(std::make_pair(name, format));
		}

		// serialize child specifications
		int childCount = reader.ReadInt32();
		for(int i = 0; i < childCount; i++)
		{
			auto name = reader.ReadString();
			auto format = UnifiedSettings::DeserializeFormat(reader);

			res->childFormats.insert(std::make_pair(name, format));
		}

		return res;
	}

	void SerializeData(BinaryWriter& writer) const
	{
		writer.WriteString("RAYLICE - UNIFIED SETTINGS DATA");
		writer.WriteInt32(0x1000); // version

		writer.WriteBoolean(false); // IsExpanded

		// serialize property specifications
		writer.WriteInt32(properties.size());
		for(const auto& e : properties)
		{
			writer.WriteString(e.first);
			e.second->SerializeData(writer);
		}

		// serialize child specifications
		writer.WriteInt32(childInstances.size());
		for(const auto& e : childInstances)
		{
			writer.WriteString(e.first);
			e.second->SerializeData(writer);
		}
	}

	void DeserializeData(BinaryReader& reader)
	{
		childInstances.clear();

		if(reader.ReadString() != "RAYLICE - UNIFIED SETTINGS DATA")
			throw std::invalid_argument("Data stream is corrupt!");

		if(reader.ReadInt32() != 0x1000)
			throw std::invalid_argument("Unsupported version.");

		reader.ReadBoolean(); // IsExpanded

		// deserialize property data
		int propCount = reader.ReadInt32();
		for(int i = 0; i < propCount; i++)
		{
			auto name = reader.ReadString();
			auto it = properties.find(name);
			if(it == properties.end())
			{
				UnifiedProperty::Skip(reader);
				continue; // ignore unknown properties
			}

			// update data in current model
			it->second->DeserializeData(reader);
		}

		// deserialize child data
		int childCount = reader.ReadInt32();
		for(int i = 0; i < childCount; i++)
		{
			auto name = reader.ReadString();
			auto it = childFormats.find(name);
			if(it == childFormats.end())
			{
				UnifiedSettings::Skip(reader);
				continue; // ignore unknown children
			}

			// update data in current model
			auto instance = it->second->Clone();
			instance->DeserializeData(reader);

			childInstances.insert(std::make_pair(name, instance));
		}
	}

	std::shared_ptr<UnifiedSettings> Clone()
	{
		std::stringstream format, data;

		SerializeFormat(BinaryWriter(format));
		SerializeData(BinaryWriter(data));

		format.seekg(std::ios_base::beg, 0);
		data.seekg(std::ios_base::beg, 0);

		auto result = UnifiedSettings::DeserializeFormat(BinaryReader(format));
		result->DeserializeData(BinaryReader(data));

		return result;
	}


};

extern void RunTest_UnifiedSettings();

