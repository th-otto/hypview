#ifndef __BGH_H__
#define __BGH_H__

typedef enum {
	BGH_DIAL,
	BGH_ALERT,
	BGH_USER,
	BGH_MORE
} bgh_type;

typedef struct bgh_head BGH_head;

#ifndef __BGH_IMPLEMENTATION__
ANONYMOUS_STRUCT_DUMMY(bgh_head)
#endif


BGH_head *BGH_load(const char *Name);
void BGH_free(BGH_head *bgh_handle);
const char *BGH_gethelpstring(BGH_head *bgh_handle, bgh_type typ, short group, short idx);
void BGH_action(BGH_head *bgh_handle, short mx, short my, bgh_type typ, short group, short idx);
void BGH_help(short mx, short my, const char *helpstring);

#endif  /* __BGH_H__ */
