/* Copyright (C) 2007 Jan Kundrát <jkt@gentoo.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef IMAP_SOCKETFACTORY_H
#define IMAP_SOCKETFACTORY_H

#include <tr1/memory>
#include <QStringList>
#include "Imap/Socket.h"

/** @short Namespace for IMAP interaction */
namespace Imap {

/** @short Classes for handling of mailboxes and connections */
namespace Mailbox {

/** @short Abstract interface for creating new socket that is somehow connected
 * to the IMAP server */
class SocketFactory {
public:
    virtual ~SocketFactory() {};
    /** @short Create new socket and return a smart pointer to it */
    virtual Imap::SocketPtr create() = 0;
};

typedef std::auto_ptr<SocketFactory> SocketFactoryPtr;

/** @short Manufacture sockets based on QProcess */
class ProcessSocketFactory: public SocketFactory {
    /** @short Name of executable file to launch */
    QString _executable;
    /** @short Arguments to launch the process with */
    QStringList _args;
public:
    ProcessSocketFactory( const QString& executable, const QStringList& args );
    virtual Imap::SocketPtr create();
};

}

}

#endif /* IMAP_SOCKETFACTORY_H */
