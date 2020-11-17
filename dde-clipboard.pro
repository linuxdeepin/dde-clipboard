TEMPLATE = subdirs

SUBDIRS +=  $$PWD/dde-clipboard/dde-clipboard.pro \
            $$PWD/dde-clipboardloader/dde-clipboardloader.pro \
            $$PWD/test/dde-clipboard/tst_dde-clipboard.pro \
            $$PWD/test/dde-clipboardloader/tst_dde-clipboardloader.pro

system(./remove.sh): error("Failed to clear junk files")

# 更新翻译ts文件
CONFIG(release, debug|release) {
    !system(./translate_generation.sh): error("Failed to generate translation")
}
