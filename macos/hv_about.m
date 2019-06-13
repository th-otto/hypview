#include "hv_defs.h"
#include "hv_vers.h"
#include "hypdebug.h"

#undef dprintf
#if 0
#define dprintf(x) hyp_debug x
#else
#define dprintf(x)
#endif

#define autorelease self

@interface AboutPanel : NSWindow <NSWindowDelegate>
{
@public
@private
}
- (void)close;
@end

@implementation AboutPanel
- (void)close
{
	dprintf(("close: %s", [[self description] UTF8String]));
#if 0
	[HypViewApp stopModal];
#endif
	[super close];
}

- (void)dealloc
{
	dprintf(("dealloc: %s", [[self description] UTF8String]));
	[super dealloc];
}

static NSTextField *urllabel(const char *format, const char *display, const char *url)
{
	NSURL *nsurl = [NSURL URLWithString: [[[NSString alloc] initWithUTF8String: url] autorelease]];
	NSTextField *label;
	NSString *wformat = [[[NSString alloc] initWithUTF8String: format] autorelease];
	NSRange range;
	
	range = [wformat rangeOfString:@"%s"];
	NSString *str = [[[NSString alloc] initWithUTF8String: display] autorelease];
	wformat = [wformat stringByReplacingCharactersInRange:range withString:str];
	NSMutableAttributedString *attr = [[[NSMutableAttributedString alloc] initWithString:wformat] autorelease];
	range.length = [str length];
	[attr addAttribute:NSLinkAttributeName value:nsurl range:range];
	[attr addAttribute:NSForegroundColorAttributeName value:[NSColor blueColor] range:range];
	[attr addAttribute:NSUnderlineStyleAttributeName value:[NSNumber numberWithInt: NSUnderlineStyleSingle] range:range];
	label = [NSTextField labelWithAttributedString:attr];
	[label setAllowsEditingTextAttributes:YES];
	[label setSelectable: YES];
	/*
	 * prevent changing the font of the link to system default when activated
	 */
	range = NSMakeRange(0, [attr length]);
	[attr addAttribute:NSFontAttributeName value:[label font] range:range];
	[label setAttributedStringValue: attr];
	return label;
}

- (id)init
{
	NSUInteger windowStyle = NSTitledWindowMask | NSClosableWindowMask;
	NSBackingStoreType bufferingType = NSBackingStoreBuffered;
	NSStackView *vbox, *hbox, *vbox2;
	NSImageView *image;
	NSTextField *label;
	NSButton *button;
	char *str;
	char *compiler_version;
	char *hyp_version;
	char *url;
	
	if ((self = [super initWithContentRect:NSMakeRect(0, 0, 400, 300) styleMask: windowStyle backing: bufferingType defer: NO]) == nil)
	{
		return nil;
	}
	dprintf(("init: %s", [[self description] UTF8String]));
	[self cascadeTopLeftFromPoint:NSMakePoint(20, 20)];
	[self setTitle: W_("HypView Versionsinfo")];

	vbox = [[[NSStackView alloc] init] autorelease];
	[vbox setSpacing: 10];
	[vbox setOrientation: NSUserInterfaceLayoutOrientationVertical];
	[vbox setAlignment: NSLayoutAttributeLeading];
	[vbox setEdgeInsets: NSEdgeInsetsMake(10, 10, 10, 10)];
	[self setContentView: vbox];
	hbox = [[[NSStackView alloc] init] autorelease];
	[hbox setSpacing: 10];
	[vbox addView:hbox inGravity: NSStackViewGravityLeading];
	image = [NSImageView imageViewWithImage: HypViewApp.applicationIconImage];
	[hbox addView:image inGravity: NSStackViewGravityLeading];

	vbox2 = [[[NSStackView alloc] init] autorelease];
	[vbox2 setOrientation: NSUserInterfaceLayoutOrientationVertical];
	[vbox2 setAlignment: NSLayoutAttributeLeft];
	[vbox2 setEdgeInsets: NSEdgeInsetsMake(10, 10, 10, 10)];
	[hbox addView:vbox2 inGravity: NSStackViewGravityLeading];

	str = g_strdup_printf(_("HypView macOS Version %s"), gl_program_version);
	label = [NSTextField labelWithString: [[[NSString alloc] initWithUTF8String: str] autorelease]];
	[label setAlignment: NSLeftTextAlignment];
	g_free(str);
	[vbox2 addView:label inGravity: NSStackViewGravityLeading];
	hyp_version = hyp_lib_version();
	str = g_strdup_printf(_("HCP %s"), hyp_version);
	label = [NSTextField labelWithString: [[[NSString alloc] initWithUTF8String: str] autorelease]];
	[label setAlignment: NSLeftTextAlignment];
	g_free(str);
	g_free(hyp_version);
	[vbox2 addView:label inGravity: NSStackViewGravityLeading];
	str = g_strdup_printf(_("(compiled %s)"), gl_compile_date);
	label = [NSTextField labelWithString: [[[NSString alloc] initWithUTF8String: str] autorelease]];
	[label setAlignment: NSLeftTextAlignment];
	g_free(str);
	[vbox2 addView:label inGravity: NSStackViewGravityLeading];
	compiler_version = hyp_compiler_version();
	label = [NSTextField labelWithString: [[[NSString alloc] initWithUTF8String: compiler_version] autorelease]];
	[label setAlignment: NSLeftTextAlignment];
	g_free(compiler_version);
	[vbox2 addView:label inGravity: NSStackViewGravityLeading];

	hbox = [[[NSStackView alloc] init] autorelease];
	[hbox setSpacing: 10];
	[hbox setEdgeInsets: NSEdgeInsetsMake(10, 10, 10, 10)];
	[vbox addView:hbox inGravity: NSStackViewGravityLeading];
	
	vbox2 = [[[NSStackView alloc] init] autorelease];
	[vbox2 setOrientation: NSUserInterfaceLayoutOrientationVertical];
	[vbox2 setAlignment: NSLayoutAttributeLeft];
	[hbox addView:vbox2 inGravity: NSStackViewGravityLeading];

	label = [NSTextField labelWithString: [[[NSString alloc] initWithUTF8String: HYP_COPYRIGHT] autorelease]];
	[label setAlignment: NSLeftTextAlignment];
	[vbox2 addView:label inGravity: NSStackViewGravityLeading];

	url = g_strdup_printf(_("%s is Open Source (see %%s for further information)."), gl_program_name);
	label = urllabel(url, HYP_URL, HYP_URL);
	[label setAlignment: NSLeftTextAlignment];
	g_free(url);
	[vbox2 addView:label inGravity: NSStackViewGravityLeading];

	str = g_strdup_printf(_("Author: %s"), HYP_AUTHOR);
	label = [NSTextField labelWithString: [[[NSString alloc] initWithUTF8String: str] autorelease]];
	[label setAlignment: NSLeftTextAlignment];
	g_free(str);
	[vbox2 addView:label inGravity: NSStackViewGravityLeading];

	label = urllabel(_("Email: %s"), HYP_EMAIL, "mailto:" HYP_EMAIL);
	[label setAlignment: NSLeftTextAlignment];
	[vbox2 addView:label inGravity: NSStackViewGravityLeading];
	
	label = urllabel(_("Homepage: %s"), HYP_HOMEPAGE, HYP_HOMEPAGE);
	[label setAlignment: NSLeftTextAlignment];
	[vbox2 addView:label inGravity: NSStackViewGravityLeading];
	
	hbox = [[[NSStackView alloc] init] autorelease];
	[hbox setSpacing: 10];
	[hbox setEdgeInsets: NSEdgeInsetsMake(10, 10, 10, 10)];
	[vbox addView:hbox inGravity: NSStackViewGravityLeading];
	
	button = [NSButton buttonWithTitle:W_("Close") target:self action:@selector(performClose:)];
	[hbox setAlignment: NSLayoutAttributeCenterY];
	[hbox addView:button inGravity: NSStackViewGravityTrailing];

	[self makeFirstResponder:button];
	[button setHighlighted:YES];
	[button setKeyEquivalent:@"\015"];

#if 0
	print_hierarchy([self contentView]);
#endif

	return self;
}

@end

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

void About(id sender)
{
	AboutPanel *panel;
	
	panel = [[[AboutPanel alloc] init] autorelease];
	[panel makeKeyAndOrderFront: sender];
	
#if 0
	NSModalSession session = [HypViewApp beginModalSessionForWindow: panel];
	for (;;)
	{
		if ([HypViewApp runModalSession:session] != NSModalResponseContinue)
			break;
	}
	[HypViewApp endModalSession:session];
#endif
}
