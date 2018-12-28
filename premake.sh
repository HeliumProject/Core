DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

if [ ! -f $DIR/Dependencies/premake/bin/release/premake5 ]; then

	pushd $DIR/Dependencies/premake > /dev/null

	if [ `uname` == "Darwin" ]; then
		make -f Bootstrap.mak osx
	elif [ `uname` == "Linux" ]; then
		make -f Bootstrap.mak linux
	else
		VSINSTALL=`"/c/Program Files (x86)/Microsoft Visual Studio/Installer/vswhere.exe" -property installationPath`
		VSINSTALL_BASH=`cygpath -a "$VSINSTALL"`
		cmd //c "$VSINSTALL_BASH/VC/Auxiliary/Build/vcvars64.bat" \&\& nmake -f Bootstrap.mak windows
	fi

	popd > /dev/null

fi

$DIR/Dependencies/premake/bin/release/premake5 "$@"
