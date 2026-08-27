using System;

namespace Lyra.LitJson.Extensions {

    /// <summary>
    /// ��չ����
    /// </summary>
	public static class Extensions {

		public static void WriteProperty(this JsonWriter w,string name,long value){
			w.WritePropertyName(name);
			w.Write(value);
		}
		
		public static void WriteProperty(this JsonWriter w,string name,string value){
			w.WritePropertyName(name);
			w.Write(value);
		}
		
		public static void WriteProperty(this JsonWriter w,string name,bool value){
			w.WritePropertyName(name);
			w.Write(value);
		}
		
		public static void WriteProperty(this JsonWriter w,string name,double value){
			w.WritePropertyName(name);
			w.Write(value);
		}

	}

    /// <summary>
    /// �������л��ı�ǩ
    /// </summary>
    [AttributeUsage(AttributeTargets.Field | AttributeTargets.Property, AllowMultiple = false)]
    public sealed class JsonIgnore : Attribute
    {
	    public static readonly Type Type = typeof(JsonIgnore);
    }
    
    /// <summary>
    /// ����json�ڵ��д����
    /// </summary>
    [AttributeUsage(AttributeTargets.Field | AttributeTargets.Property, AllowMultiple = false)]
    public sealed class JsonNode : Attribute
    {
	    public string Name;
	    public JsonNode(string name)
	    {
		    Name = name;
	    }
	    public static readonly Type Type = typeof(JsonNode);
    }
}