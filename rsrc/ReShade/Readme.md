# ReShade DLL Wrapper Chaining Fix  

**Author: Sean Pesce**  


## Summary  
This is a fix for [ReShade](https://github.com/crosire/reshade) v3.0.8 to make it more compatible with my DirectX9 overlay (and other DirectX wrapper DLLs, such as [SweetFX](https://sfx.thelazy.net) or [PvP Watchdog](http://www.nexusmods.com/darksouls/mods/849) for Dark Souls: Prepare to Die Edition). ReShade wasn't working correctly when set as the d3d9Chain for my overlay because ReShade doesn't work correctly if you change the file name of the DLL (so if it's being used as a wrapper for d3d9.dll, ReShade MUST be named d3d9.dll). Additionally, ReShade doesn't have the functionality for loading proxy DLLs, so loading my overlay through ReShade isn't an option. In this directory are two different ways to fix these issues:  

* Move the ReShade files to their own subdirectory (see the "[Unmodified](https://github.com/SeanPesce/Direct3D9-Overlay/tree/master/rsrc/ReShade/Unmodified/GAME_DIRECTORY)" folder for an example of correct directory structure and config file setups) and load them using my overlay.  

* The first option works, but still doesn't fix the problem that ReShade doesn't support wrapper DLL chaining, which means the DLL chain stops with ReShade. If you want to be able to chain multiple DLL wrappers, I made a modified version of ReShade that supports DLL chaining (see the "[Modified](https://github.com/SeanPesce/Direct3D9-Overlay/tree/master/rsrc/ReShade/Modified%20(Supports%20DLL%20chaining)/GAME_DIRECTORY)" folder for an example of correct directory structure and config file setups, as well as the modified ReShade DLL file).  

________________________

If you have any questions, concerns, or suggestions, you can contact me at:  

* [Reddit](https://reddit.com/u/SeanPesce)  
* [GitHub](https://github.com/SeanPesce)  
* [Twitter](https://twitter.com/SeanPesce)  
* [YouTube](https://www.youtube.com/channel/UCgsMpXiR3PawqKM7MWLJGzQ)  
* [Discord](https://discordapp.com) (@SeanP#5604)  
