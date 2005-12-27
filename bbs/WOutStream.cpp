/**************************************************************************/
/*                                                                        */
/*                              WWIV Version 5.0x                         */
/*             Copyright (C)1998-2005, WWIV Software Services             */
/*                                                                        */
/*    Licensed  under the  Apache License, Version  2.0 (the "License");  */
/*    you may not use this  file  except in compliance with the License.  */
/*    You may obtain a copy of the License at                             */
/*                                                                        */
/*                http://www.apache.org/licenses/LICENSE-2.0              */
/*                                                                        */
/*    Unless  required  by  applicable  law  or agreed to  in  writing,   */
/*    software  distributed  under  the  License  is  distributed on an   */
/*    "AS IS"  BASIS, WITHOUT  WARRANTIES  OR  CONDITIONS OF ANY  KIND,   */
/*    either  express  or implied.  See  the  License for  the specific   */
/*    language governing permissions and limitations under the License.   */
/*                                                                        */
/**************************************************************************/
#include "wwiv.h"
#include "WOutStreamBuffer.h"
#include "WStringUtils.h"

using std::ostream;

std::ostream::int_type WOutStreamBuffer::overflow( std::ostream::int_type c )
{
    if ( c != EOF )
    {
        bputch( static_cast<char>( c ), false );
    }
    return c;
}

std::streamsize WOutStreamBuffer::xsputn( const char *pszText, std::streamsize numChars )
{
    if ( numChars == 0 )
    {
        return 0;
    }
    char szBuffer[ 4096 ];
    strncpy( szBuffer, pszText, 4096 );
    szBuffer[ numChars ] = '\0';
    return GetSession()->bout.Write( pszText );
}


void WOutStream::Color(int wwivColor)
{
    unsigned char c = '\0';

    if ( wwivColor <= -1 && wwivColor >= -16 )
    {
        c = ( GetSession()->GetCurrentUser()->HasColor() ?
        rescolor.resx[207 + abs(wwivColor)] : GetSession()->GetCurrentUser()->GetBWColor( 0 ) );
    }
    if ( wwivColor >= 0 && wwivColor <= 9 )
    {
        c = ( GetSession()->GetCurrentUser()->HasColor() ?
        GetSession()->GetCurrentUser()->GetColor( wwivColor ) : GetSession()->GetCurrentUser()->GetBWColor( wwivColor ) );
    }
    if ( wwivColor >= 10 && wwivColor <= 207 )
    {
        c = ( GetSession()->GetCurrentUser()->HasColor() ?
        rescolor.resx[wwivColor - 10] : GetSession()->GetCurrentUser()->GetBWColor( 0 ) );
    }
    if ( c == curatr )
    {
        return;
    }

    GetSession()->bout.SystemColor( c );

    makeansi( GetSession()->GetCurrentUser()->HasColor() ?
        GetSession()->GetCurrentUser()->GetColor( 0 ) : GetSession()->GetCurrentUser()->GetBWColor( 0 ), endofline, false );
}


void WOutStream::ResetColors()
{
    // ANSI Clear Attributes String
    Write( "\x1b[0m" );
}


void WOutStream::GotoXY(int x, int y)
{
    if ( okansi() )
    {
		y = std::min<int>( y, GetSession()->screenlinest );	// Don't get Y get too big or mTelnet will not be happy
        *this << "\x1b[" << y << ";" << x << "H";
    }
}


void WOutStream::NewLine(int nNumLines)
{
    for (int i = 0; i < nNumLines; i++)
    {
        if (endofline[0])
	    {
            Write(endofline);
            endofline[0] = '\0';
        }
        Write("\r\n");
        // TODO Change this to fire a notification to a Subject
        // that we should process instant messages now.
        if ( inst_msg_waiting() && !bChatLine )
	    {
            process_inst_msgs();
	    }
    }
}


void WOutStream::BackSpace()
{
    bool bSavedEcho = echo;
    echo = true;
    Write("\b \b");
    echo = bSavedEcho;
}


void WOutStream::SystemColor( int nColor )
{
    char szBuffer[255];
    makeansi( nColor, szBuffer, false );
    Write( szBuffer );
}


void WOutStream::DisplayLiteBar(const char *pszFormatText,...)
{
    va_list ap;
    char s[1024], s1[1024];

    va_start( ap, pszFormatText );
    vsnprintf( s, sizeof( s ), pszFormatText, ap );
    va_end( ap );

    if ( strlen( s ) % 2 != 0 )
    {
        strcat( s, " " );
    }
    int i = ( 74 - strlen( s ) ) / 2;
    if ( okansi() )
    {
        snprintf( s1, sizeof( s1 ), "%s%s%s", charstr( i, ' ' ), stripcolors( s ), charstr( i, ' ' ) );
		*this << "\x1B[0;1;37m" << charstr( strlen( s1 ) + 4, '�' ) << wwiv::endl;
        *this << "\x1B[0;34;47m  " << s1 << "  \x1B[40m\r\n";
		*this << "\x1B[0;1;30m" << charstr( strlen( s1 ) + 4, '�' ) << wwiv::endl;
    }
    else
    {
		*this << charstr( i, ' ' ) << s << wwiv::endl;
    }
}


void WOutStream::BackLine()
{
    Color( 0 );
    bputch(SPACE);
    for (int i = localIO()->WhereX(); i > 0; i--)
    {
        this->BackSpace();
    }
}


/**
 * Clears the local and remote screen using ANSI (if enabled), otherwise DEC 12
 */
void WOutStream::ClearScreen()
{
    if ( okansi() )
    {
        Write( "\x1b[2J" );
        GotoXY( 1, 1 );
    }
    else
    {
		bputch( CL );
    }
}


/**
 * Moves the cursor to the end of the line using ANSI sequences.  If the user
 * does not have ansi, this this function does nothing.
 */
void WOutStream::ClearEOL()
{
	if ( okansi() )
	{
		Write( "\x1b[K" );
	}
}


void WOutStream::ColorizedInputField( int nNumberOfChars )
{
    if ( okansi() )
    {
        Color( 4 );
        for ( int i = 0; i < nNumberOfChars; i++ )
        {
            bputch( ' ', true );
        }
        FlushOutComChBuffer();
        *this << "\x1b[" << nNumberOfChars << "D";
    }
}


int WOutStream::Write(const char *pszText )
{
    if ( !pszText || !( *pszText ) )
    {
        return 0;
    }
    int displayed = strlen( pszText );

    CheckForHangup();
    if ( !hangup )
    {
        int i = 0;

        while ( pszText[i] )
        {
            bputch( pszText[i++], true );
        }
        FlushOutComChBuffer();
    }

    return displayed;
}


int  WOutStream::WriteFormatted(const char *pszFormatText,...)
{
    va_list ap;
    char szBuffer[ 4096 ];

    va_start( ap, pszFormatText );
    vsnprintf( szBuffer, sizeof( szBuffer ), pszFormatText, ap );
    va_end( ap );
    return Write( szBuffer );
}


