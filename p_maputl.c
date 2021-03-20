// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This source is available for distribution and/or modification
// only under the terms of the DOOM Source Code License as
// published by id Software. All rights reserved.
//
// The source is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// FITNESS FOR A PARTICULAR PURPOSE. See the DOOM Source Code License
// for more details.
//
// $Log:$
//
// DESCRIPTION:
//	Movement/collision utility functions,
//	as used by function in p_map.c. 
//	BLOCKMAP Iterator functions,
//	and some PIT_* functions to use for iteration.
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id: p_maputl.c,v 1.5 1997/02/03 22:45:11 b1 Exp $";


#include <stdlib.h>


#include "m_bbox.h"

#include "doomdef.h"
#include "p_local.h"


// State.
#include "r_state.h"

//
// P_AproxDistance
// Gives an estimation of distance (not exact)
//

fixed_t
P_AproxDistance
( fixed_t	dx,
  fixed_t	dy )
{
    dx = abs(dx);
    dy = abs(dy);
    if (dx < dy)
	return dx+dy-(dx>>1);
    return dx+dy-(dy>>1);
}


//
// P_PointOnLineSide
// Returns 0 or 1
//
int
P_PointOnLineSide
( fixed_t	x,
  fixed_t	y,
  line_t*	line )
{
    fixed_t	dx;
    fixed_t	dy;
    fixed_t	left;
    fixed_t	right;
	
    if (!line->dx)
    {
	if (x <= line->v1->x)
	    return line->dy > 0;
	
	return line->dy < 0;
    }
    if (!line->dy)
    {
	if (y <= line->v1->y)
	    return line->dx < 0;
	
	return line->dx > 0;
    }
	
    dx = (x - line->v1->x);
    dy = (y - line->v1->y);
	
    left = FixedMul ( line->dy>>FRACBITS , dx );
    right = FixedMul ( dy , line->dx>>FRACBITS );
	
    if (right < left)
	return 0;		// front side
    return 1;			// back side
}



//
// P_BoxOnLineSide
// Considers the line to be infinite
// Returns side 0 or 1, -1 if box crosses the line.
//
int
P_BoxOnLineSide
( fixed_t*	tmbox,
  line_t*	ld )
{
    int		p1;
    int		p2;
	
    switch (ld->slopetype)
    {
      case ST_HORIZONTAL:
	p1 = tmbox[BOXTOP] > ld->v1->y;
	p2 = tmbox[BOXBOTTOM] > ld->v1->y;
	if (ld->dx < 0)
	{
	    p1 ^= 1;
	    p2 ^= 1;
	}
	break;
	
      case ST_VERTICAL:
	p1 = tmbox[BOXRIGHT] < ld->v1->x;
	p2 = tmbox[BOXLEFT] < ld->v1->x;
	if (ld->dy < 0)
	{
	    p1 ^= 1;
	    p2 ^= 1;
	}
	break;
	
      case ST_POSITIVE:
	p1 = P_PointOnLineSide (tmbox[BOXLEFT], tmbox[BOXTOP], ld);
	p2 = P_PointOnLineSide (tmbox[BOXRIGHT], tmbox[BOXBOTTOM], ld);
	break;
	
      case ST_NEGATIVE:
	p1 = P_PointOnLineSide (tmbox[BOXRIGHT], tmbox[BOXTOP], ld);
	p2 = P_PointOnLineSide (tmbox[BOXLEFT], tmbox[BOXBOTTOM], ld);
	break;
    }

    if (p1 == p2)
	return p1;
    return -1;
}


//
// P_PointOnDivlineSide
// Returns 0 or 1.
//
int
P_PointOnDivlineSide
( fixed_t	x,
  fixed_t	y,
  divline_t*	line )
{
    fixed_t	dx;
    fixed_t	dy;
    fixed_t	left;
    fixed_t	right;
	
    if (!line->dx)
    {
	if (x <= line->x)
	    return line->dy > 0;
	
	return line->dy < 0;
    }
    if (!line->dy)
    {
	if (y <= line->y)
	    return line->dx < 0;

	return line->dx > 0;
    }
	
    dx = (x - line->x);
    dy = (y - line->y);
	
    // try to quickly decide by looking at sign bits
    if ( (line->dy ^ line->dx ^ dx ^ dy)&0x80000000 )
    {
	if ( (line->dy ^ dx) & 0x80000000 )
	    return 1;		// (left is negative)
	return 0;
    }
	
    left = FixedMul ( line->dy>>8, dx>>8 );
    right = FixedMul ( dy>>8 , line->dx>>8 );
	
    if (right < left)
	return 0;		// front side
    return 1;			// back side
}



//
// P_MakeDivline
//
void
P_MakeDivline
( line_t*	li,
  divline_t*	dl )
{
    dl->x = li->v1->x;
    dl->y = li->v1->y;
    dl->dx = li->dx;
    dl->dy = li->dy;
}



//
// P_InterceptVector
// Returns the fractional intercept point
// along the first divline.
// This is only called by the addthings
// and addlines traversers.
//
fixed_t
P_InterceptVector
( divline_t*	v2,
  divline_t*	v1 )
{
#if 1
    fixed_t	frac;
    fixed_t	num;
    fixed_t	den;
	
    den = FixedMul (v1->dy>>8,v2->dx) - FixedMul(v1->dx>>8,v2->dy);

    if (den == 0)
	return 0;
    //	I_Error ("P_InterceptVector: parallel");
    
    num =
	FixedMul ( (v1->x - v2->x)>>8 ,v1->dy )
	+FixedMul ( (v2->y - v1->y)>>8, v1->dx );

    frac = FixedDiv (num , den);

    return frac;
#else	// UNUSED, float debug.
    float	frac;
    float	num;
    float	den;
    float	v1x;
    float	v1y;
    float	v1dx;
    float	v1dy;
    float	v2x;
    float	v2y;
    float	v2dx;
    float	v2dy;

    v1x = (float)v1->x/FRACUNIT;
    v1y = (float)v1->y/FRACUNIT;
    v1dx = (float)v1->dx/FRACUNIT;
    v1dy = (float)v1->dy/FRACUNIT;
    v2x = (float)v2->x/FRACUNIT;
    v2y = (float)v2->y/FRACUNIT;
    v2dx = (float)v2->dx/FRACUNIT;
    v2dy = (float)v2->dy/FRACUNIT;
	
    den = v1dy*v2dx - v1dx*v2dy;

    if (den == 0)
	return 0;	// parallel
    
    num = (v1x - v2x)*v1dy + (v2y - v1y)*v1dx;
    frac = num / den;

    return frac*FRACUNIT;
#endif
}


//
// P_LineOpening
// Sets opentop and openbottom to the window
// through a two sided line.
// OPTIMIZE: keep this precalculated
//
fixed_t opentop;
fixed_t openbottom;
fixed_t openrange;
fixed_t	lowfloor;


void P_LineOpening (line_t* linedef)
{
    sector_t*	front;
    sector_t*	back;
	
    if (linedef->sidenum[1] == -1)
    {
	// single sided line
	openrange = 0;
	return;
    }
	 
    front = linedef->frontsector;
    back = linedef->backsector;
	
    if (front->ceilingheight < back->ceilingheight)
	opentop = front->ceilingheight;
    else
	opentop = back->ceilingheight;

    if (front->floorheight > back->floorheight)
    {
	openbottom = front->floorheight;
	lowfloor = back->floorheight;
    }
    else
    {
	openbottom = back->floorheight;
	lowfloor = front->floorheight;
    }
	
    openrange = opentop - openbottom;
}


//
// THING POSITION SETTING
//


//
// P_UnsetThingPosition
// Unlinks a thing from block map and sectors.
// On each position change, BLOCKMAP and other
// lookups maintaining lists ot things inside
// these structures need to be updated.
//
void P_UnsetThingPosition (mobj_t* thing)
{
    int		blockx;
    int		blocky;

    if ( ! (thing->flags & MF_NOSECTOR) )
    {
	// inert things don't need to be in blockmap?
	// unlink from subsector
	if (thing->snext)
	    thing->snext->sprev = thing->sprev;

	if (thing->sprev)
	    thing->sprev->snext = thing->snext;
	else
	    thing->subsector->sector->thinglist = thing->snext;
    }
	
    if ( ! (thing->flags & MF_NOBLOCKMAP) )
    {
	// inert things don't need to be in blockmap
	// unlink from block map
	if (thing->bnext)
	    thing->bnext->bprev = thing->bprev;
	
	if (thing->bprev)
	    thing->bprev->bnext = thing->bnext;
	else
	{
	    blockx = (thing->x - bmaporgx)>>MAPBLOCKSHIFT;
	    blocky = (thing->y - bmaporgy)>>MAPBLOCKSHIFT;

	    if (blockx>=0 && blockx < bmapwidth
		&& blocky>=0 && blocky <bmapheight)
	    {
		blocklinks[blocky*bmapwidth+blockx] = thing->bnext;
	    }
	}
    }
}


//
// P_SetThingPosition
// Links a thing into both a block and a subsector
// based on it's x y.
// Sets thing->subsector properly
//
void
P_SetThingPosition (mobj_t* thing)
{
    subsector_t*	ss;
    sector_t*		sec;
    int			blockx;
    int			blocky;
    mobj_t**		link;

    
    // link into subsector
    ss = R_PointInSubsector (thing->x,thing->y);
    thing->subsector = ss;
    
    if ( ! (thing->flags & MF_NOSECTOR) )
    {
	// invisible things don't go into the sector links
	sec = ss->sector;
	
	thing->sprev = NULL;
	thing->snext = sec->thinglist;

	if (sec->thinglist)
	    sec->thinglist->sprev = thing;

	sec->thinglist = thing;
    }

    
    // link into blockmap
    if ( ! (thing->flags & MF_NOBLOCKMAP) )
    {
	// inert things don't need to be in blockmap		
	blockx = (thing->x - bmaporgx)>>MAPBLOCKSHIFT;
	blocky = (thing->y - bmaporgy)>>MAPBLOCKSHIFT;

	if (blockx>=0
	    && blockx < bmapwidth
	    && blocky>=0
	    && blocky < bmapheight)
	{
	    link = &blocklinks[blocky*bmapwidth+blockx];
	    thing->bprev = NULL;
	    thing->bnext = *link;
	    if (*link)
		(*link)->bprev = t  �DA.�%W���  ��  2 �,4@RPrv	j@���PA|h
>��=?�H�7�}�a�"��i���0����A�d���Ú��֛U�  ��  ��  ��   ����  ��O�À  pp  pp  pp  � (@a4@�	 � ˀd ��i4 C	���e�膀�I�ԕ�O�%yV>P\��(�&41������Rh�tBr�b�M! gA)��me"�@��� :V`ht�h 2H" `�$�N��p����X`h��0�C5������Lp�T@;O!��[�  =��� �g��x�=՘�n`�Bh� P��I4[� �2@B� ��Y:���l쵱�z®`!�M=a`v�̢br�I��`�4Z6�� ��	��)�a����Of��A���2�b )��b���l �0`I =G�j-@?��d.���M��Y��40�W�@�j ���Dm�9�i��h�@�@4X! Gp�xR�g!:��	�����Qۋ�� �,�@
�Rh�'���f&d�CP
!�S�!�)E�����(�T�� d쀐B x�X��1�H� ~~A0
� ��T�^Ae�FP��߄6�  b�* �Gp0X�$s�@; {��e����I`	� � z .�0���`�@ ��e"`�
�'NJu�JV�@��A���; \ ؘ�� `W(o&��+�@@-,�`Ű�	�����ĭ����@���_`L?\� n�C R�@Np�� f ���7%��P����&����?@L	0 �!`������,3$8L ��@@�����$�j�>+'�!��M��&� 3`� ��o�JZG@���CP���S�j�ſf��}`���3 ^RP }�C]%�x@�Id����Ae#񛰰tHN��L '!�����|8 ��h@�eX���:� �x��� R	� b� +O�� �v�-ޝ���a�(7֐����O
�&�@;��M�oL�W\���Z��%�4H�q����+o�D�=�><q>�0�ay�a�������0���}��j��!���b_���,.��i͸�4��.{n6o�)v�n�A��l��  ?��'J���P? �� ���9XC� @L�`ii��j�����"`c����3���rL1,�$lwH�0p���hJ@CK�CrY�XI�%���g$��	b�٘�����@��]#O�b����1gIy�~��!�\�g]Ztѻ�������`0��PgC�0sD��
��t�&��n+-�U�%��$m�.Nh���pj1\����B��<���% ��� %f�&$�P�� p���%%'q13��=�����5�Yl&��S�R�>u3C����5����djRz�o�&ۘȇ�'-%�>6#D��DC윀&�ä�ף�I7��d�SG��6���ܯucƐ����$�2J�˻&%fřl>���cl&�1��� ~� � ������[���� ; ̵�ݱX��9L�㱾: HL ��@�L �4�E�/�ro<�1_���2I1�VL p �0 D 
� ��L&C/�i����$���� �Ua�A�(,[��B��B�$"CHY<���vT R ��E ( ��dɀ3zCZ�,{%{(�_@�� � ;��r��G�8P	 ��3�d@t_&���~���dҜ	���<�R�� op1�׺ ~ �
�. b@ ��h��J��:��  䚔�X�s�6�C�� �&k>a}��{!��!H%!���Aa�����bj8��ņfN�
�%Q�Z��-�~�9�2N��  �DA.Ͷ���  ��  #o���4Y)5�>Ҙ�
Ad������B�)<�V�&�1\�*��t���XhhbЁ��h�5���c�g%�G�/��9���(���^a| xy�(�����s��
+���E�0y.P�=)^誆a�r0=�)Jx����E#l�q���?�ǡ�cb�Pʝ����ZUG�
Q��s��Q��I�&NCRO�wp�Fç�Rpخ�7�J����|�ʒ5��јP�@�����U�I
�2Y��.�{�䴦�?�Қ=R���@  �0 �4 tr,���Y��'�� (0@�`$
 dH@;I/΅�	X�@ �H��H@`S�<�H `���	� +�8fK���7�X�f$�C�@�JA� c��|Iei�*B��V&�-(`������V�y\���Qly��C2���-b�`�|}�G�p�!Қ)����ߺy9PO@J�x�fd��������9�S������%�q��<.��G��5'?�ϑ�J�{�,��� �-_��<�Z���z��729��P�0& �h�h�)���v탁����|��E���pW$}$���n�
O��F�0C�l���t�h�כ����¥�PN]XW `	�H � T5��d$�_�p8�/�1#Rp�#�1!���	�0&� tYI�0��;+�r��S�nt�!<?�F��FP
�D�� �օE�rtXt����t���e3h�%���M9��^��YW , r�2 �E��X�����HR����T �PP| ��� ���1Eb��Q��E����*KOw�2�3�� �H(T���!�h! :a�D���U�$0��x7�K���x�5W�2ǆ2\9�y��A/�v�܍d5�7�G|-���b��������X�(���1��Ɍ��Į�!����,�ue��D���Pi<�$�eEb%���z�%��U�a��!ӎ�w��A6�@� $.Ps����XB;c���)s5�1Lu��aqj�B��	#G��wm3�\�Y��AU��I����u�1�@  �����3@;&��o&7Ƿ8Q"ɀ1+�Fr����	?�G��2`!�.�3t�,%q+�`����8qqE,�Wi�[ {x����1�8�/';�qIF�;q����e���/3,΀�ݒy#������ؔ����r]�n4Չy�ϓyhpу:r�RG��b�5	Ř�8y �n?��xEV���g�洉w�D�H�p��D ��$3�F%$�&��>�b@�+`0?���D�Ŗ���:��M�Y��u��<M, 4�7�#n)�B	A(�%�Ŗaa����M%�@]��M�Ig���FN�p����#��e���ʷ�P�I�`w�z �C�U��0��'[?�μ�7�A�j�������H�"�p�x��e����,ZΒP�p��`%aT��0 zL!`*�� �&�H,�p{D����a`&&����$�XiH$g1�PC�0�H`;!�G( ��d�����@x�@�p�K�Z��Rp�*�ZL�ā� 3xvQ,���lP�;!��0�+�gq�� f�jq5����@T �RR���0Z�,1Lp�|���Yk�'ZZKYiJ�@�I�n_Q�J	@1,����|P��x��	�z�HvG@TCJ�~y�m0A
���@RV(05 ��_�G� ط�o�� �	�C@����p	����[���%�܄���c:�g
u���� �T���]$�:3�(�Pέ��d�&���a0hhoA ���rjI�s�tY��a�!�;���!���^N�`͜a�N�~`IHQh��@�(z��kP�8V��F*J8���%��n��;+�a�;�|����2AD���x
�x0?��+�g��c��QHY4�=��&��f  �DA.�LU���  ��  G�x�R@�s!��V��{H���.���t���a���je�?��ǑG���c��Z�v|i��� qs8��]�l�|Z#Lh^^�H	�D��w��"g�v���][HNh�SM�  ?���I��F (4��߰� �`C۝����l�K�PK�������8�vr$�iqJ��(���¸Uґ���Yn�Df�w��\Ą�;�>�I�ʑ����GRS�W�t���r�l"�w���|D��m�$����X���4�8�
�JH�P�l�����Y�7; ��t��@I�?P�(P�N���3��{0s����9H!�rk'�o�sOZ��{Ba��`nb� �7$����($�!%�p� �Y>&���� �aE��%pD��)�n���b��$��' ��h`���@��o�	SU��:��aB!���_/A�>�6�0! 4��l����9���;����hƃzs�<:�B)u��H���?��Yȳ�ox����aL��'�|��;,7� �VBvK`
A#�?�� ",��8,�3���h ���z�nD�a�)&�|��3IG��T�[rX�LiL,�7� P�?{(� ~M&�����ٍP�.�>-,��b�ͩ8���MߠYŘ`���g)��̔�!ǐ$��(�zq+�����|�i �$���$�ĥ��1�` ~��� �P��h ���	�Fp�ԝ���4��Ē���T�J/ f�a1 T0��ސҲ�b=RHE`��<�!�� �G�K�&Paa��nD]i@@vK� �e��,  ��XA��n��� $�'B �d��?��]�`*��z��	��
%��7�����zAl�9����7�C9O��p�x((�~[��CI,�]�z=�~��0����Y�l�N������I/�!~X�h}%g�*Й���'l�����&l#e����\�'��CKN8���J��o0f&�
�:@r�s�¸�h���D��F�0g@��P����&INm���v�P�G�]���v��U����c=Zg1���!4�)�6wa[�㱐��I��w���K�WC�O~U��I:8�w�[�գ)F  	��^ t J ���@��d �h(��,	� j �8�`0�(M:J/�{wT �L� �ہD������e�   / *&����P T`P1(G��K��2�O�0�" zY(�@	d��N'X�\0�����D����t�@�b"��q>C7�y.J}�W���J�hL!���m�0�0�`贖�_��B�� B ����J<Mr�j^�?}4�}��Ni!�\���K'��j�|w:���Y<_Bb���%!}��<sI!���U�2��0�/��y����u�؍��~�?,z�F�4CAEr�	$'�@%I@=;�$C9�l0`Ag��K�6䜔� P�&�B�)<�B��h��`�p	���p(��c�;�0=�|8 �d�8������D���`�f�L�X��$�����>�P�-A(tn�mU��A���fƓO��RX�4�0p�B���:��)k�;��Ϯ�1�K��n��Ƀ�}F�ں&�_��.�V��V���?�DQ4
 rВ��u�7�RƲ�4�\ +�v�A�0�4���0�!�pP�@��,�X�ih$-,_Ŝa���"�+�h0`C9� ���{�&������c���OG�Н�����e�ф�,����C!��
dpd����y�>!�ZrR���e�Z�D��J��)��y;[(Y{���8:(5$�`�p}��HJ�уC�!�W����Ʉ0 5&�:&�Vb��h�k)$ҿ	�Y� }�����P@:(�I�JIX�������)�3w�LI��YD�C��r��;+  �DA.�����  ��  �ib3�:I3��{m�ݼ^BGm���aq� 5A�`�	�bh@\Ƨ���̎S ��fp�� ),��� �4@:�E�f�VYKe	� >�!�0�2 Zd� �܂d����� �4���nLNv� PB	4�M,0
����Q�!p/�?�Cg:�Aˀ5�T ұ�i�	� �q���;�<�����dčl�*�$>�F�l���� ����kA5bX����
�Hi0�N�7�@�L��5%�4��c5ʘ�j2\�V>�Iv��zn�W��wT����U���U�O�KfvQZ΀ = F ��@v�c�b>��/N�04���� ��T]� ���ŕ�s` �� &I`6�:&�t$`n%%\Z�>uu[��0��Z�gz,��R�,���H�e�r�45"��á��e�-�}���μ����2�  
>��b@t���:�� �@A� ��$�=�u�� 
@ � �� ���?v@@ 3 L �B���
�`�~e�e����& ͏����.�`
S�E�?� �9``7��@��񸾄�j�x}p��w,���V��s�b�Qӆe��p�?��!'�뚾�& h0�0��@2��*�. l�(� �N �! $	)�!�z��a88���`�����M��P|����T7��0�B/ၝ= �� 6$��Vo�"Y�_JX5�7s�Bm��ܖ���a�3p�Jɉ��I-,BP"�� ��gGvӹ�oX�?�����0_,��+l��Y�KG&�2������q��kq�;&�ܔ���N@h���Ɩ�0VI��l0b
�e�B�V`Q} \[�@�G?��"%�%��*�K@,�'�B��ha_�)"|�&�R_�`��P)�C1C~VsZB��^� �À�ц�0J\���V� ���ؚ�8O�wPG��p
�tKC3�x�Pm��:O��2�օ�᷆��D����O�E�l�v�^eV��o%�_;�F�Kg�aL���Q 7 (Xp�0�@XT��1Hh� >�a�WHb0 Ș�xi: ��:ŖW����d"��Ŗ���̥��@��X ��Z�֒ٙ�rQD�	g��%V�&D���h0o% ��O��(QD̅d���iC7����E���ʦU |�죊�I���Hf-ni�)�-� K��+�6(����q�c,�e���;��0�4����(	i�	d�>I+�a���h�&@�nO�S�3�0Af��@� �2� �a��Y���HũV._����)C��5�XM�T���a�u���x|4��%c���@�C�ZK��� � 0&P(��JI������4``�:��L�J������@`���!#F������@ B������k���(0�b��M�l���z�@рU9��d�Ŋ� \A���BL��ۊ^�3y%t�19YCf�7(07|����1,�& �@�C��* �Q�'S��tL���>+�tK%`	��GO�]���X�"�X`�[�"�umJLteMvqo}�Ԫ�n�N�� @ �y` tR tL�T��`� ��JT�"> -8@�@� br�`1) �?�0�ɩ	�3� TC�& )0�L�`q�>R<j��  ��@4 �ye��� �3�r��l%{-R �����0d0���^CH�k$