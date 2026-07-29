# Homebrew formula template for Cypescript.
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
  depends_on "llvm"

  def install
    system "cmake", "-B", "build", "-DCMAKE_BUILD_TYPE=Release", *std_cmake_args
    system "cmake", "--build", "build"
    bin.install "build/cscript"
    lib.install "build/libcypescript.a"
  end

  test do
    (testpath/"hello.csc").write <<~EOS
      println("hello from homebrew");
    EOS
    system bin/"cscript", "-o", testpath/"hello", testpath/"hello.csc"
    assert_equal "hello from homebrew", shell_output(testpath/"hello").strip
  end
end
