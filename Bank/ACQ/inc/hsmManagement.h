/*============================================================================*/
/* 프로그램명  : strinet.h                                                    */
/* 실행화일명  : libsysutil.a                                                 */
/* 목      적  : inet domain Stream socket Interface Libarary Header File     */
/* Programmer  : DongYoung Lee                                                */
/* Target User : CHOHUNG VINA BANK                                            */
/* Description : Copyright (C) 2003 NAUTILUS HYOSUNG. Ltd                     */
/*               All rights Reserved                                          */
/*----------------------------------------------------------------------------*/
/*   VERSION      DATE          NAME             COMMENT                      */
/*   VER 1.00     2003.11.06    DYLEE            최초작성                     */
/*----------------------------------------------------------------------------*/  
#ifndef _H_HSMMANAGEMENT_
#define _H_HSMMANAGEMENT_
#include <strinet.h>
#include <sys/un.h>
#include <fcntl.h>
#include <time.h>
#include <sys/poll.h>   

int custLoglevel;

#define  UNIX

#define LOG_DEBUG 1
#define LOG_INFO 2
#define LOG_WARNING 3
#define LOG_ERROR 9
/*----------------------------------------------------------------*/
/* IN/OUT Struct define     			                          */
/*----------------------------------------------------------------*/
#define LEN_MGATHSMEXEC_IN_TMN_IP            15
#define LEN_MGATHSMEXEC_IN_RQST_SVC_C        12
#define LEN_MGATHSMEXEC_IN_SRC_ZPK_CTT       200
#define LEN_MGATHSMEXEC_IN_DEST_ZPK_CTT      200
#define LEN_MGATHSMEXEC_IN_DEST_MAK_CTT      200
#define LEN_MGATHSMEXEC_IN_ZMK_CTT           200
#define LEN_MGATHSMEXEC_IN_MAC_CTT           16
#define LEN_MGATHSMEXEC_IN_MAC_MSG           800
#define LEN_MGATHSMEXEC_IN_LMK_ID            2
#define LEN_MGATHSMEXEC_IN_ATM_PAN_CTT       19
#define LEN_MGATHSMEXEC_IN_ATM_PSN_PIN_NW    33

struct hsmManagement_in {
    long sh_lcl_corp_c;
    long trx_func_d;
    char tmn_ip[LEN_MGATHSMEXEC_IN_TMN_IP + 1];
    long brd_connt_port_no;
    char rqst_svc_c[LEN_MGATHSMEXEC_IN_RQST_SVC_C + 1];
    char src_zpk_ctt[LEN_MGATHSMEXEC_IN_SRC_ZPK_CTT + 1];
    char dest_zpk_ctt[LEN_MGATHSMEXEC_IN_DEST_ZPK_CTT + 1];
    char dest_mak_ctt[LEN_MGATHSMEXEC_IN_DEST_MAK_CTT + 1];
    char zmk_ctt[LEN_MGATHSMEXEC_IN_ZMK_CTT + 1];
    char mac_ctt[LEN_MGATHSMEXEC_IN_MAC_CTT + 1];
    char mac_msg[LEN_MGATHSMEXEC_IN_MAC_MSG + 1];
    char lmk_id[LEN_MGATHSMEXEC_IN_LMK_ID + 1];
    char atm_pan_ctt[LEN_MGATHSMEXEC_IN_ATM_PAN_CTT + 1];
    char atm_psn_pin_nw[LEN_MGATHSMEXEC_IN_ATM_PSN_PIN_NW + 1];
};
typedef struct hsmManagement_in hsmManagement_in;

#define LEN_MGATHSMEXEC_OUT_SRC_ZPK_CTT      200
#define LEN_MGATHSMEXEC_OUT_DEST_ZPK_CTT     200
#define LEN_MGATHSMEXEC_OUT_DEST_MAK_CTT     200
#define LEN_MGATHSMEXEC_OUT_MAC_CTT          16
#define LEN_MGATHSMEXEC_OUT_ATM_PSN_PIN_NW   33
#define LEN_MGATHSMEXEC_OUT_ATM_RESPOND_C    2
#define LEN_MGATHSMEXEC_OUT_SRC_MAK_CTT      200

struct hsmManagement_out {
    char src_zpk_ctt[LEN_MGATHSMEXEC_OUT_SRC_ZPK_CTT + 1];
    char dest_zpk_ctt[LEN_MGATHSMEXEC_OUT_DEST_ZPK_CTT + 1];
    char dest_mak_ctt[LEN_MGATHSMEXEC_OUT_DEST_MAK_CTT + 1];
    char mac_ctt[LEN_MGATHSMEXEC_OUT_MAC_CTT + 1];
    char atm_psn_pin_nw[LEN_MGATHSMEXEC_OUT_ATM_PSN_PIN_NW + 1];
    char atm_respond_c[LEN_MGATHSMEXEC_OUT_ATM_RESPOND_C + 1];
    char src_mak_ctt[LEN_MGATHSMEXEC_OUT_SRC_MAK_CTT + 1];
};
typedef struct hsmManagement_out hsmManagement_out;


#define INIT_hsmManagement_out(x) do {     memset((x).src_zpk_ctt, 0x00, sizeof((x).src_zpk_ctt));     memset((x).dest_zpk_ctt, 0x00, sizeof((x).dest_zpk_ctt));     memset((x).dest_mak_ctt, 0x00, sizeof((x).dest_mak_ctt));     memset((x).mac_ctt, 0x00, sizeof((x).mac_ctt));     memset((x).atm_psn_pin_nw, 0x00, sizeof((x).atm_psn_pin_nw));     memset((x).atm_respond_c, 0x00, sizeof((x).atm_respond_c));     memset((x).src_mak_ctt, 0x00, sizeof((x).src_mak_ctt)); } while(0)

#define INIT_hsmManagement_in(x) do {     (x).sh_lcl_corp_c = 0;    (x).trx_func_d = 0;    memset((x).tmn_ip, 0x00, sizeof((x).tmn_ip));     memset((x).rqst_svc_c, 0x00, sizeof((x).rqst_svc_c));     memset((x).src_zpk_ctt, 0x00, sizeof((x).src_zpk_ctt));     memset((x).dest_zpk_ctt, 0x00, sizeof((x).dest_zpk_ctt));     memset((x).dest_mak_ctt, 0x00, sizeof((x).dest_mak_ctt));     memset((x).zmk_ctt, 0x00, sizeof((x).zmk_ctt));     memset((x).mac_ctt, 0x00, sizeof((x).mac_ctt));     memset((x).mac_msg, 0x00, sizeof((x).mac_msg));     memset((x).lmk_id, 0x00, sizeof((x).lmk_id));     memset((x).atm_pan_ctt, 0x00, sizeof((x).atm_pan_ctt));     memset((x).atm_psn_pin_nw, 0x00, sizeof((x).atm_psn_pin_nw)); } while(0)

typedef struct {
    hsmManagement_in *input; 
    hsmManagement_out *output; 
    char r_data[1024 + 1];
    char r_code[2 + 1];
    char hsm_recvBuffer[1024 + 1];
    long hsm_rcvMsgLen;  
    char hsm_sendBuffer[1024 + 1];
    long hsm_sndMsgLen;
    int sockfd_hsm;
} hsmManagementContext;

/*----------------------------------------------------------------*/
/* M6 -> M7 (Generate a MAC) 			                          */
/*----------------------------------------------------------------*/
typedef struct
{
    char    msghead         [ 4];           /* Message Header              */
    char    cmdcode         [ 2];           /* Command Code                */
    char    modflag         [ 1];           /* Mode Flag (usually set "0") */
    char    Inputformatflag [ 1];           /* '0' binary, '1' Hex Encoded Binary, '2' Text */
    char    macsize         [ 1];   		/* use '1'					  */	
    char    macalgorithm    [ 1];           /* use '3'					  */	
    char    padmethod       [ 1];         /* use ?				  		  */	
    char    keytype         [ 3];           /* 008 ZAK					  */
    char    key             [ 200];              /* 					          */
    char    msglength       [ 4];         /* 					          */
    char    msg             [ 560];         	   /* 					          */
    char    lmkdeli         [1];        /* Value '%'. Optional; if present, the following field must also be present.            */
    char    lmkiden         [2];        /* LMK Identifier           */
} s_Cmdm6;

typedef struct
{
    char    msghead  [ 4];         /* Message Header                */
    char    respcode [ 2];         /* Response Code                 */
    char    errcode  [ 2];         /* Error Code                    */
    char    macdata	 [16];         /* return Mac Value				*/	
} r_Cmdm7;

/*-------------------------------------------------------------------*/
/* M8 -> M9 Verify Mac												 */
/*-------------------------------------------------------------------*/
typedef struct
{
    char    msghead         [ 4];           /* Message Header              */
    char    cmdcode         [ 2];           /* Command Code                */
    char    modflag         [ 1];           /* Mode Flag (usually set "0") */
    char    Inputformatflag [ 1];   /* '0' binary, '1' Hex Encoded Binary, '2' Text */
    char    macsize         [ 1];   		   /* use '1'					  */	
    char    macalgorithm    [ 1];      /* use '3'					  */	
    char    padmethod       [ 1];         /* use ?				  		  */	
    char    keytype         [ 3];           /* 008 ZAK					  */
    char    key             [ 200];              /* 					          */
    char    msglength       [ 4];         /* 					          */
    char    msg             [ 560];         	   /* 					          */
    char    mac             [  16];         	   /* 					          */
    char    lmkdeli         [1];        /* Value '%'. Optional; if present, the following field must also be present.            */
    char    lmkiden         [2];        /* LMK Identifier           */
} s_Cmdm8;

typedef struct
{
    char    msghead  [ 4];         /* Message Header           */
    char    respcode [ 2];         /* Response Code            */
    char    errcode  [ 2];         /* Error Code               */
} r_Cmdm9;


/*-------------------------------------------------------------------*/
/* CC -> CD: Translate a PIN from ZPK to ZPK Encryption              */
/*-------------------------------------------------------------------*/
typedef struct
{
    char    msghead      [ 4];       /* Message Header          */
    char    cmdcode      [ 2];       /* Command Code            */
    char    srczpk       [200];       /* Source TPK              */
    char    dstzpk       [200];       /* Destination ZPK         */
    char    maxpinlen    [ 2];       /* Maximum PIN Length      */
    char    srcpinblock  [32];       /* Source PIN block        */
    char    srcpinfmtcode[ 2];       /* Source PIN block format code      */
    char    dstpinfmtcode[ 2];       /* Destination PIN block format code */
    char    pan          [19];       /* Account Number          */
    char    aesdeli      [1];        /* Value ';'. Only present if an AES Key Block LMK is used.   */
    char    lmkdeli      [1];        /* Value '%'. Optional; if present, the following field must also be present.            */
    char    lmkiden      [2];        /* LMK Identifier           */
} s_cmdcc;

typedef struct
{
    char    msghead    [ 4];         /* Message Header           */
    char    respcode   [ 2];         /* Response Code            */
    char    errcode    [ 2];         /* Error Code               */
    char    pinlen     [ 2];         /* PIN Length               */
    char    dstpinblock[32];         /* Destination PIN block    */
    char    dstpinfmt  [ 2];         /* Destination PIN block format */
} r_cmdcd;


/*-------------------------------------------------------------------*/
/* A6 -> A7: Import  a key encrypted under a ZMK.                    */
/*-------------------------------------------------------------------*/
typedef struct
{
    char    msghead      [ 4];       /* Message Header          */
    char    cmdcode      [ 2];       /* Command Code            */
    char    keytype      [3];        /* Destination key type    */
    char    zmk          [200];      /* ZMK                     */
    char    dstblockkey  [200];     /* Destination Block key   */
    char    kschem       [1];        /* Key Scheme              */
    char    deli         [1];        /* Delimiter               */
    char    lmkiden      [2];        /* LMK Identifier          */
} s_cmda6;

typedef struct
{
    char    msghead    [ 4];         /* Message Header           */
    char    respcode   [ 2];         /* Response Code            */
    char    errcode    [ 2];         /* Error Code               */
    char    encKey     [200];        /* Key Under LMK            */
    char    kvc        [6];          /* KCV                      */
} r_cmda7;


/*-------------------------------------------------------------------*/
/* BA -> BB: Encrypt a clear text PIN Under LMK                      */
/*-------------------------------------------------------------------*/
typedef struct
{
    char    msghead      [ 4];       /* Message Header          */  
    char    cmdcode      [ 2];       /* Command Code            */
    char    pin          [13];       /* Plaintext Pin Number    */
    char    pan          [19];       /* PAN                     */
    char    aesdeli      [1];
    char    lmkdeli      [1];        /* Value '%'. Optional; if present, the following field must also be present.            */
    char    lmkiden      [2];        /* LMK Identifier           */
} s_cmdba;

typedef struct
{
    char    msghead    [ 4];         /* Message Header           */
    char    respcode   [ 2];         /* Response Code            */
    char    errcode    [ 2];         /* Error Code               */
    char    encpin     [33];        /*  The PIN encrypted under the LMK.           */
} r_cmdbb;

/*-------------------------------------------------------------------*/
/* JG -> JH: Translate a PIN from LMK to ZPK Encryption                      */
/*-------------------------------------------------------------------*/
typedef struct
{
    char    msghead      [ 4];       /* Message Header          */
    char    cmdcode      [ 2];       /* Command Code            */
    char    deszpk       [200];      /* Destination ZPK         */
    char    pinform      [2];        /* PIN Block Format Code   */
    char    pan          [19];       /* PAN                     */
    char    pindeli      [1];        /* Value ';'.              */
    char    encpin       [33];        /*  The PIN encrypted under the LMK.           */
    char    lmkdeli      [1];        /* Value '%'. Optional; if present, the following field must also be present.            */
    char    lmkiden      [2];        /* LMK Identifier           */
} s_cmdjg;

typedef struct
{
    char    msghead    [ 4];         /* Message Header           */
    char    respcode   [ 2];         /* Response Code            */
    char    errcode    [ 2];         /* Error Code               */
    char    pinblock   [32];        /*  The PIN encrypted under the ZPK  */
} r_cmdjh;

/*-------------------------------------------------------------------*/
/* JE -> JF: Translate a PIN from ZPK to LMK Encryption                    */
/*-------------------------------------------------------------------*/
typedef struct
{
    char    msghead      [ 4];       /* Message Header          */
    char    cmdcode      [ 2];       /* Command Code            */
    char    deszpk       [200];      /* Destination ZPK         */
    char    pinblock     [32];        /*  The PIN encrypted under the ZPK.           */
    char    pinform      [2];        /* PIN Block Format Code   */
    char    pan          [19];       /* PAN                     */
    char    pindeli      [1];        /* Value ';'.              */
    char    lmkdeli      [1];        /* Value '%'. Optional; if present, the following field must also be present.            */
    char    lmkiden      [2];        /* LMK Identifier           */
} s_cmdje;
  
typedef struct
{
    char    msghead    [ 4];         /* Message Header           */
    char    respcode   [ 2];         /* Response Code            */
    char    errcode    [ 2];         /* Error Code               */
    char    encpin     [33];        /*  The PIN encrypted under the LMK  */
} r_cmdjf;

/*-------------------------------------------------------------------*/
/* NG -> NH: Decrypt an Encrypted PIN                               */
/*-------------------------------------------------------------------*/
typedef struct
{
    char    msghead      [ 4];       /* Message Header          */  
    char    cmdcode      [ 2];       /* Command Code            */
    char    pan          [12];       /* PAN                     */
    char    encpin       [33];        /*  The PIN encrypted under the LMK  */
    char    lmkdeli      [1];        /* Value '%'. Optional; if present, the following field must also be present.            */
    char    lmkiden      [2];        /* LMK Identifier           */
} s_cmdng;

typedef struct
{
    char    msghead    [ 4];         /* Message Header           */
    char    respcode   [ 2];         /* Response Code            */
    char    errcode    [ 2];         /* Error Code               */
    char    pin        [13];         /* Plaintext Pin Number     */
    char    ref        [12];         /* The reference number derived by encrypting the PAN under the LMK.     */
} r_cmdnh;

// FILE *logfp;/
s_cmda6 s_cmdA6;
s_cmdba s_cmdBA;
s_cmdcc s_cmdCC;
s_cmdje s_cmdJE;
s_cmdjg s_cmdJG;
s_Cmdm6 s_cmdM6;
s_Cmdm8 s_cmdM8;
s_cmdng s_cmdNG;

r_cmda7 r_cmdA7;
r_cmdbb r_cmdBB;
r_cmdcd r_cmdCD;
r_cmdjf r_cmdJF;
r_cmdjh r_cmdJH;
r_Cmdm7 r_cmdM7;
r_Cmdm9 r_cmdM9;
r_cmdnh r_cmdNH;

#define UNIX_SOCKET_PATH_HSM "/home/atmadm/.config/"
#define HSM_SND_ID_SMTIFD ("smtifd")
#define HSM_SND_ID_ATMIFD ("hsmifd")

#define TRX_D_IMPORT_ZPKKEY (1)
#define TRX_D_IMPORT_MAKKEY (2)
#define TRX_D_GENMAC (3)
#define TRX_D_VERMAC (4)
#define TRX_D_TLTPIN_ZPKZPK (5)
#define TRX_D_ENCPIN_LMK (6)
#define TRX_D_TLTPIN_LMKZPK (7)
#define TRX_D_TLTPIN_ZPKLMK (8)
#define TRX_D_DECPIN_LMK (9)
#define TRX_D_DECPIN_SMRT (10)
#define TRX_D_ENCPIN_SMRT (11)

#define INPUT  (context->input)
#define OUTPUT (context->output)
#define LOGGER(level, format, ...) logMessage(level, __func__, __LINE__, format, __VA_ARGS__)


void logMessage(int logLvl, char *caller, int line, char *format, ...);
static int importZpkKey(hsmManagementContext *context); // Validation
static int importMakKey(hsmManagementContext *context); // Main Proc
static int generateMAC(hsmManagementContext *context); // Error return
static int verifyMAC(hsmManagementContext *context); // Error return
static int translatePinZpkToZpk(hsmManagementContext *context); // Error return
static int encPinUnderLmk(hsmManagementContext *context); // Error return
static int translatePinLmkToZpk(hsmManagementContext *context); // Error return
static int decryptPINByLmk(hsmManagementContext *context); // Error return
static int smartDecrypt(hsmManagementContext *context); // Error return
static int smartEncrypt(hsmManagementContext *context); // Error return



#endif /* _H_HSMMANAGEMENT_ */