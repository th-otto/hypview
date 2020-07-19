/*
 * HypView - (c)      - 2006 Philipp Donze
 *               2006 -      Philipp Donze & Odd Skancke
 *
 * A replacement hypertext viewer
 *
 * This file is part of HypView.
 *
 * HypView is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * HypView is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with HypView; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "hv_defs.h"
#include "picture.h"
#include "hypdebug.h"

struct _viewer_colors viewer_colors;

static _WORD const rgb_to_vdi_tab[256] = {
   0,    4,    8,   12,   16,   20,   24,   28,   32,   36,   40,   43,   47,   51,   55,   59,
  63,   67,   71,   75,   79,   83,   86,   90,   94,   98,  102,  106,  110,  114,  118,  122,
 126,  129,  133,  137,  141,  145,  149,  153,  157,  161,  165,  168,  172,  176,  180,  184,
 188,  192,  196,  200,  204,  208,  211,  215,  219,  223,  227,  231,  235,  239,  243,  247,
 251,  254,  258,  262,  266,  270,  274,  278,  282,  286,  290,  294,  298,  301,  305,  309,
 313,  317,  321,  325,  329,  333,  337,  341,  345,  349,  352,  356,  360,  364,  368,  372,
 376,  380,  384,  388,  392,  396,  400,  403,  407,  411,  415,  419,  423,  427,  431,  435,
 439,  443,  447,  450,  454,  458,  462,  466,  470,  474,  478,  482,  486,  490,  494,  498,
 501,  505,  509,  513,  517,  521,  525,  529,  533,  537,  541,  545,  549,  552,  556,  560,
 564,  568,  572,  576,  580,  584,  588,  592,  596,  600,  603,  607,  611,  615,  619,  623,
 627,  631,  635,  639,  643,  647,  650,  654,  658,  662,  666,  670,  674,  678,  682,  686,
 690,  694,  698,  701,  705,  709,  713,  717,  721,  725,  729,  733,  737,  741,  745,  749,
 752,  756,  760,  764,  768,  772,  776,  780,  784,  788,  792,  796,  800,  803,  807,  811,
 815,  819,  823,  827,  831,  835,  839,  843,  847,  850,  854,  858,  862,  866,  870,  874,
 878,  882,  886,  890,  894,  898,  901,  905,  909,  913,  917,  921,  925,  929,  933,  937,
 941,  945,  949,  952,  956,  960,  964,  968,  972,  976,  980,  984,  988,  992,  996, 1000
};

/*** ---------------------------------------------------------------------- ***/

_WORD pic_rgb_to_vdi(unsigned char c)
{
	return rgb_to_vdi_tab[c];
}

/*** ---------------------------------------------------------------------- ***/

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

static void parse_color(const char *name, _WORD rgb[3])
{
	unsigned char val;
	if (name == NULL || *name != '#' || strlen(name) != 7)
	{
		rgb[0] = rgb[1] = rgb[2] = 0;
		return;
	}
	val = parse_hex(name + 1);
	rgb[0] = pic_rgb_to_vdi(val);
	val = parse_hex(name + 3);
	rgb[1] = pic_rgb_to_vdi(val);
	val = parse_hex(name + 5);
	rgb[2] = pic_rgb_to_vdi(val);
}

/*** ---------------------------------------------------------------------- ***/

static _WORD get_color(const char *name)
{
	_WORD rgb[3];
	_WORD color[3];
	_WORD i, display_colors;
	_WORD closest = G_BLACK;
	long closest_dist = 1000l * 1000l + 1000l * 1000l + 1000l * 1000l;
	long dist, diff;
	
	parse_color(name, rgb);
	display_colors = GetNumColors();
	if (display_colors > 256)
		display_colors = 256;
	for (i = 0; i < display_colors; i++)
	{
		vq_color(vdi_handle, i, 1, color);
		diff = color[0] - rgb[0];
		dist = diff * diff;
		diff = color[1] - rgb[1];
		dist += diff * diff;
		diff = color[2] - rgb[2];
		dist += diff * diff;
		if (dist < closest_dist)
		{
			closest_dist = dist;
			closest = i;
		}
	}
	return closest;
}

/*** ---------------------------------------------------------------------- ***/

void hv_init_colors(void)
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
		viewer_colors.error = viewer_colors.text;
}
