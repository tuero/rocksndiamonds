/* 2000-08-20T09:41:18Z
 *
 * identify all emerald mine caves and turn them into v6 format.
 * fixes illegal tiles, acid, wheel, limits times, cleans flags.
 *
 * these tables weed out bad tiles for older caves (eg. wheel on -> wheel off)
 * and clean up v6 caves (acid, number limits) which should(!) be inconsequential,
 * but no doubt it will break some caves.
 */

#include "tile.h"
#include "level.h"


#if defined(TARGET_X11)

static unsigned char remap_v6[256] = { /* filter crap for v6 */
	0,0,2,2,4,4,118,118,8,9,10,11,12,13,14,15,16,16,18,18,20,21,22,23,24,25,26,27,28,
	28,118,28,0,16,2,18,36,37,37,37,40,41,42,43,44,45,128,128,128,148,148,148,45,45,45,
	148,0,57,58,59,60,61,62,63,64,65,66,67,68,69,69,69,69,73,74,75,118,75,75,75,75,75,
	75,75,75,153,153,153,153,153,153,153,153,153,153,153,153,153,153,99,100,68,68,68,
	68,68,68,68,68,118,118,118,118,118,114,115,131,118,118,119,120,121,122,118,118,118,
	118,118,128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,144,145,146,
	147,148,149,150,151,152,153,154,155,156,157,158,159,160,161,162,163,164,165,165,118,
	168,169,170,171,172,173,174,175,176,177,178,179,180,181,182,183,184,185,186,187,188,
	189,68,191,192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,208,209,
	210,211,212,213,214,215,216,217,218,219,220,221,222,223,224,225,226,227,228,229,230,
	231,232,233,234,235,236,237,238,239,240,241,242,243,244,245,153,153,153,153,153,153,
	153,153,153,153
};
static unsigned char remap_v5[256] = { /* filter crap for v5 */
	0,0,2,2,4,4,118,118,8,9,10,11,12,13,14,15,16,16,18,18,20,21,22,23,24,25,26,27,28,
	28,118,28,0,16,2,18,36,37,37,37,147,41,42,43,44,45,128,128,128,148,148,148,45,45,
	45,148,0,57,58,59,60,61,62,63,64,65,66,67,68,153,153,153,153,153,153,153,153,153,
	153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,
	153,153,68,68,68,68,68,68,68,68,118,118,118,118,118,114,115,131,118,118,119,120,121,
	122,118,118,118,118,118,128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,
	143,144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,160,153,153,153,
	153,153,153,118,168,169,170,171,172,173,174,175,176,177,178,179,180,181,182,183,184,
	185,186,187,188,189,68,153,153,153,153,153,153,153,153,153,200,201,202,203,204,205,
	206,207,208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,224,225,226,
	227,228,229,230,231,232,233,234,235,236,237,238,239,240,241,153,153,153,153,153,153,
	153,153,153,153,153,153,153,153
};
static unsigned char remap_v4[256] = { /* filter crap for v4 */
	0,0,2,2,4,4,118,118,8,9,10,11,12,13,14,15,16,16,18,18,20,21,22,23,24,25,26,27,28,
	28,118,28,0,16,2,18,36,37,37,37,147,41,42,43,44,45,128,128,128,148,148,148,45,45,
	45,148,0,153,153,59,60,61,62,63,64,65,66,153,153,153,153,153,153,153,153,153,153,
	153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,
	153,153,153,153,153,153,153,153,153,153,153,153,153,153,153,118,114,115,131,118,118,
	119,120,121,122,118,118,118,118,118,128,129,130,131,132,133,134,135,136,137,138,139,
	140,141,142,143,144,145,146,147,148,149,150,151,152,68,154,155,156,157,158,160,160,
	160,160,160,160,160,160,160,160,160,160,160,160,160,160,175,153,153,153,153,153,153,
	153,153,153,153,153,153,153,153,68,153,153,153,153,153,153,153,153,153,200,201,202,
	203,204,205,206,207,208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,
	224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,240,241,153,153,153,
	153,153,153,153,153,153,153,153,153,153,153
};
static unsigned char remap_v4eater[28] = { /* filter crap for v4 */
	128,18,2,0,4,8,16,20,28,37,41,45,130,129,131,132,133,134,135,136,146,147,175,65,66,
	64,2,18
};

int clean_emerald(unsigned char *src, unsigned long *length)
{
	unsigned int i;
	if(*length >= 2172 && src[2106] == 255 && src[2107] == 54 && src[2108] == 48 && src[2109] == 48) {
		for(i = 0; i < 2048; i++) src[i] = remap_v6[src[i]];
		for(i = 2048; i < 2084; i++) src[i] = remap_v6[src[i]];
		for(i = 2112; i < 2148; i++) src[i] = remap_v6[src[i]];
		goto v6;
	}
	if(*length >= 2110 && src[2106] == 255 && src[2107] == 53 && src[2108] == 48 && src[2109] == 48) {
		for(i = 0; i < 2048; i++) src[i] = remap_v5[src[i]];
		for(i = 2048; i < 2084; i++) src[i] = remap_v5[src[i]];
		for(i = 2112; i < 2148; i++) src[i] = src[i - 64];
		goto v5;
	}
	if(*length >= 2106 && src[1983] == 116) {
		for(i = 0; i < 2048; i++) src[i] = remap_v4[src[i]];
		for(i = 2048; i < 2084; i++) src[i] = remap_v4eater[src[i] >= 28 ? 0 : src[i]];
		for(i = 2112; i < 2148; i++) src[i] = src[i - 64];
		goto v4;
	}
	if(*length >= 2106 && src[0] == 241 && src[1983] == 27) {
		unsigned char j = 94;
		for(i = 0; i < 2106; i++) src[i] = (src[i] ^ (j += 7)) - 0x11;
		src[1] = 131;
		for(i = 0; i < 2048; i++) src[i] = remap_v4[src[i]];
		for(i = 2048; i < 2084; i++) src[i] = remap_v4eater[src[i] >= 28 ? 0 : src[i]];
		for(i = 2112; i < 2148; i++) src[i] = src[i - 64];
		goto v3;
	}
	return(1); /* unrecognized cave */
v3:
v4:
v5:
	src[2106] = 255; src[2107] = 54; src[2108] = 48; src[2109] = 48; /* id */
	i = src[2094] * 10; src[2110] = i >> 8; src[2111] = i; /* time */
	for(i = 2148; i < 2172; i++) src[i] = 0;
	src[2159] = 128; /* ball data */
v6:
	for(i = 0; i < 2048; i++) if(src[i] == 40) break; /* fix wheel */
	for(i++; i < 2048; i++) if(src[i] == 40) src[i] = 147;
	for(i = 64; i < 2048; i++) if(src[i] == 63) src[i - 64] = 101; /* fix acid */
	for(i = 2051; i < 2057; i++) if(src[i] == 63) src[i - 3] = 101; /* fix acid in eater 1 */
	for(i = 2060; i < 2066; i++) if(src[i] == 63) src[i - 3] = 101; /* fix acid in eater 2 */
	for(i = 2069; i < 2075; i++) if(src[i] == 63) src[i - 3] = 101; /* fix acid in eater 3 */
	for(i = 2078; i < 2084; i++) if(src[i] == 63) src[i - 3] = 101; /* fix acid in eater 4 */
	for(i = 2115; i < 2121; i++) if(src[i] == 63) src[i - 3] = 101; /* fix acid in eater 5 */
	for(i = 2124; i < 2130; i++) if(src[i] == 63) src[i - 3] = 101; /* fix acid in eater 6 */
	for(i = 2133; i < 2139; i++) if(src[i] == 63) src[i - 3] = 101; /* fix acid in eater 7 */
	for(i = 2142; i < 2148; i++) if(src[i] == 63) src[i - 3] = 101; /* fix acid in eater 8 */
	src[2094] = 0; /* old style time */
	src[2096] &= 7; src[src[2096] << 8 | src[2097]] = 128; /* player 1 pos */
	src[2098] &= 7; src[src[2098] << 8 | src[2099]] = 128; /* player 2 pos */
	if((src[2100] << 8 | src[2101]) > 9999) { src[2100] = 39; src[2101] = 15; } /* ameuba speed */
	if((src[2102] << 8 | src[2103]) > 9999) { src[2102] = 39; src[2103] = 15; } /* time wonderwall */
	if((src[2110] << 8 | src[2111]) > 9999) { src[2110] = 39; src[2111] = 15; } /* time */
	i = src[2149]; i &= 15; i &= -i; src[2149] = i; /* wind direction */
	if((src[2154] << 8 | src[2155]) > 9999) { src[2154] = 39; src[2155] = 15; } /* time lenses */
	if((src[2156] << 8 | src[2157]) > 9999) { src[2156] = 39; src[2157] = 15; } /* time magnify */
	src[2158] = 0; src[2159] = remap_v6[src[2159]]; /* ball object */
	if((src[2160] << 8 | src[2161]) > 9999) { src[2160] = 39; src[2161] = 15; } /* ball pause */
	src[2162] &= 129; if(src[2162] & 1) src[2163] = 0; /* ball data */
	if((src[2164] << 8 | src[2165]) > 9999) { src[2164] = 39; src[2165] = 15; } /* android move pause */
	if((src[2166] << 8 | src[2167]) > 9999) { src[2166] = 39; src[2167] = 15; } /* android clone pause */
	src[2168] &= 31; /* android data */

	*length = 2172; /* size of v6 cave */
	return(0);
}

/* 2000-07-30T00:26:00Z
 *
 * Read emerald mine caves version 6
 * 
 * v4 and v5 emerald mine caves are converted to v6 (which completely supports older versions)
 * 
 * converting to the internal format loses /significant/ information which can breaks lots of caves.
 * 
 * major incompatibilities:
 * borderless caves behave completely differently, the player no longer "warps" to the other side.
 * a compile time option for spring can make it behave differently when it rolls.
 * a compile time option for rolling objects (stone, nut, spring, bomb) only in eater.
 * acid is always deadly even with no base beneath it.
 *
 * so far all below have not broken any caves:
 *
 * active wheel inside an eater will not function, eater explosions will not change settings.
 * initial collect objects (emerald, diamond, dynamite) dont exist.
 * initial rolling objects will be moved manually and made into sitting objects.
 * drips always appear from dots.
 * more than one thing can fall into acid at the same time.
 * acid explodes when the player walks into it, rather than splashing.
 * simultaneous explosions may be in a slightly different order.
 * quicksand states have been reduced.
 * acid base is effectively an indestructable wall now which can affect eater explosions.
 * android can clone forever with a clone pause of 0 (emeralds, diamonds, nuts, stones, bombs, springs).
 *
 * 2001-03-12T02:46:55Z
 *   rolling stuff is now allowed in the cave, i didn't like making this decision.
 *   if BAD_ROLL is not defined, initial rolling objects are moved by hand.
 *   initial collect objects break some cave in elvis mine 5.
 *   different timing for wonderwall break some cave in exception mine 2.
 *   i think i'm pretty locked into always using the bad roll. *sigh*
 *   rolling spring is now turned into regular spring. it appears the emc editor only uses
 *   the force code for initially moving spring. i will follow this in my editor.
 */

static unsigned short remap_emerald[256] = {
	Xstone, Xstone, Xdiamond, Xdiamond, Xalien, Xalien, Xblank, Xblank,
	Xtank_n, Xtank_e, Xtank_s, Xtank_w, Xtank_gon, Xtank_goe, Xtank_gos, Xtank_gow,
	Xbomb, Xbomb, Xemerald, Xemerald, Xbug_n, Xbug_e, Xbug_s, Xbug_w,
	Xbug_gon, Xbug_goe, Xbug_gos, Xbug_gow, Xdrip_eat, Xdrip_eat, Xdrip_eat, Xdrip_eat,
	Xstone, Xbomb, Xdiamond, Xemerald, Xwonderwall, Xnut, Xnut, Xnut,
	Xwheel, Xeater_n, Xeater_s, Xeater_w, Xeater_e, Xsand_stone, Xblank, Xblank,
	Xblank, Xsand, Xsand, Xsand, Xsand_stone, Xsand_stone, Xsand_stone, Xsand,
	Xstone, Xgrow_ew, Xgrow_ns, Xdynamite_1, Xdynamite_2, Xdynamite_3, Xdynamite_4, Xacid_s,
	Xexit_1, Xexit_2, Xexit_3, Xballoon, Xplant, Xspring, Xspring, Xspring,
	Xspring, Xball_1, Xball_2, Xandroid, Xblank, Xandroid, Xandroid, Xandroid,
	Xandroid, Xandroid, Xandroid, Xandroid, Xandroid, Xblank, Xblank, Xblank,
	Xblank, Xblank, Xblank, Xblank, Xblank, Xblank, Xblank, Xblank,
#ifdef BAD_ROLL
	Xblank, Xblank, Xblank, Xspring_force_w, Xspring_force_e, Xacid_1, Xacid_2, Xacid_3,
	Xacid_4, Xacid_5, Xacid_6, Xacid_7, Xacid_8, Xblank, Xblank, Xblank,
	Xblank, Xblank, Xnut_force_w, Xnut_force_e, Xsteel_1, Xblank, Xblank, Xbomb_force_w,
	Xbomb_force_e, Xstone_force_w, Xstone_force_e, Xblank, Xblank, Xblank, Xblank, Xblank,
#else
	Xblank, Xblank, Xblank, Xspring, Xspring, Xacid_1, Xacid_2, Xacid_3,
	Xacid_4, Xacid_5, Xacid_6, Xacid_7, Xacid_8, Xblank, Xblank, Xblank,
	Xblank, Xblank, Xnut, Xnut, Xsteel_1, Xblank, Xblank, Xbomb,
	Xbomb, Xstone, Xstone, Xblank, Xblank, Xblank, Xblank, Xblank,
#endif
	Xblank, Xround_wall_1, Xgrass, Xsteel_1, Xwall_1, Xkey_1, Xkey_2, Xkey_3,
	Xkey_4, Xdoor_1, Xdoor_2, Xdoor_3, Xdoor_4, Xdripper, Xfake_door_1, Xfake_door_2,
	Xfake_door_3, Xfake_door_4, Xwonderwall, Xwheel, Xsand, Xacid_nw, Xacid_ne, Xacid_sw,
	Xacid_se, Xfake_blank, Xameuba_1, Xameuba_2, Xameuba_3, Xameuba_4, Xexit, Xalpha_arrow_w,
	Xfake_grass, Xlenses, Xmagnify, Xfake_blank, Xfake_grass, Xswitch, Xswitch, Xblank,
	Xdecor_8, Xdecor_9, Xdecor_10, Xdecor_5, Xalpha_comma, Xalpha_quote, Xalpha_minus, Xdynamite,
	Xsteel_3, Xdecor_6, Xdecor_7, Xsteel_2, Xround_wall_2, Xdecor_2, Xdecor_4, Xdecor_3,
	Xwind_nesw, Xwind_e, Xwind_s, Xwind_w, Xwind_n, Xdirt, Xplant, Xkey_5,
	Xkey_6, Xkey_7, Xkey_8, Xdoor_5, Xdoor_6, Xdoor_7, Xdoor_8, Xbumper,
	Xalpha_a, Xalpha_b, Xalpha_c, Xalpha_d, Xalpha_e, Xalpha_f, Xalpha_g, Xalpha_h,
	Xalpha_i, Xalpha_j, Xalpha_k, Xalpha_l, Xalpha_m, Xalpha_n, Xalpha_o, Xalpha_p,
	Xalpha_q, Xalpha_r, Xalpha_s, Xalpha_t, Xalpha_u, Xalpha_v, Xalpha_w, Xalpha_x,
	Xalpha_y, Xalpha_z, Xalpha_0, Xalpha_1, Xalpha_2, Xalpha_3, Xalpha_4, Xalpha_5,
	Xalpha_6, Xalpha_7, Xalpha_8, Xalpha_9, Xalpha_perio, Xalpha_excla, Xalpha_colon, Xalpha_quest,
	Xalpha_arrow_e, Xdecor_1, Xfake_door_5, Xfake_door_6, Xfake_door_7, Xfake_door_8, Xblank, Xblank,
	Xblank, Xblank, Xblank, Xblank, Xblank, Xblank, Xblank, Xblank,
};

void convert_emerald(unsigned char *src)
{
	unsigned int x, y, temp;

	lev.width = 64;
	lev.height = 32;
	temp = ((src[0x83E] << 8 | src[0x83F]) * 25 + 3) / 4; if(temp == 0 || temp > 9999) temp = 9999;
	lev.time = temp;
	lev.required = src[0x82F];

	temp = src[0x830] << 8 | src[0x831];
	ply1.oldx = ply1.x = (temp & 63) + 1;
	ply1.oldy = ply1.y = (temp >> 6 & 31) + 1;
	temp = src[0x832] << 8 | src[0x833];
	ply2.oldx = ply2.x = (temp & 63) + 1;
	ply2.oldy = ply2.y = (temp >> 6 & 31) + 1;

	lev.alien_score = src[0x826];
	temp = (src[0x834] << 8 | src[0x835]) * 28; if(temp > 9999) temp = 9999;
	lev.ameuba_time = temp;
	lev.android_move_cnt = lev.android_move_time = src[0x874] << 8 | src[0x875];
	lev.android_clone_cnt = lev.android_clone_time = src[0x876] << 8 | src[0x877];
	lev.ball_pos = 0;
	lev.ball_random = src[0x872] & 1 ? 1 : 0;
	lev.ball_state = src[0x872] & 128 ? 1 : 0;
	lev.ball_cnt = lev.ball_time = src[0x870] << 8 | src[0x871];
	lev.bug_score = src[0x828];
	lev.diamond_score = src[0x825];
	lev.dynamite_score = src[0x82B];
	lev.eater_pos = 0;
	lev.eater_score = src[0x829];
	lev.emerald_score = src[0x824];
	lev.exit_score = src[0x82D] * 8 / 5;
	lev.key_score = src[0x82C];
	lev.lenses_cnt = 0;
	lev.lenses_score = src[0x867];
	lev.lenses_time = src[0x86A] << 8 | src[0x86B];
	lev.magnify_cnt = 0;
	lev.magnify_score = src[0x868];
	lev.magnify_time = src[0x86C] << 8 | src[0x86D];
	lev.nut_score = src[0x82A];
	lev.shine_cnt = 0;
	lev.slurp_score = src[0x869];
	lev.tank_score = src[0x827];
	lev.wheel_cnt = 0;
	lev.wheel_x = 1;
	lev.wheel_y = 1;
	lev.wheel_time = src[0x838] << 8 | src[0x839];
	lev.wind_cnt = src[0x865] & 15 ? 9999 : 0;
	temp = src[0x865];
	lev.wind_direction = temp & 8 ? 0 : temp & 1 ? 1 : temp & 2 ? 2 : temp & 4 ? 3 : 0;
	lev.wind_time = 9999;
	lev.wonderwall_state = 0;
	lev.wonderwall_time = src[0x836] << 8 | src[0x837];

	for(x = 0; x < 9; x++) lev.eater_array[0][x] = remap_emerald[src[0x800 + x]];
	for(x = 0; x < 9; x++) lev.eater_array[1][x] = remap_emerald[src[0x809 + x]];
	for(x = 0; x < 9; x++) lev.eater_array[2][x] = remap_emerald[src[0x812 + x]];
	for(x = 0; x < 9; x++) lev.eater_array[3][x] = remap_emerald[src[0x81B + x]];
	for(x = 0; x < 9; x++) lev.eater_array[4][x] = remap_emerald[src[0x840 + x]];
	for(x = 0; x < 9; x++) lev.eater_array[5][x] = remap_emerald[src[0x849 + x]];
	for(x = 0; x < 9; x++) lev.eater_array[6][x] = remap_emerald[src[0x852 + x]];
	for(x = 0; x < 9; x++) lev.eater_array[7][x] = remap_emerald[src[0x85B + x]];
	temp = remap_emerald[src[0x86F]];
	for(y = 0; y < 8; y++) {
		if(src[0x872] & 1) {
			for(x = 0; x < 8; x++) lev.ball_array[y][x] = temp;
		} else {
			lev.ball_array[y][1] = (src[0x873] & 1) ? temp : Xblank; /* north */
			lev.ball_array[y][6] = (src[0x873] & 2) ? temp : Xblank; /* south */
			lev.ball_array[y][3] = (src[0x873] & 4) ? temp : Xblank; /* west */
			lev.ball_array[y][4] = (src[0x873] & 8) ? temp : Xblank; /* east */
			lev.ball_array[y][7] = (src[0x873] & 16) ? temp : Xblank; /* south east */
			lev.ball_array[y][5] = (src[0x873] & 32) ? temp : Xblank; /* south west */
			lev.ball_array[y][2] = (src[0x873] & 64) ? temp : Xblank; /* north east */
			lev.ball_array[y][0] = (src[0x873] & 128) ? temp : Xblank; /* north west */
		}
	}
	for(temp = 0; temp < TILE_MAX; temp++) lev.android_array[temp] = Xblank;
	temp = src[0x878] << 8 | src[0x879];
	if(temp & 1) {
		lev.android_array[Xemerald] = lev.android_array[Xemerald_pause] =
		lev.android_array[Xemerald_fall] = lev.android_array[Yemerald_sB] =
		lev.android_array[Yemerald_eB] = lev.android_array[Yemerald_wB] = Xemerald;
	}
	if(temp & 2) {
		lev.android_array[Xdiamond] = lev.android_array[Xdiamond_pause] =
		lev.android_array[Xdiamond_fall] = lev.android_array[Ydiamond_sB] =
		lev.android_array[Ydiamond_eB] = lev.android_array[Ydiamond_wB] = Xdiamond;
	}
	if(temp & 4) {
		lev.android_array[Xstone] = lev.android_array[Xstone_pause] =
		lev.android_array[Xstone_fall] = lev.android_array[Ystone_sB] =
		lev.android_array[Ystone_eB] = lev.android_array[Ystone_wB] = Xstone;
	}
	if(temp & 8) {
		lev.android_array[Xbomb] = lev.android_array[Xbomb_pause] =
		lev.android_array[Xbomb_fall] = lev.android_array[Ybomb_sB] =
		lev.android_array[Ybomb_eB] = lev.android_array[Ybomb_wB] = Xbomb;
	}
	if(temp & 16) {
		lev.android_array[Xnut] = lev.android_array[Xnut_pause] =
		lev.android_array[Xnut_fall] = lev.android_array[Ynut_sB] =
		lev.android_array[Ynut_eB] = lev.android_array[Ynut_wB] = Xnut;
	}
	if(temp & 32) {
		lev.android_array[Xtank_n] = lev.android_array[Xtank_gon] = lev.android_array[Ytank_nB] =
		lev.android_array[Ytank_n_e] = lev.android_array[Ytank_n_w] = Xtank_n;
		lev.android_array[Xtank_e] = lev.android_array[Xtank_goe] = lev.android_array[Ytank_eB] =
		lev.android_array[Ytank_e_s] = lev.android_array[Ytank_e_n] = Xtank_e;
		lev.android_array[Xtank_s] = lev.android_array[Xtank_gos] = lev.android_array[Ytank_sB] =
		lev.android_array[Ytank_s_w] = lev.android_array[Ytank_s_e] = Xtank_s;
		lev.android_array[Xtank_w] = lev.android_array[Xtank_gow] = lev.android_array[Ytank_wB] =
		lev.android_array[Ytank_w_n] = lev.android_array[Ytank_w_s] = Xtank_w;
	}
	if(temp & 64) {
		lev.android_array[Xeater_n] = lev.android_array[Yeater_nB] = Xeater_n;
		lev.android_array[Xeater_e] = lev.android_array[Yeater_eB] = Xeater_e;
		lev.android_array[Xeater_s] = lev.android_array[Yeater_sB] = Xeater_s;
		lev.android_array[Xeater_w] = lev.android_array[Yeater_wB] = Xeater_w;
	}
	if(temp & 128) {
		lev.android_array[Xbug_n] = lev.android_array[Xbug_gon] = lev.android_array[Ybug_nB] =
		lev.android_array[Ybug_n_e] = lev.android_array[Ybug_n_w] = Xbug_gon;
		lev.android_array[Xbug_e] = lev.android_array[Xbug_goe] = lev.android_array[Ybug_eB] =
		lev.android_array[Ybug_e_s] = lev.android_array[Ybug_e_n] = Xbug_goe;
		lev.android_array[Xbug_s] = lev.android_array[Xbug_gos] = lev.android_array[Ybug_sB] =
		lev.android_array[Ybug_s_w] = lev.android_array[Ybug_s_e] = Xbug_gos;
		lev.android_array[Xbug_w] = lev.android_array[Xbug_gow] = lev.android_array[Ybug_wB] =
		lev.android_array[Ybug_w_n] = lev.android_array[Ybug_w_s] = Xbug_gow;
	}
	if(temp & 256) {
		lev.android_array[Xalien] = lev.android_array[Xalien_pause] =
		lev.android_array[Yalien_nB] = lev.android_array[Yalien_eB] =
		lev.android_array[Yalien_sB] = lev.android_array[Yalien_wB] = Xalien;
	}
	if(temp & 512) {
		lev.android_array[Xspring] = lev.android_array[Xspring_pause] =
		lev.android_array[Xspring_e] = lev.android_array[Yspring_eB] = lev.android_array[Yspring_kill_eB] =
		lev.android_array[Xspring_w] = lev.android_array[Yspring_wB] = lev.android_array[Yspring_kill_wB] =
		lev.android_array[Xspring_fall] = lev.android_array[Yspring_sB] = Xspring;
	}
	if(temp & 1024) {
		lev.android_array[Yballoon_nB] = lev.android_array[Yballoon_eB] =
		lev.android_array[Yballoon_sB] = lev.android_array[Yballoon_wB] =
		lev.android_array[Xballoon] = Xballoon;
	}
	if(temp & 2048) {
		lev.android_array[Xdripper] = lev.android_array[XdripperB] =
		lev.android_array[Xameuba_1] = lev.android_array[Xameuba_2] =
		lev.android_array[Xameuba_3] = lev.android_array[Xameuba_4] =
		lev.android_array[Xameuba_5] = lev.android_array[Xameuba_6] =
		lev.android_array[Xameuba_7] = lev.android_array[Xameuba_8] = Xdrip_eat;
	}
	if(temp & 4096) {
		lev.android_array[Xdynamite] = Xdynamite;
	}

	for(temp = 1; temp < 2047; temp++) {
		switch(src[temp]) {
		case 0x24: lev.wonderwall_state = 1; lev.wonderwall_time = 9999; break; /* wonderwall */
		case 0x28: lev.wheel_x = temp & 63; lev.wheel_y = temp >> 6; lev.wheel_cnt = lev.wheel_time; break; /* wheel */
#ifndef BAD_ROLL
		case 0x63: src[temp - 1] = 0x45; src[temp] = 0x80; break; /* spring roll left */
		case 0x64: src[temp + 1] = 0x45; src[temp] = 0x80; break; /* spring roll right */
		case 0x72: src[temp - 1] = 0x25; src[temp] = 0x80; break; /* nut roll left */
		case 0x73: src[temp + 1] = 0x25; src[temp] = 0x80; break; /* nut roll right */
		case 0x77: src[temp - 1] = 0x10; src[temp] = 0x80; break; /* bomb roll left */
		case 0x78: src[temp + 1] = 0x10; src[temp] = 0x80; break; /* bomb roll right */
		case 0x79: src[temp - 1] = 0x00; src[temp] = 0x80; break; /* stone roll left */
		case 0x7A: src[temp + 1] = 0x00; src[temp] = 0x80; break; /* stone roll right */
#endif
		case 0xA3: lev.lenses_cnt = 9999; break; /* fake blank */
		case 0xA4: lev.magnify_cnt = 9999; break; /* fake grass */
		}
	}
	for(y = 0; y < HEIGHT; y++) for(x = 0; x < WIDTH; x++) Cave[y][x] = ZBORDER;
	temp = 0; for(y = 0; y < lev.height; y++) for(x = 0; x < lev.width; x++) {
		Cave[y+1][x+1] = remap_emerald[src[temp++]];
	}
	if(ply1.alive) Cave[ply1.y][ply1.x] = Zplayer;
	if(ply2.alive) Cave[ply2.y][ply2.x] = Zplayer;
	for(y = 0; y < HEIGHT; y++) for(x = 0; x < WIDTH; x++) Next[y][x] = Cave[y][x];
	for(y = 0; y < HEIGHT; y++) for(x = 0; x < WIDTH; x++) Draw[y][x] = Cave[y][x];
}

#endif
