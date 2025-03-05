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
          buildInputs = let
            imgui = (pkgs.imgui.overrideAttrs rec {
              version = "1.91.5-docking";
              src = pkgs.fetchFromGitHub {
                owner = "ocornut";
                repo = "imgui";
                tag = "v${version}";
                hash = "sha256-6VOs7a31bEfAG75SQAY2X90h/f/HvqZmN615WXYkUOA=";
              };
            }).override { IMGUI_BUILD_GLFW_BINDING = true; };

            imguizmo = let
              vcpkgSource = pkgs.fetchFromGitHub {
                owner = "microsoft";
                repo = "vcpkg";
                rev = "4110d43398232f486c3fd0b74e2dc9e4ca5e2a59";
                hash = "sha256-fqNuZRrdtpPFIU3+N7qSG/Z1QRpBBdCAm7wFe05fPmA=";
                sparseCheckout = [ "ports/imguizmo" ];
              };
            in pkgs.stdenv.mkDerivation {
              pname = "imguizmo";
              version = "unstable-2024-11-14";

              src = pkgs.fetchFromGitHub {
                owner = "CedricGuillemet";
                repo = "ImGuizmo";
                rev = "b10e91756d32395f5c1fefd417899b657ed7cb88";
                hash = "sha256-nOMapXR2Gzz0yg5qqf0JW44JbwPkrLFxlZYQOK5dQnM=";
              };

              cmakeRules = "${vcpkgSource}/ports/imguizmo";
              postPatch = ''
                cp "$cmakeRules"/CMakeLists.txt ./
              '';

              nativeBuildInputs = [ pkgs.cmake ];
              buildInputs = [ imgui ];
            };
          in with pkgs;
          [ assimp glm glfw zlib nlohmann_json ] ++ [ imgui imguizmo ];
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
