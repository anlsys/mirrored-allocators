# development shell, includes aml dependencies and dev-related helpers
{ pkgs ? import ./. { } }:
with pkgs;
mkShell {
  nativeBuildInputs = [ autoreconfHook pkgconf ];
  buildInputs = [
    # deps for debug
    gdb
    valgrind
    # style checks
    clang-tools
  ];
}
