{
  packageOverrides = pkgs: {
    openssl_static = pkgs.openssl.override { static = true; };
    cryptopp = pkgs.cryptopp.overrideAttrs (oldAttrs: rec {
      preInstall = ''echo "we want to keep the static lib"'';
    });
  };
}
