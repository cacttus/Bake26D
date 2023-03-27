using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Runtime.InteropServices;
using Loft;

namespace TestApp
{

  //i686-w64-mingw32-g++ -mdll -std=c++2a -fPIC -shared -I/usr/share/mingw-w64/include/ -I/usr/include/ -o ./bin/Debug/libtestapp.dll -static-libstdc++ -lGLEW -lOpenGL -lGLX -lGLU /usr/lib/x86_64-linux-gnu/libglfw.so.3.3 ./src/testapp.cpp
  //clang --target=windows-shared-x64
  //c++ -std=c++2a -fPIC -shared -o ./bin/Debug/libtestapp.dll -static-libstdc++ -lGLEW -lOpenGL -lGLX -lGLU /usr/lib/x86_64-linux-gnu/libglfw.so.3.3 ./src/testapp.cpp
  public class External
  {
    [DllImport("libtestapp.so", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
    public static extern void DoSomething();
    [DllImport("libtestapp.so", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi, EntryPoint = "main")]
    public static extern void main([In] int argc, [In] string[] argv);
    //[DllImport("LibName.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
    //public static extern void GetData([In, Out] [MarshalAs(UnmanagedType.LPArray)] byte[] data, uint length);
  }

  public class MainClass
  {
    public static void Main(string[] args)
    {
      try
      {
        System.Console.WriteLine("Hello world - calling the C++ function");

        try
        {
          var asmdir = System.IO.Path.GetDirectoryName(System.Reflection.Assembly.GetExecutingAssembly().Location);
          asmdir = System.IO.Path.Combine(asmdir, "libtestapp.so");
          var cpp_args = new string[] { asmdir };
        // Gu.InitGlobals();
        // var win = new MainWindow(
        //   new ivec2(Gu.EngineConfig.WindowInitX, Gu.EngineConfig.WindowInitY),
        //   new ivec2(Gu.EngineConfig.WindowInitW, Gu.EngineConfig.WindowInitH),
        //   new vec2(Gu.EngineConfig.WindowInitScaleW, Gu.EngineConfig.WindowInitScaleH)
        //   );
        // Gu.InitScripts();
        // Gu.Run();
          External.main(cpp_args.Length, cpp_args);//cpp_args);
        }
        catch(Exception ex)
        {
          System.Console.WriteLine("Exception:\n " + ex.ToString());
        }

      }
      catch (Exception ex)
      {
        string strex = Gu.GetAllException(ex);
        if (Gu.Log != null)
        {
          Gu.Log.Error("Fatal Error: " + strex);
        }
        else
        {
          Console.WriteLine(ex);
        }
        System.Environment.Exit(0);
      }
      finally
      {
        // Gu.Trap();
      }
    }
  }
}