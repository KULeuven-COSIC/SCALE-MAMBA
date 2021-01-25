if ! which rustup >/dev/null 2>&1; then
	curl https://sh.rustup.rs -sSf | sh -s -- -y
	source ~/.cargo/env
fi

rustup update stable
rustup update nightly
rustup target add wasm32-unknown-unknown --toolchain stable
rustup target add wasm32-unknown-unknown --toolchain nightly

# While ink! is pinned to a specific nightly version of the Rust compiler you will need to explicitly install that toolchain.
rustup install nightly-2019-05-21
rustup target add wasm32-unknown-unknown --toolchain nightly-2019-05-21

echo "Installing wasm-prune into ~/.cargo/bin"
cargo install pwasm-utils-cli --bin wasm-prune --force

# Copy WASM binaries after successful rust/cargo install.
if [[ "$OSTYPE" == "linux-gnu" ]]; then
	set -e

	BUILD_NUM=`curl -s https://storage.googleapis.com/wasm-llvm/builds/linux/lkgr.json | jq -r '.build'`
	if [ -z ${BUILD_NUM+x} ]; then
		echo "Could not fetch the latest build number.";
		exit 1;
	fi

	tmp=`mktemp -d`
	pushd $tmp > /dev/null
	echo "Downloading wasm-binaries.tbz2";
	curl -L -o wasm-binaries.tbz2 https://storage.googleapis.com/wasm-llvm/builds/linux/$BUILD_NUM/wasm-binaries.tbz2

	declare -a binaries=("wasm2wat" "wat2wasm") # Default binaries
	if [ "$#" -ne 0 ]; then
		echo "Installing selected binaries.";
		binaries=("$@");
	else
		echo "Installing default binaries.";
	fi

	for bin in "${binaries[@]}"
	do
		echo "Installing $bin into ~/.cargo/bin"
		tar -xvjf wasm-binaries.tbz2 wasm-install/bin/$bin > /dev/null
		cp -f wasm-install/bin/$bin ~/.cargo/bin/
	done
	popd > /dev/null
fi

echo ""
echo "Run source ~/.cargo/env now to update environment."
echo ""
