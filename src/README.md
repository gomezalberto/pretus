# PRETUS - Software organization
**P**lug-in based, **Re**al-**t**ime **U**ltra**s**ound

---

The source code is organised as follows:

* The main application (the executable) is in the folder [App](App)
* The code for the `Common` library is in the folder [Common](Common). This library implements image-related functionality.
* The plug-ins parent classes are implemented by the plugin library, in the folder [PluginLib](PluginLib). This library implements the parent classes for the Plug-ins, their Workers and their Widgets. All plug-ins must inherit the `Plugin` class.
* The default plugins are included in the [Plugins](Plugins) folder.

