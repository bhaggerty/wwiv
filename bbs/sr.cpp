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
#include <algorithm>
#include <cmath>

#include "wwiv.h"

void calc_CRC(unsigned char b) {
  checksum = checksum + b;

  crc ^= (((unsigned short)(b)) << 8);
  for (int i = 0; i < 8; i++) {
    if (crc & 0x8000) {
      crc = (crc << 1);
      crc ^= 0x1021;
    } else {
      crc = (crc << 1);
    }
  }
}


char gettimeout(double d, bool *abort) {
  if (bkbhitraw()) {
    return bgetchraw();
  }

  double d1 = timer();
  while (fabs(timer() - d1) < d && !bkbhitraw() && !hangup && !*abort) {
    if (GetSession()->localIO()->LocalKeyPressed()) {
      char ch = GetSession()->localIO()->getchd();
      if (ch == 0) {
        GetSession()->localIO()->getchd();
      } else if (ch == ESC) {
        *abort = true;
      }
    }
    CheckForHangup();
  }
  return (bkbhitraw()) ? bgetchraw() : 0;
}


int extern_prot(int nProtocolNum, const char *pszFileNameToSend, bool bSending) {
  char s1[81], s2[81], szFileName[81], sx1[21], sx2[21], sx3[21];

  if (bSending) {
    GetSession()->bout.NewLine();
    GetSession()->bout << "-=> Beginning file transmission, Ctrl+X to abort.\r\n";
    if (nProtocolNum < 0) {
      strcpy(s1, over_intern[(-nProtocolNum) - 1].sendfn);
    } else {
      strcpy(s1, (externs[nProtocolNum].sendfn));
    }
  } else {
    GetSession()->bout.NewLine();
    GetSession()->bout << "-=> Ready to receive, Ctrl+X to abort.\r\n";
    if (nProtocolNum < 0) {
      strcpy(s1, over_intern[(-nProtocolNum) - 1].receivefn);
    } else {
      strcpy(s1, (externs[nProtocolNum].receivefn));
    }
  }
  strcpy(szFileName, pszFileNameToSend);
  stripfn_inplace(szFileName);
  int nEffectiveXferSpeed = std::min<int>(com_speed, 57600);
  sprintf(sx1, "%d", nEffectiveXferSpeed);
  if (com_speed == 1) {
    strcpy(sx1, "115200");
  }
  // Use this since fdsz doesn't like 115200
  nEffectiveXferSpeed = std::min<int>(modem_speed, 57600);
  sprintf(sx3, "%d", nEffectiveXferSpeed);
  sx2[0] = '0' + syscfgovr.primaryport;
  sx2[1] = '\0';
  if (wwiv::strings::IsEqualsIgnoreCase(GetSession()->GetCurrentSpeed().c_str(), "TELNET")) {
    StringReplace(s1, strlen(s1), "MDMDSZ", "FDSZ");
  }
  const std::string command = stuff_in(s1, sx1, sx2, szFileName, sx3, "");
  if (!command.empty()) {
    GetSession()->localIO()->set_protect(0);
    sprintf(s2, "%s is currently online at %u bps",
            GetSession()->GetCurrentUser()->GetUserNameAndNumber(GetSession()->usernum), modem_speed);
    GetSession()->localIO()->LocalPuts(s2);
    GetSession()->localIO()->LocalPuts("\r\n\r\n");
    GetSession()->localIO()->LocalPuts(command.c_str());
    GetSession()->localIO()->LocalPuts("\r\n");
    if (incom) {
      int nRetCode = ExecuteExternalProgram(command, GetApplication()->GetSpawnOptions(SPWANOPT_PROT_SINGLE));
      GetApplication()->UpdateTopScreen();
      return nRetCode;
    } else {
      GetApplication()->UpdateTopScreen();
      return -5;
    }
  }
  return -5;
}


bool ok_prot(int nProtocolNum, xfertype xt) {
  bool ok = false;

  if (xt == xf_none) {
    return false;
  }

  if (nProtocolNum > 0 && nProtocolNum < (GetSession()->GetNumberOfExternalProtocols() + WWIV_NUM_INTERNAL_PROTOCOLS)) {
    switch (nProtocolNum) {
    case WWIV_INTERNAL_PROT_ASCII:
      if (xt == xf_down || xt == xf_down_temp) {
        ok = true;
      }
      break;
    case WWIV_INTERNAL_PROT_XMODEM:
    case WWIV_INTERNAL_PROT_XMODEMCRC:
    case WWIV_INTERNAL_PROT_YMODEM:
    case WWIV_INTERNAL_PROT_ZMODEM:
      if (xt != xf_up_batch && xt != xf_down_batch && xt != xf_bi) {
        ok = true;
      }
      if (nProtocolNum == WWIV_INTERNAL_PROT_YMODEM && xt == xf_down_batch) {
        ok = true;
      }
      if (nProtocolNum == WWIV_INTERNAL_PROT_ZMODEM && xt == xf_down_batch) {
        ok = true;
      }
      if (nProtocolNum == WWIV_INTERNAL_PROT_ZMODEM && xt == xf_up_temp) {
        ok = true;
      }
      if (nProtocolNum == WWIV_INTERNAL_PROT_ZMODEM && xt == xf_up_batch) {
        ok = true;
      }
      if (nProtocolNum == WWIV_INTERNAL_PROT_ZMODEM && !GetSession()->IsUseInternalZmodem()) {
        // If AllowInternalZmodem is not true, don't allow it.
        ok = false;
      }
      break;
    case WWIV_INTERNAL_PROT_BATCH:
      if (xt == xf_up) {
        for (int i = 0; i < GetSession()->GetNumberOfExternalProtocols(); i++) {
          if (externs[i].receivebatchfn[0] || externs[i].bibatchfn) {
            ok = true;
          }
        }
      } else if (xt == xf_down) {
        for (int i = 0; i < GetSession()->GetNumberOfExternalProtocols(); i++) {
          if (externs[i].sendbatchfn[0] || externs[i].bibatchfn) {
            ok = true;
          }
        }
      }
      if (xt == xf_up || xt == xf_down) {
        ok = true;
      }
      break;
    default:
      switch (xt) {
      case xf_up:
      case xf_up_temp:
        if (externs[nProtocolNum - WWIV_NUM_INTERNAL_PROTOCOLS].receivefn[0]) {
          ok = true;
        }
        break;
      case xf_down:
      case xf_down_temp:
        if (externs[nProtocolNum - WWIV_NUM_INTERNAL_PROTOCOLS].sendfn[0]) {
          ok = true;
        }
        break;
      case xf_up_batch:
        if (externs[nProtocolNum - WWIV_NUM_INTERNAL_PROTOCOLS].receivebatchfn[0]) {
          ok = true;
        }
        break;
      case xf_down_batch:
        if (externs[nProtocolNum - WWIV_NUM_INTERNAL_PROTOCOLS].sendbatchfn[0]) {
          ok = true;
        }
        break;
      case xf_bi:
        if (externs[nProtocolNum - WWIV_NUM_INTERNAL_PROTOCOLS].bibatchfn[0]) {
          ok = true;
        }
        break;
      case xf_none:
        break;
      }
      if (externs[nProtocolNum - WWIV_NUM_INTERNAL_PROTOCOLS].othr & othr_error_correct) {
        if (!(modem_flag & flag_ec)) {
          ok = false;
        }
      }
      break;
    }
  }
  return ok;
}


char *prot_name(int nProtocolNum) {
  static char szProtocolName[ 21 ];
  strcpy(szProtocolName, "-=>NONE<=-");
  char *ss = szProtocolName;

  switch (nProtocolNum) {
  case WWIV_INTERNAL_PROT_ASCII:
    strcpy(szProtocolName, "ASCII");
    break;
  case WWIV_INTERNAL_PROT_XMODEM:
    strcpy(szProtocolName, "Xmodem");
    break;
  case WWIV_INTERNAL_PROT_XMODEMCRC:
    strcpy(szProtocolName, "Xmodem-CRC");
    break;
  case WWIV_INTERNAL_PROT_YMODEM:
    strcpy(szProtocolName, "Ymodem");
    break;
  case WWIV_INTERNAL_PROT_BATCH:
    strcpy(szProtocolName, "Batch");
    break;
  case WWIV_INTERNAL_PROT_ZMODEM:
    strcpy(szProtocolName, "Zmodem (Internal)");
  default:
    if (nProtocolNum >= WWIV_NUM_INTERNAL_PROTOCOLS &&
        nProtocolNum < (GetSession()->GetNumberOfExternalProtocols() + WWIV_NUM_INTERNAL_PROTOCOLS)) {
      ss = externs[nProtocolNum - WWIV_NUM_INTERNAL_PROTOCOLS].description;
    }
    break;
  }

  return ss;
}


#define BASE_CHAR '!'

int get_protocol(xfertype xt) {
  char s[81], s1[81], oks[81], ch, oks1[81], *ss, ch1, fl[80];
  int prot;

  if (ok_prot(GetSession()->GetCurrentUser()->GetDefaultProtocol(), xt)) {
    prot = GetSession()->GetCurrentUser()->GetDefaultProtocol();
  } else {
    prot = 0;
  }

  unsigned char cyColorSave = GetSession()->GetCurrentUser()->GetColor(8);
  GetSession()->GetCurrentUser()->SetColor(8, 1);
  int oks1p = 0;
  oks1[0] = '\0';
  strcpy(oks, "Q?0");
  int i1 = strlen(oks);
  int only = 0;
  int maxprot = (WWIV_NUM_INTERNAL_PROTOCOLS - 1) + GetSession()->GetNumberOfExternalProtocols();
  for (int i = 1; i <= maxprot; i++) {
    fl[ i ] = '\0';
    if (ok_prot(i, xt)) {
      if (i < 10) {
        oks[i1++] = static_cast<char>('0' + i);
      } else {
        oks[i1++] = static_cast<char>(BASE_CHAR + i - 10);
      }
      if (only == 0) {
        only = i;
      } else {
        only = -1;
      }
      if (i >= 3) {
        ch1 = upcase(*prot_name(i));
        if (strchr(oks1, ch1) == 0) {
          oks1[oks1p++] = ch1;
          oks1[oks1p] = 0;
          fl[i] = 1;
        }
      }
    }
  }
  oks[i1] = 0;
  strcat(oks, oks1);
  if (only > 0) {
    prot = only;
  }

  if (only == 0 && xt != xf_none) {
    GetSession()->bout.NewLine();
    GetSession()->bout << "No protocols available for that.\r\n\n";
    return -1;
  }
  if (prot) {
    ss = prot_name(prot);
    sprintf(s, "Protocol (?=list, <C/R>=%s) : ", ss);
    strcpy(s1, oks);
    strcat(s1, "\r");
  } else {
    strcpy(s, "Protocol (?=list) : ");
    strcpy(s1, oks);
  }
  if (!prot) {
    GetSession()->bout.NewLine();
    GetSession()->bout << "|#3Available Protocols :|#1\r\n\n";
    GetSession()->bout << "|#8[|#7Q|#8] |#1Abort Transfer(s)\r\n";
    GetSession()->bout << "|#8[|#70|#8] |#1Don't Transfer\r\n";
    for (int j = 1; j <= maxprot; j++) {
      if (ok_prot(j, xt)) {
        ch1 = upcase(*prot_name(j));
        if (fl[j] == 0) {
          GetSession()->bout.WriteFormatted("|#8[|#7%c|#8] |#1%s\r\n", (j < 10) ? (j + '0') : (j + BASE_CHAR - 10),
                                            prot_name(j));
        } else {
          GetSession()->bout.WriteFormatted("|#8[|#7%c|#8] |#1%s\r\n", *prot_name(j), prot_name(j));
        }
      }
    }
    GetSession()->bout.NewLine();
  }
  bool done = false;
  do {
    GetSession()->bout.NewLine();
    GetSession()->bout << "|#7" << s;
    ch = onek(s1);
    if (ch == '?') {
      GetSession()->bout.NewLine();
      GetSession()->bout << "|#3Available Protocols :|#1\r\n\n";
      GetSession()->bout << "|#8[|#7Q|#8] |#1Abort Transfer(s)\r\n";
      GetSession()->bout << "|#8[|#70|#8] |#1Don't Transfer\r\n";
      for (int j = 1; j <= maxprot; j++) {
        if (ok_prot(j, xt)) {
          ch1 = upcase(*prot_name(j));
          if (fl[ j ] == 0) {
            GetSession()->bout.WriteFormatted("|#8[|#7%c|#8] |#1%s\r\n", (j < 10) ? (j + '0') : (j + BASE_CHAR - 10),
                                              prot_name(j));
          } else {
            GetSession()->bout.WriteFormatted("|#8[|#7%c|#8] |#1%s\r\n", *prot_name(j), prot_name(j));
          }
        }
      }
      GetSession()->bout.NewLine();
    } else {
      done = true;
    }
  } while (!done && !hangup);
  GetSession()->GetCurrentUser()->SetColor(8, cyColorSave);
  if (ch == RETURN) {
    return prot;
  }
  if (ch >= '0' && ch <= '9') {
    if (ch != '0') {
      GetSession()->GetCurrentUser()->SetDefaultProtocol(ch - '0');
    }
    return ch - '0';
  } else {
    if (ch == 'Q') {
      return -1;
    } else {
      i1 = ch - BASE_CHAR + 10;
      GetSession()->GetCurrentUser()->SetDefaultProtocol(i1);
      if (i1 < GetSession()->GetNumberOfExternalProtocols() + WWIV_NUM_INTERNAL_PROTOCOLS) {
        return ch - BASE_CHAR + 10;
      }
      for (int j = 3; j < GetSession()->GetNumberOfExternalProtocols() + WWIV_NUM_INTERNAL_PROTOCOLS; j++) {
        if (upcase(*prot_name(j)) == ch) {
          return j;
        }
      }
    }
  }
  return -1;
}


void ascii_send(const char *pszFileName, bool *sent, double *percent) {
  char b[2048];

  WFile file(pszFileName);
  if (file.Open(WFile::modeBinary | WFile::modeReadOnly)) {
    long lFileSize = file.GetLength();
    lFileSize = std::max<long>(lFileSize, 1);
    int nNumRead = file.Read(b, 1024);
    long lTotalBytes = 0L;
    bool abort = false;
    while (nNumRead && !hangup && !abort) {
      int nBufferPos = 0;
      while (!hangup && !abort && nBufferPos < nNumRead) {
        CheckForHangup();
        bputch(b[nBufferPos++]);
        checka(&abort);
      }
      lTotalBytes += static_cast<long>(nBufferPos);
      checka(&abort);
      nNumRead = file.Read(b, 1024);
    }
    file.Close();
    if (!abort) {
      *sent = true;
    } else {
      *sent = false;
      GetSession()->GetCurrentUser()->SetDownloadK(GetSession()->GetCurrentUser()->GetDownloadK() + bytes_to_k(lTotalBytes));
    }
    *percent = static_cast<double>(lTotalBytes) / static_cast<double>(lFileSize);
  } else {
    GetSession()->bout.NewLine();
    GetSession()->bout << "File not found.\r\n\n";
    *sent = false;
    *percent = 0.0;
  }
}


void maybe_internal(const char *pszFileName, bool *xferred, double *percent, bool bSend, int prot) {
  if (over_intern && (over_intern[prot - 2].othr & othr_override_internal) &&
      ((bSend && over_intern[prot - 2].sendfn[0]) ||
       (!bSend && over_intern[prot - 2].receivefn[0]))) {
    if (extern_prot(-(prot - 1), pszFileName, bSend) == over_intern[prot - 2].ok1) {
      *xferred = 1;
    }
  } else {
    if (incom) {
      if (bSend) {
        switch (prot) {
        case WWIV_INTERNAL_PROT_XMODEM:
          xymodem_send(pszFileName, xferred, percent, false, false, false);
          break;
        case WWIV_INTERNAL_PROT_XMODEMCRC:
          xymodem_send(pszFileName, xferred, percent, true, false, false);
          break;
        case WWIV_INTERNAL_PROT_YMODEM:
          xymodem_send(pszFileName, xferred, percent, true, true, false);
          break;
        case WWIV_INTERNAL_PROT_ZMODEM:
          zmodem_send(pszFileName, xferred, percent);
          break;
        }
      } else {
        switch (prot) {
        case WWIV_INTERNAL_PROT_XMODEM:
          xymodem_receive(pszFileName, xferred, false);
          break;
        case WWIV_INTERNAL_PROT_XMODEMCRC:
        case WWIV_INTERNAL_PROT_YMODEM:
          xymodem_receive(pszFileName, xferred, true);
          break;
        case WWIV_INTERNAL_PROT_ZMODEM:
          zmodem_receive(pszFileName, xferred);
          break;
        }
      }
    } else {
      GetSession()->bout << "Would use internal " << prot_name(prot) << wwiv::endl;
    }
  }
}


void send_file(const char *pszFileName, bool *sent, bool *abort, const char *sfn, int dn, long fs) {
  double percent, t;

  *sent = false;
  *abort = false;
  int nProtocol = 0;
  if (fs < 0) {
    nProtocol = get_protocol(xf_none);
  } else {
    nProtocol = (dn == -1) ? get_protocol(xf_down_temp) : get_protocol(xf_down);
  }
  bool ok = false;
  percent = 0.0;
  if (check_batch_queue(sfn)) {
    *sent = false;
    if (nProtocol > 0) {
      GetSession()->bout.NewLine();
      GetSession()->bout << "That file is already in the batch queue.\r\n\n";
    } else if (nProtocol == -1) {
      *abort = true;
    }
  } else {
    switch (nProtocol) {
    case -1:
      *abort = true;
      ok = true;
      break;
    case 0:
      ok = true;
      break;
    case WWIV_INTERNAL_PROT_ASCII:
      ascii_send(pszFileName, sent, &percent);
      break;
    case WWIV_INTERNAL_PROT_XMODEM:
    case WWIV_INTERNAL_PROT_XMODEMCRC:
    case WWIV_INTERNAL_PROT_YMODEM:
    case WWIV_INTERNAL_PROT_ZMODEM:
      maybe_internal(pszFileName, sent, &percent, true, nProtocol);
      break;
    case WWIV_INTERNAL_PROT_BATCH:
      ok = true;
      if (GetSession()->numbatch >= GetSession()->max_batch) {
        GetSession()->bout.NewLine();
        GetSession()->bout << "No room left in batch queue.\r\n\n";
        *sent = false;
        *abort = false;
      } else {
        if (modem_speed) {
          t = (12.656) / ((double)(modem_speed)) * ((double)(fs));
        } else {
          t = 0.0;
        }
        if (nsl() <= (batchtime + t)) {
          GetSession()->bout.NewLine();
          GetSession()->bout << "Not enough time left in queue.\r\n\n";
          *sent = false;
          *abort = false;
        } else {
          if (dn == -1) {
            GetSession()->bout.NewLine();
            GetSession()->bout << "Can't add temporary file to batch queue.\r\n\n";
            *sent = false;
            *abort = false;
          } else {
            batchtime += static_cast<float>(t);
            strcpy(batch[GetSession()->numbatch].filename, sfn);
            batch[GetSession()->numbatch].dir = static_cast<short>(dn);
            batch[GetSession()->numbatch].time = static_cast<float>(t);
            batch[GetSession()->numbatch].sending = 1;
            batch[GetSession()->numbatch].len = fs;

            GetSession()->numbatch++;
            ++GetSession()->numbatchdl;
            GetSession()->bout.NewLine(2);
            GetSession()->bout << "File added to batch queue.\r\n";
            GetSession()->bout << "Batch: Files - " << GetSession()->numbatch <<
                               "  Time - " << ctim(batchtime) << "\r\n\n";
            *sent = false;
            *abort = false;
          }
        }
      }
      break;
    default:
      int nTempProt = extern_prot(nProtocol - WWIV_NUM_INTERNAL_PROTOCOLS, pszFileName, true);
      *abort = false;
      if (nTempProt == externs[nProtocol - WWIV_NUM_INTERNAL_PROTOCOLS].ok1) {
        *sent = true;
      }
      break;
    }
  }
  if (!*sent && !ok) {
    if (percent == 1.0) {
      *sent = true;
      add_ass(10, "Aborted on last block");
    } else {
      sysoplogf("Tried D/L \"%s\" %3.2f%%", stripfn(pszFileName), percent * 100.0);
    }
  }
}


void receive_file(const char *pszFileName, int *received, const char *sfn, int dn) {
  bool bReceived;
  int nProtocol = (dn == -1) ? get_protocol(xf_up_temp) : get_protocol(xf_up);

  switch (nProtocol) {
  case -1:
    *received = 0;
    break;
  case 0:
    *received = 0;
    break;
  case WWIV_INTERNAL_PROT_XMODEM:
  case WWIV_INTERNAL_PROT_XMODEMCRC:
  case WWIV_INTERNAL_PROT_YMODEM:
  case WWIV_INTERNAL_PROT_ZMODEM: {
    std::cout << "maybe_internal, filename=" << pszFileName;
    maybe_internal(pszFileName, &bReceived, NULL, false, nProtocol);
    *received = (bReceived) ? 1 : 0;
  }
  break;
  case WWIV_INTERNAL_PROT_BATCH:
    if (dn != -1) {
      if (GetSession()->numbatch >= GetSession()->max_batch) {
        GetSession()->bout.NewLine();
        GetSession()->bout << "No room left in batch queue.\r\n\n";
        *received = 0;
      } else {
        *received = 2;
        strcpy(batch[GetSession()->numbatch].filename, sfn);
        batch[GetSession()->numbatch].dir = static_cast<short>(dn);
        batch[GetSession()->numbatch].time = 0;
        batch[GetSession()->numbatch].sending = 0;
        batch[GetSession()->numbatch].len = 0;

        GetSession()->numbatch++;
        GetSession()->bout.NewLine();
        GetSession()->bout << "File added to batch queue.\r\n\n";
        GetSession()->bout << "Batch upload: files - " << (GetSession()->numbatch - GetSession()->numbatchdl) << "\r\n\n";
      }
    } else {
      GetSession()->bout.NewLine();
      GetSession()->bout << "Can't batch upload that.\r\n\n";
    }
    break;
  default:
    if (nProtocol > (WWIV_NUM_INTERNAL_PROTOCOLS - 1) && incom) {
      extern_prot(nProtocol - WWIV_NUM_INTERNAL_PROTOCOLS, pszFileName, false);
      *received = WFile::Exists(pszFileName);
    }
    break;
  }
}



char end_batch1() {
  char b[128];

  memset(b, 0, 128);

  bool done = false;
  int  nerr = 0;
  bool bAbort = false;
  char ch = 0;
  do {
    send_block(b, 5, 1, 0);
    ch = gettimeout(5.0, &bAbort);
    if (ch == CF || ch == CX) {
      done = true;
    } else {
      ++nerr;
      if (nerr >= 9) {
        done = true;
      }
    }
  } while (!done && !hangup && !bAbort);
  if (ch == CF) {
    return CF;
  }
  if (ch == CX) {
    return CX;
  }
  return CU;
}


void endbatch() {
  bool abort = false;
  int terr = 0;
  int oldx = GetSession()->localIO()->WhereX();
  int oldy = GetSession()->localIO()->WhereY();
  bool ucrc = false;
  if (!okstart(&ucrc, &abort)) {
    abort = true;
  }
  if (!abort && !hangup) {
    char ch = end_batch1();
    if (ch == CX) {
      abort = true;
    }
    if (ch == CU) {
      WFile nullFile;
      send_b(nullFile, 0L, 3, 0, &ucrc, "", &terr, &abort);
      abort = true;
    }
    /*
    if ((!hangup) && (!abort))
    {
      WFile nullFile;
      send_b(nullFile,0L,2,0,&ucrc,"",&terr,&abort);
    }
    */
  }
  GetSession()->localIO()->LocalGotoXY(oldx, oldy);
}
