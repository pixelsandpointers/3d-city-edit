{
  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";

  outputs = { self, nixpkgs }:
    let pkgs = nixpkgs.legacyPackages.x86_64-linux;
    in {
      formatter.x86_64-linux = pkgs.nixfmt-classic;

      packages.x86_64-linux.default = pkgs.clang18Stdenv.mkDerivation {
        name = "3d";
        src = ./.;
        nativeBuildInputs = with pkgs; [ cmake ninja pkg-config ];
        buildInputs = with pkgs; [
          assimp
          glm
          glfw
          imgui

          libGL
          libffi
          libxkbcommon
          wayland
          wayland-protocols
          wayland-scanner
          xorg.libX11
          xorg.libXcursor
          xorg.libXext
          xorg.libXi
          xorg.libXinerama
          xorg.libXrandr
        ];
      };

      devShells.x86_64-linux.default =
        self.packages.x86_64-linux.default.overrideAttrs (old: {
          nativeBuildInputs = old.nativeBuildInputs ++ [ pkgs.ccache ];
          ASAN_OPTIONS = "symbolize=1";
          ASAN_SYMBOLIZER_PATH = "${pkgs.llvm}/bin/llvm-symbolizer";
          LD_LIBRARY_PATH = pkgs.lib.makeLibraryPath old.buildInputs;
          CMAKE_BUILD_TYPE = "Debug";
          hardeningDisable = [ "fortify" ];
        });
    };
}
