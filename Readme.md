# DirectX 9 Overlay  

**Author: Sean Pesce**  


## Overview  

A Direct3D9 Wrapper DLL/classes with a built-in overlay framework that implements an in-game CLI and text feed for printing on-screen messages in DirectX 9 programs. ~~[Plugins](https://github.com/SeanPesce/Direct3D9-Overlay#plugins) are also supported for extending the features/functionality of the overlay~~ (**Update:** Plugins are still supported, but [see below](https://github.com/SeanPesce/Direct3D9-Overlay#plugins)).  

## Media  

## [Showcase video (Early build)](https://www.youtube.com/watch?v=F2FiOhFi0pw)  

![image](http://i.imgur.com/9DH8LWB.jpg)  
![image](http://i.imgur.com/DAZGHDi.png)  
<sup>Dark Souls: Prepare to Die Edition</sup>  

![image](http://i.imgur.com/EVVCn05.jpg)  
![image](http://i.imgur.com/z1l4jYB.png)  
<sup>Deus Ex: Human Revolution</sup>  

![image](http://i.imgur.com/Z89F8DR.jpg)  
<sup>The Elder Scrolls IV: Oblivion</sup>  

## Plugins  

~~To utilize various features of the overlay from an external DLL, the external DLL must be a plugin for this project which imports/exports various functions. See the [ExampleOverlayPlugin](https://github.com/SeanPesce/Direct3D9-Overlay/tree/master/ExampleOverlayPlugin) subproject for reference.~~ **EDIT:** Plugin support has been vastly improved (most importantly much easier to implement), but the files in that subproject are extremely outdated. I'll update them eventually, but I'm not sure when I'll have the time to do it just yet. For an example of a working plugin, see the [Dark Souls Overhaul repository](https://github.com/metal-crow/Dark-Souls-1-Overhaul/tree/PtDE).  

## Acknowledgments  

*  **Michael Koch** for his [DirectX 9 Proxy DLL example](http://www.codeguru.com/cpp/g-m/directx/directx8/article.php/c11453/Intercept-Calls-to-DirectX-with-a-Proxy-DLL.htm), which I relied on heavily as a reference in the early stages of this project  
*  **Microsoft** for the [DirectX SDK](https://www.microsoft.com/en-us/download/details.aspx?id=8109) and [Detours library](https://www.microsoft.com/en-us/research/project/detours/)  
*  **Atom0s**, **Topblast**, and **Renkokuken** (Sakuri) from [UnknownCheats.me](https://www.unknowncheats.me) for the [modified CD3DFont class](https://www.unknowncheats.me/forum/d3d-tutorials-and-source/74839-modified-cd3dfont-d3d9-shadows-light-effect.html), which is much more efficient (performance-wise) than Direct3D9's built-in ID3DXFont class.  
*  **[Mumble](https://wiki.mumble.info/wiki/Main_Page)** VoIP [overlay source code](https://github.com/mumble-voip/mumble), which I used as a [reference](https://github.com/mumble-voip/mumble/blob/73fe4578bc01b0ef8e8742d7ce5d172b9b9c0f5b/overlay/d3d9.cpp) to create a suitable render state block when drawing my overlay  


## Built With  
 * [Visual Studio 2015](https://www.visualstudio.com/vs/older-downloads/)  
 * [June 2010 DirectX SDK](https://www.microsoft.com/en-us/download/details.aspx?id=8109)  
 * [MS Detours](https://www.microsoft.com/en-us/research/project/detours/)  
 * [SeqAn](https://www.seqan.de/)  


## Disclaimer  

This overlay is meant for **single-player games only**. Use of this overlay in multiplayer games **may trigger anti-cheat software and result in a ban**. Additionally, I **do not** create or support video game hacks that give the user an unfair advantage over other players. Any mods I make are for fun, replay value, or game fixes.  


## License  
[GNU General Public License v3.0](LICENSE)  


---------------------------------------------

For inquiries and/or information about me, visit my **[personal website](https://SeanPesce.github.io)**.  
