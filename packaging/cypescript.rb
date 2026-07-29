# Homebrew formula for Cypescript.
#
# To publish:
#   1. Create a GitHub release with a source tarball (git tag v<x.y.z> && push)
#   2. Fill in `url` and `sha256` below (shasum -a 256 <tarball>)
#   3. Put this file in a tap repo: github.com/kr4ckhe4d/homebrew-cypescript
#      as Formula/cypescript.rb
#   4. Users install with:  brew tap kr4ckhe4d/cypescript && brew install cypescript
class Cypescript < Formula
  desc "TypeScript-style language that compiles to native code via LLVM"
  homepage "https://github.com/kr4ckhe4d/Cypescript"
  url "https://github.com/kr4ckhe4d/Cypescript/archive/refs/tags/v1.1.0.tar.gz"
  sha256 "fe97686d62da0f1efac33b2b5a91620b2ceb13a6ffb1061c89342707f594faf4"
  license "MIT"

  depends_on "cmake" => :build
  # llvm is a *runtime* dependency, not just a build one: since the linkage
  # change, cscript links libLLVM.dylib rather than static component archives.
  depends_on "llvm"
  # raylib likewise. The build vendors it with FetchContent by default, which
  # Homebrew refuses ("please use a formula dependency") because a build gets no
  # network — so this formula links the raylib formula instead, via
  # -DCYPESCRIPT_VENDOR_RAYLIB=OFF below.
  depends_on "raylib"

  def install
    system "cmake", "-B", "build", "-DCMAKE_BUILD_TYPE=Release",
           "-DCYPESCRIPT_VENDOR_RAYLIB=OFF", *std_cmake_args
    system "cmake", "--build", "build"

    bin.install "build/cscript"
    lib.install "build/libcypescript.a"

    # The game runtime, and the module of `declare` bindings that fronts it.
    # Without both, an installed cscript compiles ordinary programs but fails on
    # `import { } from "game";` — cscript looks for <prefix>/lib/cypescript/game.csc.
    lib.install "build/libcypescript_game.a"
    (lib/"cypescript").install "lib/game.csc"

    # game.csc says `link "raylib";`, and cscript passes -L only for its own lib
    # directory. Homebrew keeps raylib in its own prefix, which is not a default
    # linker search path on macOS, so compiling a game fails with
    # `ld: library 'raylib' not found`.
    #
    # Point the installed copy at the raylib formula. Symlinking libraylib.a
    # into our own lib/ would collide with raylib's linked file and leave this
    # formula unlinkable, so add a search path rather than duplicate the library.
    inreplace lib/"cypescript"/"game.csc",
              "link \"raylib\";",
              "link macos path \"#{formula_opt_lib("raylib")}\";\nlink \"raylib\";"
  end

  test do
    # A class with a method, so the test covers more than parsing a literal
    (testpath/"hello.csc").write <<~EOS
      class Greeter {
          name: string = "homebrew";
          greet(): string { return `hello from ${this.name}`; }
      }
      let g: Greeter = new Greeter();
      println(g.greet());
    EOS
    system bin/"cscript", "-o", testpath/"hello", testpath/"hello.csc"
    assert_equal "hello from homebrew", shell_output(testpath/"hello").strip

    # The game runtime is installed and linkable. Headless, so it needs no
    # display: the loop runs for one frame and exits.
    #
    # Not named game.csc: a source file of that name shadows the bundled "game"
    # module and imports itself, leaving every binding undefined.
    (testpath/"arcade.csc").write <<~EOS
      import { } from "game";
      openWindow(320, 240, "brew test");
      while (!windowShouldClose()) {
          beginFrame();
          clearScreen(rgb(0, 0, 0));
          endFrame();
      }
      closeWindow();
      println("game ok");
    EOS
    system bin/"cscript", "-o", testpath/"arcade", testpath/"arcade.csc"
    assert_equal "game ok",
                 shell_output("CYPS_HEADLESS=1 CYPS_FRAMES=1 #{testpath}/arcade").strip
  end
end
