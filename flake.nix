{
  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";

  outputs = { self, nixpkgs }:
    let
      lib = nixpkgs.lib;
      forAllSupportedSystems = f:
        lib.genAttrs [
          "x86_64-linux"
          "aarch64-linux"
          "x86_64-darwin"
          "aarch64-darwin"
        ] (system: f nixpkgs.legacyPackages.${system});
    in {
      formatter = forAllSupportedSystems (pkgs: pkgs.nixfmt-classic);

      packages = forAllSupportedSystems (pkgs: {
        default = pkgs.clang18Stdenv.mkDerivation {
          name = "3d";
          src = ./.;
          nativeBuildInputs = with pkgs; [ cmake ninja pkg-config ];
          buildInputs = with pkgs; [
            assimp
            glm
            glfw
            ((imgui.overrideAttrs rec {
              version = "1.91.5-docking";
              src = fetchFromGitHub {
                owner = "ocornut";
                repo = "imgui";
                tag = "v${version}";
                hash = "sha256-6VOs7a31bEfAG75SQAY2X90h/f/HvqZmN615WXYkUOA=";
              };
            }).override { IMGUI_BUILD_GLFW_BINDING = true; })
            zlib
            nlohmann_json
          ];
        };
      });

      devShells = forAllSupportedSystems (pkgs: {
        default = self.packages.${pkgs.system}.default.overrideAttrs (old: {
          nativeBuildInputs = old.nativeBuildInputs ++ [ pkgs.ccache ];
          ASAN_OPTIONS = "symbolize=1";
          ASAN_SYMBOLIZER_PATH = "${pkgs.llvm}/bin/llvm-symbolizer";
          CMAKE_BUILD_TYPE = "Debug";
          hardeningDisable = [ "fortify" ];
        });
      });
    };
}
