#pragma once


enum E_ASIC_TYPE
{
	AT_TAHITI,
	AT_PITCAIRN,
	AT_CAPEVERDE,
	AT_OLAND,
	AT_HAINAN,
	AT_BONAIRE,
	AT_HAWAII,
	AT_KALINDI,
	AT_SPECTRE,
	AT_SPOOKY,
	AT_ICELAND,
	AT_TONGA,
	AT_CARRIZO,
	AT_FIJI
};


class CDisassemblerGCNISA
{
public:
	CDisassemblerGCNISA();
	~CDisassemblerGCNISA();

	std::string Compile(unsigned char* data, unsigned int length, E_ASIC_TYPE asicType);

private:
	HMODULE	m_module;
};