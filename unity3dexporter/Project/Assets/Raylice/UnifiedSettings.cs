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
using UnityEngine;

namespace Raylice
{

    public enum EPropertyType
    {
        Integer,
        Float,
        Texture2D,
        String,
        Boolean,
        Vector3,
        Color,
        Matrix,
    };

    public abstract class UnifiedProperty
    {
        protected bool hasValue;
        private EPropertyType type;
        private string title;
        private string description;
        internal string name;
        internal UnifiedSettings parent;

        public static void Skip(BinaryReader reader)
        {
            var type = (EPropertyType)reader.ReadInt32();
            reader.BaseStream.Position -= 4;

            switch (type)
            {
                case EPropertyType.Integer: new UnifiedIntegerProperty().DeserializeData(reader); break;
                case EPropertyType.Float: new UnifiedFloatProperty().DeserializeData(reader); break;
                case EPropertyType.Texture2D: new UnifiedTexture2DProperty().DeserializeData(reader); break;
                case EPropertyType.String: new UnifiedStringProperty().DeserializeData(reader); break;
                case EPropertyType.Boolean: new UnifiedBooleanProperty().DeserializeData(reader); break;
                case EPropertyType.Vector3: new UnifiedVector3Property("", new Vector3()).DeserializeData(reader); break;
                case EPropertyType.Matrix: new UnifiedMatrixProperty().DeserializeData(reader); break;
                case EPropertyType.Color: new UnifiedColorProperty().DeserializeData(reader); break;
            }
        }

        protected abstract void DeserializeFormatInternal(BinaryReader reader);


        public UnifiedProperty(EPropertyType type, string title, string description = "")
        {
            this.hasValue = false;
            this.type = type;
            this.title = title;
            this.description = description;
        }

        public EPropertyType GetType()
        {
            return type;
        }

        public UnifiedSettings GetParent()
        {
            return parent;
        }

        public string GetName()
        {
            return name;
        }

        public string GetPropertyPath()
        {
            UnifiedSettings settings = parent;
            String result = name;
            while (settings.GetParent() != null)
            {
                var list = settings.GetParent().GetChildInstances(settings.GetName()).ToList();
                result = settings.GetName() + "$" + list.IndexOf(settings) + "/" + result;
                settings = settings.GetParent();
            } 

            //Debug.Log(result);

            return result;
        }

        public string GetTitle()
        {
            return title; 
        }

        public string GetDescription()
        {
            return description;
        }

        public virtual string GetString()
        {
            throw new InvalidCastException("This property is not a string!");
        }

        public virtual void SetString(string value)
        {
            throw new InvalidCastException("This property is not a string!");
        }

        public virtual Int64 GetInteger()
        {
            throw new InvalidCastException("This property is not an integer!");
        }

        public virtual void SetInteger(Int64 value)
        {
            throw new InvalidCastException("This property is not an integer!");
        }

        public virtual float GetSingle()
        {
            throw new InvalidCastException("This property is not a float!");
        }

        public virtual void SetSingle(float value)
        {
            throw new InvalidCastException("This property is not a float!");
        }

        public virtual bool GetBoolean()
        {
            throw new InvalidCastException("This property is not a bool!");
        }

        public virtual void SetBoolean(bool value)
        {
            throw new InvalidCastException("This property is not a bool!");
        }

        public virtual Color GetColor()
        {
            throw new InvalidCastException("This property is not a color!");
        }

        public virtual void SetColor(Color value)
        {
            throw new InvalidCastException("This property is not a color!");
        }

        public virtual Vector3 GetVector3()
        {
            throw new InvalidCastException("This property is not a vector!");
        }

        public virtual void SetVector3(Vector3 value)
        {
            throw new InvalidCastException("This property is not a vector!");
        }

        public virtual Matrix4x4 GetMatrix()
        {
            throw new InvalidCastException("This property is not a matrix!");
        }

        public virtual void SetMatrix(Matrix4x4 value)
        {
            throw new InvalidCastException("This property is not a matrix!");
        }

        public virtual Texture2D GetTexture2D()
        {
            throw new InvalidCastException("This property is not a 2D texture!");
        }

        public virtual void SetTexture2D(Texture2D value)
        {
            throw new InvalidCastException("This property is not a 2D texture!");
        }

        public virtual void SerializeFormat(BinaryWriter writer)
        {
            writer.Write((Int32)(int)type);
            writer.WriteNativeString(title);
            writer.WriteNativeString(description);
        }

        public static UnifiedProperty DeserializeFormat(BinaryReader reader)
        {
            UnifiedProperty res;
            var type = (EPropertyType)reader.ReadInt32();

            switch (type)
            {
                case EPropertyType.Integer:
                    res = new UnifiedIntegerProperty();
                    break;
                case EPropertyType.Float:
                    res = new UnifiedFloatProperty();
                    break;
                case EPropertyType.Texture2D:
                    res = new UnifiedTexture2DProperty();
                    break;
                case EPropertyType.String:
                    res = new UnifiedStringProperty();
                    break;
                case EPropertyType.Boolean:
                    res = new UnifiedBooleanProperty();
                    break;
                case EPropertyType.Vector3:
                    res = new UnifiedVector3Property("", new Vector3());
                    break;
                case EPropertyType.Matrix:
                    res = new UnifiedMatrixProperty();
                    break;
                case EPropertyType.Color:
                    res = new UnifiedColorProperty();
                    break;
                default:
                    throw new ArgumentException("Unknown property format!");
            }

            // invoke type specific deserialization
            res.title = reader.ReadNativeString();
            res.description = reader.ReadNativeString();
            res.DeserializeFormatInternal(reader);

            return res;
        }

        public virtual void SerializeData(BinaryWriter writer)
        {
            writer.Write((Int32)(int)type);
            writer.Write((Boolean)hasValue);
        }

        public virtual void DeserializeData(BinaryReader reader)
        {
            if ((EPropertyType)reader.ReadInt32() != type)
                throw new ArgumentException("Deserialized property does not match type!");

            hasValue = reader.ReadBoolean();
        }

        public virtual void ClearData()
        {
            hasValue = false;
        }
    }

    public class UnifiedIntegerProperty : UnifiedProperty
    {
        private Int64 integerMin;
        private Int64 integerMax;
        private Int64 integerDefault;
        private Int64 integer;

        protected override void DeserializeFormatInternal(BinaryReader reader)
        {
            integerDefault = reader.ReadInt64();
            integerMin = reader.ReadInt64();
            integerMax = reader.ReadInt64();
        }

        public UnifiedIntegerProperty(
            string title = "",
            Int64 defaultValue = 0,
            Int64 min = Int64.MinValue,
            Int64 max = Int64.MaxValue,
            string description = "")
            :
                base(EPropertyType.Integer, title, description)
        {
            this.integerMin = min;
            this.integerMax = Math.Max(min, max);
            this.integerDefault = Math.Max(integerMin, Math.Min(defaultValue, integerMax));
            this.integer = integerDefault;
        }

        public Int64 Min { get { return integerMin; } }
        public Int64 Max { get { return integerMax; } }

        public override Int64 GetInteger()
        {
            return hasValue ? integer : integerDefault;
        }

        public override void SetInteger(Int64 value)
        {
            hasValue = true;
            integer = Math.Max(integerMin, Math.Min(value, integerMax));
        }

        public override void SerializeFormat(BinaryWriter writer)
        {
            base.SerializeFormat(writer);

            writer.Write((Int64)integerDefault);
            writer.Write((Int64)integerMin);
            writer.Write((Int64)integerMax);
        }

        public override void SerializeData(BinaryWriter writer)
        {
            base.SerializeData(writer);
            writer.Write((Int64)integer);
        }

        public override void DeserializeData(BinaryReader reader)
        {
            base.DeserializeData(reader);
            integer = reader.ReadInt64();
        }
    }

    public class UnifiedFloatProperty : UnifiedProperty
    {
        private float singleMin;
        private float singleMax;
        private float singleDefault;
        private float single;

        protected override void DeserializeFormatInternal(BinaryReader reader)
        {
            singleDefault = reader.ReadSingle();
            singleMin = reader.ReadSingle();
            singleMax = reader.ReadSingle();
        }

        public UnifiedFloatProperty(
            string title = "",
            float defaultValue = 0,
            float min = Single.NegativeInfinity,
            float max = Single.PositiveInfinity,
            string description = "")
            :
                base(EPropertyType.Float, title, description)
        {
            this.singleMin = min;
            this.singleMax = Math.Max(min, max);
            this.singleDefault = Math.Max(singleMin, Math.Min(defaultValue, singleMax));
            this.single = singleDefault;
        }

        public float Min { get { return singleMin; } }
        public float Max { get { return singleMax; } }

        public override float GetSingle()
        {
            return hasValue ? single : singleDefault;
        }

        public override void SetSingle(float value)
        {
            hasValue = true;
            single = Math.Max(singleMin, Math.Min(value, singleMax));
        }

        public override void SerializeFormat(BinaryWriter writer)
        {
            base.SerializeFormat(writer);

            writer.Write((Single)singleDefault);
            writer.Write((Single)singleMin);
            writer.Write((Single)singleMax);
        }

        public override void SerializeData(BinaryWriter writer)
        {
            base.SerializeData(writer);
            writer.Write((Single)single);
        }

        public override void DeserializeData(BinaryReader reader)
        {
            base.DeserializeData(reader);
            single = reader.ReadSingle();
        }
    }

    public class UnifiedStringProperty : UnifiedProperty
    {
        private string stringValue;
        private string stringDefault;

        protected override void DeserializeFormatInternal(BinaryReader reader)
        {
            stringDefault = reader.ReadNativeString();
        }

        public UnifiedStringProperty(string title = "", string stringDefault = "", string description = "")
            : base(EPropertyType.String, title, description)
        {

            this.stringDefault = stringDefault;
            this.stringValue = stringDefault;
        }

        public override string GetString()
        {
            return hasValue ? stringValue : stringDefault;
        }

        public override void SetString(string value)
        {
            hasValue = true;
            stringValue = value;
        }

        public override void SerializeFormat(BinaryWriter writer)
        {
            base.SerializeFormat(writer);
            writer.WriteNativeString(stringDefault);
        }

        public override void SerializeData(BinaryWriter writer)
        {
            base.SerializeData(writer);
            writer.WriteNativeString(stringValue);
        }

        public override void DeserializeData(BinaryReader reader)
        {
            base.DeserializeData(reader);
            stringValue = reader.ReadNativeString();
        }
    };

    public class UnifiedBooleanProperty : UnifiedProperty
    {
        private bool boolean;
        private bool booleanDefault;

        protected override void DeserializeFormatInternal(BinaryReader reader)
        {
            booleanDefault = reader.ReadBoolean();
        }

        public UnifiedBooleanProperty(string title = "", bool defaultValue = false, string description = "") :
            base(EPropertyType.Boolean, title, description)
        {
            booleanDefault = defaultValue;
            boolean = booleanDefault;
        }

        public override bool GetBoolean()
        {
            return hasValue ? boolean : booleanDefault;
        }

        public override void SetBoolean(bool value)
        {
            hasValue = true;
            boolean = value;
        }

        public override void SerializeFormat(BinaryWriter writer)
        {
            base.SerializeFormat(writer);
            writer.Write((Boolean)booleanDefault);
        }

        public override void SerializeData(BinaryWriter writer)
        {
            base.SerializeData(writer);
            writer.Write((Boolean)boolean);
        }

        public override void DeserializeData(BinaryReader reader)
        {
            base.DeserializeData(reader);
            boolean = reader.ReadBoolean();
        }
    };

    public class UnifiedVector3Property : UnifiedProperty
    {
        private Vector3 value;
        private Vector3 valueDefault;

        protected override void DeserializeFormatInternal(BinaryReader reader)
        {
            valueDefault = reader.ReadVector3();
        }

        public UnifiedVector3Property(string title, Vector3 defaultValue, string description = "") :
            base(EPropertyType.Vector3, title, description)
        {
            this.valueDefault = defaultValue;
            this.value = valueDefault;
        }

        public override Vector3 GetVector3()
        {
            return hasValue ? value : valueDefault;
        }

        public override void SetVector3(Vector3 value)
        {
            hasValue = true;
            this.value = value;
        }

        public override void SerializeFormat(BinaryWriter writer)
        {
            base.SerializeFormat(writer);
            writer.WriteVector3(valueDefault);
        }

        public override void SerializeData(BinaryWriter writer)
        {
            base.SerializeData(writer);
            writer.WriteVector3(value);
        }

        public override void DeserializeData(BinaryReader reader)
        {
            base.DeserializeData(reader);
            value = reader.ReadVector3();
        }
    }

    public class UnifiedColorProperty : UnifiedProperty
    {
        private Color value;
        private Color valueDefault;

        protected override void DeserializeFormatInternal(BinaryReader reader)
        {
            valueDefault = reader.ReadColor();
        }

        public UnifiedColorProperty(string title = "", Color defaultValue = new Color(), string description = "") :
            base(EPropertyType.Color, title, description)
        {
            this.valueDefault = defaultValue;
            this.value = valueDefault;
        }

        public override Color GetColor()
        {
            return hasValue ? value : valueDefault;
        }

        public override void SetColor(Color value)
        {
            hasValue = true;
            this.value = value;
        }

        public override void SerializeFormat(BinaryWriter writer)
        {
            base.SerializeFormat(writer);
            writer.WriteColor(valueDefault);
        }

        public override void SerializeData(BinaryWriter writer)
        {
            base.SerializeData(writer);
            writer.WriteColor(value);
        }

        public override void DeserializeData(BinaryReader reader)
        {
            base.DeserializeData(reader);
            value = reader.ReadColor();
        }
    }

    public class UnifiedMatrixProperty : UnifiedProperty
    {
        private Matrix4x4 value;
        private Matrix4x4 valueDefault;

        protected override void DeserializeFormatInternal(BinaryReader reader)
        {
            valueDefault = reader.ReadMatrix();
        }

        public UnifiedMatrixProperty(string title = "", Matrix4x4 defaultValue = new Matrix4x4(), string description = "") :
            base(EPropertyType.Matrix, title, description)
        {
            this.valueDefault = defaultValue;
            this.value = valueDefault;
        }

        public override Matrix4x4 GetMatrix()
        {
            return hasValue ? value : valueDefault;
        }

        public override void SetMatrix(Matrix4x4 value)
        {
            hasValue = true;
            this.value = value;
        }

        public override void SerializeFormat(BinaryWriter writer)
        {
            base.SerializeFormat(writer);
            writer.WriteMatrix(valueDefault);
        }

        public override void SerializeData(BinaryWriter writer)
        {
            base.SerializeData(writer);
            writer.WriteMatrix(value);
        }

        public override void DeserializeData(BinaryReader reader)
        {
            base.DeserializeData(reader);
            value = reader.ReadMatrix();
        }
    }

    public class UnifiedTexture2DProperty : UnifiedProperty
    {
        internal Int64 id = -1;
        private Texture2D texture;

        protected override void DeserializeFormatInternal(BinaryReader reader)
        {
        }

        public UnifiedTexture2DProperty(string title = "", string description = "") :
            base(EPropertyType.Texture2D, title, description)
        {
            UScale = 1;
            VScale = 1;
        }

        public float UOffset { get; set; }
        public float VOffset { get; set; }
        public float UScale { get; set; }
        public float VScale { get; set; }

        public override Texture2D GetTexture2D()
        {
            return texture;
        }

        public override void SetTexture2D(Texture2D value)
        {
            texture = value;
        }

        public override void SerializeFormat(BinaryWriter writer)
        {
            base.SerializeFormat(writer);
        }

        public override void SerializeData(BinaryWriter writer)
        {
            base.SerializeData(writer);
            writer.Write((Int64)id);
            writer.Write((Single)UOffset);
            writer.Write((Single)VOffset);
            writer.Write((Single)UScale);
            writer.Write((Single)VScale);
        }

        public override void DeserializeData(BinaryReader reader)
        {
            base.DeserializeData(reader);
            id = reader.ReadInt64();
            UOffset = reader.ReadSingle();
            VOffset = reader.ReadSingle();
            UScale = reader.ReadSingle();
            VScale = reader.ReadSingle();
        }
    }

    [Flags]
    public enum EUnifiedFormatFlag
    {
        None = 0,
        Switch = 1,
    }

    public class UnifiedSettings
    {
        private readonly SortedDictionary<string, UnifiedSettings> childFormats = new SortedDictionary<string, UnifiedSettings>();
        private readonly SortedDictionary<string, List<UnifiedSettings>> childInstances = new SortedDictionary<string, List<UnifiedSettings>>();
        private readonly SortedDictionary<string, UnifiedProperty> properties = new SortedDictionary<string, UnifiedProperty>();
        private string title;
        private string description;
        private string name;
        private UnifiedSettings parent;

        public EUnifiedFormatFlag Flags { get; set; }
        public bool IsExpanded { get; set; } 

        public UnifiedSettings GetParent()
        {
            return parent;
        }

        public string GetName()
        {
            return name;
        }

        private static void Skip(BinaryReader reader)
        {
            new UnifiedSettings().DeserializeData(reader);
        }

        public string GetDescription()
        {
            return description;
        }

        public string GetTitle()
        {
            return title;
        }

        public UnifiedSettings(string title = "", string description = "")
        {
            this.title = title;
            this.description = description;
        }

        private void AddProperty(string name, UnifiedProperty prop)
        {
            properties.Add(name, prop);
            prop.name = name;
            prop.parent = this;
        }

        public void AddIntegerProperty(string name, string title, Int64 defaultValue, Int64 min, Int64 max, string description = "")
        {
            AddProperty(name, new UnifiedIntegerProperty(title, defaultValue, min, max, description));
        }

        public void AddSingleProperty(string name, string title, float defaultValue, float min, float max, string description = "")
        {
            AddProperty(name, new UnifiedFloatProperty(title, defaultValue, min, max, description));
        }

        public void AddVector3Property(string name, string title, Vector3 defaultValue, string description = "")
        {
            AddProperty(name, new UnifiedVector3Property(title, defaultValue, description));
        }

        public void AddColorProperty(string name, string title, Color defaultValue, string description = "")
        {
            AddProperty(name, new UnifiedColorProperty(title, defaultValue, description));
        }

        public void AddMatrixProperty(string name, string title, Matrix4x4 defaultValue, string description = "")
        {
            AddProperty(name, new UnifiedMatrixProperty(title, defaultValue, description));
        }

        public void AddTexture2DProperty(string name, string title, string description = "")
        {
            AddProperty(name, new UnifiedTexture2DProperty(title, description));
        }

        public void AddBooleanProperty(string name, string title, bool defaultValue, string description = "")
        {
            AddProperty(name, new UnifiedBooleanProperty(title, defaultValue, description));
        }

        public void AddStringProperty(string name, string title, string defaultValue, string description = "")
        {
            AddProperty(name, new UnifiedStringProperty(title, defaultValue, description));
        }

        public UnifiedProperty TryGet(string name)
        {
            try
            {
                return Get(name);
            }
            catch (Exception)
            {
                return null;
            }
        }

        public UnifiedProperty Get(string name)
        {
            UnifiedProperty prop;

            if(name.Contains('/'))
            {
                UnifiedSettings current = this;
                String parsed = "";
                string[] splitted = name.Split('/');

                foreach(var _node in splitted.Take(splitted.Length - 1))
                {
                    String node = _node;
                    int index = 0;

                    if (String.IsNullOrEmpty(node))
                        continue;
                     
                    try
                    {
                        if (node.Contains('$'))
                        {
                            index = Int32.Parse(node.Split('$')[1]);
                            node = node.Split('$')[0];
                        }

                        current = current.GetChildInstances(node).ElementAt(index);
                        parsed += node + "/";
                    }
                    catch (Exception e)
                    {
                        throw new ArgumentException("Property path \"" + name + "\" is invalid after \"" + parsed + "\".", e);
                    }
                } 

                if (!current.properties.TryGetValue(splitted.Last(), out prop))
                    throw new ArgumentException("Unknown property \"" + splitted.Last() + "\" designated by property path \"" + name + "\" (parsed \"" + parsed + "\").");
            }
            else
            {
                if (!properties.TryGetValue(name, out prop))
                    throw new ArgumentException("Unknown property \"" + name + "\".");
            }

            return prop;   
        }

        public float GetSingle(string name)
        {
            return Get(name).GetSingle();
        }

        public Vector3 GetVector3(string name)
        {
            return Get(name).GetVector3();
        }

        public Matrix4x4 GetMatrix(string name)
        {
            return Get(name).GetMatrix();
        }

        public Color GetColor(string name)
        {
            return Get(name).GetColor();
        }

        public bool GetBoolean(string name)
        {
            return Get(name).GetBoolean();
        }

        public string GetString(string name)
        {
            return Get(name).GetString();
        }

        public Texture2D GetTexture2D(string name)
        {
            return Get(name).GetTexture2D();
        }



        public void SetTexture2D(string name, Texture2D value)
        {
            Get(name).SetTexture2D(value);
        }

        public void SetSingle(string name, float value)
        {
            Get(name).SetSingle(value);
        }

        public void SetVector3(string name, Vector3 value)
        {
            Get(name).SetVector3(value);
        }

        public void SetMatrix(string name, Matrix4x4 value)
        {
            Get(name).SetMatrix(value);
        }

        public void SetColor(string name, Color value)
        {
            Get(name).SetColor(value);
        }

        public void SetBoolean(string name, bool value)
        {
            Get(name).SetBoolean(value);
        }

        public void SetString(string name, string value)
        { 
            Get(name).SetString(value);
        }

        public bool HasProperty(String name)
        {
            return properties.ContainsKey(name);
        }

        public UnifiedSettings GetChildFormat(string name)
        {
            UnifiedSettings child;
            if (!childFormats.TryGetValue(name, out child))
                throw new ArgumentException("Unknown child \"" + name + "\".");

            return child;
        }

        public bool HasFlag(EUnifiedFormatFlag flag)
        {
            return (Flags & flag) != 0;
        }

        public IEnumerable<UnifiedProperty> EnumProperties()
        {
            return properties.Values;
        }

        public IEnumerable<UnifiedSettings> EnumChildFormats()
        {
            return childFormats.Values;
        }

        public IEnumerable<UnifiedSettings> EnumChildInstances()
        {
            var list = new List<UnifiedSettings>();

            return childInstances.Aggregate(
                list,
                (u, e) =>
                {
                    list.AddRange(e.Value);
                    return list;
                });

            return list;
        }

        public IEnumerable<UnifiedTexture2DProperty> EnumTextures()
        {
            var list = new List<UnifiedTexture2DProperty>();
            EnumTextures(list);
            return list;
        }

        private void EnumTextures(List<UnifiedTexture2DProperty> list)
        {
            foreach (var prop in properties.Values)
            {
                var tex = prop as UnifiedTexture2DProperty;
                if((tex == null) || (tex.GetTexture2D() == null))
                    continue;

                list.Add(tex);
            }

            foreach (var instance in childInstances)
            {
                foreach (var child in instance.Value)
                {
                    child.EnumTextures(list);
                }
            }
        }

        public UnifiedSettings AddChildFormat(
            string name,
            string title = "",
            string description = "")
        {
            var child = new UnifiedSettings(title, description);
            childFormats.Add(name, child);
            child.name = name;
            child.parent = this;
            return child;
        }

        public void RemoveChildInstance(UnifiedSettings instance)
        {
            foreach(var list in childInstances)
            {
                int pos = list.Value.IndexOf(instance);
                if (pos >= 0)
                {
                    list.Value.RemoveAt(pos);
                    return;
                }
            }
        }

        public UnifiedSettings AddChildInstance(string name)
        {
            UnifiedSettings childFmt;
            if (!childFormats.TryGetValue(name, out childFmt))
                throw new ArgumentException("A child format named \"" + name + "\" does not exist!");

            var child = childFmt.Clone();
            if (childInstances.ContainsKey(name))
                childInstances[name].Add(child);
            else
                childInstances.Add(name, new List<UnifiedSettings>(new[] { child }));

            return child;
        }

        public void ClearData() 
        {
            childInstances.Clear();
            foreach (var prop in properties.Values) prop.ClearData();
        }

        public IEnumerable<UnifiedSettings> GetChildInstances(string name)
        {
            List<UnifiedSettings> res = new List<UnifiedSettings>();
            foreach (var e in childInstances)
            {
                if (e.Key == name)
                    res.AddRange(e.Value);
            }

            return res;
        }

        public void SerializeFormat(BinaryWriter writer)
        {
            writer.WriteNativeString("RAYLICE - UNIFIED SETTINGS FORMAT");
            writer.Write((Int32)0x1000); // version

            writer.WriteNativeString(title);
            writer.WriteNativeString(description);

            writer.Write((Int64)Flags);

            // serialize property specifications
            writer.Write((Int32)properties.Count);
            foreach (var e in properties)
            {
                writer.WriteNativeString(e.Key);
                e.Value.SerializeFormat(writer);
            } 

            // serialize child specifications
            writer.Write((Int32)childFormats.Count);
            foreach (var e in childFormats)
            {
                writer.WriteNativeString(e.Key);
                e.Value.SerializeFormat(writer);
            }
        }

        public static UnifiedSettings DeserializeFormat(BinaryReader reader)
        {
            if (reader.ReadNativeString() != "RAYLICE - UNIFIED SETTINGS FORMAT")
                throw new ArgumentException("Format stream is corrupt!");

            if (reader.ReadInt32() != 0x1000)
                throw new ArgumentException("Unsupported version.");

            var title = reader.ReadNativeString();
            var description = reader.ReadNativeString();
            var res = new UnifiedSettings(title, description);

            res.Flags = (EUnifiedFormatFlag) reader.ReadInt64();
             
            // serialize property specifications
            int propCount = reader.ReadInt32();
            for (int i = 0; i < propCount; i++)
            {
                var name = reader.ReadNativeString();
                var format = UnifiedProperty.DeserializeFormat(reader);

                res.AddProperty(name, format);
            } 

            // serialize child specifications
            int childCount = reader.ReadInt32();
            for (int i = 0; i < childCount; i++)
            {
                var name = reader.ReadNativeString();
                var format = UnifiedSettings.DeserializeFormat(reader);

                res.childFormats.Add(name, format);
                format.name = name;
                format.parent = res;
            }

            return res;
        }

        public void SerializeData(BinaryWriter writer)
        {
            writer.WriteNativeString("RAYLICE - UNIFIED SETTINGS DATA");
            writer.Write((Int32)0x1000); // version

            writer.Write((Boolean)IsExpanded);

            // serialize property specifications
            writer.Write((Int32)properties.Count);
            foreach (var e in properties)
            {
                writer.WriteNativeString(e.Key);
                e.Value.SerializeData(writer);
            }

            // serialize child specifications
            writer.Write((Int32)childInstances.Sum(e => e.Value.Count));
            foreach (var e in childInstances)
            {
                foreach (var instance in e.Value)
                {
                    writer.WriteNativeString(e.Key);
                    instance.SerializeData(writer);
                }
            }
        }

        public void DeserializeData(BinaryReader reader)
        {
            childInstances.Clear();

            if (reader.ReadNativeString() != "RAYLICE - UNIFIED SETTINGS DATA")
                throw new ArgumentException("Data stream is corrupt!");

            if (reader.ReadInt32() != 0x1000)
                throw new ArgumentException("Unsupported version.");

            IsExpanded = reader.ReadBoolean();

            // deserialize property data
            int propCount = reader.ReadInt32();
            for (int i = 0; i < propCount; i++)
            {
                var name = reader.ReadNativeString();
                UnifiedProperty prop;

                if (!properties.TryGetValue(name, out prop))
                {
                    UnifiedProperty.Skip(reader);
                    continue; // ignore unknown properties
                }

                // update data in current model
                prop.DeserializeData(reader);
            }

            // deserialize child data
            int childCount = reader.ReadInt32();
            for (int i = 0; i < childCount; i++)
            {
                var name = reader.ReadNativeString();
                UnifiedSettings childFmt;

                if (!childFormats.TryGetValue(name, out childFmt))
                {
                    UnifiedSettings.Skip(reader);
                    continue; // ignore unknown children
                }

                // update data in current model
                var instance = childFmt.Clone();
                instance.DeserializeData(reader);

                if (childInstances.ContainsKey(name))
                    childInstances[name].Add(instance);
                else 
                    childInstances.Add(name, new List<UnifiedSettings>(new[] { instance }));

                instance.name = name;
                instance.parent = this;
            }
        }

        public UnifiedSettings Clone()
        {
            MemoryStream format = new MemoryStream();
            MemoryStream data = new MemoryStream();

            SerializeFormat(new BinaryWriter(format));
            SerializeData(new BinaryWriter(data));

            format.Position = 0;
            data.Position = 0;

            var result = UnifiedSettings.DeserializeFormat(new BinaryReader(format));
            result.DeserializeData(new BinaryReader(data));

            return result;
        }

        public static UnifiedSettings TryFromString(string binary)
        {
            try
            {
                MemoryStream stream = new MemoryStream(Convert.FromBase64String(binary));
                var reader = new BinaryReader(stream);
                var result = UnifiedSettings.DeserializeFormat(reader);
                result.DeserializeData(reader);
                return result;
            }
            catch
            {
                return null;
            }
        }

        public string SaveToString()
        { 
            MemoryStream stream = new MemoryStream();
            BinaryWriter writer = new BinaryWriter(stream);
            SerializeFormat(writer);
            SerializeData(writer);
            return Convert.ToBase64String(stream.ToArray());
        }
    }
}
