/*****************************************************************************
 * OBJECT.H
 *****************************************************************************/

#ifndef __OBJECT_H__
#define __OBJECT_H__

#ifndef __PORTAB_H__
#  include <portab.h>
#endif
#ifndef __PORTAES_H__
#  include <portaes.h>
#endif

EXTERN_C_BEG


typedef enum
{
	POPUP_NONE,
	POPUP_ORCS,
	POPUP_HONKA,
	POPUP_HONKA_BOX,
	POPUP_ORCS_SIMPLE,
	POPUP_ORCS_SINGLE,
	POPUP_ORCS_MULTIPLE,
	POPUP_ORCS_BOX
} popup_type;


void DelObjState(OBJECT *tree, _WORD obj, _UWORD state);
void SetObjState(OBJECT *tree, _WORD obj, _UWORD state);
_BOOL GetObjState(OBJECT *tree, _WORD obj, _UWORD state);
void DelObjFlags(OBJECT *tree, _WORD obj, _UWORD flags);
void SetObjFlags(OBJECT *tree, _WORD obj, _UWORD flags);
_BOOL GetObjFlags(OBJECT *tree, _WORD obj, _UWORD flags);
void EnableObjState(OBJECT *tree, _WORD obj, _UWORD state, _BOOL setit);
void EnableObjFlags(OBJECT *tree, _WORD obj, _UWORD flags, _BOOL setit);

#ifndef NO_MACROS

#define DelObjState(tree, obj, state) (tree[obj].ob_state &= ~(state))
#define SetObjState(tree, obj, state) (tree[obj].ob_state |= (state))
#define DelObjFlags(tree, obj, flags) (tree[obj].ob_flags &= ~(flags))
#define SetObjFlags(tree, obj, flags) (tree[obj].ob_flags |= (flags))

#define GetObjState(tree, obj, state) ((tree[obj].ob_state & (state)) ? TRUE : FALSE)
#define EnableObjState(tree, obj, state, cond) \
	((cond) ? SetObjState(tree, obj, state) : DelObjState(tree, obj, state))
#define GetObjFlags(tree, obj, flags) ((tree[obj].ob_flags & (flags)) ? TRUE : FALSE)
#define EnableObjFlags(tree, obj, flags, cond) \
	((cond) ? SetObjFlags(tree, obj, flags) : DelObjFlags(tree, obj, flags))

#endif /* NO_MACROS */



popup_type is_popup(OBJECT * head);
OBJECT *get_popup_tree(OBJECT * tree, _WORD *head);


void Objc_Rect(OBJECT *tree, _WORD obj, GRECT *gr);
void Objc_Offset(const OBJECT *tree, _WORD idx, _WORD *xp, _WORD *yp);
void Objc_Xywh(const OBJECT *tree, _WORD obj, GRECT *gr);
_WORD Objc_Get_Parent(const OBJECT *tree, _WORD obj);
_WORD Objc_Get_Parent_Root(const OBJECT *tree, _WORD obj);

_WORD Objc_Count(OBJECT *tree, _WORD start);
_WORD Objc_Delete(OBJECT *tree, _WORD idx);
void Objc_Unlink(OBJECT *tree, _WORD idx);
_WORD Objc_Add(OBJECT *tree, _WORD parent, _WORD idx);

_UBYTE *Obj_StrPtr(OBJECT *tree, _WORD obj);
_UBYTE *Obj_TemplatePtr(OBJECT *tree, _WORD obj);

void form_calc(OBSPEC obspec, _WORD type, _UWORD flags, _UWORD state, GRECT *gr);
void Form_Center(OBJECT *tree, GRECT *gr);
void form_size(OBJECT *tree, _WORD idx, GRECT *gr);

void ClipToDesktop(_WORD *x, _WORD *y, _WORD w, _WORD h);
void ClipObjToDesktop(OBJECT *tree, _WORD start);

EXTERN_C_END

#endif /* __OBJECT_H__ */
