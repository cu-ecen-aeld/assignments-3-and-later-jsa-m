#!/bin/bash
# Script outline to install and build kernel.
# Author: Joaquin Sopena.

set -e
set -u

OUTDIR=/tmp/aeld
KERNEL_REPO=git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git
KERNEL_VERSION=v5.15.163
BUSYBOX_VERSION=1_33_1
FINDER_APP_DIR=$(realpath $(dirname $0))
ARCH=arm64
CROSS_COMPILE=aarch64-none-linux-gnu-

if [ $# -lt 1 ]
then
	echo "Using default directory ${OUTDIR} for output"
else
	OUTDIR=$1
	echo "Using passed directory ${OUTDIR} for output"
fi

mkdir -p ${OUTDIR}

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/linux-stable" ]; then
    #Clone only if the repository does not exist.
	echo "CLONING GIT LINUX STABLE VERSION ${KERNEL_VERSION} IN ${OUTDIR}"
	git clone ${KERNEL_REPO} --depth 1 --single-branch --branch ${KERNEL_VERSION}
fi

#Kernel build steps here:

if [ ! -e ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ]; then
    cd linux-stable
    echo "Checking out version ${KERNEL_VERSION}"
    git checkout ${KERNEL_VERSION}

    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} mrproper # mrproper Remove all generated files + config + various backup files

    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} defconfig # Configure the kernel for ARM64

    make -j4 ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} all # Build a kernel image for booting with QEMU

    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} modules # Build any kernel modules

    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} dtbs #Build the device tree 

fi

#Create necessary base directories:

echo "Adding the Image in outdir"


cp ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ${OUTDIR}/

echo "Creating the staging directory for the root filesystem"
cd "$OUTDIR"
if [ -d "${OUTDIR}/rootfs" ]
then
	echo "Deleting rootfs directory at ${OUTDIR}/rootfs and starting over"
    sudo rm  -rf ${OUTDIR}/rootfs
fi

mkdir -p ${OUTDIR}/rootfs
cd rootfs
mkdir -p bin dev etc home lib lib64 proc sbin sys tmp usr var
mkdir -p usr/bin usr/lib usr/sbin
mkdir -p var/log

# TODO: Make and install busybox

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/busybox" ]
then
git clone git://busybox.net/busybox.git
    cd busybox
    git checkout ${BUSYBOX_VERSION}
    
    make distclean #This command cleans the BusyBox source tree, removing any remnants of previous builds

    make defconfig #This sets up a default configuration for BusyBox. It generates a baseline configuration file that includes a set of standard utilities

    #make menuconfig

    #This launches an interactive configuration menu where you can customize which utilities to include or tweak various settings. 
    #This step allows you to tailor BusyBox to your specific needs. Note: In an automated script, you might want to skip this 
    #step if no customization is needed, as it requires user interaction.

    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} #This command compiles BusyBox using the specified architecture

    make CONFIG_PREFIX=${OUTDIR}/rootfs ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} install #Tells BusyBox where to install its files (binaries, symlinks, etc.).
    # It will mirror the standard filesystem hierarchy 
else
    cd busybox
fi

echo "Library dependencies"
${CROSS_COMPILE}readelf -a ${OUTDIR}/busybox/busybox | grep "program interpreter"
${CROSS_COMPILE}readelf -a ${OUTDIR}/busybox/busybox | grep "Shared library"

# # TODO: Add library dependencies to rootfs

SYSROOT=$(${CROSS_COMPILE}gcc -print-sysroot)
echo "Sysroot is: ${SYSROOT}"

#Most cross-compilers provide a way to print the sysroot (the directory where the libraries and headers for the target system reside).

# Copy the dynamic linker:
cp -v ${SYSROOT}/lib/ld-linux-aarch64.so.1 ${OUTDIR}/rootfs/lib/

# Copy the shared libraries:
cp -v ${SYSROOT}/lib64/libc.so.6 ${OUTDIR}/rootfs/lib64/
cp -v ${SYSROOT}/lib64/libm.so.6 ${OUTDIR}/rootfs/lib64/
cp -v ${SYSROOT}/lib64/libresolv.so.2 ${OUTDIR}/rootfs/lib64/

# TODO: Make device nodes

sudo mknod -m 666 ${OUTDIR}/rootfs/dev/null c 1 3
sudo mknod -m 600 ${OUTDIR}/rootfs/dev/console c 5 1

# TODO: Clean and build the writer utility

cd "$FINDER_APP_DIR"

make cross

# TODO: Copy the finder related scripts and executables to the /home directory
# on the target rootfs

cp -v ${FINDER_APP_DIR}/writer ${OUTDIR}/rootfs/home/
cp -v ${FINDER_APP_DIR}/finder.sh ${OUTDIR}/rootfs/home/

cp -v ${FINDER_APP_DIR}/finder-test.sh ${OUTDIR}/rootfs/home/ 
cp -v ${FINDER_APP_DIR}/autorun-qemu.sh ${OUTDIR}/rootfs/home/ 

mkdir -p ${OUTDIR}/rootfs/home/conf
cp -v ${FINDER_APP_DIR}/conf/username.txt ${OUTDIR}/rootfs/home/conf/  #/conf?
cp -v ${FINDER_APP_DIR}/conf/assignment.txt ${OUTDIR}/rootfs/home/conf/ #/conf?

# TODO: Chown the root directory

sudo chown -R root:root ${OUTDIR}/rootfs 
# Change the owner of your root filesystem directory and its contents to root

# TODO: Create initramfs.cpio.gz

cd ${OUTDIR}/rootfs

find . | cpio -H newc -ov --owner root:root > ${OUTDIR}/initramfs.cpio

#Create Archive: You use find and cpio to create an archive of the entire root filesystem in the 
#newc format, ensuring all files are owned by root.

gzip -f ${OUTDIR}/initramfs.cpio

#Compress Archive: Finally, you compress the archive to produce initramfs.cpio.gz, which is the final 
#bootable image used by the kernel during startup.

