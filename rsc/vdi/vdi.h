/*
 * This is an implementation of a VDI driver screen that actually draws
 * to an offscreen framebuffer.
 *
 * KNOWN BUGS:
 * - v_justified() ignores the char_space and word_space parameters
 *
 * - v_contourfill is not yet implemented
 *
 * - text rotation is not implemented
 *
 * - text effects italics and outlined are not yet supported
 *
 * - vro_cpyfm is only supported from memory to screen
 *
 * - NDC coordinates are not supported
 */
 
#include "config.h"
#include <portab.h>
#include <portvdi.h>
#include <string.h>

#include "vdidefs.h"
#include "fonthdr.h"

/*
Opcode      VDI Name                   Function

    1       v_opnwk                    Open Workstation
    2       v_clswk                    Close Workstation
    3       v_clrwk                    Clear Workstation
    4       v_updwk                    Update Workstation
    5       v_escape                   Escape Functions
        1   vq_chcells                 Inquire addressable character cells
        2   v_exit_cur                 Exit alpha mode
        3   v_enter_cur                Enter alpha mode
        4   v_curup                    Cursor up
        5   v_curdown                  Cursor down
        6   v_curright                 Cursor right
        7   v_curleft                  Cursor left
        8   v_curhome                  Home Cursor
        9   v_eeos                     Erase to end of screen
       10   v_eeol                     Erase to end of line
       11   vs_curaddress              Direct cursor address
       12   v_curtext                  Output cursor addressable text
       13   v_rvon                     Start reverse video
       14   v_rvoff                    End reverse video
       15   vq_curaddress              Inquire current alpha cursor address
       16   vq_tabstatus               Inquire Tablet Status
       17   v_hardcopy                 Hardcopy
       18   v_dspcur                   Place graphic cursor
       19   v_rmcur                    Remove last graphic cursor
       20   v_form_adv                 Eject page
       21   v_output_window            Output Window
       22   v_clear_disp_list          Clear Display List
       23   v_bit_image                Output Bit Image File
       24   vq_scan                    Inquire Printer Scan
       25   v_alpha_text               Output Alpha Text
       27   v_orient                   Set Orientation for Output
       28   v_copies                   Set Number of Copies
       29   v_tray,v_trays             Set Input and Output Tray
       32   v_ps_halftone              PostScript halftoning
       36   vq_traynames               Inquire Tray Names
       37   v_page_size                Set Page Size
       38   vq_page_name               Inquire Page Name
       39   vq_prn_scaling             Inquire Printer Scaling
       60   vs_palette                 Select Palette
       61   v_sound                    generate specified tone
       62   vs_mute                    set/clear tone muting flag
       76   vs_calibrate               Set calibration
       77   vq_calibrate               Inquire calibration
       81   vt_resolution              set tablet axis resolution in lines/inch
       82   vt_axis                    set tablet axis resolution in lines
       83   vt_origin                  set tablet x and y origin
       84   vq_tdimensions             return tablet x and y dimensions
       85   vt_alignment               set tablet alignment
       91   vqp_films,vqp_filmname     Inquire Polaroid Film Types
       92   vqp_state                  Inquire Polaroid Driver State
       93   vsp_state                  Set Polaroid Driver State
       93   vsc_expose                 Disable Or Enable Film Exposure For Frame
       94   vsp_save                   Save Polaroid Driver State
       95   vsp_message                Suppress Polaroid Message
       96   vqp_error                  Polaroid Error Inquire
       98   v_meta_extents             Update Metafile Extents
       99   v_write_meta               Write metafile item
       99,0 vm_pagesize                Set the physical page size for metafiles
       99,1 vm_coords                  Custom coordinate system for metafiles
       99,32 v_bez_qual                Set Bezier quality
       100  vm_filename                Change GEM VDI File Name
       101  v_offset                   Set Line Offset
       101  v_xbit_image               Output Bit Image File
       102  v_fontinit                 Init System Font
       102  vs_bkcolor                 Set Background Color
       2000 v_escape2000               Special function for ATARI page-printer
       2100 vq_margins                 Inquire Printer Margins
       2103 vs_document_info           Set Document Info
       0x4844 v_setrgbi                Set Color Representation
       0x4845 v_topbot                 Set Character Height (abs.)
    6,0     v_pline                    Polyline
    6,13    v_bez                      Output Bezier
    7       v_pmarker                  Polymarker
    8       v_gtext                    Text
    9,0     v_fillarea                 Filled area
    9,13    v_bez_fill                 Output filled Bezier
   10       v_cellarray                Cell array
   11                                  GDP
        1   v_bar                      Bar
        2   v_arc                      Arc
        3   v_pieslice                 Pie
        4   v_circle                   Circle
        5   v_ellipse                  Ellipse
        6   v_ellarc                   Elliptical arc
        7   v_ellpie                   Elliptical pie
        8   v_rbox                     Rounded rectangle
        9   v_rfbox                    Filled rounded rectangle
       10   v_justified                Justified graphics text
       11   v_etext                    Output text graphically at a given position
       12   unused
       13   v_bez_on,v_bez_off         Enable/Disable Bezier capabilities
    12      vst_height                 Set character height, absolute mode
    13      vst_rotation               Set text rotation
    14      vs_color                   Set color representation
    15      vsl_type                   Set polyline linetype
    16      vsl_width                  Set polyline width
    17      vsl_color                  Set polyline color index
    18      vsm_type                   Set polymarker type
    19      vsm_height                 Set polymarker height
    20      vsm_color                  Set polymarker color index
    21      vst_font                   Set text face
    22      vst_color                  Set graphics text color index
    23      vsf_interior               Set fill interior style
    24      vsf_style                  Set fill style index
    25      vsf_color                  Set fill color index
    26      vq_color                   Inquire color representation
    27      vq_cellarray               Inquire color lookup table
    28      vrq_locator,vsm_locator    Input locator
    29      vrq_valuator,vsm_valuator  Input Valuator
    30      vrq_choice,vsm_choice      Input Choice
    31      vrq_string,vsm_string      Input string
    32      vswr_mode                  Set writing mode
    33      vsin_mode                  Set input mode
    34      unused
    35      vql_attribues              Inquire current polyline attributes
    36      vqm_attributes             Inquire current polymarker attributes
    37      vqf_attributes             Inquire current fill area attributes
    38      vqt_attributes             Inquire current graphic text attributes
    39      vst_alignment              Set graphic text alignment
   100,0    v_opnvwk                   Open virtual screen workstation
   100,1    v_opnbm                    Open offscreen bitmap
   100,2    v_resize_bm                Resize offscreen bitmap
   100,3    v_open_bm                  Open offscreen bitmap
   101,0    v_clsvwk                   Close virtual screen workstation
   101,1    v_clsbm                    Close offscreen bitmap
   102,0    vq_extnd                   Extended inquire function
   102,1    vq_scrninfo                Inquire Screen Information
   103      v_contourfill              Countour fill
   104      vsf_perimeter              Set fill perimeter visibility
   105      v_get_pixel                Get Pixel
   106      vst_effects                Set graphic text special effects
   107      vst_point                  Set character cell height, points mode
   108      vsl_ends                   Set polyline end styles
   109      vro_cpyfm                  Copy raster, opaque
   110      vr_trnfm                   Transform form
   111      vsc_form                   Set mouse form
   112      vsf_udpat                  Set user-defined fill pattern
   113      vsl_udsty                  Set user-defined linestyle
   114      vr_recfl                   Fill rectangle
   115      vqin_mode                  Inquire input mode
   116      vqt_extent                 Inquire text extent
   117      vqt_width                  Inquire character cell width
   118      vex_timv                   Exchange timer interrupt vector
   119      vst_load_fonts             Load Fonts
   120      vst_unload_fonts           Unload Fonts
   121      vrt_cpyfm                  Copy raster, transparent
   122      v_show_c                   Show cursor
   123      v_hide_c                   Hide cursor
   124      vq_mouse                   Sample mouse button state
   125      vex_butv                   Exchange button change vector
   126      vex_motv                   Exchange mouse movement vector
   127      vex_curv                   Exchange cursor change vector
   128      vq_key_s                   Sample keyboard state information
   129      vs_clip                    Set clipping rectangle
   130,0    vqt_name                   Inquire face name and index
   130,1    vqt_ext_name               Inquire face name and index
   131      vqt_fontinfo               Inquire current face information
   132      vqt_justified              Inquire justified graphics text
   133      vs_grayoverride            Override gray level
   134      vex_wheelv                 Exchange mouse wheel vector
   134      v_pat_rotate,vex_wheelv    Pattern rotation
   138      v_setrgb                   Set Color Representation
   170,0    vr_transfer_bits           Transfer Bitmap
   171,0    vr_clip_rects_by_dst       Clip Rects By Destination Rectangle
   171,1    vr_clip_rects_by_src       Clip Rects By Source Rectangle
   171,2    vr_clip_rects32_by_dst     Clip Rects By Destination Rectangle
   171,3    vr_clip_rects32_by_src     Clip Rects By Source Rectangle
   180      v_create_driver_info       Inquire info about a driver
   181      v_delete_driver_info       Discard information about a driver
   182,0    v_read_default_settings    Read default settings
   182,1    v_write_default_settings   Write default settings
   190      vqt_char_index             Inquire Character Index
   200,0    vst_fg_color               Set Text Foreground Color
   200,1    vsf_fg_color               Set Fill Foreground Color
   200,2    vsl_fg_color               Set Line Foreground Color
   200,3    vsm_fg_color               Set Marker Foreground Color
   200,4    vsr_fg_color               Set Raster Foreground Color
   201,0    vst_bg_color               Set Text Background Color
   201,1    vsf_bg_color               Set Fill Background Color
   201,2    vsl_bg_color               Set Line Background Color
   201,3    vsm_bg_color               Set Marker Background Color
   201,4    vsr_bg_color               Set Raster Background Color
   202,0    vqt_fg_color               Inquire Text Foreground Color
   202,1    vqf_fg_color               Inquire Fill Foreground Color
   202,2    vql_fg_color               Inquire Line Foreground Color
   202,3    vqm_fg_color               Inquire Marker Foreground Color
   202,4    vqr_fg_color               Inquire Raster Foreground Color
   203,0    vqt_bg_color               Inquire Text Background Color
   203,1    vqf_bg_color               Inquire Fill Background Color
   203,2    vql_bg_color               Inquire Line Background Color
   203,3    vqm_bg_color               Inquire Marker Background Color
   203,4    vqr_bg_color               Inquire Raster Background Color
   204,0    v_color2value              Translate Color Entry To Pixel Value
   204,1    v_value2color              Translate Pixel Value To Color Entry
   204,2    v_color2nearest            Inquire Nearest Color Entry
   204,3    vq_px_format               Inquire pixel format and color space
   205,0    vs_ctab                    Set Color Table
   205,1    vs_ctab_entry              Set Color Table Entry
   205,2    vs_dflt_ctab               Set Default Color Table
   206,0    vq_ctab                    Inquire Current Color Table
   206,1    vq_ctab_entry              Inquire Color Table Entry
   206,2    vq_ctab_id                 Inquire Current Color Table Id
   206,3    v_ctab_idx2vdi             Translate Color Table Index To Vdi Color Index
   206,4    v_ctab_vdi2idx             Translate Vdi Color Index To Color Table Index
   206,5    v_ctab_idx2value           Inquire Color Table Value
   206,6    v_get_ctab_id              Get Unique Color Table Id
   206,7    vq_dflt_ctab               Inquire Default Color Table
   206,8    v_create_ctab              Create Color Table
   206,9    v_delete_ctab              Delete Color Table
   207,0	vs_hilite_color            Set Hilite Color
   207,1	vs_min_color               Set Minimum Color
   207,2	vs_max_color               Set Maximum Color
   207,1	vs_weight_color            Set Weight Color
   208,0	v_create_itab              Create Inverse Color Table Reference
   208,1	v_delete_itab              Delete Inverse Color Table Reference
   209,0	vq_hilite_color            Inquire Hilite Color
   209,1	vq_min_color               Inquire Minimum Color
   209,2	vq_max_color               Inquire Maximum Color
   209,3	vq_weight_color            Inquire Weight Color
   224,100  vs_backmap
   224,101  vs_outmode
   224,105  vs_use_fonts
   225      vqt_drv_avail
   226,1    v_set_cachedir
   226,2    v_get_cachedir
   226,3    v_def_cachedir
   226,4    v_clr_cachedir
   226,5    v_delete_cache
   226,6    v_save_cache
   229,0    vqt_xfntinfo               Inquire extended font information
   230,0    vst_name                   Set text face by name
   230,100  vqt_name_and_id            Inquire face name and ID by name
   231      vst_width                  Set character width
   232      vqt_fontheader             Inquire Speedo header information
   233      v_mono_ftext
   234      vqt_trackkern              Inquire track kerning information
   235      vqt_pairkern               Inquire pair kerning information
   236      vst_charmap,vst_map_mode   Set character mapping mode
   237      vst_kern                   Set kerning mode
   237      vst_track_offset           Set track-kerning offset
   238      vq_ptsinsz                 Get size of the PTSIN array
   239      v_getbitmap_info           Get character bitmap information
   240,0    vqt_f_extent               Inquire Outline-Font Text Extent
   240,4200 vqt_real_extent            Inquire real outline font text extent
   241,0    v_ftext                    Output Outlined Text
   241,0    v_ftext_offset             Output Outlined Text with Offsets
   242      v_killoutline              Kill FSM outline
   243,0    v_getoutline               Get character outline
   243,1    v_get_outline              Get character outline 
   243,31   v_fgetoutline              Get character outline
   244      vst_scratch                Set scratch buffer allocation mode
   245      vst_error                  Set Outline Font error mode
   246      vst_arbpt,vst_arbpt32      Set character cell height by arbitrary points
   247      vqt_advance,vqt_advance32  Inquire Outline text advance placement vector
   248      vq_devinfo                 Inquire device status info
   248,4242 vq_ext_devinfo             Inquire extended device status info
   249      v_savecache                Save Outline cache to disk
   250      v_loadcache                Load Outline cache
   251      v_flushcache               Flush Outline cache
   252      vst_setsize,vst_setsize32  Set character cell width by arbitrary points
   253      vst_skew                   Set outline font skew
   254      vqt_get_table              Get Character Mapping Table
   255,0    vqt_cachesize              Get Outline cache size
   255,100  vqt_cacheinfo
   
   -1,1     GEM/3: page out GEM
   -1,2     GEM/3: set font file extension
   -1,3     GEM/3: PD list. Returns path to GDOS in INTOUT and driver paths at ptr1.
   -1,4     GEM/3: v_get_driver_info
   -1,5     GEM/3: Get font info. Returns INTOUT[0] = segment of font buffer and INTOUT[1] = size of font buffer in bytes.
   -1,6     GEM/3: v_set_app_buff
   -1,7     GEM/4: Fill INTOUT with a 0-terminated ASCII string.
   -1,8     GEM/4: Fill INTOUT with a 0-terminated ASCII version number (for GEM/4, this is "3.91".
*/


#define MAX_VWK 100
#define MAX_POINTS 1024
#define MAX_VDI_COLS	256

#define LAST_SCREEN_DEVICE 10
#define VDI_SCREEN_DEVICE   1
#define VDI_PLOTTER_DEVICE 11
#define VDI_PRINTER_DEVICE 21
#define VDI_META_DEVICE    31
#define VDI_CAMERA_DEVICE  41
#define VDI_TABLET_DEVICE  51
#define VDI_MEMORY_DEVICE  61
#define VDI_OFFSCREEN_DEVICE (VDI_MEMORY_DEVICE + 1)
#define VDI_FAX_DEVICE     81
#define VDI_BITMAP_DEVICE  91
#define VDI_MM_DEVICE      101
#define VDI_SOUND_DEVICE   111


/*
 * Meta-Escape (v_write_meta) Sub-Opcodes
 */
#define GEM_START_GROUP    10  	/* Defines the start of a group, i.e. of a set of associated objects */
#define GEM_END_GROUP      11  	/* Marks the end of a group, i.e. of a set of associated objects */
#define GEM_BEZ_QUAL       32  	/* See v_bez_qual */
#define GEM_UNK_34         34	/* No information available at present */
#define GEM_NO_LINE_STYLE  49  	/* Any kind of line style should be switched off */
#define GEM_START_SHADOW   50  	/* Set Attribute Shadow On */
#define GEM_END_SHADOW     51 	/* Set Attribute Shadow Off */
#define GEM_START_FILL     80  	/* Start Draw Area Type Primitive */
#define GEM_END_FILL       81  	/* End Draw Area Type Primitive */
#define GEM_VM_VER_APP    101  	/* Gives information about the name and the version number */
#define GEM_COLOR         102  	/* No information available at present */
#define GEM_UNK_103       103	/* No information available at present */
#define GEM_UNK_107       107	/* No information available at present */
#define GEM_UNK_111       111	/* No information available at present */
#define GEM_START_BGIF    170  	/* Starts text output with a BGI vector font */
#define GEM_END_BGIF      171  	/* Marks the end of text output with a BGI vector font */
#define GEM_WIND          190  	/* Sets the position, zoom step and format of a window */
#define GEM_GRID          191  	/* Permits permanent saving of raster settings of a window */
#define GEM_ALIGN         192  	/* Describes the reference object */
#define GEM_START_GREY    193  	/* Specifies a grey-tone as fill area */
#define GEM_END_GREY      194  	/* Marks the end of a user-defined grey-tone pattern */
#define GEM_START_BEZIER  195  	/* Defines a Bezier stroke */
#define GEM_END_BEZIER    196  	/* Marks the end of a Bezier stroke */
#define GEM_START_JOIN    197	/* Permits the saving of more coordinates than the metafile driver allows */
#define GEM_END_JOIN      198  	/* Marks the end of a section defined by GEM_START_JOIN */
#define GEM_UNK_199       199	/* No information available at present */
#define GEM_UNK_201       201	/* No information available at present */
#define GEM_UNK_203       203	/* No information available at present */
#define GEM_UNK_230       230

/* input device */

#define MODE_UNDEFINED 0

/* text effects */

#define TXT_NORMAL       0x0000
#define TXT_THICKENED    0x0001
#define TXT_LIGHT        0x0002
#define TXT_SKEWED       0x0004
#define TXT_UNDERLINED   0x0008
#define TXT_OUTLINED     0x0010
#define TXT_SHADOWED     0x0020

/* colors */
#define WHITE            0
#define BLACK            1
#define RED              2
#define GREEN            3
#define BLUE             4
#define CYAN             5
#define YELLOW           6
#define MAGENTA          7
#define LWHITE           8
#define LBLACK           9
#define LRED            10
#define LGREEN          11
#define LBLUE           12
#define LCYAN           13
#define LYELLOW         14
#define LMAGENTA        15

#define CSPACE_RGB	0x0001
#define CSPACE_ARGB	0x0002
#define CSPACE_CMYK	0x0004


/* vst_charmap modes */
#define MAP_BITSTREAM   0
#define MAP_ATARI       1
#define MAP_UNICODE     2 /* for vst_map_mode, NVDI 4 */

/* bit blt rules */

#define ALL_WHITE        0		/* D := 0 */
#define S_AND_D          1		/* D := S AND D */
#define S_AND_NOTD       2		/* D := S AND (NOT D) */
#define S_ONLY           3		/* D := S */
#define NOTS_AND_D       4		/* D := (NOT S) AND D */
#define D_ONLY           5		/* D := D */
#define S_XOR_D          6		/* D := S XOR D */
#define S_OR_D           7		/* D := S OR D */
#define NOT_SORD         8		/* D := NOT (S OR D) */
#define NOT_SXORD        9		/* D := NOT (S XOR D) */
#define NOT_D           10		/* D := NOT D */
#define S_OR_NOTD       11		/* D := S OR (NOT D) */
#define NOT_S           12		/* D := NOT S */
#define NOTS_OR_D       13		/* D := (NOT S) OR D */
#define NOT_SANDD       14		/* D := NOT (S AND D) */
#define ALL_BLACK       15		/* D := 1 */

/* polymarker types */

#define PM_MAX           PM_DIAMOND

/* linetypes */

#define LT_MAX          USERLINE


#define SYSTEM_FONT_ID 1
#define SYSTEM_FONT_NAME "6x6 system font"


#define MOUSE_CURSOR_WIDTH 16
#define MOUSE_CURSOR_HEIGHT 16
#define MOUSE_CURSOR_MASKSIZE (MOUSE_CURSOR_HEIGHT * MOUSE_CURSOR_WIDTH / 16)

#define PATTERN_WIDTH 16
#define PATTERN_HEIGHT 16
#define PATTERN_SIZE (PATTERN_HEIGHT * PATTERN_WIDTH / 16)

void vdi_init(void);
void vditrap(VDIPB *pb);
int vdi_output_c(_WORD dev, unsigned char c);
int vdi_phys_handle(void);
void vdi_change_colors(void);
gboolean vdi_vq_vgdos(void);
int vdi_cursconf(_WORD func, _WORD rate);
void vdi_cursblink(void);


#define SYSFONTS    3
#define NLSFONTSETS 5

#define SUPPORTED_EFFECTS (TXT_THICKENED | TXT_LIGHT | TXT_SKEWED | TXT_UNDERLINED | TXT_OUTLINED)


typedef _WORD vdi_palette[MAX_VDI_COLS][3];


struct devinfo {
	_WORD max_x;			/* maximum horizontal position */
	_WORD max_y;			/* maximum vertical position */
	_WORD scale_flag;		/* scaling flag */
	_WORD pix_width;		/* pixel width */
	_WORD pix_height;		/* pixel height */
	_WORD font_sizes;		/* number of font sizes */
	_WORD line_types;		/* number of line types */
	_WORD line_widths;		/* number of line widths */
	_WORD marker_types;		/* number of marker types */
	_WORD marker_sizes;		/* number of marker sizes */
	_WORD num_fonts;		/* number of fonts */
	_WORD num_patterns;		/* number of patterns */
	_WORD num_shapes;		/* number of shapes */
	_WORD num_colors;		/* number of colors */
	_WORD num_gdps;			/* number of gdps */
	_WORD gdp_funcs[10];	/* available gdps */
	_WORD gdp_attribs[10];	/* gdp attribute types */
	_WORD color_flag;		/* colors available */
	_WORD rotation_flag;	/* text rotation available */
	_WORD fillarea_flag;	/* fill area available */
	_WORD cellarray_flag;	/* CELLARRAY available */
	_WORD available_colors;	/* number of available colors */
	_WORD cursor_control;	/* cursor control */
	_WORD valuator_control;	/* valuator control */
	_WORD choice_control;	/* choice control */
	_WORD string_control;	/* string control */
	_WORD device_type;		/* device type */
};

struct sizeinfo {
	_WORD min_char_width;	/* XXX min. char width */
	_WORD min_char_height;	/* XXX min. baseline-topline */
	_WORD max_char_width;	/* XXX max. char width */
	_WORD max_char_height;	/* XXX max. baseline-topline */
	_WORD min_line_width;	/* XXX min. line width */
	_WORD reserved1;
	_WORD max_line_width;	/* XXX max. line width */
	_WORD reserved2;
	_WORD min_marker_width;	/* XXX min. marker width */
	_WORD min_marker_height;/* XXX min. marker height */
	_WORD max_marker_width;	/* XXX max. marker width */
	_WORD max_marker_height;/* XXX max. marker height */
};


struct inqinfo {
	_WORD screen_type;					/* shared graphic/text */
	_WORD background_colors;			/* number of background colors */
	_WORD supported_effects;			/* supported text effects */
	_WORD scaling_flag;					/* scaling */
	_WORD planes;						/* planes */
	_WORD clut_flag;					/* lookup table */
	_WORD blits_per_sec;				/* blit operations/s */
	_WORD contourfill_flag;				/* contourfill available */
	_WORD rotation_flag;				/* text rotation available in 90 degree steps */
	_WORD num_wrmodes;					/* # writing modes */
	_WORD max_input_mode;				/* highest input mode */
	_WORD justification_flag;			/* text justification available */
	_WORD pen_change;					/* pen change */
	_WORD ribbon_change;				/* ribbon change */
	_WORD max_points;					/* max # of points for pline/pmarker/fillarea */
	_WORD max_intin_size;				/* max size of intin array */
	_WORD mouse_buttons;				/* mouse buttons */
	_WORD line_types_flag;				/* line types for linewidth > 1 available */
	_WORD wrmode_flag;					/* writing modes for linewidth > 1 available */
	_WORD clipping;						/* clipping flag */
	_WORD pixsize_flag;					/* extended precision pixel size information in following fields */
	_WORD pix_width;					/* pixel width in 1/10, 1/100 or 1/1000 microns */
	_WORD pix_height;					/* pixel height in 1/10, 1/100 or 1/1000 microns */
	_WORD hdpi;							/* horizontal resolution in dpi */
	_WORD vdpi;							/* vertical resolution in dpi */
	_WORD image_rotation;				/* image rotation available */
	_WORD quarter_screen_high;			/* address of quarter screen buffer (PC/GEM) */
	_WORD quarter_screen_low;
	_WORD bezier_flag;					/* bezier flag */
	_WORD reserved1;
	_WORD raster_flags;					/* raster flags */
	_WORD reserved2;
	_WORD color_flags;					/* color management etc */
	_WORD reserved3;
	_WORD reserved4;
	_WORD reserved5;
	_WORD reserved6;
	_WORD reserved7;
	_WORD reserved8;
	_WORD reserved9;
	_WORD left_border;					/* not imprintable left border in pixels (printers/plotters) */
	_WORD top_border;					/* not imprintable upper border in pixels (printers/plotters) */
	_WORD right_border;					/* not imprintable right border in pixels (printers/plotters) */
	_WORD bottom_border;				/* not imprintable lower border in pixels (printers/plotters) */
	_WORD page_size;					/* page size/paper format (printers etc.) */
};


/*
 * global state variables for alpha text/vt52.
 * Shared between physical workstation, all
 * virtual screen workstations and BIOS/XBIOS calls.
 * Most (if not all) of these variables directly
 * correspond to information also found in
 * negative LineA-Variables.
 */
struct alpha_info {
	int ax, ay;          /* -> V_CUR_XY */
	int aw, ah;          /* -> V_CEL_MX, V_CEL_MY */
	int acw;
	int ach;             /* -> V_CEL_HT */
	int curs_hid_cnt;    /* -> V_HID_CNT */
	int start_esc;
	int asavex, asavey;  /* -> SV_CUR_X, SV_CUR_Y */
	int wrap_on;         /* -> Bit #3 of V_STAT_0 */
	int rev_on;          /* -> Bit #4 of V_STAT_0 */
	int afg, abg;        /* -> V_COL_FG, V_COL_BG */
	int blinking;        /* -> Bit #0 of V_STAT_0 */
	int blink_rate;      /* -> V_PERIOD */
	int blink_count;     /* -> V_CUR_CT */
	int cursor_inverse;  /* -> Bit #1 of V_STAT_0 */
	int blink_delay;     /* -> V_DELAY */
	int v_cur_of;        /* -> V_CUR_OF; not implemented */
	int font_index;
};


typedef struct _vdi_rectangle {
	int x, y;
	int width, height;
} vdi_rectangle;

struct bezier {
	gboolean on;
	int qual;
};

typedef struct _vwk VWK;
struct _vwk
{
	int handle;
	int phys_wk;

	int width;
	int height;
	int planes;
	int form_id;						/* INTERLEAVED/STANDARD/PIXPACKED */
	int bit_order;
	void *bitmap_addr;
	void *to_free;
	gboolean can_clip;
	_WORD xfm_mode;
	_WORD driver_id;
	
	vdi_palette *req_col;				/* colors as requested by vs_color */
	
	int wrmode;
	int clipping;
	vdi_rectangle clipr;
	int input_mode[5];
	
	int line_color;
	int line_type;
	int line_width;
	int line_ends;
#define SQUARED 0
#define ARROWED 1
#define ROUNDED 2
	_UWORD ud_linepat;

	int text_color;
	int text_rotation;
	int font_id;
	int font_index;
	int font_yoff;
	int font_xoff;
	int v_align;
	int h_align;
	int text_style;
	int underline_size;
	int outline_size;
	_UWORD dda_inc;
	_WORD skew;
	_WORD mapmode;
	_WORD pairmode;
	fix31 kern_offset;
	
	int fill_color;
	int fill_interior;
	int fill_style;
	int fill_perimeter;
	int fill_perimeter_whichcolor; /* 0 = use fill color; 1 = use line color */
	
	int marker_color;
	int marker_type;
	int marker_height;
	int marker_scale;
	
	int bg_color;
	struct bezier bezier;
	
	/* variables for alpha text/vt52 */
	struct alpha_info *vt52;
	
	struct devinfo dev_tab;
	struct sizeinfo siz_tab;
	struct inqinfo inq_tab;

	_UWORD ud_fill_pattern[PATTERN_SIZE];

	struct {
		int (*v_clswk)(VWK *v, VDIPB *pb);
		int (*v_clrwk)(VWK *v, VDIPB *pb);
		int (*v_updwk)(VWK *v, VDIPB *pb);
		int (*v_form_adv)(VWK *v, VDIPB *pb);
		int (*v_output_window)(VWK *v, VDIPB *pb);
		int (*v_clear_disp_list)(VWK *v, VDIPB *pb);
		int (*v_bit_image)(VWK *v, VDIPB *pb);
		int (*vq_scan)(VWK *v, VDIPB *pb);
		int (*v_alpha_text)(VWK *v, VDIPB *pb);
		int (*v_orient)(VWK *v, VDIPB *pb);
		int (*v_copies)(VWK *v, VDIPB *pb);
		int (*v_tray)(VWK *v, VDIPB *pb);
		int (*v_ps_halftone)(VWK *v, VDIPB *pb);
		int (*vq_tray_names)(VWK *v, VDIPB *pb);
		int (*v_page_size)(VWK *v, VDIPB *pb);
		int (*vq_page_name)(VWK *v, VDIPB *pb);
		int (*vq_prn_scaling)(VWK *v, VDIPB *pb);
		int (*vs_calibrate)(VWK *v, VDIPB *pb);
		int (*vq_calibrate)(VWK *v, VDIPB *pb);
		int (*vt_resolution)(VWK *v, VDIPB *pb);
		int (*vt_axis)(VWK *v, VDIPB *pb);
		int (*vt_origin)(VWK *v, VDIPB *pb);
		int (*vq_tdimensions)(VWK *v, VDIPB *pb);
		int (*vt_alignment)(VWK *v, VDIPB *pb);
		int (*vqp_films)(VWK *v, VDIPB *pb);
		int (*vqp_state)(VWK *v, VDIPB *pb);
		int (*vsp_state)(VWK *v, VDIPB *pb);
		int (*vsp_save)(VWK *v, VDIPB *pb);
		int (*vsp_message)(VWK *v, VDIPB *pb);
		int (*vqp_error)(VWK *v, VDIPB *pb);
		int (*v_meta_extents)(VWK *v, VDIPB *pb);
		int (*vm_filename)(VWK *v, VDIPB *pb);
		int (*v_offset)(VWK *v, VDIPB *pb);
		int (*v_pline)(VWK *v, VDIPB *pb);
		int (*v_bez)(VWK *v, VDIPB *pb);
		int (*v_pmarker)(VWK *v, VDIPB *pb);
		int (*v_gtext)(VWK *v, VDIPB *pb);
		int (*v_fillarea)(VWK *v, VDIPB *pb);
		int (*v_bez_fill)(VWK *v, VDIPB *pb);
		int (*v_bar)(VWK *v, VDIPB *pb);
		int (*v_arc)(VWK *v, VDIPB *pb);
		int (*v_pieslice)(VWK *v, VDIPB *pb);
		int (*v_circle)(VWK *v, VDIPB *pb);
		int (*v_ellipse)(VWK *v, VDIPB *pb);
		int (*v_ellarc)(VWK *v, VDIPB *pb);
		int (*v_ellpie)(VWK *v, VDIPB *pb);
		int (*v_rbox)(VWK *v, VDIPB *pb);
		int (*v_rfbox)(VWK *v, VDIPB *pb);
		int (*v_justified)(VWK *v, VDIPB *pb);
		int (*v_clsbm)(VWK *v, VDIPB *pb);
		int (*v_contourfill)(VWK *v, VDIPB *pb);
		int (*v_get_pixel)(VWK *v, VDIPB *pb);
		int (*vro_cpyfm)(VWK *v, VDIPB *pb);
		int (*vr_recfl)(VWK *v, VDIPB *pb);
		int (*vrt_cpyfm)(VWK *v, VDIPB *pb);
		int (*v_ftext)(VWK *v, VDIPB *pb);
	} drv;
};

typedef struct {
	short lbearing;			/* origin to left edge of raster */
	short rbearing;			/* origin to right edge of raster */
	short width;			/* advance to next char's origin */
	short ascent;			/* baseline to top edge of raster */
	short descent;			/* baseline to bottom edge of raster */
} vdi_charinfo;

#define VDI_FONTNAMESIZE 32

#define VDI_FONT_BITMAP   0
#define VDI_FONT_SPEEDO   1
#define VDI_FONT_TRUETYPE 2
#define VDI_FONT_TYPE1    3

typedef struct
{
	uint8_t height;
	uint8_t cellheight;
	uint8_t top;
	uint8_t ascent;
	uint8_t half;
	uint8_t descent;
	uint8_t bottom;
	uint8_t width;
	uint8_t cellwidth;
	uint8_t left_offset;
	uint8_t right_offset;
	uint8_t point;
} sysfontinfo;

typedef struct _font_desc {
	char name[VDI_FONTNAMESIZE];
	int charset;
	int cellwidth;
	int cellheight;
	int width;
	int height;
	int top;
	int ascent;
	int half;
	int descent;
	int bottom;
	int left_offset;
	int right_offset;
	int pointsize;
	int first_char;
	int last_char;
	int default_char;
	int thicken;
	int underline_size;
	int lighten;
	int skew;
	
	gboolean monospaced;
	gboolean all_chars_exist;
	int font_format;
	gboolean scaled;
	int font_id;
	int font_index;
	const FONT_HDR *hdr;
	vdi_charinfo per_char[256];
} FONT_DESC;


#ifndef TRACE_VDI
#define TRACE_VDI 0
#endif

/* call was done completly native; dont pass to ROM */
#define VDI_DONE TRUE
/* pass call to ROM, and call vdi_post when that call returns */
#define VDI_PASS FALSE

#if TRACE_VDI
#include "debug.h"
#define V(fmt, ...) debugout("VDI: " fmt "\n", ## __VA_ARGS__)
#else
#define V(fmt, ...)
#endif


#define MIN_MKWD  15
#define MIN_MKHT  11
#define MAX_MKWD  120
#define MAX_MKHT  88


#define PV_CONTROL(pb) (pb)->control
#define PV_INTIN(pb)   (pb)->intin
#define PV_PTSIN(pb)   (pb)->ptsin
#define PV_INTOUT(pb)  (pb)->intout
#define PV_PTSOUT(pb)  (pb)->ptsout

#define V_CONTRL(pb, _x) control[_x]
#define V_INTIN(pb, _x)  intin[_x]
#define V_PTSIN(pb, _x)  ptsin[_x]
#define V_PTSINX(pb, _x) V_PTSIN(pb, 2 * (_x) + 0)
#define V_PTSINY(pb, _x) V_PTSIN(pb, 2 * (_x) + 1)
#define V_PTSINX32(pb, _x) V_PTSIN(pb, 4 * (_x) + 0)
#define V_PTSINY32(pb, _x) V_PTSIN(pb, 4 * (_x) + 2)
#define V_INTOUT(pb, _x) intout[_x]
#define PTSOUT(_x)       ptsout[_x]
#define PTSOUTX(_x) PTSOUT(2 * (_x) + 0)
#define PTSOUTY(_x) PTSOUT(2 * (_x) + 1)

#define VDI_NPTRINTS (sizeof(void *) / sizeof(short))

#ifdef __GNUC__

/* to avoid "dereferencing type-punned pointer" */
static __inline int32_t *__vdi_intin_long(_WORD n, _WORD *vdi_intin)
{
	return ((int32_t *)(vdi_intin   +n));
}
#define vdi_intin_long(n)  *__vdi_intin_long(n, intin)

static __inline int32_t *__vdi_intout_long(_WORD n, _WORD *vdi_intout)
{
	return ((int32_t *)(vdi_intout   +n));
}
#define vdi_intout_long(n)  *__vdi_intout_long(n, intout)

static __inline void **__vdi_intout_ptr(_WORD n, _WORD *vdi_intout)
{
	return ((void **)(vdi_intout   +n));
}
#define vdi_intout_ptr(n, t)  *((t *)__vdi_intout_ptr(n, intout))

static __inline int32_t *__vdi_ptsout_long(_WORD n, _WORD *vdi_ptsout)
{
	return ((int32_t *)(vdi_ptsout   +n));
}
#define vdi_ptsout_long(n)  *__vdi_ptsout_long(n, ptsout)

static __inline int32_t *__vdi_ptsin_long(_WORD n, _WORD *vdi_ptsin)
{
	return ((int32_t *)(vdi_ptsin   +n));
}
#define vdi_ptsin_long(n)  *__vdi_ptsin_long(n, ptsin)

static __inline void **__vdi_intin_ptr(_WORD n, _WORD *vdi_intin)
{
	return ((void**)(vdi_intin + n));
}
#define vdi_intin_ptr(n, t)  *((t *)__vdi_intin_ptr(n, intin))

static __inline void **__vdi_control_ptr(_WORD n, _WORD *vdi_control)
{
	return ((void**)(vdi_control + 7 + n * VDI_NPTRINTS));
}
#define vdi_control_ptr(n, t)  *((t *)__vdi_control_ptr(n, control))

#else

#define vdi_control_ptr(n, t)   *((t *)(control + 7 + (n) * VDI_NPTRINTS))
#define vdi_intin_ptr(n, t)     *((t *)(intin + (n)))
#define vdi_intin_long(n)       *((int32_t *)(intin + (n)))
#define vdi_intout_long(n)      *((int32_t *)(intout + (n)))
#define vdi_intout_ptr(n, t)    *((t *)(intout + n))
#define vdi_ptsout_long(n)      *((int32_t *)(ptsout + n))
#define vdi_ptsin_long(n)       *((int32_t *)(ptsin + n))

#endif


#define V_OPCODE(pb)     V_CONTRL(pb, 0)
#define V_NPTSIN(pb)     V_CONTRL(pb, 1)
#define V_NPTSOUT(pb, x) V_CONTRL(pb, 2) = x
#define V_NINTIN(pb)     V_CONTRL(pb, 3)
#define V_NINTOUT(pb, x) V_CONTRL(pb, 4) = x
#define V_SUBCODE(pb)    (_UWORD)V_CONTRL(pb, 5)
#define V_HANDLE(pb)     V_CONTRL(pb, 6)
#define P_ADDR(pb, x)    V_CONTRL(pb, 7 + 2 * (x))

#define OPCODE(op, sub) (((uint32_t)(int16_t)(op) << 16) | (uint32_t)(uint16_t)(sub))

#define V_OPNWK OPCODE(1, 0)
#define V_CLSWK OPCODE(2, 0)
#define V_OPNVWK OPCODE(100, 0)
#define V_OPNBM OPCODE(100, 1)
#define V_OPEN_BM OPCODE(100, 3)


/*
 * returns TRUE if H is a valid VDI handle
 */
#define VALID_V_HANDLE(h) ((h) >= 0 && (h) < MAX_VWK && vwk[h] != NULL)
/*
 * returns TRUE if V is a VWK for the screen
 */
#define IS_SCREEN_V(v) ((v)->phys_wk == phys_handle)
/*
 * returns TRUE if H is a valid VDI handle for the screen
 */
#define VALID_S_HANDLE(h) (VALID_V_HANDLE(h) && IS_SCREEN_V(vwk[h]))


#define	MIN_BEZIER_QUAL		0
#define	MAX_BEZIER_QUAL		7

#define	BEZIER_START	0x01
#define	POINT_MOVE		0x02

typedef struct {
	_WORD x;
	_WORD y;
} VDI_POINT;

struct v_bez_pars
{
	short num_pts;
	VDI_POINT *points;
	char *bezarr;
	short extent[4];
	short totpoints;
	short totmoves;
	vdi_rectangle clipr;
};

void vdi_init_bm(VWK *v);
void vdi_init_common(VWK *v);
void vdi_release_handle(VDIPB *pb, _WORD h);
void vdi_clswk(VWK *v);

void vditrap(VDIPB *pb);
