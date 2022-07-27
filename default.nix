{ pkgs ? import (builtins.fetchTarball "https://github.com/NixOS/nixpkgs/archive/22.05.tar.gz") {}
}:
pkgs
