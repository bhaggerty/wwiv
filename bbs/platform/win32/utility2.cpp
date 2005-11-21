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
#include "WStringUtils.h"


void show_files( const char *pszFileName, const char *pszDirectoryName )
// Displays list of files matching filespec pszFileName in directory pszDirectoryName.
{
    char s[MAX_PATH];
    char drive[MAX_DRIVE], direc[MAX_DIR], file[MAX_FNAME], ext[MAX_EXT];

    char c = ( okansi() ) ? '\xCD' : '=';
    nl();

    _splitpath(pszDirectoryName, drive, direc, file, ext);
    _snprintf(s, sizeof( s ), "|#7[|B1|15 FileSpec: %s    Dir: %s%s |B0|#7]", WWIV_STRUPR(stripfn(pszFileName)), drive, direc);
    int i = ( GetSession()->thisuser.GetScreenChars() - 1 ) / 2 - strlen(stripcolors(s)) / 2;
    GetSession()->bout << "|#7" << charstr( i, c ) << s;
    i = GetSession()->thisuser.GetScreenChars() - 1 - i - strlen(stripcolors(s));
    GetSession()->bout << "|#7" << charstr( i, c );

    char szFullPathName[ MAX_PATH ];
    _snprintf( szFullPathName, sizeof( szFullPathName ), "%s%s", pszDirectoryName, WWIV_STRUPR( stripfn(pszFileName ) ) );
    WFindFile fnd;
    bool bFound = fnd.open( szFullPathName, 0 );
    while (bFound)
    {
        strncpy(s, fnd.GetFileName(), MAX_PATH);
        align(s);
        _snprintf( szFullPathName, sizeof( szFullPathName ), "|#7[|#2%s|#7]|#1 ", s );
        if ( GetApplication()->GetLocalIO()->WhereX() > ( GetSession()->thisuser.GetScreenChars() - 15 ) )
        {
            nl();
        }
        GetSession()->bout << szFullPathName;
        bFound = fnd.next();
    }

    nl();
    ansic( 7 );
    GetSession()->bout << charstr( GetSession()->thisuser.GetScreenChars() - 1, c );
    nl( 2 );
}



// Used only by WWIV_make_abs_cmd

char *exts[] =
{
	"",
	".COM",
	".EXE",
	".BAT",
	".BTM",
	".CMD",
	0
};



char *WWIV_make_abs_cmd( char *pszOutBuffer )
{
    // pszOutBuffer must be at least MAX_PATH in size.
    char s[MAX_PATH], s1[MAX_PATH], s2[MAX_PATH], *ss1;
    char szTempBuf[MAX_PATH];

    char szWWIVHome[MAX_PATH];
    strncpy( szWWIVHome, GetApplication()->GetHomeDir(), MAX_PATH );

    strncpy( s1, pszOutBuffer, MAX_PATH );

    if ( s1[1] == ':' )
    {
        if ( s1[2] != '\\' )
        {
            _getdcwd( wwiv::UpperCase<char>(s1[0]) - 'A' + 1, s, MAX_PATH );
            if (s[0])
            {
                _snprintf( s1, sizeof( s1 ), "%c:\\%s\\%s", s1[0], s, pszOutBuffer + 2 );
            }
            else
            {
                _snprintf( s1, sizeof( s1 ), "%c:\\%s", s1[0], pszOutBuffer + 2 );
            }
        }
    }
    else if ( s1[0] == '\\' )
    {
        _snprintf( s1, sizeof( s1 ), "%c:%s", szWWIVHome[0], pszOutBuffer );
    }
    else
    {
        strncpy(s2, s1, sizeof(s2));
        strtok(s2, " \t");
        if (strchr(s2, '\\'))
        {
            _snprintf( s1, sizeof( s1 ), "%s%s", GetApplication()->GetHomeDir(), pszOutBuffer );
        }
    }

    char* ss = strchr( s1, ' ' );
    if ( ss )
    {
        *ss = '\0';
        _snprintf( s2, sizeof( s2 ), " %s", ss + 1 );
    }
    else
    {
        s2[0] = '\0';
    }
    for (int i = 0; exts[i]; i++)
    {
        if (i == 0)
        {
            ss1 = strrchr(s1, '\\');
            if (!ss1)
            {
                ss1 = s1;
            }
            if (strchr(ss1, '.') == 0)
            {
                continue;
            }
        }
        _snprintf( s, sizeof( s ), "%s%s", s1, exts[i] );
        if ( s1[1] == ':' )
        {
            if ( WFile::Exists( s ) )
            {
                _snprintf( pszOutBuffer, MAX_PATH, "%s%s", s, s2 );
                goto got_cmd;
            }
        }
        else
        {
            if (WFile::Exists(s))
            {
                _snprintf( pszOutBuffer, MAX_PATH, "%s%s%s", GetApplication()->GetHomeDir(), s, s2 );
                goto got_cmd;
            }
            else
            {
                _searchenv( s, "PATH", szTempBuf );
                ss1 = szTempBuf;
                if ( ss1 && strlen( ss1 ) > 0 )
                {
                    _snprintf( pszOutBuffer, MAX_PATH, "%s%s", ss1, s2 );
                    goto got_cmd;
                }
            }
        }
    }

    _snprintf( pszOutBuffer, MAX_PATH, "%s%s%s", GetApplication()->GetHomeDir(), s1, s2 );

got_cmd:
    return pszOutBuffer;
}


#define LAST(s)	s[strlen(s)-1]


int WWIV_make_path(char *s)
{
    char drive, current_path[MAX_PATH], current_drive, *p, *flp;

    p = flp = WWIV_STRDUP(s);
    _getdcwd(0, current_path, MAX_PATH);
    current_drive = static_cast< char >( *current_path - '@' );
    if (LAST(p) == WWIV_FILE_SEPERATOR_CHAR)
    {
        LAST(p) = 0;
    }
    if (p[1] == ':')
    {
		drive = static_cast< char >( wwiv::UpperCase<char>(p[0]) - 'A' + 1 );
        if (_chdrive(drive))
        {
            chdir(current_path);
            _chdrive(current_drive);
            return -2;
        }
        p += 2;
    }
    if (*p == WWIV_FILE_SEPERATOR_CHAR)
    {
        chdir(WWIV_FILE_SEPERATOR_STRING);
        p++;
    }
    for (; (p = strtok(p, WWIV_FILE_SEPERATOR_STRING)) != 0; p = 0)
    {
        if (chdir(p))
        {
            if (mkdir(p))
            {
                chdir(current_path);
                _chdrive(current_drive);
                return -1;
            }
            chdir(p);
        }
    }
    chdir(current_path);
    if (flp)
    {
        BbsFreeMemory(flp);
    }
    return 0;
}

#if defined (LAST)
#undef LAST
#endif


void WWIV_Delay(unsigned long usec)
{
    Sleep( usec );
}

void WWIV_OutputDebugString( const char *pszString )
{
    ::OutputDebugString( pszString );
}

