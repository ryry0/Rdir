with import <nixpkgs> {}; {
	rdirEnv = stdenv.mkDerivation {
		name = "rdir-env";
		shellHook = ''
		export RPROMPT=[rdir-shell]$RPROMPT
		'';

		buildInputs = [
		        ncurses
		];
	};
}

