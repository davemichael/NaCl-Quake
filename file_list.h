// This is a list of files to download to play quake. Note that the first bunch
// are roughly ordered based on what Quake reads first on startup, and there
// may be duplicates because I'm too lazy to cull them right now. But the
// FileManager code doesn't 'refetch' if Fetch is called twice for the same
// file name, so there's little harm in the duplicates.
const char* file_list[] = {
"id1/config.cfg",
"id1/gfx.wad",
"id1/gfx/palette.lmp",
"id1/gfx/colormap.lmp",
"id1/gfx/complete.lmp",
"id1/sound/ambience/water1.wav",
"id1/sound/ambience/wind2.wav",
"id1/sound/wizard/hit.wav",
"id1/sound/hknight/hit.wav",
"id1/sound/weapons/tink1.wav",
"id1/sound/weapons/ric1.wav",
"id1/sound/weapons/ric2.wav",
"id1/sound/weapons/ric3.wav",
"id1/sound/weapons/r_exp3.wav",
"id1/quake.rc",
"id1/default.cfg",
"id1/demo1.dem",
"id1/maps/e1m3.bsp",
"id1/progs/player.mdl",
"id1/progs/eyes.mdl",
"id1/progs/h_player.mdl",
"id1/progs/gib1.mdl",
"id1/progs/gib2.mdl",
"id1/progs/gib3.mdl",
"id1/progs/s_bubble.spr",
"id1/progs/s_explod.spr",
"id1/progs/v_axe.mdl",
"id1/progs/v_shot.mdl",
"id1/progs/v_nail.mdl",
"id1/progs/v_rock.mdl",
"id1/progs/v_shot2.mdl",
"id1/progs/v_nail2.mdl",
"id1/progs/v_rock2.mdl",
"id1/progs/bolt.mdl",
"id1/progs/bolt2.mdl",
"id1/progs/bolt3.mdl",
"id1/progs/lavaball.mdl",
"id1/progs/missile.mdl",
"id1/progs/grenade.mdl",
"id1/progs/spike.mdl",
"id1/progs/s_spike.mdl",
"id1/progs/backpack.mdl",
"id1/progs/zom_gib.mdl",
"id1/progs/v_light.mdl",
"id1/progs/s_light.spr",
"id1/progs/flame.mdl",
"id1/progs/flame2.mdl",
"id1/progs/zombie.mdl",
"id1/progs/h_zombie.mdl",
"id1/progs/w_g_key.mdl",
"id1/progs/ogre.mdl",
"id1/progs/h_ogre.mdl",
"id1/progs/demon.mdl",
"id1/progs/h_demon.mdl",
"id1/progs/wizard.mdl",
"id1/progs/h_wizard.mdl",
"id1/progs/w_spike.mdl",
"id1/progs/g_nail.mdl",
"id1/maps/b_shell1.bsp",
"id1/maps/b_bh25.bsp",
"id1/maps/b_bh10.bsp",
"id1/maps/b_bh100.bsp",
"id1/progs/g_shot.mdl",
"id1/maps/b_shell0.bsp",
"id1/maps/b_rock0.bsp",
"id1/maps/b_nail1.bsp",
"id1/maps/b_nail0.bsp",
"id1/progs/armor.mdl",
"id1/maps/b_rock1.bsp",
"id1/progs/g_rock.mdl",
"id1/progs/invisibl.mdl",
"id1/sound/weapons/rocket1i.wav",
"id1/sound/weapons/sgun1.wav",
"id1/sound/weapons/guncock.wav",
"id1/sound/weapons/spike2.wav",
"id1/sound/weapons/grenade.wav",
"id1/sound/weapons/bounce.wav",
"id1/sound/weapons/shotgn2.wav",
"id1/sound/demon/dland2.wav",
"id1/sound/misc/h2ohit1.wav",
"id1/sound/items/itembk2.wav",
"id1/sound/player/plyrjmp8.wav",
"id1/sound/player/land.wav",
"id1/sound/player/land2.wav",
"id1/sound/player/drown1.wav",
"id1/sound/player/drown2.wav",
"id1/sound/player/gasp1.wav",
"id1/sound/player/gasp2.wav",
"id1/sound/player/h2odeath.wav",
"id1/sound/misc/talk.wav",
"id1/sound/player/teledth1.wav",
"id1/sound/misc/r_tele1.wav",
"id1/sound/misc/r_tele2.wav",
"id1/sound/misc/r_tele3.wav",
"id1/sound/misc/r_tele4.wav",
"id1/sound/misc/r_tele5.wav",
"id1/sound/weapons/lock4.wav",
"id1/sound/weapons/pkup.wav",
"id1/sound/items/armor1.wav",
"id1/sound/weapons/lhit.wav",
"id1/sound/player/gib.wav",
"id1/sound/player/udeath.wav",
"id1/sound/player/tornoff2.wav",
"id1/sound/player/pain1.wav",
"id1/sound/player/pain2.wav",
"id1/sound/player/pain3.wav",
"id1/sound/player/pain4.wav",
"id1/sound/player/pain5.wav",
"id1/sound/player/pain6.wav",
"id1/sound/player/death1.wav",
"id1/sound/player/death2.wav",
"id1/sound/player/death3.wav",
"id1/sound/player/death4.wav",
"id1/sound/player/death5.wav",
"id1/sound/weapons/ax1.wav",
"id1/sound/player/axhit1.wav",
"id1/sound/player/axhit2.wav",
"id1/sound/player/h2ojump.wav",
"id1/sound/player/slimbrn2.wav",
"id1/sound/player/inh2o.wav",
"id1/sound/player/inlava.wav",
"id1/sound/misc/outwater.wav",
"id1/sound/player/lburn1.wav",
"id1/sound/player/lburn2.wav",
"id1/sound/misc/water1.wav",
"id1/sound/misc/water2.wav",
"id1/sound/doors/medtry.wav",
"id1/sound/doors/meduse.wav",
"id1/sound/doors/stndr1.wav",
"id1/sound/doors/stndr2.wav",
"id1/sound/misc/null.wav",
"id1/sound/plats/train2.wav",
"id1/sound/plats/train1.wav",
"id1/sound/buttons/switch02.wav",
"id1/sound/doors/drclos4.wav",
"id1/sound/doors/doormv1.wav",
"id1/sound/ambience/fire1.wav",
"id1/sound/doors/basesec1.wav",
"id1/sound/doors/basesec2.wav",
"id1/sound/doors/latch2.wav",
"id1/sound/doors/winch2.wav",
"id1/sound/zombie/z_idle.wav",
"id1/sound/zombie/z_idle1.wav",
"id1/sound/zombie/z_shot1.wav",
"id1/sound/zombie/z_gib.wav",
"id1/sound/zombie/z_pain.wav",
"id1/sound/zombie/z_pain1.wav",
"id1/sound/zombie/z_fall.wav",
"id1/sound/zombie/z_miss.wav",
"id1/sound/zombie/z_hit.wav",
"id1/sound/zombie/idle_w2.wav",
"id1/sound/plats/medplat1.wav",
"id1/sound/plats/medplat2.wav",
"id1/sound/buttons/airbut1.wav",
"id1/sound/misc/medkey.wav",
"id1/sound/ambience/hum1.wav",
"id1/sound/ogre/ogdrag.wav",
"id1/sound/ogre/ogdth.wav",
"id1/sound/ogre/ogidle.wav",
"id1/sound/ogre/ogidle2.wav",
"id1/sound/ogre/ogpain1.wav",
"id1/sound/ogre/ogsawatk.wav",
"id1/sound/ogre/ogwake.wav",
"id1/sound/wizard/wattack.wav",
"id1/sound/wizard/wdeath.wav",
"id1/sound/wizard/widle1.wav",
"id1/sound/wizard/widle2.wav",
"id1/sound/wizard/wpain.wav",
"id1/sound/wizard/wsight.wav",
"id1/sound/demon/ddeath.wav",
"id1/sound/demon/dhit2.wav",
"id1/sound/demon/djump.wav",
"id1/sound/demon/dpain1.wav",
"id1/sound/demon/idle1.wav",
"id1/sound/demon/sight2.wav",
"id1/sound/items/health1.wav",
"id1/sound/items/r_item1.wav",
"id1/sound/misc/secret.wav",
"id1/sound/items/inv1.wav",
"id1/sound/items/inv2.wav",
"id1/sound/items/inv3.wav",
"id1/sound/ambience/drip1.wav",
"id1/sound/ambience/swamp2.wav",
"id1/sound/ambience/swamp1.wav",
"id1/sound/items/r_item1.wav",
"id1/sound/items/r_item2.wav",
"id1/sound/items/health1.wav",
"id1/sound/misc/medkey.wav",
"id1/sound/misc/runekey.wav",
"id1/sound/items/protect.wav",
"id1/sound/items/protect2.wav",
"id1/sound/items/protect3.wav",
"id1/sound/items/suit.wav",
"id1/sound/items/suit2.wav",
"id1/sound/items/inv1.wav",
"id1/sound/items/inv2.wav",
"id1/sound/items/inv3.wav",
"id1/sound/items/damage.wav",
"id1/sound/items/damage2.wav",
"id1/sound/items/damage3.wav",
"id1/sound/weapons/rocket1i.wav",
"id1/sound/weapons/sgun1.wav",
"id1/sound/weapons/guncock.wav",
"id1/sound/weapons/spike2.wav",
"id1/sound/weapons/grenade.wav",
"id1/sound/weapons/bounce.wav",
"id1/sound/weapons/shotgn2.wav",
"id1/sound/misc/menu1.wav",
"id1/sound/misc/menu2.wav",
"id1/sound/misc/menu3.wav",
"id1/sound/demon/dland2.wav",
"id1/sound/misc/h2ohit1.wav",
"id1/sound/items/itembk2.wav",
"id1/sound/player/plyrjmp8.wav",
"id1/sound/player/land.wav",
"id1/sound/player/land2.wav",
"id1/sound/player/drown1.wav",
"id1/sound/player/drown2.wav",
"id1/sound/player/gasp1.wav",
"id1/sound/player/gasp2.wav",
"id1/sound/player/h2odeath.wav",
"id1/sound/misc/talk.wav",
"id1/sound/player/teledth1.wav",
"id1/sound/misc/r_tele1.wav",
"id1/sound/misc/r_tele2.wav",
"id1/sound/misc/r_tele3.wav",
"id1/sound/misc/r_tele4.wav",
"id1/sound/misc/r_tele5.wav",
"id1/sound/weapons/lock4.wav",
"id1/sound/weapons/pkup.wav",
"id1/sound/items/armor1.wav",
"id1/sound/weapons/lhit.wav",
"id1/sound/weapons/lstart.wav",
"id1/sound/misc/power.wav",
"id1/sound/player/gib.wav",
"id1/sound/player/udeath.wav",
"id1/sound/player/tornoff2.wav",
"id1/sound/player/pain1.wav",
"id1/sound/player/pain2.wav",
"id1/sound/player/pain3.wav",
"id1/sound/player/pain4.wav",
"id1/sound/player/pain5.wav",
"id1/sound/player/pain6.wav",
"id1/sound/player/death1.wav",
"id1/sound/player/death2.wav",
"id1/sound/player/death3.wav",
"id1/sound/player/death4.wav",
"id1/sound/player/death5.wav",
"id1/sound/weapons/ax1.wav",
"id1/sound/player/axhit1.wav",
"id1/sound/player/axhit2.wav",
"id1/sound/player/h2ojump.wav",
"id1/sound/player/slimbrn2.wav",
"id1/sound/player/inh2o.wav",
"id1/sound/player/inlava.wav",
"id1/sound/misc/outwater.wav",
"id1/sound/player/lburn1.wav",
"id1/sound/player/lburn2.wav",
"id1/sound/misc/water1.wav",
"id1/sound/misc/water2.wav",
"id1/sound/doors/medtry.wav",
"id1/sound/doors/meduse.wav",
"id1/sound/doors/runetry.wav",
"id1/sound/doors/runeuse.wav",
"id1/sound/doors/basetry.wav",
"id1/sound/doors/baseuse.wav",
"id1/sound/misc/null.wav",
"id1/sound/doors/drclos4.wav",
"id1/sound/doors/doormv1.wav",
"id1/sound/doors/hydro1.wav",
"id1/sound/doors/hydro2.wav",
"id1/sound/doors/stndr1.wav",
"id1/sound/doors/stndr2.wav",
"id1/sound/doors/ddoor1.wav",
"id1/sound/doors/ddoor2.wav",
"id1/sound/doors/latch2.wav",
"id1/sound/doors/winch2.wav",
"id1/sound/doors/airdoor1.wav",
"id1/sound/doors/airdoor2.wav",
"id1/sound/doors/basesec1.wav",
"id1/sound/doors/basesec2.wav",
"id1/sound/buttons/airbut1.wav",
"id1/sound/buttons/switch21.wav",
"id1/sound/buttons/switch02.wav",
"id1/sound/buttons/switch04.wav",
"id1/sound/misc/secret.wav",
"id1/sound/misc/trigger1.wav",
"id1/sound/ambience/hum1.wav",
"id1/sound/ambience/windfly.wav",
"id1/sound/plats/plat1.wav",
"id1/sound/plats/plat2.wav",
"id1/sound/plats/medplat1.wav",
"id1/sound/plats/medplat2.wav",
"id1/sound/plats/train2.wav",
"id1/sound/plats/train1.wav",
"id1/sound/ambience/fl_hum1.wav",
"id1/sound/ambience/buzz1.wav",
"id1/sound/ambience/fire1.wav",
"id1/sound/ambience/suck1.wav",
"id1/sound/ambience/drone6.wav",
"id1/sound/ambience/drip1.wav",
"id1/sound/ambience/comp1.wav",
"id1/sound/ambience/thunder1.wav",
"id1/sound/ambience/swamp1.wav",
"id1/sound/ambience/swamp2.wav",
"id1/sound/ogre/ogdrag.wav",
"id1/sound/ogre/ogdth.wav",
"id1/sound/ogre/ogidle.wav",
"id1/sound/ogre/ogidle2.wav",
"id1/sound/ogre/ogpain1.wav",
"id1/sound/ogre/ogsawatk.wav",
"id1/sound/ogre/ogwake.wav",
"id1/sound/demon/ddeath.wav",
"id1/sound/demon/dhit2.wav",
"id1/sound/demon/djump.wav",
"id1/sound/demon/dpain1.wav",
"id1/sound/demon/idle1.wav",
"id1/sound/demon/sight2.wav",
"id1/sound/shambler/sattck1.wav",
"id1/sound/shambler/sboom.wav",
"id1/sound/shambler/sdeath.wav",
"id1/sound/shambler/shurt2.wav",
"id1/sound/shambler/sidle.wav",
"id1/sound/shambler/ssight.wav",
"id1/sound/shambler/melee1.wav",
"id1/sound/shambler/melee2.wav",
"id1/sound/shambler/smack.wav",
"id1/sound/knight/kdeath.wav",
"id1/sound/knight/khurt.wav",
"id1/sound/knight/ksight.wav",
"id1/sound/knight/sword1.wav",
"id1/sound/knight/sword2.wav",
"id1/sound/knight/idle.wav",
"id1/sound/soldier/death1.wav",
"id1/sound/soldier/idle.wav",
"id1/sound/soldier/pain1.wav",
"id1/sound/soldier/pain2.wav",
"id1/sound/soldier/sattck1.wav",
"id1/sound/soldier/sight1.wav",
"id1/sound/wizard/wattack.wav",
"id1/sound/wizard/wdeath.wav",
"id1/sound/wizard/widle1.wav",
"id1/sound/wizard/widle2.wav",
"id1/sound/wizard/wpain.wav",
"id1/sound/wizard/wsight.wav",
"id1/sound/dog/dattack1.wav",
"id1/sound/dog/ddeath.wav",
"id1/sound/dog/dpain1.wav",
"id1/sound/dog/dsight.wav",
"id1/sound/dog/idle.wav",
"id1/sound/zombie/z_idle.wav",
"id1/sound/zombie/z_idle1.wav",
"id1/sound/zombie/z_shot1.wav",
"id1/sound/zombie/z_gib.wav",
"id1/sound/zombie/z_pain.wav",
"id1/sound/zombie/z_pain1.wav",
"id1/sound/zombie/z_fall.wav",
"id1/sound/zombie/z_miss.wav",
"id1/sound/zombie/z_hit.wav",
"id1/sound/zombie/idle_w2.wav",
"id1/sound/boss1/out1.wav",
"id1/sound/boss1/sight1.wav",
"id1/sound/boss1/throw.wav",
"id1/sound/boss1/pain.wav",
"id1/sound/boss1/death.wav",
"id1/progs/g_nail2.mdl",
"id1/progs/g_rock2.mdl",
"id1/progs/g_light.mdl",
"id1/maps/b_batt1.bsp",
"id1/maps/b_batt0.bsp",
"id1/progs/w_s_key.mdl",
"id1/progs/m_s_key.mdl",
"id1/progs/m_g_key.mdl",
"id1/progs/end1.mdl",
"id1/progs/invulner.mdl",
"id1/progs/suit.mdl",
"id1/progs/quaddama.mdl",
"id1/maps/b_explob.bsp",
"id1/progs/shambler.mdl",
"id1/progs/s_light.mdl",
"id1/progs/h_shams.mdl",
"id1/progs/knight.mdl",
"id1/progs/h_knight.mdl",
"id1/progs/soldier.mdl",
"id1/progs/h_guard.mdl",
"id1/progs/h_dog.mdl",
"id1/progs/dog.mdl",
"id1/progs/boss.mdl",
"id1/progs.dat",
"id1/end1.bin",
"id1/demo2.dem",
"id1/demo3.dem",
"id1/gfx/inter.lmp",
"id1/gfx/ranking.lmp",
"id1/gfx/vidmodes.lmp",
"id1/gfx/finale.lmp",
"id1/gfx/conback.lmp",
"id1/gfx/qplaque.lmp",
"id1/gfx/menudot1.lmp",
"id1/gfx/menudot2.lmp",
"id1/gfx/menudot3.lmp",
"id1/gfx/menudot4.lmp",
"id1/gfx/menudot5.lmp",
"id1/gfx/menudot6.lmp",
"id1/gfx/menuplyr.lmp",
"id1/gfx/bigbox.lmp",
"id1/gfx/dim_modm.lmp",
"id1/gfx/dim_drct.lmp",
"id1/gfx/dim_ipx.lmp",
"id1/gfx/dim_tcp.lmp",
"id1/gfx/dim_mult.lmp",
"id1/gfx/mainmenu.lmp",
"id1/gfx/box_tl.lmp",
"id1/gfx/box_tm.lmp",
"id1/gfx/box_tr.lmp",
"id1/gfx/box_ml.lmp",
"id1/gfx/box_mm.lmp",
"id1/gfx/box_mm2.lmp",
"id1/gfx/box_mr.lmp",
"id1/gfx/box_bl.lmp",
"id1/gfx/box_bm.lmp",
"id1/gfx/box_br.lmp",
"id1/gfx/sp_menu.lmp",
"id1/gfx/ttl_sgl.lmp",
"id1/gfx/ttl_main.lmp",
"id1/gfx/ttl_cstm.lmp",
"id1/gfx/mp_menu.lmp",
"id1/gfx/netmen1.lmp",
"id1/gfx/netmen2.lmp",
"id1/gfx/netmen3.lmp",
"id1/gfx/netmen4.lmp",
"id1/gfx/netmen5.lmp",
"id1/gfx/sell.lmp",
"id1/gfx/help0.lmp",
"id1/gfx/help1.lmp",
"id1/gfx/help2.lmp",
"id1/gfx/help3.lmp",
"id1/gfx/help4.lmp",
"id1/gfx/help5.lmp",
"id1/gfx/pause.lmp",
"id1/gfx/loading.lmp",
"id1/gfx/p_option.lmp",
"id1/gfx/p_load.lmp",
"id1/gfx/p_save.lmp",
"id1/gfx/p_multi.lmp",
"id1/maps/start.bsp",
"id1/s0.sav",
"id1/s1.sav",
"id1/s2.sav",
"id1/s3.sav",
"id1/s4.sav",
"id1/s5.sav",
"id1/s6.sav",
"id1/s7.sav",
"id1/s8.sav",
"id1/s9.sav",
"id1/s10.sav",
"id1/s11.sav",
"id1/maps/e1m1.bsp",
"id1/maps/e1m2.bsp",
"id1/maps/e1m4.bsp",
"id1/maps/e1m5.bsp",
"id1/maps/e1m6.bsp",
"id1/maps/e1m7.bsp",
"id1/maps/e1m8.bsp",
NULL};
