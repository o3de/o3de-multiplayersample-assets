# Contribution Guidelines

Before you start participating in and contributing to O3DE code such as this Gem, review our [code of conduct](https://o3de.org/docs/contributing/code-of-conduct/) . Contributing guidelines for our projects are hosted in their GitHub repositories, within the root directory in a `CONTRIBUTING.md` file.

https://www.o3de.org/contribute/#contribution-guidelines

Please use a changelog.md or worklog.md file alongside your asset work, especially for impactful changes, or when documenting a change adds meaningful value for others.

## Contributions:

Contributions are more than just welcome. Fork this repo and create a new branch, then submit a pull request:

### General workflow

1. Fork the asset repo: https://github.com/aws-lumberyard/o3de-dreamstudio
2. Clone your fork `git clone https://github.com/aws-lumberyard/o3de-dreamstudio-your-fork`
4. Create your feature branch `git checkout -b my-new-feature`
5. Commit your changes `git commit -am 'Add some feature'`
6. Push to the branch `git push origin my-new-feature`
7. Create new Pull Request

### Create a new branch in your local workspace

*Switch to the branch you want to use as a base (e.g. create a feature branch off of development)*

> git switch development
> git pull

*Create a new branch*

> git switch -c <user/branch_name>

### Develop on Branch

Make changes (like adding new asset files) and commit these locally

*This will display files that have changes:*

> git status
> git add <filename_or_directory>
> git commit -s -m "<description of your change>"

Push changes to the origin. This is required to share your changes, run Automated Reviews, and submit Pull Requests.

*Use the following to push the commits in your current branch to origin. -u will set the branch to track the origin repo. Only required one time.*

> git push -u origin HEAD

*After using -u, the next time, you can just run a simple push for that branch*

> git push

### Submit a Pull Request

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

#### Notes:
* Get a few distinctly different people to review your PR, this repo only requires 2 approvals to merge, but it’s not a bad idea to get others to not only look at the changes but also try the asset changes themselves.
* This repo is not set up with AR, so make sure your changes work before merging to development, or from development to main.
* Test your changes, you don’t need to test them in MultiplayerSample (game)
  * You can use the MPS Asset Test Project folder:
    * C:\path\to\repo\o3de-multiplayersample-assets\Project
  * Or, you can activate these asset gems into your own test project...

## Asset Contributions:

If you are authoring and submitting content to this o3de-multiplayer-assets repository, you should abide by the best practices outlined in the guides located in the following location [o3de-multiplayersample-assets/Guides](https://github.com/o3de/o3de-multiplayersample-assets/tree/main/Guides) 

These guides are a work in progress, may not be complete and are maintained by the community.  If there are important workflows or information that is critical but missing, please [file a GHI for this repo (link)](https://github.com/o3de/o3de-multiplayersample-assets/issues/new). We encourage contributors and the community to also write additional content for these guides, so feel free to make changes and push a PR with updates.

#### [Getting Started (link)](https://github.com/o3de/o3de-multiplayersample-assets/blob/main/Guides/GettingStarted.md)

This guide will help you get up and running with Digital Content Creation tools (DCC), such as Blender or Maya.

#### [Content Guide (link)](https://github.com/o3de/o3de-multiplayersample-assets/blob/main/Guides/ContentGuide.md)

This guide outlines in depth best practices about our standards for asset contributions such as 3D model props, materials, terrain, and other general asset types etc.

#### [Kitbash3D Guide (link)](https://github.com/o3de/o3de-multiplayersample-assets/blob/main/Guides/Kb3dGuide.md)

This guide has additional information related to converting assets from the [Kitbash3D High Tech Street (kit)](https://kitbash3d.com/products/high-tech-streets) and ported them for use in the MultiPlayerSample NewStarbase level.

## Licensing and Legal Guidance

You must be aware of asset licensing restrictions present in this repository.

Some Assets have been developed by third parties and are subject to separate license terms (such as the Kitbash3D assets or Adobe Mixamo files). It is your responsibility to comply with the applicable licenses for such content should you choose to use these assets in any other project. Information on third party materials, and the applicable license terms, are referenced in or included with the asset artifacts (3d models, texture images, etc.), such as in separate LICENSE*.txt files accompanying the materials in each Gem or Asset subfolders. 

If you are contributing new assets to this repository, all contributions must be freely available public domain content, wholly new works of art, or modified and ported conversions of assets with similarly permissible licensing. You must document the origin of assets even if modified and include licensing information for the assets within the assets subfolder (add a LICENSE.txt). Generally all new asset contributions must be made under a permissible open source license, such as: Creative-Commons license (such as LICENSE-CC0.txt or LICENSE-CC-BY-4.0.txt)

### License

For general terms please see the LICENSE*.TXT files at the root of this distribution.
