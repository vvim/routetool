routetool
=========

Using Google Maps API to add locations and calculate the shortest route by Traveling Salesman Algorithm

prerequisites
-------------

see the information at https://code.google.com/p/qt-google-maps/w/list .

1. QJSON

On QJSON: all I get is a textfile containing

    The project has been moved to github: https://github.com/flavio/qjson/ git config remote.origin.url git://github.com/flavio/qjson.git 

so I simply ran "git clone git://github.com/flavio/qjson.git" instead of the above mentioned 'gitorious'-link .

Afterwards, apparently my CMake version was not the right version (I use a Debian 6 which apparently has a CMake 2.8.2), this QJson requires CMake 2.8.8 or higher. I downloaded the Linux binary from http://www.cmake.org/cmake/resources/software.html and ran that straight as "cmake" following the README.md instructions from QJson.

Now, building qt-google-maps was not a problem, but running it required me to copy the files "libqjson.so" and "libqjson.so.0" (from ...../qjson/build/lib/ ) to /usr/lib/ . 

All two folders: routetool and qjson must be in the same directory.

Then you need to compile qjson library first. We count that qjson library is in the qjson/build/lib directory.

Then the qt-google-maps project will be compiled OK. To run it you need to adjust LD_LIBRARY_PATH on Linux (or PATH on Windows) so that variable points to the qjson/build/lib directory. 

2. GOOGLE MAPS API KEY

Just as the 'qt-google-maps' example from https://code.google.com/p/qt-google-maps/ , routetool uses Google Maps API v3. To use API you need to register your own application on Google. Do not worry: the procedure is very simple.

Details:

You need to login to Google, so first you need to create simple Google account. Then you can visit the page

https://code.google.com/apis/console

there you can create your application. You need to check access to Google Maps API v3 in tab "Services".

Then you need create client ID in tab "API Access".

Create the file 'routetool.ini' and add the line "apiKey=.............." where the dots resemble the key given to you by the Google Api Console.

After that you can compile and run routetool.

Any problem? Please contact me directly.
