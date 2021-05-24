DESCRIPTION = "gpio driver"

LICENSE = "GPLv2+"
LIC_FILES_CHKSUM = "file://${BPN}.c;endline=19;md5=02d0f4fb7e7b7125483125efd96a39dc"


inherit module


PR = "r0"


SRC_URI = "file://Makefile file://${BPN}.c"

S = "${WORKDIR}"
