#!/bin/sh

./make.sh mrproper
./make.sh 622_defconfig
./make.sh uImage
./make.sh modules
./make.sh modules_install
