# JAEnhanced

### [Visit the JKHub Page to download](https://jkhub.org/files/file/2550-jedi-academy-enhanced)

NOTE: If you don't already have it and you're on Windows, you'll need the [Visual C++ 2015 redistributable](https://www.microsoft.com/en-gb/download/details.aspx?id=48145).

**Jedi Academy: Enhanced** is a mod for Jedi Academy Single Payer which adds RGB sabers, saber customization, holstering, new force powers, some extra character customization options using "head swapping" and optionally allows you to use AJL's SFX Sabers, and many more. Full feature list below.

It's based on OpenJK (so the code is released under the GPL and available at <https://github.com/redsaurus/OpenJK/tree/custom>) and a slightly modified version (for SP) of AJL's SFX Saber code. It also uses Open Jedi Project code for TrueView.

**Features and Commands**

-   **All OpenJK features and fixes**.
-   **RGB Sabers -** These can be set in the menus or by setting the sabercolor to a hex code in the console - for example "**sabercolor 1 xff0000**" will set the first lightsaber blade to be red. It is possible to set the sabercolors of NPCs by setting their sabercolor to a hex code in the .npc file. If you want to set one sabercolor for base and one for this mod, you can set the sabercolor of the NPC or lightsaber to the base value, and sabercolorRGB to the RGB value for this mod. Higher blade numbers are set with saberColorRGB2, saber2ColorRGB3, etc.
-   **SFX Sabers -** SFX Sabers can be enabled in the console by setting cg_SFXSabers to 1. This is on by default, allows for more vibrant and high quality saber blades.
-   **Ignition Flare -** A lightsaber ignition flare can be enabled in the console by setting **cg_ignitionFlare** to 1. A custom ignition flare can be specified for a lightsaber with "ignitionFlare <shader>" in the .sab file.
-   **Ignition twirl animation disable -** If that little twirl of the saber that you do in SP when activating it bothered you, you can now disable it to be more like MP with **g_noIgniteTwirl 1**.
-   **Disable idle animations** - Use the command **g_UseIdleAnims 0** to disable them. Very helpful when taking screenshots.
-   **Saber Holsters -** Lightsabers are now holstered on the belt when not in use. A tag_holsterorigin can be added to a hilt for better placement. Adding "holsterPlace <none/hips/back/lhip>" in the .sab file specifies where a hilt will be holstered.
-   **Headswapping -** Several new heads are available for the human male and human female species. You can add your own heads - see the .headswap files in the sp_custom.pk3 for examples. NPCs can have heads set using the playerHeadModel and customHeadSkin commands in their .npc file.
-   **RGB Character Colors -** Adds an RGB slider option to all player species.
-   **Better Entity Spawning -** The /spawn command now supports entity keys, e.g. "**spawn fx_runner fxFile the/file**".
-   **.eent files -** Maps now load entities from mapentities/mapname.eent in addition to loading them from the .bsp file.
-   **MP Movement** - Not identical to MP but close. Allows for bunny hopping and less "slide" effect when moving. **g_bunnyhopping 1**. Also in the menu.
-   **Extra Player Tints -** (Unused) Playermodels are able to have multiple tints. If you enter "newPlayerTint 0 R G B", any shader stages for the player with "rgbGen lightingDiffuseEntity 0" will be tinted to this color rather than the usual.
-   **Ghoul2 view models** First person view weapon models are now allowed to use .glm models using eezstreet's code.
-   **Detachable E-Web** The player can detach an E-Web from its mount by pressing the Use Force button whilst using it. While the E-Web is equipped, the player moves more slowly.
-   **More usable weapons** The tusken rifle and noghri stick are fully usable by the player. The DC-15A clone rifle (made by Pahricida), DC-15S clone blaster (Made by AshuraDX) and E-5 droid blaster (made by KhorneSyrup) have also been added. Only given via cheats right now.
    -   give weapon_tusken_rifle
    -   give weapon_noghri_stick
    -   give weapon_e5
    -   give weapon_dc15s
    -   give weapon_dc15a
-   **Saber throw is now a force power -** If you'd like to return it to the old function, you can simply bind both attack2 and throw to the same key or button like this: **bind Mouse2 "+alt_attack;+saber_throw"**
-   **New force powers -** Force Insanity, Destruction, Repulse and Stasis have been added. Force Repulse is gained automatically during the SP campaign. Bind the keys in the Controls menu. NPCs can use Destruction and Stasis. For faster force regeneration, **g_forceRegenTime** has been brought over from MP.
-   **First person lightsaber with TrueView -** As seen in Open Jedi Project and all the other mods that used it, TrueView shows the player model in first person view. You can turn it on for guns with **cg_trueguns** and turn it on for sabers with **cg_fpls** or through the menu. Change FOV with **cg_trueFOV**. Recommend set to 120 if using first person lightsaber.
-   **Radar -** The radar system from Siege in MP now works in SP. Giving NPCs and misc_radar_icon entities the icon key will set a custom icon. A 2D minimap is also loaded from minimaps/mapname.mmap.
-   **AI workshop -** Created by eezstreet to give more control over NPC AI. [See full thread here](https://jkhub.org/forums/topic/10194-ai-workshop/).
-   **Switch pistols -** Toggle between DL-44 and Bryar if added to inventory with pistol bind (+weapon_2)
-   **Saber ignition speed** - cg_ignitionSpeed scales saber ignition speed
-   **Click-drag to rotate player model in customization screen - **To help with seeing your character more easily instead of waiting on it to rotate around again.
-   **MP-style saber hilt list -** Lists lightsabers in the menu without the need for adding menu listings
-   **r_mode -2 is now default -** sets the game to the monitor's native resolution at launch. Change back to r_mode -1 to use windowed mode.
-   **Removed black bars in cutscenes •** this helps with widescreen resolutions not cutting off half of the scene.

**Optional features (separate PK3's):**

-   **Improved jedi_hm** DT's very nice improved Human Male jedi is included with Jedi Robe options with RGB tinting features.
-   **Build Your Own Lightsaber** - Now lightsabers can be customized like the player species. Example customizable hilts are included thanks to AshuraDX and Plasma.

**Unfinished features, only use if testing:**

-   **Katarn saber style -** A gun / saber stance. No animations yet, but you play around with it (with cheats enabled) by doing "**give weapon_bryar_pistol**" and then "**setsaberstyle katarn**" in the console.
-   **Z-6 rotary cannon -** Added slot for this weapon, but it has no model yet. **give weapon_z6**


## Based on OpenJK

OpenJK is an effort by the JACoders group to maintain and improve the game engines on which the Jedi Academy (JA) and Jedi Outcast (JO) games run on, while maintaining *full backwards compatibility* with the existing games. *This project does not attempt to rebalance or otherwise modify core gameplay*.

Our aims are to:
* Improve the stability of the engine by fixing bugs and improving performance.
* Provide a clean base from which new JO and JA code modifications can be made.
* Make available this engine to more operating systems. To date, we have ports on Linux and macOS.

Currently, the most stable portion of this project is the Jedi Academy multiplayer code, with the single player code in a reasonable state.

Rough support for Jedi Outcast single player is also available, however this should be considered heavily work in progress. This is not currently actively worked on or tested. OpenJK does not have Jedi Outcast multiplayer support.

Please use discretion when making issue requests on GitHub. The [JKHub sub-forum](http://jkhub.org/forum/51-discussion/) is a better place for support queries, discussions, and feature requests.

<a href="https://discord.gg/dPNCfeQ"><img src="https://img.shields.io/badge/discord-join-7289DA.svg?logo=discord&longCache=true&style=flat" /></a>
[![Forum](https://img.shields.io/badge/forum-JKHub.org%20OpenJK-brightgreen.svg)](http://jkhub.org/forum/51-discussion/)

[![Coverity Scan Build Status](https://scan.coverity.com/projects/1153/badge.svg)](https://scan.coverity.com/projects/1153)

## License

OpenJK is licensed under GPLv2 as free software. You are free to use, modify and redistribute OpenJK following the terms in LICENSE.txt.


## For players

To install OpenJK, you will first need Jedi Academy installed. If you don't already own the game you can buy it from online stores such as [Steam](http://store.steampowered.com/app/6020/), [Amazon](http://www.amazon.com/Star-Wars-Jedi-Knight-Academy-Pc/dp/B0000A2MCN) or [GOG](https://www.gog.com/game/star_wars_jedi_knight_jedi_academy).

Installing and running OpenJK:

1. [Download the latest build](http://builds.openjk.org) for your operating system.
2. Extract the contents of the file into the Jedi Academy `GameData/` folder. For Steam users, this will be in `<Steam Folder>/steamapps/common/Jedi Academy/GameData`.
3. Run `openjk.x86.exe` (Windows), `openjk.i386` (Linux 32-bit), `openjk.x86_64` (Linux 64-bit) or the `OpenJK` app bundle (macOS), depending on your operating system.


**Linux Instructions**

If you do not have a windows partition and need to download the game base.

1. Download  and Install SteamCMD [SteamCMD](https://developer.valvesoftware.com/wiki/SteamCMD#Linux) .
2. Set the download path using steamCMD, force_install_dir <path> .
3. Using SteamCMD Set the platform to windows to download any windows game on steam. @sSteamCmdForcePlatformType "windows"
4. Using SteamCMD download the game,  app_update 6020.
5. [Download the latest build](http://builds.openjk.org) for your operating system.
6. Extract the contents of the file into the Jedi Academy `GameData/` folder. For Steam users, this will be in `<Steam Folder>/steamapps/common/Jedi Academy/GameData`.


**macOS Instructions**

If you have the Mac App Store Version of Jedi Academy, follow these steps to get OpenJK runnning under macOS:

1. Install [Homebrew](http://brew.sh/) if you don't have it.
2. Open the Terminal app, and enter the command `brew install sdl2`.
3. Extract the contents of the OpenJK DMG ([Download the latest build](http://builds.openjk.org)) into the game directory `/Applications/Star Wars Jedi Knight: Jedi Academy.app/Contents/`
4. Run `OpenJK.app` or `OpenJK SP.app` 
5. Savegames, Config Files and Log Files are stored in `/Users/<USER>/Library/Application Support/OpenJK/`


## For Developers


### Building OpenJK

* [Compilation guide](https://github.com/JACoders/OpenJK/wiki/Compilation-guide)
* [Debugging guide](https://github.com/JACoders/OpenJK/wiki/Debugging)


### Contributing to OpenJK

* [Fork](https://github.com/JACoders/OpenJK/fork) the project on GitHub
* Create a new branch and make your changes
* Send a [pull request](https://help.github.com/articles/creating-a-pull-request) to upstream (JACoders/OpenJK)


### Using OpenJK as a base for a new mod

* [Fork](https://github.com/JACoders/OpenJK/fork) the project on GitHub
* Change the GAMEVERSION define in codemp/game/g_local.h from "OpenJK" to your project name
* If you make a nice change, please consider back-porting to upstream via pull request as described above. This is so everyone benefits without having to reinvent the wheel for every project.



## Maintainers of OpenJK (in alphabetical order)

* Ensiform
* Razish
* Xycaleth


## Significant contributors of OpenJK (in alphabetical order)

* eezstreet
* exidl
* ImperatorPrime
* mrwonko
* redsaurus
* Scooper
* Sil
* smcv
