export KBUILD_BUILD_USER="h0sch180"
export KBUILD_BUILD_HOST="ANUBIS"
export CROSS_COMPILE=/android/aarch64-linux-android-4.9/bin/aarch64-linux-android-

export ARCH=arm64

make clean && make mrproper

make k5fpr_defconfig

make -j24
