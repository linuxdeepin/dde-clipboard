// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CLIPBOARD_LOADER_PLUGIN_GLOBAL_H
#define CLIPBOARD_LOADER_PLUGIN_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(CLIPBOARD_LIBRARY)
#  define CLIPBOARDLOADERSHARED_EXPORT Q_DECL_EXPORT
#else
#  define CLIPBOARDLOADERSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // CLIPBOARD_LOADER_PLUGIN_GLOBAL_H
