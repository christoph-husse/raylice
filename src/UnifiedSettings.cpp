#include "stdafx.h"

void UnifiedProperty::Skip(BinaryReader& reader)
{
	auto type = (EPropertyType)reader.ReadInt32();
	reader.source.seekg(std::ios_base::cur, -4);

	switch(type)
	{
	case EPropertyType::Integer: UnifiedIntegerProperty().DeserializeData(reader); break;
	case EPropertyType::Float: UnifiedFloatProperty().DeserializeData(reader); break;
	case EPropertyType::Texture2D: UnifiedTexture2DProperty().DeserializeData(reader); break;
	case EPropertyType::String: UnifiedStringProperty().DeserializeData(reader); break;
	case EPropertyType::Boolean: UnifiedBooleanProperty().DeserializeData(reader); break;
	case EPropertyType::Vector3: UnifiedVector3Property().DeserializeData(reader); break;
	case EPropertyType::Matrix: UnifiedMatrixProperty().DeserializeData(reader); break;
	case EPropertyType::Pixel: UnifiedPixelProperty().DeserializeData(reader); break;
	}
}

std::shared_ptr<UnifiedProperty> UnifiedProperty::DeserializeFormat(BinaryReader& reader)
{
	std::shared_ptr<UnifiedProperty> res;
	auto type = (EPropertyType)reader.ReadInt32();

	switch(type)
	{
	case EPropertyType::Integer: res = std::make_shared<UnifiedIntegerProperty>(); break;
	case EPropertyType::Float: res = std::make_shared<UnifiedFloatProperty>(); break;
	case EPropertyType::Texture2D: res = std::make_shared<UnifiedTexture2DProperty>(); break;
	case EPropertyType::String: res = std::make_shared<UnifiedStringProperty>(); break;
	case EPropertyType::Boolean: res = std::make_shared<UnifiedBooleanProperty>(); break;
	case EPropertyType::Vector3: res = std::make_shared<UnifiedVector3Property>(); break;
	case EPropertyType::Matrix: res = std::make_shared<UnifiedMatrixProperty>(); break;
	case EPropertyType::Pixel: res = std::make_shared<UnifiedPixelProperty>(); break;
	default:
		throw std::invalid_argument("Unknown property format!");
	}

	// invoke type specific deserialization
	res->title = reader.ReadString();
	res->description = reader.ReadString();
	res->DeserializeFormatInternal(reader);

	return res;
}

void RunTest_UnifiedSettings()
{
	// create test specification
	auto settingsFormat = std::make_shared<UnifiedSettings>("Raylice Settings");

	auto matFormat = settingsFormat->AddChildFormat("Material", "A BSDF material.");
	{
		matFormat->AddStringProperty("Name", "Material Name:");

		auto visibilityFormat = std::make_shared<UnifiedSettings>("Has Visibility Constraints", "Controls where, when and how much a BSDF contributes to the image.");
		{
			visibilityFormat->SetFlag(EUnifiedFormatFlag::Switch);
			visibilityFormat->AddSingleProperty("Percentage", "Percentage", 1, 0, 10);

			auto alphaFormat = visibilityFormat->AddChildFormat("AlphaBlending", "Use Alpha Blending");
			alphaFormat->SetFlag(EUnifiedFormatFlag::Switch);
			alphaFormat->AddBooleanProperty("Automatic", "Enable Automatic Transmission", true, "Automatically add a transmissive BSDF with inverse settings for alpha blending. This will result in the usual game-style alpha blending. (Will have no effect, when there already is a transmissive BSDF)");
			alphaFormat->AddSingleProperty("Value", "Alpha Value", 1, 0, 1);
			alphaFormat->AddTexture2DProperty("Texture", "Alpha Map");
			alphaFormat->AddBooleanProperty("InvertTexture", "Invert Alpha Map", false);

			auto fresnelFormat = visibilityFormat->AddChildFormat("Fresnel", "Use Fresnel");
			fresnelFormat->SetFlag(EUnifiedFormatFlag::Switch);
			fresnelFormat->AddVector3Property("RealRefractiveIndex", "Refractive Index (real)", Vector3(1.5f, 1.5f, 1.5f));
			fresnelFormat->AddVector3Property("ImRefractiveIndex", "Refractive Index (imaginary)", Vector3(0, 0, 0));
		}

		auto causticFormat = matFormat->AddChildFormat("Caustic", "Caustic BSDF", "A BSDF which only changes the direction of screen rays and photons.");
		{
			causticFormat->AddBooleanProperty("EnableCaustics", "Enable Caustics", false);

			auto fmt = causticFormat->AddChildFormat("Reflection", "Reflection", "Simulates total, fresnel and glossy reflections.");
			fmt->AddSingleProperty("GlossyAngle", "Glossy Angle", 0, 0, Math::PI / 2);
			fmt->AddChildFormat("Visibility")->Inherit(visibilityFormat);

			fmt = causticFormat->AddChildFormat("Refraction", "Refraction", "Simulates total, fresnel and glossy refractions.");
			fmt->AddSingleProperty("GlossyAngle", "Glossy Angle", 0, 0, Math::PI / 2);
			fmt->AddChildFormat("Visibility")->Inherit(visibilityFormat);
		}

		auto surfaceFormat = matFormat->AddChildFormat("Surface", "Surface BSDF", "A BSDF which acts as a surface shader, and can not interact with screen rays and photons.");
		{
			auto fmt = surfaceFormat->AddChildFormat("Diffuse", "Diffuse", "Standard surface shader.");
			fmt->AddColorProperty("Color", "Color", Pixel(1, 1, 1));
			fmt->AddTexture2DProperty("Texture", "Texture");
			fmt->AddChildFormat("Visibility")->Inherit(visibilityFormat);

			auto photonFormat = fmt->AddChildFormat("Photon", "Emits Photons");
			photonFormat->SetFlag(EUnifiedFormatFlag::Switch);
			photonFormat->AddBooleanProperty("IsDirectional", "Is Directional Light", false);
			photonFormat->AddSingleProperty("PhotonIntensity", "Photon Intensity", 1, 0, 10);
		}

		auto lightingFormat = matFormat->AddChildFormat("Lighting", "Lighting BSDF", "A BSDF which acts as a lighting model, and can not interact with screen rays and photons.");
		{
			auto fmt = lightingFormat->AddChildFormat("Lambert", "Lambert", "Use the Lambert surface illumination model.");
			fmt->AddSingleProperty("EmissiveIntensity", "Emissive Intensity", 0, 0, 100);
			fmt->AddChildFormat("Visibility")->Inherit(visibilityFormat);
		}
	}

	auto cameraFormat = settingsFormat->AddChildFormat("Camera", "A camera from which to render the scene.");
	{
		cameraFormat->AddMatrixProperty("Projection");
		cameraFormat->AddMatrixProperty("LocalToWorld");
		cameraFormat->AddStringProperty("Name");
		cameraFormat->AddSingleProperty("Aspect");	
		cameraFormat->AddVector3Property("RectOrigin");	
		cameraFormat->AddVector3Property("RectSize");	
	}

	std::ofstream cfgFormat("./config.format.raylice", std::ios_base::trunc | std::ios::binary);
	settingsFormat->SerializeFormat(BinaryWriter(cfgFormat));
}