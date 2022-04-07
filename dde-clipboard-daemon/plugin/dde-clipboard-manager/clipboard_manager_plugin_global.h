/*
 * Copyright (C) 2019 ~ 2022 Uniontech Technology Co., Ltd.
 *
 * Author:     fanpengcheng <fanpengcheng@uniontech.com>
 *
 * Maintainer: fanpengcheng <fanpengcheng@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef CLIPBOARD_MANAGER_PLUGIN_GLOBAL_H
#define CLIPBOARD_MANAGER_PLUGIN_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(CLIPBOARDMANAGER_LIBRARY)
#  define CLIPBOARDMANAGERSHARED_EXPORT Q_DECL_EXPORT
#else
#  define CLIPBOARDMANAGERSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // CLIPBOARD_MANAGER_PLUGIN_GLOBAL_H
