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

[CustomEditor(typeof(RayliceMappingPreview))]
public class RayliceMappingPreviewEditor : Editor
{
    public override void OnInspectorGUI()
    {
        var mapper = target as RayliceMappingPreview;
        var ctrl = RayliceController.Get(mapper.gameObject);

        mapper.unityMaterial = (Material)EditorGUILayout.ObjectField("Unity Material To Preview", mapper.unityMaterial, typeof(Material));

        if (mapper.unityMaterial != null)
        {
            var rayMat = ctrl.GetMappedMaterial(mapper.unityMaterial);

            if (rayMat == null)
                EditorGUILayout.HelpBox("Failed to find a material mapping for material \"" + mapper.unityMaterial + "\".", MessageType.Error, true);
            else
                RayliceUnifiedSettingsEditor.Configure(rayMat, rayMat);
        }

        if (GUI.changed)
            EditorUtility.SetDirty(target);
    }

}