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

UnityImporter::UnityImporter(std::string file)
	: stream(file, std::ios_base::binary), reader(stream)
{
	if(stream.fail())
		throw std::invalid_argument("File could not be read!");

	DeserializeScene();

	// collect cameras
	for(auto& camSettings : settings->GetChildInstances("Camera"))
	{
		cameras.push_back(Camera(
			camSettings->GetString("Name"),
			camSettings->GetSingle("Aspect"),
			camSettings->GetMatrix("LocalToWorld"),
			camSettings->GetMatrix("Projection")));
	}

	if(cameras.empty())
		throw std::invalid_argument("Every scene needs at least one camera!");

	// collect lights
	int iLight = 0;
	for(const auto& mesh : meshes)
	{
		if(mesh->GetLight())
		{
			auto light = mesh->GetLight();
			light->SetIndex(iLight++);
			lights.push_back(light);
		}
	}

	if(lights.empty())
		throw std::invalid_argument("Every scene needs at least one light!");

}

void UnityImporter::ReadSync()
{
	if(reader.ReadInt64() != 0x72BCC3123489D6EA)
		throw std::invalid_argument("Lost sync with stream.");
}

std::shared_ptr<Mesh> Mesh::Instanciate(Matrix4x4 transform, std::shared_ptr<UnifiedSettings> material)
{
	auto instance = std::make_shared<Mesh>();

	instance->vertices = vertices;

	for(int i = 0; i < instance->vertices.size(); i++)
	{
		auto& v = instance->vertices[i];
		v.normal = Math::TransformDirection(transform, vertices[i].normal);
		v.position = Math::TransformVector(transform, vertices[i].position);
	}

	instance->triangles.reserve(triangles.size());
	for (int i = 0, j = 0; i < triangles.size(); i++, j +=3)
    {
		instance->triangles.push_back(Triangle(instance.get(), j, j + 1, j + 2));
	}

	instance->material = BSDFMaterial::FromTemplate(material);
	instance->light = LightSource::TryFromTemplate(material, instance);

	return instance;
}

void UnityImporter::DeserializeScene()
{
	// read last Int64 to get offset of first byte of metadata
	stream.seekg(-8, std::ios_base::end);
	stream.seekg(reader.ReadInt64(), std::ios_base::beg);

    // deserialize global config
    ReadSync();
    settings = UnifiedSettings::DeserializeFormat(reader);
    settings->DeserializeData(reader);

    // deserialize texture dictionary
    ReadSync();
    int texCount = reader.ReadInt32();
    for (int i = 0; i < texCount; i++)
    {
        int64_t id = reader.ReadInt64();
        int64_t offset = reader.ReadInt64();

		texIdToTex.insert(std::make_pair(id, ReadTexture(offset)));
    }

    // deserialize materials
    ReadSync();
    int matCount = reader.ReadInt32();
	auto matFormat = settings->GetChild("Material");
	std::vector<std::shared_ptr<UnifiedSettings>> activeMaterials;
    for (int i = 0; i < matCount; i++)
    {
        auto matSettings = matFormat->Clone();
        matSettings->DeserializeData(reader);

        for(auto tex : matSettings->EnumTextures())
        {
            auto unused = texIdToTex[tex->GetTextureId()];
        }

        activeMaterials.push_back(matSettings);
    }

    // deserialize mesh dictionary
    ReadSync();
    int meshCount = reader.ReadInt32();
    for (int i = 0; i < meshCount; i++)
    {
        int64_t id = reader.ReadInt64();
        int64_t offset = reader.ReadInt64();

        meshIdToMesh.insert(std::make_pair(id, DeserializeTriangles(offset)));
    }

    // deserialize active meshes
    ReadSync();
    meshCount = reader.ReadInt32();
    for (int i = 0; i < meshCount; i++)
    {
        int64_t id = reader.ReadInt64();
        int matIndex = reader.ReadInt32();
        Matrix4x4 transform = reader.ReadMatrix();
		
		meshes.push_back(meshIdToMesh[id]->Instanciate(transform, activeMaterials.at(matIndex)));
    }

    ReadSync();
}

std::shared_ptr<Mesh> UnityImporter::DeserializeTriangles(int64_t fileOffset)
{
	int64_t oldPosition = stream.tellg();
	stream.seekg(fileOffset, std::ios_base::beg);

	std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>();

	ReadSync();

    int triangleCount = reader.ReadInt32() / 3;
    bool hasUVs = reader.ReadBoolean();
	
	mesh->vertices.reserve(triangleCount * 3);
	triangleBuffer.resize(triangleCount * 3 * (hasUVs ? 8 : 6));

	reader.ReadSingleArray(triangleBuffer, triangleBuffer.size());

	if(0x76FA0B62 != reader.ReadInt32())
		throw std::invalid_argument("Stream is out of sync.");

    for (int i = 0, j = 0; i < triangleCount * 3; i++)
    {
		Vertex v;

        float x = triangleBuffer[j++];
        float y = triangleBuffer[j++];
        float z = triangleBuffer[j++];
		v.position = Vector3(x, y, z);

        x = triangleBuffer[j++];
        y = triangleBuffer[j++];
        z = triangleBuffer[j++];
		v.normal = Vector3(x, y, z);

        if (hasUVs)
        {
            x = triangleBuffer[j++];
            y = triangleBuffer[j++];
			v.matUv = Vector3(x, y, 0);
        }
		
		mesh->vertices.push_back(v);
    }

	mesh->triangles.reserve(triangleCount);
	for (int i = 0, j = 0; i < triangleCount; i++, j +=3)
    {
		mesh->triangles.push_back(Triangle(mesh.get(), j, j + 1, j + 2));
	}

	stream.seekg(oldPosition, std::ios_base::beg);

	return mesh;
} 

std::shared_ptr<TextureMap> UnityImporter::ReadTexture(int64_t fileOffset)
{
	//if (reader.ReadInt32() != TEXTURE_UID)
	//	throw std::invalid_argument("This is not the begin of a texture.");

	//if(!reader.ReadBoolean())
	//	return nullptr;

	//int64_t hashCode = reader.ReadInt64();
	//auto existing = textures.find(hashCode);
	//std::shared_ptr<TextureMap> texture;

	//if(existing == textures.end())
	//{
	//	// read pixels from file
	//	reader.ReadString();
	//	auto channelCount = reader.ReadInt32();
	//	auto width = reader.ReadInt32();
	//	auto height = reader.ReadInt32();
	//	auto sizeInBytes = reader.ReadInt32();
	//	reader.ReadBytes(sizeInBytes, textureBuffer);
	//				
	//	texture = std::make_shared<TextureMap>(width, height, channelCount);
	//	texture->SetBytesRGBA(textureBuffer);

	//	textures.insert(std::make_pair(hashCode, texture));
	//}
	//else
	//	texture = existing->second;

	return std::make_shared<TextureMap>(1, 1, 1);
}