// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMMANDMESSAGE
#define COMMANDMESSAGE

class CommandMessage
{
public:
    enum Error {
        NoError,
        ExecuteError
    };

    CommandMessage() = default;
    virtual ~CommandMessage() {}

    inline Error error() const {
        return m_error;
    }

    inline void setError(Error err) {
        m_error = err;
    }

protected:
    Error m_error = NoError;
};

#endif  // COMMANDMESSAGE
