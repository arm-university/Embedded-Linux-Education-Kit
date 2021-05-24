DESCRIPTION = "ARM DS5 Gator kernel module and daemon"
AUTHOR = "Massimo Violante <massimo.violante@polito.it>"
SECTION = "kernel"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM="file://driver/COPYING;md5=b234ee4d69f5fce4486a80fdaf4a4263"
S="${WORKDIR}/git"
BP="${BPN}"
DEPENDS = "linux-raspberrypi"
inherit module

SRC_URI = "git://github.com/ARM-software/gator.git;protocol=https;rev=6.1 \
           file://0001-patch-daemon-Makefile.patch"
INHIBIT_PACKAGE_DEBUG_SPLIT = "1"
INHIBIT_PACKAGE_DEBUG_SPLIT-dev = "1"

INSANE_SKIP_${PN}-dev += " ldflags"
INSANE_SKIP_${PN} += " ldflags"

do_compile() {
        cd ${S}/driver
        ${MAKE} -C ${STAGING_KERNEL_DIR} M=`pwd` ARCH=${TARGET_ARCH} CROSS_COMPILE=${TARGET_PREFIX} modules
        unset CFLAGS CPPFLAGS CXXFLAGS LDFLAGS MACHINE
        cd ${S}/daemon
        ${MAKE}
}

do_install() {
        INIT_DIR=${D}${sysconfdir}/init.d/
        install -d ${INIT_DIR}
        install -m 0644 ${S}/driver/gator.ko ${INIT_DIR}
        install -m 0755 ${S}/daemon/gatord ${INIT_DIR}/gatord
        echo "#!/bin/bash\n/etc/init.d/gatord &" > ${INIT_DIR}/rungator.sh
        chmod a+x ${INIT_DIR}/rungator.sh
}

FILES_${PN} = "${sysconfdir}/init.d/gator.ko ${sysconfdir}/init.d/gatord ${sysconfdir}/init.d/rungator.sh"
