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
#define PLUG_VERSION "1.0"
//  standard library headers.
#include <cstdio>
// Makes gcc stdlib to not define non-underscored versions of non-ANSI functions (ie memicmp, strlwr)
#define _NO_OLDNAMES
#include <cstring>
#undef _NO_OLDNAMES
#define USE_STANDARD_FILE_FUNCTIONS
#include <hexrays.hpp>

//  other headers.
#include  "MAPReader.h"
#include "stdafx.h"

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
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <filesystem>

hexdsp_t* hexdsp = nullptr;

void linearAddressToSymbolAddr(MapFile::MAPSymbol &sym, unsigned long linear_addr)
{
    sym.seg = get_segm_num(linear_addr);
    segment_t * sseg = getnseg((int) sym.seg);
    if (sseg != NULL)
        sym.addr = linear_addr - sseg->start_ea;
    else
        sym.addr = -1;
}

typedef struct _tagPLUGIN_OPTIONS {
    int bVerbose;      //< show detail messages
} PLUGIN_OPTIONS;

const size_t g_minLineLen = 14; // For a "xxxx:xxxxxxxx " line

static char g_szIniPath[MAXPATH] = { 0 };


/// @brief Global variable for options of plugin
static PLUGIN_OPTIONS g_options = { 0 };

static const cfgopt_t g_optsinfo[] =
{
	cfgopt_t("VERBOSE_MESSAGES", &g_options.bVerbose, 0, 1),
};

////////////////////////////////////////////////////////////////////////////////
/// @name Ini Section and Key names
/// @{
static char g_szLoadMapSection[] = "MapSourceGen";
static char g_szOptionsKey[] = "Options";
/// @}

////////////////////////////////////////////////////////////////////////////////
/// @brief Output a formatted string to messages window [analog of printf()]
///     only when the verbose flag of plugin's options is true
/// @param  format const char * printf() style message string.
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
/// @author TQN
/// @date 2004.09.11
////////////////////////////////////////////////////////////////////////////////
static void showOptionsDlg(void)
{
    // Build the format string constant used to create the dialog
    const char format[] =
        "STARTITEM 0\n"                             // TabStop
        "MapSourceGen Options\n"                         // Title
        "<Show verbose messages:C>>\n\n";           // Checkbox Button

    // Create the option dialog.
    short verbose = (g_options.bVerbose ? 1 : 0);
    if (ask_form(format, &verbose))
    {
        g_options.bVerbose = (1 == verbose);
    }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief Plugin initialize function
/// @return PLUGIN_KEEP always
/// @author TQN
/// @date 2004.09.11
 ////////////////////////////////////////////////////////////////////////////////
plugmod_t* idaapi init(void)
{
    if ( !init_hexrays_plugin() )
        return nullptr; // no decompiler
    
    msg("\nMapSourceGenerator: Plugin v%s init.\n\n",PLUG_VERSION);

    // Get the full path to user config dir
	qstrncpy(g_szIniPath, get_user_idadir(), sizeof(g_szIniPath));
	qstrncat(g_szIniPath, "mapsourcegen.cfg", sizeof(g_szIniPath));
    g_szIniPath[sizeof(g_szIniPath) - 1] = '\0';

    // Get options saved in cfg file
    read_config_file("mapsourcegen", g_optsinfo, qnumber(g_optsinfo), NULL);

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

std::string makeFileName(const char* libname)
{
    bool bIsCPP = true;
    const char* extString = strstr(libname, ".cpp");
    if (extString == nullptr) {
        extString = strstr(libname, ".c");
        bIsCPP = false;
        if (extString == nullptr) {
            return {};
        }
    }

    const char* stringBegin = extString;
    --stringBegin;
    while (true) {
        if (*(stringBegin - 1) == '.' || *(stringBegin - 1) == ' ' || *(stringBegin - 1) == '\0') {
            break;
        }

        --stringBegin;
    }

    std::string outString = std::string(stringBegin, extString - stringBegin);
    outString += (bIsCPP ? ".cpp" : ".c");

    return outString;
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

    std::unordered_map<std::string, std::fstream> filesMap;

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

    std::string folderPath = mapFileName;
    size_t folderOffset = folderPath.find_last_of('\\');
    if (folderOffset == size_t(-1)) {
        folderOffset = folderPath.find_last_of('/');
    }
    
    folderPath.erase(folderOffset + 1, folderPath.size() - folderOffset - 1);
    folderPath.append("sources/");

    std::error_code err;
    std::filesystem::remove_all(folderPath, err);
    std::filesystem::create_directory(folderPath, err);
    
    // Show open map file dialog
    char *fname = ask_file(0, mapFileName, "Open MAP file");
    if (NULL == fname)
    {
        msg("MapSourceGen: User cancel\n");
        return false;
    }

    char * pMapStart = NULL;
	size_t mapSize = INVALID_MAPFILE_SIZE;
	const MapFile::MAPResult eRet = MapFile::openMAP(mapFileName, pMapStart, mapSize);
	switch (eRet) {
	case MapFile::WIN32_ERROR:
		printf("Could not open file '%s'.\n", mapFileName);
		return -1;

	case MapFile::FILE_EMPTY_ERROR:
		printf("File '%s' is empty, zero size", mapFileName);
		return -1;

	case MapFile::FILE_BINARY_ERROR:
		printf("File '%s' seem to be a binary or Unicode file", mapFileName);
		return -1;

	case MapFile::OPEN_NO_ERROR:
	default:
		break;
	}

    show_wait_box("Generating sources for '%s'", fname);

	MapFile::SectionType sectnHdr = MapFile::NO_SECTION;
	unsigned long sectnNumber = 0;
	unsigned long generated = 0;
	unsigned long invalidSyms = 0;

	// The mark pointer to the end of memory map file
	// all below code must not read or write at and over it
	const char * pMapEnd = pMapStart + mapSize;

	    try
    {
        const char * pLine = pMapStart;
        const char * pEOL = pMapStart;
        MapFile::MAPSymbol sym;
        MapFile::MAPSymbol prvsym;
        sym.seg = 16;
        sym.addr = -1;
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
            if (lineLen < 14)
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
                    qsnprintf(fmt, sizeof(fmt), "Section start line: '%%.%ds'.\n",  (int)lineLen);
                    continue;
                }
            } else
            {
                sectnHdr = MapFile::recognizeSectionEnd(sectnHdr, pLine, lineLen);
                if (sectnHdr == MapFile::NO_SECTION)
                {
                    qsnprintf(fmt, sizeof(fmt), "Section end line: '%%.%ds'.\n", (int)lineLen);
                    continue;
                }
            }
            MapFile::ParseResult parsed;
            prvsym.seg = sym.seg;
            prvsym.addr = sym.addr;
            qstrncpy(prvsym.name, sym.name, sizeof(sym.name));
            sym.seg = 16;
            sym.addr = -1;
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
                parsed = parseMsSymbolLine(sym,pLine,lineLen,14,9/*numOfSegs*/);
                break;
            case MapFile::WATCOM_MAP:
                parsed = parseWatcomSymbolLine(sym,pLine,lineLen,14,9/*numOfSegs*/);
                break;
            case MapFile::GCC_MAP:
                parsed = parseGccSymbolLine(sym,pLine,lineLen,14,9/*numOfSegs*/);
                break;
            }
            
            if (parsed == MapFile::STATICS_LINE)
              inStaticsSection = true;

            if (parsed == MapFile::SKIP_LINE || parsed == MapFile::STATICS_LINE)
            {
                qsnprintf(fmt, sizeof(fmt), "Skipping line: '%%.%ds'.\n",  (int)lineLen);
                continue;
            }
            if (parsed == MapFile::FINISHING_LINE)
            {
                sectnHdr = MapFile::NO_SECTION;
                // we have parsed to end of value/name symbols table or reached EOF
                qsnprintf(fmt, sizeof(fmt), "Parsing finished at line: '%%.%ds'.\n",  (int)lineLen);
                continue;
            }
            if (parsed == MapFile::INVALID_LINE)
            {
                invalidSyms++;
                qsnprintf(fmt, sizeof(fmt), "Invalid map line: %%.%ds.\n",  (int)lineLen);
                continue;
            }

            unsigned long la = sym.addr + getnseg((int) sym.seg)->start_ea;
            flags_t f = get_full_flags(la);
            
            curLibName = sym.libname;
            curLibIsXboxLibrary = MapFile::isXboxLibraryFile(sym.libname);
            if (sym.type == 'f') {
                auto_make_proc(la);
                auto_recreate_insn(la);

                func_t *pfn = get_func(la);
                if (pfn == nullptr) {
                    continue;
                }

                hexrays_failure_t hf;
                cfuncptr_t cfunc = decompile(pfn, &hf, DECOMP_NO_WAIT);
                const strvec_t& sv = cfunc->get_pseudocode();
                if (sv.empty()) {
                    continue;
                }

                std::string fileName = makeFileName(sym.libname);
                if (!filesMap[fileName].is_open()) {
                    std::string filePath = folderPath + fileName;
                    filesMap[fileName].open(filePath, std::ios::out | std::ios::binary);
                    if (!filesMap[fileName].is_open()) {
                        continue;
                    }
                }

                for (const auto& line : sv) {
                    qstring buf;
                    tag_remove(&buf, line.line);
                    filesMap[fileName] << buf.c_str();
                    filesMap[fileName] << "\n";
                }

                filesMap[fileName] << "\n";
                filesMap[fileName].flush();
                generated++;
            }
        }
    }
    catch (...)
    {
        printf("Exception while parsing MAP file");
        invalidSyms++;
    }

    MapFile::closeMAP(pMapStart);
    filesMap.clear();
    
    hide_wait_box();
    
    msg("results for %s file: \nGenerated function : %d\nInvalid symbols: %d", fname, generated, invalidSyms);

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
    msg("MapSourceGenerator: Plugin v%s terminate.\n",PLUG_VERSION);
}

////////////////////////////////////////////////////////////////////////////////
/// @name Plugin information
/// @{
char wanted_name[]   = "Generate source tree from .MAP";
char wanted_hotkey[] = "Ctrl-L";
char comment[]       = "Generate source tree from a VC/BC/Watcom/Dede map file.";
char help[]          = "MapSourceGenerator, Visual C/Borland C/Watcom C/Dede map file import plugin."
                              "This module reads selected map file, and generates source tree from symbols.\n"
                              "Click it while holding Shift to see options.";
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
