## Call ML build command

```lua
project "MyProject"
    kind "ConsoleApp"
    language "C++"

    files { "main.cpp", "*.asm" } -- Include all .asm files in the project directory

    filter "files:**.asm"
        buildcommands { "ml /c /Fo$(OutDir)/%(Filename).obj %(FullPath)" }
        buildoutputs { "$(OutDir)/%(Filename).obj" }
```

## MASM build action

```lua
project "MyProject"
    kind "ConsoleApp"
    language "C++"

    files { "main.cpp", "*.asm" } -- Include all .asm files in the project directory

    filter "files:**.asm"
        buildaction "Masm"
```
[Premake5 Build Actions](https://premake.github.io/docs/buildaction/)

## About ML

Microsoft Macro Assembler (MASM) is an x86 assembler that uses the Intel syntax for the x86 architecture. It is a part of the Microsoft Visual Studio suite of tools, but can also be installed separately.

Make sure these are in your PATH:
```
C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\<latest>\bin\Hostx64\x86\ml.exe
C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\<latest>\bin\Hostx64\x64\ml64.exe
```

[ML CLI Reference](https://learn.microsoft.com/en-us/cpp/assembler/masm/ml-and-ml64-command-line-reference?view=msvc-170)