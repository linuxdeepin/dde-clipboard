#!/bin/bash

BUILD_DIR=build
REPORT_DIR=report
#EXTRACT_ARGS="src"

cd ../
rm -rf $BUILD_DIR
mkdir $BUILD_DIR
cd $BUILD_DIR
qmake CONFIG+=debug ../dde-clipboard/
make
qmake CONFIG+=debug ../dde-clipboardloader/
make

cd ../tests/
rm -rf $BUILD_DIR
mkdir $BUILD_DIR
cd $BUILD_DIR
qmake CONFIG+=debug ../dde-clipboard/
TESTARGS="--gtest_output=xml:dde_test_report_clipboard.xml"  make check -j$(nproc)
mv asan_loader.log* asan_dde-clipboard.log
qmake CONFIG+=debug ../dde-clipboardloader/
TESTARGS="--gtest_output=xml:dde_test_report_clipboardloader.xml"  make check -j$(nproc)
mv asan_loader.log* asan_dde-clipboardloader.log

lcov -d ./ -c -o coverage_all.info
#lcov --extract coverage_all.info $EXTRACT_ARGS --output-file coverage.info
lcov --remove coverage_all.info "*/tests/*" "*/usr/include*" "*build/src*" "*/dde-clipboard/dbus/*" --output-file coverage.info
cd ..
genhtml -o $REPORT_DIR $BUILD_DIR/coverage.info
#rm -rf $BUILD_DIR
#rm -rf ../$BUILD_DIR
