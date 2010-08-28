using System;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.Xml;
using System.Xml.Serialization;
using System.Diagnostics;
using System.Reflection;

namespace Zelous
{
    // Clients must implement this interface and register themselves on the manager
    public interface ISerializationClient
    {
        void OnSerialize(Serializer serializer, ref object saveData);
    }

    // The manager
    public class SerializationMgr
    {
        // Object that gets serialized (internal)
        [XmlRootAttribute(Namespace = "Root")]
        public class SaveDataRoot
        {
            public object[] mSaveDataObjects;
        }

        private class Serializables
        {
            public List<ISerializationClient> mClients = new List<ISerializationClient>();
            public List<Type> mSaveDataTypes = new List<Type>();
        }
        Serializables mSerializables = new Serializables();

        // Clients must call this once (before Save/Load is ever called)
        public void RegisterSerializable(ISerializationClient client, Type saveDataType)
        {
            mSerializables.mClients.Add(client);
            mSerializables.mSaveDataTypes.Add(saveDataType);
        }

        private XmlSerializer CreateSerializer()
        {
            return new XmlSerializer(typeof(SaveDataRoot), mSerializables.mSaveDataTypes.ToArray());
        }

        public void Save(string filePath)
        {
            // Get each client to save their root, and collect them into a list
            List<Object> allSaveDataObjects = new List<object>();

            for (int i = 0; i < mSerializables.mClients.Count; ++i)
            {
                // Construct a default client of the client root object to pass to Serialize()
                object saveData = mSerializables.mSaveDataTypes[i].GetConstructor(Type.EmptyTypes).Invoke(null);
                
                Serializer serializer = new Serializer(true);
                mSerializables.mClients[i].OnSerialize(serializer, ref saveData);
                allSaveDataObjects.Add(saveData);
            }

            // Create a writer
            XmlWriterSettings xmlWriterSettings = new XmlWriterSettings();
            xmlWriterSettings.Indent = true;
            xmlWriterSettings.IndentChars = "    ";
            xmlWriterSettings.NewLineOnAttributes = true;
            
            XmlWriter xmlWriter = XmlWriter.Create(filePath, xmlWriterSettings);

            // Create and populate root object
            SaveDataRoot root = new SaveDataRoot();
            root.mSaveDataObjects = allSaveDataObjects.ToArray();

            // Save out xml
            XmlSerializer xmlSerializer = CreateSerializer();
            xmlSerializer.Serialize(xmlWriter, root);

            xmlWriter.Close();
        }

        public void Load(string filePath)
        {
            // If file doesn't exist, then we create a new one on the fly, then save current root, and reload them
            if (!File.Exists(filePath))
            {
                Save(filePath);
            }

            // Create reader and load data from xml
            XmlReader xmlReader = XmlReader.Create(filePath);
            XmlSerializer xmlSerializer = CreateSerializer();

            SaveDataRoot root = (SaveDataRoot)xmlSerializer.Deserialize(xmlReader);
            xmlReader.Close();

            // Call back clients with their serialized data
            for (int i = 0; i < mSerializables.mClients.Count; ++i)
            {
                Serializer serializer = new Serializer(false);
                mSerializables.mClients[i].OnSerialize(serializer, ref root.mSaveDataObjects[i]);
            }
        }
    }

    // Passed into OnSerialize() to perform member copying
    public class Serializer
    {
        private bool mIsSaving;
        public Serializer(bool isSaving)
        {
            mIsSaving = isSaving;
        }

        public bool IsSaving { get { return mIsSaving; } }
        public bool IsLoading { get { return !IsSaving; } }

        public void Assign<T>(ref T saveDataValue, ref T value)
        {
            if (mIsSaving)
            {
                saveDataValue = value;
            }
            else
            {
                value = saveDataValue;
            }
        }

        public void AssignProperty<T, U>(ref T saveDataValue, string propertyName, U propertyOwner)
            where U : class
        {
            PropertyInfo propInfo = typeof(U).GetProperty(propertyName);

            if (mIsSaving)
            {
                saveDataValue = (T)propInfo.GetGetMethod().Invoke(propertyOwner, null);
            }
            else
            {
                propInfo.GetSetMethod().Invoke(propertyOwner, new object[] { saveDataValue });
            }
        }
    }
}
