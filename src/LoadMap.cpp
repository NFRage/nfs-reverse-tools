////////////////////////////////////////////////////////////////////////////////
/// @file LoadMap.cpp
///     Implementation of an IDA plugin, which loads a VC++/BCC map file.
/// @par Purpose:
///     An IDA plugin, which loads a VC/Borland/Dede map file into IDA Database.
///     Based on the idea of loadmap plugin by Toshiyuki Tega.
/// @author TQN <truong_quoc_ngan@yahoo.com>
/// @author TL <mefistotelis@gmail.com>
/// @date 2004.09.11 - 2018.11.08
/// @version 1.3 - 2018.11.08 - Compiling in VS2010, SDK from IDA 7.0
/// @version 1.2 - 2012.07.18 - Loading GCC MAP files, compiling in IDA 6.2
/// @version 1.1 - 2011.09.13 - Loading Watcom MAP files, compiling in IDA 6.1
/// @version 1.0 - 2004.09.11 - Initial release
/// @par  Copying and copyrights:
///     This program is free software; you can redistribute it and/or modify
///     it under the terms of the GNU General Public License as published by
///     the Free Software Foundation; either version 2 of the License, or
///     (at your option) any later version.
////////////////////////////////////////////////////////////////////////////////
#define PLUG_VERSION "1.3"
//  standard library headers.
#include <cstdio>
// Makes gcc stdlib to not define non-underscored versions of non-ANSI functions (ie memicmp, strlwr)
#define _NO_OLDNAMES
#include <cstring>
#undef _NO_OLDNAMES

//  other headers.
#include  "MAPReader.h"
#include "stdafx.h"

//#define USE_STANDARD_FILE_FUNCTIONS
//#define USE_DANGEROUS_FUNCTIONS

// IDA SDK Header Files
#include <ida.hpp>
#include <idp.hpp>
#include <loader.hpp>
#include <kernwin.hpp>
#include <diskio.hpp>
#include <bytes.hpp>
#include <name.hpp>
#include <entry.hpp>
#include <fpro.h>
#include <prodir.h> // just for MAXPATH
#include <auto.hpp>


typedef struct _tagPLUGIN_OPTIONS {
    int bNameApply;    //< true - apply to name, false - apply to comment
    int bReplace;      //< replace the existing name or comment
    int bVerbose;      //< show detail messages
} PLUGIN_OPTIONS;

const size_t g_minLineLen = 14; // For a "xxxx:xxxxxxxx " line

static char g_szIniPath[MAXPATH] = { 0 };

/// @brief Global variable for options of plugin
static PLUGIN_OPTIONS g_options = { 0 };

static const cfgopt_t g_optsinfo[] =
{
	cfgopt_t("NAME_APPLY", &g_options.bNameApply, 0, 1),
    cfgopt_t("REPLACE_EXISTING", &g_options.bReplace, 0, 1),
	cfgopt_t("VERBOSE_MESSAGES", &g_options.bVerbose, 0, 1),
};

////////////////////////////////////////////////////////////////////////////////
/// @name Ini Section and Key names
/// @{
static char g_szLoadMapSection[] = "LoadMap";
static char g_szOptionsKey[] = "Options";
/// @}

void linearAddressToSymbolAddr(MapFile::MAPSymbol &sym, unsigned long linear_addr)
{
    sym.seg = get_segm_num(linear_addr);
    segment_t * sseg = getnseg((int) sym.seg);
    if (sseg != NULL)
        sym.addr = linear_addr - sseg->start_ea;
    else
        sym.addr = -1;
}

BOOL IsXboxLibraryFile(const char* filename)
{
  // eww.. for some reason this is the fastest method to check all these..

  if (inf.filetype != f_LOADER)
    return false; // only valid for xbox/xbox360 libraries (which require a loader)

  /* Xbox OG libraries */
  const char* lib_d3d8_xbox = "d3d8-xbox:";
  const char* lib_D3D8 = "D3D8:";
  const char* lib_D3DX8 = "D3DX8:";
  const char* lib_D3DX8d = "D3DX8d:";
  const char* lib_d3dx8dt = "d3dx8dt:";
  const char* lib_d3dxof = "d3dxof:";
  const char* lib_dxguid = "dxguid:";
  const char* lib_winhttp5 = "winhttp5:";
  const char* lib_xboxdbg = "xboxdbg:";
  const char* lib_xcontent = "xcontent:";
  const char* lib_xgraphics = "xgraphics:";
  const char* lib_xonservr = "xonservr:";
  const char* lib_d3d8d = "d3d8d:";
  const char* lib_d3d8i = "d3d8i:";
  const char* lib_d3d8ltcg = "d3d8ltcg:";
  const char* lib_dmusic = "dmusic:";
  const char* lib_dmusicd = "dmusicd:";
  const char* lib_dmusici = "dmusici:";
  const char* lib_dmusicltcg = "dmusicltcg:";
  const char* lib_dsound = "dsound:";
  const char* lib_dsoundd = "dsoundd:";
  const char* lib_libc = "libc:";
  const char* lib_libcd = "libcd:";
  const char* lib_libcmt = "libcmt:";
  const char* lib_libcmtd = "libcmtd:";
  const char* lib_libcp = "libcp:";
  const char* lib_libcpd = "libcpd:";
  const char* lib_libcpmt = "libcpmt:";
  const char* lib_libcpmtd = "libcpmtd:";
  const char* lib_oldnames = "oldnames:";
  const char* lib_uix = "uix:";
  const char* lib_uixd = "uixd:";
  const char* lib_uuid = "uuid:";
  const char* lib_xacteng = "xacteng:";
  const char* lib_xactengd = "xactengd:";
  const char* lib_xactengi = "xactengi:";
  const char* lib_xactengltcg = "xactengltcg:";
  const char* lib_xapilib = "xapilib:";
  const char* lib_xapilibd = "xapilibd:";
  const char* lib_xbdm = "xbdm:";
  const char* lib_xboxkrnl = "xboxkrnl:";
  const char* lib_xgraphicsd = "xgraphicsd:";
  const char* lib_xgraphicsltcg = "xgraphicsltcg:";
  const char* lib_xkbd = "xkbd:";
  const char* lib_xkbdd = "xkbdd:";
  const char* lib_xmv = "xmv:";
  const char* lib_xmvd = "xmvd:";
  const char* lib_xnet = "xnet:";
  const char* lib_xnetd = "xnetd:";
  const char* lib_xnetn = "xnetn:";
  const char* lib_xnetnd = "xnetnd:";
  const char* lib_xnets = "xnets:";
  const char* lib_xnetsd = "xnetsd:";
  const char* lib_xonline = "xonline:";
  const char* lib_xonlined = "xonlined:";
  const char* lib_xonlinel = "xonlinel:";
  const char* lib_xonlineld = "xonlineld:";
  const char* lib_xonlinels = "xonlinels:";
  const char* lib_xonlinelsd = "xonlinelsd:";
  const char* lib_xonlinen = "xonlinen:";
  const char* lib_xonlinend = "xonlinend:";
  const char* lib_xonlines = "xonlines:";
  const char* lib_xonlinesd = "xonlinesd:";
  const char* lib_xperf = "xperf:";
  const char* lib_xsndtrk = "xsndtrk:";
  const char* lib_xsndtrkd = "xsndtrkd:";
  const char* lib_xvoice = "xvoice:";
  const char* lib_xvoiced = "xvoiced:";

  /* Xbox 360 libraries */
  const char* lib_d3d9 = "d3d9:";
  const char* lib_d3d9d = "d3d9d:";
  const char* lib_d3d9i = "d3d9i:";
  const char* lib_d3d9ltcg = "d3d9ltcg:";
  const char* lib_d3d9ltcgi = "d3d9ltcgi:";
  const char* lib_d3dx9 = "d3dx9:";
  const char* lib_d3dx9d = "d3dx9d:";
  const char* lib_d3dx9i = "d3dx9i:";
  const char* lib_dxerr9 = "dxerr9:";
  const char* lib_libcMT = "libcMT:";
  const char* lib_libcMTd = "libcMTd:";
  const char* lib_libcpMT = "libcpMT:";
  const char* lib_libcpMTd = "libcpMTd:";
  const char* lib_libpmcpb = "libpmcpb:";
  const char* lib_libpmcpbd = "libpmcpbd:";
  const char* lib_multidisc = "multidisc:";
  const char* lib_multidiscd = "multidiscd:";
  const char* lib_nuiapi = "nuiapi:";
  const char* lib_nuiapid = "nuiapid:";
  const char* lib_NuiAudio = "NuiAudio:";
  const char* lib_NuiAudiod = "NuiAudiod:";
  const char* lib_nuifitnessapi = "nuifitnessapi:";
  const char* lib_nuifitnessapid = "nuifitnessapid:";
  const char* lib_nuihandles = "nuihandles:";
  const char* lib_nuihandlesd = "nuihandlesd:";
  const char* lib_nuispeech = "nuispeech:";
  const char* lib_nuispeechd = "nuispeechd:";
  //const char* lib_oldnames = "oldnames:";
  const char* lib_qnetxaudio2 = "qnetxaudio2:";
  const char* lib_qnetxaudio2d = "qnetxaudio2d:";
  const char* lib_st = "st:";
  const char* lib_std = "std:";
  const char* lib_stltcg = "stltcg:";
  const char* lib_tracerecording = "tracerecording:";
  const char* lib_tracerecordingd = "tracerecordingd:";
  const char* lib_vcomp = "vcomp:";
  const char* lib_vcompd = "vcompd:";
  const char* lib_x3daudio = "x3daudio:";
  const char* lib_x3daudiod = "x3daudiod:";
  const char* lib_x3daudioi = "x3daudioi:";
  const char* lib_x3daudioltcg = "x3daudioltcg:";
  const char* lib_xact3 = "xact3:";
  const char* lib_xact3i = "xact3i:";
  const char* lib_xact3ltcg = "xact3ltcg:";
  const char* lib_xacta3 = "xacta3:";
  const char* lib_xactad3 = "xactad3:";
  const char* lib_xactd3 = "xactd3:";
  //const char* lib_xapilib = "xapilib:";
  //const char* lib_xapilibd = "xapilibd:";
  const char* lib_xapilibi = "xapilibi:";
  const char* lib_XAPOBase = "XAPOBase:";
  const char* lib_XAPOBaseD = "XAPOBaseD:";
  const char* lib_XAPOFX = "XAPOFX:";
  const char* lib_XAPOFXD = "XAPOFXD:";
  const char* lib_xaudio2 = "xaudio2:";
  const char* lib_xaudiod2 = "xaudiod2:";
  const char* lib_xauth = "xauth:";
  const char* lib_xauthd = "xauthd:";
  const char* lib_xav = "xav:";
  const char* lib_xavatar2 = "xavatar2:";
  const char* lib_xavatar2d = "xavatar2d:";
  const char* lib_xavatar2ltcg = "xavatar2ltcg:";
  const char* lib_xavd = "xavd:";
  const char* lib_xbc = "xbc:";
  const char* lib_xbcd = "xbcd:";
  //const char* lib_xbdm = "xbdm:";
  //const char* lib_xboxkrnl = "xboxkrnl:";
  const char* lib_xcam = "xcam:";
  const char* lib_xcamd = "xcamd:";
  const char* lib_xffb = "xffb:";
  const char* lib_xffbd = "xffbd:";
  const char* lib_xgetserviceendpoint = "xgetserviceendpoint:";
  const char* lib_xgetserviceendpointd = "xgetserviceendpointd:";
  //const char* lib_xgraphics = "xgraphics:";
  //const char* lib_xgraphicsd = "xgraphicsd:";
  const char* lib_xhttp = "xhttp:";
  const char* lib_xhttpd = "xhttpd:";
  const char* lib_xhv2 = "xhv2:";
  const char* lib_xhvd2 = "xhvd2:";
  const char* lib_xime = "xime:";
  const char* lib_ximed = "ximed:";
  const char* lib_xinput2 = "xinput2:";
  const char* lib_xinput2d = "xinput2d:";
  const char* lib_xinputremap = "xinputremap:";
  const char* lib_xinputremapd = "xinputremapd:";
  const char* lib_xjson = "xjson:";
  const char* lib_xjsond = "xjsond:";
  const char* lib_xmahal = "xmahal:";
  const char* lib_xmahald = "xmahald:";
  const char* lib_xmahali = "xmahali:";
  const char* lib_xmahalltcg = "xmahalltcg:";
  const char* lib_xmcore = "xmcore:";
  const char* lib_xmcored = "xmcored:";
  const char* lib_xmcorei = "xmcorei:";
  const char* lib_xmcoreltcg = "xmcoreltcg:";
  const char* lib_xmedia2 = "xmedia2:";
  const char* lib_xmediad2 = "xmediad2:";
  const char* lib_xmic = "xmic:";
  const char* lib_xmicd = "xmicd:";
  const char* lib_xmp = "xmp:";
  const char* lib_xmpd = "xmpd:";
  //const char* lib_xnet = "xnet:";
  const char* lib_xnetconfiginfo = "xnetconfiginfo:";
  const char* lib_xnetconfiginfod = "xnetconfiginfod:";
  //const char* lib_xnetd = "xnetd:";
  //const char* lib_xonline = "xonline:";
  //const char* lib_xonlined = "xonlined:";
  const char* lib_xparty = "xparty:";
  const char* lib_xpartyd = "xpartyd:";
  const char* lib_xrnm = "xrnm:";
  const char* lib_xrnmd = "xrnmd:";
  const char* lib_xrnms = "xrnms:";
  const char* lib_xrnmsd = "xrnmsd:";
  const char* lib_xsim = "xsim:";
  const char* lib_xsimd = "xsimd:";
  const char* lib_xsocialpost = "xsocialpost:";
  const char* lib_xsocialpostd = "xsocialpostd:";
  const char* lib_xstudio = "xstudio:";
  const char* lib_xtms = "xtms:";
  const char* lib_xtmsd = "xtmsd:";
  const char* lib_xuihtml = "xuihtml:";
  const char* lib_xuihtmld = "xuihtmld:";
  const char* lib_xuirender = "xuirender:";
  const char* lib_xuirenderd = "xuirenderd:";
  const char* lib_xuirenderltcg = "xuirenderltcg:";
  const char* lib_xuirun = "xuirun:";
  const char* lib_xuiruna = "xuiruna:";
  const char* lib_xuirunad = "xuirunad:";
  const char* lib_xuirund = "xuirund:";
  const char* lib_xuirunltcg = "xuirunltcg:";
  const char* lib_xuivideo = "xuivideo:";
  const char* lib_xuivideod = "xuivideod:";
  const char* lib_xwmadecode = "xwmadecode:";
  const char* lib_xwmadecoded = "xwmadecoded:";

  // H4 only?
  const char* lib_retaildump = "retaildump:";

  if (strchr(filename, ':') == nullptr)
    return false;

  // todo: a lot more xbox libs!
  return (strncasecmp(filename, lib_d3d8_xbox, strlen(lib_d3d8_xbox)) == 0 ||
    strncasecmp(filename, lib_D3D8, strlen(lib_D3D8)) == 0 ||
    strncasecmp(filename, lib_D3DX8, strlen(lib_D3DX8)) == 0 ||
    strncasecmp(filename, lib_D3DX8d, strlen(lib_D3DX8d)) == 0 ||
    strncasecmp(filename, lib_d3dx8dt, strlen(lib_d3dx8dt)) == 0 ||
    strncasecmp(filename, lib_d3dxof, strlen(lib_d3dxof)) == 0 ||
    strncasecmp(filename, lib_dxguid, strlen(lib_dxguid)) == 0 ||
    strncasecmp(filename, lib_winhttp5, strlen(lib_winhttp5)) == 0 ||
    strncasecmp(filename, lib_xboxdbg, strlen(lib_xboxdbg)) == 0 ||
    strncasecmp(filename, lib_xcontent, strlen(lib_xcontent)) == 0 ||
    strncasecmp(filename, lib_xgraphics, strlen(lib_xgraphics)) == 0 ||
    strncasecmp(filename, lib_xonservr, strlen(lib_xonservr)) == 0 ||
    strncasecmp(filename, lib_d3d8d, strlen(lib_d3d8d)) == 0 ||
    strncasecmp(filename, lib_d3d8i, strlen(lib_d3d8i)) == 0 ||
    strncasecmp(filename, lib_d3d8ltcg, strlen(lib_d3d8ltcg)) == 0 ||
    strncasecmp(filename, lib_dmusic, strlen(lib_dmusic)) == 0 ||
    strncasecmp(filename, lib_dmusicd, strlen(lib_dmusicd)) == 0 ||
    strncasecmp(filename, lib_dmusici, strlen(lib_dmusici)) == 0 ||
    strncasecmp(filename, lib_dmusicltcg, strlen(lib_dmusicltcg)) == 0 ||
    strncasecmp(filename, lib_dsound, strlen(lib_dsound)) == 0 ||
    strncasecmp(filename, lib_dsoundd, strlen(lib_dsoundd)) == 0 ||
    strncasecmp(filename, lib_libc, strlen(lib_libc)) == 0 ||
    strncasecmp(filename, lib_libcd, strlen(lib_libcd)) == 0 ||
    strncasecmp(filename, lib_libcmt, strlen(lib_libcmt)) == 0 ||
    strncasecmp(filename, lib_libcmtd, strlen(lib_libcmtd)) == 0 ||
    strncasecmp(filename, lib_libcp, strlen(lib_libcp)) == 0 ||
    strncasecmp(filename, lib_libcpd, strlen(lib_libcpd)) == 0 ||
    strncasecmp(filename, lib_libcpmt, strlen(lib_libcpmt)) == 0 ||
    strncasecmp(filename, lib_libcpmtd, strlen(lib_libcpmtd)) == 0 ||
    strncasecmp(filename, lib_oldnames, strlen(lib_oldnames)) == 0 ||
    strncasecmp(filename, lib_uix, strlen(lib_uix)) == 0 ||
    strncasecmp(filename, lib_uixd, strlen(lib_uixd)) == 0 ||
    strncasecmp(filename, lib_uuid, strlen(lib_uuid)) == 0 ||
    strncasecmp(filename, lib_xacteng, strlen(lib_xacteng)) == 0 ||
    strncasecmp(filename, lib_xactengd, strlen(lib_xactengd)) == 0 ||
    strncasecmp(filename, lib_xactengi, strlen(lib_xactengi)) == 0 ||
    strncasecmp(filename, lib_xactengltcg, strlen(lib_xactengltcg)) == 0 ||
    strncasecmp(filename, lib_xapilib, strlen(lib_xapilib)) == 0 ||
    strncasecmp(filename, lib_xapilibd, strlen(lib_xapilibd)) == 0 ||
    strncasecmp(filename, lib_xbdm, strlen(lib_xbdm)) == 0 ||
    strncasecmp(filename, lib_xboxkrnl, strlen(lib_xboxkrnl)) == 0 ||
    strncasecmp(filename, lib_xgraphicsd, strlen(lib_xgraphicsd)) == 0 ||
    strncasecmp(filename, lib_xgraphicsltcg, strlen(lib_xgraphicsltcg)) == 0 ||
    strncasecmp(filename, lib_xkbd, strlen(lib_xkbd)) == 0 ||
    strncasecmp(filename, lib_xkbdd, strlen(lib_xkbdd)) == 0 ||
    strncasecmp(filename, lib_xmv, strlen(lib_xmv)) == 0 ||
    strncasecmp(filename, lib_xmvd, strlen(lib_xmvd)) == 0 ||
    strncasecmp(filename, lib_xnet, strlen(lib_xnet)) == 0 ||
    strncasecmp(filename, lib_xnetd, strlen(lib_xnetd)) == 0 ||
    strncasecmp(filename, lib_xnetn, strlen(lib_xnetn)) == 0 ||
    strncasecmp(filename, lib_xnetnd, strlen(lib_xnetnd)) == 0 ||
    strncasecmp(filename, lib_xnets, strlen(lib_xnets)) == 0 ||
    strncasecmp(filename, lib_xnetsd, strlen(lib_xnetsd)) == 0 ||
    strncasecmp(filename, lib_xonline, strlen(lib_xonline)) == 0 ||
    strncasecmp(filename, lib_xonlined, strlen(lib_xonlined)) == 0 ||
    strncasecmp(filename, lib_xonlinel, strlen(lib_xonlinel)) == 0 ||
    strncasecmp(filename, lib_xonlineld, strlen(lib_xonlineld)) == 0 ||
    strncasecmp(filename, lib_xonlinels, strlen(lib_xonlinels)) == 0 ||
    strncasecmp(filename, lib_xonlinelsd, strlen(lib_xonlinelsd)) == 0 ||
    strncasecmp(filename, lib_xonlinen, strlen(lib_xonlinen)) == 0 ||
    strncasecmp(filename, lib_xonlinend, strlen(lib_xonlinend)) == 0 ||
    strncasecmp(filename, lib_xonlines, strlen(lib_xonlines)) == 0 ||
    strncasecmp(filename, lib_xonlinesd, strlen(lib_xonlinesd)) == 0 ||
    strncasecmp(filename, lib_xperf, strlen(lib_xperf)) == 0 ||
    strncasecmp(filename, lib_xsndtrk, strlen(lib_xsndtrk)) == 0 ||
    strncasecmp(filename, lib_xsndtrkd, strlen(lib_xsndtrkd)) == 0 ||
    strncasecmp(filename, lib_xvoice, strlen(lib_xvoice)) == 0 ||
    strncasecmp(filename, lib_xvoiced, strlen(lib_xvoiced)) == 0 ||

    strncasecmp(filename, lib_d3d9, strlen(lib_d3d9)) == 0 ||
    strncasecmp(filename, lib_d3d9d, strlen(lib_d3d9d)) == 0 ||
    strncasecmp(filename, lib_d3d9i, strlen(lib_d3d9i)) == 0 ||
    strncasecmp(filename, lib_d3d9ltcg, strlen(lib_d3d9ltcg)) == 0 ||
    strncasecmp(filename, lib_d3d9ltcgi, strlen(lib_d3d9ltcgi)) == 0 ||
    strncasecmp(filename, lib_d3dx9, strlen(lib_d3dx9)) == 0 ||
    strncasecmp(filename, lib_d3dx9d, strlen(lib_d3dx9d)) == 0 ||
    strncasecmp(filename, lib_d3dx9i, strlen(lib_d3dx9i)) == 0 ||
    strncasecmp(filename, lib_dxerr9, strlen(lib_dxerr9)) == 0 ||
    strncasecmp(filename, lib_libcMT, strlen(lib_libcMT)) == 0 ||
    strncasecmp(filename, lib_libcMTd, strlen(lib_libcMTd)) == 0 ||
    strncasecmp(filename, lib_libcpMT, strlen(lib_libcpMT)) == 0 ||
    strncasecmp(filename, lib_libcpMTd, strlen(lib_libcpMTd)) == 0 ||
    strncasecmp(filename, lib_libpmcpb, strlen(lib_libpmcpb)) == 0 ||
    strncasecmp(filename, lib_libpmcpbd, strlen(lib_libpmcpbd)) == 0 ||
    strncasecmp(filename, lib_multidisc, strlen(lib_multidisc)) == 0 ||
    strncasecmp(filename, lib_multidiscd, strlen(lib_multidiscd)) == 0 ||
    strncasecmp(filename, lib_nuiapi, strlen(lib_nuiapi)) == 0 ||
    strncasecmp(filename, lib_nuiapid, strlen(lib_nuiapid)) == 0 ||
    strncasecmp(filename, lib_NuiAudio, strlen(lib_NuiAudio)) == 0 ||
    strncasecmp(filename, lib_NuiAudiod, strlen(lib_NuiAudiod)) == 0 ||
    strncasecmp(filename, lib_nuifitnessapi, strlen(lib_nuifitnessapi)) == 0 ||
    strncasecmp(filename, lib_nuifitnessapid, strlen(lib_nuifitnessapid)) == 0 ||
    strncasecmp(filename, lib_nuihandles, strlen(lib_nuihandles)) == 0 ||
    strncasecmp(filename, lib_nuihandlesd, strlen(lib_nuihandlesd)) == 0 ||
    strncasecmp(filename, lib_nuispeech, strlen(lib_nuispeech)) == 0 ||
    strncasecmp(filename, lib_nuispeechd, strlen(lib_nuispeechd)) == 0 ||
    //strncasecmp(filename, lib_oldnames, strlen(lib_oldnames)) == 0 ||
    strncasecmp(filename, lib_qnetxaudio2, strlen(lib_qnetxaudio2)) == 0 ||
    strncasecmp(filename, lib_qnetxaudio2d, strlen(lib_qnetxaudio2d)) == 0 ||
    strncasecmp(filename, lib_st, strlen(lib_st)) == 0 ||
    strncasecmp(filename, lib_std, strlen(lib_std)) == 0 ||
    strncasecmp(filename, lib_stltcg, strlen(lib_stltcg)) == 0 ||
    strncasecmp(filename, lib_tracerecording, strlen(lib_tracerecording)) == 0 ||
    strncasecmp(filename, lib_tracerecordingd, strlen(lib_tracerecordingd)) == 0 ||
    strncasecmp(filename, lib_vcomp, strlen(lib_vcomp)) == 0 ||
    strncasecmp(filename, lib_vcompd, strlen(lib_vcompd)) == 0 ||
    strncasecmp(filename, lib_x3daudio, strlen(lib_x3daudio)) == 0 ||
    strncasecmp(filename, lib_x3daudiod, strlen(lib_x3daudiod)) == 0 ||
    strncasecmp(filename, lib_x3daudioi, strlen(lib_x3daudioi)) == 0 ||
    strncasecmp(filename, lib_x3daudioltcg, strlen(lib_x3daudioltcg)) == 0 ||
    strncasecmp(filename, lib_xact3, strlen(lib_xact3)) == 0 ||
    strncasecmp(filename, lib_xact3i, strlen(lib_xact3i)) == 0 ||
    strncasecmp(filename, lib_xact3ltcg, strlen(lib_xact3ltcg)) == 0 ||
    strncasecmp(filename, lib_xacta3, strlen(lib_xacta3)) == 0 ||
    strncasecmp(filename, lib_xactad3, strlen(lib_xactad3)) == 0 ||
    strncasecmp(filename, lib_xactd3, strlen(lib_xactd3)) == 0 ||
    //strncasecmp(filename, lib_xapilib, strlen(lib_xapilib)) == 0 ||
    //strncasecmp(filename, lib_xapilibd, strlen(lib_xapilibd)) == 0 ||
    strncasecmp(filename, lib_xapilibi, strlen(lib_xapilibi)) == 0 ||
    strncasecmp(filename, lib_XAPOBase, strlen(lib_XAPOBase)) == 0 ||
    strncasecmp(filename, lib_XAPOBaseD, strlen(lib_XAPOBaseD)) == 0 ||
    strncasecmp(filename, lib_XAPOFX, strlen(lib_XAPOFX)) == 0 ||
    strncasecmp(filename, lib_XAPOFXD, strlen(lib_XAPOFXD)) == 0 ||
    strncasecmp(filename, lib_xaudio2, strlen(lib_xaudio2)) == 0 ||
    strncasecmp(filename, lib_xaudiod2, strlen(lib_xaudiod2)) == 0 ||
    strncasecmp(filename, lib_xauth, strlen(lib_xauth)) == 0 ||
    strncasecmp(filename, lib_xauthd, strlen(lib_xauthd)) == 0 ||
    strncasecmp(filename, lib_xav, strlen(lib_xav)) == 0 ||
    strncasecmp(filename, lib_xavatar2, strlen(lib_xavatar2)) == 0 ||
    strncasecmp(filename, lib_xavatar2d, strlen(lib_xavatar2d)) == 0 ||
    strncasecmp(filename, lib_xavatar2ltcg, strlen(lib_xavatar2ltcg)) == 0 ||
    strncasecmp(filename, lib_xavd, strlen(lib_xavd)) == 0 ||
    strncasecmp(filename, lib_xbc, strlen(lib_xbc)) == 0 ||
    strncasecmp(filename, lib_xbcd, strlen(lib_xbcd)) == 0 ||
    //strncasecmp(filename, lib_xbdm, strlen(lib_xbdm)) == 0 ||
    //strncasecmp(filename, lib_xboxkrnl, strlen(lib_xboxkrnl)) == 0 ||
    strncasecmp(filename, lib_xcam, strlen(lib_xcam)) == 0 ||
    strncasecmp(filename, lib_xcamd, strlen(lib_xcamd)) == 0 ||
    strncasecmp(filename, lib_xffb, strlen(lib_xffb)) == 0 ||
    strncasecmp(filename, lib_xffbd, strlen(lib_xffbd)) == 0 ||
    strncasecmp(filename, lib_xgetserviceendpoint, strlen(lib_xgetserviceendpoint)) == 0 ||
    strncasecmp(filename, lib_xgetserviceendpointd, strlen(lib_xgetserviceendpointd)) == 0 ||
    //strncasecmp(filename, lib_xgraphics, strlen(lib_xgraphics)) == 0 ||
    //strncasecmp(filename, lib_xgraphicsd, strlen(lib_xgraphicsd)) == 0 ||
    strncasecmp(filename, lib_xhttp, strlen(lib_xhttp)) == 0 ||
    strncasecmp(filename, lib_xhttpd, strlen(lib_xhttpd)) == 0 ||
    strncasecmp(filename, lib_xhv2, strlen(lib_xhv2)) == 0 ||
    strncasecmp(filename, lib_xhvd2, strlen(lib_xhvd2)) == 0 ||
    strncasecmp(filename, lib_xime, strlen(lib_xime)) == 0 ||
    strncasecmp(filename, lib_ximed, strlen(lib_ximed)) == 0 ||
    strncasecmp(filename, lib_xinput2, strlen(lib_xinput2)) == 0 ||
    strncasecmp(filename, lib_xinput2d, strlen(lib_xinput2d)) == 0 ||
    strncasecmp(filename, lib_xinputremap, strlen(lib_xinputremap)) == 0 ||
    strncasecmp(filename, lib_xinputremapd, strlen(lib_xinputremapd)) == 0 ||
    strncasecmp(filename, lib_xjson, strlen(lib_xjson)) == 0 ||
    strncasecmp(filename, lib_xjsond, strlen(lib_xjsond)) == 0 ||
    strncasecmp(filename, lib_xmahal, strlen(lib_xmahal)) == 0 ||
    strncasecmp(filename, lib_xmahald, strlen(lib_xmahald)) == 0 ||
    strncasecmp(filename, lib_xmahali, strlen(lib_xmahali)) == 0 ||
    strncasecmp(filename, lib_xmahalltcg, strlen(lib_xmahalltcg)) == 0 ||
    strncasecmp(filename, lib_xmcore, strlen(lib_xmcore)) == 0 ||
    strncasecmp(filename, lib_xmcored, strlen(lib_xmcored)) == 0 ||
    strncasecmp(filename, lib_xmcorei, strlen(lib_xmcorei)) == 0 ||
    strncasecmp(filename, lib_xmcoreltcg, strlen(lib_xmcoreltcg)) == 0 ||
    strncasecmp(filename, lib_xmedia2, strlen(lib_xmedia2)) == 0 ||
    strncasecmp(filename, lib_xmediad2, strlen(lib_xmediad2)) == 0 ||
    strncasecmp(filename, lib_xmic, strlen(lib_xmic)) == 0 ||
    strncasecmp(filename, lib_xmicd, strlen(lib_xmicd)) == 0 ||
    strncasecmp(filename, lib_xmp, strlen(lib_xmp)) == 0 ||
    strncasecmp(filename, lib_xmpd, strlen(lib_xmpd)) == 0 ||
    //strncasecmp(filename, lib_xnet, strlen(lib_xnet)) == 0 ||
    strncasecmp(filename, lib_xnetconfiginfo, strlen(lib_xnetconfiginfo)) == 0 ||
    strncasecmp(filename, lib_xnetconfiginfod, strlen(lib_xnetconfiginfod)) == 0 ||
    //strncasecmp(filename, lib_xnetd, strlen(lib_xnetd)) == 0 ||
    //strncasecmp(filename, lib_xonline, strlen(lib_xonline)) == 0 ||
    //strncasecmp(filename, lib_xonlined, strlen(lib_xonlined)) == 0 ||
    strncasecmp(filename, lib_xparty, strlen(lib_xparty)) == 0 ||
    strncasecmp(filename, lib_xpartyd, strlen(lib_xpartyd)) == 0 ||
    strncasecmp(filename, lib_xrnm, strlen(lib_xrnm)) == 0 ||
    strncasecmp(filename, lib_xrnmd, strlen(lib_xrnmd)) == 0 ||
    strncasecmp(filename, lib_xrnms, strlen(lib_xrnms)) == 0 ||
    strncasecmp(filename, lib_xrnmsd, strlen(lib_xrnmsd)) == 0 ||
    strncasecmp(filename, lib_xsim, strlen(lib_xsim)) == 0 ||
    strncasecmp(filename, lib_xsimd, strlen(lib_xsimd)) == 0 ||
    strncasecmp(filename, lib_xsocialpost, strlen(lib_xsocialpost)) == 0 ||
    strncasecmp(filename, lib_xsocialpostd, strlen(lib_xsocialpostd)) == 0 ||
    strncasecmp(filename, lib_xstudio, strlen(lib_xstudio)) == 0 ||
    strncasecmp(filename, lib_xtms, strlen(lib_xtms)) == 0 ||
    strncasecmp(filename, lib_xtmsd, strlen(lib_xtmsd)) == 0 ||
    strncasecmp(filename, lib_xuihtml, strlen(lib_xuihtml)) == 0 ||
    strncasecmp(filename, lib_xuihtmld, strlen(lib_xuihtmld)) == 0 ||
    strncasecmp(filename, lib_xuirender, strlen(lib_xuirender)) == 0 ||
    strncasecmp(filename, lib_xuirenderd, strlen(lib_xuirenderd)) == 0 ||
    strncasecmp(filename, lib_xuirenderltcg, strlen(lib_xuirenderltcg)) == 0 ||
    strncasecmp(filename, lib_xuirun, strlen(lib_xuirun)) == 0 ||
    strncasecmp(filename, lib_xuiruna, strlen(lib_xuiruna)) == 0 ||
    strncasecmp(filename, lib_xuirunad, strlen(lib_xuirunad)) == 0 ||
    strncasecmp(filename, lib_xuirund, strlen(lib_xuirund)) == 0 ||
    strncasecmp(filename, lib_xuirunltcg, strlen(lib_xuirunltcg)) == 0 ||
    strncasecmp(filename, lib_xuivideo, strlen(lib_xuivideo)) == 0 ||
    strncasecmp(filename, lib_xuivideod, strlen(lib_xuivideod)) == 0 ||
    strncasecmp(filename, lib_xwmadecode, strlen(lib_xwmadecode)) == 0 ||
    strncasecmp(filename, lib_xwmadecoded, strlen(lib_xwmadecoded)) == 0 ||
    strncasecmp(filename, lib_retaildump, strlen(lib_retaildump)) == 0);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief Output a formatted string to messages window [analog of printf()]
///     only when the verbose flag of plugin's options is true
/// @param  format const char * printf() style message string.
/// @return void
/// @author TQN
/// @date 2004.09.11
 ////////////////////////////////////////////////////////////////////////////////
void showMsg(const char *format, ...)
{
    if (g_options.bVerbose)
    {
        va_list va;
        va_start(va, format);
        (void) vmsg(format, va);
        va_end(va);
    }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief Show options dialog for getting user desired options
/// @return void
/// @author TQN
/// @date 2004.09.11
////////////////////////////////////////////////////////////////////////////////
static void showOptionsDlg(void)
{
    // Build the format string constant used to create the dialog
    const char format[] =
        "STARTITEM 0\n"                             // TabStop
        "LoadMap Options\n"                         // Title
        "<Apply Map Symbols for Name:R>\n"          // Radio Button 0
        "<Apply Map Symbols for Comment:R>>\n"    // Radio Button 1
        "<Replace Existing Names/Comments:C>>\n"  // Checkbox Button
        "<Show verbose messages:C>>\n\n";           // Checkbox Button

    // Create the option dialog.
    short name = (g_options.bNameApply ? 0 : 1);
    short replace = (g_options.bReplace ? 1 : 0);
    short verbose = (g_options.bVerbose ? 1 : 0);
    if (ask_form(format, &name, &replace, &verbose))
    {
        g_options.bNameApply = (0 == name);
        g_options.bReplace = (1 == replace);
        g_options.bVerbose = (1 == verbose);
    }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief Plugin initialize function
/// @return PLUGIN_KEEP always
/// @author TQN
/// @date 2004.09.11
 ////////////////////////////////////////////////////////////////////////////////
int idaapi init(void)
{
    msg("\nLoadMap: Plugin v%s init.\n\n",PLUG_VERSION);

    // Get the full path to user config dir
	qstrncpy(g_szIniPath, get_user_idadir(), sizeof(g_szIniPath));
	qstrncat(g_szIniPath, "loadmap.cfg", sizeof(g_szIniPath));
    g_szIniPath[sizeof(g_szIniPath) - 1] = '\0';

    // Get options saved in cfg file
    read_config_file("loadmap", g_optsinfo, qnumber(g_optsinfo), NULL);

    switch (inf.filetype)
    {
    case f_PE:
    case f_COFF:
    case f_LE:
    case f_LX:
    case f_ELF:
    case f_EXE:
    case f_LOADER:
        return PLUGIN_KEEP;
    }
    return PLUGIN_SKIP;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief Plugin run function, which does the actual job
/// @param   int    Not used
/// @return void
/// @author TQN
/// @date 2004.09.11
////////////////////////////////////////////////////////////////////////////////
bool idaapi run(size_t)
{
    static char mapFileName[_MAX_PATH] = { 0 };

    // If user press shift key, show options dialog
    if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
    {
        showOptionsDlg();
    }

    unsigned long numOfSegs = get_segm_qty();
    if (0 == numOfSegs)
    {
        warning("Not found any segments");
        return false;
    }

    if ('\0' == mapFileName[0])
    {
        // First run (after all, mapFileName is static)
        get_input_file_path(mapFileName, sizeof(mapFileName));
        pathExtensionSwitch(mapFileName, ".map", sizeof(mapFileName));
    }

    // Show open map file dialog
    char *fname = ask_file(0, mapFileName, "Open MAP file");
    if (NULL == fname)
    {
        msg("LoadMap: User cancel\n");
        return false;
    }

    // Open the map file
    char * pMapStart = NULL;
    size_t mapSize = INVALID_MAPFILE_SIZE;
    MapFile::MAPResult eRet = MapFile::openMAP(fname, pMapStart, mapSize);
    switch (eRet)
    {
        case MapFile::WIN32_ERROR:
            warning("Could not open file '%s'.\nWin32 Error Code = 0x%08X",
                    fname, GetLastError());
            return false;

        case MapFile::FILE_EMPTY_ERROR:
            warning("File '%s' is empty, zero size", fname);
            return false;

        case MapFile::FILE_BINARY_ERROR:
            warning("File '%s' seem to be a binary or Unicode file", fname);
            return false;

        case MapFile::OPEN_NO_ERROR:
        default:
            break;
    }

    MapFile::SectionType sectnHdr = MapFile::NO_SECTION;
    unsigned long sectnNumber = 0;
    unsigned long validSyms = 0;
    unsigned long invalidSyms = 0;

    // The mark pointer to the end of memory map file
    // all below code must not read or write at and over it
    const char * pMapEnd = pMapStart + mapSize;

    show_wait_box("Parsing and applying symbols from the Map file '%s'", fname);

    try
    {
        const char * pLine = pMapStart;
        const char * pEOL = pMapStart;
        MapFile::MAPSymbol sym;
        MapFile::MAPSymbol prvsym;
        sym.seg = SREG_NUM;
        sym.addr = BADADDR;
        sym.name[0] = '\0';
        bool inStaticsSection = false;

        std::string curLibName = "";
        bool curLibIsXboxLibrary = false;

        while (pLine < pMapEnd)
        {
            // Skip the spaces, '\r', '\n' characters, blank lines, seek to the
            // non space character at the beginning of a non blank line
            pLine = MapFile::skipSpaces(pEOL, pMapEnd);

            // Find the EOL '\r' or '\n' characters
            pEOL = MapFile::findEOL(pLine, pMapEnd);

            size_t lineLen = (size_t) (pEOL - pLine);
            if (lineLen < g_minLineLen)
            {
                continue;
            }
            char fmt[80];
            fmt[0] = '\0';

            // Check if we're on section header or section end
            if (sectnHdr == MapFile::NO_SECTION)
            {
                sectnHdr = MapFile::recognizeSectionStart(pLine, lineLen);
                if (sectnHdr != MapFile::NO_SECTION)
                {
                    sectnNumber++;
                    qsnprintf(fmt, sizeof(fmt), "Section start line: '%%.%ds'.\n", lineLen);
                    showMsg(fmt, pLine);
                    continue;
                }
            } else
            {
                sectnHdr = MapFile::recognizeSectionEnd(sectnHdr, pLine, lineLen);
                if (sectnHdr == MapFile::NO_SECTION)
                {
                    qsnprintf(fmt, sizeof(fmt), "Section end line: '%%.%ds'.\n", lineLen);
                    showMsg(fmt, pLine);
                    continue;
                }
            }
            MapFile::ParseResult parsed;
            prvsym.seg = sym.seg;
            prvsym.addr = sym.addr;
            qstrncpy(prvsym.name,sym.name,sizeof(sym.name));
            sym.seg = SREG_NUM;
            sym.addr = BADADDR;
            sym.name[0] = '\0';
            parsed = MapFile::INVALID_LINE;

            switch (sectnHdr)
            {
            case MapFile::NO_SECTION:
                parsed = MapFile::SKIP_LINE;
                break;
            case MapFile::MSVC_MAP:
            case MapFile::BCCL_NAM_MAP:
            case MapFile::BCCL_VAL_MAP:
                parsed = parseMsSymbolLine(sym,pLine,lineLen,g_minLineLen,numOfSegs);
                break;
            case MapFile::WATCOM_MAP:
                parsed = parseWatcomSymbolLine(sym,pLine,lineLen,g_minLineLen,numOfSegs);
                break;
            case MapFile::GCC_MAP:
                parsed = parseGccSymbolLine(sym,pLine,lineLen,g_minLineLen,numOfSegs);
                break;
            }
            if (parsed == MapFile::STATICS_LINE)
              inStaticsSection = true;

            if (parsed == MapFile::SKIP_LINE || parsed == MapFile::STATICS_LINE)
            {
                qsnprintf(fmt, sizeof(fmt), "Skipping line: '%%.%ds'.\n", lineLen);
                showMsg(fmt, pLine);
                continue;
            }
            if (parsed == MapFile::FINISHING_LINE)
            {
                sectnHdr = MapFile::NO_SECTION;
                // we have parsed to end of value/name symbols table or reached EOF
                qsnprintf(fmt, sizeof(fmt), "Parsing finished at line: '%%.%ds'.\n", lineLen);
                showMsg(fmt, pLine);
                continue;
            }
            if (parsed == MapFile::INVALID_LINE)
            {
                invalidSyms++;
                qsnprintf(fmt, sizeof(fmt), "Invalid map line: %%.%ds.\n", lineLen);
                showMsg(fmt, pLine);
                continue;
            }
            // If shouldn't apply names
            bool bNameApply = g_options.bNameApply;
            if (parsed == MapFile::COMMENT_LINE)
            {
                qsnprintf(fmt, sizeof(fmt), "Comment line: %%.%ds.\n", lineLen);
                showMsg(fmt, pLine);
                if (BADADDR == sym.addr)
                    continue;
            }
            // Determine the DeDe map file
            char *pname = sym.name;
            if (('<' == pname[0]) && ('-' == pname[1]))
            {
                // Functions indicator symbol of DeDe map
                pname += 2;
                bNameApply = true;
            }
            else if ('*' == pname[0])
            {
                // VCL controls indicator symbol of DeDe map
                pname++;
                bNameApply = false;
            }
            else if (('-' == pname[0]) && ('>' == pname[1]))
            {
                // VCL methods indicator symbol of DeDe map
                pname += 2;
                bNameApply = false;
            }

            unsigned long la = sym.addr + getnseg((int) sym.seg)->start_ea;
            flags_t f = get_full_flags(la);

            if (bNameApply) // Apply symbols for name
            {
                //  Add name if there's no meaningful name assigned.
                if (g_options.bReplace ||
                    (!has_name(f) || has_dummy_name(f) || has_auto_name(f)))
                {
                    if (set_name(la, pname, SN_NOWARN | SN_FORCE))
                    {
                        showMsg("%04X:%08X - Change name to '%s' succeeded\n",
                            sym.seg, la, pname);
                        validSyms++;
                    }
                    else
                    {
                        showMsg("%04X:%08X - Change name to '%s' failed\n",
                            sym.seg, la, pname);
                        invalidSyms++;
                    }
                }
                if (sym.type == 'f')
                {

                  // force IDA to recognize addr as code, so we can add it as a library function
                  auto_make_proc(la);
                  auto_recreate_insn(la);

                  if (strcmp(curLibName.c_str(), sym.libname) != 0)
                  {
                    curLibName = sym.libname;
                    curLibIsXboxLibrary = IsXboxLibraryFile(sym.libname);
                  }

                  flags_t flags = 0;

                  if (curLibIsXboxLibrary)
                    flags |= FUNC_LIB;

                  if (inStaticsSection)
                    flags |= FUNC_STATICDEF;

                  if (flags != 0)
                  {
                    func_t* existing = get_func(la);
                    if (existing)
                    {
                      existing->flags |= flags;
                      update_func(existing);
                    }
                    else
                    {
                      func_t func(la, BADADDR, flags);
                      add_func_ex(&func);
                    }
                  }
                }
            }
            else if (g_options.bReplace || !has_cmt(f))
            {
                // Apply symbols for comment
                if (set_cmt(la, pname, false))
                {
                    showMsg("%04X:%08X - Change comment to '%s' succeeded\n",
                        sym.seg, la, pname);
                    validSyms++;
                }
                else
                {
                    showMsg("%04X:%08X - Change comment to '%s' failed\n",
                        sym.seg, la, pname);
                    invalidSyms++;
                }
            }
        }

    }
    catch (...)
    {
        warning("Exception while parsing MAP file '%s'");
        invalidSyms++;
    }
    MapFile::closeMAP(pMapStart);
    hide_wait_box();

    if (sectnNumber == 0)
    {
        warning("File '%s' is not a valid Map file; publics section header wasn't found", fname);
    }
    else
    {
        // Save file name for next askfile_c dialog
        qstrncpy(mapFileName, fname, sizeof(mapFileName));

        // Show the result
        msg("Result of loading and parsing the Map file '%s'\n"
            "   Number of Symbols applied: %d\n"
            "   Number of Invalid Symbols: %d\n\n",
            fname, validSyms, invalidSyms);
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief Plugin terminate callback function
/// @return void
/// @author TQN
/// @date 2004.09.11
////////////////////////////////////////////////////////////////////////////////
void idaapi term(void)
{
    msg("LoadMap: Plugin v%s terminate.\n",PLUG_VERSION);

    // Write the plugin's options to cfg file
    /*_VERIFY(WritePrivateProfileStruct(g_szLoadMapSection, g_szOptionsKey, &g_options,
                                      sizeof(g_options), g_szIniPath));
    The old, windows-centric way is now disabled. Instead, we should open g_szIniPath
    and write config in normal IDA format (like the files in IDA/cfg filder).
    */
}

////////////////////////////////////////////////////////////////////////////////
/// @name Plugin information
/// @{
char wanted_name[]   = "Load Symbols From MAP File";
char wanted_hotkey[] = "Ctrl-M";
char comment[]       = "LoadMap loads symbols from a VC/BC/Watcom/Dede map file.";
char help[]          = "LoadMap, Visual C/Borland C/Watcom C/Dede map file import plugin."
                              "This module reads selected map file, and loads symbols\n"
                              "into IDA database. Click it while holding Shift to see options.";
/// @}

////////////////////////////////////////////////////////////////////////////////
/// @brief Plugin description block
extern "C" {
plugin_t PLUGIN =
{
    IDP_INTERFACE_VERSION,
    0,                    // Plugin flags
    init,                 // Initialize
    term,                 // Terminate
    run,                  // Main function
    comment,              // Comment about the plugin
    help,
    wanted_name,          // preferred short name of the plugin
    wanted_hotkey         // preferred hotkey to run the plugin
};
};
////////////////////////////////////////////////////////////////////////////////
