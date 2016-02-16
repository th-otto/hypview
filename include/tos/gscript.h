/*****************************************************************************
 * TOS/GSCRIPT.H
 *****************************************************************************/

#ifndef __GSCRIPT_H__
#define __GSCRIPT_H__

typedef _BOOL (*SCRIPT_COMMAND)(_WORD argc, const char *const *argv, char **erg);
typedef void (*GS_RESULT_FUNC)(const char *res, _WORD error);

typedef _WORD GS_TARGET;

/* Messages */	
#define GS_REQUEST		0x1350
#define GS_REPLY		0x1351
#define GS_COMMAND		0x1352
#define GS_ACK			0x1353
#define GS_QUIT			0x1354
#define GS_OPENMACRO	0x1355
#define GS_MACRO		0x1356
#define GS_WRITE		0x1357
#define GS_CLOSEMACRO	0x1358


#define GSM_COMMAND   0x0001	/* Understands GS_COMMAND. */
#define GSM_MACRO     0x0002	/* Understands GS_OPENMACRO, GS_WRITE and GS_CLOSEMACRO.
                                   Can send GS_MACRO (Interpreter) */
#define GSM_WRITE     0x0004	/* Can send GS_OPENMACRO, GS_WRITE and GS_CLOSEMACRO.
                                   Understands GS_MACRO (recording application) */
#define GSM_HEXCODING 0x0008    /* Understands Hex-Coding */

#define GSACK_OK	  0
#define GSACK_UNKNOWN 1
#define GSACK_ERROR   2

void gemscript_record(_WORD argc, const char *const *argv);
void gemscript_start_recording(void);
void gemscript_stop_recording(void);
_BOOL gemscript_register_command(const char *command, SCRIPT_COMMAND func);
_BOOL gemscript_init(void);
void gemscript_start(GS_TARGET apid);
void gemscript_stop(GS_TARGET apid);
_BOOL gemscript_info(GS_TARGET apid, _UWORD *version, _UWORD *msgs);
void gemscript_send_command(GS_TARGET apid, _WORD argc, const char *const *argv, GS_RESULT_FUNC func);
void gemscript_exit(void);
_BOOL gemscript_handle_message(const _WORD *message);

#endif /* __GSCRIPT_H__ */
