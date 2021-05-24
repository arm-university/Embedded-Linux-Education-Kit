DESCRIPTION = "hcsr04 driver"
LICENSE = "GPLv2+"
LIC_FILES_CHKSUM = "file://${BPN}.c;md5=94af90acd77ed5583caa62cbe903e559"

inherit module

PR = "r0"

SRC_URI = "file://Makefile file://${BPN}.c"
S = "${WORKDIR}"
