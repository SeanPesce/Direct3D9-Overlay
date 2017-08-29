# Direct3D9 Overlay  

**Author: Sean Pesce**  


## Overview:  

A Direct3D9 Wrapper DLL/classes with a built-in overlay framework that implements an in-game console and text feed for printing on-screen messages in DirectX9 programs. Plugins are also supported for extending the features/functionality of the overlay.  

## Media:  

## [Showcase video (Early build)](https://www.youtube.com/watch?v=F2FiOhFi0pw)  

![image](http://i.imgur.com/9DH8LWB.jpg)  
![image](http://i.imgur.com/DAZGHDi.png)  
<sup>Dark Souls: Prepare to Die Edition</sup>  

![image](http://i.imgur.com/EVVCn05.jpg)  
![image](http://i.imgur.com/z1l4jYB.png)  
<sup>Deus Ex: Human Revolution</sup>  

![image](http://i.imgur.com/Z89F8DR.jpg)  
<sup>The Elder Scrolls IV: Oblivion</sup>  

## Plugins:  

To utilize various features of the overlay from an external DLL, the external DLL must be a plugin for this project which imports/exports various functions. See the [ExampleOverlayPlugin](https://github.com/SeanPesce/Direct3D9-Overlay/tree/master/ExampleOverlayPlugin) subproject for reference.  

## Credits:  

*  **Michael Koch** for his [DirectX9 Proxy DLL example](http://www.codeguru.com/cpp/g-m/directx/directx8/article.php/c11453/Intercept-Calls-to-DirectX-with-a-Proxy-DLL.htm), which I relied on heavily in the early stages of this project  
*  **Microsoft** for the [DirectX SDK](https://www.microsoft.com/en-us/download/details.aspx?id=8109) and [Detours library](https://www.microsoft.com/en-us/research/project/detours/)  
*  **[SeQan](https://www.seqan.de/)**, a library used in this project for more efficient string searches  
*  **Atom0s**, **Topblast**, and **Renkokuken** (Sakuri) from [UnknownCheats.me](https://www.unknowncheats.me) for the [modified CD3DFont class](https://www.unknowncheats.me/forum/d3d-tutorials-and-source/74839-modified-cd3dfont-d3d9-shadows-light-effect.html), which is much more efficient (performance-wise) than Direct3D9's built-in ID3DXFont class.  
*  **[Mumble](https://wiki.mumble.info/wiki/Main_Page)** VoIP [overlay source code](https://github.com/mumble-voip/mumble), which I used as a [reference](https://github.com/mumble-voip/mumble/blob/73fe4578bc01b0ef8e8742d7ce5d172b9b9c0f5b/overlay/d3d9.cpp) to create a suitable render state block when drawing my overlay  


## Disclaimer:  

This overlay is meant for **single-player games only**. Use of this overlay in multiplayer games **may trigger anti-cheat software and result in a ban**.  
