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
#include "wcomm.h"

#if defined ( _WIN32 )
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "platform/win32/wios.h"
#include "platform/win32/wiot.h"
#elif defined ( __unix__ ) || defined ( __APPLE__ )
#include "wiou.h"
#endif

char WComm::m_szErrorText[8192];

int WComm::GetComPort() const {
  return m_ComPort;
}


void WComm::SetComPort(int nNewPort) {
  m_ComPort = nNewPort;
}

const char* WComm::GetLastErrorText() {
#if defined ( _WIN32 )
  LPVOID lpMsgBuf;
  FormatMessage(
    FORMAT_MESSAGE_ALLOCATE_BUFFER |
    FORMAT_MESSAGE_FROM_SYSTEM |
    FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL,
    GetLastError(),
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
    (LPTSTR) &lpMsgBuf,
    0,
    NULL
  );
  strcpy(m_szErrorText, (LPCTSTR)lpMsgBuf);
  LocalFree(lpMsgBuf);
  return static_cast<const char *>(m_szErrorText);
#else
  return NULL;
#endif
}


WComm* WComm::CreateComm(bool bUseSockets, unsigned int nHandle) {
#if defined ( _WIN32 )
  if (bUseSockets) {
    return new WIOTelnet(nHandle);
  } else {
    return new WIOSerial(nHandle);
  }
#elif defined ( __unix__ ) || defined ( __APPLE__ )
  return new WIOUnix();
  /*
#elif defined ( __APPLE__ )
  if ( bUseSockets )
  {
    //return new WIOTelnet( nHandle );
  }
  else
  {
    //return new WIOSerial( nHandle );
  }
  */
#endif
}
