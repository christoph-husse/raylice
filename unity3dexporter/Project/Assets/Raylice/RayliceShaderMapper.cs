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

using System;
using System.Collections.Generic;
using Raylice;
using UnityEditor;
using UnityEngine;

public class RayliceShaderMapper : MonoBehaviour
{
    [Serializable]
    public class MappingEntry
    {
        [SerializeField] public string shaderProperty;
        [SerializeField] public string raylicePath;
    }

    public bool useSpecificShaderObject;
    public RayliceMaterial rayliceMaterial;
    public Shader unityShaderToMap;
    [SerializeField]
    public MappingEntry[] mappings;

    internal UnifiedSettings GetMappedMaterial(Material unityMaterial)
    {
        if (unityMaterial.shader != unityShaderToMap)
            return null;

        var rayMat = rayliceMaterial.Settings.Clone();
        
        foreach(var e in mappings)
        {
            var target = rayMat.Get(e.raylicePath);
            switch(target.GetType())
            {
                case EPropertyType.Vector3: target.SetVector3(unityMaterial.GetVector(e.shaderProperty));
                    break;
                case EPropertyType.Color: target.SetColor(unityMaterial.GetColor(e.shaderProperty));
                    break;
                case EPropertyType.Texture2D: target.SetTexture2D(unityMaterial.GetTexture(e.shaderProperty) as Texture2D);
                    break;
                case EPropertyType.Float: target.SetSingle(unityMaterial.GetFloat(e.shaderProperty));
                    break;
                default:
                    throw new ApplicationException();
            }
        }

        return rayMat;
    }

    internal void ApplyMappedMaterial(Material unityMaterial, UnifiedSettings rayMaterial)
    {
        if (unityMaterial.shader != unityShaderToMap)
            return;

        foreach (var e in mappings)
        {
            var target = rayMaterial.TryGet(e.raylicePath);

            if (target == null)
                continue;

            switch (target.GetType())
            {
                case EPropertyType.Vector3: target.SetVector3(unityMaterial.GetVector(e.shaderProperty));
                    break;
                case EPropertyType.Color: target.SetColor(unityMaterial.GetColor(e.shaderProperty));
                    break;
                case EPropertyType.Texture2D: target.SetTexture2D(unityMaterial.GetTexture(e.shaderProperty) as Texture2D);
                    break;
                case EPropertyType.Float: target.SetSingle(unityMaterial.GetFloat(e.shaderProperty));
                    break;
                default:
                    throw new ApplicationException();
            }
        }
    }
}