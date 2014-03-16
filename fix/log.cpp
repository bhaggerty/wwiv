/**************************************************************************/
/*                                                                        */
/*                              WWIV Version 5.0x                         */
/*             Copyright (C)1998-2004, WWIV Software Services             */
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
#include "log.h"

WFile hLogFile;

char* szLogTypeArray[] = {
	"+ ",
	"! ",
	"? ",
	0
};

void Print(int nType, bool bLogIt, char* szText, ...) {
	char szBuffer[256];
	va_list ap;

	va_start(ap, szText);
	vsprintf(szBuffer, szText, ap);
	va_end(ap);
	printf("%s%s\n", szLogTypeArray[nType], szBuffer);

	if (bLogIt && hLogFile.IsOpen()) {
		struct tm *time_now;
		time_t secs_now;
		char str[81];
		char szBuf[512];
		time(&secs_now);
		time_now = localtime(&secs_now);
		strftime(str, 80, "%H:%M:%S", time_now);
		sprintf(szBuf, "%s%s  %s\n", szLogTypeArray[nType], str, szBuffer);
		hLogFile.Write(szBuf, strlen(szBuf));
	}
}

bool OpenLogFile(char* szFileName) {
	struct tm *time_now;
	time_t secs_now;
	char str[81];
	char szBuf[512];

	hLogFile.SetName(szFileName);
	if(!hLogFile.Open(WFile::modeReadWrite | WFile::modeCreateFile)) {
		Print(NOK, false, "Cannot open Log File %s", szFileName);
		return false;
	}

	_tzset();
	time(&secs_now);
	time_now = localtime(&secs_now);
	strftime(str, 80, "%a %d %b %Y", time_now);
	sprintf(szBuf, "\n----------  %s, fix %s\n", str, wwiv_version);
	hLogFile.Write(szBuf, strlen(szBuf));

	return true;
}


void CloseLogFile() {
	if (hLogFile.IsOpen()) {
		hLogFile.Close();
	}
}
