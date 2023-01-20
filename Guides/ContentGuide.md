# Content Guide

Each Asset Gem is portable, and can be enabled for use with any game project.  From the standpoint of creating content for a Gem, each Asset Gem is it's own mini-project root.

With DCC tools, it's often beneficial to teams to utilize relative paths for things like asset references, such as texture paths.  This makes something like an encapsulated Gem more portable; moved across projects, amongst teams and individuals, etc. This means that the Gem could be cloned anywhere locally (nested folder structure, even different drive letters) and not break reference paths when opened on someone else's machine.

Blender works with relative paths, with the current scene file as the root. Maya by default, not so much, if a project root is not specified then it will store absolute paths, and this will break portability - leading to headaches for other users (each person, locally, has to repath/resolve file references. Ick. Lots of churn.) We can do better.

## Maya

Maya does allow you to [Organize Files into Projects](https://knowledge.autodesk.com/support/maya/learn-explore/caas/CloudHelp/cloudhelp/2019/ENU/Maya-ManagingScenes/files/GUID-9CE78B5A-7E9F-45E6-AB6D-66795E5656F4-htm.html), and utilize the project root to enable relative paths.  Essentially, the project root is marked by a workspace.mel file, a file that also contains project folder rules. This can be extremely useful, and it what makes portability work.  You'll notice, these gems contain this file at the root of each Gem.

![](../assets/img/posts/ContentGuide-assets/2023-01-20-14-24-01-image.png)

There are several ways you could go about correctly utilizing this feature.

### O3DE Bootstrapping

The O3DE [DccScriptingInterface Gem](https://github.com/o3de/o3de/tree/development/Gems/AtomLyIntegration/TechnicalArt/DccScriptingInterface) provides patterns for configuration and settings, launching, and boostrapping DCC tools like Maya with O3DE capabilities.  If you enable this Gem for your project, you can for instance start Maya from a Editor menu (this Gem is already enabled in the [MPS-Asset-Test project](C:\depot\MPS_assets\o3de-multiplayersample-assets\Project)). 
