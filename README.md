# Kholin Chess Engine
This chess engine was created as a part of my master's thesis at the Department
of informatics, University of Oslo. Delivered autumn 2016.

### Compilation
To compile you need to have GCC with pthreads installed. In a terminal, navigate
to the src-folder and type 'make'.

A precompiled windows 64-bit version is included. It is located in the
executables folder.

### Running the engine
The engine can be run in any UCI-compliant chess GUI. If you just want to play
against the engine, I suggest Pychess. You can download Pychess
[here](http://www.pychess.org/download/).

Open Pychess, click Edit-\>Engines. A new window, *manage engines*, pops up.
Push the *New* button and navigate to the compiled version of my engine. Close
the pop up window. You can now select KholinCE in the drop down menu.
