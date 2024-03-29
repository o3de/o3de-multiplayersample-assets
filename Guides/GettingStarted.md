# MPS Getting Started with Content Tools

You can follow on of these guide to get up MPS and running:

[Multiplayer sample (MPS) project for the Open 3D Engine](https://github.com/o3de/o3de-multiplayersample)

[O3DE multiplayer sample assets](https://github.com/o3de/o3de-multiplayersample-assets) (this guides parent repository)

Or you could work with these assets in your own game project.

## Licensing

You must be aware of asset licensing restrictions present in this repository.

Some Assets have been developed by third parties and are subject to separate license terms (such as the Kitbash3D assets or Adobe Mixamo files). It is your responsibility to comply with the applicable licenses for such content should you choose to use these assets in any other project. Information on third party materials, and the applicable license terms, are referenced in or included with the asset artifacts (3d models, texture images, etc.), such as in separate LICENSE*.txt files accompanying the materials in each Gem or Asset subfolders.

If you are contributing new assets to this repository, all contributions must be freely available public domain content, wholly new works of art, or modified and ported conversions of assets with similarly permissible licensing. You must document the origin of assets even if modified and include licensing information for the assets within the assets subfolder (add a LICENSE.txt). Generally all new asset contributions must be made under a permissible open source license, such as: Creative-Commons license (such as LICENSE-CC0.txt or LICENSE-CC-BY-4.0.txt)

## Asset Gems

Each Asset Gem is portable and can be enabled for use with any game project.  From the standpoint of creating content for a Gem, each Asset Gem is its own mini-project root.

With DCC tools, it's often beneficial for teams to utilize relative paths for asset references such as texture paths.  This makes something like an encapsulated Gem more portable; it can be moved across projects, amongst teams and individuals, etc. This means that the Gem can be cloned anywhere locally (nested folder structure, even different drive letters) and not break reference paths when opened on someone else's machine.

Blender works with relative paths, with the current scene file as the root. Maya doesn't by default. If a project root is not specified then it will store absolute paths, and this will break portability, leading to headaches for other users. Each person, locally, has to repath/resolve file references, causing lots of churn and extra work. We can do better.

## Maya

Maya does allow you to [Organize Files into Projects](https://knowledge.autodesk.com/support/maya/learn-explore/caas/CloudHelp/cloudhelp/2019/ENU/Maya-ManagingScenes/files/GUID-9CE78B5A-7E9F-45E6-AB6D-66795E5656F4-htm.html), and utilize the project root to enable relative paths.  Essentially, the project root is marked by a `workspace.mel` file, a file that also contains project folder rules. This can be extremely useful, and it's what makes portability work.  These Gems contain this file at the root of each Gem.

![image](https://user-images.githubusercontent.com/23222931/225958367-c2df01b2-427a-4464-ac88-000f47630008.png)

There are several ways you could go about correctly utilizing this feature.

### O3DE Bootstrapping

The O3DE [DccScriptingInterface Gem](https://github.com/o3de/o3de/tree/development/Gems/AtomLyIntegration/TechnicalArt/DccScriptingInterface) (aka DCCsi) provides patterns for configuration and settings, launching, and boostrapping DCC tools like Maya with O3DE capabilities.  If you enable this Gem for your project, you can for instance start Maya from a Editor menu (this Gem is already enabled in the [MPS-Asset-Test project](https://github.com/o3de/o3de-multiplayersample-assets/tree/development/Project). You can also start DCC apps like Maya externally but will utilize the O3DE bootstrapping, for Asset Gems this is currently the best approach, with managed boot we can set the Asset Gem as the Maya project and use its `workspace.mel`.

Note: the following only needs to be reviewed and set up once, so the information is retained in this guide for educational and troubleshooting purposes.  Have a technical artist help you get set up the first time, if you encounter any trouble ping the [Discord o3de #general-support](https://discord.com/channels/805939474655346758/869974333009854464) or the [#technical-artits](https://discord.com/channels/805939474655346758/842110573625081876) group chats.  If you are all set up, just jump ahead to the section below titled **Launch Maya**.

#### Env .bat files

In each Asset Gem, you'll find a Launch_Maya.bat, such as:

`o3de-multiplayersample-assets\Gems\level_art_mps\Tools\Launch_Maya.bat`

On Windows, a double-click of that will launch Maya2023 with O3DE bootstrapping, during boot this will manage setting the project root.

Note: there are many ways O3DE can be configured, versioned engine install, nightly installer build, developer engine fork, etc, you may need to specify which engine to use for bootstrapping to perform correctly.  You can use the following file:

`level_art_mps\Tools\Env_Dev.bat`

(copy and modify `Env_Dev.bat.template`)

###### Engine Root (O3DE_DEV)

As a developer working on projects like MPS, you may be building a local engine fork (with changes to engine and project code.) In such a case, you should map your engine like this (inside of `Env_Dev.bat`):

```batch
:: set the location of my local fork engine root
set O3DE_DEV=C:\depot\o3de-dev
```

Or if you are using release or nightly installers, the root would be as such:

```batch
:: root for release installer buid
set O3DE_DEV=C:\O3DE\22.05.0

:: nightly
set O3DE_DEV=C:\O3DE\0.0.0
```

###### MAYA_PROJECT

Environment variables are a useful way to modify the standard path locations and behavior of Maya (a common bootstrapping pattern for many DCC tools.)

The `Launch_Maya.bat` calls other .bat files as well, to chain together the environment for launch and boostrapping of Maya.  The Gem as the project root path for Maya, is specified by the envar `PATH_O3DE_PROJECT` in this file:

`o3de-multiplayersample-assets\Gems\level_art_mps\Tools\Project_Env.bat`

###### DccScriptingInterface Gem (DCCsi)

The DCCsi itself has the core environment .bat files that are eventually called, when you start `Launch_maya.bat` it will trigger `CALL %~dp0Env_Dev.bat` and then `CALL %~dp0Project_Env.bat`, and then finally call:

```batch
CALL %O3DE_DEV%\Gems\AtomLyIntegration\TechnicalArt\DccScriptingInterface\Tools\Dev\Windows\Env_DCC_Maya.bat
```

Our managed `PATH_O3DE_PROJECT` envar, is mapped to the Maya managed envar `MAYA_PROJECT` 

```batch
set MAYA_PROJECT=%PATH_O3DE_PROJECT%
```

With this envar set at launch, Maya will now search for the `workspace.mel` file, and if it's found will use this as the project root, along with any folder rules specified within the workspace file.

###### ENVARS

You can use the `Env_Dev.bat` to override other aspects of the environment as well.  One example would be mapping the O3DE binaries folder where executables live.  It comes down to how you are working and your configuration patterns, with an installer build, or as a engine developer.  The BIN folder has not only `*.exe`, but `*.dll` files, and compiled python `*.pyd` files that potentially some python script or tool might utilize. If you are [building from source](https://github.com/o3de/o3de.org/blob/main/content/docs/user-guide/build/configure-and-build.md), you specify the build folder and thus may want or need to map that with the DCCsi envars, something like:

```batch
set "PATH_O3DE_BIN=%O3DE_DEV%\build\bin\profile"
```

### Launch Maya

Double-click the `Launch_Maya.bat` to start Maya with our O3DE environment.

#### Maya, Project Window

if everything is properly configured, we should be in a rooted workspace.

In Maya, you can validate that:

    Maya > File > Project Window

![image](https://user-images.githubusercontent.com/23222931/225958877-cdb6aa6a-7b6f-44e6-ad1c-6db24099586d.png)

Now when you use an open command, it will conveniently place you in the Assets folder of the current Gem.  Beyond making assets more portable (relative file path references), working with Maya project workspaces brings a lot of conveniences.  This open dialog also gives you quick access to the root, asset folder (and other folder rules if they were added), and the ability to set the project without leaving the open dialog box.

![image](https://user-images.githubusercontent.com/23222931/225958556-69dfce81-76b0-4352-b373-45b9f1889e5d.png)

([check out Pipeline2](https://liorbenhorin.gumroad.com/l/pipeline2) an extended project manager for Maya)

#### Set Project

If you want to work with assets in a different Gem, you can manually switch projects with out restarting Maya.

    Maya > File > Set Project

Point the file dialog to another location with a `workspace.mel` such as the other Gems in this repo:

`o3de-multiplayersample-assets\Gems\character_mps`

`o3de-multiplayersample-assets\Gems\kb3d_mps`

`...`

## Exporting Content from Maya
This section of the guide covers best practices for Exporting 3D FBX files from Maya for O3DE.
< to do: write this section >

### Maya Standard FBX Exporter
< to do >

### Maya Game Exporter
< to do >

### O3DE Scene Exporter
< to do >

## Exporting Content from Blender
< to do >

### Blender Standard FBX Exporter
< to do >

### O3DE Blender Scene Exporter AddOn
< to do >

