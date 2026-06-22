/*----------------------------------------------------------------------------*/
/* acqifcom.h common header file                                              */
/*----------------------------------------------------------------------------*/

#ifndef	 _ACQIFCOM_H
#define	 _ACQIFCOM_H

#include<openssl/aes.h>

/*--------------------------------------------------------------------*/
/* CONSTANT DEFINE                                                    */
/*--------------------------------------------------------------------*/
#define SHMKEY_ACQMAIN	0x31202
#define SHMKEY_ACQIF	0x31302
#define ACQ_PORT		(30900)  


#define ACQ_PROC_NAME	"acqifpxx"

#define UNIX_SOCKET_PATH "/home/atmadm/.config/acqsckttmp"
#define ACQ_PROC_PATH "/home/atmadm/.config/"


typedef struct
{
    char    userid  [10];           /* teller/user id             */
    char    pstat   [ 1];           /* "R"(running) / "D"(down)   */
    char    pidname [ 8];           /* process name               */
    char    pid     [10];           /* process id                 */
    char    snquekey[ 8];           /* send message que key value */
    char    rvquekey[ 8];           /* recv message que key value */
    char    queowner[ 7];           /* message queue owner value  */
    char    querdcnt[ 7];           /* read count from msg queue  */
    char    quewrcnt[ 7];           /* write count to msg queue   */
    char    tcpsncnt[ 7];           /* send count to atm socket   */
    char    tcprvcnt[ 7];           /* recv count from atm socket */
    char    pstrtdt [14];           /* process start date         */
    char    pdowndt [14];           /* process down date          */
    char    bankcd  [ 2];           /* bank code                  */
    char    branchcd[ 4];           /* branch code                */
    char    clientip[15];           /* client ip address          */
    char    atmstat [56];           /* atm status data            */
    char    lastinfo[60];           /* transaction last info      */
    char    mastkey [16];           /* DES Master Key             */
    char    workkey [32];           /* encryption working key     */
} s_newprocinfo;

/*----------------------------------------------------------------*/
/* ATM PROCESS CONTROL SHARED MEMORY STRUCTURE                    */
/*----------------------------------------------------------------*/
typedef struct
{
    char    pidname [10];           /* process name               */
    char    pstat   [ 1];           /* "R"(running) / "D"(down)   */
    char    startdt [14];           /* process start date         */
    char    downdt  [14];           /* process down date          */
    char    desc    [20];           /* process description        */
    char    reserved[20];           /* reserved area              */
    char    newkey  [200];           /* new key under LMK          */
    char    zmastkey[200];           /* ZONE Master Key            */
    char    zpinkey [200];           /* ZONE PIN Key               */
} s_newdaemoninfo;

struct msgValidRule
{
    int pos;     /* field position                     */
    int type;  /* field type 1 : string - 2 : long    */
    int length;  /* field length   */
    int mandYn;  /* mandatory field 1 : yes */
    char mandVal[50];       /* constrain value */
} msgValid[] =
{
    {	0	,	1	,	8	,	1	, "00002655"      },	
{	8	,	2	,	4	,	1	,	  ""              },	
{	12	,	2	,	4	,	1	,	  ""              },	
{	16	,	1	,	1	,	0	,	  ""              },	
{	17	,	1	,	1	,	0	,	  ""              },	
{	18	,	1	,	1	,	0	,	  ""              },	
{	19	,	1	,	8	,	0	,	  ""              },	
{	27	,	1	,	8	,	0	,	  ""              },	
{	35	,	1	,	2	,	0	,	  ""              },	
{	37	,	1	,	12	,	1	,	  "SGAT9451A001"  },	
{	49	,	1	,	1	,	0	,	  ""              },	
{	50	,	1	,	2	,	0	,	  ""              },	
{	52	,	1	,	1	,	0	,	  ""              },	
{	53	,	1	,	1	,	0	,	  ""              },	
{	54	,	1	,	1	,	0	,	  ""              },	
{	55	,	1	,	1	,	0	,	  ""              },	
{	56	,	1	,	1	,	0	,	  ""              },	
{	57	,	1	,	1	,	0	,	  ""              },	
{	58	,	1	,	30	,	1	,	  ""              },	  
{	88	,	1	,	8	,	0	,	  ""              },	
{	96	,	1	,	4	,	0	,	  ""              },	
{	100	,	1	,	10	,	0	,	  ""              },	
{	110	,	1	,	20	,	0	,	  ""              },	
{	130	,	2	,	1	,	0	,	  ""              },	
{	131	,	1	,	6	,	0	,	  "000000"        },	   
{	137	,	2	,	12	,	0	,	  ""              },	
{	149	,	2	,	2	,	0	,	  ""              },	
{	151	,	2	,	4	,	0	,	  ""              },	
{	155	,	1	,	4	,	1	,	  "02000420"     },	
{	159	,	1	,	16	,	0	,	  ""              },	
{	175	,	1	,	16	,	0	,	  ""              },	
{	191	,	1	,	19	,	1	,	  ""              },	
{	210	,	1	,	6	,	1	,	  "000000"        },	
{	216	,	2	,	12	,	1	,	  ""              },	
{	228	,	1	,	10	,	1	,	  ""              },	
{	238	,	2	,	6	,	1	,	  ""              },	
{	244	,	1	,	6	,	1	,	  ""              },	
{	250	,	1	,	4	,	1	,	  ""              },	
{	254	,	1	,	4	,	0	,	  ""              },	
{	258	,	2	,	4	,	1	,	  ""              },	
{	262	,	2	,	3	,	0	,	  ""              },	
{	265	,	1	,	4	,	1	,	  ""              },	
{	269	,	1	,	2	,	1	,	  ""              },	
{	271	,	1	,	11	,	1	,	  "970424     "   },	
{	282	,	1	,	37	,	0	,	  ""              },	
{	319	,	1	,	12	,	1	,	  ""              },	
{	331	,	1	,	6	,	0	,	  ""              },	
{	337	,	1	,	2	,	0	,	  ""              },	
{	339	,	1	,	8	,	1	,	  ""              },	
{	347	,	1	,	15	,	1	,	  ""              },	
{	362	,	1	,	40	,	1	,	  ""              },	
{	402	,	1	,	3	,	1	,	  "704"           },	
{	405	,	1	,	16	,	0	,	  ""              },	
{	421	,	1	,	20	,	0	,	  ""              },	
{	441	,	1	,	3	,	0	,	  ""              },	
{	444	,	1	,	42	,	0	,	  ""              },	
{	486	,	1	,	28	,	0	,	  ""              },	
{	514	,	1	,	28	,	0	,	  ""              },	
{	542	,	1	,	200	,	0	,	  ""              },	
{	742	,	1	,	16	,	0	,	  ""              },	
{	758	,	1	,	30	,	0	,	  ""              },	
{	788	,	1	,	30	,	0	,	  ""              },	
{	818	,	1	,	2	,	0	,	  ""              },	
{	820	,	1	,	32	,	0	,	  ""              },	
{	852	,	1	,	8	,	0	,	  ""              },	
{	860	,	2	,	11	,	0	,	  ""              },	
{	871	,	1	,	25	,	0	,	  ""              },	
{	896	,	1	,	10	,	0	,	  ""              },	
{	906	,	2	,	11	,	0	,	  ""              },	
{	917	,	1	,	8	,	0	,	  ""              },	
{	925	,	1	,	50	,	0	,	  ""              },	
{	975	,	1	,	79	,	0	,	  ""              },	
{	1054	,	2	,	2	,	0	,	  ""          },	
{	1056	,	2	,	2	,	0	,	  ""          },	
{	1058	,	1	,	10	,	0	,	  ""          },	
{	1068	,	1	,	50	,	0	,	  ""          },	
{	1118	,	1	,	16	,	0	,	  ""          },	
{	1134	,	1	,	16	,	0	,	  ""          },	
{	1150	,	1	,	5	,	0	,	  ""          },	
{	1155	,	1	,	2	,	0	,	  ""          },	
{	1157	,	1	,	210	,	0	,	  ""          },	
{	1367	,	1	,	700	,	0	,	  ""          },	
{	2067	,	2	,	12	,	0	,	  ""          },	
{	2079	,	2	,	12	,	0	,	  ""          },	
{	2091	,	2	,	8	,	0	,	  ""          },	
{	2099	,	2	,	8	,	0	,	  ""          },	
{	2107	,	1	,	3	,	0	,	  ""          },	
{	2110	,	1	,	3	,	0	,	  ""          },	
{	2113	,	1	,	43	,	0	,	  ""          },	
{	2156	,	1	,	2	,	0	,	  ""          },	
{	2158	,	1	,	500	,	0	,	  ""          },	 
{	2658	,	2	,	3	,	0	,	  ""          },	
{	0	,	0	,	0	,	0	,	  ""              }
};

#ifdef MAINSRC
    char AESClearKey[200 +1];
    s_newprocinfo    pNewPrcInfo;   
    s_newdaemoninfo  pNewDmonInfo;
#else
    extern s_newprocinfo    pNewPrcInfo;
    extern s_newdaemoninfo  pNewDmonInfo;
    extern char AESClearKey[200 +1];
#endif

#endif  /* _ACQIFCOM_H */
