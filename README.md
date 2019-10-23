# UmikoBot
## Setting up the project
To build Umiko you have to first run the init script `./init.sh`.
After that you can get the project files by running `generate_project_files.sh` with the according target.
**Note**: on linux you are going to need to build it with qmake (to get it just install qt5-qmake)

## Building
### On Linux
To build on linux you're going to need to have qt installed which is done by installing `qt5-default` and `libqt5websockets5-dev`
After that's done, go into `sln` and write `qmake` followed by `make`

### On Windows
TODO: Gaztin

## Running
The token is taken as a command line argument

