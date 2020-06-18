TypeLib反序列化提权利用工具

#### 介绍

>漏洞利用工具测试方式如下,需要管理员运行

1.只适用Windows7系统直接运行,无参数,替换默认Typelib

MyComEop.exe

2.替换指定接口TypeLIb文件路径的Com组件TypeLIb,比如C:\xxx.dll

MyComEop.exe [u] [TypeLib_Path]

3.替换指定接口IID的Com组件TypeLIb,比如 {55e3ea25-55cb-4650-8887-18e8d30bb4bc}

MyComEop.exe [u] [IID_Interface]

4.高级模式接口IID=[IID_Interface],接口名称=[InterfaceName],接口的TypeLib_GUID=[TypeLib_GUID_Interface],接口TypeLIb文件路径=[TypeLib_Path]

MyComEop.exe [u] [IID_Interface] [InterfaceName] [TypeLib_GUID_Interface] [TypeLib_Path] [Disable_Redirection]

5.不替换文件,仅测试指定接口IID的Com组件TypeLIb利用,比如 {55e3ea25-55cb-4650-8887-18e8d30bb4bc}

MyComEop.exe [t] [IID_Interface]
