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

#include <QPair>
#include <QStringList>
#include <QVariant>
#include "Imap/LowLevelParser.h"
#include "Imap/Exceptions.h"
#include "Imap/rfccodecs.h"

namespace Imap {
namespace LowLevelParser {

uint getUInt( const QByteArray& line, int& start )
{
    if ( start == line.size() )
        throw NoData( line, start );

    QByteArray item;
    bool breakIt = false;
    while ( !breakIt && start < line.size() ) {
        switch (line[start]) {
            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
                item.append( line[start] );
                ++start;
                break;
            default:
                breakIt = true;
                break;
        }
    }

    bool ok;
    uint number = item.toUInt( &ok );
    if (!ok)
        throw ParseError( line, start );
    return number;
}

QByteArray getAtom( const QByteArray& line, int& start )
{
    if ( start == line.size() )
        throw NoData( line, start );

    int old(start);
    bool breakIt = false;
    while (!breakIt && start < line.size() ) {
        if ( line[start] <= '\x1f' ) {
            // CTL characters (excluding 0x7f) as defined in ABNF
            breakIt = true;
            break;
        }
        switch (line[start]) {
            case '(': case ')': case '{': case '\x20': case '\x7f':
            case '%': case '*': case '"': case '\\': case ']':
                breakIt  = true;
                break;
            default:
                ++start;
        }
    }

    if ( old == start )
        throw ParseError( line, start );
    return line.mid( old, start - old );
}

QPair<QByteArray,ParsedAs> getString( const QByteArray& line, int& start )
{
    if ( start == line.size() )
        throw NoData( line, start );

    if ( line[start] == '"' ) {
        // quoted string
        ++start;
        bool escaping = false;
        QByteArray res;
        bool terminated = false;
        while ( start != line.size() && !terminated ) {
            if (escaping) {
                escaping = false;
                if ( line[start] == '"' || line[start] == '\\' )
                    res.append( line[start] );
                else
                    throw UnexpectedHere( line, start );
            } else {
                switch (line[start]) {
                    case '"':
                        terminated = true;
                        break;
                    case '\\':
                        escaping = true;
                        break;
                    case '\r': case '\n':
                        throw ParseError( line, start );
                    default:
                        res.append( line[start] );
                }
            }
            ++start;
        }
        if (!terminated)
            throw NoData( line, start );
        return qMakePair( res, QUOTED );
    } else if ( line[start] == '{' ) {
        // literal
        ++start;
        int size = getUInt( line, start );
        if ( line.mid( start, 3 ) != "}\r\n" )
            throw ParseError( line, start );
        start += 3;
        if ( start >= line.size() + size )
            throw NoData( line, start );
        int old(start);
        start += size;
        return qMakePair( line.mid(old, size), LITERAL );
    } else
        throw UnexpectedHere( line, start );
}

QPair<QByteArray,ParsedAs> getAString( const QByteArray& line, int& start )
{
    if ( start == line.size() )
        throw NoData( line, start );

    if ( line[start] == '{' || line[start] == '"' )
        return getString( line, start );
    else
        return qMakePair( getAtom( line, start ), ATOM );
}

QPair<QByteArray,ParsedAs> getNString( const QByteArray& line, int& start )
{
    QPair<QByteArray,ParsedAs> r = getAString( line, start );
    if ( r.second == ATOM && r.first.toUpper() == "NIL" ) {
        r.first.clear();
        r.second = NIL;
    }
    return r;
}

QString getMailbox( const QByteArray& line, int& start )
{
    QPair<QByteArray,ParsedAs> r = getAString( line, start );
    if ( r.second == ATOM && r.first.toUpper() == "INBOX" )
        return "INBOX";
    else
        return KIMAP::decodeImapFolderName( r.first );

}

QVariantList parseList( const char open, const char close,
        const QByteArray& line, int& start )
{
    if ( start >= line.size() )
        throw NoData( line, start );

    if ( line[start] == open ) {
        // found the opening parenthesis
        ++start;
        if ( start == line.size() )
            throw ParseError( line, start );

        QVariantList res;
        if ( line[start] == close ) {
            ++start;
            return res;
        }
        while ( line[start] != close ) {
            res.append( getAnything( line, start ) );
            if ( start == line.size() )
                throw NoData( line, start ); // truncated list
            if ( line[start] == close ) {
                ++start;
                return res;
            }
            ++start;
        }
        return res;
    } else
        throw UnexpectedHere( line, start );
}

QVariant getAnything( const QByteArray& line, int& start )
{
    if ( start >= line.size() )
        throw NoData( line, start );

    if ( line[start] == '[' ) {
        QVariant res = parseList( '[', ']', line, start );
        return res;
    } else if ( line[start] == '(' ) {
        QVariant res = parseList( '(', ')', line, start );
        return res;
    } else if ( line[start] == '"' || line[start] == '{' ) {
        QPair<QByteArray,ParsedAs> res = getString( line, start );
        return res.first;
    } else if ( line[start] == '\\' ) {
        // valid for "flag"
        ++start;
        if ( start >= line.size() )
            throw NoData( line, start );
        if ( line[start] == '*' ) {
            ++start;
            return QByteArray( "\\*" );
        }
        return QByteArray( 1, '\\' ) + getAtom( line, start );
    } else {
        try {
            return getUInt( line, start );
        } catch ( ParseError& e ) {
            return getAtom( line, start );
        }
    }
}

}
}
