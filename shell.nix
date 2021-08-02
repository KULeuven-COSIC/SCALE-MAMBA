let
  sources = import ./nix/sources.nix;
  binaryen_100 = import ./nix/binaryen.nix;
  pkgs = import sources.nixpkgs { config = import ./nix/config.nix; };
  unstable = import sources.unstable { config = { allowUnfree = true; }; };
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
    cmake
    doxygen
    python2
    killall
    binaryen_100
    wabt
    rustup
    unstable.rust-analyzer
    clang-tools
    pkg-config
    python27Packages.gmpy2
    graphviz
  ];
}
