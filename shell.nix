let
  sources = import ./nix/sources.nix;
  rust = import ./nix/rust.nix { inherit sources; };
  pkgs = import sources.nixpkgs { };
in pkgs.mkShell {
  buildInputs = with pkgs; [
    gnumake
    mpir
    cryptopp
    zlib
    gcc10
    openssl
    rust
    cmake
    doxygen
    python2
    killall
    binaryen
    wabt
    clang-tools
  ];
}
