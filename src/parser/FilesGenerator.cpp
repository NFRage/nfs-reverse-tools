#include <fstream>
#include <vector>

#include "../MAPReader.h"

std::streampos fileSize(std::fstream& stream ){

	std::streampos fsize = 0;
	fsize = stream.tellg();
	stream.seekg( 0, std::ios::end );
	fsize = stream.tellg() - fsize;
	stream.seekg( 0, std::ios::beg );
	
	return fsize;
}

std::vector<char> fileData;

int main(int argc, char *argv[])
{
	if (argc < 2) {
		printf("No input file provided. Aborting...");
		return -1;
	}

	std::fstream file;
	file.open(argv[1], std::ios::in | std::ios::binary);
	if (!file.is_open()) {
		printf("Can't open file %s. Aborting...", argv[1]);
		return -1;
	}

	fileData.resize(fileSize(file));
	file.read(fileData.data(), fileData.size());
	file.close();

	char * pMapStart = NULL;
	size_t mapSize = INVALID_MAPFILE_SIZE;
	const MapFile::MAPResult eRet = MapFile::openMAP(argv[1], pMapStart, mapSize);
	switch (eRet) {
	case MapFile::WIN32_ERROR:
		printf("Could not open file '%s'.\n", argv[1]);
		return -1;

	case MapFile::FILE_EMPTY_ERROR:
		printf("File '%s' is empty, zero size", argv[1]);
		return -1;

	case MapFile::FILE_BINARY_ERROR:
		printf("File '%s' seem to be a binary or Unicode file", argv[1]);
		return -1;

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
                    snprintf(fmt, sizeof(fmt), "Section start line: '%%.%ds'.\n", lineLen);
                    continue;
                }
            } else
            {
                sectnHdr = MapFile::recognizeSectionEnd(sectnHdr, pLine, lineLen);
                if (sectnHdr == MapFile::NO_SECTION)
                {
                    snprintf(fmt, sizeof(fmt), "Section end line: '%%.%ds'.\n", lineLen);
                    continue;
                }
            }
            MapFile::ParseResult parsed;
            prvsym.seg = sym.seg;
            prvsym.addr = sym.addr;
            strncpy(prvsym.name, sym.name, sizeof(sym.name));
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
                snprintf(fmt, sizeof(fmt), "Skipping line: '%%.%ds'.\n", lineLen);
                continue;
            }
            if (parsed == MapFile::FINISHING_LINE)
            {
                sectnHdr = MapFile::NO_SECTION;
                // we have parsed to end of value/name symbols table or reached EOF
                snprintf(fmt, sizeof(fmt), "Parsing finished at line: '%%.%ds'.\n", lineLen);
                continue;
            }
            if (parsed == MapFile::INVALID_LINE)
            {
                invalidSyms++;
                snprintf(fmt, sizeof(fmt), "Invalid map line: %%.%ds.\n", lineLen);
                continue;
            }
        }
    }
    catch (...)
    {
        printf("Exception while parsing MAP file");
        invalidSyms++;
    }

	return 0;
}