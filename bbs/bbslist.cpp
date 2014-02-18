/**************************************************************************/
/*                                                                        */
/*                              WWIV Version 5.0x                         */
/*             Copyright (C)1998-2014, WWIV Software Services             */
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
// $Header$

#include "wwiv.h"

//
// Local function prototypes
//

char ShowBBSListMenuAndGetChoice();
bool IsBBSPhoneNumberUnique( const std::string& phoneNumber );
bool IsBBSPhoneNumberValid( const std::string& phoneNumber );
void AddBBSListLine( const std::string bbsListLine );
void AddBBSListEntryImpl();
void AddBBSListEntry();
void DeleteBBSListEntry();



char ShowBBSListMenuAndGetChoice() {
	GetSession()->bout.NewLine();
	if ( so() ) {
		GetSession()->bout << "|#9(|#2Q|#9=|#1Quit|#9) [|#2BBS list|#9]: (|#1R|#9)ead, (|#1A|#9)dd, (|#1D|#9)elete, (|#1N|#9)et : ";
		return onek("QRNAD");
	} else {
		GetSession()->bout << "|#9(|#2Q|#9=|#1Quit|#9) [|#2BBS list|#9] (|#1R|#9)ead, (|#1A|#9)dd, (|#1N|#9)et : ";
		return onek("QRNA");
	}
}


bool IsBBSPhoneNumberUnique( const std::string& phoneNumber ) {
	bool ok = true;
	WFile file( syscfg.gfilesdir, BBSLIST_MSG );
	if ( file.Open( WFile::modeReadOnly | WFile::modeBinary ) ) {
		file.Seek( 0L, WFile::seekBegin );
		long lBbsListLength = file.GetLength();
		char *ss = static_cast<char *>( BbsAllocA( lBbsListLength + 500L ) );
		if ( ss == NULL ) {
			file.Close();
			return true;
		}
		file.Read( ss, lBbsListLength );
		long lBbsListPos = 0L;
		while ( lBbsListPos < lBbsListLength && ok ) {
			char szBbsListLine[ 255 ];
			int i = 0;
			char ch = '\0';
			do {
				ch = ss[ lBbsListPos++ ];
				szBbsListLine[i] = ch;
				if ( ch == '\r' ) {
					szBbsListLine[i] = '\0';
				}
				++i;
			} while ( ch != '\n' && i < 120 && lBbsListPos < lBbsListLength );
			if ( strstr( szBbsListLine, phoneNumber.c_str() ) != NULL ) {
				ok = false;
			}
			if ( strncmp( szBbsListLine, phoneNumber.c_str(), 12 ) == 0 ) {
				ok = false;
			}
		}
		BbsFreeMemory( ss );
		ss = NULL;
		file.Close();
	}
	return ok;
}


bool IsBBSPhoneNumberValid( const std::string& phoneNumber ) {
	if ( phoneNumber.empty() ) {
		return false;
	}
	if ( phoneNumber[3] != '-' || phoneNumber[7] != '-' ) {
		return false;
	}
	if ( phoneNumber.length() != 12 ) {
		return false;
	}
	for ( std::string::const_iterator iter = phoneNumber.begin(); iter != phoneNumber.end(); iter++ ) {
		if ( strchr("0123456789-", (*iter)) == 0 ) {
			return false;
		}
	}
	return true;
}


void AddBBSListLine( const std::string bbsListLine ) {
	WFile file( syscfg.gfilesdir, BBSLIST_MSG );
	bool bOpen = file.Open( WFile::modeReadWrite | WFile::modeCreateFile | WFile::modeBinary, WFile::shareUnknown, WFile::permReadWrite );
	if ( bOpen && file.GetLength() > 0 ) {
		file.Seek( -1L, WFile::seekEnd );
		char chLastChar = 0;
		file.Read( &chLastChar, 1 );
		if ( chLastChar == CZ ) {
			// If last char is a EOF, skip it.
			file.Seek( -1L, WFile::seekEnd );
		}
	}
	file.Write( bbsListLine.c_str(), bbsListLine.length() );
	file.Close();
}


void AddBBSListEntryImpl() {
	GetSession()->bout << "\r\nPlease enter phone number:\r\n ###-###-####\r\n:";
	std::string bbsPhoneNumber;
	input( bbsPhoneNumber, 12, true );
	if ( IsBBSPhoneNumberValid( bbsPhoneNumber.c_str() ) ) {
		if ( IsBBSPhoneNumberUnique( bbsPhoneNumber.c_str() ) ) {
			std::string bbsName, bbsSpeed, bbsType;
			GetSession()->bout << "|#3This number can be added! It is not yet in BBS list.\r\n\n\n"
			                   << "|#7Enter the BBS name and comments about it (incl. V.32/HST) :\r\n:";
			inputl( bbsName, 50, true );
			GetSession()->bout << "\r\n|#7Enter maximum speed of the BBS:\r\n"
			                   << "|#7(|#1example: 14.4,28.8, 33.6, 56k|#7)\r\n:";
			input( bbsSpeed, 4, true );
			GetSession()->bout << "\r\n|#7Enter BBS type (ie, |#1WWIV|#7):\r\n:";
			input( bbsType, 4, true );

			char szBbsListLine[ 255 ];
			snprintf( szBbsListLine, sizeof( szBbsListLine ), "%12s  %-50s  [%4s] (%4s)\r\n",
			          bbsPhoneNumber.c_str(), bbsName.c_str(), bbsSpeed.c_str(), bbsType.c_str() );
			GetSession()->bout.NewLine( 2 );
			GetSession()->bout << szBbsListLine;
			GetSession()->bout.NewLine( 2 );
			GetSession()->bout << "|#5Is this information correct? ";
			if ( yesno() ) {
				AddBBSListLine( szBbsListLine );
				GetSession()->bout << "\r\n|#3This entry was added to BBS list.\r\n";
			}
			GetSession()->bout.NewLine();
		} else {
			GetSession()->bout << "|#6Sorry, It's already in the BBS list.\r\n\n\n";
		}
	} else {
		GetSession()->bout << "\r\n|#6 Error: Please enter number in correct format.\r\n\n";
	}
}


void AddBBSListEntry() {
	if ( GetSession()->GetEffectiveSl() <= 10 ) {
		GetSession()->bout << "\r\n\nYou must be a validated user to add to the BBS list.\r\n\n";
	} else if ( GetSession()->GetCurrentUser()->IsRestrictionAutomessage() ) {
		GetSession()->bout << "\r\n\nYou can not add to the BBS list.\r\n\n\n";
	} else {
		AddBBSListEntryImpl();
	}

}


void DeleteBBSListEntry() {
	GetSession()->bout << "\r\n|#7Please enter phone number in the following format:\r\n";
	GetSession()->bout << "|#1 ###-###-####\r\n:";
	std::string bbsPhoneNumber;
	input( bbsPhoneNumber, 12, true );
	if ( bbsPhoneNumber[3] != '-' || bbsPhoneNumber[7] != '-' ) {
		bbsPhoneNumber.clear();
	}
	if ( bbsPhoneNumber.length() == 12 ) {
		bool ok = false;
		WTextFile fi( syscfg.gfilesdir, BBSLIST_MSG, "r" );
		WTextFile fo(syscfg.gfilesdir, BBSLIST_TMP, "w");
		if (fi.IsOpen()) {
			if (fo.IsOpen()) {
				std::string line;
				while (fi.ReadLine(line)) {
					if ( strstr( line.c_str(), bbsPhoneNumber.c_str() ) ) {
						ok = true;
					} else {
						fo.Write( line );
					}
				}
				fo.Close();
			}
			fi.Close();
		}
		GetSession()->bout.NewLine();
		if (ok) {
			WFile::Remove(fi.GetFullPathName());
			WFile::Rename(fo.GetFullPathName(), fi.GetFullPathName());
			GetSession()->bout << "|#7* |#1Number removed.\r\n";
		} else {
			WFile::Remove(fo.GetFullPathName());
			GetSession()->bout << "|#6Error: Couldn't find that in the bbslist file.\r\n";
		}
	} else {
		GetSession()->bout << "\r\n|#6Error: Please enter number in correct format.\r\n\n";
	}
}


void BBSList() {
	bool done = false;
	do {
		char chInput = ShowBBSListMenuAndGetChoice();
		switch ( chInput ) {
		case 'Q':
			done = true;
			break;
		case 'R':
			printfile( BBSLIST_MSG, true, true );
			break;
		case 'N':
			print_net_listing( false );
			break;
		case 'A':
			AddBBSListEntry();
			break;
		case 'D':
			DeleteBBSListEntry();
			break;
		}
	} while ( !done && !hangup );
}


