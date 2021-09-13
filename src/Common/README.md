# Common lib
Author: Alberto Gomez (alberto.gomez@kcl.ac.uk)

# Summary

The common lib supports generic functionality used accross plug-ins and the main app. The main functionalities and associated classes are:

* `ifindImage` is the base image class that includes image data, overlays and metadata. This class is used to transfer information between plugins and through the Streams.
* ifindImage support classes for writing, reading, metadata management
* `ifindImagePeriodicTimer` serves as a means to transmit data at a certain frame rate. This in turn can be used to ensure that plug-ins work at a certain frame rate too.
* `ifindStreamTypeHelper` is a class to manage, read, set and print stream information.
* `inputParser` is a class to manage comand line arguments. It is inspired by the work from [Iain](https://stackoverflow.com/questions/865668/parsing-command-line-arguments-in-c).

