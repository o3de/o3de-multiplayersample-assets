![image](https://user-images.githubusercontent.com/23222931/225687331-b0f3ada2-447c-404f-8892-f278aac8e11f.png)

# O3DE MultiPlayerSample Assets

This repository contains:

1. A collection of O3DE Asset Gems, used in o3de-multiplayersample project
   1. The source folder for each Gem, for development by Contributors.
   2. E.g. an Asset Gem for sample characters: https://github.com/o3de/o3de-multiplayersample-assets/Gems/character_mps
2. "MPS-Asset-Test" project in: o3de-multiplayersample-assets/tree/main/Project
3. (Future) A `repo.json` file containing information about this O3DE Remote Gem Repository
4. (Future) A GitHub release folder
   1. The gem .zip containing the gem and associated gem.json for each gem

## If you are using o3de-multiplayer-sample Game

These gems are required for and utilized by the o3de-multiplayer sample project, you can find the repo and instructions for that project and adding these gems here:

[GitHub - o3de/o3de-multiplayersample: Multiplayer sample project for the Open 3D Engine](https://github.com/o3de/o3de-multiplayersample)

## If you want to use these Gems in your own o3de game project

### (Future) Option #1. Remote Repository, use packaged Gems with the Project Manager

**!!! Skip to Option #2 !!!**

**<u>Use of this as a remote gem repository is not yet implemented!</u>**

Add this remote repository in the Project Manager using this URL:

```
https://raw.githubusercontent.com/AMZN-temp-user-replace-me/o3de-remote-gem-repo-demo/main
```

You can then browse the gems in the Project Manager and add them to your Project.

### Option #2.  Download and use source Gems

You can clone the repo to download the repository source, then register the local gem source folders with the engine to make available for use in a Project.  This entails the same steps as a developer contributing content creation or performing other maintenance of the gem data. (see the next section below.)

## If you are a developer contributing to these asset gems

### Download and Install

This repository uses Git LFS for storing large binary files.  You will need to create a Github personal access token to authenticate with the LFS service.

#### Create a Git Personal Access Token

You will need your personal access token credentials to authenticate when you clone the repository.

[Create a personal access token with the 'repo' scope.](https://docs.github.com/en/github/authenticating-to-github/creating-a-personal-access-token)

#### (Recommended) Verify you have a credential manager installed to store your credentials

Recent versions of Git install a credential manager to store your credentials so you don't have to put in the credentials for every request. It is highly recommended you check that you have a [credential manager installed and configured](https://github.com/microsoft/Git-Credential-Manager-Core)

#### Step 1. Clone the repository

You can clone the project to any folder locally, including inside the engine folder. If you clone the project inside an existing Git repository (e.g. o3de) you should add the project folder to the Git exclude file for the existing repository.ne-the-repository)

##### Option #1 (Highly Recommended) - cloning into a folder outside the engine repository folder

```shell
# clone the project into a folder outside your engine repository folder
> git clone https://github.com/o3de/o3de-multiplayersample-assets.git
Cloning into 'o3de-multiplayersample-assets'...
```

##### Option #2 - cloning into the engine repository folder

```shell
# clone the project into a folder named 'o3de-multiplayersample' in your existing engine repository folder
> git clone https://github.com/o3de/o3de-multiplayersample-assets.git c:/path/to/o3de/o3de-multiplayersample-assets
Cloning into 'o3de-multiplayersample'...

# modify the local engine git exclude file to ignore the project folder
> echo o3de-multiplayersample-assets > c:/path/to/o3de/.git/info/exclude
```

If you have a Git credential helper configured, you should not be prompted for your credentials anymore.

### Step 2. Register the gems with the engine

You may have already done this, these are the same steps as setting up the o3de-multiplayer sample project.  But if you are adding them to your own project these are the steps to do so.

Make sure your engine is registered.

```batch
:: register the gems with the engine, you only need to do this once
> c:\path\to\o3de\scripts\o3de register --this-engine
```

Make sure your project is registered.

```batch
:: register the project with the engine, you only need to do this once
> c:\path\to\o3de\scripts\o3de register -p c:\path\to\o3de-multiplayersample
```

Now make sure that the source gems are registered

```batch
:: register the gems with the engine, you only need to do this once
> o3de register --all-gems-path c:\path\to\o3de-multiplayersample-assets\Gems
```

 The above command will recursively scan the input path, then registers all paths with gem.json files into the ~/.o3de/o3de_manifest.json

Now these Gems will be available in the Project Manager and can be added to your Project.

## (Optional) MPS: Asset Test Project

This repository contains it's own O3DE Game project, which can be used by contributors to build, validate, and maintain new assets independently.

```shell
# the test project for mps assets, is in this project folder in repo
> cd C:\path\to\o3de-multiplayersample-assets\Project
```

### Option 1. Add Project to O3DE Project Manager

1. Launch your O3DE Project Manager (o3de.exe)

2. Use the blue button in the upper right labeled "New Project ...", and with the pulldown select "Open Existing Project"

3. Browse to and add the MPS Asset Test Project folder:  C:\path\to\repo\o3de-multiplayersample-assets\Project

4. You may be prompted to rebuild the project (there are other options for building as a developer)

5. Launch the Editor for the project

### Option 2. Add via cli and build yourself

You can use the o3de cli tools to register your project with your engine.

```shell
# change directory to the engine root folder
> cd c:\path\to\your\o3de-engine
# register the gems with the engine, you only need to do this once
> scripts\o3de register --project-path C:\depot\o3de-multiplayersample-assets\Project
```

There are a number of ways to configure and build the engine and project ...

[Configure and Build - Open 3D Engine](https://www.o3de.org/docs/user-guide/build/configure-and-build/)

As a developer, I often find it useful to build in an engine-centric way

```shell
# change directory to the engine root folder
> cd c:\path\to\your\o3de-engine
# create a build folder
> mkdir build
> cd build
# configure cmake and create solution
> cmake .. -G "Visual Studio 16" -A x64 -DLY_3RDPARTY_PATH="%LY_3RDPARTY_PATH%" -DLY_UNITY_BUILD=OFF -DLY_PROJECTS="C:\path\to\repo\o3de-multiplayersample-assets\Project"
```

This should generate a build\O3DE.sln file, which can be opened in Visual Studio 2019 and compiled.

Don't forget to build the project in Profile

![image](https://user-images.githubusercontent.com/23222931/185497967-b94e1e5e-722c-4b84-b950-a00477343e56.png)

Building the Editor project will build with all dependencies.

![image](https://user-images.githubusercontent.com/23222931/185498026-0f235b71-ff10-4135-8008-ba067b6a285b.png)

The built binary executables will be in:

```shell
# bin folder
> cd C:\depot\o3de-dev\build\bin\profile\
# executables
# C:\depot\o3de-dev\build\bin\profile\o3de.exe
# C:\depot\o3de-dev\build\bin\profile\editor.exe
# ...
```

If this is the project you are primarily working with, you can also set it as the "default project". 

```shell
# change directory to the engine root folder
> cd c:\path\to\your\o3de-engine
# register the gems with the engine, you only need to do this once
> scripts\o3de set-global-project -pp <project-path>
```

This will make this the project that launches by default if you manually directly double-click on a o3de executable like editor.exe

# Contributing to the MPS Asset Repo

After following all the steps above, you should be ready to start working.  Once you are in the Editor, you can develop assets to be committed to the asset repo.  Working with the o3de-multiplayersample-assets repo is a similar workflow to the o3de codebase, the contributor guide is here for reference: https://docs.o3de.org/docs/contributing/to-code/git-workflow/

The following steps help you validate you are set up correctly and get working to contribute changes.

## O3DE Asset Processor (AP)

After the Asset Processor (https://www.o3de.org/docs/user-guide/assets/asset-processor/interface/) and Editor have started, the first suggestion would be to validate that the Gems are active and processing. This is an important troubleshooting step if you actually are having asset issues, like them not showing up in the editor.

1. In the Windows task-bar tray, there should be an icon for the O3DE Asset Processor
   - ![image](https://user-images.githubusercontent.com/67011188/201197997-d8908bed-cb42-4604-bb87-8b08fa34c233.png)
2. To view the Asset Processor interface, *right-click* the  Asset Processor icon in the system tray and select *Show*.
3. Click on the Assets tab
4. The Project: path should be visible, something like: 
   - C:\path\to\repo\o3de-multiplayersample-assets\Project
5. The Root: is the path to the active engine install
6. The o3de-multiplayersample-assets\Gems should be visible:
7. ![image](https://user-images.githubusercontent.com/67011188/201198240-fb9aa04c-9f62-4849-afd8-4e13e890cf96.png)

## MPS Asset Gem Folders

This is a collection of modular Asset Gems used in o3de-multiplayersample game, but you can develop and contribute assets simply using the MPS Asset Test project.  You simply need to place your assets into the appropriate Gem.

### Asset/ Folders on Disk

1. Asset Gems have an Assets/ folder, this is the default location within the gem folder structure that the AP scans for assets to process and make game ready.
2. From the AP, you can get to the folder by ...
   1. Selecting the gem, such as ‘character_mps’
   2. Right-click, from the context menu select ‘Open in Explorer’
3. Or you can manually browse to them on disk in Windows File Explorer
   1. Examples:
      1. C:\path\to\repo\o3de-multiplayersample-assets\Gems\kb3d_mps\Assets
      2. C:\path\to\repo\o3de-multiplayersample-assets\Gems\character_mps\Assets
      3. ...
4. Organize your files into the \Assets folder.  We prefer one-folder per-asset, such as:
   1. Gems\character_mps\Assets\MyCharacter\
   2. Gems\character_mps\Assets\MyCharacter\mycharacter.fbx
   3. Gems\character_mps\Assets\MyCharacter\mycharacter.material
   4. Gems\character_mps\Assets\MyCharacter\textures
   5. Gems\character_mps\Assets\MyCharacter\textures\mycharacter_basecolor.png
   6. Gems\character_mps\Assets\MyCharacter\textures\mycharacter_roughness.png
   7. ...
5. As you add asset files to the Gem folder, the AP will scan and process them (into runtime ready assets); so they will show up in your Game Project data in the Editor

### Asset/ Folders in Editor

The folder structure for each asset gem is retained and accessible in the Editor as well.

1. Select a folder, right-click and ‘Open in Explorer’
   ![image](https://user-images.githubusercontent.com/67011188/201198691-df9f074c-13af-4c42-9375-b465753e77f4.png)

### Create a new branch in your local workspace

*Switch to the branch you want to use as a base (e.g. create a feature branch off of development):*

`git switch development`

*Pull the branch state:*

`git pull`

*Create a new branch from that state:*

`git switch -c <user/branch_name>`

### Develop on Branch

Make changes (like adding new asset files) and commit these locally

*This will display files that have changes:*

`git status`

*This action will stage your work:*

`git add <filename_or_directory>`

*This action will commit the new work:*

`git commit -s -m "<description of your change>"`

Push changes to the origin. This is required to share your changes, run Automated Reviews, and submit Pull Requests.

*Use the following to push the commits in your current branch to origin. -u will set the branch to track the origin repo. Only required one time:*

`git push -u origin HEAD`

*After using -u, the next time, you can just run a simple push for that branch:*

`git push`

## Submit a Pull Request

You’ll want to submit a Pull Request and get 2 reviewers before you merge your changes to the public repo.

1. Go to: https://github.com/o3de/o3de-multiplayersample-assets 
2. Click *New Pull Request*
3. Click compare and select your branch. 
   1. Base branch: development
   2. Compare: <user/branch_name>
   3. Click *Create pull request*

Note: If you are comparing across forks (if you made one), the workflow will be slightly different.  If you have any trouble, let us know so we can help.

Configuring your PR should look something like:

![image](https://user-images.githubusercontent.com/67011188/201199896-ce1e070c-7533-44b8-a4d0-ca6e3f4e4b5f.png)

Add the necessary details to the PR

1. Enter title and description
2. Add reviewers: assign to GitHub users or teams
3. Get reviews and approvals needed
4. Then merge your PR

Notes:

* Get a few distinctly different people to review your PR, this repo only requires 2 approvals to merge, but it’s not a bad idea to get others to not only look at the changes but also try the asset changes themselves.
* This repo is not set up with AR, so make sure your changes work before merging to development, or from development to main.
* Test your changes, you don’t need to test them in MultiplayerSample (game)
  * You can use the MPS Asset Test Project folder:
    * C:\path\to\repo\o3de-multiplayersample-assets\Project
  * Or, you can activate these asset gems into your own test project...

# Appendix

After the gems are registered and added to a project, you can validate they are operational via the Asset Processor

![image](https://user-images.githubusercontent.com/23222931/178357181-b9d6689e-00c1-46b3-84a7-049615c6d244.png)

How these Asset Gems were made with O3DE.

1. The Gems were created from the Asset Gem template

2. Then the gem.json (metadata) was manually updated per-gem

```batch
cd c:\path\to\o3de\
scripts\o3de create-gem --gem-path C:\o3de-multiplayersample-assets\Gems\props_mps --template-path C:\o3de\Templates\AssetGem
scripts\o3de create-gem --gem-path C:\o3de-multiplayersample-assets\Gems\level_art_mps --template-path C:\o3de\Templates\AssetGem
scripts\o3de create-gem --gem-path C:\o3de-multiplayersample-assets\Gems\character_mps --template-path C:\o3de\Templates\AssetGem
scripts\o3de create-gem --gem-path C:\o3de-multiplayersample-assets\Gems\landscape_mps --template-path C:\o3de\Templates\AssetGem
scripts\o3de create-gem --gem-path C:\o3de-multiplayersample-assets\Gems\pbr_material_pack_mps --template-path C:\o3de\Templates\AssetGem
scripts\o3de create-gem --gem-path C:\o3de-multiplayersample-assets\Gems\particlefx_mps --template-path C:\o3de\Templates\AssetGem
scripts\o3de create-gem --gem-path C:\o3de-multiplayersample-assets\Gems\kb3d_mps --template-path C:\o3de\Templates\AssetGem
```
