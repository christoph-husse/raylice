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
using System.IO;
using System.Linq;
using System.Text;
using UnityEditor;
using UnityEngine;

namespace Raylice
{
    /**
     * There is only one thing to say about this class: 
     * http://www.tubechop.com/watch/1140495
     */
    public class RayliceUnifiedSettingsEditor
    {
        private class GUIState
        {
            public int adder_SelectionIndex = 0;
        }

        private readonly static Dictionary<UnifiedSettings, GUIState> guiStates = new Dictionary<UnifiedSettings, GUIState>();

        private static GUIState GetState(UnifiedSettings settings)
        {
            GUIState res;
            if (guiStates.TryGetValue(settings, out res))
                return res; 
            guiStates.Add(settings, res = new GUIState());
            return res; 
        }

        public static UnifiedSettings Configure(UnifiedSettings format, UnifiedSettings data)
        {
            if (data == null)
                data = format.Clone();

            if (GUILayout.Button("Update Format"))
            {
                MemoryStream stream = new MemoryStream();
                data.SerializeData(new BinaryWriter(stream));
                stream.Position = 0;
                var oldTex = data.EnumTextures().ToArray();
                data = format.Clone();
                data.DeserializeData(new BinaryReader(stream));

                // try to reassign textures (this may partially or even completely fail depending on how much the format has changed)
                foreach (var tex in oldTex)
                {
                    if (tex.GetTexture2D() == null)
                        continue;

                    try
                    {
                        data.SetTexture2D(tex.GetPropertyPath(), tex.GetTexture2D());
                    }
                    catch
                    {
                        Debug.LogWarning("Texture at \"" + tex.GetPropertyPath() + "\" could not be transferred to updated data-format.", tex.GetTexture2D());
                    }
                }
            }

            return ConfigureInternal(data);
        }

        private static bool Toggle(string title, bool value, int indent = 0)
        {
            bool res;
            EditorGUILayout.BeginHorizontal();
            {
                GUILayout.Space(indent);
                res = EditorGUILayout.Toggle(value, GUILayout.Width(15));
                EditorGUILayout.LabelField(title);
            }
            EditorGUILayout.EndHorizontal();
            return res;
        }

        private static ShaderMappingEntry[] ExtractShaderProperties(Shader shader)
        {
            var result = new List<ShaderMappingEntry>();
            int count = ShaderUtil.GetPropertyCount(shader);
            for (int i = 0; i < count; i++)
            {
                var name = ShaderUtil.GetPropertyName(shader, i);
                var entry = new ShaderMappingEntry();

                switch(ShaderUtil.GetPropertyType(shader, i))
                {
                    case ShaderUtil.ShaderPropertyType.TexEnv:
                        if (ShaderUtil.GetTexDim(shader, i) == ShaderUtil.ShaderPropertyTexDim.TexDim2D)
                            entry.type = EPropertyType.Texture2D;
                        else
                            entry = null;
                        break;

                    case ShaderUtil.ShaderPropertyType.Color:
                        entry.type = EPropertyType.Color;
                        break;

                    case ShaderUtil.ShaderPropertyType.Float:
                        entry.type = EPropertyType.Float;
                        break;

                    case ShaderUtil.ShaderPropertyType.Vector:
                        entry.type = EPropertyType.Vector3;
                        break;

                    case ShaderUtil.ShaderPropertyType.Range:
                        entry.type = EPropertyType.Float;
                        break;
                }

                if (entry == null)
                    continue;

                entry.shaderProperty = name;
                result.Add(entry);
            }
            return result.ToArray();
        }

        public static void ConfigureMapping(RayliceShaderMapper mapping)
        {
            var uniShader = mapping.unityShaderToMap;

            if ((uniShader != null) && (mapping.rayliceMaterial != null))
            {
                var sources = ExtractShaderProperties(uniShader);
                var target = mapping.rayliceMaterial.Settings;
                
                // unpack mappings
                foreach(var entry in sources)
                {
                    if (mapping.mappings == null)
                        continue;

                    foreach(var mapTo in mapping.mappings.Where(e => e.shaderProperty == entry.shaderProperty).Select(e => e.raylicePath))
                    {
                        try
                        {
                            entry.raylicePaths.Add(target.Get(mapTo));
                        }
                        catch
                        {
                        }
                    }
                }

                ConfigureMappingInternal(target, sources);

                // pack mappings
                var packed = new List<RayliceShaderMapper.MappingEntry>();
                foreach (var entry in sources)
                {
                    foreach (var mapTo in entry.raylicePaths)
                    {
                        packed.Add(new RayliceShaderMapper.MappingEntry(){raylicePath = mapTo.GetPropertyPath(), shaderProperty = entry.shaderProperty});
                    } 
                }
                mapping.mappings = packed.ToArray();
            } 
            else
            {
                // TODO: warning
            }
        }

        private class ShaderMappingEntry
        {
            public EPropertyType type = EPropertyType.Matrix;
            public string shaderProperty = "[Inherited]";
            public readonly List<UnifiedProperty> raylicePaths = new List<UnifiedProperty>();
        }

        private static void DisplayPropertyMapper(UnifiedProperty target, ShaderMappingEntry[] sources, int indent)
        {
            var typedSources = new ShaderMappingEntry[1] { new ShaderMappingEntry() }.Union(sources.Where(e => e.type == target.GetType())).ToArray();

            if(typedSources.Length == 1)
                return;

            EditorGUILayout.BeginHorizontal();
            {
                GUILayout.Space(indent);
                EditorGUILayout.LabelField(target.GetTitle());

                // find shader propert that is currently assigned to target, if any
                int index = 0;
                for(int i = 1; i < typedSources.Length; i++)
                {
                    if (typedSources[i].raylicePaths.Contains(target))
                    {
                        index = i;
                        break;
                    }
                }

                int newIndex = EditorGUILayout.Popup(index, typedSources.Select(e => e.shaderProperty).ToArray());
                if(newIndex != index)
                {
                    typedSources[index].raylicePaths.Remove(target);

                    if(newIndex > 0)
                        typedSources[newIndex].raylicePaths.Add(target);
                }

            }
            EditorGUILayout.EndHorizontal();
        }

        private static void ConfigureMappingInternal(UnifiedSettings data, ShaderMappingEntry[] sources, int indent = 0, Boolean forcedIndent = false)
        {
            var state = GetState(data);

            if (indent > 60)
                indent = (forcedIndent ? 80 : 60);

            // add new child switches
            foreach (var @switch in data.EnumChildFormats().Where(e => e.HasFlag(EUnifiedFormatFlag.Switch)).Select(e => e).ToArray())
            {
                var instance = data.GetChildInstances(@switch.GetName()).FirstOrDefault();

                if (instance != null)
                {
                    EditorGUILayout.BeginHorizontal();
                    {
                        GUILayout.Space(indent);
                        EditorGUILayout.Foldout(true, @switch.GetTitle());
                    }
                    EditorGUILayout.EndHorizontal();

                    ConfigureMappingInternal(instance, sources, indent + 20, true);
                }
            }

            // show properties
            foreach (var target in data.EnumProperties())
            {
                DisplayPropertyMapper(target, sources, indent);
            }

            // enumerate child instances
            foreach (var instance in data.EnumChildInstances().Where(e => !e.HasFlag(EUnifiedFormatFlag.Switch)))
            {
                EditorGUILayout.BeginHorizontal();
                {
                    GUILayout.Space(indent);
                    EditorGUILayout.Foldout(true, instance.GetTitle());
                }
                EditorGUILayout.EndHorizontal();
                EditorGUILayout.Space();

                if (instance.IsExpanded)
                    ConfigureMappingInternal(instance, sources, indent + 20);
            }
        }

        private static UnifiedSettings ConfigureInternal(UnifiedSettings data, int indent = 0, Boolean forcedIndent = false)
        {
            var state = GetState(data);
            indent = Math.Min(60, indent) + (forcedIndent ? 20 : 0);

            // add new child instances
            var childFormats = data.EnumChildFormats().Where(e => !e.HasFlag(EUnifiedFormatFlag.Switch)).Select(e => e.GetTitle()).ToArray();

            if (childFormats.Length > 0)
            {
                EditorGUILayout.BeginHorizontal();
                {
                    GUILayout.Space(indent);
                    state.adder_SelectionIndex = EditorGUILayout.Popup("Add a new", state.adder_SelectionIndex, childFormats);
                    if (GUILayout.Button("Create!", GUILayout.MaxHeight(14), GUILayout.MaxWidth(60)))
                    {
                        data.AddChildInstance(data.EnumChildFormats().ElementAt(state.adder_SelectionIndex).GetName());
                    }
                }
                EditorGUILayout.EndHorizontal();
            }

            // add new child switches
            foreach (var @switch in data.EnumChildFormats().Where(e => e.HasFlag(EUnifiedFormatFlag.Switch)).Select(e => e).ToArray())
            {
                var instance = data.GetChildInstances(@switch.GetName()).FirstOrDefault();

                if (instance != null)
                {
                    if (!Toggle(@switch.GetTitle(), true, indent))
                        data.RemoveChildInstance(instance);
                    else
                        ConfigureInternal(instance, indent + 20, true);
                }
                else 
                {
                    if (Toggle(@switch.GetTitle(), false, indent))
                        data.AddChildInstance(@switch.GetName());
                }
            }

            // show properties
            foreach (var prop in data.EnumProperties())
            {
                switch (prop.GetType())
                {
                    case EPropertyType.Matrix:
                        {
                            var iProp = prop as UnifiedMatrixProperty;
                            Matrix4x4 mat = iProp.GetMatrix();
                            EditorGUILayout.BeginHorizontal();
                            {
                                mat.SetColumn(0, EditorGUILayout.Vector3Field(iProp.GetTitle(), mat.GetColumn(0)));
                                mat.SetColumn(1, EditorGUILayout.Vector3Field("", mat.GetColumn(1)));
                                mat.SetColumn(2, EditorGUILayout.Vector3Field("", mat.GetColumn(2)));
                                mat.SetColumn(3, EditorGUILayout.Vector3Field("", mat.GetColumn(3)));
                            }
                            EditorGUILayout.EndHorizontal();
                            iProp.SetMatrix(mat);
                        }
                        continue;

                    case EPropertyType.Texture2D:
                        {
                            var iProp = prop as UnifiedTexture2DProperty;

                            EditorGUILayout.BeginHorizontal();
                            {
                                GUILayout.Space(indent - 3);

                                EditorGUILayout.BeginVertical();
                                {
                                    EditorGUILayout.LabelField(iProp.GetTitle());
                                    EditorGUILayout.BeginHorizontal();
                                    GUILayout.Space(20);
                                    var offset = EditorGUILayout.Vector2Field("UV Offset:", new Vector2(iProp.UOffset, iProp.VOffset));
                                    EditorGUILayout.EndHorizontal();
                                    EditorGUILayout.BeginHorizontal();
                                    GUILayout.Space(20);
                                    var scale = EditorGUILayout.Vector2Field("UV Scale:", new Vector2(iProp.UScale, iProp.VScale));
                                    EditorGUILayout.EndHorizontal();

                                    iProp.UOffset = offset.x;
                                    iProp.VOffset = offset.y;
                                    iProp.UScale = scale.x;
                                    iProp.VScale = scale.y;
                                }
                                EditorGUILayout.EndVertical();

                                iProp.SetTexture2D((Texture2D)EditorGUILayout.ObjectField(iProp.GetTexture2D(), typeof(Texture2D), false, GUILayout.Width(100), GUILayout.Height(100)));
                            }
                            EditorGUILayout.EndHorizontal();

                        }
                        break;
                }

                // show properties with only one GUI line
                EditorGUILayout.BeginHorizontal();
                {
                    GUILayout.Space(indent);
                    switch (prop.GetType())
                    {
                        case EPropertyType.Integer:
                            {
                                var iProp = prop as UnifiedIntegerProperty;
                                iProp.SetInteger(EditorGUILayout.IntSlider(iProp.GetTitle(), (int)iProp.GetInteger(), (int)iProp.Min, (int)iProp.Max));
                            }
                            break;

                        case EPropertyType.Float:
                            {
                                var iProp = prop as UnifiedFloatProperty;
                                iProp.SetSingle(EditorGUILayout.Slider(iProp.GetTitle(), iProp.GetSingle(), iProp.Min, iProp.Max));
                            }
                            break;

                        case EPropertyType.Boolean:
                            {
                                var iProp = prop as UnifiedBooleanProperty;
                                iProp.SetBoolean(Toggle(iProp.GetTitle(), iProp.GetBoolean()));
                            }
                            break;

                        case EPropertyType.Color:
                            {
                                var iProp = prop as UnifiedColorProperty;
                                iProp.SetColor(EditorGUILayout.ColorField(iProp.GetTitle(), iProp.GetColor()));
                            }
                            break;

                        case EPropertyType.Vector3:
                            {
                                var iProp = prop as UnifiedVector3Property;
                                iProp.SetVector3(EditorGUILayout.Vector3Field(iProp.GetTitle(), iProp.GetVector3()));
                            }
                            break;

                        case EPropertyType.String:
                            {
                                var iProp = prop as UnifiedStringProperty;
                                iProp.SetString(EditorGUILayout.TextField(iProp.GetTitle(), iProp.GetString()));
                            }
                            break;
                    }
                }
                EditorGUILayout.EndHorizontal();
            }

            // enumerate child instances
            foreach (var instance in data.EnumChildInstances().Where(e => !e.HasFlag(EUnifiedFormatFlag.Switch)))
            {
                var childState = GetState(instance);

                EditorGUILayout.BeginHorizontal();
                {
                    GUILayout.Space(indent);
                    instance.IsExpanded = EditorGUILayout.Foldout(instance.IsExpanded, instance.GetTitle());
                    GUILayout.FlexibleSpace();
                    if (GUILayout.Button("Remove", GUILayout.MaxHeight(14), GUILayout.MaxWidth(60)))
                        data.RemoveChildInstance(instance);
                }
                EditorGUILayout.EndHorizontal();
                EditorGUILayout.Space();

                if (instance.IsExpanded)
                {
                    ConfigureInternal(instance, indent + 20);
                }
            }

            return data;
        }
    }
}
