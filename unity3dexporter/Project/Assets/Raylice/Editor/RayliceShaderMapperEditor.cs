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
using System.Linq;
using System.Collections;

[CustomEditor(typeof(RayliceShaderMapper))]
public class RayliceShaderMapperEditor : Editor
{
    public override void OnInspectorGUI()
    {
        RayliceShaderMapper mapper = target as RayliceShaderMapper;

        // select Unity shader
        if (!(mapper.useSpecificShaderObject = EditorGUILayout.Toggle("Provide Shader Object", mapper.useSpecificShaderObject)))
            mapper.unityShaderToMap = ShaderMenuUtility.ShaderField("Select Shader", mapper.unityShaderToMap);
        else
            mapper.unityShaderToMap = (Shader)EditorGUILayout.ObjectField("Specific Shader Object", mapper.unityShaderToMap, typeof (Shader));

        // select raylice shader
        mapper.rayliceMaterial = (RayliceMaterial)EditorGUILayout.ObjectField("Target Raylice Material", mapper.rayliceMaterial, typeof(RayliceMaterial));

        if(mapper.unityShaderToMap == null)
            EditorGUILayout.HelpBox("You need to assign a Unity3D Shader for this script to work!", MessageType.Error, true);
        else if (mapper.rayliceMaterial == null)
            EditorGUILayout.HelpBox("You need to assign a Raylice Material for this script to work!", MessageType.Error, true);
        else
        {
            // configure mapping
            RayliceUnifiedSettingsEditor.ConfigureMapping(mapper);
        }

        if (GUI.changed)
            EditorUtility.SetDirty(target);
    }

}