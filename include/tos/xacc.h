/*****************************************************************************
 *		TOS/XACC.H
 *****************************************************************************/

#ifndef __XACC_H__
#define __XACC_H__

#define XACC_PROT_HELP 0


/* Message number for xAcc-protocoll by Konrad Hinsen.
 * Venus returns ACC_ID "VENUS.APP" and Gemini returns "GEMINI.APP".
 * Gemini supports xAcc Level 0.
 */
/* basic messages */
#define ACC_ID		  0x0400
#define ACC_OPEN	  0x0401
#define ACC_CLOSE	  0x0402
#define ACC_ACC		  0x0403
#define ACC_EXIT	  0x0404

/* message group 1 */
#define ACC_ACK		  0x0500
#define ACC_TEXT	  0x0501
#define ACC_KEY		  0x0502

/* message group 2 */
#define ACC_META	  0x0503
#define ACC_IMG		  0x0504

/* remote mailmerge protocol (Extended-Feature "RM") */
#define ACC_REQUEST   0x0480
#define ACC_REPLY     0x0481

/* inquiry protocol (Extended-Feature "DI") */
#define ACC_GETDSI    0x0510
#define ACC_DSINFO    0x0511
#define ACC_FILEINFO  0x0512
#define ACC_GETFIELDS 0x0513
#define ACC_FIELDINFO 0x0514

/* request/reply protocol (Extended-Feature "RQ") */
#define ACC_FORCESDF  0x0520
#define ACC_GETSDF    0x0521

_BOOL xacc_proto_init(const char *appName, size_t len);
_WORD xacc_send_text(char *text, _WORD mode);
void xacc_send_ack(_WORD apid, _WORD ack);

#endif /* __XACC_H__ */
