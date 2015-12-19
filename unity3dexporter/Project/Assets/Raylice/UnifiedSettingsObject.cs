
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
using UnityEngine;

public abstract class UnifiedSettingsObject : MonoBehaviour
{
    [Serializable]
    private class TextureEntry
    {
        [SerializeField]
        public Texture2D texture;
        [SerializeField]
        public string property;
    }

    [NonSerialized] 
    private UnifiedSettings settings = null;
    [SerializeField] 
    private string settingsPersistent = null;
    [SerializeField]
    private TextureEntry[] textures = null;

    public abstract String FormatPath { get; }

    public UnifiedSettings Settings
    {
        get
        {
            if (settings != null)
                return settings;

            if (settingsPersistent != null)
            {
                settings = UnifiedSettings.TryFromString(settingsPersistent);

                if (settings == null)
                {
                    settingsPersistent = null;
                    textures = null;
                }
                else if (textures != null)
                {
                    // reassign textures
                    foreach(var e in textures)
                    {
                        try
                        {
                            settings.SetTexture2D(e.property, e.texture);
                        }
                        catch
                        {
                        }
                    }
                }
            }

            return settings;
        }

        set
        {
            settings = value;

            if (settings == null)
            {
                settingsPersistent = null;
                textures = null;
            }
            else
            {
                settingsPersistent = settings.SaveToString();

                // save texture references
                textures = settings.EnumTextures().Select(
                    e => new TextureEntry()
                             {
                                 texture = e.GetTexture2D(),
                                 property = e.GetPropertyPath()
                             }).ToArray();

                if (textures.Length == 0)
                    textures = null;
            }
        }
    }
}