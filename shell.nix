let
  sources = import ./nix/sources.nix;
  rust = import ./nix/rust.nix { inherit sources; };
  pkgs = import sources.nixpkgs { config = import ./nix/config.nix; };
in pkgs.mkShell {
  # this env var can be removed when
  # https://github.com/wbhart/mpir/issues/288
  # gets fixed
  LIBMPIR_PATH = "${pkgs.mpir}/lib";
  LIBCRYPTO_PATH = "${pkgs.cryptopp}/lib";
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
    pkg-config
    python27Packages.gmpy2
  ];
}
