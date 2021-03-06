/**************************************************************************/
/*                                                                        */
/*                              WWIV Version 5.0x                         */
/*             Copyright (C)1998-2007, WWIV Software Services             */
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
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <direct.h>

#include "bbs.h"
#include "core/wwivport.h"
#include "core/strings.h"

double WWIV_WIN32_FreeSpaceForDriveLetter(int nDrive);

typedef BOOL (WINAPI *P_GDFSE)(LPCTSTR, PULARGE_INTEGER,
                               PULARGE_INTEGER, PULARGE_INTEGER);
typedef UINT(WINAPI *P_GDT)(LPCTSTR);

/**
 * Returns the free disk space for a drive letter, where nDrive is the drive number
 * 1 = 'A', 2 = 'B', etc.
 *
 * @param nDrive The drive number to get the free disk space for.
 */
double WWIV_WIN32_FreeSpaceForDriveLetter(int nDrive) {
  unsigned __int64 i64FreeBytesToCaller, i64TotalBytes, i64FreeBytes;
  DWORD dwSectPerClust, dwBytesPerSect, dwFreeClusters, dwTotalClusters;
  P_GDFSE pGetDiskFreeSpaceEx = NULL;
  BOOL fResult = FALSE;
  P_GDT pGetDriveType = NULL;
  UINT driveType = DRIVE_FIXED;

  pGetDiskFreeSpaceEx = (P_GDFSE) GetProcAddress(GetModuleHandle("kernel32.dll"), "GetDiskFreeSpaceExA");
  char s[] = "X:\\";
  s[0] = static_cast< char >('A' + static_cast< char >(nDrive - 1));
  char *pszDrive = (nDrive) ? s : NULL;


  // if we can get the process handle try and determine if it is a valid drive
  pGetDriveType = (P_GDT)GetProcAddress(GetModuleHandle("kernel32.dll"), "GetDriveTypeA");
  if (pGetDriveType != NULL) {
    driveType = pGetDriveType(pszDrive);
  }

  // check if the drive exists
  if (driveType != DRIVE_UNKNOWN && driveType != DRIVE_NO_ROOT_DIR) {
    if (pGetDiskFreeSpaceEx) {
      // win95 osr2+ allows the GetDiskFreeSpaceEx call
      fResult = pGetDiskFreeSpaceEx(pszDrive,
                                    reinterpret_cast<PULARGE_INTEGER>(&i64FreeBytesToCaller),
                                    reinterpret_cast<PULARGE_INTEGER>(&i64TotalBytes),
                                    reinterpret_cast<PULARGE_INTEGER>(&i64FreeBytes));
      if (fResult) {
        return static_cast<double>((signed _int64) i64FreeBytesToCaller / 1024);
      }

    } else {
      // this one will artificially cap free space at 2 gigs
      fResult = GetDiskFreeSpace(pszDrive, &dwSectPerClust,
                                 &dwBytesPerSect,
                                 &dwFreeClusters,
                                 &dwTotalClusters);
      if (fResult) {
        return (static_cast<double>(dwTotalClusters * dwSectPerClust * dwBytesPerSect)) / 1024.0;
      }
    }
  }

  // Nothing worked, just give up.
  return -1.0;
}


double WWIV_GetFreeSpaceForPath(const char * szPath) {
#ifndef NOT_BBS
  int nDrive = GetApplication()->GetHomeDir()[0];

  if (szPath[1] == ':') {
    nDrive = szPath[0];
  }

  nDrive = wwiv::UpperCase<int> (nDrive - 'A' + 1);

  return WWIV_WIN32_FreeSpaceForDriveLetter(nDrive);
#else
  return 0;
#endif
}


void WWIV_ChangeDirTo(const char *s) {
  char szBuffer[MAX_PATH];

  strcpy(szBuffer, s);
  int i = strlen(szBuffer) - 1;
  int db = (szBuffer[i] == '\\');
  if (i == 0) {
    db = 0;
  }
  if ((i == 2) && (szBuffer[1] == ':')) {
    db = 0;
  }
  if (db) {
    szBuffer[i] = '\0';
  }
  _chdir(szBuffer);
  if (s[1] == ':') {
    int driveNum = s[0];
    // handle both upper and lower case in a single line here.  If the drive letter is over 'Z' then it is lower case.
    driveNum -= driveNum > 'Z' ? 'a' : 'A';
    driveNum++; // FIX, On Win32, _chdrive is 'A' = 1, etc..
    _chdrive(driveNum);
    if (s[2] == 0) {
      _chdir("\\");
    }
  }
}


void WWIV_GetDir(char *s, bool be) {
  strcpy(s, "X:\\");
  s[0] = static_cast< char >('A' + static_cast< char >(_getdrive() - 1));
  _getdcwd(0, &s[0], MAX_PATH);
  if (be) {
    if (s[strlen(s) - 1] != '\\') {
      strcat(s, "\\");
    }
  }
}


void WWIV_GetFileNameFromPath(const char *pszPath, char *pszFileName) {
  _splitpath(pszPath, NULL, NULL, pszFileName, NULL);
}


