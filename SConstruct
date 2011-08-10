import os

# TODO: add a configure pass to check for nacl and SDL

# Either set the environment variable 'NACL_SDK_ROOT' or default to /opt/nacl_sdk
def findNaclRoot():
    try:
        return os.environ['NACL_SDK_ROOT']
    except KeyError:
        return '/opt/nacl_sdk'

def setupLibraries(environment):
    libraries = Split("ppruntime ppapi_cpp platform gio m pthread srpc imc imc_syscalls")
    environment.Append(LIBS = libraries)
    return environment

def baseEnvironment():
    environment = Environment(ENV = os.environ)
    naclRoot = findNaclRoot()
    environment.PrependENVPath('PATH', naclRoot + '/toolchain/linux_x86/bin')
    environment.Append(CPPDEFINES = 'SDL')

    wrapped_symbols = Split("open read close lseek write")

    def wrap(symbol):
        return '-Wl,--wrap=%s' % symbol

    environment.Append(LINKFLAGS = map(wrap, wrapped_symbols))
    return environment

def setupTools(environment, prefix):
    environment['CC'] = '%s-gcc' % prefix
    environment['LD'] = '%s-ld' % prefix
    environment['CXX'] = '%s-g++' % prefix
    environment['AS'] = '%s-as' % prefix
    environment['AR'] = '%s-ar' % prefix
    environment['OBJCOPY'] = '%s-objcopy' % prefix
    environment['STRIP'] = '%s-strip' % prefix
    return environment

def environment32():
    environment = baseEnvironment()
    environment['BUILD_DIR'] = 'build/32'
    environment.VariantDir(environment['BUILD_DIR'], '.')
    flags = ['-O2', '-m32']
    environment.Append(CCFLAGS = flags)
    environment.Append(CXXFLAGS = flags)
    environment.PrependENVPath('PATH', findNaclRoot() + '/toolchain/linux_x86/nacl/usr/bin')
    environment.ParseConfig('sdl-config --libs --cflags')
    setupTools(environment, 'nacl')
    setupLibraries(environment)
    return environment

def environment64():
    environment = baseEnvironment()
    environment['BUILD_DIR'] = 'build/64'
    environment.VariantDir(environment['BUILD_DIR'], '.')
    environment.PrependENVPath('PATH', findNaclRoot() + '/toolchain/linux_x86/nacl64/usr/bin')
    flags = ['-O2', '-m64']
    environment.Append(CCFLAGS = flags)
    environment.Append(CXXFLAGS = flags)
    environment.ParseConfig('sdl-config --libs --cflags')
    setupTools(environment, 'nacl64')
    setupLibraries(environment)
    return environment

def sources():
    cc_files = Split("nacl_file.cc geturl_handler.cc quake_module.cc quake_instance.cc")
    c_files = Split("""cd_nacl.c chase.c cl_demo.c cl_input.c cl_main.c cl_parse.c
                       cl_tent.c cmd.c common.c console.c crc.c cvar.c d_edge.c d_fill.c
                       d_init.c d_modech.c d_part.c d_polyse.c d_scan.c d_sky.c d_sprite.c
                       d_surf.c d_zpoint.c draw.c host.c host_cmd.c keys.c mathlib.c menu.c
                       model.c net_bsd.c net_dgrm.c net_loop.c net_main.c net_udp.c
                       net_vcr.c net_wso.c pr_cmds.c pr_edict.c pr_exec.c r_aclip.c
                       r_alias.c r_bsp.c r_draw.c r_edge.c r_efrag.c r_light.c r_main.c
                       r_misc.c r_part.c r_sky.c r_sprite.c r_surf.c r_vars.c r_varsa.S
                       sbar.c screen.c snd_null.c sv_main.c stubs.c
                       sv_move.c sv_phys.c sv_user.c sys_nacl.c vid_sdl.c view.c wad.c
                       world.c zone.c""")
    x86_files = Split("""snd_mixa.S sys_dosa.S d_draw.S d_draw16.S d_parta.S d_polysa.S
                         d_scana.S d_spr8.S d_varsa.S math.S r_aclipa.S r_aliasa.S
                         r_drawa.S r_edgea.S surf16.S surf8.S worlda.S""")

    nonx86_files = Split("d_vars.c nonintel.c")

    return c_files + cc_files + x86_files + nonx86_files

def build(environment, source):
    quake = environment.Program('%s/quake' % environment['BUILD_DIR'], map(lambda file: '%s/%s' % (environment['BUILD_DIR'], file), source))
    environment.AddPostAction(quake, Action('$STRIP $TARGET'))
    return quake

build(environment32(), sources())
build(environment64(), sources())
