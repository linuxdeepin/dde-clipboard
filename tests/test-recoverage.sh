#!/bin/bash

mkdir build/report -p
mkdir build/tmp -p

# dde-clipboardloader
lcov -c -i -d ../../build-dde-clipboard-unknown-Debug/tests/dde-clipboardloader -o build/tmp/init.info
./tests
lcov -c -d ../../build-dde-clipboard-unknown-Debug/tests/dde-clipboardloader -o build/tmp/cover.info
lcov -a build/tmp/init.info -a build/tmp/cover.info -o build/tmp/total.info
lcov --remove build/tmp/total.info '*/usr/include/*' '*/usr/lib/*' '*/usr/lib64/*' '*/usr/local/include/*' '*/usr/local/lib/*' '*/usr/local/lib64/*' '*/tests/*' -o final.info
# 生成报告
genhtml -o cover_report --legend --title "lcov"  --prefix=./ final.info
#打开报告
nohup x-www-browser ./cover_report/index.html &

# dde-clipboard
lcov -c -i -d ../../build-dde-clipboard-unknown-Debug/tests/dde-clipboard -o build/tmp/init.info
./tests
lcov -c -d ../../build-dde-clipboard-unknown-Debug/tests/dde-clipboard -o build/tmp/cover.info
lcov -a build/tmp/init.info -a build/tmp/cover.info -o build/tmp/total.info
lcov --remove build/tmp/total.info '*/usr/include/*' '*/usr/lib/*' '*/usr/lib64/*' '*/usr/local/include/*' '*/usr/local/lib/*' '*/usr/local/lib64/*' '*/tests/*' -o final.info
# 生成报告
genhtml -o cover_report --legend --title "lcov"  --prefix=./ final.info
#打开报告
nohup x-www-browser ./cover_report/index.html &

exit 0
