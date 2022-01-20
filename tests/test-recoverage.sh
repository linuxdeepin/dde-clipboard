#!/bin/bash

BUILD_DIR=build
REPORT_DIR=report

cd ../
rm -rf $BUILD_DIR
mkdir $BUILD_DIR
cd $BUILD_DIR

cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j 8

lcov -c -i -d ./ -o init.info
./ut-dde-clipboard --gtest_output=xml:ut-dde-clipboard.xml
./ut-dde-clipboardloader --gtest_output=xml:ut-dde-clipboardloader.xml
lcov -c -d ./ -o cover.info
lcov -a init.info -a cover.info -o total.info
lcov -r total.info "*/tests/*" "*/usr/include*" '*tests*' -o final.info

rm -rf ../tests/$REPORT_DIR
mkdir -p ../tests/$REPORT_DIR
genhtml -o ../tests/$REPORT_DIR final.info

mv asan_board.log* asan_dde-clipboard.log
mv asan_loader.log* asan_dde-clipboardloader.log