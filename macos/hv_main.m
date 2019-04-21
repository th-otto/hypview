#include "hv_defs.h"
#include "hv_vers.h"


#include <Cocoa/Cocoa.h>
#include <objc/runtime.h>
#include <errno.h>

#define autorelease self

static char const program_title[] = "HypView";
	
static NSAutoreleasePool *pool;
static NSString *m_executablePath;

/* handle of a window */
#define HWND HypViewWindow *
#define NO_WINDOW ((HWND)0)

/*
 * original definition (MSUIntegerMax) gives warning with -pedantic
 */
#undef NSAnyEventMask
#define NSAnyEventMask (NSUInteger)~0L

typedef unsigned int Pixel;


@interface HypViewView : NSView
{
@private
	CGDirectDisplayID w_display;
	CGColorSpaceRef colorspace;
}
@end

@interface HypViewWindow : NSWindow <NSWindowDelegate>
{
	HypViewView *view;
}
@end

@interface NoNibBundle : NSBundle
{
	int dummy;
}
@end

@interface NSBundle (NoNibAdditions)

+(BOOL) NoNib_loadNibNamed:(NSString *)aNibNamed owner:(id)owner;
- (id)NoNib_objectForInfoDictionaryKey:(NSString *)key;
- (NSDictionary *)NoNib_infoDictionary;
- (NSDictionary *)NoNib_localizedInfoDictionary;
- (NSString *)NoNib_bundleIdentifier;

@end

@interface HypViewApplication : NSApplication <NSApplicationDelegate>
{
@private
	NSDate *				_distantFuture;
	NSDate *				_distantPast;
	BOOL _isPackaged;
	NSImage *icon;
	NSString *m_applicationName;
}

- (void)runOnce: (BOOL) block;
- (void)runModal: (int *) flag;
- (void)setPackaged:(BOOL)packaged;
- (NSString *) applicationName;
- (NSString *) executablePath;

@end

@interface HypViewApplication (NSMenuValidation)
- (BOOL)validateMenuItem:(NSMenuItem *)item;
@end


@interface NSAutoreleasePool (Debug)
- (void)dump;
@end

@interface HypViewMenuPopulator : NSObject
{
	int dummy;
}

+(void) populateMainMenu;

+(void) populateApplicationMenu:(NSMenu *)menu;
+(void) populateEditMenu:(NSMenu *)menu;
+(void) populateFileMenu:(NSMenu *)menu;
+(void) populateHelpMenu:(NSMenu *)menu;
+(void) populateWindowMenu:(NSMenu *)menu;

@end




int main(int argc, char **argv)
{
	char *real;
	const char *argv0;

	/* Much of Cocoa needs one of these to be available. */
	pool = [[NSAutoreleasePool alloc] init];
	
	argv0 = [[[[NSProcessInfo processInfo] arguments] objectAtIndex:0] fileSystemRepresentation];
	real = realpath(argv0, NULL);
	m_executablePath = [[NSString stringWithCString:real encoding:NSUTF8StringEncoding] stringByStandardizingPath];
	free(real);

	return 0;	
}
