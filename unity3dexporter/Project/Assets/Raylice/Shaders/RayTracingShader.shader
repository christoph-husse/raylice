Shader "RayTracingShader" 
{
	Properties {
		BumpMap ("Bump Texture ({A} or {R}GB or RGB{A})", 2D) = "white" {}
		 
		LAMBERT_Color ("Diffuse Color", Color) = (1,1,1,1)
 		LAMBERT_ColorMap ("Diffuse Texture (RGB)", 2D) = "white" {}
		LAMBERT_AlphaMap ("Alpha Texture ({A} or {R}GB or RGB{A})", 2D) = "white" {}
		LAMBERT_EmissiveIntensity ("Emissive Intensity", Range (0.01, 100)) = 0.5
		LAMBERT_PhotonIntensity ("Photon Intensity", Range (0.01, 100)) = 3
		LAMBERT_PhotonMultiplier ("Photon Multiplier", Range (0.0001, 100000)) = 1

		LAMBERT_RefractiveIndex  ("Refractive Index", Range (1.1, 10)) = 1.5
		LAMBERT_VisibilityPercentage  ("Visibility Percentage", Range (0, 10)) = 1

		TRANSMISSIVE_Alpha ("Alpha", Range (0, 1)) = 0.5
 		TRANSMISSIVE_AlphaMap ("Alpha Texture (A)", 2D) = "gray" {}
		TRANSMISSIVE_RefractiveIndex  ("Refractive Index", Range (1.1, 10)) = 1.5
		TRANSMISSIVE_VisibilityPercentage  ("Visibility Percentage", Range (0, 10)) = 1
		TRANSMISSIVE_AlphaMap ("Alpha Texture ({A} or {R}GB or RGB{A})", 2D) = "white" {}

		REFLECTIVE_RefractiveIndex  ("Refractive Index", Range (1.1, 10)) = 1.5
		REFLECTIVE_VisibilityPercentage  ("Visibility Percentage", Range (0, 10)) = 1
		REFLECTIVE_AlphaMap ("Alpha Texture ({A} or {R}GB or RGB{A})", 2D) = "white" {}

		REFRACTIVE_RefractiveIndex  ("Refractive Index", Range (1.1, 10)) = 1.5
		REFRACTIVE_VisibilityPercentage  ("Visibility Percentage", Range (0, 10)) = 1
		REFRACTIVE_AlphaMap ("Alpha Texture ({A} or {R}GB or RGB{A})", 2D) = "white" {}

		MICROFACET_RefractiveIndex  ("Refractive Index", Range (1.1, 10)) = 1.5
		MICROFACET_VisibilityPercentage  ("Visibility Percentage", Range (0, 10)) = 1
		MICROFACET_AlphaMap ("Alpha Texture ({A} or {R}GB or RGB{A})", 2D) = "white" {}
 	}

	CustomEditor "RayTracingShaderEditor"

	SubShader 
	{
		Tags { "RenderType"="Opaque" }
		Cull Off
		LOD 200

	CGPROGRAM
		#pragma surface surf Lambert

		sampler2D LAMBERT_ColorMap;
		sampler2D LAMBERT_AlphaMap;
		sampler2D BumpMap;
		fixed4 LAMBERT_Color;
		float LAMBERT_EmissiveIntensity;
		float LAMBERT_PhotonIntensity;
		float LAMBERT_PhotonMultiplier;
		float LAMBERT_RefractiveIndex;
		float LAMBERT_VisibilityPercentage;

		float TRANSMISSIVE_Alpha;
		sampler2D TRANSMISSIVE_AlphaMap;
		float TRANSMISSIVE_RefractiveIndex;
		float TRANSMISSIVE_VisibilityPercentage;

		sampler2D REFLECTIVE_AlphaMap;
		float REFLECTIVE_RefractiveIndex;
		float REFLECTIVE_VisibilityPercentage;

		sampler2D REFRACTIVE_AlphaMap;
		float REFRACTIVE_RefractiveIndex;
		float REFRACTIVE_VisibilityPercentage;

		sampler2D MICROFACET_AlphaMap;
		float MICROFACET_RefractiveIndex;
		float MICROFACET_VisibilityPercentage;

		struct Input {
			float2 uvLAMBERT_ColorMap;
		};
		   
		void surf (Input IN, inout SurfaceOutput o) {
			fixed4 c = tex2D(LAMBERT_ColorMap, IN.uvLAMBERT_ColorMap) * LAMBERT_Color;
			o.Albedo = c.rgb;

			// no immediate use, but we need to sample all textures to prevent
			// "missing texture" issues... (they would be optimized away)
			o.Alpha = clamp( 
				c.a +  
				tex2D(LAMBERT_AlphaMap, IN.uvLAMBERT_ColorMap).a +
				tex2D(TRANSMISSIVE_AlphaMap, IN.uvLAMBERT_ColorMap).a +
				tex2D(REFLECTIVE_AlphaMap, IN.uvLAMBERT_ColorMap).a +
				tex2D(REFRACTIVE_AlphaMap, IN.uvLAMBERT_ColorMap).a +
				tex2D(MICROFACET_AlphaMap, IN.uvLAMBERT_ColorMap).a +
				tex2D(BumpMap, IN.uvLAMBERT_ColorMap).a
				, 0.0, 1.0);
		}
	ENDCG
	}

	Fallback "VertexLit"
}
