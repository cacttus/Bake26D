#Bake26

Blender plugin for baking 2.6D sprites

Test app

# dependencies
build-essential
mono-build
libglew-dev -y
libglfw3-dev -y

Notes

The test was supposed to be in C# but tried in C++ just to get done faster but still needs to be in c# anyway,

Mono does not support loading a c++ compiled so/dll from .net, you have to go the other way around and load C# mono runtime into c++. 

