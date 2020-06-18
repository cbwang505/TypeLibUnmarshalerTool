using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ComProfile
{
	class Program
	{
		public static void ProcessSub(Microsoft.Win32.RegistryKey subkey, List<string> ls)
		{
			string[] lsName = subkey.GetSubKeyNames();
			foreach (string typelib in lsName)
			{
				if (typelib.ToLower() == "win32" || typelib.ToLower() == "win64")
				{
					Microsoft.Win32.RegistryKey typelibKey = subkey.OpenSubKey(typelib);
					object pth= typelibKey.GetValue(null);
					if (pth != null)
					{
						ls.Add(pth.ToString());
					}
					
				}
				else
				{
					Microsoft.Win32.RegistryKey typelibKey = subkey.OpenSubKey(typelib);

					  ProcessSub(typelibKey, ls);
				}
				
			}

			
		}

		static void Main(string[] args)
		{
			Console.Write(System.IO.File.Exists(@"C:\Windows\System32\icsvc.dll"));
			return;
			Microsoft.Win32.RegistryKey subkey = Microsoft.Win32.Registry.ClassesRoot.OpenSubKey("TypeLib");
			List<string> ls=new List<string>();
			ProcessSub(subkey, ls);

			foreach (string ph in ls.Distinct().Where(h=>!string.IsNullOrEmpty(h)))
			{
				string ret = Environment.ExpandEnvironmentVariables(ph);
				if (!System.IO.File.Exists(ret))
				{
					Console.Write(ret+"\r\n");
				}
			}
			

		}
	}
}
