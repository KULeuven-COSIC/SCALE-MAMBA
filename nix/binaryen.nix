with import <nixpkgs> {};

stdenv.mkDerivation rec {
  pname = "binaryen";
  version = "100";

  src = fetchFromGitHub {
    owner = "WebAssembly";
    repo = "binaryen";
    rev = "version_${version}";
    sha256 = "0lxqvibli0wg6y53yzfpi250d9adpw9a2jmyrccvsqdsq75ww2qn";
  };
  nativeBuildInputs = [ cmake python3 ];

  meta = with lib; {
    homepage = "https://github.com/WebAssembly/binaryen";
    description = "Compiler infrastructure and toolchain library for WebAssembly, in C++";
    platforms = platforms.all;
    maintainers = with maintainers; [ bnjjj ];
    license = licenses.asl20;
  };

  passthru.tests = {
    inherit emscripten;
  };
}
