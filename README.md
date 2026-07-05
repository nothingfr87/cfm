# CFM

C File Manager is just a simple file manager

---

## Description

CFM is a simple TUI file manager fully written in C focusing on performance, and memory & cpu usage

---

## Requirements

- Ncurses (TUI)
- GCC (Compiler)

---

## Features

- Lightweight

And just like any file manager:

- Open Files/Folders
- Create Files/Folders
- Delete Files/Folders

- Focused on keyboard keybinds which makes it faster to use
    - q/Escape: Exit
    - a: Create a new file/folder
    - d: Delete a file/folder
    - Left Arrow/Enter: Enter a folder or open a file
    - Right Arrow: Go back

---

## Installation

Installing CFM is very easy

Before that let's install the required libraries

```bash
sudo apt update
sudo apt install build-essentials  libncurses5-dev libncursesw5-dev
```

#### Fedora Distros

```bash
sudo dnf groupinstall "Development Tools"
sudo dnf install ncurses-devel
```

#### Arch Distros

```bash
sudo pacman -S base-devel ncurses
```

Clone the repo , and build the source code

```bash
git clone https://github.com/nothingfr87/cfm.git
cd cfm/

make build
```

Now you have the binary you can do anything you want with it!

To install CFM on your PC

```bash
sudo make all install
```

On Windows, I don't know how to install binaries so you'll have to build the source code then move the binary file to a safe place then add that folder to your PATH environment

---

## Supported OS:

CFM Supports:

- Linux (Tested on Debian 13 Trixie)
- MacOS (Not Tested)
- Windows (Not Tested)

---

## License

This project is licensed under the [MIT License](LICENSE)

---

Quick Note:

Lightweight, Fast, and Practical -> CFM

---

Please consider giving a star if you liked the project, it would be great :)
