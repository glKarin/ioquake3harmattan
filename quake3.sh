#!/bin/sh

VER="build"
RUN="run"
Q3="ioquake3.arm"
Q3_DED="ioq3ded.arm"
OUT_DIR="${VER}/release-linux-arm"

for i in $*
do
	case ${i} in
		"clean")
			echo "make clean..."
			make clean;
			exit 0;
			;;
		"server")
			Q3=${Q3_DED}
			;;
		"build")
			echo "dpkg-buildpackage";
			dpkg-buildpackage;
			exit 0;
			;;
		"debug")
			RUN="debug"
			;;
		*)
			;;
	esac
done

echo "${RUN}..."

#rm -f ~user/baseq3/*.so
#cp -r "${PWD}/${OUT_DIR}/baseq3/*.so" ~user/baseq3

cp -r resc "${PWD}/${OUT_DIR}"

cd "${PWD}/${OUT_DIR}"

if [ x${RUN} == "xdebug" ]; then
	echo "gdb --args ./${Q3} $*"
	gdb --args ./${Q3} $*
else
	echo "./${Q3} $*"
	./${Q3} $*
fi
