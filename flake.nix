{
  description = "Flake for mbdsp";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, utils }@inputs:
    utils.lib.eachDefaultSystem
      (system:
        let
          pkgs = import nixpkgs { inherit system; };
        in
        {
          packages = with pkgs;{
            pkg = pkgs.stdenv.mkDerivation {
              pname = "mbdsp";
              version = "0.1.0";
              src = ./.;

              buildInputs = [
                cmake
              ];

              meta = with lib;
                {
                  description = "mbdsp";
                  license = licenses.gpl3;
                  platforms = platforms.all;
                  maintainers = with maintainers; [ michaeldonovan ];
                };
            };
          };

          defaultPackage = self.packages.${system}.pkg;
        });
}

