#!/bin/bash
build_locales() {
        make -C locales
}
build() {
        mkdir -p build/$1
        cd build/$1
        cmake ${@:2} -DCMAKE_BUILD_TYPE=Release -DDEBUG_VERBOSE=ON ../..
        make
        make install DESTDIR=../../dist/$1
        cd ../..
}
package() {
        cp -p /var/www/vtrains/ETCS/ETCS_$1.zip /var/www/vtrains/ETCS/ETCS_$1_old.zip
        cd dist/$1
        zip -FSr /var/www/vtrains/ETCS/ETCS_$1.zip *
        cd ../..
}
build_linux_appimage() {
        build linux-appimage -DETCS_VENDORED=OFF -DETCS_PACK_VENDORED=ON -DCMAKE_INSTALL_LIBDIR=lib -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_INSTALL_RPATH='$ORIGIN/../lib'
        ../linuxdeploy-x86_64.AppImage --appdir=dist/linux-appimage --icon-file=utils/ETCS.png --desktop-file=utils/ETCS.desktop
        ../appimagetool-x86_64.AppImage dist/linux-appimage /var/www/vtrains/ETCS/ETCS-x86_64.AppImage
}
build_android() {
        ./gradlew build
        mv /var/www/vtrains/ETCS/ETCS_Android.apk /var/www/vtrains/ETCS/ETCS_Android_old.apk
        mv app/build/outputs/apk/debug/app-debug.apk /var/www/vtrains/ETCS/ETCS_Android.apk
}
build_win32() {
        build win32 -DETCS_VENDORED=ON -DETCS_PACK_VENDORED=ON -DCMAKE_TOOLCHAIN_FILE=utils/TC-mingw.cmake -DCMAKE_INSTALL_PREFIX=/
        cp -p $(x86_64-w64-mingw32-gcc -print-file-name=libwinpthread-1.dll) dist/win32/
        cp -p $(x86_64-w64-mingw32-gcc -print-file-name=libstdc++-6.dll) dist/win32/
        cp -p $(x86_64-w64-mingw32-gcc -print-file-name=libgcc_s_seh-1.dll) dist/win32/
        package win32
}
ETCS_DIR="${ETCS_DIR:-$HOME/ETCS}"
cd $ETCS_DIR
git fetch
if [[ $(LANG="C" git status) != *"Your branch is up to date"* || "$1" == "-f" || "$1" == "--force" ]]; then
        git pull

        build_locales

        for arg in "$@"; do
                case $arg in
                        win32)
                                build_win32
                        ;;
                        linux)
                                build linux
                                package linux
                        ;;
                        linux_appimage)
                                build_linux_appimage
                        ;;
                        aarch64)
                                build aarch64 -DCMAKE_TOOLCHAIN_FILE=utils/TC-aarch64.cmake
                                package aarch64
                        ;;
                        android)
                                build_android
                        ;;
                        -f|--force)
                        ;;
                        *)
                        echo "Unknown target $arg"
                        ;;
                esac
        done
fi

