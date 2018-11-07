#include "PCH.h"
#include "DisassemblerGCNISA.h"

//! external
#include "AmdDxGsaCompile.h"
#include "devices.h"
#include "elf32.h"

// Done with pretty much the same fashion as in RenderDoc.

#define MAKE_FOURCC(a, b, c, d) \
  (((uint32_t)(d) << 24) | ((uint32_t)(c) << 16) | ((uint32_t)(b) << 8) | (uint32_t)(a))


namespace
{
	static const char* errMessage = "; Unable to load atidxx64.dll";

	// todo: Extend.
	HMODULE GetAMDModule()
	{
		//char chCWD[256];
		//GetCurrentDirectoryA(256, chCWD);

		HMODULE hModule = ::LoadLibraryA( "atidxx64.dll" );
		return hModule;	
	}
}




CDisassemblerGCNISA::CDisassemblerGCNISA()
{
	m_module = GetAMDModule();
}

CDisassemblerGCNISA::~CDisassemblerGCNISA()
{
	// Free AMD GCN ISA module
	::FreeLibrary( m_module );
}

//------------------------------------------------------------------------
std::string CDisassemblerGCNISA::Compile( unsigned char* pData, unsigned int length, E_ASIC_TYPE asicType )
{
	if ( NULL == m_module )
	{
		return std::string( errMessage );
	}

	// Obtain pointers to functions
	PfnAmdDxGsaCompileShader compileShader = (PfnAmdDxGsaCompileShader) ::GetProcAddress(m_module, "AmdDxGsaCompileShader");
	PfnAmdDxGsaFreeCompiledShader freeShader = (PfnAmdDxGsaFreeCompiledShader) ::GetProcAddress(m_module, "AmdDxGsaFreeCompiledShader");
	if ( (compileShader == nullptr) || (freeShader == nullptr) )
	{
		return std::string("One or more neccessary functions were not found in provided DLL");
	}

	AmdDxGsaCompileShaderInput in = {};
	AmdDxGsaCompileShaderOutput out = {};
	AmdDxGsaCompileOption compileOptions[1];

	in.inputType = AmdDxGsaInputType::GsaInputDxAsmBin;
	in.numCompileOptions = 0;
	in.pCompileOptions = compileOptions;

	switch (asicType)
	{
		case AT_TAHITI:
			in.chipFamily = FAMILY_SI;
			in.chipRevision = SI_TAHITI_P_B1;
			break;

		case AT_PITCAIRN:
			in.chipFamily = FAMILY_SI;
			in.chipRevision = SI_PITCAIRN_PM_A1;
			break;

		case AT_CAPEVERDE:
			in.chipFamily = FAMILY_SI;
			in.chipRevision = SI_CAPEVERDE_M_A1;
			break;

		case AT_OLAND:
			in.chipFamily = FAMILY_SI;
			in.chipRevision = SI_OLAND_M_A0;
			break;

		case AT_HAINAN:
			in.chipFamily = FAMILY_SI;
			in.chipRevision = SI_HAINAN_V_A0;
			break;

		case AT_BONAIRE:
			in.chipFamily = FAMILY_CI;
			in.chipRevision = CI_BONAIRE_M_A0;
			break;

		case AT_HAWAII:
			in.chipFamily = FAMILY_CI;
			in.chipRevision = CI_HAWAII_P_A0;
			break;

		case AT_KALINDI:
			in.chipFamily = FAMILY_CI;
			in.chipRevision = CI_BONAIRE_M_A0;
			break;

		case AT_SPECTRE:
			in.chipFamily = FAMILY_CI;
			in.chipRevision = KV_SPECTRE_A0;
			break;

		case AT_SPOOKY:
			in.chipFamily = FAMILY_CI;
			in.chipRevision = KV_SPOOKY_A0;
			break;

		case AT_ICELAND:
			in.chipFamily = FAMILY_VI;
			in.chipRevision = VI_ICELAND_M_A0;
			break;

		case AT_TONGA:
			in.chipFamily = FAMILY_VI;
			in.chipRevision = VI_TONGA_P_A0;
			break;

		case AT_CARRIZO:
			in.chipFamily = FAMILY_CZ;
			in.chipRevision = CARRIZO_A0;
			break;

		case AT_FIJI:
			in.chipFamily = FAMILY_VI;
			in.chipRevision = VI_FIJI_P_A0;
			break;
	}

	bool amdil = false;

	const uint8_t* base = (const uint8_t*) pData;
	const uint32_t* end = (const uint32_t*) (base + length );
	const uint32_t* dxbc = (const uint32_t*) base;

	if (*dxbc != MAKE_FOURCC('D', 'X', 'B', 'C') )
	{
		return std::string("different magic");
	}

	dxbc++;		// fourcc
	dxbc += 4;	// hash
	dxbc++;		// unknown
	dxbc++;		// filelength

	if (dxbc >= end)
		return std::string("eof");

	const uint32_t numChunks = *dxbc;

	std::vector<uint32_t> chunkOffsets;
	for (uint32_t i=0; i < numChunks; i++ )
	{
		if (dxbc >= end)
		{
			return std::string("err");
		}

		chunkOffsets.push_back( *dxbc );
		dxbc++;
	}

	in.pShaderByteCode = nullptr;
	in.byteCodeLength = 0;

	for ( uint32_t offs : chunkOffsets )
	{
		dxbc = (const uint32_t*) (base + offs );

		if (*dxbc == MAKE_FOURCC('S', 'H', 'E', 'X') || *dxbc == MAKE_FOURCC('S', 'H', 'D', 'R') )
		{
			dxbc++;
			in.byteCodeLength = *dxbc;
			dxbc++;
			in.pShaderByteCode = dxbc;

			if (dxbc + (in.byteCodeLength / 4) > end )
			{
				return "err";
			}

			break;
		}
	}

	if (in.byteCodeLength == 0)
	{
		return "err";
	}

	out.size = sizeof(out);


	compileShader(&in, &out);

	if (out.pShaderBinary == nullptr || out.shaderBinarySize < 16)
		return "failed to disassemble shader";


	const uint8_t* elf = (const uint8_t*) out.pShaderBinary;
	const Elf32_Ehdr *elfHeader = (const Elf32_Ehdr*) elf;

	std::string ret;

	// Minimal code to extract data from elf. We assume the ELF is well-formed.
	if ( IS_ELF(*elfHeader) && elfHeader->e_ident[EI_CLASS] == ELFCLASS32 )
	{
		const Elf32_Shdr *strtab =
			(const Elf32_Shdr *)( elf + elfHeader->e_shoff + sizeof(Elf32_Shdr) * elfHeader->e_shstrndx);

		const uint8_t* strtabData = elf + strtab->sh_offset;

		const AmdDxGsaCompileStats* pStats = nullptr;

		for (int section=1; section < elfHeader->e_shnum; section++)
		{
			if ( section == elfHeader->e_shstrndx )
				continue;

			const Elf32_Shdr* sectHeader = 
				(const Elf32_Shdr*) (elf + elfHeader->e_shoff + sizeof(Elf32_Shdr) * section );

			const char *name = (const char*)(strtabData + sectHeader->sh_name );
			const uint8_t* data = elf + sectHeader->sh_offset;

			if (!strcmp(name, ".stats"))
			{
				pStats = (AmdDxGsaCompileStats*) data;
			}
			else if (amdil && !strcmp(name, ".amdil_disassembly"))
			{
				ret.insert(0, (const char*) data, (size_t) sectHeader->sh_size);
			}
			else if (!amdil && !strcmp(name, ".disassembly"))
			{
				ret.insert(0, (const char*) data, (size_t) sectHeader->sh_size);
			}
		}

		if ( pStats && !amdil )
		{
			char chBuffer[8192];
			sprintf_s(chBuffer, 
					   "; -------- Statistics --------------------\n"
					   "SGPRs: %d out of %d used\n"
					   "VGPRs: %d out of %d used\n"
					   "LDS: %d out of %d used\n"
					   "%d bytes scratch space used\n"
					   "instructions: %d ALU, %d control flow, %d TFETCH\n\n",

					   pStats->numSgprsUsed, pStats->availableSgprs, pStats->numVgprsUsed, pStats->availableVgprs,
					   pStats->usedLdsBytes, pStats->availableLdsBytes, pStats->usedScratchBytes, pStats->numAluInst,
					   pStats->numControlFlowInst, pStats->numTfetchInst );

			const std::string sBuffer(chBuffer);
			ret.insert( ret.begin(), sBuffer.begin(), sBuffer.end() );
		}
	}
	else
	{
		return "invalid elf";
	}

	// Free memory
	freeShader(out.pShaderBinary);

	return ret;
}
