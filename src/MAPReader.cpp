////////////////////////////////////////////////////////////////////////////////
/// @file MAPReader.h
///     MAP file analysis and loading routines.
/// @par Purpose:
///     Library for loading MAP file entries.
/// @author TQN <truong_quoc_ngan@yahoo.com>
/// @author TL <mefistotelis@gmail.com>
/// @date 2004.09.11 - 2018.11.08
/// @par  Copying and copyrights:
///     This program is free software; you can redistribute it and/or modify
///     it under the terms of the GNU General Public License as published by
///     the Free Software Foundation; either version 2 of the License, or
///     (at your option) any later version.
////////////////////////////////////////////////////////////////////////////////

#include  "MAPReader.h"

#include  <cstring>
#include  <cctype>
#include  <cassert>
#include  <cstdlib>
#include <string>
#include <vector>
#include <iterator>
#include <sstream>
#include "stdafx.h"

using namespace std;

namespace MapFile {

/// @name Strings used to identify start of symbol table in various MAP files.
/// @{
const char MSVC_HDR_START[]        = "Address         Publics by Value              Rva+Base     Lib:Object";
const char MSVC_HDR_START2[]       = "Address         Publics by Value              Rva+Base       Lib:Object";
const char BCCL_HDR_NAME_START[]   = "Address         Publics by Name";
const char BCCL_HDR_VALUE_START[]  = "Address         Publics by Value";
const char WATCOM_MEMMAP_START[]   = "Address        Symbol";
const char WATCOM_MEMMAP_SKIP[]   = "=======        ======";
const char WATCOM_MEMMAP_COMMENT[] = "Module: ";
const char WATCOM_END_TABLE_HDR[]  = "+----------------------+";
const char MSVC_LINE_NUMBER[]      = "Line numbers for ";
const char MSVC_FIXUP[]            = "FIXUPS: ";
const char MSVC_EXPORTS[]          = " Exports";
const char GCC_MEMMAP_START[]      = "Linker script and memory map";
const char GCC_MEMMAP_SKIP1[]       = ".";
const char GCC_MEMMAP_SKIP2[]       = " .";
const char GCC_MEMMAP_SKIP3[]       = "*";
const char GCC_MEMMAP_SKIP4[]       = " *";
const char GCC_MEMMAP_END[]        = "OUTPUT(";
const char GCC_MEMMAP_LOAD[]       = "LOAD ";

/// @}

};

bool MapFile::isXboxLibraryFile(const char* filename)
{
  // eww.. for some reason this is the fastest method to check all these..
    
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
/// @brief Open a map file and map the file content to virtual memory
/// @param lpszFileName  Path name of file to open.
/// @param dwSize Out variable to receive size of file.
/// @param lpMapAddr The pointer to memory address of mapped file
/// @return enum value of OPEN_FILE_ERROR
/// @author TQN
/// @date 2004.09.12
////////////////////////////////////////////////////////////////////////////////
MapFile::MAPResult MapFile::openMAP(const char * fileName, char * &mapAddr, size_t &dwSize)
{
    // Set default values for output parameters
    mapAddr = NULL;
    dwSize = INVALID_MAPFILE_SIZE;

    // Validate all input pointer parameters
    assert(NULL != fileName);
    if (NULL == fileName)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return WIN32_ERROR;
    }

    // Open the file
    HANDLE hFile = CreateFile(fileName, GENERIC_READ, FILE_SHARE_READ, NULL,
                              OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (INVALID_HANDLE_VALUE == hFile)
    {
        return WIN32_ERROR;
    }

    dwSize = GetFileSize(hFile, NULL);
    if ((INVALID_MAPFILE_SIZE == dwSize) || (0 == dwSize))
    {
        // File too large or empty
        WIN32CHECK(CloseHandle(hFile));
        return ((0 == dwSize) ? FILE_EMPTY_ERROR : WIN32_ERROR);
    }

    HANDLE hMap = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
    if (NULL == hMap)
    {
        WIN32CHECK(CloseHandle(hFile));
        return WIN32_ERROR;
    }

    // Mapping creation successful, do not need file handle anymore
    WIN32CHECK(CloseHandle(hFile));

    mapAddr = (LPSTR) MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, dwSize);
    if (NULL == mapAddr)
    {
        WIN32CHECK(CloseHandle(hMap));
        return WIN32_ERROR;
    }

    // Map View successful, do not need the map handle anymore
    WIN32CHECK(CloseHandle(hMap));

    if (NULL != memchr(mapAddr, 0, dwSize))
    {
        // File is binary or Unicode file
        WIN32CHECK(UnmapViewOfFile(mapAddr));
        mapAddr = NULL;
        return FILE_BINARY_ERROR;
    }

    return OPEN_NO_ERROR;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief Close memory map file which opened by MemMapFileOpen function.
/// @param lpAddr: Pointer to memory return by MemMapFileOpen.
/// @author TQN
/// @date 2004.09.12
////////////////////////////////////////////////////////////////////////////////
void MapFile::closeMAP(const void * lpAddr)
{
    WIN32CHECK(UnmapViewOfFile(lpAddr));
}

////////////////////////////////////////////////////////////////////////////////
/// @brief Seek to non space character at the beginning of a memory buffer.
/// @param  lpStart Pointer to start of buffer
/// @param  lpEnd Pointer to end of buffer
/// @return Pointer to first non space character at the beginning of buffer
/// @author TQN
/// @date 2004.09.12
////////////////////////////////////////////////////////////////////////////////
const char * MapFile::skipSpaces(const char * pStart, const char * pEnd)
{
    assert(pStart != NULL);
    assert(pEnd != NULL);
    assert(pStart <= pEnd);

    const char * p = pStart;
    while ((p < pEnd) && isspace(*p))
    {
        p++;
    }

    return p;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief Find the EOL character '\r' or '\n' in a memory buffer
/// @param  lpStart LPSTR Pointer to start of buffer
/// @param  lpEnd LPSTR Pointer to end of buffer
/// @return LPSTR Pointer to first EOL character in the buffer
/// @author TQN
/// @date 2004.09.12
////////////////////////////////////////////////////////////////////////////////
const char * MapFile::findEOL(const char * pStart, const char * pEnd)
{
    assert(pStart != NULL);
    assert(pEnd != NULL);
    assert(pStart <= pEnd);

    const char * p = pStart;
    while ((p < pEnd) && ('\r' != *p) && ('\n' != *p))
    {
        p++;
    }

    return p;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief Checks if a line is the starting line of a section to be analyzed.
/// @param  pLine Pointer to start of buffer
/// @param  lineLen Length of the current line
/// @return Type of the new section, or NO_SECTION
/// @author TL
/// @date 2011.09.10
////////////////////////////////////////////////////////////////////////////////
MapFile::SectionType MapFile::recognizeSectionStart(const char *pLine, size_t lineLen)
{
    if (strncasecmp(pLine, MSVC_HDR_START, lineLen) == 0 || strncasecmp(pLine, MSVC_HDR_START2, lineLen) == 0)
        return MapFile::MSVC_MAP;
    if (strncasecmp(pLine, BCCL_HDR_NAME_START, lineLen) == 0)
        return MapFile::BCCL_NAM_MAP;
    if (strncasecmp(pLine, BCCL_HDR_VALUE_START, lineLen) == 0)
        return MapFile::BCCL_VAL_MAP;
    if (strncasecmp(pLine, WATCOM_MEMMAP_START, lineLen) == 0)
        return MapFile::WATCOM_MAP;
    if (strncasecmp(pLine, GCC_MEMMAP_START, lineLen) == 0)
        return MapFile::GCC_MAP;
    return MapFile::NO_SECTION;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief Checks if a line is the ending line of a section we analyzed.
/// @param secType Type of the opened section.
/// @param  pLine Pointer to start of buffer
/// @param  lineLen Length of the current line
/// @return Type of the new section, or value of secType if no change
/// @author TL
/// @date 2011.09.10
////////////////////////////////////////////////////////////////////////////////
MapFile::SectionType MapFile::recognizeSectionEnd(MapFile::SectionType secType, const char *pLine, size_t lineLen)
{
    switch (secType)
    {
    case MapFile::MSVC_MAP:
        if (strncmp(pLine, MSVC_LINE_NUMBER, std::strlen(MSVC_LINE_NUMBER)) == 0)
            return MapFile::NO_SECTION;
        if (strncmp(pLine, MSVC_FIXUP, std::strlen(MSVC_FIXUP)) == 0)
            return MapFile::NO_SECTION;
        if (strncmp(pLine, MSVC_EXPORTS, std::strlen(MSVC_EXPORTS)) == 0)
            return MapFile::NO_SECTION;
        break;
    case MapFile::BCCL_NAM_MAP:
    case MapFile::BCCL_VAL_MAP:
        break;
    case MapFile::WATCOM_MAP:
        if (strncmp(pLine, WATCOM_END_TABLE_HDR, std::strlen(WATCOM_END_TABLE_HDR)) == 0)
            return MapFile::NO_SECTION;
        break;
    case MapFile::GCC_MAP:
        if (strncmp(pLine, GCC_MEMMAP_END, std::strlen(GCC_MEMMAP_END)) == 0)
            return MapFile::NO_SECTION;
        break;
    }
    return secType;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief Reads one entry of Ms-like MAP file.
/// @param sym Target  buffer for symbol data.
/// @param  pLine Pointer to start of buffer
/// @param  lineLen Length of the current line
/// @param  minLineLen Minimal accepted length of line
/// @param numOfSegs Number of segments in IDA, used to verify segment number range
/// @return Result of the parsing
/// @author TL
/// @date 2011.09.10
////////////////////////////////////////////////////////////////////////////////
MapFile::ParseResult MapFile::parseMsSymbolLine(MapFile::MAPSymbol &sym, const char *pLine, size_t lineLen, size_t minLineLen, size_t numOfSegs)
{
    // Skip any entrypoint / "static symbols" lines
    const char* testStr = "entry point at";
    if(strncasecmp(pLine, testStr, strlen(testStr)) == 0)
      return MapFile::SKIP_LINE;
    const char* testStr2 = "Static symbols";
    if (strncasecmp(pLine, testStr2, strlen(testStr2)) == 0)
      return MapFile::STATICS_LINE;

    // Get segment number, address, name, by pass spaces at beginning,
    // between ':' character, between address and name
    long lineCut = lineLen;
    if (lineCut > MAXNAMELEN + minLineLen)
        lineCut = MAXNAMELEN + minLineLen;
    std::string testLine((const char*)pLine, lineCut);
  /*  size_t off = std::string::npos;
    while (off = testLine.find("  ") != std::string::npos)
    {
      testLine = testLine.replace(off, 2, " ");
    }
    if (testLine.find(' ') == 0)
      testLine = testLine.substr(1);*/

    std::istringstream iss(testLine);
    std::vector<std::string> results((std::istream_iterator<std::string>(iss)),
      std::istream_iterator<std::string>());

    // Parse line parts
    if(results.size() < 3)
      return MapFile::FINISHING_LINE; // Failed, we must have parsed to end of value/name symbols table or reached EOF

    auto sepIdx = results[0].find(':');
    sym.seg = std::stoi(results[0], 0, 0x10);
    sym.addr = std::stoi(results[0].substr(sepIdx+1), 0, 0x10);

    auto& name = results[1];
    auto& fulladdr = results[2];
    std::string type = "";
    if (results.size() > 3)
      sym.type = results[3].at(0);
    else
      sym.type = 0;

    strcpy(sym.name, name.c_str());

    auto& libname = results[results.size() - 1];
    if (libname.length())
      strcpy(sym.libname, libname.c_str());

    if ((0 == sym.seg) || (--sym.seg >= numOfSegs) ||
            (-1 == sym.addr) || (std::strlen(sym.name) == 0) )
    {
        return MapFile::INVALID_LINE;
    }
    // Ensure name is NULL terminated
    sym.name[MAXNAMELEN] = '\0';
    sym.libname[260] = '\0';
    return MapFile::SYMBOL_LINE;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief Reads one entry of Watcom-like MAP file.
/// @param sym Target  buffer for symbol data.
/// @param  pLine Pointer to start of buffer
/// @param  lineLen Length of the current line
/// @param  minLineLen Minimal accepted length of line
/// @param numOfSegs Number of segments in IDA, used to verify segment number range
/// @return Result of the parsing
/// @author TL
/// @date 2011.09.10
////////////////////////////////////////////////////////////////////////////////
MapFile::ParseResult MapFile::parseWatcomSymbolLine(MapFile::MAPSymbol &sym, const char *pLine, size_t lineLen, size_t minLineLen, size_t numOfSegs)
{
    // Get segment number, address, name, by pass spaces at beginning,
    // between ':' character, between address and name
    long lineCut = lineLen;
    if (lineCut > MAXNAMELEN + minLineLen)
        lineCut = MAXNAMELEN + minLineLen;
    char * dupLine = (char *)std::malloc(lineCut+1);
    strncpy(dupLine,pLine,lineCut);
    dupLine[lineCut] = '\0';
    if (strncasecmp(dupLine, ";", 1) == 0)
    {
        strncpy(sym.name,dupLine+1,MAXNAMELEN-1);
        sym.name[MAXNAMELEN] = '\0';
        std::free(dupLine);
        return MapFile::COMMENT_LINE;
    }
    if (strncasecmp(dupLine, WATCOM_MEMMAP_SKIP, std::strlen(WATCOM_MEMMAP_SKIP)) == 0)
    {
        std::free(dupLine);
        return MapFile::SKIP_LINE;
    }
    if (strncasecmp(dupLine, WATCOM_MEMMAP_COMMENT, std::strlen(WATCOM_MEMMAP_COMMENT)) == 0)
    {
        strncpy(sym.name,dupLine+std::strlen(WATCOM_MEMMAP_COMMENT),MAXNAMELEN-1);
        sym.name[MAXNAMELEN] = '\0';
        std::free(dupLine);
        return MapFile::COMMENT_LINE;
    }
    int ret = sscanf(dupLine, " %04X : %08X%*c %[^\t\n;]", &sym.seg, &sym.addr, sym.name);
    std::free(dupLine);
    if (3 != ret)
    {
        // we have parsed to end of value/name symbols table or reached EOF
        return MapFile::FINISHING_LINE;
    }
    else if ((0 == sym.seg) || (--sym.seg >= numOfSegs) ||
            (-1 == sym.addr) || (std::strlen(sym.name) == 0) )
    {
        return MapFile::INVALID_LINE;
    }
    // Ensure name is NULL terminated
    sym.name[MAXNAMELEN] = '\0';
    return MapFile::SYMBOL_LINE;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief Reads one entry of GCC-like MAP file.
/// @param sym Target  buffer for symbol data.
/// @param  pLine Pointer to start of buffer
/// @param  lineLen Length of the current line
/// @param  minLineLen Minimal accepted length of line
/// @param numOfSegs Number of segments in IDA, used to verify segment number range
/// @return Result of the parsing
/// @author TL
/// @date 2012.07.18
////////////////////////////////////////////////////////////////////////////////
MapFile::ParseResult MapFile::parseGccSymbolLine(MapFile::MAPSymbol &sym, const char *pLine, size_t lineLen, size_t minLineLen, size_t numOfSegs)
{
    // Get segment number, address, name, by pass spaces at beginning,
    // between ':' character, between address and name
    long lineCut = lineLen;
    if (lineCut > MAXNAMELEN + minLineLen)
        lineCut = MAXNAMELEN + minLineLen;
    char * dupLine = (char *)std::malloc(lineCut+1);
    strncpy(dupLine,pLine,lineCut);
    dupLine[lineCut] = '\0';
    if (strncasecmp(dupLine, ";", 1) == 0)
    {
        strncpy(sym.name,dupLine+1,MAXNAMELEN-1);
        sym.name[MAXNAMELEN] = '\0';
        std::free(dupLine);
        return MapFile::COMMENT_LINE;
    }
    if ( (strncasecmp(dupLine, GCC_MEMMAP_SKIP1, std::strlen(GCC_MEMMAP_SKIP1)) == 0) ||
         (strncasecmp(dupLine, GCC_MEMMAP_SKIP2, std::strlen(GCC_MEMMAP_SKIP2)) == 0) )
    {
        std::free(dupLine);
        return MapFile::SKIP_LINE;
    }
    if ( (strncasecmp(dupLine, GCC_MEMMAP_SKIP3, std::strlen(GCC_MEMMAP_SKIP3)) == 0) ||
         (strncasecmp(dupLine, GCC_MEMMAP_SKIP4, std::strlen(GCC_MEMMAP_SKIP4)) == 0) )
    {
        std::free(dupLine);
        return MapFile::SKIP_LINE;
    }
    if (strncasecmp(dupLine, GCC_MEMMAP_LOAD, std::strlen(GCC_MEMMAP_LOAD)) == 0)
    {
        strncpy(sym.name,dupLine,MAXNAMELEN-1);
        sym.name[MAXNAMELEN] = '\0';
        std::free(dupLine);
        return MapFile::COMMENT_LINE;
    }
    unsigned long linear_addr;
    int ret = sscanf(dupLine, " 0x%08X%*c %[^\t\n;]", &linear_addr, sym.name);
    std::free(dupLine);
    if (2 != ret)
    {
        // we have parsed to end of value/name symbols table or reached EOF
        return MapFile::FINISHING_LINE;
    }
    linearAddressToSymbolAddr(sym, linear_addr);
    if ((sym.seg >= numOfSegs) || (-1 == sym.addr) || (std::strlen(sym.name) == 0) )
    {
        return MapFile::INVALID_LINE;
    }
    // Ensure name is NULL terminated
    sym.name[MAXNAMELEN] = '\0';
    return MapFile::SYMBOL_LINE;
}

////////////////////////////////////////////////////////////////////////////////
