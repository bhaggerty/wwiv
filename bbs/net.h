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
#ifndef __INCLUDED_NET_H__
#define __INCLUDED_NET_H__

#include <cstdint>

#pragma pack(push, 1)

/* Defining USE_FTS activates The File Transfer System */
#define USE_FTS

/* All network nodes except the destination will only look at:
 *   tosys - the destination system.  Is 0 if more than one system
 *   list_len - if (tosys==0) then list_len is the number of destination
 *     systems in a list immediately after the header.  NOTE this list will
 *     be 2*list_len bytes long, since each system entry is 2 bytes.
 *   length - is the overall length of the message, not including the header
 *     or the system ist length.  Thus the overall length of a message in
 *     the net will be (sizeof(net_header_rec) + 2*list_len + length)
 */


struct net_header_rec {
  uint16_t  tosys,    /* destination system */
           touser,     /* destination user */
           fromsys,    /* originating system */
           fromuser,   /* originating user */
           main_type,  /* main message type */
           minor_type, /* minor message type */
           list_len;   /* # of entries in system list */
  uint32_t   daten,      /* date/time sent */
             length;     /* # of bytes of msg after header */
  uint16_t  method;   /* method of compression */
};


/*
 * Please note that not all of these are used yet, some will probably never
 * be used, but sounded like a good idea at the time.
 */

#define main_type_net_info        0x0001  /* type 1 normal network updates */
#define main_type_email           0x0002  /* type 2 email by user number */
#define main_type_post            0x0003  /* type 3 post from sub host */
#define main_type_file            0x0004  /* type 4 file transfer system */
#define main_type_pre_post        0x0005  /* type 5 post to sub host */
#define main_type_external        0x0006  /* type 6 external message */
#define main_type_email_name      0x0007  /* type 7 email by user name */
#define main_type_net_edit        0x0008  /* type 8 network editor packet */
#define main_type_sub_list        0x0009  /* type 9 subs.lst update */
#define main_type_extra_data      0x000a  /* type 10 unused */
#define main_type_group_bbslist   0x000b  /* type 11 network update from GC */
#define main_type_group_connect   0x000c  /* type 12 network update from GC */
#define main_type_unsed_1         0x000d  /* type 13 unused */
#define main_type_group_info      0x000e  /* type 14 misc update from GC */
#define main_type_ssm             0x000f  /* type 15 xxx read your mail */
#define main_type_sub_add_req     0x0010  /* type 16 add me to your sub */
#define main_type_sub_drop_req    0x0011  /* type 17 remove me from your sub*/
#define main_type_sub_add_resp    0x0012  /* type 18 status of add, 0=ok */
#define main_type_sub_drop_resp   0x0013  /* type 19 status of drop, 0=ok */
#define main_type_sub_list_info   0x0014  /* type 20 info for subs.lst file */

#define main_type_new_post        0x001a  /* type 26 post by sub name */
#define main_type_new_extern      0x001b  /* type 27 auto-proc ext. msgs */
#define main_type_game_pack       0x001c  /* type 28 game packs */


/* these are in main_type_sub_*_resp, as the first byte of the text */
#define sub_adddrop_ok            0x00  /* you've been added/removed */
#define sub_adddrop_not_host      0x01  /* don't host that sub */
#define sub_adddrop_not_there     0x02  /* can't drop, you're not listed */
#define sub_adddrop_not_allowed   0x03  /* not allowed to add/drop you */
#define sub_adddrop_already_there 0x04  /* already in sub */
#define sub_adddrop_error         0xff  /* internal error */

struct net_contact_rec {
  uint16_t  systemnumber,       /* System number of the contact */
           numcontacts,        /* # of contacts with system */
           numfails;           /* # of consec failed calls out */
  uint32_t firstcontact,       /* time of first contact w/ system */
           lastcontact,        /* time of most recent contact */
           lastcontactsent,    /* time of last contact w/data sent */
           lasttry,            /* time of last try to connect */
           bytes_received,     /* bytes received from system */
           bytes_sent,         /* bytes sent to system */
           bytes_waiting;      /* bytes waiting to be sent */
};

/* Each system will hold a file of these records.  Each record will hold the
 * data pertaining to all contacts with other systems.
 *
 * systemnumber tells what other system this data is for.
 * numcontacts - how many times a connection has been made with that system
 * numfails tells how many times this system has tried to call this other
 *   system but failed.  This is consecutive times, and is zeroed out
 *   when a successful connection is made by calling out.
 * firstcontact is time in unix format of the last time that system has been
 *   contacted on the net.
 * lastcontact is the time the last contact with that system was made.
 * lastcontactsent is the last time data was SENT to that system.
 * lasttry is the last time we tried to call out to that system.
 * bytes_received is number of bytes received from that system.
 * bytes_sent is number of bytes sent to that system.
 * bytes_waiting is the number of bytes waiting to be sent out to that
 *   system.
 *
 * This data is maintained so that the BBS can decide which other system to
 * attempt connection with next.
 */


struct net_system_list_rec {
  uint16_t  sysnum;         /* system number of the system */
  char            phone[13],      /* phone number of system */
                  name[40];       /* name of system */
  uint8_t   group;          /* group of the system */
  uint16_t  speed,          /* max baud rate of system */
           other,          /* other info about sys (bit-mapped)*/
           forsys;         /* how to get there */
  int16_t           numhops;        /* how long to get there */
  union {
    uint16_t  rout_fact;  /* routing factor */
    float           cost;       /* cost factor */
    int32_t            temp;       /* temporary variable */
  } xx;
};

// This data is all read in from a text file which holds info about all of
// the systems in the network.  This text file doesn't hold connection info
// between the systems.  The purpose of all records should be obvious.



//
// BBSLIST designators
//

#define other_inet          0x0001  /* $ - System is PPP capable         */
#define other_fido          0x0002  /* \ - System run Fido frontend      */
#define other_telnet        0x0004  /* | - System is a telnet node       */
#define other_no_links      0x0008  /* < - System refuses links          */
#define other_fts_blt       0x0010  /* > - System uses FTS/BLT system    */
#define other_direct        0x0020  /* ! - System accepts direct connects*/
#define other_unregistered  0x0040  /* / - System is unregistered        */
#define other_fax           0x0080  /* ? - System accepts faxes          */
#define other_end_system    0x0100  /* _ - System is a dead end node     */
#define other_net_server    0x0200  /* + - System is network server      */
#define other_unused        0x0400  /* = - Unused identifier 2           */

/* system type designators */
#define other_net_coord     0x0800  /* & - NC */
#define other_group_coord   0x1000  /* % - GC */
#define other_area_coord    0x2000  /* ^ - AC */
#define other_subs_coord    0x4000  /* ~ - Sub Coordinator */



struct net_interconnect_rec {
  uint16_t  sysnum,     /* outward calling system */
           numsys;     /* num systems it can call */
  uint16_t  *connect;   /* list of numsys systems */
  float           *cost;      /* list of numsys costs */
};

/* This data is also read in from a text file.  It tells how much it costs for
 * sysnum to call out to other systems.
 *
 * numsys - is the number of systems that sysnum can call out to
 * connect[] - points to an array of numsys integers that tell which
 *   other systems sysnum can connect to
 * cost[] - points to an array of numsys floating point numbers telling
 *   how much it costs to connect to that system, per minute.  ie, it would
 *   cost (cost[1]) dollars per minute for sysnum to call out to system
 *   number (connect[1]).
 */


struct net_call_out_rec {
  uint16_t  sysnum;         /* system number */
  uint8_t   macnum;         /* macro/script to use */
  uint16_t  options;        /* bit mapped */
  uint8_t   call_anyway;    /* days between callouts */
  char            min_hr,         /* callout min hour */
                  max_hr;         /* callout max hour */
  char            password[20];   /* password for system */
  uint8_t   times_per_day;  /* number of calls per day */
  uint8_t   call_x_days;    /* call only every x days */
  uint16_t  min_k;          /* minimum # k before callout */
  char *          opts;           /* options or NULL */
};

/* This record holds info about other systems that the sysop has determined
 * that we can call out to.
 *
 * sysnum - system number that we can call
 * macnum - macro/script number to use to call that system.  This is set
 *   to zero if we just dial it directly.
 * options - bit mapped.  See #define's below.
 * call_anyway - if non-zero, then the minimum number of days between calls
 *   to that system, even if there is nothing waiting to be sent there.  This
 *   should only be non-zero if options_sendback is also set for this system.
 * password - is the password used for connection to this system.
 */


#define options_sendback      0x0001   /* & they can send data back */
#define options_ATT_night     0x0002   /* - callout only at AT&T nigh hours */
#define options_ppp           0x0004   /* _ transfer via PPP */
#define options_no_call       0x0008   /* + don't call that system, it will */
#define options_receive_only  0x0010   /* ~ never send anything */
#define options_once_per_day  0x0020   /* ! only call once per day */
#define options_compress      0x0040   /* ; compress data */
#define options_hslink        0x0080   /* ^ use HSLINK if available */
#define options_force_ac      0x0100   /* $ force area code on dial */
#define options_dial_ten      0x0200   /* * use ten digit dialing format */
#define options_hide_pend     0x0400   /* = hide in pending display */

struct sys_for_rec {
  uint16_t  sysnum;         /* system number we can call */
  int16_t           nument;         /* number of entries in list */
  uint16_t  *list;          /* list of systems */
};


struct net_networks_rec {
  uint8_t   type;           /* type of network */
  char            name[16];       /* network name */
  char            dir[69];        /* directory for net data */
  uint16_t  sysnum;         /* system number */
  net_call_out_rec  *con;         /* ptr to callout data */
  net_contact_rec  *ncn;          /* ptr to contact info */
  int16_t           num_con;        /* number in array */
  int16_t           num_ncn;        /* number in array */
};

#define net_type_wwivnet  0
#define net_type_fidonet  1
#define net_type_internet 2

/*************************/
/* FTS System Structures */
/*************************/
#ifdef USE_FTS
struct fts_header {
  char            filename[13];       /* name of file */
  int32_t            filesize;           /* size of file */
  char            description[59];    /* description  */
  uint32_t   crc32value;         /* crc value of file */
  int16_t           maintype,           /* fts maintype */
                  fdltype,            /* fdl type */
                  chunkcount,         /* number of this chunk */
                  totalchunks;        /* total number of chunks */
  char            sysname[60],        /* name of originating system */
                  tonet[16];          /* destination network name */
  int16_t           tosys,              /* destination system */
                  touser;             /* destination user */
  char            fromnet[16];        /* originating network name */
  int16_t           fromsys,            /* originating system */
                  fromuser,           /* originating user */
                  ver,                /* version of fts system */
                  info;               /* 1=key 2=sec'd file req  */
  char            xtrastuff[16];      /*                                */
};

#define fdl_requestable 0x0001
#define fdl_postable    0x0004
#define fdl_wwivreg     0x0008

struct fdlrec_rec {
  char  name[61];
  int16_t fdl,
        host;
  int32_t  status;
  char  res[41];
};

struct bltrec_rec {
  char  name[61];
  int16_t blt,
        host;
  int32_t  status;
  char  res[41];
};

#endif

#pragma pack(pop)

#endif // __INCLUDED_NET_H__
