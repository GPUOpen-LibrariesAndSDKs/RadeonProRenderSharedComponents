# RadeonProRender Shared Components

This repository contains code that can shared among various RPR integration plugins.  RPR plugins
are implemented in a number of programming languages so the directory structure distinguishes
the code type being used.

To pull this repository, do the following:

If FireRender.Components directory does not exist:
1. Change to the top level of the plugin repository
2. git subtree add --prefix FireRender.Components https://github.com/Radeon-Pro/RadeonProRenderSharedComponents.git master --squash
	
If the subtree add has already been done:
1. Change to the top level  of the plugin repository
2. git subtree pull --prefix FireRender.Components https://github.com/Radeon-Pro/RadeonProRenderSharedComponents.git master --squash
3. git commit --amend  # To change "Merge commit (SHA-1 hash)" to a more descriptive commit such as:
	"git subtree pull latest FireRender.Components shared code"
4. Save and close the commit text file to register the change

When finished adding or pulling the subtree, update the projects or cmake files to bring the correct files into the plugin build.


