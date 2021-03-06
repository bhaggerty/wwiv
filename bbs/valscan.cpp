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

#include "wwiv.h"
#include "subxtr.h"

void valscan() {
  // Must be local cosysop or better
  if (!lcs()) {
    return;
  }

  int ac = 0;
  int os = GetSession()->GetCurrentMessageArea();

  if (uconfsub[1].confnum != -1 && okconf(GetSession()->GetCurrentUser())) {
    ac = 1;
    tmp_disable_conf(true);
  }
  bool done = false;
  for (int sn = 0; sn < GetSession()->num_subs && !hangup && !done; sn++) {
    if (!iscan(sn)) {
      continue;
    }
    if (GetSession()->GetCurrentReadMessageArea() < 0) {
      return;
    }

    uint32_t sq = qsc_p[sn];

    // Must be sub with validation "on"
    if (!(xsubs[GetSession()->GetCurrentReadMessageArea()].num_nets)
        || !(subboards[GetSession()->GetCurrentReadMessageArea()].anony & anony_val_net)) {
      continue;
    }

    GetSession()->bout.NewLine();
    GetSession()->bout.Color(2);
    GetSession()->bout.ClearEOL();
    GetSession()->bout << "{{ ValScanning " << subboards[GetSession()->GetCurrentReadMessageArea()].name << " }}\r\n";
    lines_listed = 0;
    GetSession()->bout.ClearEOL();
    if (okansi() && !newline) {
      GetSession()->bout << "\r\x1b[2A";
    }

    for (int i = 1; i <= GetSession()->GetNumMessagesInCurrentMessageArea() && !hangup && !done; i++) {    // was i = 0
      if (get_post(i)->status & status_pending_net) {
        CheckForHangup();
        GetSession()->localIO()->tleft(true);
        if (i > 0 && i <= GetSession()->GetNumMessagesInCurrentMessageArea()) {
          bool next;
          int val;
          read_message(i, &next, &val);
          GetSession()->bout << "|#4[|#4Subboard: " << subboards[GetSession()->GetCurrentReadMessageArea()].name << "|#1]\r\n";
          GetSession()->bout <<  "|#1D|#9)elete, |#1R|#9)eread |#1V|#9)alidate, |#1M|#9)ark Validated, |#1Q|#9)uit: |#2";
          char ch = onek("QDVMR");
          switch (ch) {
          case 'Q':
            done = true;
            break;
          case 'R':
            i--;
            continue;
          case 'V': {
            open_sub(true);
            resynch(&i, nullptr);
            postrec *p1 = get_post(i);
            p1->status &= ~status_pending_net;
            write_post(i, p1);
            close_sub();
            send_net_post(p1, subboards[GetSession()->GetCurrentReadMessageArea()].filename,
                          GetSession()->GetCurrentReadMessageArea());
            GetSession()->bout.NewLine();
            GetSession()->bout << "|#7Message sent.\r\n\n";
          }
          break;
          case 'M':
            if (lcs() && i > 0 && i <= GetSession()->GetNumMessagesInCurrentMessageArea() &&
                subboards[GetSession()->GetCurrentReadMessageArea()].anony & anony_val_net &&
                xsubs[GetSession()->GetCurrentReadMessageArea()].num_nets) {
              open_sub(true);
              resynch(&i, nullptr);
              postrec *p1 = get_post(i);
              p1->status &= ~status_pending_net;
              write_post(i, p1);
              close_sub();
              GetSession()->bout.NewLine();
              GetSession()->bout << "|#9Not set for net pending now.\r\n\n";
            }
            break;
          case 'D':
            if (lcs()) {
              if (i > 0) {
                open_sub(true);
                resynch(&i, nullptr);
                postrec p2 = *get_post(i);
                delete_message(i);
                close_sub();
                if (p2.ownersys == 0) {
                  WUser tu;
                  GetApplication()->GetUserManager()->ReadUser(&tu, p2.owneruser);
                  if (!tu.IsUserDeleted()) {
                    if (static_cast<unsigned long>(date_to_daten(tu.GetFirstOn())) < p2.daten) {
                      GetSession()->bout.NewLine();
                      GetSession()->bout << "|#2Remove how many posts credit? ";
                      char szNumCredits[ 11 ];
                      input(szNumCredits, 3, true);
                      int nNumPostCredits = 1;
                      if (szNumCredits[0]) {
                        nNumPostCredits = atoi(szNumCredits);
                      }
                      nNumPostCredits = std::min<int>(tu.GetNumMessagesPosted(), nNumPostCredits);
                      if (nNumPostCredits) {
                        tu.SetNumMessagesPosted(tu.GetNumMessagesPosted() - static_cast<unsigned short>(nNumPostCredits));
                      }
                      GetSession()->bout.NewLine();
                      GetSession()->bout << "|#3Post credit removed = " << nNumPostCredits << wwiv::endl;
                      tu.SetNumDeletedPosts(tu.GetNumDeletedPosts() + 1);
                      GetApplication()->GetUserManager()->WriteUser(&tu, p2.owneruser);
                      GetApplication()->UpdateTopScreen();
                    }
                  }
                }
                resynch(&i, &p2);
              }
            }
            break;
          }
        }
      }
    }
    qsc_p[sn] = sq;
  }

  if (ac) {
    tmp_disable_conf(false);
  }

  GetSession()->SetCurrentMessageArea(os);
  GetSession()->bout.NewLine(2);
}
