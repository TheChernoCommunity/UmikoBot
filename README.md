
# UmikoBot
## Prerequisites
### Windows
- [Qt](https://www.qt.io/) version bigger than 5
- [OpenSSL](https://slproweb.com/products/Win32OpenSSL.html) version Win64 OpenSSL v1.0.2u
 - Visual Studio 2017 or 2019
 - Some sort of bash
 
### Linux
To build on linux you're going to need to have qt installed which is done by installing `qt5-default` and `libqt5websockets5-dev`

## Setting up the project
To build Umiko you have to first run the init script `./init.sh`.
**Note**: on windows you are going to get asked the paths for Qt, x86 and x64, and for OpenSSL, you don't need both x86 and x64, you can just use one and build for that specific platform

After that you can get the project files by running `generate_project_files.sh` with the according target.
**Note**: on linux you are going to need to build it with qmake (to get it just install qt5-qmake)
**Note**: on windows you can just run the script normally, it will create the project for vs 2017.
## Building
### On Linux
After that's done, go into `sln` and write `qmake` followed by `make`

### On Windows
Go into `sln` and open the project with visual studio.
Follow [this link](https://doc.qt.io/qtvstools/qtvstools-managing-projects.html) to set up the version for Qt.




## Running
The token is taken as a command line argument
**Note**: In Visual Studio you can put the token like so:
![](https://cdn.discordapp.com/attachments/353076704945766403/680397059068919808/unknown.png)

