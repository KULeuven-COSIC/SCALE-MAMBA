{ sources ? import ./sources.nix }:

let
  pkgs =
    import sources.nixpkgs { overlays = [ (import sources.nixpkgs-mozilla) ]; };
  channel = "stable";
  date = "2020-10-08";
  targets = [ "wasm32-unknown-unknown" ];
  chan = pkgs.rustChannelOfTargets channel date targets;
in chan
