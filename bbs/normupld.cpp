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


//////////////////////////////////////////////////////////////////////////////
//
// Implementation
//
//
//


void normalupload(int dn)
{
    uploadsrec u, u1;
    memset( &u, 0, sizeof( uploadsrec ) );
    memset( &u1, 0, sizeof( uploadsrec ) );

    int ok = 1;

    dliscan1( dn );
    directoryrec d = directories[dn];
    if (GetSession()->numf >= d.maxfiles)
    {
        nl( 3 );
        GetSession()->bout << "This directory is currently full.\r\n\n";
        return;
    }
    if ((d.mask & mask_no_uploads) && (!dcs()))
    {
        nl( 2 );
        GetSession()->bout << "Uploads are not allowed to this directory.\r\n\n";
        return;
    }
    GetSession()->bout << "|#9Filename: ";
    char szInputFileName[ MAX_PATH ];
    input( szInputFileName, 12 );
    if (!okfn( szInputFileName ))
    {
        szInputFileName[0] = '\0';
    }
    else
    {
        if ( !is_uploadable( szInputFileName ) )
        {
            if (so())
            {
                nl();
                GetSession()->bout << "|#5In filename database - add anyway? ";
                if (!yesno())
                {
                    szInputFileName[0] = '\0';
                }
            }
            else
            {
                szInputFileName[0] = '\0';
                nl();
                GetSession()->bout << "|12File either already here or unwanted.\r\n";
            }
        }
    }
    align( szInputFileName );
    if ( strchr( szInputFileName, '?' ) )
    {
        return;
    }
    if (d.mask & mask_archive)
    {
        ok = 0;
        std::string supportedExtensions;
        for (int k = 0; k < MAX_ARCS; k++)
        {
            if (arcs[k].extension[0] && arcs[k].extension[0] != ' ')
            {
                if ( !supportedExtensions.empty() )
                {
                    supportedExtensions += ", ";
                }
                supportedExtensions += arcs[k].extension;
                if ( wwiv::stringUtils::IsEquals( szInputFileName + 9, arcs[k].extension ) )
                {
                    ok = 1;
                }
            }
        }
        if ( !ok )
        {
            nl();
            GetSession()->bout << "Sorry, all uploads to this dir must be archived.  Supported types are:\r\n" << supportedExtensions << "\r\n\n";
            return;
        }
    }
    strcpy( u.filename, szInputFileName );
    u.ownerusr = static_cast<unsigned short>( GetSession()->usernum );
    u.ownersys = 0;
    u.numdloads = 0;
    u.filetype = 0;
    u.mask = 0;
    strcpy( u.upby, GetSession()->thisuser.GetUserNameAndNumber( GetSession()->usernum ) );
    strcpy( u.date, date() );
    nl();
    ok = 1;
    bool xfer = true;
    if (check_batch_queue(u.filename))
    {
        ok = 0;
        nl();
        GetSession()->bout << "That file is already in the batch queue.\r\n\n";
    }
    else
    {
        if ( !wwiv::stringUtils::IsEquals( szInputFileName, "        .   " ) )
        {
            GetSession()->bout << "|#5Upload '" << szInputFileName << "' to " << d.name << "? ";
        }
        else
        {
            ok = 0;
        }
    }
    if ( ok && yesno() )
    {
        WFile file( d.path, szInputFileName );
        if ( file.Exists() )
        {
            if ( dcs() )
            {
                xfer = false;
                nl( 2 );
                GetSession()->bout << "File already exists.\r\n|#5Add to database anyway? ";
                if ( !yesno() )
                {
                    ok = 0;
                }
            }
            else
            {
                nl( 2 );
                GetSession()->bout << "That file is already here.\r\n\n";
                ok = 0;
            }
        }
        else if (!incom)
        {
            nl();
            GetSession()->bout << "File isn't already there.\r\nCan't upload locally.\r\n\n";
            ok = 0;
        }
        if ((d.mask & mask_PD) && ok )
        {
            nl();
            GetSession()->bout << "|#5Is this program PD/Shareware? ";
            if (!yesno())
            {
                nl();
                GetSession()->bout << "This directory is for Public Domain/\r\nShareware programs ONLY.  Please do not\r\n";
                GetSession()->bout << "upload other programs.  If you have\r\ntrouble with this policy, please contact\r\n";
                GetSession()->bout << "the sysop.\r\n\n";
                char szBuffer[ 255 ];
                sprintf( szBuffer , "Wanted to upload \"%s\"", u.filename );
                add_ass( 5, szBuffer );
                ok = 0;
            }
            else
            {
                u.mask = mask_PD;
            }
        }
        if ( ok && !GetApplication()->HasConfigFlag( OP_FLAGS_FAST_SEARCH ) )
        {
            nl();
            GetSession()->bout << "Checking for same file in other directories...\r\n\n";
            int nLastLineLength = 0;
            for ( int i = 0; i < GetSession()->num_dirs && udir[i].subnum != -1; i++ )
            {
                std::string buffer = "Scanning ";
                buffer += directories[udir[i].subnum].name;
                int nBufferLen = buffer.length();
                for ( int i3 = nBufferLen; i3 < nLastLineLength; i3++ )
                {
                    buffer += " ";
                }
                nLastLineLength = nBufferLen;
                GetSession()->bout << buffer << "\r";
                dliscan1( udir[i].subnum );
                int i1 = recno( u.filename );
                if ( i1 >= 0 )
                {
                    nl();
					GetSession()->bout << "Same file found on " << directories[udir[i].subnum].name << wwiv::endl;
                    if (dcs())
                    {
                        nl();
                        GetSession()->bout << "|#5Upload anyway? ";
                        if (!yesno())
                        {
                            ok = 0;
                            break;
                        }
                        nl();
                    }
                    else
                    {
                        ok = 0;
                        break;
                    }
                }
            }
            std::string filler = charstr( nLastLineLength, SPACE );
            GetSession()->bout << filler << "\r";
            if (ok)
            {
                dliscan1(dn);
            }
            nl();
        }
        if (ok)
        {
            nl();
			GetSession()->bout << "Please enter a one line description.\r\n:";
            inputl(u.description, 58);
            nl();
            char *pszExtendedDescription = NULL;
            modify_extended_description( &pszExtendedDescription, directories[dn].name, u.filename );
            if ( pszExtendedDescription )
            {
                add_extended_description( u.filename, pszExtendedDescription );
                u.mask |= mask_extended;
                BbsFreeMemory( pszExtendedDescription );
            }
            nl();
            char szReceiveFileName[ MAX_PATH ];
            memset( szReceiveFileName, 0, sizeof( szReceiveFileName ) );
            if ( xfer )
            {
                write_inst(INST_LOC_UPLOAD, udir[GetSession()->GetCurrentFileArea()].subnum, INST_FLAGS_ONLINE);
                double ti = timer();
                receive_file( szReceiveFileName, &ok, reinterpret_cast<char*>( &u.filetype ) , u.filename, dn );
                ti = timer() - ti;
                if (ti < 0)
                {
                    ti += SECONDS_PER_HOUR_FLOAT * HOURS_PER_DAY_FLOAT;
                }
                GetSession()->thisuser.SetExtraTime( GetSession()->thisuser.GetExtraTime() + static_cast<float>( ti ) );
            }
            if (ok)
            {
				WFile file( szReceiveFileName );
                if (ok == 1)
                {
					if ( !file.Open( WFile::modeBinary | WFile::modeReadOnly ) )
                    {
                        ok = 0;
                        nl( 2 );
                        GetSession()->bout << "OS error - File not found.\r\n\n";
                        if (u.mask & mask_extended)
                        {
                            delete_extended_description(u.filename);
                        }
                    }
                    if (ok && syscfg.upload_c[0])
                    {
						file.Close();
                        GetSession()->bout << "Please wait...\r\n";
                        if ( !check_ul_event( dn, &u ) )
                        {
                            if ( u.mask & mask_extended )
                            {
                                delete_extended_description( u.filename );
                            }
                            ok = 0;
                        }
                        else
                        {
                            file.Open( WFile::modeBinary | WFile::modeReadOnly );
                        }
                    }
                }
                if (ok)
                {
                    if (ok == 1)
                    {
						long lFileLength = file.GetLength();
                        u.numbytes = lFileLength;
						file.Close();
                        GetSession()->thisuser.SetFilesUploaded( GetSession()->thisuser.GetFilesUploaded() + 1 );
                        modify_database( u.filename, true );
                        GetSession()->thisuser.SetUploadK( GetSession()->thisuser.GetUploadK() + bytes_to_k( u.numbytes ) );

                        get_file_idz(&u, dn);
                    }
                    else
                    {
                        u.numbytes = 0;
                    }
                    time_t lCurrentTime;
                    time( &lCurrentTime );
                    u.daten = static_cast<unsigned long>(lCurrentTime);
					WFile fileDownload( g_szDownloadFileName );
					fileDownload.Open( WFile::modeBinary|WFile::modeCreateFile|WFile::modeReadWrite, WFile::shareUnknown, WFile::permReadWrite );
                    for (int j = GetSession()->numf; j >= 1; j--)
                    {
                        FileAreaSetRecord( fileDownload, j );
						fileDownload.Read( &u1, sizeof( uploadsrec ) );
                        FileAreaSetRecord( fileDownload, j + 1 );
						fileDownload.Write( &u1, sizeof( uploadsrec ) );
                    }
                    FileAreaSetRecord( fileDownload, 1 );
					fileDownload.Write( &u, sizeof( uploadsrec ) );
                    ++GetSession()->numf;
                    FileAreaSetRecord( fileDownload, 0 );
					fileDownload.Read( &u1, sizeof( uploadsrec ) );
                    u1.numbytes = GetSession()->numf;
                    u1.daten = lCurrentTime;
                    GetSession()->m_DirectoryDateCache[dn] = lCurrentTime;
                    FileAreaSetRecord( fileDownload, 0 );
					fileDownload.Write( &u1, sizeof( uploadsrec ) );
					fileDownload.Close();
                    if (ok == 1)
                    {
                        WStatus *pStatus = GetApplication()->GetStatusManager()->BeginTransaction();
                        pStatus->IncrementNumUploadsToday();
                        pStatus->IncrementFileChangedFlag( WStatus::fileChangeUpload );
                        GetApplication()->GetStatusManager()->CommitTransaction( pStatus );
                        sysoplogf( "+ \"%s\" uploaded on %s", u.filename, directories[dn].name);
                        nl( 2 );
                        bprintf( "File uploaded.\r\n\nYour ratio is now: %-6.3f\r\n", ratio() );
                        nl( 2 );
                        if ( GetSession()->IsUserOnline() )
                        {
                            GetApplication()->GetLocalIO()->UpdateTopScreen();
                        }
                    }
                }
            }
            else
            {
                nl( 2 );
                GetSession()->bout << "File transmission aborted.\r\n\n";
                if (u.mask & mask_extended)
                {
                    delete_extended_description(u.filename);
                }
            }
        }
    }
}

