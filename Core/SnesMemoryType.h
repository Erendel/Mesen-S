#pragma once

#pragma pack(push, 1)
enum class SnesMemoryType : uint8_t
{
	CpuMemory,
	SpcMemory,
	Sa1Memory,
	NecDspMemory,
	GsuMemory,
	Cx4Memory,
	GameboyMemory,
	PrgRom,
	WorkRam,
	SaveRam,
	VideoRam,
	SpriteRam,
	CGRam,
	SpcRam,
	SpcRom,
	DspProgramRom,
	DspDataRom,
	DspDataRam,
	Sa1InternalRam,
	GsuWorkRam,
	Cx4DataRam,
	BsxPsRam,
	BsxMemoryPack,
	GbPrgRom,
	GbWorkRam,
	GbCartRam,
	GbHighRam,
	GbBootRom,
	GbVideoRam,
	GbSpriteRam,
	Register
};
#pragma pack(pop)
