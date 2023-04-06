DESCRIPTION = "hello driver"
LICENSE = "GPLv2+"
LIC_FILES_CHKSUM = "file://${BPN}.c;endline=19;md5=4866f9824d27c1cd5324fd5e84caeb6e"

inherit module

PR = "r0"

SRC_URI = "file://Makefile file://${BPN}.c"
S = "${WORKDIR}"
