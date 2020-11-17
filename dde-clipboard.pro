TEMPLATE = subdirs

SUBDIRS +=  $$PWD/dde-clipboard/dde-clipboard.pro \
            $$PWD/dde-clipboardloader/dde-clipboardloader.pro

# 更新翻译ts文件
CONFIG(release, debug|release) {
    !system(./translate_generation.sh): error("Failed to generate translation")
}

