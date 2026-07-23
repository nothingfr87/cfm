#!/bin/sh

INSTALL_DIR="/usr/local/bin/"
GIT_URL="https://github.com/nothingfr87/cfm.git"

cat << 'EOF'
  /$$$$$$  /$$$$$$$$ /$$      /$$
 /$$__  $$| $$_____/| $$$    /$$$
| $$  \__/| $$      | $$$$  /$$$$
| $$      | $$$$$   | $$ $$/$$ $$
| $$      | $$__/   | $$  $$$| $$
| $$    $$| $$      | $$\  $ | $$
|  $$$$$$/| $$      | $$ \/  | $$
 \______/ |__/      |__/     |__/
EOF

sleep 0.7

printf "\n"
printf "\n"
printf "\n"
printf "\n"
printf "\n"
printf "Do you want to install CFM? (y/n): "
read -r choice

if [ "$choice" = "y" ] || [ "$choice" = "Y" ]; then
  echo "Checking Dependencies"
  if [ -f /etc/os-release ]; then
      . /etc/os-release
      OS=$ID
  else
      echo "Error: Cannot detect OS distribution. /etc/os-release missing."
      exit 1
    fi

    MISSING_DEPS=""

    if ! command -v git >/dev/null 2>&1; then
        MISSING_DEPS="git $MISSING_DEPS"
    fi
    if ! command -v pkg-config >/dev/null 2>&1; then
        MISSING_DEPS="pkg-config $MISSING_DEPS"
    fi

    case "$OS" in
      ubuntu|debian)
        if ! dpkg -s build-essential >/dev/null 2>&1; then
          MISSING_DEPS="build-essential $MISSING_DEPS"
        fi
        if ! dpkg -s libncurses-dev >/dev/null 2>&1; then
          MISSING_DEPS="libncurses-dev $MISSING_DEPS"
        fi
                    ;;
        fedora|rhel|centos)
            if ! rpm -q gcc >/dev/null 2>&1; then
                MISSING_DEPS="gcc $MISSING_DEPS"
            fi
            if ! rpm -q ncurses-devel >/dev/null 2>&1; then
                MISSING_DEPS="ncurses-devel $MISSING_DEPS"
            fi
            ;;
        arch|manjaro)
            if ! pacman -Qi base-devel >/dev/null 2>&1; then
                MISSING_DEPS="base-devel $MISSING_DEPS"
            fi
            if ! pacman -Qi ncurses >/dev/null 2>&1; then
                MISSING_DEPS="ncurses $MISSING_DEPS"
            fi
            ;;
        *)
            echo "Unsupported distribution: $OS"
            echo "Please install git, ncurses, pkg-config, and build tools manually."
            exit 1
            ;;
    esac

        if [ -n "$MISSING_DEPS" ]; then
        echo "The following missing dependencies will be installed: $MISSING_DEPS"
        echo "Requesting sudo permissions..."
        
        case "$OS" in
            ubuntu|debian|raspbian)
                sudo apt-get update && sudo apt-get install -y $MISSING_DEPS
                ;;
            fedora|rhel|centos)
                if echo "$MISSING_DEPS" | grep -q "gcc"; then
                    sudo dnf groupinstall -y "Development Tools"
                    MISSING_DEPS=$(echo "$MISSING_DEPS" | sed 's/gcc//')
                fi
                if [ -n "$MISSING_DEPS" ]; then
                    sudo dnf install -y $MISSING_DEPS
                fi
                ;;
            arch|manjaro)
                sudo pacman -Sy --needed --noconfirm $MISSING_DEPS
                ;;
        esac
    else
        echo "All dependencies are already installed!"
    fi

    echo "Cloning CFM Repo......"
    rm -rf cfm
    git clone $GIT_URL --depth 1
    cd cfm/ || exit 1

    echo "Building & Installing CFM....."
    sudo make all install

    echo "Done! CFM Installed in $INSTALL_DIR"

    echo "Removing CFM Direcotry"
    cd ..
    rm -rf cfm/
    

else
    echo "Installation canceled."
    exit 0
fi
