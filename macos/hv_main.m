#include "hv_defs.h"
#include "hv_vers.h"
#include "xgetopt.h"


#include <objc/runtime.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <errno.h>

#if 1
#undef dprintf
#define dprintf(x) hyp_debug  x
#endif

#define autorelease self

char const gl_program_name[] = "HypView";
char const gl_program_version[] = HYPVIEW_VERSION;
char const gl_compile_date[12] = __DATE__;

static gboolean bShowVersion;
static gboolean bShowHelp;
static gboolean bNewWindow;
static const char *geom_arg;
	
static NSAutoreleasePool *pool;
static NSString *m_executablePath;
NSMenu *m_bookmarks_menu;
NSMenu *m_recent_menu;

struct _viewer_colors viewer_colors;

/*
 * original definition (MSUIntegerMax) gives warning with -pedantic
 */
#undef NSAnyEventMask
#define NSAnyEventMask (NSUInteger)~0L



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
+(void) populateFileMenu:(NSMenu *)menu;
+(void) populateEditMenu:(NSMenu *)menu;
+(void) populateNavigationMenu:(NSMenu *)menu;
+(void) populateWindowMenu:(NSMenu *)menu;
+(void) populateOptionsMenu:(NSMenu *)menu;
+(void) populateHelpMenu:(NSMenu *)menu;

@end


/********************************************************************/
/*------------------------------------------------------------------*/
/********************************************************************/

static void default_plist_infodictionary(NSMutableDictionary *mutable)
{
	id val;

#define set_default(k, v) \
	val = [mutable objectForKey:k]; \
	if (val == nil) \
		[mutable setObject:v forKey:k]

	set_default(@"CFBundleDevelopmentRegion", @"English");
	set_default(@"CFBundleExecutable", @"hypview");
	set_default(@"CFBundleName", [[[NSString alloc] initWithCString: gl_program_name encoding:NSUTF8StringEncoding] autorelease]);
	set_default(@"CFBundleIdentifier", @"com.th-otto.hypview");
	set_default(@"CFBundleInfoDictionaryVersion", @"6.0");
	set_default(@"CFBundlePackageType", @"APPL"),
	set_default(@"CFBundleSignature", @"????"),
	set_default(@"CFBundleVersion", [[[NSString alloc] initWithCString: gl_program_version encoding:NSUTF8StringEncoding] autorelease]);
	set_default(@"CFBundleShortVersionString", [[[NSString alloc] initWithCString: gl_program_version encoding:NSUTF8StringEncoding] autorelease]);
	set_default(@"NSPrincipalClass", @"HypViewApplication");
	set_default(@"NSHumanReadableCopyright", @"Copyright \xC2\xA9 2019 Th.Otto");
	
#undef set_default
}


#if 0
static void dump_infoplist(const char *filename)
{
	NSMutableDictionary *mutable = [[NSMutableDictionary alloc] init];
	default_plist_infodictionary(mutable);
	if ([mutable writeToFile:[[NSString alloc] initWithCString:filename encoding: NSUTF8StringEncoding] atomically:YES] == NO)
		exit(1);
	exit(0);
}
#endif


static NSDictionary *fake_infoDictionary(NSDictionary *res, BOOL localized)
{
	UNUSED(localized);
	if (res == nil || [res objectForKey:@"CFBundleExecutable"] == nil)
	{
		NSMutableDictionary *mutable;
		NSEnumerator *enumerator;
		id key;
		id val;
		NSDictionary *faked;
		NSString *appPath;
		NSString *bundlePath;
		NSURL *bundleUrl;
		
		[NSApp setPackaged:FALSE];
		mutable = [[NSMutableDictionary alloc] init];
		if (res != nil)
		{
			enumerator = [res keyEnumerator];
			while ((key = [enumerator nextObject]))
			{
				val = [res objectForKey: key];
				[mutable setObject:val forKey:key];
			}
		}
		
		val = [mutable objectForKey:@"CFBundleExecutablePath"];
		if (val == nil)
		{
			val = m_executablePath;
			[mutable setObject:val forKey:@"CFBundleExecutablePath"];
		}
		
		[mutable setObject:[val lastPathComponent] forKey: @"CFBundleExecutable"];
		appPath = [val stringByDeletingLastPathComponent];
		if ([[appPath lastPathComponent] isEqual:@"MacOS"])
		{
			appPath = [[[appPath stringByDeletingLastPathComponent] stringByDeletingLastPathComponent] stringByDeletingLastPathComponent];
		}
		
		default_plist_infodictionary(mutable);
		
		bundlePath = [appPath stringByAppendingPathComponent:@"hypview.app"];
		[mutable setObject:bundlePath forKey:@"NSBundleInitialPath"];
		[mutable setObject:bundlePath forKey:@"NSBundleResolvedPath"];
		
		bundleUrl = [NSURL fileURLWithPath: bundlePath isDirectory: YES];
		val = [@"Contents/Info.plist -- " stringByAppendingString: [bundleUrl absoluteString]];
		[mutable setObject:val forKey:@"CFBundleInfoPlistURL"];
		
		faked = [NSDictionary dictionaryWithDictionary: mutable];
		[mutable release];
		res = [faked retain];
	} else
	{
		[NSApp setPackaged:TRUE];
	}
	
	return res;
}


/*
 * implementation for old objc runtime
 */
@implementation NoNibBundle

+(BOOL) loadNibNamed:(NSString *)aNibNamed owner:(id)owner
{
	if (aNibNamed == nil && owner == NSApp)
	{
		/* We're lying here. Don't load anything. */
		return YES;
	} else
	{
		return [super loadNibNamed:aNibNamed owner:owner];
	}
}


- (id)objectForInfoDictionaryKey:(NSString *)key
{
	id res;
	
	res = [super objectForInfoDictionaryKey:key];
	return res;
}


- (NSDictionary *)infoDictionary
{
	NSDictionary *res;
	
	res = [super infoDictionary];
	if (self == [NSBundle mainBundle])
		res = fake_infoDictionary(res, FALSE);
	return res;
}


- (NSDictionary *)localizedInfoDictionary
{
	NSDictionary *res;
	
	res = [super localizedInfoDictionary];
/*
	if (self == [NSBundle mainBundle])
		res = fake_infoDictionary(res, TRUE);
*/
	return res;
}


- (NSString *)bundleIdentifier
{
	NSString *str;
	str = [super bundleIdentifier];
	return str;
}

@end


/*
 * implementation for new objc runtime
 */
@implementation NSBundle (NoNibAdditions)

+(BOOL) NoNib_loadNibNamed:(NSString *)aNibNamed owner:(id)owner
{
	if (aNibNamed == nil && owner == NSApp)
	{
		/* We're lying here. Don't load anything. */
		return YES;
	} else
	{
		/* The current implementation of +[NSBundle NoNib_loadNibNamed:owner:] is the original implementation of +[NSBundle loadNibNamed:owner:] */
		return [self NoNib_loadNibNamed:aNibNamed owner:owner];
	}
}


- (id)NoNib_objectForInfoDictionaryKey:(NSString *)key
{
	id res;
	
	/* The current implementation of +[NSBundle NoNib_objectForInfoDictionaryKey:] is the original implementation of +[NSBundle objectForInfoDictionaryKey:] */
	res = [self NoNib_objectForInfoDictionaryKey:key];
	return res;
}


- (NSDictionary *)NoNib_infoDictionary
{
	NSDictionary *res;
	
	/* The current implementation of +[NSBundle NoNib_infoDictionary:] is the original implementation of +[NSBundle infoDictionary] */
	res = [self NoNib_infoDictionary];
	if (self == [NSBundle mainBundle])
		res = fake_infoDictionary(res, FALSE);
	return res;
}


- (NSDictionary *)NoNib_localizedInfoDictionary
{
	NSDictionary *res;
	
	/* The current implementation of +[NSBundle NoNib_localizedInfoDictionary:] is the original implementation of +[NSBundle localizedInfoDictionary] */
	res = [self NoNib_localizedInfoDictionary];
/*
	if (self == [NSBundle mainBundle])
		res = fake_infoDictionary(res, TRUE);
*/
	return res;
}


- (NSString *)NoNib_bundleIdentifier
{
	NSString *str;
	str = [self NoNib_bundleIdentifier];
	return str;
}

@end


#ifndef _OBJC_RUNTIME_H
typedef struct objc_method *Method;
#endif


static void (*stfu_isoc(void *ptr))(void)
{
	union {
		void *func;
		void (*f)(void);
	} f;
	f.func = ptr;
	return f.f;
}


static void nonib_init(void)
{
	Method (*p_class_getClassMethod)(Class cls, SEL name);
	Method (*p_class_getInstanceMethod)(Class cls, SEL name);
	void (*p_method_exchangeImplementations)(Method m1, Method m2);
	
	p_class_getClassMethod = (Method (*)(Class, SEL))stfu_isoc(dlsym(RTLD_DEFAULT, "class_getClassMethod"));
	p_class_getInstanceMethod = (Method (*)(Class, SEL))stfu_isoc(dlsym(RTLD_DEFAULT, "class_getInstanceMethod"));
	p_method_exchangeImplementations = (void (*)(Method, Method))stfu_isoc(dlsym(RTLD_DEFAULT, "method_exchangeImplementations"));
	if (p_class_getClassMethod && p_class_getInstanceMethod && p_method_exchangeImplementations)
	{
		Class klass;
		Method originalMethod, categoryMethod;
		
#define swap_class_method(m) \
		originalMethod = p_class_getClassMethod(klass, @selector(m)); \
		categoryMethod = p_class_getClassMethod(klass, @selector(NoNib_##m)); \
		assert(categoryMethod); \
		if (originalMethod && categoryMethod) p_method_exchangeImplementations(originalMethod, categoryMethod)
#define swap_instance_method(m) \
		originalMethod = p_class_getInstanceMethod(klass, @selector(m)); \
		categoryMethod = p_class_getInstanceMethod(klass, @selector(NoNib_##m)); \
		assert(categoryMethod); \
		if (originalMethod && categoryMethod) p_method_exchangeImplementations(originalMethod, categoryMethod)

		klass = [NSBundle class];
		swap_class_method(loadNibNamed:owner:);

		swap_instance_method(objectForInfoDictionaryKey:);
		swap_instance_method(infoDictionary);
		swap_instance_method(localizedInfoDictionary);
		swap_instance_method(bundleIdentifier);
		
#undef swap_class_method
#undef swap_instance_method
	} else
	{
		[[NoNibBundle class] poseAsClass:[NSBundle class]];
	}
}

/********************************************************************/
/*------------------------------------------------------------------*/
/********************************************************************/

/* XPM */
static const char *const hypview_icon[] = {
/* width height ncolors chars_per_pixel
"32 32 2 1",
"x c yellow",
"  c None",
   pixels */
"                                ",
"                                ",
"                                ",
"                                ",
"                                ",
"                                ",
"                                ",
"                                ",
"          xxxxxxxxxxx           ",
"         xxxxxxxxxxxxx          ",
"         xxxxxxxxxxxxx          ",
"         xxx       xxx          ",
"         xxx       xxx          ",
"                   xxx          ",
"              xxxxxxxx          ",
"              xxxxxxxx          ",
"              xxxxxxx           ",
"              xxx               ",
"              xxx               ",
"              xxx               ",
"                                ",
"              xxx               ",
"              xxx               ",
"              xxx               ",
"                                ",
"                                ",
"                                ",
"                                ",
"                                ",
"                                ",
"                                ",
"                                "
};

static NSImage *make_icon(void)
{
	int x, y;
	Pixel colors[2];
	CGImageRef icon;
	const char *s;
	Pixel icon_data[32][32];
	CGDataProviderRef prov;
	NSRect imageRect;
	CGRect cgRect;
	CGContextRef imageContext;
	NSImage *newImage;
	CGColorSpaceRef colorspace;
	
	colors[0] = 0;
	colors[1] = 0xffffff00;
	for (y = 0; y < 32; y++)
	{
		s = hypview_icon[y];
		for (x = 0; x < 32; x++)
		{
			if (s[x] != ' ')
			{
				icon_data[y][x] = colors[1];
			} else
			{
				icon_data[y][x] = 0;
			}
		}
	}
	prov = CGDataProviderCreateWithData(NULL, icon_data, sizeof(icon_data), NULL);
	colorspace = CGColorSpaceCreateDeviceRGB();
	icon = CGImageCreate(32, 32, /* width, height */
						 8,      /* bitsPerCompenent */
						 32,     /* bitsPerPixel */
						 4 * 32, /* bytesPerRow */
						 colorspace,
						 /* Host-ordered, since we're using the
						    address of an int as the color data.
						  */
						 (kCGImageAlphaFirst | 
						 kCGBitmapByteOrder32Host),
						 prov, 
						 NULL,  /* decode[] */
						 NO, /* interpolate */
						 kCGRenderingIntentDefault);
	CGDataProviderRelease(prov);
	CFRelease(colorspace);
	
	/*  Create a new image to receive the Quartz image data. */
	imageRect.origin.x = 0;
	imageRect.origin.y = 0;
	imageRect.size.height = CGImageGetHeight(icon);
	imageRect.size.width = CGImageGetWidth(icon);
	
	newImage = [[[NSImage alloc] initWithSize:imageRect.size] retain];
	[newImage lockFocus];

	/* Get the Quartz context and draw. */
	imageContext = [[NSGraphicsContext currentContext] graphicsPort];
	cgRect.origin.x = imageRect.origin.x;
	cgRect.origin.y = imageRect.origin.y;
	cgRect.size.width = imageRect.size.width;
	cgRect.size.height = imageRect.size.height;
	CGContextDrawImage(imageContext, cgRect, icon);
	[newImage unlockFocus];
	CGImageRelease(icon);
	[newImage setName:@"NSApplicationIcon"];
	return newImage;
}

/********************************************************************/
/*------------------------------------------------------------------*/
/********************************************************************/

@implementation HypViewApplication

-(id) init
{
	dprintf(("NSApplication::init"));
	if ((self = [super init]) == nil)
		return nil;
	_running = FALSE;
	[self retain];
	[self setDelegate:self];
	
	_distantFuture = [[NSDate distantFuture] retain];
	_distantPast = [[NSDate distantPast] retain];
	self->icon = make_icon();
	_isPackaged = FALSE;

	return self;
}


-(void) dealloc
{
	id delegate;

	dprintf(("NSApplication::dealloc"));
	delegate = [self delegate];
	if (delegate)
	{
		[self setDelegate:nil];
	}
	if (m_executablePath)
	{
		[m_executablePath release];
		m_executablePath = nil;
	}
	[[NSNotificationCenter defaultCenter] removeObserver:self];
	if (self->icon)
	{
		[self->icon release];
		self->icon = NULL;
	}
	if (m_applicationName)
	{
		[m_applicationName release];
		m_applicationName = nil;
	}
	[super dealloc];
}


- (void)finalize
{
	[[NSNotificationCenter defaultCenter] removeObserver:self];
	[super finalize];
}


- (void)setPackaged:(BOOL)packaged
{
	/* dprintf(("setPackaged(%p): %d -> %d", self, self->_isPackaged, packaged)); */
	self->_isPackaged = packaged;
}


- (NSString *)executablePath
{
	if (m_executablePath)
	{
		return [[m_executablePath retain] autorelease];
	}
	return nil;
}


-(NSString *) applicationName
{
	if (!m_applicationName)
	{
		m_applicationName = [[[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleName"] retain];
		if (!m_applicationName)
		{
			dprintf(("![[NSBundle mainBundle] objectForInfoDictionaryKey:@\"CFBundleName\"]"));
			m_applicationName = [[[NSString alloc] initWithCString: gl_program_name encoding:NSUTF8StringEncoding] retain];
		}
	}
	return [[m_applicationName retain] autorelease];
}


- (void)updateAppMenu
{
	[useAltFontMenuItem setState: gl_profile.viewer.use_xfont ? NSOnState : NSOffState];
	[expandSpacesMenuItem setState: gl_profile.viewer.expand_spaces ? NSOnState : NSOffState];
}


- (void)menuWillOpen:(NSMenu *)menu
{
	dprintf(("menuWillOpen %s", [[menu title] UTF8String]));
	if (menu == m_bookmarks_menu)
		MarkerUpdate(top_window());
	else if (menu == m_recent_menu)
		RecentUpdate(top_window());
}


- (BOOL)isRunning
{
	if (_running)
		return YES;
	return [super isRunning];
}


- (void)finishLaunching
{
	dprintf(("NSApplication::finishLaunching"));
	if (![self isRunning])
	{
		_running = TRUE;
	}
	[super finishLaunching];
}


- (void)stop:(id)sender
{
	_running = FALSE;
	[super stop:sender];
}


- (void)about:(id)sender
{
	dprintf(("NSApplication::about"));
	About(sender);
}


- (void)openPreferences:(id)sender
{
	WINDOW_DATA *win = top_window();
	UNUSED(sender);
	dprintf(("NSApplication::preferences"));
	hv_preferences(win);
}


- (void)selectFont:(id)sender
{
	WINDOW_DATA *win = top_window();
	UNUSED(sender);
	SelectFont(win);
}


- (void)selectColor:(id)sender
{
	WINDOW_DATA *win = top_window();
	UNUSED(sender);
	hv_config_colors(win);
}


- (void)configOutput:(id)sender
{
	WINDOW_DATA *win = top_window();
	UNUSED(sender);
	hv_config_output(win);
}


- (void)toggleAltfont:(id)sender
{
	WINDOW_DATA *win = top_window();
	UNUSED(sender);
	gl_profile.viewer.use_xfont = !gl_profile.viewer.use_xfont;
	SwitchFont(win, FALSE);
	[self updateAppMenu];
}


- (void)toggleExpandSpaces:(id)sender
{
	WINDOW_DATA *win = top_window();
	UNUSED(sender);
	gl_profile.viewer.expand_spaces = !gl_profile.viewer.expand_spaces;
	if (win)
	{
		DOCUMENT *doc = win->data;
		if (doc && doc->prepNode)
		{
			doc->start_line = hv_win_topline(win);
			ReInitWindow(win, TRUE);
		}
	}
	[self updateAppMenu];
}


- (void)toggleScaleBitmaps:(id)sender
{
	UNUSED(sender);
	gl_profile.viewer.scale_bitmaps = !gl_profile.viewer.scale_bitmaps;
	[self updateAppMenu];
}


- (void)newWindow:(id)sender
{
}


- (void)openDocument:(id)sender
{
	WINDOW_DATA *win = top_window();
	UNUSED(sender);
	dprintf(("NSApplication::openDocument:"));
	SelectFileLoad(win);
}


- (void)clearRecentDocuments:(id)sender
{
	UNUSED(sender);
	dprintf(("NSApplication::clearRecentDocuments:"));
	RecentExit();
}


- (void)performClose:(id)sender
{
	WINDOW_DATA *win = top_window();
	UNUSED(sender);
	dprintf(("NSApplication::performClose:"));
	if (win)
		[win performClose:win];
}


- (void)saveDocument:(id)sender
{
	UNUSED(sender);
	dprintf(("NSApplication::saveDocument:"));
}


- (void)saveDocumentAs:(id)sender
{
	WINDOW_DATA *win = top_window();
	char *filename;
	gboolean selection_only = FALSE;
	DOCUMENT *doc = win->data;
				
	dprintf(("NSApplication::saveDocumentAs:"));
	UNUSED(sender);
	if (win->selection.valid)
		selection_only = TRUE;
	filename = SelectFileSave(win, HYP_FT_ASCII);
	if (filename)
	{
		if (doc->type == HYP_FT_HYP && !selection_only)
			hv_recompile((HYP_DOCUMENT *)doc->data, filename, HYP_FT_ASCII);
		else
			BlockAsciiSave(win, filename);
		g_free(filename);
	}
}


- (void)recompile:(id)sender
{
	WINDOW_DATA *win = top_window();
	char *filename;
	DOCUMENT *doc = win->data;
	
	dprintf(("NSApplication::recompile:"));
	if (doc->type == HYP_FT_HYP)
	{
		filename = SelectFileSave(win, HYP_FT_STG);
		if (filename)
		{
			hv_recompile((HYP_DOCUMENT *)doc->data, filename, HYP_FT_STG);
			g_free(filename);
		}
	}
}


- (void)print:(id)sender
{
	UNUSED(sender);
	dprintf(("NSApplication::print:"));
	fprintf(stderr, "NYI: print\n");
}


- (void)cut:(id)sender
{
	WINDOW_DATA *win = top_window();
	UNUSED(sender);
	dprintf(("NSApplication::cut:"));
	BlockOperation(win, CO_CUT);
}


- (void)copy:(id)sender
{
	WINDOW_DATA *win = top_window();
	UNUSED(sender);
	dprintf(("NSApplication::copy:"));
	BlockOperation(win, CO_COPY);
}


- (void)paste:(id)sender
{
	WINDOW_DATA *win = top_window();
	UNUSED(sender);
	dprintf(("NSApplication::paste:"));
	BlockOperation(win, CO_PASTE);
}


- (void)delete:(id)sender
{
	dprintf(("NSApplication::delete:"));
	UNUSED(sender);
}


- (void)selectAll:(id)sender
{
	WINDOW_DATA *win = top_window();
	UNUSED(sender);
	dprintf(("NSApplication::selectAll:"));
	BlockOperation(win, CO_SELECT_ALL);
}


- (void)performFindPanelAction:(id)sender
{
	WINDOW_DATA *win = top_window();
	dprintf(("NSApplication::performFindPanelAction:"));
	if ([sender tag] == NSFindPanelActionShowFindPanel)
		BlockOperation(win, CO_SEARCH);
	else if ([sender tag] == NSFindPanelActionNext)
		BlockOperation(win, CO_SEARCH_AGAIN);
	else if ([sender tag] == NSFindPanelActionPrevious)
		BlockOperation(win, CO_SEARCH_AGAIN);
}


- (void)showHelp:(id)sender
{
	WINDOW_DATA *win = top_window();
	dprintf(("NSApplication::showHelp"));
	if (_isPackaged)
		[super showHelp:sender];
	else
		Help_Contents(win);
}


- (void)navigateCatalog:(id)sender
{
	WINDOW_DATA *win = top_window();
	GoThisButton(win, TO_CATALOG);
}


- (void)navigateDefaultFile:(id)sender
{
	WINDOW_DATA *win = top_window();
	GotoDefaultFile(win);
}


- (void)startRemarker:(id)sender
{
	WINDOW_DATA *win = top_window();
	dprintf(("NSApplication::startRemarker"));
	BlockOperation(win, CO_REMARKER);
}


- (void)documentInfo:(id)sender
{
	WINDOW_DATA *win = top_window();
	DocumentInfos(win);
}


- (void)navigateBack:(id)sender
{
	WINDOW_DATA *win = top_window();
	GoThisButton(win, TO_BACK);
}


- (void)navigateClearstack:(id)sender
{
	WINDOW_DATA *win = top_window();
	BlockOperation(win, CO_DELETE_STACK);
}


- (void)openBookmarks:(id)sender
{
	WINDOW_DATA *win = top_window();
	dprintf(("openBookmarks"));
	MarkerUpdate(win);
	MarkerPopup(win, 1);
}


- (void)openHistory:(id)sender
{
	WINDOW_DATA *win = top_window();
	dprintf(("openHistory"));
	HistoryPopup(win, TO_HISTORY, 1);
}


- (void)openReferences:(id)sender
{
	WINDOW_DATA *win = top_window();
	HypExtRefPopup(win, 1);
}


- (void)navigatePrev:(id)sender
{
	WINDOW_DATA *win = top_window();
	GoThisButton(win, TO_PREV);
}


- (void)navigateNext:(id)sender
{
	WINDOW_DATA *win = top_window();
	GoThisButton(win, TO_NEXT);
}


- (void)navigatePrevPhys:(id)sender
{
	WINDOW_DATA *win = top_window();
	GoThisButton(win, TO_PREV_PHYS);
}


- (void)navigateNextPhys:(id)sender
{
	WINDOW_DATA *win = top_window();
	GoThisButton(win, TO_NEXT_PHYS);
}


- (void)navigateFirst:(id)sender
{
	WINDOW_DATA *win = top_window();
	GoThisButton(win, TO_FIRST);
}


- (void)navigateLast:(id)sender
{
	WINDOW_DATA *win = top_window();
	GoThisButton(win, TO_LAST);
}


- (void)navigateToc:(id)sender
{
	WINDOW_DATA *win = top_window();
	GoThisButton(win, TO_HOME);
}


- (void)navigateIndex:(id)sender
{
	WINDOW_DATA *win = top_window();
	GotoIndex(win);
}


- (void)navigateHelp:(id)sender
{
	WINDOW_DATA *win = top_window();
	GotoHelp(win);
}


-(void) applicationWillFinishLaunching:(NSNotification *)notification
{
	UNUSED(notification);

	dprintf(("NSApplicationDelegate::applicationWillFinishLaunching: %s", [[[notification object] description] UTF8String]));
	[NSApp setApplicationIconImage: self->icon];
#if defined(MAC_OS_X_VERSION_10_12) && (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_12)
	if (NSAppKitVersionNumber > 1500)
	{
		[NSWindow setAllowsAutomaticWindowTabbing: NO];
	}
#endif
}


-(void) applicationDidFinishLaunching:(NSNotification *)notification
{
	NSUInteger i, count;
	NSArray *windows;
	WINDOW_DATA *win;
	id app;
	
	dprintf(("NSApplicationDelegate::applicationDidFinishLaunching: %s", [[[notification object] description] UTF8String]));

	[NSApp activateIgnoringOtherApps:YES];

	[HypViewMenuPopulator populateMainMenu];

	/*
	 * update windows menu after event loop has started;
	 * the first call to setTitle did not update it
	 */
	app = [notification object];
	windows = [app windows];
	count = [windows count];
	for (i = 0; i < count; i++)
	{
		win = [windows objectAtIndex:i];
		[app changeWindowsItem: win title: [win title] filename:NO];
	}
	
	/*
	 * remove the "Start Dictation" and "Special Characters" entries
	 * from the Edit menu; we don't need them
	 */
	{
		NSMenu *menu = [[[NSApp mainMenu] itemWithTitle:@"Edit"] submenu];
		SEL sel;
		NSInteger last;
		
		last = [menu numberOfItems] - 1;
		sel = [[menu itemAtIndex: last] action];
		if (sel == NSSelectorFromString(@"orderFrontCharacterPalette:"))
			[menu removeItemAtIndex: last];
		last = [menu numberOfItems] - 1;
		sel = [[menu itemAtIndex: last] action];
		if (sel == NSSelectorFromString(@"startDictation:"))
			[menu removeItemAtIndex: last];
		last = [menu numberOfItems] - 1;
		if ([[menu itemAtIndex: last] isSeparatorItem])
			[menu removeItemAtIndex: last];
	}
}


-(void) applicationWillTerminate:(NSNotification *)notification
{
	UNUSED(notification);
	dprintf(("NSApplicationDelegate::applicationWillTerminate: %s", [[[notification object] description] UTF8String]));
	[NSApp setDelegate:nil];
	[NSApp stop:NSApp];
}


-(void) applicationWillBecomeActive:(NSNotification *)notification
{
	UNUSED(notification);
	dprintf(("NSApplicationDelegate::applicationWillBecomeActive: %s", [[[notification object] description] UTF8String]));
}


-(void) applicationDidBecomeActive:(NSNotification *)notification
{
	UNUSED(notification);
	dprintf(("NSApplicationDelegate::applicationDidBecomeActive: %s", [[[notification object] description] UTF8String]));
}


-(void) applicationWillResignActive:(NSNotification *)notification
{
	UNUSED(notification);
	dprintf(("NSApplicationDelegate::applicationWillResignActive: %s", [[[notification object] description] UTF8String]));
}


-(void) applicationDidResignActive:(NSNotification *)notification
{
	UNUSED(notification);
	dprintf(("NSApplicationDelegate::applicationDidResignActive: %s", [[[notification object] description] UTF8String]));
}


- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender
{
	UNUSED(sender);
	dprintf(("NSApplicationDelegate::applicationShouldTerminate: %s", [[sender description] UTF8String]));
	return NSTerminateNow;
}


- (NSMenu *)applicationDockMenu:(NSApplication *)sender
{
	UNUSED(sender);
	return nil;
}


-(BOOL) application:(NSApplication*) theApplication openFile:(NSString*) file
{
	WINDOW_DATA *win = top_window();
	const char *name;
	
	UNUSED(theApplication);
	dprintf(("NSApplicationDelegate::openFile: %s", [file UTF8String]));
	name = [file UTF8String];
	if (!empty(name))
	{
		hv_recent_add(name);
		win = OpenFileInWindow(win, name, NULL, 0, TRUE, FALSE, FALSE);
		if (win != NULL)
			return TRUE;
	}
	return FALSE;
}

static gboolean clipregion_empty(WINDOW_DATA *win)
{
	return win == NULL || !win->selection.valid;
}


- (BOOL)validateUserInterfaceItem:(id <NSValidatedUserInterfaceItem, NSObject, NSCopying, NSCoding>)item
{
	SEL action;
	WINDOW_DATA *win = top_window();
	DOCUMENT *doc = win ? win->data : NULL;

	action = [item action];
	if (action == @selector(openDocument:))
		return TRUE;
	if (action == @selector(performClose:))
		return win != NO_WINDOW;
	if (action == @selector(saveDocument:))
		return doc && doc->buttons.save;
	if (action == @selector(saveDocumentAs:))
		return doc && doc->buttons.save;
	if (action == @selector(recompile:))
		return doc && doc->buttons.save && doc->type == HYP_FT_HYP;
	if (action == @selector(startRemarker:))
		return doc && doc->buttons.remarker;
	if (action == @selector(documentInfo:))
		return win != NO_WINDOW;
	if (action == @selector(runPageLayout:))
		return TRUE;
	if (action == @selector(print:))
		return win != NO_WINDOW;
	if (action == @selector(copy:))
		return !clipregion_empty(win);
	if (action == @selector(selectAll:))
		return win != NO_WINDOW;
	if (action == @selector(performFindPanelAction:))
		return win != NO_WINDOW;

	if (action == @selector(navigateBack:))
		return doc && doc->buttons.back;
	if (action == @selector(navigateClearstack:))
		return doc && doc->buttons.history;
	if (action == @selector(navigateCatalog:))
		return !empty(gl_profile.viewer.catalog_file);
	if (action == @selector(navigatePrev:))
		return doc && doc->buttons.previous;
	if (action == @selector(navigatePrevPhys:))
		return doc && doc->buttons.prevphys;
	if (action == @selector(navigateToc:))
		return doc && doc->buttons.home;
	if (action == @selector(navigateNext:))
		return doc && doc->buttons.next;
	if (action == @selector(navigateNextPhys:))
		return doc && doc->buttons.nextphys;
	if (action == @selector(navigateFirst:))
		return doc && doc->buttons.first;
	if (action == @selector(navigateLast:))
		return doc && doc->buttons.last;
	if (action == @selector(navigateIndex:))
		return doc && doc->buttons.index;
	if (action == @selector(navigateHelp:))
		return doc && doc->buttons.help;

	if (action == @selector(openHistory:))
		return doc && doc->buttons.history;
	if (action == @selector(openBookmarks:))
		return doc && doc->buttons.bookmarks;
	if (action == @selector(openReferences:))
		return doc && doc->buttons.references;

	return [super validateUserInterfaceItem:item];
}

/*
		{
			HMENU submenu = (HMENU)wParam;
			
			if (submenu == win->bookmarks_menu)
				MarkerUpdate(win);
			else if (submenu == win->recent_menu)
				RecentUpdate(win);
		}
*/

@end

@implementation HypViewApplication (NSMenuValidation)
- (BOOL)validateMenuItem:(NSMenuItem *)item
{
	return [super validateMenuItem:item];
}
@end

/********************************************************************/
/*------------------------------------------------------------------*/
/********************************************************************/

@interface NSApplication (NiblessAdditions)
- (void) setAppleMenu:(NSMenu *)menu;
@end

@interface NSMenu (NiblessAdditions)
- (void) _setMenuName:(id)arg1;
@end

@implementation HypViewMenuPopulator

+(void) populateMainMenu
{
	NSMenu *mainMenu;
	NSMenuItem *item;
	NSMenu *submenu;
	
	mainMenu = [[[NSMenu alloc] initWithTitle:@"MainMenu"] autorelease];
	[mainMenu setAutoenablesItems:NO];
	
	/* The titles of the menu items are for identification purposes only and shouldn't be localized. */
	/* The strings in the menu bar come from the submenu titles, */
	/* except for the application menu, whose title is ignored at runtime. */
	item = [mainMenu addItemWithTitle:@"Apple" action:NULL keyEquivalent:@""];
	submenu = [[[NSMenu alloc] initWithTitle:@"Apple"] autorelease];
	[submenu setAutoenablesItems:YES];
	[NSApp performSelector:@selector(setAppleMenu:) withObject:submenu];
	/* [NSApp performSelector:NSSelectorFromString(@"setAppleMenu:") withObject:submenu]; */
	[self populateApplicationMenu:submenu];
	[mainMenu setSubmenu:submenu forItem:item];
	
	item = [mainMenu addItemWithTitle:@"File" action:NULL keyEquivalent:@""];
	submenu = [[[NSMenu alloc] initWithTitle:NSLocalizedString(@"File", nil)] autorelease];
	[submenu setAutoenablesItems:YES];
	[self populateFileMenu:submenu];
	[mainMenu setSubmenu:submenu forItem:item];
	
	item = [mainMenu addItemWithTitle:@"Edit" action:NULL keyEquivalent:@""];
	submenu = [[[NSMenu alloc] initWithTitle:NSLocalizedString(@"Edit", nil)] autorelease];
	[submenu setAutoenablesItems:YES];
	[self populateEditMenu:submenu];
	[mainMenu setSubmenu:submenu forItem:item];
	
	item = [mainMenu addItemWithTitle:@"Navigate" action:NULL keyEquivalent:@""];
	submenu = [[[NSMenu alloc] initWithTitle:NSLocalizedString(@"Navigate", nil)] autorelease];
	[submenu setAutoenablesItems:YES];
	[self populateNavigationMenu:submenu];
	[mainMenu setSubmenu:submenu forItem:item];
	
	item = [mainMenu addItemWithTitle:@"Window" action:NULL keyEquivalent:@""];
	submenu = [[[NSMenu alloc] initWithTitle:NSLocalizedString(@"Window", nil)] autorelease];
	[submenu setAutoenablesItems:YES];
	[self populateWindowMenu:submenu];
	[mainMenu setSubmenu:submenu forItem:item];
	[NSApp setWindowsMenu:submenu];
	
	item = [mainMenu addItemWithTitle:@"Options" action:NULL keyEquivalent:@""];
	submenu = [[[NSMenu alloc] initWithTitle:NSLocalizedString(@"Options", nil)] autorelease];
	[submenu setAutoenablesItems:YES];
	[self populateOptionsMenu:submenu];
	[mainMenu setSubmenu:submenu forItem:item];
	
	item = [mainMenu addItemWithTitle:@"Help" action:NULL keyEquivalent:@""];
	submenu = [[[NSMenu alloc] initWithTitle:NSLocalizedString(@"Help", nil)] autorelease];
	[submenu setAutoenablesItems:YES];
	[self populateHelpMenu:submenu];
	[mainMenu setSubmenu:submenu forItem:item];
	
	[mainMenu setDelegate:NSApp];
	[NSApp setMainMenu:mainMenu];
	[mainMenu update];
}

+(void) populateApplicationMenu:(NSMenu *)menu
{
	NSString *applicationName = [[NSApp delegate] performSelector:@selector(applicationName)];
	NSMenuItem *item;
	NSMutableString *title;
	NSMenu *servicesMenu;
	
	title = [NSLocalizedString(@"About <X>", nil) mutableCopy];
	[title replaceCharactersInRange: [title rangeOfString: @"<X>"] withString: applicationName];
	item = [menu addItemWithTitle:title                                         action:@selector(about:)                        keyEquivalent:@""];
	[item setTarget:NSApp];
	[title release];
	
	[menu addItem:[NSMenuItem separatorItem]];
	item = [menu addItemWithTitle:NSLocalizedString(@"Preferences...", nil)     action:@selector(openPreferences:)              keyEquivalent:@","];
	[item setTarget:NSApp];

	[menu addItem:[NSMenuItem separatorItem]];
	
	item = [menu addItemWithTitle:NSLocalizedString(@"Services", nil)           action:NULL                                     keyEquivalent:@""];
	servicesMenu = [[[NSMenu alloc] initWithTitle:@"Services"] autorelease];
	[servicesMenu setAutoenablesItems:YES];
	[menu setSubmenu:servicesMenu forItem:item];
	[NSApp setServicesMenu:servicesMenu];
	
	[menu addItem:[NSMenuItem separatorItem]];
	
	title = [NSLocalizedString(@"Hide <X>", nil) mutableCopy];
	[title replaceCharactersInRange: [title rangeOfString: @"<X>"] withString: applicationName];
	item = [menu addItemWithTitle:title                                         action:@selector(hide:)                         keyEquivalent:@"h"];
	[item setTarget:NSApp];
	[title release];
	
	item = [menu addItemWithTitle:NSLocalizedString(@"Hide Others", nil)        action:@selector(hideOtherApplications:)        keyEquivalent:@"h"];
	[item setKeyEquivalentModifierMask:NSCommandKeyMask | NSAlternateKeyMask];
	[item setTarget:NSApp];
	
	item = [menu addItemWithTitle:NSLocalizedString(@"Show All", nil)           action:@selector(unhideAllApplications:)        keyEquivalent:@""];
	[item setTarget:NSApp];
	
	[menu addItem:[NSMenuItem separatorItem]];
	
	title = [NSLocalizedString(@"Quit <X>", nil) mutableCopy];
	[title replaceCharactersInRange: [title rangeOfString: @"<X>"] withString: applicationName];
	item = [menu addItemWithTitle:title                                         action:@selector(terminate:)                    keyEquivalent:@"q"];
	[item setTarget:NSApp];
	[title release];
}

+(void) populateFileMenu:(NSMenu *)menu
{
	NSMenuItem *item;
	NSString *title;
	NSMenu *recent_menu;

	item = [menu addItemWithTitle:NSLocalizedString(@"Open...", nil)            action:@selector(openDocument:)                 keyEquivalent:@"o"];
	[item setTarget:NSApp];
	item = [menu addItemWithTitle:NSLocalizedString(@"Open Recent", nil)        action:nil                                      keyEquivalent:@""];
	[item setTarget:NSApp];
	recent_menu = [[[NSMenu alloc] initWithTitle:@"Open Recent"] autorelease];
	[recent_menu setAutoenablesItems:YES];
	[recent_menu performSelector:@selector(_setMenuName:) withObject:@"NSRecentDocumentsMenu"];
	[menu setSubmenu:recent_menu forItem:item];
	m_recent_menu = recent_menu;
	[recent_menu setDelegate:NSApp];

	title = NSLocalizedString(@"Clear Menu", nil);
	item = [recent_menu addItemWithTitle:title                                  action:@selector(clearRecentDocuments:)         keyEquivalent:@""];
	[item setTarget:NSApp];
	
	[menu addItem:[NSMenuItem separatorItem]];
	item = [menu addItemWithTitle:NSLocalizedString(@"Close", nil)              action:@selector(performClose:)                 keyEquivalent:@"w"];
	[item setTarget:NSApp];
	item = [menu addItemWithTitle:NSLocalizedString(@"Save As...", nil)         action:@selector(saveDocumentAs:)               keyEquivalent:@"s"];
	[item setTarget:NSApp];
	item = [menu addItemWithTitle:NSLocalizedString(@"Recompile...", nil)       action:@selector(recompile:)                    keyEquivalent:@"r"];
	[item setTarget:NSApp];

	[menu addItem:[NSMenuItem separatorItem]];
	item = [menu addItemWithTitle:NSLocalizedString(@"Catalog", nil)            action:@selector(navigateCatalog:)              keyEquivalent:@"k"];
	[item setKeyEquivalentModifierMask: NSAlternateKeyMask];
	[item setTarget:NSApp];
	item = [menu addItemWithTitle:NSLocalizedString(@"Default file", nil)       action:@selector(navigateDefaultFile:)          keyEquivalent:@"d"];
	[item setKeyEquivalentModifierMask: NSAlternateKeyMask];
	[item setTarget:NSApp];
	item = [menu addItemWithTitle:NSLocalizedString(@"Run Remarker", nil)       action:@selector(startRemarker:)                keyEquivalent:@"r"];
	[item setKeyEquivalentModifierMask: NSAlternateKeyMask];
	[item setTarget:NSApp];

	[menu addItem:[NSMenuItem separatorItem]];
	item = [menu addItemWithTitle:NSLocalizedString(@"File info...", nil)       action:@selector(documentInfo:)                 keyEquivalent:@"i"];

	[menu addItem:[NSMenuItem separatorItem]];
	item = [menu addItemWithTitle:NSLocalizedString(@"Page Setup...", nil)      action:@selector(runPageLayout:)                keyEquivalent:@"P"];
	[item setTarget:NSApp];
	item = [menu addItemWithTitle:NSLocalizedString(@"Print...", nil)           action:@selector(print:)                        keyEquivalent:@"p"];
	[item setTarget:NSApp];
}

+(void) populateEditMenu:(NSMenu *)menu
{
	NSMenuItem *item;
	
	item = [menu addItemWithTitle:NSLocalizedString(@"Copy", nil)               action:@selector(copy:)                         keyEquivalent:@"c"];
	[item setTarget:NSApp];
	item = [menu addItemWithTitle:NSLocalizedString(@"Paste", nil)              action:@selector(paste:)                        keyEquivalent:@"v"];
	[item setTarget:NSApp];
	item = [menu addItemWithTitle:NSLocalizedString(@"Select All", nil)         action:@selector(selectAll:)                    keyEquivalent:@"a"];
	[item setTarget:NSApp];
	
	[menu addItem:[NSMenuItem separatorItem]];
	item = [menu addItemWithTitle:NSLocalizedString(@"Find...", nil)            action:@selector(performFindPanelAction:)       keyEquivalent:@"f"];
	[item setTarget:NSApp];
	[item setTag:NSFindPanelActionShowFindPanel];
	item = [menu addItemWithTitle:NSLocalizedString(@"Find Next", nil)          action:@selector(performFindPanelAction:)       keyEquivalent:@"g"];
	[item setTarget:NSApp];
	[item setTag:NSFindPanelActionNext];
	item = [menu addItemWithTitle:NSLocalizedString(@"Find Previous", nil)      action:@selector(performFindPanelAction:)       keyEquivalent:@"G"];
	[item setTarget:NSApp];
	[item setTag:NSFindPanelActionPrevious];
}

+(void) populateNavigationMenu:(NSMenu *)menu
{
	NSMenuItem *item;
	NSMenu *bookmarks_menu;

	item = [menu addItemWithTitle:NSLocalizedString(@"Previous logical page", nil)  action:@selector(navigatePrev:)              keyEquivalent:@"\xef\x9c\x82"];
	[item setKeyEquivalentModifierMask: NSControlKeyMask];
	[item setTarget:NSApp];
	item = [menu addItemWithTitle:NSLocalizedString(@"Next logical page", nil)      action:@selector(navigateNext:)              keyEquivalent:@"\xef\x9c\x83"];
	[item setKeyEquivalentModifierMask: NSControlKeyMask];
	[item setTarget:NSApp];
	
	[menu addItem:[NSMenuItem separatorItem]];
	item = [menu addItemWithTitle:NSLocalizedString(@"Previous physical page", nil) action:@selector(navigatePrevPhys:)          keyEquivalent:@""];
	[item setTarget:NSApp];
	item = [menu addItemWithTitle:NSLocalizedString(@"Next logical page", nil)      action:@selector(navigateNextPhys:)          keyEquivalent:@""];
	[item setTarget:NSApp];

	[menu addItem:[NSMenuItem separatorItem]];
	item = [menu addItemWithTitle:NSLocalizedString(@"First page", nil)             action:@selector(navigateFirst:)            keyEquivalent:@""];
	[item setTarget:NSApp];
	item = [menu addItemWithTitle:NSLocalizedString(@"Last page", nil)              action:@selector(navigateLast:)             keyEquivalent:@""];
	[item setTarget:NSApp];

	[menu addItem:[NSMenuItem separatorItem]];
	item = [menu addItemWithTitle:NSLocalizedString(@"Contents", nil)               action:@selector(navigateToc:)              keyEquivalent:@"t"];
	[item setKeyEquivalentModifierMask: NSAlternateKeyMask];
	[item setTarget:NSApp];
	item = [menu addItemWithTitle:NSLocalizedString(@"Index", nil)                  action:@selector(navigateIndex:)            keyEquivalent:@"x"];
	[item setKeyEquivalentModifierMask: NSAlternateKeyMask];
	[item setTarget:NSApp];
	item = [menu addItemWithTitle:NSLocalizedString(@"Show help page", nil)         action:@selector(navigateHelp:)             keyEquivalent:@"h"];
	[item setKeyEquivalentModifierMask: NSAlternateKeyMask];
	[item setTarget:NSApp];

	[menu addItem:[NSMenuItem separatorItem]];
	item = [menu addItemWithTitle:NSLocalizedString(@"Bookmarks", nil)              action:nil                                  keyEquivalent:@""];
	[item setTarget:NSApp];
	bookmarks_menu = [[[NSMenu alloc] initWithTitle:@"Bookmarks"] autorelease];
	[bookmarks_menu setAutoenablesItems:YES];
	[menu setSubmenu:bookmarks_menu forItem:item];
	m_bookmarks_menu = bookmarks_menu;
	[bookmarks_menu setDelegate:NSApp];

	[menu addItem:[NSMenuItem separatorItem]];
	item = [menu addItemWithTitle:NSLocalizedString(@"Back one page", nil)          action:@selector(navigateBack:)             keyEquivalent:@"\x0008"];
	[item setKeyEquivalentModifierMask: 0];
	[item setTarget:NSApp];
	item = [menu addItemWithTitle:NSLocalizedString(@"Clear stack", nil)            action:@selector(navigateClearstack:)       keyEquivalent:@"e"];
	[item setKeyEquivalentModifierMask: NSAlternateKeyMask];
	[item setTarget:NSApp];
}

+(void) populateWindowMenu:(NSMenu *)menu
{
	[menu addItemWithTitle:NSLocalizedString(@"Minimize", nil)                  action:@selector(performMiniaturize:)           keyEquivalent:@"m"];
	[menu addItemWithTitle:NSLocalizedString(@"Zoom", nil)                      action:@selector(performZoom:)                  keyEquivalent:@""];
	[menu addItem:[NSMenuItem separatorItem]];
	[menu addItemWithTitle:NSLocalizedString(@"Bring All to Front", nil)        action:@selector(arrangeInFront:)               keyEquivalent:@""];
}

+(void) populateOptionsMenu:(NSMenu *)menu
{
	NSMenuItem *item;

	item = [menu addItemWithTitle:NSLocalizedString(@"Font...", nil)            action:@selector(selectFont:)                   keyEquivalent:@"z"];
	[item setKeyEquivalentModifierMask: NSAlternateKeyMask];
	[item setTarget:NSApp];
	item = [menu addItemWithTitle:NSLocalizedString(@"Colors...", nil)          action:@selector(selectColor:)                  keyEquivalent:@"c"];
	[item setKeyEquivalentModifierMask: NSAlternateKeyMask];
	[item setTarget:NSApp];

	[menu addItem:[NSMenuItem separatorItem]];
	item = [menu addItemWithTitle:NSLocalizedString(@"Output...", nil)          action:@selector(configOutput:)                 keyEquivalent:@"o"];
	[item setKeyEquivalentModifierMask: NSAlternateKeyMask];
	[item setTarget:NSApp];

	[menu addItem:[NSMenuItem separatorItem]];
	item = [menu addItemWithTitle:NSLocalizedString(@"Alternative font...", nil) action:@selector(toggleAltfont:)               keyEquivalent:@"z"];
	[item setKeyEquivalentModifierMask: NSControlKeyMask];
	[item setTarget:NSApp];
	HypViewApp->useAltFontMenuItem = item;
	item = [menu addItemWithTitle:NSLocalizedString(@"Expand multiple spaces...", nil) action:@selector(toggleExpandSpaces:)    keyEquivalent:@"l"];
	[item setKeyEquivalentModifierMask: NSControlKeyMask];
	[item setTarget:NSApp];
	HypViewApp->expandSpacesMenuItem = item;

	[menu addItem:[NSMenuItem separatorItem]];
	item = [menu addItemWithTitle:NSLocalizedString(@"Settings...", nil)        action:@selector(openPreferences:)              keyEquivalent:@"s"];
	[item setKeyEquivalentModifierMask: NSAlternateKeyMask];
	[item setTarget:NSApp];
}

+(void) populateHelpMenu:(NSMenu *)menu
{
	NSString *applicationName = [[NSApp delegate] performSelector:@selector(applicationName)];
	NSMenuItem *item;
	NSMutableString *title;
	
	title = [NSLocalizedString(@"<X> Help", nil) mutableCopy];
	[title replaceCharactersInRange: [title rangeOfString: @"<X>"] withString: applicationName];
	item = [menu addItemWithTitle:title                                         action:@selector(showHelp:)                     keyEquivalent:@"?"];
	[item setTarget:NSApp];
	[title release];
}

@end

/********************************************************************/
/*------------------------------------------------------------------*/
/********************************************************************/

@implementation NSAutoreleasePool (Debug)
- (void)dump
{
#if 0
	Ivar v;
	id value;
	void *value2 = 0;
	
	v = class_getInstanceVariable([self class], "_token");
	value = object_getIvar(self, v);
	printf("%p: %s: \n", value, ivar_getTypeEncoding(v));
	v = object_getInstanceVariable(self, "_token", &value2);
	printf("%p: %s: \n", value2, ivar_getTypeEncoding(v));
#endif
}
@end


/********************************************************************/
/*------------------------------------------------------------------*/
/********************************************************************/

static unsigned char parse_hex(const char *str)
{
	unsigned char val;
	if (str[0] >= '0' && str[0] <= '9')
		val = str[0] - '0';
	else if (str[0] >= 'a' && str[0] <= 'f')
		val = str[0] - 'a' + 10;
	else if (str[0] >= 'A' && str[0] <= 'F')
		val = str[0] - 'A' + 10;
	else
		val = 0;
	val <<= 4;
	if (str[1] >= '0' && str[1] <= '9')
		val |= str[1] - '0';
	else if (str[1] >= 'a' && str[1] <= 'f')
		val |= str[1] - 'a' + 10;
	else if (str[1] >= 'A' && str[1] <= 'F')
		val |= str[1] - 'A' + 10;
	return val;
}

/*** ---------------------------------------------------------------------- ***/

static void parse_color(const char *name, int rgb[3])
{
	if (name == NULL || *name != '#' || strlen(name) != 7)
	{
		rgb[0] = rgb[1] = rgb[2] = 0;
		return;
	}
	rgb[0] = parse_hex(name + 1);
	rgb[1] = parse_hex(name + 3);
	rgb[2] = parse_hex(name + 5);
}

/*** ---------------------------------------------------------------------- ***/

static Pixel get_color(const char *name)
{
	int rgb[3];
	
	parse_color(name, rgb);
	return (rgb[0] << 24) | (rgb[1] << 16) | rgb[2] | 0xff000000;
}

/*** ---------------------------------------------------------------------- ***/

static void ValidateColors(void)
{
	_WORD display_colors = GetNumColors();
	
	viewer_colors.background = get_color(gl_profile.colors.background);
	viewer_colors.text = get_color(gl_profile.colors.text);
	viewer_colors.link = get_color(gl_profile.colors.link);
	viewer_colors.xref = get_color(gl_profile.colors.xref);
	viewer_colors.popup = get_color(gl_profile.colors.popup);
	viewer_colors.system = get_color(gl_profile.colors.system);
	viewer_colors.rx = get_color(gl_profile.colors.rx);
	viewer_colors.rxs = get_color(gl_profile.colors.rxs);
	viewer_colors.quit = get_color(gl_profile.colors.quit);
	viewer_colors.close = get_color(gl_profile.colors.close);
	viewer_colors.error = get_color("#ff0000"); /* used to display invalid links in hypertext files */
	viewer_colors.ghosted = get_color(gl_profile.colors.ghosted);
	
	if (viewer_colors.background == viewer_colors.text)
		viewer_colors.background = viewer_colors.text ^ 1;
	if (display_colors < 16)
		viewer_colors.link =
		viewer_colors.popup =
		viewer_colors.xref =
		viewer_colors.system =
		viewer_colors.rx =
		viewer_colors.rxs =
		viewer_colors.quit =
		viewer_colors.close =
		viewer_colors.ghosted =
		viewer_colors.error = viewer_colors.text;
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

enum {
	OPT_GEOMETRY = 256,
	OPT_NEW_WINDOW
};

static struct option const long_options[] = {
	{ "geometry", required_argument, NULL, OPT_GEOMETRY },
	{ "new-window", no_argument, NULL, OPT_NEW_WINDOW },
	{ "help", no_argument, NULL, 'h' },
	{ "version", no_argument, NULL, 'V' },
	
	{ NULL, no_argument, NULL, 0 }
};
	

static gboolean NOINLINE ParseCommandLine(int *argc, const char ***pargv)
{
	struct _getopt_data *d;
	gboolean retval = TRUE;
	int c;
	int i;
	const char **argv = *pargv;
	
	bShowVersion = FALSE;
	bShowHelp = FALSE;
	bNewWindow = FALSE;

	for (i = 1; i < *argc; i++)
	{
		if (strncmp(argv[i], "-psn_", 5) == 0)
		{
			/* Some funky Finder thing. */
			char curdir[PATH_MAX];
			const char *home;
			
			/*
			 * stupid Finder tends to run us in the root directory
			 */
			if (getcwd(curdir, sizeof(curdir)) != NULL && strcmp(curdir, "/") == 0 && (home = getenv("HOME")) != NULL)
				chdir(home);
			memmove(&argv[i], &argv[i + 1], (*argc - i - 1) * sizeof(char *));
			*argc -= 1;
		} else if (strcmp(argv[i], "-NSDocumentRevisionsDebugMode") == 0)
		{
			/* Some funky XCode thing. */
			memmove(&argv[i], &argv[i + 1], (*argc - i - 1) * sizeof(char *));
			*argc -= 1;
		}
	}
	argv[*argc] = 0;

	getopt_init_r(gl_program_name, &d);
	while ((c = getopt_long_only_r(*argc, argv, "hV?", long_options, NULL, d)) != EOF)
	{
		switch (c)
		{
		case OPT_GEOMETRY:
			geom_arg = getopt_arg_r(d);
			break;
		case OPT_NEW_WINDOW:
			bNewWindow = TRUE;
			break;
		case 'h':
			bShowHelp = TRUE;
			break;
		case 'V':
			bShowVersion = TRUE;
			break;
		case '?':
			if (getopt_opt_r(d) == '?')
			{
				bShowHelp = TRUE;
			} else
			{
				retval = FALSE;
			}
			break;
		case 0:
			/* option which just sets a var */
			break;
		
		default:
			/* error message already issued */
			retval = FALSE;
			break;
		}
	}

	if (bShowHelp)
	{
		char *msg = g_strdup_printf(_("\
HypView macOS Version %s\n\
ST-Guide Hypertext File Viewer\n\
\n\
usage: %s [FILE [CHAPTER]]"), gl_program_version, gl_program_name);
		write_console(msg, FALSE, FALSE, TRUE);
		g_free(msg);
	}
	
	if (retval)
	{
		int oind = getopt_ind_r(d);
		*argc = *argc - oind;
		*pargv += oind;
	}
	
	getopt_finish_r(&d);

	return retval;
}

/*** ---------------------------------------------------------------------- ***/

static void show_version(void)
{
	char *url = g_strdup_printf(_("%s is Open Source (see %s for further information).\n"), gl_program_name, HYP_URL);
	char *hyp_version = hyp_lib_version();
	char *msg = g_strdup_printf(
		"HypView macOS Version %s\n"
		"HCP %s\n"
		"%s\n"
		"%s",
		gl_program_version,
		hyp_version,
		HYP_COPYRIGHT,
		url);
	write_console(msg, FALSE, FALSE, FALSE);
	g_free(msg);
	g_free(hyp_version);
	g_free(url);
}

/*** ---------------------------------------------------------------------- ***/

#include "hypmain.h"

int main(int argc, const char **argv)
{
	int exit_status = EXIT_SUCCESS;
	char *real;
	const char *argv0;

	/* Much of Cocoa needs one of these to be available. */
	pool = [[NSAutoreleasePool alloc] init];
	
	argv0 = [[[[NSProcessInfo processInfo] arguments] objectAtIndex:0] fileSystemRepresentation];
	real = realpath(argv0, NULL);
	m_executablePath = [[NSString stringWithCString:real encoding:NSUTF8StringEncoding] stringByStandardizingPath];
	free(real);
	dprintf(("executableName: %s -> %s", argv0, [m_executablePath UTF8String]));

	HypProfile_Load(TRUE);
	
	if (!ParseCommandLine(&argc, &argv))
		return EXIT_FAILURE;
	
	if (bShowHelp)
	{
		/* already handled in ParseCommandLine() */
	} else if (bShowVersion)
	{
		show_version();
	} else
	{
		WINDOW_DATA *win = NULL;
		int new_window = TRUE;
		
		nonib_init();
		
		/* Need an NSApp instance */
		[HypViewApplication sharedApplication];
	
		/* Make the current process a foreground application, i.e. an app
		 * with a user interface, in case we're not running from a .app bundle
		 */
#if 1
		[NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
#else
		{
			ProcessSerialNumber psn;
				
			GetCurrentProcess(&psn);
			TransformProcessType(&psn, kProcessTransformToForegroundApplication);
		}
#endif

		hv_init();
		ValidateColors();
	
		Help_Init();
		
		if (!empty(geom_arg))
			gtk_XParseGeometry(geom_arg, &gl_profile.viewer.win_x, &gl_profile.viewer.win_y, &gl_profile.viewer.win_w, &gl_profile.viewer.win_h);
		{
			char *str= g_strdup_printf("%dx%d+%d+%d",
				gl_profile.viewer.win_w,
				gl_profile.viewer.win_h,
				gl_profile.viewer.win_x,
				gl_profile.viewer.win_y);
			hv_win_set_geometry(str);
			g_free(str);
		}
		
		if (bNewWindow)
			new_window = FORCE_NEW_WINDOW;
		
		if (argc <= 0)
		{
			/* default-hypertext specified? */
			if (gl_profile.viewer.startup == 1 &&
				(!empty(gl_profile.viewer.default_file) || !empty(gl_profile.viewer.catalog_file)))
			{
				char *filename = path_subst(empty(gl_profile.viewer.default_file) ? gl_profile.viewer.catalog_file : gl_profile.viewer.default_file);
				win = OpenFileInWindow(NULL, filename, NULL, 0, TRUE, new_window, FALSE);
				g_free(filename);
			} else if (gl_profile.viewer.startup == 2 &&
				!empty(gl_profile.viewer.last_file))
			{
				char *filename = path_subst(gl_profile.viewer.last_file);
				win = OpenFileInWindow(NULL, filename, NULL, 0, TRUE, new_window, FALSE);
				g_free(filename);
			}
		} else
		{
			if (argc == 1 && hyp_guess_filetype(argv[0]) != HYP_FT_HYP)
			{
				win = search_allref(win, argv[0], FALSE);
			} else
			{
				/* ...load this file (incl. chapter) */
				if (argc > 1)
					win = OpenFileInWindow(NULL, argv[0], argv[1], HYP_NOINDEX, TRUE, new_window, FALSE);
				else
					win = OpenFileInWindow(NULL, argv[0], NULL, 0, TRUE, new_window, FALSE);
			}
		}
		if (win == NULL)
			win = SelectFileLoad(NULL);						/* use file selector */
		
		if (win == NULL)
		{
			exit_status = EXIT_FAILURE;
		} else
		{
			hv_recent_add(win->data->path);
			hv_win_open(win);
			if (gl_profile.remarker.run_on_startup)
				StartRemarker(win, remarker_startup, FALSE);
		}
	}

	if (toplevels_open_except(NULL) != 0 || 1)
	{
		[NSApp run];
	}
	
	Help_Exit();

	hv_exit();
	HypProfile_Delete();

	x_free_resources();

	return exit_status;
}
