/* Copyright 2017 MaulingMonkey

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#ifndef ZMMK_IG_SRC_ELF_TYPES_HPP
#define ZMMK_IG_SRC_ELF_TYPES_HPP

//    Sources:
// https://en.wikipedia.org/wiki/Executable_and_Linkable_Format
// http://www.skyfree.org/linux/references/ELF_Format.pdf
// http://www.dwarfstd.org/doc/dwarf-2.0.0.pdf

#include "dwarf.leb128.hpp"

namespace mmk { namespace debug { namespace elf {
	typedef unsigned char  u8;
	typedef unsigned short u16;
	typedef unsigned int   u32;
	typedef unsigned int   off;
	typedef unsigned int   addr;

	typedef signed char  s8;
	typedef signed short s16;
	typedef signed int   s32;

	enum fileBitness { fileBitness32 = 1, fileBitness64 = 2 };
	enum fileEndian  { fileEndianLittle = 1, fileEndianBig = 2 };
	enum fileType {
		fileTypeRelocatable = 1,
		fileTypeExecutable  = 2,
		fileTypeShared      = 3,
		fileTypeCore        = 4,
	};
	enum fileArch {
		fileArchNone    = 0x00,
		fileArchx86     = 0x03,
		fileArchPPC     = 0x14,
		fileArchARM     = 0x28,
		fileArchIA64    = 0x32,
		fileArchx86_64  = 0x3E,
		fileArchAArch64 = 0xB7,
	};
	struct fileHeader {
		struct {
			char magic[4];             // "\x7fELF"
			u8   bitness;              // bitness
			u8   endian;               // endian
			u8   version;              // 1
			u8   abi;                  // likely 0 despite not being system v
			u8   abiVersion;           // likely unused
			u8   _padding0[7];         // unused
		} ident;

		u16  type;                     // fileType
		u16  arch;                     // arch
		u32  version;                  // 1
		off  entryPoint;               // 
		off  programHeaderTable;       // likely 0x34 / 0x40
		off  sectionHeaderTable;       //
		u32  archFlags;                //
		u16  size;                     // sizeof(mmkDebugElfFileHeader) ? (52/64)
		u16  programHeaderEntrySize;   // ~ sizeof(mmkDebugElfProgramHeader32)
		u16  programHeaderCount;       // # of mmkDebugElfProgramHeader32 s
		u16  sectionHeaderEntrySize;   // ~ sizeof(mmkDebugElfSectionHeader)
		u16  sectionHeaderCount;       // # of mmkDebugElfSectionHeader s
		u16  sectionHeaderNamesIndex;
	};



	enum programType {
		programHeaderTypeNull          = 0,
		programHeaderTypeLoad          = 1,
		programHeaderTypeDynamic       = 2,
		programHeaderTypeInterp        = 3,
		programHeaderTypeNote          = 4,
		programHeaderTypeSharedLibrary = 5,
		programHeaderTypePHDR          = 6,
	
		// Reserved for OS
		programHeaderTypeLowOS         = 0x60000000,
		programHeaderTypeHighOS        = 0x6FFFFFFF,
		// Reserved for Processor
		programHeaderTypeLowProcessor  = 0x70000000,
		programHeaderTypeHighProcessor = 0x7FFFFFFF,
	};
	struct programHeader {
		u32  type;            // mmkDebugElfProgramHeaderType
		off  offset;          // on disk
		addr virtualAddress;  // in memory
		addr physicalAddress; // if relevant
		u32  diskSize;        // compressed/on disk/in elf
		u32  memorySize;      // uncompressed/in memory
		u32  flags;           // type dependant
		u32  alignment;       // may be 0
	};



	enum sectionType {
		sectionTypeNull                      =  0,
		sectionTypeProgramBits               =  1,
		sectionTypeSymbolTable               =  2,
		sectionTypeStringTable               =  3,
		sectionTypeRelocationExplicitAddends =  4,
		sectionTypeSymbolHashTable           =  5, // For dynamic linking
		sectionTypeDynamicLinking            =  6, // Only one allowed
		sectionTypeNotes                     =  7,
		sectionTypeNoBits                    =  8, // Reserved for program use like ProgramBits, but no size.
		sectionTypeRelocation                =  9,
		sectionTypeSHLIB                     = 10, // Reserved
		sectionTypeLowProcessor              = 0x70000000, // Reserved for per-arch use
		sectionTypeHighProcessor             = 0x7fffffff, // Reserved for per-arch use
		sectionTypeLowUser                   = 0x80000000, // Reserved for program use
		sectionTypeHighUser                  = 0xffffffff, // Reserved for program use
	};
	struct sectionHeader {
		u32  nameShstrtabOffset; // .shstrtab section offset
		u32  type;               // sectionType
		u32  flags;              //
		off  virtualAddress;     //
		off  fileOffset;         //
		u32  size;               // of the section
		u32  linkedSectionIndex; //
		u32  extraInfo;          // depends on type
		u32  alignment;          // 0, pow2
		u32  entrySize;          //
	};
}}} // namespace mmk::debug::readElf

#endif /* ndef ZMMK_IG_SRC_ELF_TYPES_HPP */
