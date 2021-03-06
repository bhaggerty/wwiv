/**************************************************************************/
/*                                                                        */
/*                 WWIV Initialization Utility Version 5.0                */
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
#define _DEFINE_GLOBALS_

#include <cctype>
#include <cstdlib>
#include <cstring>
#include <curses.h>
#include <fcntl.h>
#include <memory>
#ifdef _WIN32
#include <direct.h>
#include <io.h>
#endif
#include <locale.h>
#include <sys/stat.h>

#include "archivers.h"
#include "editors.h"
#include "ifcns.h"
#include "input.h"
#include "init.h"
#include "instance_settings.h"
#include "languages.h"
#include "levels.h"
#include "networks.h"
#include "newinit.h"
#include "paths.h"
#include "protocols.h"
#include "regcode.h"
#include "subsdirs.h"
#include "system_info.h"
#include "user_editor.h"
#include "bbs/version.cpp"
#include "wwivinit.h"
#include "bbs/wconstants.h"
#include "platform/curses_io.h"
#include "core/wwivport.h"
#include "core/wfndfile.h"
#include "utility.h"

initinfo_rec initinfo;
configrec syscfg;
configoverrec syscfgovr;
statusrec status;
subboardrec *subboards;
chainfilerec *chains;
newexternalrec *externs, *over_intern;
editorrec *editors;
arcrec *arcs;
net_networks_rec *net_networks;
languagerec *languages;

char bbsdir[MAX_PATH];
char configdat[20] = "config.dat";

static void convcfg() {
  arcrec arc[MAX_ARCS];

  int hFile = open(configdat, O_RDWR | O_BINARY);
  if (hFile > 0) {
    out->SetColor(Scheme::INFO);
    Printf("Converting config.dat to 4.30/5.00 format...\n");
    out->SetColor(Scheme::NORMAL);
    read(hFile, (void *)(&syscfg), sizeof(configrec));
    sprintf(syscfg.menudir, "%smenus%c", syscfg.gfilesdir, WWIV_FILE_SEPERATOR_CHAR);
    strcpy(syscfg.logoff_c, " ");
    strcpy(syscfg.v_scan_c, " ");

    for (int i = 0; i < MAX_ARCS; i++) {
      if ((syscfg.arcs[i].extension[0]) && (i < 4)) {
        strncpy(arc[i].name, syscfg.arcs[i].extension, 32);
        strncpy(arc[i].extension, syscfg.arcs[i].extension, 4);
        strncpy(arc[i].arca, syscfg.arcs[i].arca, 32);
        strncpy(arc[i].arce, syscfg.arcs[i].arce, 32);
        strncpy(arc[i].arcl, syscfg.arcs[i].arcl, 32);
      } else {
        strncpy(arc[i].name, "New Archiver Name", 32);
        strncpy(arc[i].extension, "EXT", 4);
        strncpy(arc[i].arca, "archive add command", 32);
        strncpy(arc[i].arce, "archive extract command", 32);
        strncpy(arc[i].arcl, "archive list command", 32);
      }
    }
    lseek(hFile, 0L, SEEK_SET);
    write(hFile, (void *)(&syscfg), sizeof(configrec));
    close(hFile);

    char szFileName[ MAX_PATH ];
    sprintf(szFileName, "%sarchiver.dat", syscfg.datadir);
    hFile = open(szFileName, O_WRONLY | O_BINARY | O_CREAT, S_IREAD | S_IWRITE);
    if (hFile < 0) {
      Printf("Couldn't open '%s' for writing.\n", szFileName);
      Printf("Creating new file....");
      create_arcs();
      Printf("\n");
      hFile = open(szFileName, O_WRONLY | O_BINARY | O_CREAT, S_IREAD | S_IWRITE);
    }
    write(hFile, (void *)arc, MAX_ARCS * sizeof(arcrec));
    close(hFile);
  }
}

static void show_help() {
  Printf("   -Pxxx - Password via commandline (where xxx is your password)\n");
  Printf("\n\n\n");

}

WInitApp::WInitApp() {
  CursesIO::Init();
}

WInitApp::~WInitApp() {
  delete out;
  out = nullptr;
}

int main(int argc, char* argv[]) {
  std::unique_ptr<WInitApp> app(new WInitApp());
  return app->main(argc, argv);
}

int WInitApp::main(int argc, char *argv[]) {
  char s[81];
  int newbbs = 0, pwok = 0;

  setlocale (LC_ALL,"");

  char *ss = getenv("WWIV_DIR");
  if (ss) {
    WWIV_ChangeDirTo(ss);
  }
  getcwd(bbsdir, MAX_PATH);

  trimstrpath(bbsdir);

  out->Cls();
  out->SetColor(Scheme::NORMAL);

  int configfile = open(configdat, O_RDWR | O_BINARY);
  if (configfile > 0) {
    // try to read it initially so we can process args right.
    read(configfile, &syscfg, sizeof(configrec));
    close(configfile);
  }
  for (int i = 1; i < argc; ++i) {
    if (strlen(argv[i]) < 2) {
      continue;
    }

    if (argv[i][0] == '-') {
      char ch = toupper(argv[i][1]);
      switch (ch) {
      case 'P': {
        if (strlen(argv[i]) > 2) {
          if (stricmp(argv[i] + 2, syscfg.systempw) == 0) {
            pwok = 1;
          }
        }
        break;
      }
      case 'D': {
        setpaths();
        out->Cls();
        exit_init(0);
      } break;
      case '?':
        show_help();
        exit_init(0);
        break;
      }
    }
  }

  configfile = open(configdat, O_RDWR | O_BINARY);
  if (configfile < 0) {
    out->SetColor(Scheme::ERROR_TEXT);
    Printf("%s NOT FOUND.\n\n", configdat);
    if (dialog_yn("Perform initial installation")) {
      new_init();
      newbbs = 1;
      configfile = open(configdat, O_RDWR | O_BINARY);
    } else {
      exit_init(1);
    }
  }

  // Convert 4.2X to 4.3 format if needed.
  if (filelength(configfile) != sizeof(configrec)) {
    close(configfile);
    convcfg();
    configfile = open(configdat, O_RDWR | O_BINARY);
  }

  read(configfile, &syscfg, sizeof(configrec));
  close(configfile);

  configfile = open("config.ovr", O_RDONLY | O_BINARY);
  if ((configfile > 0) && (filelength(configfile) < (int)sizeof(configoverrec))) {
    // If the config.ovr file is too small then pretend it does not exist.
    close(configfile);
    configfile = -1;
  }
  if (configfile < 0) {
    // slap in the defaults
    for (int i = 0; i < 4; i++) {
      syscfgovr.com_ISR[i + 1] = syscfg.com_ISR[i + 1];
      syscfgovr.com_base[i + 1] = syscfg.com_base[i + 1];
      syscfgovr.com_ISR[i + 5] = syscfg.com_ISR[i + 1];
      syscfgovr.com_base[i + 5] = syscfg.com_base[i + 1];
    }

    syscfgovr.com_ISR[0] = syscfg.com_ISR[1];
    syscfgovr.com_base[0] = syscfg.com_base[1];

    syscfgovr.primaryport = syscfg.primaryport;
    strcpy(syscfgovr.modem_type, syscfg.modem_type);
    strcpy(syscfgovr.tempdir, syscfg.tempdir);
    strcpy(syscfgovr.batchdir, syscfg.batchdir);
  } else {
    lseek(configfile, 0, SEEK_SET);
    read(configfile, &syscfgovr, sizeof(configoverrec));
    close(configfile);
  }

  sprintf(s, "%sarchiver.dat", syscfg.datadir);
  int hFile = open(s, O_RDONLY | O_BINARY);
  if (hFile < 0) {
    create_arcs();
  } else {
    close(hFile);
  }

  if (!syscfg.dial_prefix[0]) {
    strcpy(syscfg.dial_prefix, "ATDT");
  }

  externs = (newexternalrec *) malloc(15 * sizeof(newexternalrec));
  initinfo.numexterns = 0;
  sprintf(s, "%snextern.dat", syscfg.datadir);
  hFile = open(s, O_RDWR | O_BINARY);
  if (hFile > 0) {
    initinfo.numexterns = (read(hFile, (void *)externs, 15 * sizeof(newexternalrec))) / sizeof(newexternalrec);
    close(hFile);
  }
  over_intern = (newexternalrec *) malloc(3 * sizeof(newexternalrec));
  memset(over_intern, 0, 3 * sizeof(newexternalrec));
  sprintf(s, "%snintern.dat", syscfg.datadir);
  hFile = open(s, O_RDWR | O_BINARY);
  if (hFile > 0) {
    read(hFile, over_intern, 3 * sizeof(newexternalrec));
    close(hFile);
  }

  initinfo.numeditors = 0;    
  editors = (editorrec *)   malloc(10 * sizeof(editorrec));
  sprintf(s, "%seditors.dat", syscfg.datadir);
  hFile = open(s, O_RDWR | O_BINARY);
  if (hFile > 0) {
    initinfo.numeditors = (read(hFile, (void *)editors, 10 * sizeof(editorrec))) / sizeof(editorrec);
    initinfo.numeditors = initinfo.numeditors;
    close(hFile);
  }

  bool bDataDirectoryOk = read_status();
  if (bDataDirectoryOk) {
    if ((status.net_version >= 31) || (status.net_version == 0)) {
      net_networks = (net_networks_rec *) malloc(MAX_NETWORKS * sizeof(net_networks_rec));
      if (!net_networks) {
        Printf("needed %d bytes\n", MAX_NETWORKS * sizeof(net_networks_rec));
        exit_init(2);
      }
      memset(net_networks, 0, MAX_NETWORKS * sizeof(net_networks_rec));

      sprintf(s, "%snetworks.dat", syscfg.datadir);
      hFile = open(s, O_RDONLY | O_BINARY);
      if (hFile > 0) {
        initinfo.net_num_max = filelength(hFile) / sizeof(net_networks_rec);
        if (initinfo.net_num_max > MAX_NETWORKS) {
          initinfo.net_num_max = MAX_NETWORKS;
        }
        if (initinfo.net_num_max) {
          read(hFile, net_networks, initinfo.net_num_max * sizeof(net_networks_rec));
        }
        close(hFile);
      }
    }
  }

  languages = (languagerec*) malloc(MAX_LANGUAGES * sizeof(languagerec));
  if (!languages) {
    Printf("needed %d bytes\n", MAX_LANGUAGES * sizeof(languagerec));
    exit_init(2);
  }

  sprintf(s, "%slanguage.dat", syscfg.datadir);
  int hLanguageFile = open(s, O_RDONLY | O_BINARY);
  if (hLanguageFile >= 0) {
    initinfo.num_languages = filelength(hLanguageFile) / sizeof(languagerec);
    read(hLanguageFile, languages, initinfo.num_languages * sizeof(languagerec));
    close(hLanguageFile);
  }
  if (!initinfo.num_languages) {
    initinfo.num_languages = 1;
    strcpy(languages->name, "English");
    strncpy(languages->dir, syscfg.gfilesdir, sizeof(languages->dir) - 1);
    strncpy(languages->mdir, syscfg.gfilesdir, sizeof(languages->mdir) - 1);
  }

  if (languages->mdir[0] == 0) {
    strncpy(languages->mdir, syscfg.gfilesdir, sizeof(languages->mdir) - 1);
  }

  if (!pwok) {
    nlx();
    std::vector<std::string> lines { "Please enter the System Password. "};
    if (newbbs) {
      lines.insert(lines.begin(), "");
      lines.insert(lines.begin(), "Note: Your system password defaults to 'SYSOP'.");
    }
    input_password("SY:", lines, s, 20);
    if (strcmp(s, (syscfg.systempw)) != 0) {
      out->Cls();
      nlx(2);
      out->SetColor(Scheme::ERROR_TEXT);
      Printf("I'm sorry, that isn't the correct system password.\n");
      exit_init(2);
    }
  }

  bool done = false;
  do {
    out->Cls();
    out->SetDefaultFooter();

    out->SetColor(Scheme::NORMAL);
    int y = 1;
    int x = 0;
    out->PutsXY(x, y++, "G. General System Configuration");
    out->PutsXY(x, y++, "P. System Paths");
    out->PutsXY(x, y++, "T. External Transfer Protocol Configuration");
    out->PutsXY(x, y++, "E. External Editor Configuration");
    out->PutsXY(x, y++, "S. Security Level Configuration");
    out->PutsXY(x, y++, "V. Auto-Validation Level Configuration");
    out->PutsXY(x, y++, "A. Archiver Configuration");
    out->PutsXY(x, y++, "I. Instance Configuration");
    out->PutsXY(x, y++, "L. Language Configuration");
    out->PutsXY(x, y++, "N. Network Configuration");
    out->PutsXY(x, y++, "R. Registration Information");
    out->PutsXY(x, y++, "U. User Editor");
    out->PutsXY(x, y++, "X. Update Sub/Directory Maximums");
    out->PutsXY(x, y++, "Q. Quit");

    y++;
    out->SetColor(Scheme::PROMPT);
    out->PutsXY(x, y++, "Command? ");
    out->SetColor(Scheme::NORMAL);
    switch (onek("QAEGILNPRSTUVX$\033")) {
    case 'Q':
    case '\033':
      done = true;
      break;
    case 'G':
      out->SetDefaultFooter();
      sysinfo1();
      break;
    case 'P':
      out->SetDefaultFooter();
      setpaths();
      break;
    case 'T':
      out->SetDefaultFooter();
      extrn_prots();
      break;
    case 'E':
      out->SetDefaultFooter();
      extrn_editors();
      break;
    case 'S':
      out->SetDefaultFooter();
      sec_levs();
      break;
    case 'V':
      out->SetDefaultFooter();
      autoval_levs();
      break;
    case 'A':
      out->SetDefaultFooter();
      edit_arc(0);
      break;
    case 'I':
      instance_editor();
      break;
    case 'L':
      out->SetDefaultFooter();
      edit_languages();
      break;
    case 'N':
      out->SetDefaultFooter();
      networks();
      break;
    case 'R':
      out->SetDefaultFooter();
      edit_registration_code();
      break;
    case 'U':
      user_editor();
      break;
    case '$':
      nlx();
      out->SetDefaultFooter();
      Printf("QSCan Lenth: %lu\n", syscfg.qscn_len);
      Printf("WWIV %s%s INIT compiled %s\n", wwiv_version, beta_version, const_cast<char*>(wwiv_date));
      out->GetChar();
      break;
    case 'X':
      out->SetDefaultFooter();
      up_subs_dirs();
      break;
    }
    out->SetIndicatorMode(IndicatorMode::NONE);
  } while (!done);

  // Don't leak the localIO (also fix the color when the app exits)
  delete out;
  out = nullptr;
  return 0;
}
