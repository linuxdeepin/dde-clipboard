// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CLIPBOARD_MANAGER_PLUGIN_H
#define CLIPBOARD_MANAGER_PLUGIN_H

#include "clipboard_manager_plugin_global.h"

typedef void (*UnloadFun)(const char *);

extern "C"  CLIPBOARDMANAGERSHARED_EXPORT bool Start();
extern "C"  CLIPBOARDMANAGERSHARED_EXPORT bool Stop();
extern "C"  CLIPBOARDMANAGERSHARED_EXPORT const char *Info();
extern "C"  CLIPBOARDMANAGERSHARED_EXPORT void UnloadCallBack(UnloadFun fun);

#endif // CLIPBOARD_MANAGER_PLUGIN_H
