/**
 * @file m_render.c
 */

/*
Copyright (C) 1997-2008 UFO:AI Team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "m_main.h"
#include "m_render.h"
#include "../cl_video.h"
#include "../renderer/r_draw.h"

/**
 * @brief Fills a box of pixels with a single color
 */
void MN_DrawFill (int x, int y, int w, int h, int align, const vec4_t color)
{
	R_DrawFill(x, y, w, h, color);
}

/**
 * @brief Searches for an image in the image array
 * @param[in] name The name of the image relative to pics/
 * @note name may not be null and has to be longer than 4 chars
 * @return NULL on error or image_t pointer on success
 * @sa R_FindImage
 */
const struct image_s *MN_LoadImage (const char *name)
{
	const struct image_s *image = R_FindImage(va("pics/%s", name), it_pic);
	if (image == r_noTexture)
		return NULL;
	return image;
}

void MN_DrawNormImage (float x, float y, float w, float h, float sh, float th, float sl, float tl, int align, const image_t *image)
{
	float nw, nh, x1, x2, x3, x4, y1, y2, y3, y4;
	float imageTexcoords[8];
	short imageVerts[8];

	if (!image)
		return;

	/* normalize to the screen resolution */
	x1 = x * viddef.rx;
	y1 = y * viddef.ry;

	/* provided width and height (if any) take precedence */
	if (w)
		nw = w * viddef.rx;
	else
		nw = 0;

	if (h)
		nh = h * viddef.ry;
	else
		nh = 0;

	/* horizontal texture mapping */
	if (sh) {
		if (!w)
			nw = (sh - sl) * viddef.rx;
		sh /= image->width;
	} else {
		if (!w)
			nw = ((float)image->width - sl) * viddef.rx;
		sh = 1.0f;
	}
	sl /= image->width;

	/* vertical texture mapping */
	if (th) {
		if (!h)
			nh = (th - tl) * viddef.ry;
		th /= image->height;
	} else {
		if (!h)
			nh = ((float)image->height - tl) * viddef.ry;
		th = 1.0f;
	}
	tl /= image->height;

	/* alignment */
	if (align > 0 && align < ALIGN_LAST) {
		/* horizontal (0 is left) */
		switch (align % 3) {
		case 1:
			x1 -= nw * 0.5;
			break;
		case 2:
			x1 -= nw;
			break;
		}

		/* vertical (0 is upper) */
		switch ((align % 9) / 3) {
		case 1:
			y1 -= nh * 0.5;
			break;
		case 2:
			y1 -= nh;
			break;
		}
	}

	/* fill the rest of the coordinates to make a rectangle */
	x4 = x1;
	x3 = x2 = x1 + nw;
	y2 = y1;
	y4 = y3 = y1 + nh;

	/* slanting */
	if (align >= 9 && align < ALIGN_LAST) {
		x1 += nh;
		x2 += nh;
	}

	imageTexcoords[0] = sl;
	imageTexcoords[1] = tl;
	imageTexcoords[2] = sh;
	imageTexcoords[3] = tl;
	imageTexcoords[4] = sh;
	imageTexcoords[5] = th;
	imageTexcoords[6] = sl;
	imageTexcoords[7] = th;
	imageVerts[0] = x1;
	imageVerts[1] = y1;
	imageVerts[2] = x2;
	imageVerts[3] = y2;
	imageVerts[4] = x3;
	imageVerts[5] = y3;
	imageVerts[6] = x4;
	imageVerts[7] = y4;

	R_DrawImageArray(imageTexcoords, imageVerts, image);
}

/**
 * @brief Draws an image or parts of it
 * @param[in] x X position to draw the image to
 * @param[in] y Y position to draw the image to
 * @param[in] w Width of the image
 * @param[in] h Height of the image
 * @param[in] sh Right x corner coord of the square to draw
 * @param[in] th Lower y corner coord of the square to draw
 * @param[in] sl Left x corner coord of the square to draw
 * @param[in] tl Upper y corner coord of the square to draw
 * @param[in] align The alignment we should use for placing the image onto the screen (see align_t)
 * @param[in] blend Enable the blend mode (for alpha channel images)
 * @param[in] name The name of the image - relative to base/pics
 * @sa R_RegisterImage
 * @note All these parameter are normalized to VID_NORM_WIDTH and VID_NORM_HEIGHT
 * they are adjusted in this function
 */
const image_t *MN_DrawNormImageByName (float x, float y, float w, float h, float sh, float th, float sl, float tl, int align, const char *name)
{
	const struct image_s *image;

	image = MN_LoadImage(name);
	if (!image) {
		Com_Printf("Can't find pic: %s\n", name);
		return NULL;
	}

	MN_DrawNormImage(x, y, w, h, sh, th, sl, tl, align, image);
	return image;
}

/**
 * @brief draw a panel from a texture as we can see on the image
 * The function is inline because there are often 3 or 5 const param, with it a lot of var became const too
 * @image html http://ufoai.ninex.info/wiki/images/Inline_draw_panel.png
 * @param[in] pos Position of the output panel
 * @param[in] size Size of the output panel
 * @param[in] texture Texture contain the template of the panel
 * @param[in] texX Position x of the panel template into the texture
 * @param[in] texY Position y of the panel template into the texture
 * @param[in] panelDef Array of seven elements define the panel template used in the texture.
 * From the first to the last: left width, mid width, right width,
 * top height, mid height, bottom height, and margin
 * @todo can we improve the code? is it need?
 */
void MN_DrawPanel (const vec2_t pos, const vec2_t size, const char *texture, int texX, int texY, const int *panelDef)
{
	const int leftWidth = panelDef[0];
	const int midWidth = panelDef[1];
	const int rightWidth = panelDef[2];
	const int topHeight = panelDef[3];
	const int midHeight = panelDef[4];
	const int bottomHeight = panelDef[5];
	const int marge =  panelDef[6];

	/** @todo merge texX and texY here */
	const int firstPos = 0;
	const int secondPos = firstPos + leftWidth + marge;
	const int thirdPos = secondPos + midWidth + marge;
	const int firstPosY = 0;
	const int secondPosY = firstPosY + topHeight + marge;
	const int thirdPosY = secondPosY + midHeight + marge;

	int y, h;

	const image_t *image = MN_LoadImage(texture);
	if (!image)
		return;

	/* draw top (from left to right) */
	MN_DrawNormImage(pos[0], pos[1], leftWidth, topHeight, texX + firstPos + leftWidth, texY + firstPosY + topHeight,
		texX + firstPos, texY + firstPosY, ALIGN_UL, image);
	MN_DrawNormImage(pos[0] + leftWidth, pos[1], size[0] - leftWidth - rightWidth, topHeight, texX + secondPos + midWidth, texY + firstPosY + topHeight,
		texX + secondPos, texY + firstPosY, ALIGN_UL, image);
	MN_DrawNormImage(pos[0] + size[0] - rightWidth, pos[1], rightWidth, topHeight, texX + thirdPos + rightWidth, texY + firstPosY + topHeight,
		texX + thirdPos, texY + firstPosY, ALIGN_UL, image);

	/* draw middle (from left to right) */
	y = pos[1] + topHeight;
	h = size[1] - topHeight - bottomHeight; /* height of middle */
	MN_DrawNormImage(pos[0], y, leftWidth, h, texX + firstPos + leftWidth, texY + secondPosY + midHeight,
		texX + firstPos, texY + secondPosY, ALIGN_UL, image);
	MN_DrawNormImage(pos[0] + leftWidth, y, size[0] - leftWidth - rightWidth, h, texX + secondPos + midWidth, texY + secondPosY + midHeight,
		texX + secondPos, texY + secondPosY, ALIGN_UL, image);
	MN_DrawNormImage(pos[0] + size[0] - rightWidth, y, rightWidth, h, texX + thirdPos + rightWidth, texY + secondPosY + midHeight,
		texX + thirdPos, texY + secondPosY, ALIGN_UL, image);

	/* draw bottom (from left to right) */
	y = pos[1] + size[1] - bottomHeight;
	MN_DrawNormImage(pos[0], y, leftWidth, bottomHeight, texX + firstPos + leftWidth, texY + thirdPosY + bottomHeight,
		texX + firstPos, texY + thirdPosY, ALIGN_UL, image);
	MN_DrawNormImage(pos[0] + leftWidth, y, size[0] - leftWidth - rightWidth, bottomHeight, texX + secondPos + midWidth, texY + thirdPosY + bottomHeight,
		texX + secondPos, texY + thirdPosY, ALIGN_UL, image);
	MN_DrawNormImage(pos[0] + size[0] - bottomHeight, y, rightWidth, bottomHeight, texX + thirdPos + rightWidth, texY + thirdPosY + bottomHeight,
		texX + thirdPos, texY + thirdPosY, ALIGN_UL, image);
}
