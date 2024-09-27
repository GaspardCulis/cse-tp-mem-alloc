{pkgs ? import <nixpkgs> {}}:
with pkgs;
  mkShell rec {
    packages = with pkgs; [
      helix
      clang-tools
      lldb
      cmake-language-server
      bear
      gdb
    ];
  }
