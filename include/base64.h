#ifndef __BASE64_H__
#define __BASE64_H__

#ifndef __BASE64_IMPLEMENTATION__
ANONYMOUS_STRUCT_DUMMY(_base64)
#endif
typedef struct _base64 Base64;

Base64 *Base64_New(void);
void Base64_Delete(Base64 *b);

gboolean Base64_Encode(Base64 *b, const void *p, size_t size);
gboolean Base64_Decode(Base64 *b, const char *p, size_t size);

void *Base64_DecodedMessage(Base64 *b);
char *Base64_EncodedMessage(Base64 *b);

size_t Base64_DecodedMessageSize(Base64 *b);
size_t Base64_EncodedMessageSize(Base64 *b);

gboolean Base64_AllocEncode(Base64 *b, size_t size);
gboolean Base64_AllocDecode(Base64 *b, size_t size);

#endif /* __BASE64_H__ */

