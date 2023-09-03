Homework 3 - the blockchain

Directory layout:
doc - contains project documentation
exe - contains executable form of the project
extern - if building from source, you'll need this
src - contains source code
README.txt - this

Notes for building from source:

You will need to set up Visual Studio to recognize the external libraries I used for the http-server. The library name is plibsys and I built the httpd around its socket implementation. If you're building from source in a VS project:
In the tippy top ribbon, under the PROJECT menu, go to the bottom and select [project name] PROPERTIES. 
In the window that pops up, in the left-hand pane, under CONFIGURATION PROPERTIES, select the VC++ DIRECTORIES option. Under GENERAL, click on the ... at the right-hand side of the INCLUDE DIRECTORIES menu option. Add the ./extern/include directory. This brings in the headers.

For the statically compiled plibsys library, in the same [project name] PROPERTIES window, go to LINKER in the left-hand pane. Under LINKER, select INPUT and in the ADDITIONAL DEPENDENCIES add the ./extern/lib/plibsys.lib file.

Then you should be good to build.