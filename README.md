# Working Joe's Graphics Improvements and Fixes

The Working Joe's Graphics Improvements and Fixes mod aims to improve or fix some graphical aspects of the Alien: Isolation game. The current alpha version has the following features:

- Remove pop-in of objects and disable LOD.
- Fix high resolution (4k+) rendering.

Both features can be installed seperately from each other and are compatible with both the flatscreen version and the MotherVR mod. The *Image Comparison* folder showcases examples of the mod's effects. Impact on performance should be minimal for modern hardware.

**Version History**
- v0.1.0: Alpha version.

## Remove pop-in of objects and disable LOD.

In the vanilla game some objects are only rendered when the player is within a certain distance of them even though they are not obscured for the player. This is not very immersive and especially in VR quite noticable. This mod removes the relevant distance check so that all objects that should be visible are rendered, no matter the distance. Additionally LOD is disabled, all objects are immediately rendered with maximum detail. 

**Installation**

Copy *XINPUT1_3.dll* from the zip folder to the Alien: Isolation game folder. Alien: Isolation will automatically load the dll on startup, no further configuration is needed.

## Fix high resolution (4k+) rendering

The original game was designed and optimized to render up to 4k without issues, going beyond that can cause visual glitches. This problem arises because some shader calculations assume a fixed resolution cap. This mod includes a tool that patches the game shaders to account for the actual resolution during calculations, making them resolution-independent.

A special mention for [CathodeLib](https://github.com/OpenCAGE/CathodeLib). This tool uses the nuget package for unpacking and repacking the .PAK files.

**Installation**

Copy *AIShadersPatcher.exe* from the zip folder to the Alien: Isolation game folder. The tool requires a shader decompiler (*cmd_Decompiler.exe*) and compiler (*fxc.exe*) to work. These must also be placed in the Alien: Isolation game folder before running the tool.

- Decompiler: download from [GitHub](https://github.com/etnlGD/HLSLDecompiler/releases).
- Compiler: *fxc.exe* is part of the [Windows SDK](https://developer.microsoft.com/en-us/windows/downloads/windows-sdk/). Typical location after installing the SDK is *(SDK root)\bin\\(version)\x86*.

The final folder structure should be:
```plaintext
Alien Isolation/
├── DATA/
├── ...
├── AI.exe
├── AIShadersPatcher.exe
├── cmd_Decompiler.exe
├── fxc.exe
├── PatchShaders.bat (optional)
└── RestoreShaders.bat (optional)
```
Run the patcher via the command "AIShadersPatcher.exe --P" (or use the included PatchShaders.bat) and wait for it to finish (can take some time). A backup of the original PAK files is created (extension PAK.BAK). After patching, run the game as usual, the patched shaders are automatically loaded. If the original shaders need to be restored, run the command "AIShadersPatcher.exe --R" (or use the included RestoreShaders.bat). To see all available commands, run "AIShadersPatcher.exe -h".

<p align="center">This mod is neither affiliated with nor endorsed by Creative Assembly or SEGA.</p>

