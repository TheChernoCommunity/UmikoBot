
# UmikoBot
## Prerequisites
### Windows
- [Qt](https://www.qt.io/) (`> 5`)
- [OpenSSL](https://indy.fulgan.com/SSL/Archive/Experimental/openssl-1.0.2o-x64-VC2017.zip) (`OpenSSL v1.0.2`)
 - Visual Studio 2017 or 2019
 - Some sort of bash

### Linux
To build on Linux you're going to need to have Qt installed which is done by installing `qt5-default` and `libqt5websockets5-dev`.

## Setting up the project

To build Umiko you have to first run the init script (`./init.sh`).

- On Windows you would be asked the paths for **x86** and **x64** versions of Qt, and OpenSSL. You don't need both **x86** and **x64** - you can just use one and build for that specific platform.

- After that you can get the project files by running `generate_project_files.sh` with the required target.

  - On Windows you can just run the script normally; It will create the project for VS 2017.

  - On Linux you are going to need to build it with **qmake** (to get it just install `qt5-qmake`).

## Building

### On Linux
After that's done, go into `sln` and write `qmake` followed by `make`.

### On Windows

Go into `sln` and open the project with visual studio.
Follow [this link](https://doc.qt.io/qtvstools/qtvstools-managing-projects.html) to set up the version for Qt using `Qt VS Tools` as an extension.

## Running

The token to run the bot is taken as a command line argument. For Visual Studio, you can add the token in `Property Pages > Command Arguments`, which should look like this:

![](https://cdn.discordapp.com/attachments/353076704945766403/680397059068919808/unknown.png)

