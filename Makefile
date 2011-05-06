CC:=nacl-gcc
CXX:=nacl-g++
CCFLAGS := -DSDL -O2
CCFLAGS32 := $(CCFLAGS) -m32
CCFLAGS64 := $(CCFLAGS) -m64
NACL_SDK_ROOT ?= /home/dmichael/native_client_sdk_0_2_781_0/
SDL_ROOT ?= /home/dmichael/naclports/src/packages/repository/SDL-1.2.14
NACL_TOOLCHAIN_ROOT ?= $(NACL_SDK_ROOT)/toolchain/linux_x86
#INCLUDES := -I. -I$(NACL_SDK_ROOT) -I$(NACL_TOOLCHAIN_ROOT)/nacl/include -I$(SDL_ROOT)/include -I$(SDL_ROOT)/SDL-1.2.14-build/include
INCLUDES := -I. -I$(SDL_ROOT)/include -I$(SDL_ROOT)/SDL-1.2.14-build/include

# if nacl target, copy nexe to "nacl_quake" for quake.html to launch
# and add nacl_file.c to list of files for browser support.
LDFLAGS = -Xlinker --wrap -Xlinker read
LDFLAGS += -Xlinker --wrap -Xlinker open
LDFLAGS += -Xlinker --wrap -Xlinker close
LDFLAGS += -Xlinker --wrap -Xlinker lseek
LDFLAGS += -Xlinker --wrap -Xlinker write
LDFLAGS += -lnosys \
          -lSDL \
          -lppruntime \
          -lppapi_cpp \
          -lplatform \
          -lgio \
          -lm \
          -lpthread \
          -lsrpc
LDFLAGS32 = $(LDFLAGS) -m32
LDFLAGS64 = $(LDFLAGS) -m64
CCFILES := nacl_file.cc geturl_handler.cc quake_module.cc quake_instance.cc
CFILES :=

X86_SRCS :=  snd_mixa.S sys_dosa.S d_draw.S d_draw16.S d_parta.S d_polysa.S\
             d_scana.S d_spr8.S d_varsa.S math.S r_aclipa.S r_aliasa.S\
             r_drawa.S r_edgea.S surf16.S surf8.S worlda.S

NONX86_SRCS :=  d_vars.c nonintel.c

CFILES += cd_nacl.c chase.c cl_demo.c cl_input.c cl_main.c cl_parse.c\
         cl_tent.c cmd.c common.c console.c crc.c cvar.c d_edge.c d_fill.c\
         d_init.c d_modech.c d_part.c d_polyse.c d_scan.c d_sky.c d_sprite.c\
         d_surf.c d_zpoint.c draw.c host.c host_cmd.c keys.c mathlib.c menu.c\
         model.c net_bsd.c net_dgrm.c net_loop.c net_main.c net_udp.c \
         net_vcr.c net_wso.c pr_cmds.c pr_edict.c pr_exec.c r_aclip.c\
         r_alias.c r_bsp.c r_draw.c r_edge.c r_efrag.c r_light.c r_main.c\
         r_misc.c r_part.c r_sky.c r_sprite.c r_surf.c r_vars.c r_varsa.S\
         sbar.c screen.c snd_null.c sv_main.c\
         sv_move.c sv_phys.c sv_user.c sys_nacl.c vid_sdl.c view.c wad.c\
         world.c zone.c $(X86_SRCS) $(NONX86_SRCS)
# These files were excluded from FILES because they use instructions
# disallowed by Native Client
OMITTED_S_FILES := d_copy.S dosasm.S

OBJECTS32=$(CFILES:%.c=build/32/%.o) $(CCFILES:%.cc=build/32/%.o)
OBJECTS64=$(CFILES:%.c=build/64/%.o) $(CCFILES:%.cc=build/64/%.o)

quake32: $(OBJECTS32)
	$(CXX) $(OBJECTS32) $(LDFLAGS32) -o quake32.nexe
	nacl-strip quake32.nexe

quake64: $(OBJECTS64)
	$(CXX) $(OBJECTS64) $(LDFLAGS64) -o quake64.nexe
	nacl-strip quake64.nexe

quake: quake32 quake64

all: quake32 quake64

clean:
	# remove all .os and .nexe
	rm -f ./build/*/*.o ./*.nexe

build/32/%.o: %.S
	$(CC) $(INCLUDES) $(CCFLAGS32) -c -o $@ $<

build/32/%.o: %.c
	$(CC) $(INCLUDES) $(CCFLAGS32) -c -o $@ $<

build/32/%.o: %.cc
	$(CXX) $(INCLUDES) $(CCFLAGS32) -c -o $@ $<

build/64/%.o: %.S
	$(CC) $(INCLUDES) $(CCFLAGS64) -c -o $@ $<

build/64/%.o: %.c
	$(CC) $(INCLUDES) $(CCFLAGS64) -c -o $@ $<

build/64/%.o: %.cc
	$(CXX) $(INCLUDES) $(CCFLAGS64) -c -o $@ $<

