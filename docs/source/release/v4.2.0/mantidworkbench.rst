=======================
MantidWorkbench Changes
=======================

.. contents:: Table of Contents
   :local:


User Interface
##############

- The zoom icon in the SliceViewer and plot toolbars have been replaced with clearer icons.

Improvements
############
- The keyboard shortcut Ctrl+N now opens a new tab in the script editor.
- There is now a button on the plot window's toolbar to generate a script that will re-create the plot.

Bugfixes
########
- Clicking Cancel after attempting to save a project upon closing now keeps Workbench open instead of closing without saving.
- Dialog windows no longer contain a useless ? button in their title bar.
- Instrument view now keeps the saved rendering option when loading projects. 
- Fixes an issue where choosing to not overwrite an existing project when attempting to save upon closing would cause Workbench to close without saving.
- Fit results on normalised plots are now also normalised to match the plot.

:ref:`Release 4.2.0 <v4.2.0>`
