
# UmikoBot

Umiko is the bot used on [TheCherno's Discord Server](https://discord.gg/K9rMPsu9pQ). This repository is what makes Umiko; it's the source code and it is completely open source!

## ❓ What can it do?

Umiko can do everything that a Discord bot can technically do. Of course it doesn't do all of it, but if you feel like there's something missing that would make Umiko better, contributions are always welcomed!

## ℹ️️ What does it do?

Umiko is mainly built for fun. Currently, Umiko can

- Keep track of your Currency and XP on the server: Both the Currency and XP system are extensively developed and are backed by a multitude of commands.
- Show you memes from the internet.
- Show you random GitHub repositories because why not?
- Keep track of your timezone.
- Keep track of your information so that others can use a single command to know (*everything* :eyes:) about you.
- Use your XP to assign you levels so that there's a bit of a competitive spark.
- Reward you for typing `<prefix>daily`, well, daily.
- Crash and work unexpectedly due to zillions of bugs here and there...
- Reward you with some (very useful) currency because you typed and you were lucky enough.
- Host events which tweak with Umiko's normal behavior to keep the fun going.
  
...and many more features still in works!

## 🙋‍♂️ Contributing

So you want to contribute? Awesome! Let's get you going. For starters, know that Umiko uses **[QDiscord](https://github.com/Gaztin/QDiscord)** to interface with the Discord API. This is already in the repo as a submodule, so you don't need to worry much.

There are some prerequisites to the whole setup and build process after you're done forking and cloning (checkout Git 101 under **[Help and FAQ](#-help-and-faq)** if you're unfamiliar with the whole process), so let's start with those:

### Prerequisites

#### Windows

- **[Qt](https://www.qt.io/) (`> 5`)**
- **[OpenSSL](https://indy.fulgan.com/SSL/Archive/Experimental/openssl-1.0.2o-x64-VC2017.zip) (`OpenSSL v1.0.2`)**
- We recommend using Visual Studio 2017 or 2019.
- Some sort of bash (to run the setup scripts).

#### Linux

To build on Linux you're going to need to have Qt5 and the **developer version** of Qt5WebSockets. You will also need `qt5-qmake`.

### Setting up the project

You'd have to do quite a bit of work to get the bot setup if we didn't have these two scripts:

- `init.sh`
- `generate_project_files.sh`

Thanks to those, all you need to do is run them in succession and you should have all the necessary files to work with.

#### `init.sh`

This file makes temporary folders and files to facilitate the setup. It also makes sure you have `premake5` to generate the project files and that the submodules are initialized and cloned (helpful for those who forgot to clone with `--recurse-submodules`).

On Windows it will ask you for the paths of the **x86** and **x64** versions of Qt and OpenSSL. You don't need both of the versions; you can just specify one and only build for that specific platform.

> Make sure you use forward slashes instead of the backslashes.

#### `generate_project_files.sh`

As the name suggests, this script generates the necessary files by using [Premake](https://premake.github.io/).

You can provide an action like you do with premake, but the script is configured such that it (by default) generates files for

- Visual Studio 2019 if you're on Windows.
- `qmake` if you're on Linux.

### Building

This section assumes that you used the default targets and didn't provide a custom action.

Once the files are generated, you should see a `sln/` directory.

#### On Linux

`cd` into `sln` and execute `qmake` followed by `make`.

#### On Windows

You should find a Visual Studio solution file (`.sln`) inside `sln`. Open that and then build the solution using Visual Studio.

### Running

The token to run the bot is taken as a command line argument.

For Visual Studio, you can add the token here:

|![Visual Studio Command Args](https://cdn.discordapp.com/attachments/353076704945766403/680397059068919808/unknown.png)|
|:--:|
|`Project Properties` (`Alt + Enter`) > `Configuration Properties` > `Debugging` > `Command Arguments`|

## ❓ Help and FAQ

If you weren't able to properly carry out the setup process, or if you just want to know more Umiko in general, considering checking these questions before opening an issue:

<details>
<summary>Is Umiko going to be open for invites anytime soon?</summary>
This hasn't been thought about much, but all of Umiko is developed keeping multiple servers in mind, so we're ready for that already!
</details>

<details>
<summary>Git 101
</summary>

This isn't an extensive guide by any means, but it'll bring you up to speed to start contributing.

The first thing you will need to do is to *fork* this repo (repository). You need to do this because you don't have *direct push permissions* to this repository (meaning that you can't just publish your code here directly).

*Forking* this repo essentially means creating your own *copy* of this repo on your GitHub account. It's not complicated either; just click on the Fork button on the upper-right corner of this repo's page, and voila!

Your *forked repo* (or simply *fork*) stays on GitHub and is thus called a remote (this remote is usually referred to as `origin`). What you need to do next would be to get it locally on your own system. This process is called *cloning*. To *clone* a repository into some local directory, go to that directory, fire up a terminal and type this:

```
git clone <repo-link>
```

In our case, the link would be `https://github.com/<your-github-account>/UmikoBot.git` where `<your-github-account>` is, well, your GitHub account. We also have submodules to take care of. So we use

```
git clone --recurse-submodules -j8 https://github.com/<your-github-account>/UmikoBot.git
```

(`-j8` is an optional flag which enables fetching up to 8 submodules in parallel. We don't really need to use it, but it's always better to know about it.)

> If that doesn't work, you might be using an old version of Git. Just do
>
> ```
> git clone https://github.com/> <MyGitHubAccount>/GameProject-1.git
> ```
>
> and make sure you run [`init.sh`](#initsh) right after; it will handle the submodule stuff (by using `git submodule update --init --recursive`).

You have successfully forked and cloned the project. If you came from [Contributing](#️-contributing), you can continue with the [prerequisites](#prerequisites).

The following material explains the basic commands and workflow to use for making contributions after you're done setting up and can build the project.

Remember, you can always check the *status* of your repo using

```
git status
```

You can also find more about a command by using

```
git help <command>
```

And of course, the **[documentation](https://git-scm.com/docs)** always helps.

We talked about remotes just a bit earlier. We came across the `origin` which is another name for your fork. The remote from where you forked your fork is usually called `upstream`. To list the remotes you have, use

```
git remote -v
```

You'll notice that you don't actually have an upstream setup (unless you set it up yourself, and in that case why are you here?). To add the upstream repository (which would be this repository), do

```
git remote add upstream https://github.com/TheChernoCommunity/UmikoBot.git
```

You have covered all the basic prerequisites. From now on, when you intend to add a certain feature, follow these steps:

1. Firstly, you need to create a new branch dedicated to that feature. You would usually branch out from the `master` branch of the main repo and then work on those changes.
2. With each small change you bring, make sure you commit it. Each commit is essentially a package of changes to different files. It's up to you to decide when something doesn't belong to a particular commit.
3. After the whole thing is properly done, you can then push it to your origin (you can push commits one by one while you work through them as well, but well it's up to you again).
4. The only thing is that this code isn't part of the upstream. To make it part of that, you need to open a Pull Request (or a PR) which is basically a request to pull changes from your code. Pull Requests are a GitHub construct, and to open one, simply go to `Pull Requests` (on this repo's page) > `New Pull Request`.
5. The code would then be reviewed and if it's all fine, it should be merged with the repo.
6. The branch that you worked on would then become a stale branch. You can now delete it.
7. While working on a feature if you find yourself in a scenario where upstream has had new commits that you would want to have in your version as well, just *rebase* your branch onto upstream's branch (this would usually be `master`).

The commands that you'll find useful to go with the above process are listed here (stuff beginning with `#` are comments to guide you):

```
# Make a branch basing off of the current point you're at
git branch <branch-name>

# Checkout that branch (you aren't moved to  that branch by default)
git checkout <branch-name>

# To make a new branch and also check it out
git checkout -b <branch-name>

# Checkout the origin's master
git checkout origin/master

# Deleting a branch on origin
git push -d origin <branch-name>

# Deleting a local branch
git branch -d <local-branch>

# Use add followed by a list of files separated by space to add/stage files for a commit
git add <file1> <file2> <file3> # and so on

# Stage all changes
git add -A

# To commit after staging files
git commit -m "A meaningful commit message"

# Pushing your changes to your remote
git push <remote-name> <branch-name>

# Pulling changes
git pull <remote-name> <branch-name>

# Ensuring a local branch is up to date with a branch on upstream
git fetch upstream <branch-name> 	# Fetch the meta-data
git checkout <branch-name> 			# Ensure you're at the correct branch locally
git merge upstream/<branch-name>

# Rebasing a branch (branch1) onto another branch (branch2)
git checkout branch1 	# First checkout the branch in concern
git rebase branch2 		# Then rebase
## This can also be done with a single command:
git rebase branch2 branch1

## Example: Rebasing your master to origin's master
git checkout master 	# Of course make sure you're in the correct branch
git fetch origin 		# Update origin
git rebase origin/master
## This also has the same effect as the last two commands:
git pull --rebase origin master

# Looking at the commits you made
git log --oneline
git log # More verbose
## NOTE: You will find the commit hash with the commits

# Checking out a commit temporarily
git checkout <hash>
## NOTE: This will detach your HEAD pointer. A detached state is when the HEAD points at a commit instead of a branch.
## If you already know you want to make changes, make a new branch instead with the following set of commands.

# Making a new branch basing off of a commit and checking it out
git checkout -b <new-branch-name> <hash>

# Reverting to an old commit
git reset --hard <hash>

## NOTE: ALL uncommitted work will be lost. If you have uncommitted changes you want to keep, stash them:
git stash # Firstly, stash the changes
git reset --hard <hash> # Revert to this commit
git stash pop # Pop all the changes from the stash

```

</details>

<details>
<summary>Is there some kind of a Roadmap?
</summary>

The closest thing to that we currently have is [this](https://github.com/TheChernoCommunity/UmikoBot/projects/1).
</details>

<details>
<summary>Is there some kind of a community where I can interact with fellow contributors?
</summary>

Glad you asked! We'll be happy to see you on our [Discord Server!](https://discord.gg/zrUpn7RG5k)
</details>
