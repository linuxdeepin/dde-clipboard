// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "commandservice.h"

CommandService::CommandService()
{

}

CommandService::~CommandService()
{

}

CommandService *CommandService::nextService() const
{
    return this->m_nextService;
}

void CommandService::setNextService(CommandService *service)
{
    this->m_nextService = service;
}
