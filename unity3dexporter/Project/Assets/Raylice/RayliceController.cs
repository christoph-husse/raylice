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
using System.Linq;
using System.Text;
using System.IO;
using Raylice;
using UnityEditor;
using UnityEngine;

public class RayliceController : MonoBehaviour
{
    private static DateTime cfgFormatStamp;
    private static UnifiedSettings cfgFormatCache;

    public static RayliceController Get(GameObject childNode)
    {
        var transform = childNode.transform;

        while (transform != null)
        {
            if (transform.GetComponent<RayliceController>())
                return transform.GetComponent<RayliceController>();

            transform = childNode.transform.parent;
        }

        Debug.LogError("Node is not parented by a RayliceController!", childNode);
        return null;
    }

    public UnifiedSettings GetMappedMaterial(Material unityMaterial)
    {
        UnifiedSettings rayMat = null;

        // try one-to-one mappers
        foreach(var mapper in GetComponentsInChildren<RayliceOneToOneMapping>())
        {
            if ((mapper.unityMaterialToMap == unityMaterial) && (mapper.rayliceMaterial != null))
            {
                rayMat = mapper.rayliceMaterial.Settings.Clone();
                break;
            }
        }

        if (rayMat == null)
        {
            // try material mappers
            foreach (var mapper in GetComponentsInChildren<RayliceMaterialMapper>())
            {
                if ((rayMat = mapper.GetMappedMaterial(unityMaterial)) != null)
                    break;
            }
        }

        // try shader mappers
        foreach (var mapper in GetComponentsInChildren<RayliceShaderMapper>())
        {
            if (rayMat == null)
            {
                if ((rayMat = mapper.GetMappedMaterial(unityMaterial)) != null)
                    break;
            }
            else
            {
                mapper.ApplyMappedMaterial(unityMaterial, rayMat);
            }
        }

        rayMat = rayMat ?? new UnifiedSettings();
        if (rayMat.HasProperty("Name"))
        {
            var name = AssetDatabase.GetAssetPath(unityMaterial);
            if (String.IsNullOrEmpty(name))
                name = unityMaterial.name;

            rayMat.SetString("Name", name);
        }

        return rayMat;
    }

    public static UnifiedSettings LoadConfigFormat()
    {
        string cfgFormatPath = "./config.format.raylice";

        try
        {
            while (!File.Exists(cfgFormatPath))
            {
                cfgFormatPath = "../" + cfgFormatPath;
                if (!Directory.Exists(Path.GetDirectoryName(cfgFormatPath)))
                {
                    cfgFormatPath = null;
                    break;
                }
            }
        }
        catch
        {
            cfgFormatPath = null;
        }

        if (cfgFormatPath == null)
            throw new FileNotFoundException("Raylice configuration format named \"config.format.raylice\" could not be found in current or any parent directory!");

        if (File.GetLastWriteTime(cfgFormatPath) > cfgFormatStamp)
        {
            try
            {
                using (var cfgFormat = File.OpenRead(cfgFormatPath))
                {
                    var res = UnifiedSettings.DeserializeFormat(new BinaryReader(cfgFormat));
                    cfgFormatStamp = File.GetLastWriteTime(cfgFormatPath);
                    return cfgFormatCache = res;
                }
            }
            catch (Exception e)
            {
                throw new ArgumentException("Raylice configuration format in \"" + Path.GetFullPath(cfgFormatPath) + "\" contains errors!", e);
            }
        }
        else
            return cfgFormatCache;
    }
}
