#!/bin/sh

set -eu

CC=$1
CFLAGS=$2
unix=$3
system=$4

cd "libuv"

if [ "$unix" = "true" ]; then
    if [ ! -f .libs/libuv.a ]; then
        x="$(uname)"
        if [ "$x" = "OpenBSD" ] ; then
            AUTOCONF_VERSION="$( ls -1 /usr/local/bin/autoreconf-* | sort | tail -n 1 )"
            AUTOCONF_VERSION="${AUTOCONF_VERSION##*-}"

            AUTOMAKE_VERSION="$( ls -1 /usr/local/bin/automake-* | sort | tail -n 1 )"
            AUTOMAKE_VERSION="${AUTOMAKE_VERSION##*-}"

            export AUTOCONF_VERSION AUTOMAKE_VERSION
        fi
        if [ ! -x ./configure ]; then
            ./autogen.sh
        fi

        make=gmake
        if ! which gmake >/dev/null 2>&1 ; then
            make=make
        fi
        if [ -f Makefile ]; then
            $make clean || true
            $make distclean || true
        fi
        ./configure --enable-static --disable-shared CC="$CC" CFLAGS="$CFLAGS -DNDEBUG"
        $make all CC="$CC" CFLAGS="$CFLAGS -DNDEBUG"
    fi
    cd ..
    lib="../libuv/.libs/libuv.a"
    rm -f src/libuv.a examples/libuv.a test/libuv.a
    ln -s "${lib}" src/libuv.a
    ln -s "${lib}" examples/libuv.a
    ln -s "${lib}" test/libuv.a
else
    if [ "$system" = "win32" ] || [ "$system" = "win64" ] ; then
        if [ ! -f Release/lib/libuv.lib ]; then
            if [ ! -f libuv32.vcxproj ]; then
                # checkout doesn't work at the moment. They've enabled
                # options inside common.gypi that lead to an ouptut
                # that is not understood by flexlink. Changing them via
                # grep/sed is too fragile. Things to disable: LTCG /
                # WholeProgramOptimization
                target_arch=x64
                if [ "$system" = "win32" ]; then
                    target_arch=ia32
                fi
                set +u
                if [ "$PYTHON" = "" ]; then
                    # note: cygwin's python doesn't seem to work at the moment
                    PYTHON=python
                fi
                if [ "$GIT" = "" ]; then
                    GIT=git
                fi
                set -u
                if [ ! -d build/gyp ]; then
                    "$GIT" clone --depth=1 https://chromium.googlesource.com/external/gyp build/gyp
                fi
                "$PYTHON" gyp_uv.py -Dtarget_arch="$target_arch" -Duv_library=static_library
            else
                if [ "$system" = "win32" ]; then
                    cp libuv32.vcxproj libuv.vcxproj
                else
                    cp libuv64.vcxproj libuv.vcxproj
                fi
            fi
            MSBuild.exe libuv.vcxproj '/t:Build' '/p:Configuration=Release' '/clp:NoSummary;NoItemAndPropertyList;Verbosity=minimal' '/nologo'
        fi
        lib="libuv/Release/lib/libuv.lib"
        ext_lib=".lib"
    else
        if [ ! -f .libs/libuv.a ]; then
            if [ ! -x ./configure ]; then
                ./autogen.sh
            fi
            case "$system" in
                mingw64*)
                    host='x86_64-w64-mingw32'
                    ;;
                *)
                    host='i686-w64-mingw32'
                    ;;
            esac
            ./configure --host=$host --enable-static --disable-shared CC="$CC" CFLAGS="$CFLAGS -D_WIN32_WINNT=0x0600 -DNDEBUG" ac_cv_search_pthread_create=no
            set +u
            jflags=
            if [ -n "$NUMBER_OF_PROCESSORS" ] && [ -z "$MAKEFLAGS" ]; then
                case "$NUMBER_OF_PROCESSORS" in
                    ''|*[!0-9]*)
                        jflags=
                        ;;
                    *)
                        jflags="-j$NUMBER_OF_PROCESSORS"
                        ;;
                esac
            fi
            set -u
            make all CC="$CC" CFLAGS="$CFLAGS -D_WIN32_WINNT=0x0600 -DNDEBUG" $jflags
        fi
        lib="libuv/.libs/libuv.a"
        ext_lib=".a"
    fi
    cd ..
    rm -f src/libuv"${ext_lib}" examples/libuv"${ext_lib}" test/libuv"${ext_lib}"
    cp "${lib}" src/libuv"${ext_lib}"
    cp "${lib}" examples/libuv"${ext_lib}"
    cp "${lib}" test/libuv"${ext_lib}"
fi
