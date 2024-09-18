#include "lib.h"
#ifdef __APPLE__
#define clang_path "/opt/homebrew/opt/llvm/bin/clang"
#else
#define clang_path "/usr/bin/clang"
#endif

#define RawIndex(T, i) (T ## Index) { .index = (i) }
#define Index(T, i) RawIndex(T, (i) + 1)
#define geti(i) ((i).index - 1)
#define validi(i) ((i).index != 0)
#define invalidi(T) RawIndex(T, 0)

#define InternPool(T) InternPool_ ## T
#define GetOrPut(T) T ## GetOrPut
#define declare_ip(T) \
STRUCT(InternPool(T)) \
{ \
    T ## Index * pointer; \
    u32 length;\
    u32 capacity;\
}; \
STRUCT(GetOrPut(T)) \
{\
    T ## Index index; \
    u8 existing;\
}

fn int dir_make(const char* path)
{
    return syscall_mkdir(path, 0755);
}

fn String file_read(Arena* arena, String path)
{
    String result = {};
    int file_descriptor = syscall_open(string_to_c(path), 0, 0);
    assert(file_descriptor != -1);

    struct stat stat_buffer;
    int stat_result = syscall_fstat(file_descriptor, &stat_buffer);
    assert(stat_result == 0);

    auto file_size = cast(u64, s64, stat_buffer.st_size);

    result = (String){
        .pointer = arena_allocate_bytes(arena, file_size, 64),
        .length = file_size,
    };

    // TODO: big files
    ssize_t read_result = syscall_read(file_descriptor, result.pointer, result.length);
    assert(read_result >= 0);
    assert((u64)read_result == file_size);

    auto close_result = syscall_close(file_descriptor);
    assert(close_result == 0);

    return result;
}

fn void print_string(String message)
{
#ifndef SILENT
        ssize_t result = syscall_write(1, message.pointer, message.length);
        assert(result >= 0);
        assert((u64)result == message.length);
#else
        unused(message);
#endif
}

STRUCT(ElfRelocation)
{
    u64 offset;
    u64 info;
    u64 addend;
};

typedef enum ElfDynamicEntryTag : s64
{
    DT_NULL = 0,
    DT_NEEDED = 1,
    DT_PLTRELSZ = 2,
    DT_PLTGOT = 3,
    DT_HASH = 4,
    DT_STRTAB = 5,
    DT_SYMTAB = 6,
    DT_RELA = 7,
    DT_RELASZ = 8,
    DT_RELAENT = 9,
    DT_STRSZ = 10,
    DT_SYMENT = 11,
    DT_INIT = 12,
    DT_FINI = 13,
    DT_SONAME = 14,
    DT_RPATH = 15,
    DT_SYMBOLIC = 16,
    DT_REL = 17,
    DT_RELSZ = 18,
    DT_RELENT = 19,
    DT_PLTREL = 20,
    DT_DEBUG = 21,
    DT_TEXTREL = 22,
    DT_JMPREL = 23,
    DT_BIND_NOW = 24,
    DT_INIT_ARRAY = 25,
    DT_FINI_ARRAY = 26,
    DT_INIT_ARRAYSZ = 27,
    DT_FINI_ARRAYSZ = 28,
    DT_RUNPATH = 29,
    DT_FLAGS = 30,
    DT_ENCODING = 32,
    DT_PREINIT_ARRAY = 32,
    DT_PREINIT_ARRAYSZ = 33,
    DT_SYMTAB_SHNDX = 34,
    DT_RELRSZ = 35,
    DT_RELR = 36,
    DT_RELRENT = 37,
    DT_NUM = 38,
    DT_LOOS = 0x6000000d,
    DT_HIOS = 0x6ffff000,
    DT_LOPROC = 0x70000000,
    DT_HIPROC = 0x7fffffff,
    DT_VALRNGLO = 0x6ffffd00,
    DT_GNU_PRELINKED = 0x6ffffdf5,
    DT_GNU_CONFLICTSZ = 0x6ffffdf6,
    DT_GNU_LIBLISTSZ = 0x6ffffdf7,
    DT_CHECKSUM = 0x6ffffdf8,
    DT_PLTPADSZ = 0x6ffffdf9,
    DT_MOVEENT = 0x6ffffdfa,
    DT_MOVESZ = 0x6ffffdfb,
    DT_FEATURE_1 = 0x6ffffdfc,
    DT_POSFLAG_1 = 0x6ffffdfd,

    DT_SYMINSZ = 0x6ffffdfe,
    DT_SYMINENT = 0x6ffffdff,
    DT_VALRNGHI = 0x6ffffdff,
    DT_VALNUM = 12,

    DT_ADDRRNGLO = 0x6ffffe00,
    DT_GNU_HASH = 0x6ffffef5,
    DT_TLSDESC_PLT = 0x6ffffef6,
    DT_TLSDESC_GOT = 0x6ffffef7,
    DT_GNU_CONFLICT = 0x6ffffef8,
    DT_GNU_LIBLIST = 0x6ffffef9,
    DT_CONFIG = 0x6ffffefa,
    DT_DEPAUDIT = 0x6ffffefb,
    DT_AUDIT = 0x6ffffefc,
    DT_PLTPAD = 0x6ffffefd,
    DT_MOVETAB = 0x6ffffefe,
    DT_SYMINFO = 0x6ffffeff,
    DT_ADDRRNGHI = 0x6ffffeff,
    DT_ADDRNUM = 11,

    DT_VERSYM = 0x6ffffff0,

    DT_RELACOUNT = 0x6ffffff9,
    DT_RELCOUNT = 0x6ffffffa,

    DT_FLAGS_1 = 0x6ffffffb,
    DT_VERDEF = 0x6ffffffc,

    DT_VERDEFNUM = 0x6ffffffd,
    DT_VERNEED = 0x6ffffffe,

    DT_VERNEEDNUM = 0x6fffffff,
    DT_VERSIONTAGNUM = 16,

    DT_AUXILIARY = 0x7ffffffd,
    DT_FILTER = 0x7fffffff,
    DT_EXTRANUM = 3,

    DT_SPARC_REGISTER = 0x70000001,
    DT_SPARC_NUM = 2,

    DT_MIPS_RLD_VERSION = 0x70000001,
    DT_MIPS_TIME_STAMP = 0x70000002,
    DT_MIPS_ICHECKSUM = 0x70000003,
    DT_MIPS_IVERSION = 0x70000004,
    DT_MIPS_FLAGS = 0x70000005,
    DT_MIPS_BASE_ADDRESS = 0x70000006,
    DT_MIPS_MSYM = 0x70000007,
    DT_MIPS_CONFLICT = 0x70000008,
    DT_MIPS_LIBLIST = 0x70000009,
    DT_MIPS_LOCAL_GOTNO = 0x7000000a,
    DT_MIPS_CONFLICTNO = 0x7000000b,
    DT_MIPS_LIBLISTNO = 0x70000010,
    DT_MIPS_SYMTABNO = 0x70000011,
    DT_MIPS_UNREFEXTNO = 0x70000012,
    DT_MIPS_GOTSYM = 0x70000013,
    DT_MIPS_HIPAGENO = 0x70000014,
    DT_MIPS_RLD_MAP = 0x70000016,
    DT_MIPS_DELTA_CLASS = 0x70000017,
    DT_MIPS_DELTA_CLASS_NO = 0x70000018,

    DT_MIPS_DELTA_INSTANCE = 0x70000019,
    DT_MIPS_DELTA_INSTANCE_NO = 0x7000001a,

    DT_MIPS_DELTA_RELOC = 0x7000001b,
    DT_MIPS_DELTA_RELOC_NO = 0x7000001c,

    DT_MIPS_DELTA_SYM = 0x7000001d,

    DT_MIPS_DELTA_SYM_NO = 0x7000001e,

    DT_MIPS_DELTA_CLASSSYM = 0x70000020,

    DT_MIPS_DELTA_CLASSSYM_NO = 0x70000021,

    DT_MIPS_CXX_FLAGS = 0x70000022,
    DT_MIPS_PIXIE_INIT = 0x70000023,
    DT_MIPS_SYMBOL_LIB = 0x70000024,
    DT_MIPS_LOCALPAGE_GOTIDX = 0x70000025,
    DT_MIPS_LOCAL_GOTIDX = 0x70000026,
    DT_MIPS_HIDDEN_GOTIDX = 0x70000027,
    DT_MIPS_PROTECTED_GOTIDX = 0x70000028,
    DT_MIPS_OPTIONS = 0x70000029,
    DT_MIPS_INTERFACE = 0x7000002a,
    DT_MIPS_DYNSTR_ALIGN = 0x7000002b,
    DT_MIPS_INTERFACE_SIZE = 0x7000002c,
    DT_MIPS_RLD_TEXT_RESOLVE_ADDR = 0x7000002d,

    DT_MIPS_PERF_SUFFIX = 0x7000002e,

    DT_MIPS_COMPACT_SIZE = 0x7000002f,
    DT_MIPS_GP_VALUE = 0x70000030,
    DT_MIPS_AUX_DYNAMIC = 0x70000031,

    DT_MIPS_PLTGOT = 0x70000032,

    DT_MIPS_RWPLT = 0x70000034,
    DT_MIPS_RLD_MAP_REL = 0x70000035,
    DT_MIPS_NUM = 0x36,

    DT_ALPHA_PLTRO = (DT_LOPROC + 0),
    DT_ALPHA_NUM = 1,

    DT_PPC_GOT = (DT_LOPROC + 0),
    DT_PPC_OPT = (DT_LOPROC + 1),
    DT_PPC_NUM = 2,

    DT_PPC64_GLINK = (DT_LOPROC + 0),
    DT_PPC64_OPD = (DT_LOPROC + 1),
    DT_PPC64_OPDSZ = (DT_LOPROC + 2),
    DT_PPC64_OPT = (DT_LOPROC + 3),
    DT_PPC64_NUM = 4,

    DT_IA_64_PLT_RESERVE = (DT_LOPROC + 0),
    DT_IA_64_NUM = 1,
} ElfDynamicEntryTag;

STRUCT(ElfDynamicEntry)
{
    ElfDynamicEntryTag tag;
    union
    {
        u64 address;
        s64 value;
    };
};

typedef enum ELFSectionType : u32
{
    ELF_SECTION_NULL = 0X00,
    ELF_SECTION_PROGRAM = 0X01,
    ELF_SECTION_SYMBOL_TABLE = 0X02,
    ELF_SECTION_STRING_TABLE = 0X03,
    ELF_SECTION_RELOCATION_WITH_ADDENDS = 0X04,
    ELF_SECTION_SYMBOL_HASH_TABLE = 0X05,
    ELF_SECTION_DYNAMIC = 0X06,
    ELF_SECTION_NOTE = 0X07,
    ELF_SECTION_BSS = 0X08,
    ELF_SECTION_RELOCATION_NO_ADDENDS = 0X09,
    ELF_SECTION_LIB = 0X0A, // RESERVED
    ELF_SECTION_DYNAMIC_SYMBOL_TABLE = 0X0B,
    ELF_SECTION_INIT_ARRAY = 0X0E,
    ELF_SECTION_FINI_ARRAY = 0X0F,
    ELF_SECTION_PREINIT_ARRAY = 0X10,
    ELF_SECTION_GROUP = 0X11,
    ELF_SECTION_SYMBOL_TABLE_SECTION_HEADER_INDEX = 0X12,
    ELF_SECTION_GNU_HASH = 0x6ffffff6,
    ELF_SECTION_GNU_VERDEF = 0x6ffffffd,
    ELF_SECTION_GNU_VERNEED = 0x6ffffffe,
    ELF_SECTION_GNU_VERSYM = 0x6fffffff,

} ELFSectionType;

fn String elf_section_type_to_string(ELFSectionType type)
{
    switch (type)
    {
        case_to_name(ELF_SECTION_, NULL);
        case_to_name(ELF_SECTION_, PROGRAM);
        case_to_name(ELF_SECTION_, SYMBOL_TABLE);
        case_to_name(ELF_SECTION_, STRING_TABLE);
        case_to_name(ELF_SECTION_, RELOCATION_WITH_ADDENDS);
        case_to_name(ELF_SECTION_, SYMBOL_HASH_TABLE);
        case_to_name(ELF_SECTION_, DYNAMIC);
        case_to_name(ELF_SECTION_, NOTE);
        case_to_name(ELF_SECTION_, BSS);
        case_to_name(ELF_SECTION_, RELOCATION_NO_ADDENDS);
        case_to_name(ELF_SECTION_, LIB);
        case_to_name(ELF_SECTION_, DYNAMIC_SYMBOL_TABLE);
        case_to_name(ELF_SECTION_, INIT_ARRAY);
        case_to_name(ELF_SECTION_, FINI_ARRAY);
        case_to_name(ELF_SECTION_, PREINIT_ARRAY);
        case_to_name(ELF_SECTION_, GROUP);
        case_to_name(ELF_SECTION_, SYMBOL_TABLE_SECTION_HEADER_INDEX);
        case_to_name(ELF_SECTION_, GNU_HASH);
        case_to_name(ELF_SECTION_, GNU_VERDEF);
        case_to_name(ELF_SECTION_, GNU_VERNEED);
        case_to_name(ELF_SECTION_, GNU_VERSYM);
      break;
    }
}

STRUCT(ELFSectionHeaderFlags)
{
    u64 write:1;
    u64 alloc:1;
    u64 executable:1;
    u64 blank:1;
    u64 merge:1;
    u64 strings:1;
    u64 info_link:1;
    u64 link_order:1;
    u64 os_non_conforming:1;
    u64 group:1;
    u64 tls:1;
    u64 reserved:53;
};
static_assert(sizeof(ELFSectionHeaderFlags) == sizeof(u64));

STRUCT(ELFSectionHeader)
{
    u32 name_offset;
    ELFSectionType type;
    ELFSectionHeaderFlags flags;
    u64 address;
    u64 offset;
    u64 size;
    u32 link;
    u32 info;
    u64 alignment;
    u64 entry_size;
};
static_assert(sizeof(ELFSectionHeader) == 64);
decl_vb(ELFSectionHeader);

typedef enum ElfProgramHeaderType : u32
{
    PT_NULL = 0,
    PT_LOAD = 1,
    PT_DYNAMIC = 2,
    PT_INTERP = 3,
    PT_NOTE = 4,
    PT_SHLIB = 5,
    PT_PHDR = 6,
    PT_TLS = 7,
    PT_GNU_EH_FRAME	= 0x6474e550, /* GCC .eh_frame_hdr segment */
    PT_GNU_STACK	= 0x6474e551, /* Indicates stack executability */
    PT_GNU_RELRO	= 0x6474e552, /* Read-only after relocation */
    PT_GNU_PROPERTY	= 0x6474e553, /* GNU property */
    PT_GNU_SFRAME	= 0x6474e554, /* SFrame segment.  */
} ElfProgramHeaderType;

STRUCT(ElfProgramHeaderFlags)
{
    u32 executable:1;
    u32 writeable:1;
    u32 readable:1;
    u32 reserved:29;
};

STRUCT(ElfProgramHeader)
{
    ElfProgramHeaderType type;
    ElfProgramHeaderFlags flags;
    u64 offset;
    u64 virtual_address;
    u64 physical_address;
    u64 file_size;
    u64 memory_size;
    u64 alignment;
};
static_assert(sizeof(ElfProgramHeader) == 0x38);
declare_slice(ElfProgramHeader);
decl_vb(ElfProgramHeader);

typedef enum ELFBitCount : u8
{
    bits32 = 1,
    bits64 = 2,
} ELFBitCount;

typedef enum ELFEndianness : u8
{
    little = 1,
    big = 2,
} ELFEndianness;

typedef enum ELFAbi : u8
{
    ELF_ABI_SYSTEM_V = 0,
    ELF_ABI_LINUX = 3,
} ELFAbi;

typedef enum ELFType : u16
{
    none = 0,
    relocatable = 1,
    executable = 2,
    shared = 3,
    core = 4,
} ELFType;

typedef enum ELFMachine : u16
{
    x86_64 = 0x3e,
    aarch64 = 0xb7,
} ELFMachine;

typedef enum ELFSectionIndex : u16
{
    UNDEFINED = 0,
    ABSOLUTE = 0xfff1,
    COMMON = 0xfff2,
} ELFSectionIndex;

STRUCT(ELFHeader)
{
    u8 identifier[4];
    ELFBitCount bit_count;
    ELFEndianness endianness;
    u8 format_version;
    ELFAbi abi;
    u8 abi_version;
    u8 padding[7];
    ELFType type;
    ELFMachine machine;
    u32 version;
    u64 entry_point;
    u64 program_header_offset;
    u64 section_header_offset;
    u32 flags;
    u16 elf_header_size;
    u16 program_header_size;
    u16 program_header_count;
    u16 section_header_size;
    u16 section_header_count;
    u16 section_header_string_table_index;
};
static_assert(sizeof(ELFHeader) == 0x40);

typedef enum ELFSymbolBinding : u8
{
    ELF_SYMBOL_BINDING_LOCAL = 0,
    ELF_SYMBOL_BINDING_GLOBAL = 1,
    ELF_SYMBOL_BINDING_WEAK = 2,
} ELFSymbolBinding;

typedef enum ELFSymbolType : u8
{
    ELF_SYMBOL_TYPE_NONE = 0,
    ELF_SYMBOL_TYPE_OBJECT = 1,
    ELF_SYMBOL_TYPE_FUNCTION = 2,
    ELF_SYMBOL_TYPE_SECTION = 3,
    ELF_SYMBOL_TYPE_FILE = 4,
    ELF_SYMBOL_TYPE_COMMON = 5,
    ELF_SYMBOL_TYPE_TLS = 6,
} ELFSymbolType;

typedef enum ELFSymbolVisibility : u8
{
    ELF_SYMBOL_VISIBILITY_DEFAULT = 0,
    ELF_SYMBOL_VISIBILITY_INTERNAL = 1,
    ELF_SYMBOL_VISIBILITY_HIDDEN = 2,
    ELF_SYMBOL_VISIBILITY_PROTECTED = 3,
} ELFSymbolVisibility;

STRUCT(ELFSymbol)
{
    u32 name_offset;
    ELFSymbolType type:4;
    ELFSymbolBinding binding:4;
    ELFSymbolVisibility visibility;
    u16 section_index; // In the section header table
    u64 value;
    u64 size;
};
decl_vb(ELFSymbol);
static_assert(sizeof(ELFSymbol) == 24);

STRUCT(ElfGnuHashHeader)
{
    u32 bucket_count;
    u32 symbol_offset;
    u32 bloom_size;
    u32 bloom_shift;
};

typedef enum RelocationType_x86_64 : u32
{
    R_X86_64_NONE= 0, /* No reloc */
     R_X86_64_64= 1, /* Direct 64 bit  */
    R_X86_64_PC32= 2, /* PC relative 32 bit signed */
    R_X86_64_GOT32= 3, /* 32 bit GOT entry */
    R_X86_64_PLT32 = 4, /* 32 bit PLT address */
    R_X86_64_COPY = 5, /* Copy symbol at runtime */
    R_X86_64_GLOB_DAT = 6, /* Create GOT entry */
    R_X86_64_JUMP_SLOT = 7, /* Create PLT entry */
    R_X86_64_RELATIVE = 8, /* Adjust by program base */
    R_X86_64_GOTPCREL = 9, /* 32 bit signed PC relative offset to GOT */
    R_X86_64_32 = 10, /* Direct 32 bit zero extended */
    R_X86_64_32S = 11, /* Direct 32 bit sign extended */
    R_X86_64_16 = 12, /* Direct 16 bit zero extended */
    R_X86_64_PC16 = 13, /* 16 bit sign extended pc relative */
    R_X86_64_8 = 14, /* Direct 8 bit sign extended  */
    R_X86_64_PC8 = 15, /* 8 bit sign extended pc relative */
    R_X86_64_DTPMOD64 = 16, /* ID of module containing symbol */
    R_X86_64_DTPOFF64 = 17, /* Offset in module's TLS block */
    R_X86_64_TPOFF64 = 18, /* Offset in initial TLS block */
    R_X86_64_TLSGD = 19, /* 32 bit signed PC relative offset to two GOT entries for GD symbol */
    R_X86_64_TLSLD = 20, /* 32 bit signed PC relative offset to two GOT entries for LD symbol */
    R_X86_64_DTPOFF32 = 21, /* Offset in TLS block */
    R_X86_64_GOTTPOFF = 22, /* 32 bit signed PC relative offset to GOT entry for IE symbol */
    R_X86_64_TPOFF32 = 23, /* Offset in initial TLS block */
    R_X86_64_PC64 = 24, /* PC relative 64 bit */
    R_X86_64_GOTOFF64 = 25, /* 64 bit offset to GOT */
    R_X86_64_GOTPC32 = 26, /* 32 bit signed pc relative offset to GOT */
    R_X86_64_GOT64 = 27, /* 64-bit GOT entry offset */
    R_X86_64_GOTPCREL64 = 28, /* 64-bit PC relative offset to GOT entry */
    R_X86_64_GOTPC64 = 29, /* 64-bit PC relative offset to GOT */
    R_X86_64_GOTPLT64 = 30 , /* like GOT64, says PLT entry needed */
    R_X86_64_PLTOFF64 = 31, /* 64-bit GOT relative offset to PLT entry */
    R_X86_64_SIZE32 = 32, /* Size of symbol plus 32-bit addend */
    R_X86_64_SIZE64 = 33, /* Size of symbol plus 64-bit addend */
    R_X86_64_GOTPC32_TLSDESC = 34, /* GOT offset for TLS descriptor.  */
    R_X86_64_TLSDESC_CALL = 35, /* Marker for call through TLS descriptor.  */
    R_X86_64_TLSDESC = 36, /* TLS descriptor.  */
    R_X86_64_IRELATIVE = 37, /* Adjust indirectly by program base */
    R_X86_64_RELATIVE64 = 38, /* 64-bit adjust by program base */
    /* 39 Reserved was R_X86_64_PC32_BND */
    /* 40 Reserved was R_X86_64_PLT32_BND */
    R_X86_64_GOTPCRELX = 41, /* Load from 32 bit signed pc relative offset to GOT entry without REX prefix, relaxable.  */
    R_X86_64_REX_GOTPCRELX = 42, /* Load from 32 bit signed pc relative offset to GOT entry with REX prefix, relaxable.  */
    R_X86_64_NUM = 43,
} RelocationType_x86_64;

UNION(RelocationType)
{
    RelocationType_x86_64 x86_64;
};

STRUCT(ElfRelocationWithAddendInfo)
{
    RelocationType type;
    u32 symbol;
};

STRUCT(ElfRelocationWithAddend)
{
    u64 offset;
    ElfRelocationWithAddendInfo info;
    u64 addend;
};

// Custom ELF structures

STRUCT(SymbolTable)
{
    VirtualBuffer(ELFSymbol) symbol_table;
    VirtualBuffer(u8) string_table;
};

// DWARF

typedef enum DwarfTag : u16
{
    DW_TAG_array_type = 0x01,
    DW_TAG_class_type = 0x02,
    DW_TAG_entry_point = 0x03,
    DW_TAG_enumeration_type = 0x04,
    DW_TAG_formal_parameter = 0x05,
    DW_TAG_imported_declaration = 0x08,
    DW_TAG_label = 0x0a,
    DW_TAG_lexical_block = 0x0b,
    DW_TAG_member = 0x0d,
    DW_TAG_pointer_type = 0x0f,
    DW_TAG_reference_type = 0x10,
    DW_TAG_compile_unit = 0x11,
    DW_TAG_string_type = 0x12,
    DW_TAG_structure_type = 0x13,
    DW_TAG_subroutine_type = 0x15,
    DW_TAG_typedef = 0x16,
    DW_TAG_union_type = 0x17,
    DW_TAG_unspecified_parameters = 0x18,
    DW_TAG_variant = 0x19,
    DW_TAG_common_block = 0x1a,
    DW_TAG_common_inclusion = 0x1b,
    DW_TAG_inheritance = 0x1c,
    DW_TAG_inlined_subroutine = 0x1d,
    DW_TAG_module = 0x1e,
    DW_TAG_ptr_to_member_type = 0x1f,
    DW_TAG_set_type = 0x20,
    DW_TAG_subrange_type = 0x21,
    DW_TAG_with_stmt = 0x22,
    DW_TAG_access_declaration = 0x23,
    DW_TAG_base_type = 0x24,
    DW_TAG_catch_block = 0x25,
    DW_TAG_const_type = 0x26,
    DW_TAG_constant = 0x27,
    DW_TAG_enumerator = 0x28,
    DW_TAG_file_type = 0x29,
    DW_TAG_friend = 0x2a,
    DW_TAG_namelist = 0x2b,
    DW_TAG_namelist_item = 0x2c,
    DW_TAG_packed_type = 0x2d,
    DW_TAG_subprogram = 0x2e,
    DW_TAG_template_type_parameter = 0x2f,
    DW_TAG_template_value_parameter = 0x30,
    DW_TAG_thrown_type = 0x31,
    DW_TAG_try_block = 0x32,
    DW_TAG_variant_part = 0x33,
    DW_TAG_variable = 0x34,
    DW_TAG_volatile_type = 0x35,
    DW_TAG_dwarf_procedure = 0x36,
    DW_TAG_restrict_type = 0x37,
    DW_TAG_interface_type = 0x38,
    DW_TAG_namespace = 0x39,
    DW_TAG_imported_module = 0x3a,
    DW_TAG_unspecified_type = 0x3b,
    DW_TAG_partial_unit = 0x3c,
    DW_TAG_imported_unit = 0x3d,
    DW_TAG_condition = 0x3f,
    DW_TAG_shared_type = 0x40,
    DW_TAG_type_unit = 0x41,
    DW_TAG_rvalue_reference_type = 0x42,
    DW_TAG_template_alias = 0x43,
    DW_TAG_coarray_type = 0x44,
    DW_TAG_generic_subrange = 0x45,
    DW_TAG_dynamic_type = 0x46,
    DW_TAG_atomic_type = 0x47,
    DW_TAG_call_site = 0x48,
    DW_TAG_call_site_parameter = 0x49,
    DW_TAG_skeleton_unit = 0x4a,
    DW_TAG_immutable_type = 0x4b,
    DW_TAG_lo_user = 0x4080,
    DW_TAG_hi_user = 0xffff,
} DwarfTag;

fn String dwarf_tag_to_string(DwarfTag tag)
{
    switch (tag)
    {
        case_to_name(DW_TAG_, array_type);
        case_to_name(DW_TAG_, class_type);
        case_to_name(DW_TAG_, entry_point);
        case_to_name(DW_TAG_, enumeration_type);
        case_to_name(DW_TAG_, formal_parameter);
        case_to_name(DW_TAG_, imported_declaration);
        case_to_name(DW_TAG_, label);
        case_to_name(DW_TAG_, lexical_block);
        case_to_name(DW_TAG_, member);
        case_to_name(DW_TAG_, pointer_type);
        case_to_name(DW_TAG_, reference_type);
        case_to_name(DW_TAG_, compile_unit);
        case_to_name(DW_TAG_, string_type);
        case_to_name(DW_TAG_, structure_type);
        case_to_name(DW_TAG_, subroutine_type);
        case_to_name(DW_TAG_, typedef);
        case_to_name(DW_TAG_, union_type);
        case_to_name(DW_TAG_, unspecified_parameters);
        case_to_name(DW_TAG_, variant);
        case_to_name(DW_TAG_, common_block);
        case_to_name(DW_TAG_, common_inclusion);
        case_to_name(DW_TAG_, inheritance);
        case_to_name(DW_TAG_, inlined_subroutine);
        case_to_name(DW_TAG_, module);
        case_to_name(DW_TAG_, ptr_to_member_type);
        case_to_name(DW_TAG_, set_type);
        case_to_name(DW_TAG_, subrange_type);
        case_to_name(DW_TAG_, with_stmt);
        case_to_name(DW_TAG_, access_declaration);
        case_to_name(DW_TAG_, base_type);
        case_to_name(DW_TAG_, catch_block);
        case_to_name(DW_TAG_, const_type);
        case_to_name(DW_TAG_, constant);
        case_to_name(DW_TAG_, enumerator);
        case_to_name(DW_TAG_, file_type);
        case_to_name(DW_TAG_, friend);
        case_to_name(DW_TAG_, namelist);
        case_to_name(DW_TAG_, namelist_item);
        case_to_name(DW_TAG_, packed_type);
        case_to_name(DW_TAG_, subprogram);
        case_to_name(DW_TAG_, template_type_parameter);
        case_to_name(DW_TAG_, template_value_parameter);
        case_to_name(DW_TAG_, thrown_type);
        case_to_name(DW_TAG_, try_block);
        case_to_name(DW_TAG_, variant_part);
        case_to_name(DW_TAG_, variable);
        case_to_name(DW_TAG_, volatile_type);
        case_to_name(DW_TAG_, dwarf_procedure);
        case_to_name(DW_TAG_, restrict_type);
        case_to_name(DW_TAG_, interface_type);
        case_to_name(DW_TAG_, namespace);
        case_to_name(DW_TAG_, imported_module);
        case_to_name(DW_TAG_, unspecified_type);
        case_to_name(DW_TAG_, partial_unit);
        case_to_name(DW_TAG_, imported_unit);
        case_to_name(DW_TAG_, condition);
        case_to_name(DW_TAG_, shared_type);
        case_to_name(DW_TAG_, type_unit);
        case_to_name(DW_TAG_, rvalue_reference_type);
        case_to_name(DW_TAG_, template_alias);
        case_to_name(DW_TAG_, coarray_type);
        case_to_name(DW_TAG_, generic_subrange);
        case_to_name(DW_TAG_, dynamic_type);
        case_to_name(DW_TAG_, atomic_type);
        case_to_name(DW_TAG_, call_site);
        case_to_name(DW_TAG_, call_site_parameter);
        case_to_name(DW_TAG_, skeleton_unit);
        case_to_name(DW_TAG_, immutable_type);
        case_to_name(DW_TAG_, lo_user);
        case_to_name(DW_TAG_, hi_user);
    }
}

typedef enum DwarfAttribute : u16
{
    DW_AT_sibling = 0x01, // reference
    DW_AT_location = 0x02, // exprloc, loclist
    DW_AT_name = 0x03, // string
    DW_AT_ordering = 0x09, // constant
    DW_AT_byte_size = 0x0b, // constant, exprloc, reference
    DW_AT_bit_size = 0x0d, // constant, exprloc, reference
    DW_AT_stmt_list = 0x10, // lineptr
    DW_AT_low_pc = 0x11, // address
    DW_AT_high_pc = 0x12, // address, constant
    DW_AT_language = 0x13, // constant
    DW_AT_discr = 0x15, //reference
    DW_AT_discr_value = 0x16, // constant
    DW_AT_visibility = 0x17, // constant
    DW_AT_import = 0x18, // reference
    DW_AT_string_length = 0x19, // exprloc, loclist, reference
    DW_AT_common_reference = 0x1a, // reference
    DW_AT_comp_dir = 0x1b, // string
    DW_AT_const_value = 0x1c, // block, constant, string
    DW_AT_containing_type = 0x1d, // reference
    DW_AT_default_value = 0x1e, // constant, reference, flag
    DW_AT_inline = 0x20, // constant
    DW_AT_is_optional = 0x21, // flag
    DW_AT_lower_bound = 0x22, // constant, exprloc, reference
    DW_AT_producer = 0x25, // string
    DW_AT_prototyped = 0x27, // flag
    DW_AT_return_addr = 0x2a, // exprloc, loclist
    DW_AT_start_scope = 0x2c, // constant, rnglist
    DW_AT_bit_stride = 0x2e, // constant, exprloc, reference
    DW_AT_upper_bound = 0x2f, // constant, exprloc, reference
    DW_AT_abstract_origin = 0x31, // reference
    DW_AT_accessibility = 0x32, // constant
    DW_AT_address_class = 0x33, // constant
    DW_AT_artificial = 0x34, // flag
    DW_AT_base_types = 0x35, // reference
    DW_AT_calling_convention = 0x36, // constant
    DW_AT_count = 0x37, // constant, exprloc, reference
    DW_AT_data_member_location = 0x38, // constant, exprloc, loclist
    DW_AT_decl_column = 0x39, // constant
    DW_AT_decl_file = 0x3a, // constant
    DW_AT_decl_line = 0x3b, // constant
    DW_AT_declaration = 0x3c, // flag
    DW_AT_discr_list = 0x3d, // block
    DW_AT_encoding = 0x3e, // constant
    DW_AT_external = 0x3f, // flag
    DW_AT_frame_base = 0x40, // exprloc, loclist
    DW_AT_friend = 0x41, // reference
    DW_AT_identifier_case = 0x42, // constant
    Reserved = 0x433, // macptr
    DW_AT_namelist_item = 0x44, // reference
    DW_AT_priority = 0x45, // reference
    DW_AT_segment = 0x46, // exprloc, loclist
    DW_AT_specification = 0x47, // reference
    DW_AT_static_link = 0x48, // exprloc, loclist
    DW_AT_type = 0x49, // reference
    DW_AT_use_location = 0x4a, // exprloc, loclist
    DW_AT_variable_parameter = 0x4b, //flag
    DW_AT_virtuality = 0x4c, // constant
    DW_AT_vtable_elem_location = 0x4d, // exprloc, loclist
    DW_AT_allocated = 0x4e, // constant, exprloc, reference
    DW_AT_associated = 0x4f, // constant, exprloc, reference
    DW_AT_data_location = 0x50, // exprloc
    DW_AT_byte_stride = 0x51, // constant, exprloc, reference
    DW_AT_entry_pc = 0x52, // address, constant
    DW_AT_use_UTF8 = 0x53, // flag
    DW_AT_extension = 0x54, // reference
    DW_AT_ranges = 0x55, // rnglist
    DW_AT_trampoline = 0x56, // address, flag, reference, string
    DW_AT_call_column = 0x57, // constant
    DW_AT_call_file = 0x58, // constant
    DW_AT_call_line = 0x59, // constant
    DW_AT_description = 0x5a, // string
    DW_AT_binary_scale = 0x5b, // constant
    DW_AT_decimal_scale = 0x5c, // constant
    DW_AT_small = 0x5d, // reference
    DW_AT_decimal_sign = 0x5e, // constant
    DW_AT_digit_count = 0x5f, // constant
    DW_AT_picture_string = 0x60, // string
    DW_AT_mutable = 0x61, // flag
    DW_AT_threads_scaled = 0x62, // flag
    DW_AT_explicit = 0x63, // flag
    DW_AT_object_pointer = 0x64, // reference
    DW_AT_endianity = 0x65, // constant
    DW_AT_elemental = 0x66, // flag
    DW_AT_pure = 0x67, // flag
    DW_AT_recursive = 0x68, // flag
    DW_AT_signature = 0x69, // reference
    DW_AT_main_subprogram = 0x6a, // flag
    DW_AT_data_bit_offset = 0x6b, // constant
    DW_AT_const_expr = 0x6c, // flag
    DW_AT_enum_class = 0x6d, // flag
    DW_AT_linkage_name = 0x6e, // string
    DW_AT_string_length_bit_size = 0x6f, //constant
    DW_AT_string_length_byte_size = 0x70, //constant
    DW_AT_rank = 0x71, //constant, exprloc
    DW_AT_str_offsets_base = 0x72, //stroffsetsptr
    DW_AT_addr_base = 0x73, //addrptr
    DW_AT_rnglists_base = 0x74, //rnglistsptr
    DW_AT_dwo_name = 0x76, //string
    DW_AT_reference = 0x77, //flag
    DW_AT_rvalue_reference = 0x78, //flag
    DW_AT_macros = 0x79, //macptr
    DW_AT_call_all_calls = 0x7a, //flag
    DW_AT_call_all_source_calls = 0x7b, //flag
    DW_AT_call_all_tail_calls = 0x7c, //flag
    DW_AT_call_return_pc = 0x7d, //address
    DW_AT_call_value = 0x7e, //exprloc
    DW_AT_call_origin = 0x7f, //exprloc
    DW_AT_call_parameter = 0x80, //reference
    DW_AT_call_pc = 0x81, //address
    DW_AT_call_tail_call = 0x82, //flag
    DW_AT_call_target = 0x83, //exprloc
    DW_AT_call_target_clobbered = 0x84, //exprloc
    DW_AT_call_data_location = 0x85, //exprloc
    DW_AT_call_data_value = 0x86, //exprloc
    DW_AT_noreturn = 0x87, //flag
    DW_AT_alignment = 0x88, //constant
    DW_AT_export_symbols = 0x89, //flag
    DW_AT_deleted = 0x8a, //flag
    DW_AT_defaulted = 0x8b, //constant
    DW_AT_loclists_base = 0x8c, //loclistsptr
    DW_AT_lo_user = 0x2000,
    DW_AT_hi_user = 0x3fff,
} DwarfAttribute;

String dwarf_attribute_to_string(DwarfAttribute attribute)
{
    switch (attribute)
    {
        case_to_name(DW_AT_, sibling);
        case_to_name(DW_AT_, location);
        case_to_name(DW_AT_, name);
        case_to_name(DW_AT_, ordering);
        case_to_name(DW_AT_, byte_size);
        case_to_name(DW_AT_, bit_size);
        case_to_name(DW_AT_, stmt_list);
        case_to_name(DW_AT_, low_pc);
        case_to_name(DW_AT_, high_pc);
        case_to_name(DW_AT_, language);
        case_to_name(DW_AT_, discr);
        case_to_name(DW_AT_, discr_value);
        case_to_name(DW_AT_, visibility);
        case_to_name(DW_AT_, import);
        case_to_name(DW_AT_, string_length);
        case_to_name(DW_AT_, common_reference);
        case_to_name(DW_AT_, comp_dir);
        case_to_name(DW_AT_, const_value);
        case_to_name(DW_AT_, containing_type);
        case_to_name(DW_AT_, default_value);
        case_to_name(DW_AT_, inline);
        case_to_name(DW_AT_, is_optional);
        case_to_name(DW_AT_, lower_bound);
        case_to_name(DW_AT_, producer);
        case_to_name(DW_AT_, prototyped);
        case_to_name(DW_AT_, return_addr);
        case_to_name(DW_AT_, start_scope);
        case_to_name(DW_AT_, bit_stride);
        case_to_name(DW_AT_, upper_bound);
        case_to_name(DW_AT_, abstract_origin);
        case_to_name(DW_AT_, accessibility);
        case_to_name(DW_AT_, address_class);
        case_to_name(DW_AT_, artificial);
        case_to_name(DW_AT_, base_types);
        case_to_name(DW_AT_, calling_convention);
        case_to_name(DW_AT_, count);
        case_to_name(DW_AT_, data_member_location);
        case_to_name(DW_AT_, decl_column);
        case_to_name(DW_AT_, decl_file);
        case_to_name(DW_AT_, decl_line);
        case_to_name(DW_AT_, declaration);
        case_to_name(DW_AT_, discr_list);
        case_to_name(DW_AT_, encoding);
        case_to_name(DW_AT_, external);
        case_to_name(DW_AT_, frame_base);
        case_to_name(DW_AT_, friend);
        case_to_name(DW_AT_, identifier_case);
        case_to_name(Reserv, ed);
        case_to_name(DW_AT_, namelist_item);
        case_to_name(DW_AT_, priority);
        case_to_name(DW_AT_, segment);
        case_to_name(DW_AT_, specification);
        case_to_name(DW_AT_, static_link);
        case_to_name(DW_AT_, type);
        case_to_name(DW_AT_, use_location);
        case_to_name(DW_AT_, variable_parameter);
        case_to_name(DW_AT_, virtuality);
        case_to_name(DW_AT_, vtable_elem_location);
        case_to_name(DW_AT_, allocated);
        case_to_name(DW_AT_, associated);
        case_to_name(DW_AT_, data_location);
        case_to_name(DW_AT_, byte_stride);
        case_to_name(DW_AT_, entry_pc);
        case_to_name(DW_AT_, use_UTF8);
        case_to_name(DW_AT_, extension);
        case_to_name(DW_AT_, ranges);
        case_to_name(DW_AT_, trampoline);
        case_to_name(DW_AT_, call_column);
        case_to_name(DW_AT_, call_file);
        case_to_name(DW_AT_, call_line);
        case_to_name(DW_AT_, description);
        case_to_name(DW_AT_, binary_scale);
        case_to_name(DW_AT_, decimal_scale);
        case_to_name(DW_AT_, small);
        case_to_name(DW_AT_, decimal_sign);
        case_to_name(DW_AT_, digit_count);
        case_to_name(DW_AT_, picture_string);
        case_to_name(DW_AT_, mutable);
        case_to_name(DW_AT_, threads_scaled);
        case_to_name(DW_AT_, explicit);
        case_to_name(DW_AT_, object_pointer);
        case_to_name(DW_AT_, endianity);
        case_to_name(DW_AT_, elemental);
        case_to_name(DW_AT_, pure);
        case_to_name(DW_AT_, recursive);
        case_to_name(DW_AT_, signature);
        case_to_name(DW_AT_, main_subprogram);
        case_to_name(DW_AT_, data_bit_offset);
        case_to_name(DW_AT_, const_expr);
        case_to_name(DW_AT_, enum_class);
        case_to_name(DW_AT_, linkage_name);
        case_to_name(DW_AT_, string_length_bit_size);
        case_to_name(DW_AT_, string_length_byte_size);
        case_to_name(DW_AT_, rank);
        case_to_name(DW_AT_, str_offsets_base);
        case_to_name(DW_AT_, addr_base);
        case_to_name(DW_AT_, rnglists_base);
        case_to_name(DW_AT_, dwo_name);
        case_to_name(DW_AT_, reference);
        case_to_name(DW_AT_, rvalue_reference);
        case_to_name(DW_AT_, macros);
        case_to_name(DW_AT_, call_all_calls);
        case_to_name(DW_AT_, call_all_source_calls);
        case_to_name(DW_AT_, call_all_tail_calls);
        case_to_name(DW_AT_, call_return_pc);
        case_to_name(DW_AT_, call_value);
        case_to_name(DW_AT_, call_origin);
        case_to_name(DW_AT_, call_parameter);
        case_to_name(DW_AT_, call_pc);
        case_to_name(DW_AT_, call_tail_call);
        case_to_name(DW_AT_, call_target);
        case_to_name(DW_AT_, call_target_clobbered);
        case_to_name(DW_AT_, call_data_location);
        case_to_name(DW_AT_, call_data_value);
        case_to_name(DW_AT_, noreturn);
        case_to_name(DW_AT_, alignment);
        case_to_name(DW_AT_, export_symbols);
        case_to_name(DW_AT_, deleted);
        case_to_name(DW_AT_, defaulted);
        case_to_name(DW_AT_, loclists_base);
        case_to_name(DW_AT_, lo_user);
        case_to_name(DW_AT_, hi_user);
    }
}

typedef enum DwarfForm : u8
{
    DW_FORM_addr = 0x01, // address
    DW_FORM_block2 = 0x03, // block
    DW_FORM_block4 = 0x04, // block
    DW_FORM_data2 = 0x05, // constant
    DW_FORM_data4 = 0x06, // constant
    DW_FORM_data8 = 0x07, // constant
    DW_FORM_string = 0x08, // string
    DW_FORM_block = 0x09, // block
    DW_FORM_block1 = 0x0a, // block
    DW_FORM_data1 = 0x0b, // constant
    DW_FORM_flag = 0x0c, // flag
    DW_FORM_sdata = 0x0d, // constant
    DW_FORM_strp = 0x0e, // string
    DW_FORM_udata = 0x0f, // constant
    DW_FORM_ref_addr = 0x10, // reference
    DW_FORM_ref1 = 0x11, // reference
    DW_FORM_ref2 = 0x12, // reference
    DW_FORM_ref4 = 0x13, // reference
    DW_FORM_ref8 = 0x14, // reference
    DW_FORM_ref_udata = 0x15, // reference
    DW_FORM_indirect = 0x16, // (see Section 7.5.3 on page 203)
    DW_FORM_sec_offset = 0x17, // addrptr, lineptr, loclist, loclistsptr, macptr, rnglist, rnglistsptr, stroffsetsptr
    DW_FORM_exprloc = 0x18, // exprloc
    DW_FORM_flag_present = 0x19, // flag
    DW_FORM_strx = 0x1a, // string
    DW_FORM_addrx = 0x1b, // address
    DW_FORM_ref_sup4 = 0x1c, // reference
    DW_FORM_strp_sup = 0x1d, // string
    DW_FORM_data16 = 0x1e, // constant
    DW_FORM_line_strp = 0x1f, // string
    DW_FORM_ref_sig8 = 0x20, // reference
    DW_FORM_implicit_const = 0x21, // constant
    DW_FORM_loclistx = 0x22, // loclist
    DW_FORM_rnglistx = 0x23, // rnglist
    DW_FORM_ref_sup8 = 0x24, // reference
    DW_FORM_strx1 = 0x25, // string
    DW_FORM_strx2 = 0x26, // string
    DW_FORM_strx3 = 0x27, // string
    DW_FORM_strx4 = 0x28, // string
    DW_FORM_addrx1 = 0x29, // address
    DW_FORM_addrx2 = 0x2a, // address
    DW_FORM_addrx3 = 0x2b, // address
    DW_FORM_addrx4 = 0x2c, // address
} DwarfForm;

fn String dwarf_form_to_string(DwarfForm form)
{
    switch (form)
    {
        case_to_name(DW_FORM_, addr);
        case_to_name(DW_FORM_, block2);
        case_to_name(DW_FORM_, block4);
        case_to_name(DW_FORM_, data2);
        case_to_name(DW_FORM_, data4);
        case_to_name(DW_FORM_, data8);
        case_to_name(DW_FORM_, string);
        case_to_name(DW_FORM_, block);
        case_to_name(DW_FORM_, block1);
        case_to_name(DW_FORM_, data1);
        case_to_name(DW_FORM_, flag);
        case_to_name(DW_FORM_, sdata);
        case_to_name(DW_FORM_, strp);
        case_to_name(DW_FORM_, udata);
        case_to_name(DW_FORM_, ref_addr);
        case_to_name(DW_FORM_, ref1);
        case_to_name(DW_FORM_, ref2);
        case_to_name(DW_FORM_, ref4);
        case_to_name(DW_FORM_, ref8);
        case_to_name(DW_FORM_, ref_udata);
        case_to_name(DW_FORM_, indirect);
        case_to_name(DW_FORM_, sec_offset);
        case_to_name(DW_FORM_, exprloc);
        case_to_name(DW_FORM_, flag_present);
        case_to_name(DW_FORM_, strx);
        case_to_name(DW_FORM_, addrx);
        case_to_name(DW_FORM_, ref_sup4);
        case_to_name(DW_FORM_, strp_sup);
        case_to_name(DW_FORM_, data16);
        case_to_name(DW_FORM_, line_strp);
        case_to_name(DW_FORM_, ref_sig8);
        case_to_name(DW_FORM_, implicit_const);
        case_to_name(DW_FORM_, loclistx);
        case_to_name(DW_FORM_, rnglistx);
        case_to_name(DW_FORM_, ref_sup8);
        case_to_name(DW_FORM_, strx1);
        case_to_name(DW_FORM_, strx2);
        case_to_name(DW_FORM_, strx3);
        case_to_name(DW_FORM_, strx4);
        case_to_name(DW_FORM_, addrx1);
        case_to_name(DW_FORM_, addrx2);
        case_to_name(DW_FORM_, addrx3);
        case_to_name(DW_FORM_, addrx4);
    }
}

typedef enum DwarfUnitType : u8
{
    DW_UT_compile = 0x01, 
    DW_UT_type = 0x02, 
    DW_UT_partial = 0x03, 
    DW_UT_skeleton = 0x04, 
    DW_UT_split_compile = 0x05, 
    DW_UT_split_type = 0x06, 
    DW_UT_lo_user = 0x80, 
    DW_UT_hi_user = 0xff, 
} DwarfUnitType;

typedef enum DwarfOperation : u8
{
    DW_OP_addr = 0x03, // Operands: 1
    DW_OP_deref = 0x06, // Operands: 0
    DW_OP_const1u = 0x08, // Operands: 1
    DW_OP_const1s = 0x09, // Operands: 1
    DW_OP_const2u = 0x0a, // Operands: 1
    DW_OP_const2s = 0x0b, // Operands: 1
    DW_OP_const4u = 0x0c, // Operands: 1
    DW_OP_const4s = 0x0d, // Operands: 1
    DW_OP_const8u = 0x0e, // Operands: 1
    DW_OP_const8s = 0x0f, // Operands: 1
    DW_OP_constu = 0x10, // Operands: 1, uleb128
    DW_OP_consts = 0x11, // Operands: 1 sleb128
    DW_OP_dup = 0x12, // Operands: 0
    DW_OP_drop = 0x13, // Operands: 0
    DW_OP_over = 0x14, // Operands: 0
    DW_OP_pick = 0x15, // Operands: 1, 1-byte stack index
    DW_OP_swap = 0x16, // Operands: 0
    DW_OP_rot = 0x17, // Operands: 0
    DW_OP_xderef = 0x18, // Operands: 0
    DW_OP_abs = 0x19, // Operands: 0
    DW_OP_and = 0x1a , // Operands: 0
    DW_OP_div = 0x1b, // Operands: 0
    DW_OP_minus = 0x1c, // Operands: 0
    DW_OP_mod = 0x1d, // Operands: 0
    DW_OP_mul = 0x1e, // Operands: 0
    DW_OP_neg = 0x1f, // Operands: 0
    DW_OP_not = 0x20, // Operands: 0
    DW_OP_or = 0x21, // Operands: 0
    DW_OP_plus = 0x22, // Operands: 0
    DW_OP_plus_uconst = 0x23, // Operands: 1, ULEB128 addend
    DW_OP_shl = 0x24, // Operands: 0
    DW_OP_shr = 0x25, // Operands: 0
    DW_OP_shra = 0x26, // Operands: 0
    DW_OP_xor = 0x27, // Operands: 0
    DW_OP_bra = 0x28, // Operands: 1, signed 2-byte constant
    DW_OP_eq = 0x29, // Operands: 0
    DW_OP_ge = 0x2a, // Operands: 0
    DW_OP_gt = 0x2b, // Operands: 0
    DW_OP_le = 0x2c, // Operands: 0
    DW_OP_lt = 0x2d, // Operands: 0
    DW_OP_ne = 0x2e, // Operands: 0
    DW_OP_skip = 0x2f, // Operands: 1, signed 2-byte constant
    DW_OP_lit0 = 0x30, // Operands: 0
    DW_OP_lit1 = 0x31, // Operands: 0
    DW_OP_lit2 = 0x32, // Operands: 0
    DW_OP_lit3 = 0x33, // Operands: 0
    DW_OP_lit4 = 0x34, // Operands: 0
    DW_OP_lit5 = 0x35, // Operands: 0
    DW_OP_lit6 = 0x36, // Operands: 0
    DW_OP_lit7 = 0x37, // Operands: 0
    DW_OP_lit8 = 0x38, // Operands: 0
    DW_OP_lit9 = 0x39, // Operands: 0
    DW_OP_lit10 = 0x3a, // Operands: 0
    DW_OP_lit11 = 0x3b, // Operands: 0
    DW_OP_lit12 = 0x3c, // Operands: 0
    DW_OP_lit13 = 0x3d, // Operands: 0
    DW_OP_lit14 = 0x3e, // Operands: 0
    DW_OP_lit15 = 0x3f, // Operands: 0
    DW_OP_lit16 = 0x40, // Operands: 0
    DW_OP_lit17 = 0x41, // Operands: 0
    DW_OP_lit18 = 0x42, // Operands: 0
    DW_OP_lit19 = 0x43, // Operands: 0
    DW_OP_lit20 = 0x44, // Operands: 0
    DW_OP_lit21 = 0x45, // Operands: 0
    DW_OP_lit22 = 0x46, // Operands: 0
    DW_OP_lit23 = 0x47, // Operands: 0
    DW_OP_lit24 = 0x48, // Operands: 0
    DW_OP_lit25 = 0x49, // Operands: 0
    DW_OP_lit26 = 0x4a, // Operands: 0
    DW_OP_lit27 = 0x4b, // Operands: 0
    DW_OP_lit28 = 0x4c, // Operands: 0
    DW_OP_lit29 = 0x4d, // Operands: 0
    DW_OP_lit30 = 0x4e, // Operands: 0
    DW_OP_lit31 = 0x4f, // Operands: 0
    DW_OP_reg0 = 0x50, // Operands: 0
    DW_OP_reg1 = 0x51, // Operands: 0
    DW_OP_reg2 = 0x52, // Operands: 0
    DW_OP_reg3 = 0x53, // Operands: 0
    DW_OP_reg4 = 0x54, // Operands: 0
    DW_OP_reg5 = 0x55, // Operands: 0
    DW_OP_reg6 = 0x56, // Operands: 0
    DW_OP_reg7 = 0x57, // Operands: 0
    DW_OP_reg8 = 0x58, // Operands: 0
    DW_OP_reg9 = 0x59, // Operands: 0
    DW_OP_reg10 = 0x5a, // Operands: 0
    DW_OP_reg11 = 0x5b, // Operands: 0
    DW_OP_reg12 = 0x5c, // Operands: 0
    DW_OP_reg13 = 0x5d, // Operands: 0
    DW_OP_reg14 = 0x5e, // Operands: 0
    DW_OP_reg15 = 0x5f, // Operands: 0
    DW_OP_reg16 = 0x60, // Operands: 0
    DW_OP_reg17 = 0x61, // Operands: 0
    DW_OP_reg18 = 0x62, // Operands: 0
    DW_OP_reg19 = 0x63, // Operands: 0
    DW_OP_reg20 = 0x64, // Operands: 0
    DW_OP_reg21 = 0x65, // Operands: 0
    DW_OP_reg22 = 0x66, // Operands: 0
    DW_OP_reg23 = 0x67, // Operands: 0
    DW_OP_reg24 = 0x68, // Operands: 0
    DW_OP_reg25 = 0x69, // Operands: 0
    DW_OP_reg26 = 0x6a, // Operands: 0
    DW_OP_reg27 = 0x6b, // Operands: 0
    DW_OP_reg28 = 0x6c, // Operands: 0
    DW_OP_reg29 = 0x6d, // Operands: 0
    DW_OP_reg30 = 0x6e, // Operands: 0
    DW_OP_reg31 = 0x6f, // Operands: 0
    DW_OP_breg0 = 0x70, // Operands: 1, SLEB128 offset
    DW_OP_breg1 = 0x71, // Operands: 1, SLEB128 offset
    DW_OP_breg2 = 0x72, // Operands: 1, SLEB128 offset
    DW_OP_breg3 = 0x73, // Operands: 1, SLEB128 offset
    DW_OP_breg4 = 0x74, // Operands: 1, SLEB128 offset
    DW_OP_breg5 = 0x75, // Operands: 1, SLEB128 offset
    DW_OP_breg6 = 0x76, // Operands: 1, SLEB128 offset
    DW_OP_breg7 = 0x77, // Operands: 1, SLEB128 offset
    DW_OP_breg8 = 0x78, // Operands: 1, SLEB128 offset
    DW_OP_breg9 = 0x79, // Operands: 1, SLEB128 offset
    DW_OP_breg10 = 0x7a, // Operands: 1, SLEB128 offset
    DW_OP_breg11 = 0x7b, // Operands: 1, SLEB128 offset
    DW_OP_breg12 = 0x7c, // Operands: 1, SLEB128 offset
    DW_OP_breg13 = 0x7d, // Operands: 1, SLEB128 offset
    DW_OP_breg14 = 0x7e, // Operands: 1, SLEB128 offset
    DW_OP_breg15 = 0x7f, // Operands: 1, SLEB128 offset
    DW_OP_breg16 = 0x80, // Operands: 1, SLEB128 offset
    DW_OP_breg17 = 0x81, // Operands: 1, SLEB128 offset
    DW_OP_breg18 = 0x82, // Operands: 1, SLEB128 offset
    DW_OP_breg19 = 0x83, // Operands: 1, SLEB128 offset
    DW_OP_breg20 = 0x84, // Operands: 1, SLEB128 offset
    DW_OP_breg21 = 0x85, // Operands: 1, SLEB128 offset
    DW_OP_breg22 = 0x86, // Operands: 1, SLEB128 offset
    DW_OP_breg23 = 0x87, // Operands: 1, SLEB128 offset
    DW_OP_breg24 = 0x88, // Operands: 1, SLEB128 offset
    DW_OP_breg25 = 0x89, // Operands: 1, SLEB128 offset
    DW_OP_breg26 = 0x8a, // Operands: 1, SLEB128 offset
    DW_OP_breg27 = 0x8b, // Operands: 1, SLEB128 offset
    DW_OP_breg28 = 0x8c, // Operands: 1, SLEB128 offset
    DW_OP_breg29 = 0x8d, // Operands: 1, SLEB128 offset
    DW_OP_breg30 = 0x8e, // Operands: 1, SLEB128 offset
    DW_OP_breg31 = 0x8f, // Operands: 1, SLEB128 offset
    DW_OP_regx = 0x90, // Operands: 1, ULEB128 register
    DW_OP_fbreg = 0x91, // Operands: 1, SLEB128 offset
    DW_OP_bregx = 0x92, // Operands: 2 ULEB128 register, SLEB128 offset
    DW_OP_piece = 0x93, // Operands: 1, ULEB128 size of piece
    DW_OP_deref_size = 0x94, // Operands: 1, 1-byte size of data retrieved
    DW_OP_xderef_size = 0x95, // Operands: 1, 1-byte size of data retrieved
    DW_OP_nop = 0x96, // Operands: 0
    DW_OP_push_object_address = 0x97, // Operands: 0
    DW_OP_call2 = 0x98, // Operands: 1, 2-byte offset of DIE
    DW_OP_call4 = 0x99, // Operands: 1, 4-byte offset of DIE
    DW_OP_call_ref = 0x9a, // Operands: 1, 4- or 8-byte offset of DIE
    DW_OP_form_tls_address = 0x9b, // Operands: 0
    DW_OP_call_frame_cfa = 0x9c, // Operands: 0
    DW_OP_bit_piece = 0x9d, // Operands: 2, ULEB128 size, ULEB128 offset
    DW_OP_implicit_value = 0x9e, // Operands: 2, ULEB128 size, block of that size
    DW_OP_stack_value = 0x9f, // Operands: 0
    DW_OP_implicit_pointer = 0xa0, // Operands: 2, 4- or 8-byte offset of DIE, SLEB128 constant offset
    DW_OP_addrx = 0xa1, // Operands: 1, ULEB128 indirect address
    DW_OP_constx = 0xa2, // Operands: 1, ULEB128 indirect constant
    DW_OP_entry_value = 0xa3, // Operands: 2, ULEB128 size, block of that size
    DW_OP_const_type = 0xa4, // Operands: 3, ULEB128 type entry offset, 1-byte size, constant value
    DW_OP_regval_type = 0xa5, // Operands: 2, ULEB128 register number, ULEB128 constant offset
    DW_OP_deref_type = 0xa6, // Operands: 2, 1-byte size, ULEB128 type entry offset
    DW_OP_xderef_type = 0xa7, // Operands: 2, 1-byte size, ULEB128 type entry offset
    DW_OP_convert = 0xa8, // Operands: 1, ULEB128 type entry offset
    DW_OP_reinterpret = 0xa9, // Operands: 1,  ULEB128 type entry offset
    DW_OP_lo_user = 0xe0,
    DW_OP_hi_user = 0xff,
} DwarfOperation;

String dwarf_operation_to_string(DwarfOperation operation)
{
    switch (operation)
    {
        case_to_name(DW_OP_, addr);
        case_to_name(DW_OP_, deref);
        case_to_name(DW_OP_, const1u);
        case_to_name(DW_OP_, const1s);
        case_to_name(DW_OP_, const2u);
        case_to_name(DW_OP_, const2s);
        case_to_name(DW_OP_, const4u);
        case_to_name(DW_OP_, const4s);
        case_to_name(DW_OP_, const8u);
        case_to_name(DW_OP_, const8s);
        case_to_name(DW_OP_, constu);
        case_to_name(DW_OP_, consts);
        case_to_name(DW_OP_, dup);
        case_to_name(DW_OP_, drop);
        case_to_name(DW_OP_, over);
        case_to_name(DW_OP_, pick);
        case_to_name(DW_OP_, swap);
        case_to_name(DW_OP_, rot);
        case_to_name(DW_OP_, xderef);
        case_to_name(DW_OP_, abs);
        case_to_name(DW_OP_, and);
        case_to_name(DW_OP_, div);
        case_to_name(DW_OP_, minus);
        case_to_name(DW_OP_, mod);
        case_to_name(DW_OP_, mul);
        case_to_name(DW_OP_, neg);
        case_to_name(DW_OP_, not);
        case_to_name(DW_OP_, or);
        case_to_name(DW_OP_, plus);
        case_to_name(DW_OP_, plus_uconst);
        case_to_name(DW_OP_, shl);
        case_to_name(DW_OP_, shr);
        case_to_name(DW_OP_, shra);
        case_to_name(DW_OP_, xor);
        case_to_name(DW_OP_, bra);
        case_to_name(DW_OP_, eq);
        case_to_name(DW_OP_, ge);
        case_to_name(DW_OP_, gt);
        case_to_name(DW_OP_, le);
        case_to_name(DW_OP_, lt);
        case_to_name(DW_OP_, ne);
        case_to_name(DW_OP_, skip);
        case_to_name(DW_OP_, lit0);
        case_to_name(DW_OP_, lit1);
        case_to_name(DW_OP_, lit2);
        case_to_name(DW_OP_, lit3);
        case_to_name(DW_OP_, lit4);
        case_to_name(DW_OP_, lit5);
        case_to_name(DW_OP_, lit6);
        case_to_name(DW_OP_, lit7);
        case_to_name(DW_OP_, lit8);
        case_to_name(DW_OP_, lit9);
        case_to_name(DW_OP_, lit10);
        case_to_name(DW_OP_, lit11);
        case_to_name(DW_OP_, lit12);
        case_to_name(DW_OP_, lit13);
        case_to_name(DW_OP_, lit14);
        case_to_name(DW_OP_, lit15);
        case_to_name(DW_OP_, lit16);
        case_to_name(DW_OP_, lit17);
        case_to_name(DW_OP_, lit18);
        case_to_name(DW_OP_, lit19);
        case_to_name(DW_OP_, lit20);
        case_to_name(DW_OP_, lit21);
        case_to_name(DW_OP_, lit22);
        case_to_name(DW_OP_, lit23);
        case_to_name(DW_OP_, lit24);
        case_to_name(DW_OP_, lit25);
        case_to_name(DW_OP_, lit26);
        case_to_name(DW_OP_, lit27);
        case_to_name(DW_OP_, lit28);
        case_to_name(DW_OP_, lit29);
        case_to_name(DW_OP_, lit30);
        case_to_name(DW_OP_, lit31);
        case_to_name(DW_OP_, reg0);
        case_to_name(DW_OP_, reg1);
        case_to_name(DW_OP_, reg2);
        case_to_name(DW_OP_, reg3);
        case_to_name(DW_OP_, reg4);
        case_to_name(DW_OP_, reg5);
        case_to_name(DW_OP_, reg6);
        case_to_name(DW_OP_, reg7);
        case_to_name(DW_OP_, reg8);
        case_to_name(DW_OP_, reg9);
        case_to_name(DW_OP_, reg10);
        case_to_name(DW_OP_, reg11);
        case_to_name(DW_OP_, reg12);
        case_to_name(DW_OP_, reg13);
        case_to_name(DW_OP_, reg14);
        case_to_name(DW_OP_, reg15);
        case_to_name(DW_OP_, reg16);
        case_to_name(DW_OP_, reg17);
        case_to_name(DW_OP_, reg18);
        case_to_name(DW_OP_, reg19);
        case_to_name(DW_OP_, reg20);
        case_to_name(DW_OP_, reg21);
        case_to_name(DW_OP_, reg22);
        case_to_name(DW_OP_, reg23);
        case_to_name(DW_OP_, reg24);
        case_to_name(DW_OP_, reg25);
        case_to_name(DW_OP_, reg26);
        case_to_name(DW_OP_, reg27);
        case_to_name(DW_OP_, reg28);
        case_to_name(DW_OP_, reg29);
        case_to_name(DW_OP_, reg30);
        case_to_name(DW_OP_, reg31);
        case_to_name(DW_OP_, breg0);
        case_to_name(DW_OP_, breg1);
        case_to_name(DW_OP_, breg2);
        case_to_name(DW_OP_, breg3);
        case_to_name(DW_OP_, breg4);
        case_to_name(DW_OP_, breg5);
        case_to_name(DW_OP_, breg6);
        case_to_name(DW_OP_, breg7);
        case_to_name(DW_OP_, breg8);
        case_to_name(DW_OP_, breg9);
        case_to_name(DW_OP_, breg10);
        case_to_name(DW_OP_, breg11);
        case_to_name(DW_OP_, breg12);
        case_to_name(DW_OP_, breg13);
        case_to_name(DW_OP_, breg14);
        case_to_name(DW_OP_, breg15);
        case_to_name(DW_OP_, breg16);
        case_to_name(DW_OP_, breg17);
        case_to_name(DW_OP_, breg18);
        case_to_name(DW_OP_, breg19);
        case_to_name(DW_OP_, breg20);
        case_to_name(DW_OP_, breg21);
        case_to_name(DW_OP_, breg22);
        case_to_name(DW_OP_, breg23);
        case_to_name(DW_OP_, breg24);
        case_to_name(DW_OP_, breg25);
        case_to_name(DW_OP_, breg26);
        case_to_name(DW_OP_, breg27);
        case_to_name(DW_OP_, breg28);
        case_to_name(DW_OP_, breg29);
        case_to_name(DW_OP_, breg30);
        case_to_name(DW_OP_, breg31);
        case_to_name(DW_OP_, regx);
        case_to_name(DW_OP_, fbreg);
        case_to_name(DW_OP_, bregx);
        case_to_name(DW_OP_, piece);
        case_to_name(DW_OP_, deref_size);
        case_to_name(DW_OP_, xderef_size);
        case_to_name(DW_OP_, nop);
        case_to_name(DW_OP_, push_object_address);
        case_to_name(DW_OP_, call2);
        case_to_name(DW_OP_, call4);
        case_to_name(DW_OP_, call_ref);
        case_to_name(DW_OP_, form_tls_address);
        case_to_name(DW_OP_, call_frame_cfa);
        case_to_name(DW_OP_, bit_piece);
        case_to_name(DW_OP_, implicit_value);
        case_to_name(DW_OP_, stack_value);
        case_to_name(DW_OP_, implicit_pointer);
        case_to_name(DW_OP_, addrx);
        case_to_name(DW_OP_, constx);
        case_to_name(DW_OP_, entry_value);
        case_to_name(DW_OP_, const_type);
        case_to_name(DW_OP_, regval_type);
        case_to_name(DW_OP_, deref_type);
        case_to_name(DW_OP_, xderef_type);
        case_to_name(DW_OP_, convert);
        case_to_name(DW_OP_, reinterpret);
        case_to_name(DW_OP_, lo_user);
        case_to_name(DW_OP_, hi_user);
    }
}

STRUCT(DwarfLineHeader)
{
    u32 unit_length;
    u16 version;
    u8 address_size;
    u8 segment_selector_size;
    u32 header_length;
    u8 minimum_instruction_length;
    u8 maximum_operations_per_instruction;
    u8 default_is_stmt;
    s8 line_base;
    u8 line_range;
    u8 opcode_base;
    // TODO: add dynamic fields
};

STRUCT(DwarfCompilationUnit)
{
    u32 length;
    u16 version;
    DwarfUnitType type;
    u8 address_size;
    u32 debug_abbreviation_offset;
};

STRUCT(DwarfStringOffsetsTableHeader)
{
    u32 unit_length;
    u16 version;
    u16 reserved;
};

STRUCT(DwarfAddressTableHeader)
{
    u32 unit_length;
    u16 version;
    u8 address_size;
    u8 segment_selector_size;
};

typedef enum TypeId : u32
{
    // Simple types
    TYPE_BOTTOM = 0,
    TYPE_TOP,
    TYPE_LIVE_CONTROL,
    TYPE_DEAD_CONTROL,
    // Not simple types
    TYPE_INTEGER,
    TYPE_TUPLE,

    TYPE_COUNT,
} TypeId;

STRUCT(TypeIndex)
{
    u32 index;
};

#define index_equal(a, b) (a.index == b.index)
static_assert(sizeof(TypeIndex) == sizeof(u32));
declare_slice(TypeIndex);

STRUCT(TypeInteger)
{
    u64 constant;
    u8 bit_count;
    u8 is_constant;
    u8 is_signed;
    u8 padding1;
    u32 padding;
};
static_assert(sizeof(TypeInteger) == 16);

STRUCT(TypeTuple)
{
    Slice(TypeIndex) types;
};

STRUCT(Type)
{
    Hash64 hash;
    union
    {
        TypeInteger integer;
        TypeTuple tuple;
    };
    TypeId id;
};
static_assert(offsetof(Type, hash) == 0);
decl_vb(Type);

STRUCT(DebugTypeIndex)
{
    u32 index;
};

STRUCT(DebugTypeInteger)
{
    u8 bit_count:7;
    u8 signedness:1;
};

typedef enum DebugTypeId : u8
{
    DEBUG_TYPE_VOID = 0,
    DEBUG_TYPE_INTEGER,
} DebugTypeId;

STRUCT(DebugType)
{
    union
    {
        DebugTypeInteger integer;
    };
    DebugTypeId id;
};
decl_vb(DebugType);
declare_ip(DebugType);

typedef enum BackendTypeId
{
    BACKEND_TYPE_VOID = 0x00,
    BACKEND_TYPE_INTEGER_8 = 0x01,
    BACKEND_TYPE_INTEGER_16 = 0x02,
    BACKEND_TYPE_INTEGER_32 = 0x03,
    BACKEND_TYPE_INTEGER_64 = 0x03,
    BACKEND_TYPE_POINTER = 0x04,
    BACKEND_TYPE_SCALAR_LAST = BACKEND_TYPE_POINTER,
    BACKEND_TYPE_TUPLE,
    BACKEND_TYPE_MEMORY,
    BACKEND_TYPE_CONTROL,
    BACKEND_TYPE_REGION,
} BackendTypeId;

STRUCT(TypePair)
{
    u32 raw;
};
decl_vb(TypePair);
global const TypePair type_pair_invalid;

global const u32 debug_mask = 0xffffff;

fn TypePair type_pair_make(DebugTypeIndex debug_type, BackendTypeId backend_type)
{
    u32 value = backend_type;
    value <<= 24;
    auto debug_raw = *(u32*)&debug_type;
    assert(debug_raw <= debug_mask);
    value |= debug_raw;

    return (TypePair){ .raw = value };
}

// fn DebugTypeIndex type_pair_get_debug(TypePair type_pair)
// {
//     return (DebugTypeIndex) {
//         .index = type_pair.raw & debug_mask,
//     };
// }

fn BackendTypeId type_pair_get_backend(TypePair type_pair)
{
    return type_pair.raw >> 24;
}

STRUCT(NodeIndex)
{
    u32 index;
};
declare_slice(NodeIndex);
decl_vb(NodeIndex);

STRUCT(Function)
{
    String name;
    NodeIndex root;
    TypePair return_type;
    u32 line;
    u32 column;
};
decl_vb(Function);

typedef enum NodeId : u8
{
    IR_ROOT,
    IR_PROJECTION,
    IR_RETURN,
    IR_REGION,
    IR_PHI,
    IR_SYMBOL_TABLE,

    // Binary integer
    IR_INTEGER_ADD,
    IR_INTEGER_SUBSTRACT,
    IR_INTEGER_MULTIPLY,
    IR_INTEGER_DIVIDE,
    IR_INTEGER_REMAINDER,

    IR_INTEGER_SHIFT_LEFT,
    IR_INTEGER_SHIFT_RIGHT,
    IR_INTEGER_AND,
    IR_INTEGER_OR,
    IR_INTEGER_XOR,

    IR_INTEGER_COMPARE_EQUAL,
    IR_INTEGER_COMPARE_NOT_EQUAL,

    // Unary integer
    IR_INTEGER_NEGATION,

    IR_INTEGER_CONSTANT,

    MACHINE_COPY,
    MACHINE_MOVE,
    MACHINE_JUMP,

    NODE_COUNT,
} NodeId;

// struct NodeCFG
// {
//     s32 immediate_dominator_tree_depth;
//     s32 loop_depth;
//     s32 anti_dependency;
// };
// typedef struct NodeCFG NodeCFG;

// struct NodeConstant
// {
//     TypeIndex type;
// };
// typedef struct NodeConstant NodeConstant;
//
// struct NodeStart
// {
//     // NodeCFG cfg;
//     TypeIndex arguments;
//     Function* function;
// };
// typedef struct NodeStart NodeStart;
//
// // struct NodeStop
// // {
// //     // NodeCFG cfg;
// // };
// // typedef struct NodeStop NodeStop;
//
// struct ScopePair
// {
//     StringMap values;
//     StringMap types;
// };
// typedef struct ScopePair ScopePair;
//
// struct StackScope
// {
//     ScopePair* pointer;
//     u32 length;
//     u32 capacity;
// };
// typedef struct StackScope StackScope;
//
// struct NodeScope
// {
//     StackScope stack;
// };
// typedef struct NodeScope NodeScope;
//
//
// struct NodeControlProjection
// {
//     NodeProjection projection;
//     // NodeCFG cfg;
// };
// typedef struct NodeControlProjection NodeControlProjection;
//
// struct NodeReturn
// {
//     // NodeCFG cfg;
// };
// typedef struct NodeReturn NodeReturn;
//
// struct NodeDeadControl
// {
//     // NodeCFG cfg;
// };
// typedef struct NodeDeadControl NodeDeadControl;
STRUCT(NodeProjection)
{
    u32 index;
};

STRUCT(NodeRoot)
{
    u32 function_index;
};

STRUCT(NodeRegion)
{
    NodeIndex in_mem;
};

UNION(NodeIntegerConstant)
{
    s64 signed_value;
    u64 unsigned_value;
};

STRUCT(RegisterMaskIndex)
{
    u32 index;
};
declare_slice(RegisterMaskIndex);

STRUCT(NodeMachineCopy)
{
    RegisterMaskIndex use_mask;
    RegisterMaskIndex def_mask;
};

STRUCT(Node)
{
    u32 input_offset;
    u32 output_offset;
    u16 output_count;
    u16 input_count;
    u16 input_capacity;
    u16 output_capacity;
    TypePair type;
    NodeId id:8;
    u32 interned:1;
    u32 reserved:23;
    NodeIndex next_free;

    union
    {
        NodeProjection projection;
        NodeRoot root;
        NodeRegion region;
        NodeIntegerConstant integer_constant;
        NodeMachineCopy machine_copy;
    };
    // union
    // {
    //     NodeConstant constant;
    //     NodeStart start;
    //     NodeStop stop;
    //     NodeScope scope;
    //     NodeControlProjection control_projection;
    //     NodeProjection projection;
    //     NodeReturn return_node;
    //     NodeDeadControl dead_control;
    // };
};
// See above bitset
static_assert(sizeof(NodeId) == 1);

declare_slice_p(Node);
decl_vb(Node);
decl_vbp(Node);
declare_ip(Node);

fn u8 node_is_control_projection(Node* restrict node)
{
    return node->id == IR_PROJECTION && type_pair_get_backend(node->type) == BACKEND_TYPE_TUPLE;
}

fn u8 node_is_cfg_fork(Node* restrict node)
{
    switch (node->id)
    {
    case IR_ROOT:
    case IR_PROJECTION:
    case IR_RETURN:
    case IR_REGION:
    case IR_PHI:
    case IR_SYMBOL_TABLE:
    case IR_INTEGER_ADD:
    case IR_INTEGER_SUBSTRACT:
    case IR_INTEGER_MULTIPLY:
    case IR_INTEGER_DIVIDE:
    case IR_INTEGER_REMAINDER:
    case IR_INTEGER_SHIFT_LEFT:
    case IR_INTEGER_SHIFT_RIGHT:
    case IR_INTEGER_AND:
    case IR_INTEGER_OR:
    case IR_INTEGER_XOR:
    case IR_INTEGER_COMPARE_EQUAL:
    case IR_INTEGER_COMPARE_NOT_EQUAL:
    case IR_INTEGER_NEGATION:
    case IR_INTEGER_CONSTANT:
    case MACHINE_COPY:
    case MACHINE_MOVE:
    case MACHINE_JUMP:
    case NODE_COUNT:
        return 0;
    }
}

STRUCT(ArrayReference)
{
    u32 offset;
    u32 length;
};
decl_vb(ArrayReference);

STRUCT(File)
{
    String path;
    String source;
    // StringMap values;
    // StringMap types;
};

STRUCT(FunctionBuilder)
{
    Function* function;
    File* file;
    NodeIndex current;
};

typedef u64 BitsetElement;
decl_vb(BitsetElement);
declare_slice(BitsetElement);

STRUCT(Bitset)
{
    VirtualBuffer(BitsetElement) arr;
    u32 length;
};
const u64 element_bitsize = sizeof(u64) * 8;

fn u8 bitset_get(Bitset* bitset, u64 index)
{
    auto element_index = index / element_bitsize;
    if (element_index < bitset->arr.length)
    {
        auto bit_index = index % element_bitsize;
        u8 result = (bitset->arr.pointer[element_index] & (1 << bit_index)) != 0;
        return result;
    }

    return 0;
}

fn void bitset_ensure_length(Bitset* bitset, u64 max)
{
    auto length = (max / element_bitsize) + (max % element_bitsize != 0);
    auto old_length = bitset->arr.length;
    if (old_length < length)
    {
        auto new_element_count = cast(u32, u64, length - old_length);
        unused(vb_add(&bitset->arr, new_element_count));
    }
}

fn void bitset_set_value(Bitset* bitset, u64 index, u8 value)
{
    bitset_ensure_length(bitset, index + 1);
    auto element_index = index / element_bitsize;
    auto bit_index = index % element_bitsize;
    auto mask = ~((u64)1 << bit_index);
    bitset->arr.pointer[element_index] = (bitset->arr.pointer[element_index] & mask) | ((u64)(!!value) << bit_index);

    if (value)
    {
        bitset->length += 1;
    }
    else
    {
        bitset->length -= 1;
    }

    assert(bitset_get(bitset, index) == value);
}

fn void bitset_clear(Bitset* bitset)
{
    memset(bitset->arr.pointer, 0, bitset->arr.capacity);
    bitset->arr.length = 0;
    bitset->length = 0;
}

STRUCT(WorkList)
{
    VirtualBuffer(NodeIndex) nodes;
    Bitset visited;
    Bitset bitset;
    u32 mid_assert:1;
};

enum
{
    REGISTER_CLASS_STACK = 0,
};

typedef enum x86_64_RegisterClass : u8
{
    REGISTER_CLASS_X86_64_GPR = 1,
    REGISTER_CLASS_X86_64_XMM,
    REGISTER_CLASS_X86_64_COUNT
} x86_64_RegisterClass;

const global u8 register_count_per_class[] = {
    [0] = 0,
    [REGISTER_CLASS_X86_64_GPR] = 16,
    [REGISTER_CLASS_X86_64_XMM] = 16,
};
static_assert(array_length(register_count_per_class) == REGISTER_CLASS_X86_64_COUNT);

typedef enum GPR : u8
{
    RAX = 0,
    RCX = 1,
    RDX = 2,
    RBX = 3,
    RSP = 4,
    RBP = 5,
    RSI = 6,
    RDI = 7,

    R8 = 8 + 0,
    R9 = 8 + 1,
    R10 = 8 + 2,
    R11 = 8 + 3,
    R12 = 8 + 4,
    R13 = 8 + 5,
    R14 = 8 + 6,
    R15 = 8 + 7,

    GPR_NONE = -1
} GPR;

typedef enum RegisterMask_x86_64: u8
{
    REGISTER_MASK_EMPTY = 0,
    REGISTER_MASK_GPR = 1,
} RegisterMask_x86_64;
const global auto empty_register_mask = Index(RegisterMask, REGISTER_MASK_EMPTY);

STRUCT(RegisterMask)
{
    u32 mask;
    u32 class:3;
    u32 may_spill:1;
    u32 reserved:28;
};
decl_vb(RegisterMask);
declare_ip(RegisterMask);

STRUCT(Thread)
{
    Arena* arena;
    struct
    {
        VirtualBuffer(Type) types;
        VirtualBuffer(Node) nodes;
        VirtualBuffer(DebugType) debug_types;
        VirtualBuffer(NodeIndex) uses;
        VirtualBuffer(ArrayReference) use_free_list;
        VirtualBuffer(Function) functions;
        VirtualBuffer(u8) string;
        VirtualBuffer(RegisterMask) register_masks;
    } buffer;
    struct
    {
        InternPool(Node) nodes;
        InternPool(DebugType) debug_types;
        InternPool(RegisterMask) register_masks;
    } interned;
    struct
    {
        NodeIndex nodes;
    } free_list;
    struct
    {
        TypeIndex bottom;
        TypeIndex top;
        TypeIndex live_control;
        TypeIndex dead_control;
        struct
        {
            TypeIndex top;
            TypeIndex bottom;
            TypeIndex zero;
            TypeIndex u8;
            TypeIndex u16;
            TypeIndex u32;
            TypeIndex u64;
            TypeIndex s8;
            TypeIndex s16;
            TypeIndex s32;
            TypeIndex s64;
        } integer;
        struct
        {
            union
            {
                struct
                {
                    DebugTypeIndex u8;
                    DebugTypeIndex u16;
                    DebugTypeIndex u32;
                    DebugTypeIndex u64;
                    DebugTypeIndex s8;
                    DebugTypeIndex s16;
                    DebugTypeIndex s32;
                    DebugTypeIndex s64;
                };
                DebugTypeIndex array[8];
            } integer;
        } debug;
    } types;
    s64 main_function;
    WorkList worklists[8];
    u64 worklist_bitset:3;
    u64 reserved:61;
};

STRUCT(WorkListHandle)
{
    u8 index:3;
    u8 is_valid:1;
    u8 reserved:4;
};

fn WorkListHandle thread_worklist_acquire(Thread* thread)
{
    u8 bitset = thread->worklist_bitset;
    if (bitset)
    {
        auto index = cast(u8, s32, __builtin_ctz(~thread->worklist_bitset));
        thread->worklist_bitset |= (1 << index);
        return (WorkListHandle)
        {
            .index = index,
            .is_valid = 1,
        };
    }
    else
    {
        thread->worklist_bitset |= (1 << 0);
        return (WorkListHandle)
        {
            .index = 0,
            .is_valid = 1,
        };
    }
}

fn u32 thread_worklist_length(Thread* thread, WorkListHandle handle)
{
    assert(handle.is_valid);
    assert((thread->worklist_bitset & (1 << handle.index)) != 0);
    return thread->worklists[handle.index].nodes.length;
}

fn NodeIndex thread_worklist_get(Thread* thread, WorkListHandle handle, u32 index)
{
    assert(handle.is_valid);
    assert((thread->worklist_bitset & (1 << handle.index)) != 0);
    auto* worklist = &thread->worklists[handle.index];
    assert(index < worklist->nodes.length);
    return worklist->nodes.pointer[index];
}

fn u8 thread_worklist_test(Thread* thread, WorkListHandle handle, NodeIndex node_index)
{
    assert(handle.is_valid);
    assert((thread->worklist_bitset & (1 << handle.index)) != 0);

    u8 result = 0;
    if (validi(node_index))
    {
        WorkList* restrict worklist = &thread->worklists[handle.index];
        result = bitset_get(&worklist->bitset, geti(node_index));
    }

    return result;
}

fn u8 thread_worklist_test_and_set(Thread* thread, WorkListHandle handle, NodeIndex node_index)
{
    auto result = thread_worklist_test(thread, handle, node_index);
    if (!result)
    {
        WorkList* restrict worklist = &thread->worklists[handle.index];
        bitset_set_value(&worklist->bitset, geti(node_index), 1);
    }

    return result;
}

fn Node* thread_node_get(Thread* thread, NodeIndex node_index)
{
    assert(validi(node_index));
    auto* node = &thread->buffer.nodes.pointer[geti(node_index)];
    return node;
}

may_be_unused fn String node_id_to_string(NodeId node_id)
{
    switch (node_id)
    {
    case_to_name(IR_, ROOT);
    case_to_name(IR_, PROJECTION);
    case_to_name(IR_, RETURN);
    case_to_name(IR_, REGION);
    case_to_name(IR_, PHI);
    case_to_name(IR_, SYMBOL_TABLE);
    case_to_name(IR_, INTEGER_ADD);
    case_to_name(IR_, INTEGER_SUBSTRACT);
    case_to_name(IR_, INTEGER_MULTIPLY);
    case_to_name(IR_, INTEGER_DIVIDE);
    case_to_name(IR_, INTEGER_REMAINDER);
    case_to_name(IR_, INTEGER_SHIFT_LEFT);
    case_to_name(IR_, INTEGER_SHIFT_RIGHT);
    case_to_name(IR_, INTEGER_AND);
    case_to_name(IR_, INTEGER_OR);
    case_to_name(IR_, INTEGER_XOR);
    case_to_name(IR_, INTEGER_COMPARE_EQUAL);
    case_to_name(IR_, INTEGER_COMPARE_NOT_EQUAL);
    case_to_name(IR_, INTEGER_NEGATION);
    case_to_name(IR_, INTEGER_CONSTANT);
    case_to_name(MACHINE_, COPY);
    case_to_name(MACHINE_, MOVE);
    case_to_name(MACHINE_, JUMP);
    case NODE_COUNT: unreachable();
      break;
    }
}

fn void thread_worklist_push(Thread* thread, WorkListHandle handle, NodeIndex node_index)
{
    // print("Pushing node #{u32} ({s})\n", geti(node_index), node_id_to_string(thread_node_get(thread, node_index)->id));
    if (!thread_worklist_test_and_set(thread, handle, node_index))
    {
        WorkList* restrict worklist = &thread->worklists[handle.index];
        *vb_add(&worklist->nodes, 1) = node_index;
    }
}

fn void thread_worklist_push_array(Thread* thread, WorkListHandle handle, NodeIndex node_index)
{
    assert(handle.is_valid);

    auto* worklist = &thread->worklists[handle.index];
    *vb_add(&worklist->nodes, 1) = node_index;
}

fn NodeIndex thread_worklist_pop_array(Thread* thread, WorkListHandle handle)
{
    assert(handle.is_valid);
    assert((thread->worklist_bitset & (1 << handle.index)) != 0);
    auto result = invalidi(Node);

    assert(handle.is_valid);
    auto* worklist = &thread->worklists[handle.index];
    auto len = worklist->nodes.length;
    if (len)
    {
        auto index = len - 1;
        result = worklist->nodes.pointer[index];
        worklist->nodes.length = index;
    }

    return result;
}

fn NodeIndex thread_worklist_pop(Thread* thread, WorkListHandle handle)
{
    assert(handle.is_valid);
    assert((thread->worklist_bitset & (1 << handle.index)) != 0);
    auto result = invalidi(Node);

    assert(handle.is_valid);
    auto* worklist = &thread->worklists[handle.index];
    auto len = worklist->nodes.length;
    if (len)
    {
        auto index = len - 1;
        auto node_index = worklist->nodes.pointer[index];
        worklist->nodes.length = index;
        bitset_set_value(&worklist->bitset, index, 0);
        result = node_index;
    }

    return result;
}

fn void thread_worklist_clear(Thread* thread, WorkListHandle handle)
{
    assert(handle.is_valid);
    assert((thread->worklist_bitset & (1 << handle.index)) != 0);
    auto* restrict worklist = &thread->worklists[handle.index];

    bitset_clear(&worklist->visited);
    bitset_clear(&worklist->bitset);
    worklist->nodes.length = 0;
}

// fn void thread_worklist_release(Thread* thread, WorkListHandle* handle)
// {
//     thread_worklist_clear(thread, *handle);
//     handle->is_valid = 0;
// }

fn Type* thread_type_get(Thread* thread, TypeIndex type_index)
{
    assert(validi(type_index));
    auto* type = &thread->buffer.types.pointer[geti(type_index)];
    return type;
}


fn DebugType* thread_debug_type_get(Thread* thread, DebugTypeIndex debug_type_index)
{
    assert(validi(debug_type_index));
    auto* type = &thread->buffer.debug_types.pointer[geti(debug_type_index)];
    return type;
}

fn RegisterMask* thread_register_mask_get(Thread* thread, RegisterMaskIndex register_mask_index)
{
    assert(validi(register_mask_index));
    auto* register_mask = &thread->buffer.register_masks.pointer[geti(register_mask_index)];
    return register_mask;
}

fn void thread_node_set_use(Thread* thread, u32 offset, u16 index, NodeIndex new_use)
{
    thread->buffer.uses.pointer[offset + index] = new_use;
}

// fn NodeIndex thread_node_get_use(Thread* thread, u32 offset, u16 index)
// {
//     NodeIndex i = thread->buffer.uses.pointer[offset + index];
//     return i;
// }

// fn NodeIndex node_input_get(Thread* thread, Node* node, u16 index)
// {
//     assert(index < node->input_count);
//     NodeIndex result = thread_node_get_use(thread, node->input_offset, index);
//     return result;
// }
//
// fn NodeIndex node_output_get(Thread* thread, Node* node, u16 index)
// {
//     assert(index < node->output_count);
//     NodeIndex result = thread_node_get_use(thread, node->output_offset, index);
//     return result;
// }

// fn NodeIndex scope_get_control(Thread* thread, Node* node)
// {
//     assert(node->id == NODE_SCOPE);
//     auto control = node_input_get(thread, node, 0);
//     return control;
// }

// fn NodeIndex builder_get_control_node_index(Thread* thread, FunctionBuilder* builder)
// {
//     auto* scope_node = thread_node_get(thread, builder->scope);
//     auto result = scope_get_control(thread, scope_node);
//     return result;
// }

STRUCT(UseReference)
{
    NodeIndex* pointer;
    u32 index;
};

fn UseReference thread_get_node_reference_array(Thread* thread, u16 count)
{
    u32 free_list_count = thread->buffer.use_free_list.length;
    for (u32 i = 0; i < free_list_count; i += 1)
    {
        if (thread->buffer.use_free_list.pointer[i].length >= count)
        {
            trap();
        }
    }

    u32 index = thread->buffer.uses.length;
    auto* node_indices = vb_add(&thread->buffer.uses, count);
    return (UseReference)
    {
        .pointer = node_indices,
        .index = index,
    };
}

fn void node_ensure_capacity(Thread* thread, u32* offset, u16* capacity, u16 current_length, u16 additional)
{
    auto current_offset = *offset;
    auto current_capacity = *capacity;
    auto desired_capacity = cast(u16, u32, current_length + additional);

    if (desired_capacity > current_capacity)
    {
        auto* ptr = vb_add(&thread->buffer.uses, desired_capacity);
        u32 new_offset = cast(u32, s64, ptr - thread->buffer.uses.pointer);
        memcpy(ptr, &thread->buffer.uses.pointer[current_offset], current_length * sizeof(NodeIndex));
        memset(ptr + current_length, 0, (desired_capacity - current_length) * sizeof(NodeIndex));
        *offset = new_offset;
        *capacity = desired_capacity;
    }
}

fn void node_add_one_assume_capacity(Thread* thread, NodeIndex node, u32 offset, u16 capacity, u16* length)
{
    auto index = *length;
    assert(index < capacity);
    thread->buffer.uses.pointer[offset + index] = node;
    *length = index + 1;
}

fn void node_add_one(Thread* thread, u32* offset, u16* capacity, u16* count, NodeIndex node_index)
{
    node_ensure_capacity(thread, offset, capacity, *count, 1);
    node_add_one_assume_capacity(thread, node_index, *offset, *capacity, count);
}

fn NodeIndex node_add_output(Thread* thread, NodeIndex node_index, NodeIndex output_index)
{
    auto* this_node = thread_node_get(thread, node_index);
    node_add_one(thread, &this_node->output_offset, &this_node->output_capacity, &this_node->output_count, output_index);

    return node_index;
}

// fn NodeIndex intern_pool_remove_node(Thread* thread, NodeIndex node_index);

fn Slice(NodeIndex) node_get_inputs(Thread* thread, const Node * restrict const node)
{
    auto result = (Slice(NodeIndex)) {
        .pointer = &thread->buffer.uses.pointer[node->input_offset],
        .length = node->input_count,
    };
    return result;
}

fn Slice(NodeIndex) node_get_outputs(Thread* thread, Node* node)
{
    auto result = (Slice(NodeIndex)) {
        .pointer = &thread->buffer.uses.pointer[node->output_offset],
        .length = node->output_count,
    };
    return result;
}

fn u8 node_is_constant(const Node* const restrict node)
{
    switch (node->id)
    {
        case IR_INTEGER_CONSTANT:
            return 1;
        default:
            return 0;
    }
}

fn Hash32 node_hash(Thread* thread, const Node* restrict const node)
{
    Hash32 hash = 0;
    hash += node->id;
    hash += sizeof(u8);

    auto inputs = node_get_inputs(thread, node);

    // Constants are allowed to live across functions
    if (!node_is_constant(node))
    {
        u32 valid_input_count = 0;

        for (u16 i = 0; i < inputs.length; i += 1)
        {
            auto input = inputs.pointer[i];
            if (validi(input))
            {
                valid_input_count += 1;
                hash += geti(input);
                hash += sizeof(input);
            }
        }

        hash += valid_input_count;
        hash += sizeof(u16);
    }

    auto* union_start = (u8*)&node->projection;
    auto* union_end = union_start + size_until_end(Node, projection);
    for (auto* it = union_start; it < union_end; it += 1)
    {
        hash += *it;
    }
    hash += union_end - union_start;

    auto result = hash32_fib_end(hash);
    return result;
}

fn Hash32 node_hash_index(Thread* thread, NodeIndex node_index)
{
    auto* node = thread_node_get(thread, node_index);
    auto hash = node_hash(thread, node);
    return hash;
}

fn Hash32 register_mask_hash(Thread* thread, const RegisterMask* const restrict mask)
{
    unused(thread);
    static_assert(sizeof(RegisterMask) == sizeof(u64));
    auto hash = *(Hash64*)mask;

    auto result = hash64_fib_end(hash);
    return result;
}

fn Hash32 register_mask_hash_index(Thread* thread, RegisterMaskIndex register_mask_index)
{
    auto* mask = thread_register_mask_get(thread, register_mask_index);
    auto hash = register_mask_hash(thread, mask);
    return hash;
}

fn void node_gvn_remove(Thread* thread, NodeIndex node_index);

fn void node_unlock(Thread* thread, NodeIndex node_index)
{
    unused(thread);
    unused(node_index);
    // auto* node = thread_node_get(thread, node_index);
    // if (node->interned)
    // {
    //     auto r = node_gvn_remove(thread, node_index);
    //     assert(index_equal(r, node_index));
    // }
}

fn s64 node_find(Slice(NodeIndex) nodes, NodeIndex node_index)
{
    s64 result = -1;

    for (u64 i = 0; i < nodes.length; i += 1)
    {
        if (index_equal(nodes.pointer[i], node_index))
        {
            result = cast(s64, u64, i);
            break;
        }
    }

    return result;
}

fn void thread_node_remove_use(Thread* thread, u32 offset, u16* length, u16 index)
{
    auto current_length = *length;
    assert(index < current_length);
    auto item_to_remove = &thread->buffer.uses.pointer[offset + index];
    auto substitute = &thread->buffer.uses.pointer[offset + current_length - 1];
    *item_to_remove = *substitute;
    *length = current_length - 1;
}

fn u8 node_remove_output(Thread* thread, NodeIndex node_index, NodeIndex use_index)
{
    auto* node = thread_node_get(thread, node_index);
    auto outputs = node_get_outputs(thread, node);
    auto maybe_index = node_find(outputs, use_index);
    assert(maybe_index != -1);
    auto index = cast(u16, s64, maybe_index);
    thread_node_remove_use(thread, node->output_offset, &node->output_count, index);
    return node->output_count == 0;
}

// fn void move_dependencies_to_worklist(Thread* thread, Node* node)
// {
//     assert(node->dependency_count == 0);
//     for (u32 i = 0; i < node->dependency_count; i += 1)
//     {
//         unused(thread);
//         trap();
//     }
// }

// fn u8 node_is_unused(Node* node)
// {
//     return node->output_count == 0;
// }

// fn u8 node_is_dead(Node* node)
// {
//     return node_is_unused(node) & ((node->input_count == 0) & (!validi(node->type)));
// }

// fn void node_kill(Thread* thread, NodeIndex node_index)
// {
//     node_unlock(thread, node_index);
//     auto* node = thread_node_get(thread, node_index);
//     // print("[NODE KILLING] (#{u32}, {s}) START\n", node_index.index, node_id_to_string(node));
//     assert(node_is_unused(node));
//     todo();
//     // node->type = invalidi(TypePair);
//
//     auto inputs = node_get_inputs(thread, node);
//     while (node->input_count > 0)
//     {
//         auto input_index = cast(u16, u32, node->input_count - 1);
//         node->input_count = input_index;
//         auto old_input_index = inputs.pointer[input_index];
//
//         // print("[NODE KILLING] (#{u32}, {s}) Removing input #{u32} at slot {u32}\n", node_index.index, node_id_to_string(node), old_input_index.index, input_index);
//         if (validi(old_input_index))
//         {
//             thread_worklist_push(thread, old_input_index);
//             u8 no_more_outputs = node_remove_output(thread, old_input_index, node_index);
//             if (no_more_outputs)
//             {
//                 // print("[NODE KILLING] (#{u32}, {s}) (NO MORE OUTPUTS - KILLING) Input #{u32}\n", node_index.index, node_id_to_string(node), old_input_index.index);
//                 node_kill(thread, old_input_index);
//             }
//         }
//     }
//
//     assert(node_is_dead(node));
//     // print("[NODE KILLING] (#{u32}, {s}) END\n", node_index.index, node_id_to_string(node));
// }

fn NodeIndex node_set_input(Thread* thread, NodeIndex node_index, u16 index, NodeIndex new_input)
{
    auto* node = thread_node_get(thread, node_index);
    assert(index < node->input_count);
    node_unlock(thread, node_index);
    auto inputs = node_get_inputs(thread, node);
    auto old_input = inputs.pointer[index];

    if (!index_equal(old_input, new_input))
    {
        if (validi(new_input))
        {
            node_add_output(thread, new_input, node_index);
        }

        thread_node_set_use(thread, node->input_offset, index, new_input);

        if (validi(old_input))
        {
            if (node_remove_output(thread, old_input, node_index))
            {
                // todo();
                // node_kill(thread, old_input);
            }
        }

        // move_dependencies_to_worklist(thread, node);
    }

    return new_input;
}

// fn NodeIndex builder_set_control(Thread* thread, FunctionBuilder* builder, NodeIndex node_index)
// {
//     return node_set_input(thread, builder->scope, 0, node_index);
// }

fn NodeIndex node_add_input(Thread* thread, NodeIndex node_index, NodeIndex input_index)
{
    node_unlock(thread, node_index);
    Node* this_node = thread_node_get(thread, node_index);
    node_add_one(thread, &this_node->input_offset, &this_node->input_capacity, &this_node->input_count, input_index);
    if (validi(input_index))
    {
        node_add_output(thread, input_index, node_index);
    }

    return input_index;
}

STRUCT(NodeCreate)
{
    Slice(NodeIndex) inputs;
    TypePair type_pair;
    NodeId id;
};

fn NodeIndex thread_node_add(Thread* thread, NodeCreate data)
{
    auto input_count = cast(u16, u64, data.inputs.length);
    auto input_result = thread_get_node_reference_array(thread, input_count);
    memcpy(input_result.pointer, data.inputs.pointer, sizeof(NodeIndex) * input_count);

    auto* node = vb_add(&thread->buffer.nodes, 1);
    auto node_index = Index(Node, cast(u32, s64, node - thread->buffer.nodes.pointer));
    memset(node, 0, sizeof(Node));
    node->id = data.id;
    node->input_offset = input_result.index;
    node->input_count = input_count;
    node->input_capacity = input_count;
    node->type = type_pair_invalid;
    // node->type = invalidi(TypePair);
    node->type = data.type_pair;

    // print("[NODE CREATION] #{u32} {s} | INPUTS: { ", node_index.index, node_id_to_string(node));

    for (u16 i = 0; i < input_count; i += 1)
    {
        NodeIndex input = data.inputs.pointer[i];
        // print("{u32} ", input.index);
        if (validi(input))
        {
            node_add_output(thread, input, node_index);
        }
    }

    // print("}\n");

    return node_index;
}

// fn void node_pop_inputs(Thread* thread, NodeIndex node_index, u16 input_count)
// {
//     node_unlock(thread, node_index);
//     auto* node = thread_node_get(thread, node_index);
//     auto inputs = node_get_inputs(thread, node);
//     for (u16 i = 0; i < input_count; i += 1)
//     {
//         auto old_input = inputs.pointer[node->input_count - 1];
//         node->input_count -= 1;
//         if (validi(old_input))
//         {
//             if (node_remove_output(thread, old_input, node_index))
//             {
//                 trap();
//             }
//         }
//     }
// }

// fn void scope_push(Thread* thread, FunctionBuilder* builder)
// {
//     auto* scope = thread_node_get(thread, builder->scope);
//     auto current_length = scope->scope.stack.length;
//     auto desired_length = current_length + 1;
//     auto current_capacity = scope->scope.stack.capacity;
//
//     if (current_capacity < desired_length)
//     {
//         auto optimal_capacity = MAX(round_up_to_next_power_of_2(desired_length), 8);
//         auto* new_pointer = arena_allocate(thread->arena, ScopePair, optimal_capacity);
//         memcpy(new_pointer, scope->scope.stack.pointer, current_length * sizeof(ScopePair));
//         scope->scope.stack.capacity = optimal_capacity;
//         scope->scope.stack.pointer = new_pointer;
//     }
//
//     memset(&scope->scope.stack.pointer[current_length], 0, sizeof(ScopePair));
//     scope->scope.stack.length = current_length + 1;
// }

// fn void scope_pop(Thread* thread, FunctionBuilder* builder)
// {
//     auto scope_index = builder->scope;
//     auto* scope = thread_node_get(thread, scope_index);
//     auto index = scope->scope.stack.length - 1;
//     auto popped_scope = scope->scope.stack.pointer[index];
//     scope->scope.stack.length = index;
//     auto input_count = popped_scope.values.length;
//     node_pop_inputs(thread, scope_index, input_count);
// }

// fn ScopePair* scope_get_last(Node* node)
// {
//     assert(node->id == NODE_SCOPE);
//     return &node->scope.stack.pointer[node->scope.stack.length - 1];
// }

// fn NodeIndex scope_define(Thread* thread, FunctionBuilder* builder, String name, TypeIndex type_index, NodeIndex node_index)
// {
//     auto scope_node_index = builder->scope;
//     auto* scope_node = thread_node_get(thread, scope_node_index);
//     auto* last = scope_get_last(scope_node);
//     string_map_put(&last->types, thread->arena, name, geti(type_index));
//
//     auto existing = string_map_put(&last->values, thread->arena, name, scope_node->input_count).existing;
//     NodeIndex result;
//
//     if (existing)
//     {
//         result = invalidi(Node);
//     }
//     else
//     {
//         result = node_add_input(thread, scope_node_index, node_index);
//     }
//
//     return result;
// }

// fn NodeIndex scope_update_extended(Thread* thread, FunctionBuilder* builder, String name, NodeIndex node_index, s32 nesting_level)
// {
//     NodeIndex result = invalidi(Node);
//
//     if (nesting_level >= 0)
//     {
//         auto* scope_node = thread_node_get(thread, builder->scope);
//         auto* string_map = &scope_node->scope.stack.pointer[nesting_level].values;
//         auto lookup_result = string_map_get(string_map, name);
//         if (lookup_result.existing)
//         {
//             auto index = lookup_result.value;
//             auto old_index = node_input_get(thread, scope_node, index);
//             auto* old_node = thread_node_get(thread, old_index);
//
//             // if (old_node->id == NODE_SCOPE)
//             // {
//             //     trap();
//             // }
//
//             if (validi(node_index))
//             {
//                 auto result = node_set_input(thread, builder->scope, index, node_index);
//                 return result;
//             }
//             else
//             {
//                 return old_index;
//             }
//         }
//         else
//         {
//             return scope_update_extended(thread, builder, name, node_index, nesting_level - 1);
//         }
//     }
//
//     return result;
// }

// fn NodeIndex scope_lookup(Thread* thread, FunctionBuilder* builder, String name)
// {
//     auto* scope_node = thread_node_get(thread, builder->scope);
//     return scope_update_extended(thread, builder, name, invalidi(Node), scope_node->scope.stack.length - 1);
// }

// fn NodeIndex scope_update(Thread* thread, FunctionBuilder* builder, String name, NodeIndex value_node_index)
// {
//     auto* scope_node = thread_node_get(thread, builder->scope);
//     auto result = scope_update_extended(thread, builder, name, value_node_index, scope_node->scope.stack.length - 1);
//     return result;
// }

// fn u8 type_equal(Type* a, Type* b)
// {
//     u8 result = 0;
//     if (a == b)
//     {
//         result = 1;
//     }
//     else
//     {
//         assert(a->hash);
//         assert(b->hash);
//         if ((a->hash == b->hash) & (a->id == b->id))
//         {
//             switch (a->id)
//             {
//                 case TYPE_INTEGER:
//                     {
//                         result = 
//                             ((a->integer.constant == b->integer.constant) & (a->integer.bit_count == b->integer.bit_count))
//                             &
//                             ((a->integer.is_signed == b->integer.is_signed) & (a->integer.is_constant == b->integer.is_constant));
//                     } break;
//                 case TYPE_TUPLE:
//                     {
//                         result = a->tuple.types.length == b->tuple.types.length;
//
//                         if (result)
//                         {
//                             for (u32 i = 0; i < a->tuple.types.length; i += 1)
//                             {
//                                 if (!index_equal(a->tuple.types.pointer[i], b->tuple.types.pointer[i]))
//                                 {
//                                     todo();
//                                 }
//                             }
//                         }
//                     } break;
//                 default:
//                     trap();
//             }
//         }
//     }
//
//     return result;
// }

fn Hash64 node_get_hash_default(Thread* thread, Node* node, NodeIndex node_index, Hash64 hash)
{
    unused(thread);
    unused(node);
    unused(node_index);
    return hash;
}

// fn Hash64 node_get_hash_projection(Thread* thread, Node* node, NodeIndex node_index, Hash64 hash)
// {
//     unused(thread);
//     unused(node_index);
//     auto projection_index = node->projection.index;
//     auto proj_index_bytes = struct_to_bytes(projection_index);
//     for (u32 i = 0; i < proj_index_bytes.length; i += 1)
//     {
//         hash = hash_byte(hash, proj_index_bytes.pointer[i]);
//     }
//
//     return hash;
// }

// fn Hash64 node_get_hash_control_projection(Thread* thread, Node* node, NodeIndex node_index, Hash64 hash)
// {
//     unused(thread);
//     unused(node_index);
//     auto projection_index = node->control_projection.projection.index;
//     auto proj_index_bytes = struct_to_bytes(projection_index);
//     for (u32 i = 0; i < proj_index_bytes.length; i += 1)
//     {
//         hash = hash_byte(hash, proj_index_bytes.pointer[i]);
//     }
//
//     return hash;
// }

// fn Hash64 node_get_hash_constant(Thread* thread, Node* node, NodeIndex node_index, Hash64 hash)
// {
//     unused(node_index);
//     unused(thread);
//     unused(node);
//     assert(hash == fnv_offset);
//     todo();
//     // auto type_index = node->type;
//     // auto* type = thread_type_get(thread, node->type);
//     // auto type_hash = hash_type(thread, type);
//     // print("Hashing node #{u32} (constant) (type: #{u32}) (hash: {u64:x})\n", node_index.index, type_index.index, type_hash);
//     // return type_hash;
// }

// fn Hash64 node_get_hash_scope(Thread* thread, Node* node, NodeIndex node_index, Hash64 hash)
// {
//     unused(thread);
//     unused(node);
//     unused(node_index);
//     return hash;
// }

// fn NodeIndex node_idealize_substract(Thread* thread, NodeIndex node_index)
// {
//     auto* node = thread_node_get(thread, node_index);
//     auto inputs = node_get_inputs(thread, node);
//     auto left_node_index = inputs.pointer[1];
//     auto right_node_index = inputs.pointer[2];
//     auto* left = thread_node_get(thread, left_node_index);
//     auto* right = thread_node_get(thread, right_node_index);
//     if (index_equal(left_node_index, right_node_index))
//     {
//         trap();
//     }
//     else if (right->id == IR_INTEGER_NEGATION)
//     {
//         trap();
//     }
//     else if (left->id == IR_INTEGER_NEGATION)
//     {
//         trap();
//     }
//     else
//     {
//         return invalidi(Node);
//     }
// }

// fn NodeIndex node_idealize_compare(Thread* thread, NodeIndex node_index)
// {
//     auto* node = thread_node_get(thread, node_index);
//     auto inputs = node_get_inputs(thread, node);
//     auto left_node_index = inputs.pointer[1];
//     auto right_node_index = inputs.pointer[2];
//     auto* left = thread_node_get(thread, left_node_index);
//     auto* right = thread_node_get(thread, right_node_index);
//     if (index_equal(left_node_index, right_node_index))
//     {
//         trap();
//     }
//
//     if (node->id == IR_INTEGER_COMPARE_EQUAL)
//     {
//         if (right->id != IR_CONSTANT)
//         {
//             if (left->id == IR_CONSTANT)
//             {
//                 todo();
//             }
//             else if (left_node_index.index > right_node_index.index)
//             {
//                 todo();
//             }
//         }
//
//         // TODO: null pointer
//         if (index_equal(right->type, thread->types.integer.zero))
//         {
//             todo();
//         }
//     }
//
//     // TODO: phi constant
//     
//     return invalidi(Node);
// }

fn Hash32 debug_type_hash(Thread* thread, const DebugType* const restrict type)
{
    unused(thread);
    auto* start = (const u8*) type;
    Hash32 hash = 0;
    for (auto* it = start; it < start + sizeof(*type); it += 1)
    {
        hash += *it;
    }
    auto result = hash32_fib_end(hash);
    return result;
}

fn Hash32 debug_type_hash_index(Thread* thread, DebugTypeIndex type_index)
{
    auto* type = thread_debug_type_get(thread, type_index);
    return debug_type_hash(thread, type);
}

global const u64 intern_pool_min_capacity = 64;
STRUCT(GenericInternPool)
{
    u32* pointer;
    u32 length;
    u32 capacity;
};

STRUCT(GenericInternPoolBufferResult)
{
    void* pointer;
    u32 index;
};

STRUCT(GenericGetOrPut)
{
    u32 index;
    u8 existing;
};

typedef s64 FindSlotCallback(GenericInternPool* pool, Thread* thread, Hash32 hash, u32 raw_item_index, u32 saved_index, u32 slots_ahead);
typedef GenericInternPoolBufferResult AddToBufferCallback(Thread* thread);
// typedef s64 Find

STRUCT(InternPoolInterface)
{
    FindSlotCallback * const find_slot; 
    AddToBufferCallback* const add_to_buffer;
};

fn s64 ip_find_slot_debug_type(GenericInternPool* generic_pool, Thread* thread, Hash32 hash, u32 raw_item_index, u32 saved_index, u32 slots_ahead)
{
    auto* pool = (InternPool(DebugType)*)generic_pool;
    assert(pool == &thread->interned.debug_types);
    auto* ptr = pool->pointer;

    s64 result = -1;

    unused(raw_item_index);

    for (auto index = saved_index; index < saved_index + slots_ahead; index += 1)
    {
        auto typed_index = ptr[index];
        auto debug_type = thread_debug_type_get(thread, typed_index);
        auto existing_hash = debug_type_hash(thread, debug_type);
        if (existing_hash == hash)
        {
            todo();
        }
    }

    return result;
}

fn s64 ip_generic_find_slot(GenericInternPool* pool, Thread* thread, u32 item_index, Hash32 hash, const InternPoolInterface* restrict const interface)
{
    auto* pointer = pool->pointer;
    auto existing_capacity = pool->capacity;
    auto original_index = hash & (existing_capacity - 1);
    auto it_index = original_index;
    s64 result = -1;

    while (1)
    {
        auto index = it_index & (existing_capacity - 1);
        auto* ptr = &pointer[index];
        if (!*ptr)
        {
            result = index;
            break;
        }

#if (__AVX2__)
#if (__AVX512F__)
        auto chunk = _mm512_loadu_epi32(ptr);
        auto is_zero = _mm512_cmpeq_epi32_mask(chunk, _mm512_setzero_epi32());
#elif (__AVX2__)
        auto chunk = _mm256_loadu_si256(ptr);
        auto is_zero = _mm256_movemask_ps(_mm256_castsi256_ps(_mm256_cmpeq_epi32(chunk, _mm256_setzero_si256())));
#endif
        auto occupied_slots_ahead = cast(u32, s32, __builtin_ctz(is_zero));
#else
        u32 occupied_slots_ahead = 0;
        for (u32 fake_i = it_index; fake_i < it_index + existing_capacity; fake_i += 1)
        {
            auto i = fake_i & (existing_capacity - 1);
            auto item = pointer[i];
            if (item == 0)
            {
                break;
            }
            occupied_slots_ahead += 1;
        }
#endif

        auto cap_ahead = existing_capacity - index;
        auto slots_ahead = MIN(occupied_slots_ahead, cap_ahead);
        auto slot = interface->find_slot(pool, thread, hash, item_index, index, slots_ahead);

        if (slot != -1)
        {
            assert(pointer[slot] != 0);
            result = slot;
            break;
        }

        if (occupied_slots_ahead < cap_ahead)
        {
            result = index + occupied_slots_ahead;
            break;
        }

        it_index += slots_ahead;
    }

    return result;
}


fn GenericInternPoolBufferResult ip_DebugType_add_to_buffer(Thread* thread)
{
    auto* result = vb_add(&thread->buffer.debug_types, 1);
    auto buffer_index = cast(u32, s64, result - thread->buffer.debug_types.pointer);
    auto type_index = Index(DebugType, buffer_index);
    static_assert(sizeof(type_index) == sizeof(u32));
    return (GenericInternPoolBufferResult) {
        .pointer = result,
        .index = *(u32*)&type_index,
    };
}

fn u32 ip_generic_put_new_at_assume_not_existent_assume_capacity(GenericInternPool* pool, Thread* thread, u32 item_index, const void* restrict const item_pointer, u32 item_size, u32 pool_index, const InternPoolInterface* restrict const interface)
{
    if (!item_index)
    {
        auto buffer_result = interface->add_to_buffer(thread);
        assert(buffer_result.index);
        memcpy(buffer_result.pointer, item_pointer, item_size);
        item_index = buffer_result.index;
    }
    auto* ptr = &pool->pointer[pool_index];
    *ptr = item_index;
    pool->length += 1;
    return item_index;
}

fn u32 intern_pool_put_new_assume_not_existent_assume_capacity(GenericInternPool* pool, Thread* thread, u32 item_index, const void* restrict const item_pointer, u32 item_size, Hash32 hash, const InternPoolInterface* restrict const interface)
{
    auto capacity = pool->capacity;
    assert(pool->length < capacity);
    assert(hash);
    auto pool_index = hash & (capacity - 1);
    auto result = ip_generic_put_new_at_assume_not_existent_assume_capacity(pool, thread, item_index, item_pointer, item_size, pool_index, interface);
    return result;
}

fn void ip_generic_ensure_capacity(GenericInternPool* pool, Thread* thread, u32 additional)
{
    auto current_length = pool->length;
    auto current_capacity = pool->capacity;
    auto half_capacity = current_capacity >> 1;
    auto destination_length = current_length + additional;

    if (destination_length > half_capacity)
    {
        auto new_capacity = cast(u32, u64, MAX(round_up_to_next_power_of_2(destination_length), intern_pool_min_capacity));
        auto* new_array = arena_allocate(thread->arena, u32, new_capacity);
        memset(new_array, 0, sizeof(u32) * new_capacity);

        auto old_capacity = current_capacity;

        pool->pointer = new_array;
        pool->length = 0;
        pool->capacity = new_capacity;

        if (old_capacity)
        {
            todo();
        }
    }
}

fn u32 ip_generic_put_new_assume_not_existent(GenericInternPool* pool, Thread* thread, u32 item_index, const void* item_pointer, u32 item_size, Hash32 hash, const InternPoolInterface* const restrict interface)
{
    ip_generic_ensure_capacity(pool, thread, 1);
    auto result = intern_pool_put_new_assume_not_existent_assume_capacity(pool, thread, item_index, item_pointer, item_size, hash, interface);
    return result;
}

fn GenericGetOrPut ip_generic_get_or_put(GenericInternPool* pool, Thread* thread, u32 item_index, const void* const restrict item_pointer, u32 item_size, Hash32 hash, const InternPoolInterface* const restrict interface)
{
    assert(hash);
    auto length = pool->length;
    auto capacity = pool->capacity;

    if (capacity)
    {
        auto maybe_slot = ip_generic_find_slot(pool, thread, item_index, hash, interface);
        if (maybe_slot != -1)
        {
            auto index = cast(u32, s64, maybe_slot);
            auto element = pool->pointer[index];
            u8 is_valid_or_existing = element != 0;
            if (!is_valid_or_existing)
            {
                element = ip_generic_put_new_at_assume_not_existent_assume_capacity(pool, thread, item_index, item_pointer, item_size, index, interface);
                assert(element != 0);
            }

            return (GenericGetOrPut) {
                .index = element,
                    .existing = is_valid_or_existing,
            };
        }
    }
        
    if (length < capacity)
    {
        todo();
    }
    else if (length == capacity)
    {
        auto index = ip_generic_put_new_assume_not_existent(pool, thread, item_index, item_pointer, item_size, hash, interface);
        return (GenericGetOrPut)
        {
            .index = index,
            .existing = 0,
        };
    }
    else
    {
        unreachable();
    }
}

// This assumes the indices are not equal
fn u8 node_equal(Thread* thread, NodeIndex a_index, NodeIndex b_index)
{
    u8 result = 0;
    auto a_hash = node_hash_index(thread, a_index);
    auto b_hash = node_hash_index(thread, b_index);
    auto* a = thread_node_get(thread, a_index);
    auto* b = thread_node_get(thread, b_index);
    assert(!index_equal(a_index, b_index));
    assert(a != b);

    if (((a->id == b->id) & (a_hash == b_hash)) & (a->input_count == b->input_count))
    {
        auto inputs_a = node_get_inputs(thread, a);
        auto inputs_b = node_get_inputs(thread, b);
        result = 1;

        for (u16 i = 0; i < a->input_count; i += 1)
        {
            if (!index_equal(inputs_a.pointer[i], inputs_b.pointer[i]))
            {
                result = 0;
                break;
            }
        }

        if (result)
        {
            todo();
            // switch (a->id)
            // {
            //     case IR_CONSTANT:
            //         todo();
            //         // result = index_equal(a->constant.type, b->constant.type);
            //         break;
            //     case IR_START:
            //         todo();
            //         // result = a->start.function == b->start.function;
            //         break;
            //     default:
            //         trap();
            // }
        }
    }

    return result;
}

fn u8 node_index_equal(Thread* thread, NodeIndex a, NodeIndex b)
{
    u8 result = 0;
    result = index_equal(a, b) || node_equal(thread, a, b);

    return result;
}

fn s64 ip_find_slot_node(GenericInternPool* generic_pool, Thread* thread, Hash32 hash, u32 raw_item_index, u32 saved_index, u32 slots_ahead)
{
    auto* pool = (InternPool(Node)*)generic_pool;
    assert(pool == &thread->interned.nodes);
    auto* ptr = pool->pointer;
    auto item_index = *(NodeIndex*)&raw_item_index;
    unused(hash);

    s64 result = -1;

    for (auto index = saved_index; index < saved_index + slots_ahead; index += 1)
    {
        auto typed_index = ptr[index];
        if (node_index_equal(thread, item_index, typed_index))
        {
            result = index;
            break;
        }
    }

    return result;
}

fn s64 ip_find_slot_register_mask(GenericInternPool* generic_pool, Thread* thread, Hash32 hash, u32 raw_item_index, u32 saved_index, u32 slots_ahead)
{
    auto* pool = (InternPool(RegisterMask)*)generic_pool;
    assert(pool == &thread->interned.register_masks);
    auto* ptr = pool->pointer;
    auto item_index = *(RegisterMaskIndex*)&raw_item_index;
    unused(hash);

    s64 result = -1;

    RegisterMask rm = *thread_register_mask_get(thread, item_index);

    for (auto index = saved_index; index < saved_index + slots_ahead; index += 1)
    {
        auto typed_index = ptr[index];
        static_assert(sizeof(RegisterMaskIndex) == sizeof(u32));
        if (index_equal(item_index, typed_index))
        {
            result = index;
            break;
        }

        auto register_mask = thread_register_mask_get(thread, typed_index);
        static_assert(sizeof(RegisterMask) == sizeof(u64));
        if (*(u64*)register_mask == *(u64*)&rm)
        {
            result = index;
            break;
        }
    }

    return result;
}

global const auto ip_interface_debug_type = (InternPoolInterface) {
    .add_to_buffer = &ip_DebugType_add_to_buffer,
    .find_slot = &ip_find_slot_debug_type,
};

global const auto ip_interface_node = (InternPoolInterface) {
    .find_slot = &ip_find_slot_node,
};

global const auto ip_interface_register_mask = (InternPoolInterface) {
    .find_slot = &ip_find_slot_register_mask,
};

#define declare_ip_functions(T, lower) \
fn Hash32 lower ## _hash_index(Thread* thread, T ## Index item_index); \
fn Hash32 lower ## _hash(Thread* thread, const T * const restrict item); \
\
may_be_unused fn T ## GetOrPut ip_ ## T ## _get_or_put(InternPool(T)* pool, Thread* thread, T ## Index item_index) \
{ \
    auto hash = lower ## _hash_index(thread, item_index); \
    auto* item = thread_ ## lower ## _get(thread, item_index); \
    static_assert(sizeof(item_index) == sizeof(u32));\
    auto raw_item_index =  *(u32*)&item_index;\
    auto result = ip_generic_get_or_put((GenericInternPool*)pool, thread, raw_item_index, (void*)item, sizeof(T), hash, &ip_interface_ ## lower); \
    return (T ## GetOrPut)\
    {\
        .index = *(T ## Index*)&result.index,\
        .existing = result.existing,\
    };\
}\
may_be_unused fn T ## GetOrPut ip_ ## T ## _get_or_put_new(InternPool(T)* pool, Thread* thread, const T* item) \
{ \
    auto hash = lower ## _hash(thread, item); \
    auto result = ip_generic_get_or_put((GenericInternPool*)pool, thread, 0, (void*)item, sizeof(T), hash, &ip_interface_ ## lower); \
    return (T ## GetOrPut)\
    {\
        .index = *(T ## Index*)&result.index,\
        .existing = result.existing,\
    };\
}\
may_be_unused fn T ## Index ip_ ## T ## _remove(InternPool(T)* pool, Thread* thread, T ## Index item_index)\
{\
    auto existing_capacity = pool->capacity;\
    auto* item = thread_ ## lower ## _get(thread, item_index);\
    auto hash = lower ## _hash(thread, item);\
    static_assert(sizeof(item_index) == sizeof(u32));\
    auto raw_item_index = *(u32*)&item_index;\
    auto maybe_slot = ip_generic_find_slot((GenericInternPool*)pool, thread, raw_item_index, hash, &ip_interface_ ## lower);\
    \
    if (maybe_slot != -1)\
    {\
        auto i = cast(u32, s64, maybe_slot);\
        auto* slot_pointer = &pool->pointer[i];\
        auto old_item_index = *slot_pointer;\
        assert(validi(old_item_index));\
        pool->length -= 1;\
        *slot_pointer = invalidi(T);\
        auto j = i;\
        \
        while (1)\
        {\
             j = (j + 1) & (existing_capacity - 1);\
\
             auto existing = pool->pointer[j];\
             if (!validi(existing))\
             {\
                 break;\
             }\
\
             auto existing_item_index = *(T ## Index*)&existing;\
             auto* existing_item = thread_ ## lower ## _get(thread, existing_item_index);\
             auto existing_item_hash = lower ## _hash(thread, existing_item);\
             auto k = existing_item_hash & (existing_capacity - 1);\
\
             if (i <= j)\
             {\
                 if ((i < k) & (k <= j))\
                 {\
                     continue;\
                 }\
             }\
             else\
             {\
                 if ((k <= j) | (i < k))\
                 {\
                     continue;\
                 }\
             }\
\
             pool->pointer[i] = pool->pointer[j];\
             pool->pointer[j] = invalidi(T);\
\
             i = j;\
        }\
\
        \
        return old_item_index;\
    }\
    else\
    {\
        todo();\
    }\
}

STRUCT(TypeGetOrPut)
{
    TypeIndex index;
    u8 existing;
};

// fn TypeGetOrPut intern_pool_get_or_put_new_type(Thread* thread, Type* type);

typedef NodeIndex NodeIdealize(Thread* thread, NodeIndex node_index);
typedef TypeIndex NodeComputeType(Thread* thread, NodeIndex node_index);
typedef Hash64 TypeGetHash(Thread* thread, Type* type);
typedef Hash64 NodeGetHash(Thread* thread, Node* node, NodeIndex node_index, Hash64 hash);

// fn TypeIndex thread_get_integer_type(Thread* thread, TypeInteger type_integer)
// {
//     Type type;
//     memset(&type, 0, sizeof(Type));
//     type.integer = type_integer;
//     type.id = TYPE_INTEGER;
//
//     auto result = intern_pool_get_or_put_new_type(thread, &type);
//     return result.index;
// }

fn NodeIndex peephole(Thread* thread, Function* function, NodeIndex node_index);
// fn NodeIndex constant_int_create_with_type(Thread* thread, Function* function, TypeIndex type_index)
// {
//     auto node_index = thread_node_add(thread, (NodeCreate){
//         .id = IR_CONSTANT,
//         .inputs = array_to_slice(((NodeIndex []) {
//             // function->start,
//         }))
//     });
//     auto* node = thread_node_get(thread, node_index);
//     unused(node);
//     unused(type_index);
//
//     todo();
//
//     // node->constant = (NodeConstant) {
//     //     .type = type_index,
//     // };
//     //
//     // // print("Creating constant integer node #{u32} with value: {u64:x}\n", node_index.index, thread_type_get(thread, type_index)->integer.constant);
//     //
//     // auto result = peephole(thread, function, node_index);
//     // return result;
// }

// fn NodeIndex constant_int_create(Thread* thread, Function* function, u64 value)
// {
//     auto type_index = thread_get_integer_type(thread, (TypeInteger){
//         .constant = value,
//         .bit_count = 0,
//         .is_constant = 1,
//         .is_signed = 0,
//     });
//
//     auto constant_int = constant_int_create_with_type(thread, function, type_index);
//     return constant_int;
// }

STRUCT(NodeVirtualTable)
{
    NodeComputeType* const compute_type;
    NodeIdealize* const idealize;
    NodeGetHash* const get_hash;
};

STRUCT(TypeVirtualTable)
{
    TypeGetHash* const get_hash;
};
fn Hash64 hash_type(Thread* thread, Type* type);

// fn NodeIndex idealize_null(Thread* thread, NodeIndex node_index)
// {
//     unused(thread);
//     unused(node_index);
//     return invalidi(Node);
// }

// fn TypeIndex compute_type_constant(Thread* thread, NodeIndex node_index)
// {
//     auto* node = thread_node_get(thread, node_index);
//     assert(node->id == IR_CONSTANT);
//     todo();
//     // return node->constant.type;
// }

fn Hash64 type_get_hash_default(Thread* thread, Type* type)
{
    unused(thread);
    assert(!type->hash);
    Hash64 hash = fnv_offset;

    // u32 i = 0;
    for (auto* it = (u8*)type; it < (u8*)(type + 1); it += 1)
    {
        hash = hash_byte(hash, *it);
        if (type->id == TYPE_INTEGER)
        {
            // print("Byte [{u32}] = 0x{u32:x}\n", i, (u32)*it);
            // i += 1;
        }
    }

    return hash;
}

fn Hash64 type_get_hash_tuple(Thread* thread, Type* type)
{
    Hash64 hash = fnv_offset;
    for (u64 i = 0; i < type->tuple.types.length; i += 1)
    {
        auto* tuple_type = thread_type_get(thread,type->tuple.types.pointer[i]); 
        auto type_hash = hash_type(thread, tuple_type);
        for (u8* it = (u8*)&type_hash; it < (u8*)(&type_hash + 1); it += 1)
        {
            hash = hash_byte(hash, *it);
        }
    }

    return hash;
}

// fn void intern_pool_ensure_capacity(InternPool(T)* pool, Thread* thread, u32 additional) \
// {\
//     auto current_capacity = pool->capacity; \
//     auto current_length = pool->length; \
//     assert(current_capacity % 2 == 0); \
//     auto half_capacity = current_capacity >> 1; \
//     auto destination_length = current_length + additional; \
// \
//     if (destination_length > half_capacity) \
//     {\
//         auto new_capacity = cast(u32, u64, MAX(round_up_to_next_power_of_2(destination_length), 32)); \
//         auto* new_array = arena_allocate(thread->arena, u32, new_capacity); \
//         memset(new_array, 0, sizeof(u32) * new_capacity); \
//         \
//         auto* old_pointer = pool->pointer;\
//         auto old_capacity = current_capacity;\
//         auto old_length = current_length;\
//         \
//         pool->length = 0;
//         pool->pointer = new_array;
//         pool->capacity = new_capacity;
//
//         u8* buffer;
//         u64 stride;
//         switch (kind)
//         {
//         case INTERN_POOL_KIND_TYPE:
//             buffer = (u8*)thread->buffer.types.pointer;
//             stride = sizeof(Type);
//             assert(pool == &thread->interned.types);
//             break;
//         case INTERN_POOL_KIND_NODE:
//             buffer = (u8*)thread->buffer.nodes.pointer;
//             stride = sizeof(Node);
//             assert(pool == &thread->interned.nodes);
//             break;
//         }
//
//         for (u32 i = 0; i < old_capacity; i += 1)
//         {
//            auto key = old_pointer[i];
//            if (key)
//            {
//                auto hash = *(Hash64*)(buffer + (stride * (key - 1)));
//                assert(hash);
//                switch (kind)
//                {
//                case INTERN_POOL_KIND_TYPE:
//                    {
//                        auto type_index = *(TypeIndex*)&key;
//                        auto* type = thread_type_get(thread, type_index);
//                        assert(type->hash == hash);
//                    } break;
//                case INTERN_POOL_KIND_NODE:
//                    {
//                        auto node_index = *(NodeIndex*)&key;
//                        auto* node = thread_node_get(thread, node_index);
//                        todo();
//                        // assert(node->hash == hash);
//                        // intern_pool_put_node_assume_not_existent_assume_capacity(thread, hash, node_index);
//                    } break;
//                }
//
//            }
//         }
//
//         assert(old_length == pool->length);
//         assert(pool->capacity == new_capacity);
//
//         for (u32 i = 0; i < old_capacity; i += 1)
//         {
//             auto key = old_pointer[i];
//             if (key)
//             {
//                 auto hash = *(Hash64*)(buffer + (stride * (key - 1)));
//                 assert(hash);
//                 switch (kind)
//                 {
//                 case INTERN_POOL_KIND_TYPE:
//                     {
//                         auto type_index = *(TypeIndex*)&key;
//                         unused(type_index);
//                         trap();
//                     } break;
//                 case INTERN_POOL_KIND_NODE:
//                     {
//                         auto node_index = *(NodeIndex*)&key;
//                         auto* node = thread_node_get(thread, node_index);
//                         todo();
//                         // assert(node->hash == hash);
//                         // auto result = intern_pool_get_node(thread, node_index, hash);
//                         // assert(validi(node_index));
//                         // assert(index_equal(node_index, result));
//                     } break;
//                 }
//             }
//         }
//     }
// }
// fn u8 node_is_projection(Node* n)
// {
//     return (n->id == IR_CONTROL_PROJECTION) | (n->id == IR_PROJECTION);
// }

// fn NodeIndex projection_get_control(Thread* thread, Node* node)
// {
//     assert(node_is_projection(node));
//     auto node_index = node_input_get(thread, node, 0);
//     return node_index;
// }

// fn s32 projection_get_index(Node* node)
// {
//     assert(node_is_projection(node));
//
//     switch (node->id)
//     {
//         case IR_CONTROL_PROJECTION:
//             return node->control_projection.projection.index;
//         case IR_PROJECTION:
//             return node->projection.index;
//         default:
//             trap();
//     }
// }

// fn TypeIndex compute_type_projection(Thread* thread, NodeIndex node_index)
// {
//     auto* node = thread_node_get(thread, node_index);
//     assert(node_is_projection(node));
//     auto control_node_index = projection_get_control(thread, node);
//     auto* control_node = thread_node_get(thread, control_node_index);
//     auto* control_type = thread_type_get(thread, control_node->type);
//
//     if (control_type->id == TYPE_TUPLE)
//     {
//         auto index = projection_get_index(node);
//         auto type_index = control_type->tuple.types.pointer[index];
//         return type_index;
//     }
//     else
//     {
//         return thread->types.bottom;
//     }
// }

// fn NodeIndex idealize_control_projection(Thread* thread, NodeIndex node_index)
// {
//     auto* node = thread_node_get(thread, node_index);
//     assert(node->id == IR_CONTROL_PROJECTION);
//     auto control_node_index = projection_get_control(thread, node);
//     auto* control_node = thread_node_get(thread, control_node_index);
//     auto* control_type = thread_type_get(thread, control_node->type);
//     auto index = node->control_projection.projection.index;
//
//     if (control_type->id == TYPE_TUPLE)
//     {
//         if (index_equal(control_type->tuple.types.pointer[index], thread->types.dead_control))
//         {
//             trap();
//         }
//         if (control_node->id == IR_IF)
//         {
//             trap();
//         }
//     }
//
//     if (control_node->id == IR_IF)
//     {
//         trap();
//     }
//
//     return invalidi(Node);
// }

fn NodeIndex return_get_control(Thread* thread, Node* node)
{
    return node_get_inputs(thread, node).pointer[0];
}

fn NodeIndex return_get_value(Thread* thread, Node* node)
{
    return node_get_inputs(thread, node).pointer[1];
}

// fn TypeIndex intern_pool_put_new_type_at_assume_not_existent_assume_capacity(Thread* thread, Type* type, u32 index)
// {
//     auto* result = vb_add(&thread->buffer.types, 1);
//     auto buffer_index = cast(u32, s64, result - thread->buffer.types.pointer);
//     auto type_index = Index(Type, buffer_index);
//     *result = *type;
//
//     u32 raw_type = *(u32*)&type_index;
//     thread->interned.types.pointer[index] = raw_type;
//     assert(raw_type);
//     thread->interned.types.length += 1;
//
//     return type_index;
// }

// fn TypeIndex intern_pool_put_new_type_assume_not_existent_assume_capacity(Thread* thread, Type* type)
// {
//     assert(thread->interned.types.length < thread->interned.types.capacity);
//     Hash64 hash = type->hash;
//     assert(hash);
//     auto index = cast(u32, u64, hash & (thread->interned.types.capacity - 1));
//
//     return intern_pool_put_new_type_at_assume_not_existent_assume_capacity(thread, type, index);
// }
//
// typedef enum InternPoolKind
// {
//     INTERN_POOL_KIND_TYPE,
//     INTERN_POOL_KIND_NODE,
// } InternPoolKind;

// [[gnu::hot]] fn s64 intern_pool_find_node_slot(Thread* thread, u32 original_index, NodeIndex node_index)
// {
//     assert(validi(node_index));
//     auto it_index = original_index;
//     auto existing_capacity = thread->interned.nodes.capacity;
//     s64 result = -1;
//     // auto* node = thread_node_get(thread, node_index);
//
//     for (u32 i = 0; i < existing_capacity; i += 1)
//     {
//         auto index = it_index & (existing_capacity - 1);
//         u32 key = thread->interned.nodes.pointer[index];
//
//         if (key == 0)
//         {
//             assert(thread->interned.nodes.length < thread->interned.nodes.capacity);
//             result = index;
//             break;
//         }
//         else
//         {
//             NodeIndex existing_node_index = *(NodeIndex*)&key;
//             // Exhaustive comparation, shortcircuit when possible
//             if (node_index_equal(thread, existing_node_index, node_index))
//             {
//                 result = index;
//                 break;
//             }
//         }
//
//         it_index += 1;
//     }
//
//     return result;
// }

// fn NodeIndex intern_pool_get_node(Thread* thread, NodeIndex key, Hash64 hash)
// {
//     auto original_index = cast(u32, u64, hash & (thread->interned.nodes.capacity - 1));
//     auto maybe_slot = intern_pool_find_node_slot(thread, original_index, key);
//     auto node_index = invalidi(Node);
//
//     if (maybe_slot != -1)
//     {
//         auto slot = cast(u32, s64, maybe_slot);
//         auto* pointer_to_slot = &thread->interned.nodes.pointer[slot];
//         node_index = *(NodeIndex*)pointer_to_slot;
//     }
//
//     return node_index;
// }

// fn NodeIndex intern_pool_put_node_at_assume_not_existent_assume_capacity(Thread* thread, NodeIndex node, u32 index)
// {
//     u32 raw_node = *(u32*)&node;
//     assert(raw_node);
//     thread->interned.nodes.pointer[index] = raw_node;
//     thread->interned.nodes.length += 1;
//
//     return node;
// }

// fn NodeIndex intern_pool_put_node_assume_not_existent_assume_capacity(Thread* thread, Hash64 hash, NodeIndex node)
// {
//     auto capacity = thread->interned.nodes.capacity;
//     assert(thread->interned.nodes.length < capacity);
//     auto original_index = cast(u32, u64, hash & (capacity - 1));
//
//     auto slot = intern_pool_find_node_slot(thread, original_index, node);
//     if (slot == -1)
//     {
//         fail();
//     }
//     auto index = (u32)slot;
//
//     return intern_pool_put_node_at_assume_not_existent_assume_capacity(thread, node, index);
// }

// fn void intern_pool_ensure_capacity(InternPool* pool, Thread* thread, u32 additional, InternPoolKind kind)
// {
//     auto current_capacity = pool->capacity;
//     auto current_length = pool->length;
//     auto half_capacity = current_capacity >> 1;
//     auto destination_length = current_length + additional;
//
//     if (destination_length > half_capacity)
//     {
//         auto new_capacity = cast(u32, u64, MAX(round_up_to_next_power_of_2(destination_length), 32));
//         auto* new_array = arena_allocate(thread->arena, u32, new_capacity);
//         memset(new_array, 0, sizeof(u32) * new_capacity);
//
//         auto* old_pointer = pool->pointer;
//         auto old_capacity = current_capacity;
//         auto old_length = current_length;
//
//         pool->length = 0;
//         pool->pointer = new_array;
//         pool->capacity = new_capacity;
//
//         u8* buffer;
//         u64 stride;
//         switch (kind)
//         {
//         case INTERN_POOL_KIND_TYPE:
//             buffer = (u8*)thread->buffer.types.pointer;
//             stride = sizeof(Type);
//             assert(pool == &thread->interned.types);
//             break;
//         case INTERN_POOL_KIND_NODE:
//             buffer = (u8*)thread->buffer.nodes.pointer;
//             stride = sizeof(Node);
//             assert(pool == &thread->interned.nodes);
//             break;
//         }
//
//         for (u32 i = 0; i < old_capacity; i += 1)
//         {
//            auto key = old_pointer[i];
//            if (key)
//            {
//                auto hash = *(Hash64*)(buffer + (stride * (key - 1)));
//                assert(hash);
//                switch (kind)
//                {
//                case INTERN_POOL_KIND_TYPE:
//                    {
//                        auto type_index = *(TypeIndex*)&key;
//                        auto* type = thread_type_get(thread, type_index);
//                        assert(type->hash == hash);
//                    } break;
//                case INTERN_POOL_KIND_NODE:
//                    {
//                        auto node_index = *(NodeIndex*)&key;
//                        auto* node = thread_node_get(thread, node_index);
//                        todo();
//                        // assert(node->hash == hash);
//                        // intern_pool_put_node_assume_not_existent_assume_capacity(thread, hash, node_index);
//                    } break;
//                }
//
//            }
//         }
//
//         assert(old_length == pool->length);
//         assert(pool->capacity == new_capacity);
//
//         for (u32 i = 0; i < old_capacity; i += 1)
//         {
//             auto key = old_pointer[i];
//             if (key)
//             {
//                 auto hash = *(Hash64*)(buffer + (stride * (key - 1)));
//                 assert(hash);
//                 switch (kind)
//                 {
//                 case INTERN_POOL_KIND_TYPE:
//                     {
//                         auto type_index = *(TypeIndex*)&key;
//                         unused(type_index);
//                         trap();
//                     } break;
//                 case INTERN_POOL_KIND_NODE:
//                     {
//                         auto node_index = *(NodeIndex*)&key;
//                         auto* node = thread_node_get(thread, node_index);
//                         todo();
//                         // assert(node->hash == hash);
//                         // auto result = intern_pool_get_node(thread, node_index, hash);
//                         // assert(validi(node_index));
//                         // assert(index_equal(node_index, result));
//                     } break;
//                 }
//             }
//         }
//     }
// }
//
// fn TypeIndex intern_pool_put_new_type_assume_not_existent(Thread* thread, Type* type)
// {
//     intern_pool_ensure_capacity(&thread->interned.types, thread, 1, INTERN_POOL_KIND_TYPE);
//     return intern_pool_put_new_type_assume_not_existent_assume_capacity(thread, type);
// }
//
// fn s64 intern_pool_find_type_slot(Thread* thread, u32 original_index, Type* type)
// {
//     auto it_index = original_index;
//     auto existing_capacity = thread->interned.types.capacity;
//     s64 result = -1;
//
//     for (u32 i = 0; i < existing_capacity; i += 1)
//     {
//         auto index = it_index & (existing_capacity - 1);
//         u32 key = thread->interned.types.pointer[index];
//
//         // Not set
//         if (key == 0)
//         {
//             result = index;
//             break;
//         }
//         else
//         {
//             TypeIndex existing_type_index = *(TypeIndex*)&key;
//             Type* existing_type = thread_type_get(thread, existing_type_index);
//             if (type_equal(existing_type, type))
//             {
//                 result = index;
//                 break;
//             }
//         }
//
//         it_index += 1;
//     }
//
//     return result;
// }


// fn s64 intern_pool_find_debug_type_slot(Thread* thread, const DebugType* type, Hash32 hash)
// {
//     auto it_index = original_index;
//     auto existing_capacity = thread->interned.types.capacity;
//     s64 result = -1;
//
//     for (u32 i = 0; i < existing_capacity; i += 1)
//     {
//         auto index = it_index & (existing_capacity - 1);
//         u32 key = thread->interned.types.pointer[index];
//
//         // Not set
//         if (key == 0)
//         {
//             result = index;
//             break;
//         }
//         else
//         {
//             auto existing_type_index = *(DebugTypeIndex*)&key;
//             DebugType* existing_type = thread_debug_type_get(thread, existing_type_index);
//             auto existing_hash = hash_debug_type(existing_type);
//             trap();
//             // if (type_equal(existing_type, type))
//             // {
//             //     result = index;
//             //     break;
//             // }
//         }
//
//         it_index += 1;
//     }
//
//     return result;
// }

// fn DebugTypeIndex intern_pool_put_new_debug_type_at_assume_not_existent_assume_capacity(Thread* thread, const DebugType* type, u32 index)
// {
//     auto* result = vb_add(&thread->buffer.debug_types, 1);
//     auto buffer_index = cast(u32, s64, result - thread->buffer.debug_types.pointer);
//     auto type_index = Index(DebugType, buffer_index);
//     *result = *type;
//
//     u32 raw_type = *(u32*)&type_index;
//     thread->interned.types.pointer[index] = raw_type;
//     assert(raw_type);
//     thread->interned.types.length += 1;
//
//     return type_index;
// }

// fn DebugTypeIndex intern_pool_put_new_debug_type_assume_not_existent_assume_capacity(Thread* thread, const DebugType* type, Hash32 hash)
// {
//     assert(thread->interned.types.length < thread->interned.types.capacity);
//     assert(hash);
//     auto index = hash & (thread->interned.types.capacity - 1);
//
//     return intern_pool_put_new_debug_type_at_assume_not_existent_assume_capacity(thread, type, index);
// }

// fn DebugTypeIndex intern_pool_put_new_debug_type_assume_not_existent(Thread* thread, const DebugType* type, Hash32 hash)
// {
//     intern_pool_ensure_capacity(&thread->interned.types, thread, 1, INTERN_POOL_KIND_TYPE);
//     return intern_pool_put_new_debug_type_assume_not_existent_assume_capacity(thread, type, hash);
// }

// fn DebugTypeGetOrPut intern_pool_get_or_put_new_debug_type(Thread* thread, const DebugType* type)
// {
//     auto existing_capacity = thread->interned.types.capacity;
//     auto hash = hash_debug_type(type);
//     auto original_index = cast(u32, u64, hash & (existing_capacity - 1));
//     
//     auto maybe_slot = intern_pool_find_debug_type_slot(thread, original_index, type);
//     if (maybe_slot != -1)
//     {
//         auto index = cast(u32, s64, maybe_slot);
//         auto type_index = *(DebugTypeIndex*)&thread->interned.types.pointer[index];
//         u8 existing = validi(type_index);
//         if (!existing)
//         {
//             type_index = intern_pool_put_new_debug_type_at_assume_not_existent_assume_capacity(thread, type, index);
//         }
//
//         return (DebugTypeGetOrPut) {
//             .index = type_index,
//             .existing = existing,
//         };
//     }
//     else
//     {
//         if (thread->interned.types.length < existing_capacity)
//         {
//             trap();
//         }
//         else if (thread->interned.types.length == existing_capacity)
//         {
//             auto result = intern_pool_put_new_debug_type_assume_not_existent(thread, type, hash);
//             return (DebugTypeGetOrPut) {
//                 .index = result,
//                 .existing = 0,
//             };
//         }
//         else
//         {
//             trap();
//         }
//     }
// }

// fn TypeGetOrPut intern_pool_get_or_put_new_type(Thread* thread, Type* type)
// {
//     auto existing_capacity = thread->interned.types.capacity;
//     auto hash = hash_type(thread, type);
//     auto original_index = cast(u32, u64, hash & (existing_capacity - 1));
//     
//     auto maybe_slot = intern_pool_find_type_slot(thread, original_index, type);
//     if (maybe_slot != -1)
//     {
//         auto index = cast(u32, s64, maybe_slot);
//         TypeIndex type_index = *(TypeIndex*)&thread->interned.types.pointer[index];
//         u8 existing = validi(type_index);
//         if (!existing)
//         {
//             type_index = intern_pool_put_new_type_at_assume_not_existent_assume_capacity(thread, type, index);
//         }
//
//         return (TypeGetOrPut) {
//             .index = type_index,
//             .existing = existing,
//         };
//     }
//     else
//     {
//         if (thread->interned.types.length < existing_capacity)
//         {
//             trap();
//         }
//         else if (thread->interned.types.length == existing_capacity)
//         {
//             auto result = intern_pool_put_new_type_assume_not_existent(thread, type);
//             return (TypeGetOrPut) {
//                 .index = result,
//                 .existing = 0,
//             };
//         }
//         else
//         {
//             trap();
//         }
//     }
// }


// fn TypeGetOrPut type_make_tuple(Thread* thread, Slice(TypeIndex) types)
// {
//     Type type;
//     memset(&type, 0, sizeof(Type));
//     type.tuple = (TypeTuple){
//         .types = types,
//     };
//     type.id = TYPE_TUPLE;
//     auto result = intern_pool_get_or_put_new_type(thread, &type);
//     return result;
// }

// fn TypeIndex type_make_tuple_allocate(Thread* thread, Slice(TypeIndex) types)
// {
//     auto gop = type_make_tuple(thread, types);
//     // Need to reallocate the type array
//     if (!gop.existing)
//     {
//         auto* type = thread_type_get(thread, gop.index);
//         assert(type->tuple.types.pointer == types.pointer);
//         assert(type->tuple.types.length == types.length);
//         type->tuple.types = arena_allocate_slice(thread->arena, TypeIndex, types.length);
//         memcpy(type->tuple.types.pointer, types.pointer, sizeof(TypeIndex) * types.length);
//     }
//
//     return gop.index;
// }

fn TypeIndex compute_type_return(Thread* thread, NodeIndex node_index)
{
    Node* node = thread_node_get(thread, node_index);
    auto control_type = thread_node_get(thread, return_get_control(thread, node))->type;
    unused(control_type);
    auto return_type = thread_node_get(thread, return_get_value(thread, node))->type;
    unused(return_type);
    todo();
    // Slice(TypeIndex) types = array_to_slice(((TypeIndex[]) {
    //     control_type,
    //     return_type,
    // }));
    // auto result = type_make_tuple_allocate(thread, types);
    // return result;
}

fn NodeIndex idealize_return(Thread* thread, NodeIndex node_index)
{
    auto* node = thread_node_get(thread, node_index);
    auto control_node_index = return_get_control(thread, node);
    auto* control_node = thread_node_get(thread, control_node_index);
    unused(control_node);
    // if (index_equal(control_node->type, thread->types.dead_control))
    // {
    //     return control_node_index;
    // }
    // else
    // {
    //     return invalidi(Node);
    // }

    todo();
}

// fn TypeIndex compute_type_dead_control(Thread* thread, NodeIndex node_index)
// {
//     unused(node_index);
//     return thread->types.dead_control;
// }

// fn TypeIndex compute_type_bottom(Thread* thread, NodeIndex node_index)
// {
//     unused(node_index);
//     return thread->types.bottom;
// }

// fn NodeIndex idealize_stop(Thread* thread, NodeIndex node_index)
// {
//     auto* node = thread_node_get(thread, node_index);
//     auto original_input_count = node->input_count;
//     for (u16 i = 0; i < node->input_count; i += 1)
//     {
//         auto input_node_index = node_input_get(thread, node, i);
//         auto* input_node = thread_node_get(thread, input_node_index);
//         if (index_equal(input_node->type, thread->types.dead_control))
//         {
//             trap();
//         }
//     }
//
//     if (node->input_count != original_input_count)
//     {
//         return node_index;
//     }
//     else
//     {
//         return invalidi(Node);
//     }
// }

// fn TypeIndex compute_type_start(Thread* thread, NodeIndex node_index)
// {
//     auto* node = thread_node_get(thread, node_index);
//     unused(node);
//     todo();
//     // return node->start.arguments;
// }

// fn u8 type_is_constant(Type* type)
// {
//     switch (type->id)
//     {
//         case TYPE_INTEGER:
//             return type->integer.is_constant;
//         default:
//             return 0;
//     }
// }

// fn u8 type_is_simple(Type* type)
// {
//     return type->id <= TYPE_DEAD_CONTROL;
// }

// fn TypeIndex type_meet(Thread* thread, TypeIndex a, TypeIndex b)
// {
//     TypeIndex result = invalidi(Type);
//     if (index_equal(a, b))
//     {
//         result = a;
//     }
//     else
//     {
//         Type* a_type = thread_type_get(thread, a);
//         Type* b_type = thread_type_get(thread, b);
//         TypeIndex left = invalidi(Type);
//         TypeIndex right = invalidi(Type);
//
//         assert(a_type != b_type);
//         if (a_type->id == b_type->id)
//         {
//             left = a;
//             right = b;
//         }
//         else if (type_is_simple(a_type))
//         {
//             left = a;
//             right = b;
//         }
//         else if (type_is_simple(b_type))
//         {
//             trap();
//         }
//         else
//         {
//             result = thread->types.bottom;
//         }
//
//         assert(!!validi(left) == !!validi(right));
//         assert((validi(left) & validi(right)) | (validi(result)));
//
//         if (validi(left))
//         {
//             assert(!validi(result));
//             auto* left_type = thread_type_get(thread, left);
//             auto* right_type = thread_type_get(thread, right);
//
//             switch (left_type->id)
//             {
//                 case TYPE_INTEGER:
//                     {
//                             // auto integer_bot = thread->types.integer.bottom;
//                             // auto integer_top = thread->types.integer.top;
//                             // if (index_equal(left, integer_bot))
//                             // {
//                             //     result = left; 
//                             // }
//                             // else if (index_equal(right, integer_bot))
//                             // {
//                             //     result = right; 
//                             // }
//                             // else if (index_equal(right, integer_top))
//                             // {
//                             //     result = left; 
//                             // }
//                             // else if (index_equal(left, integer_top))
//                             // {
//                             //     result = right; 
//                             // }
//                             // else
//                             // {
//                             //     result = integer_bot;
//                             // }
//                             if (left_type->integer.bit_count == right_type->integer.bit_count)
//                             {
//                                 todo();
//                             }
//                             else
//                             {
//                                 if ((!left_type->integer.is_constant & !!left_type->integer.bit_count) & (right_type->integer.is_constant & !right_type->integer.bit_count))
//                                 {
//                                     result = left;
//                                 }
//                                 else if ((left_type->integer.is_constant & !left_type->integer.bit_count) & (!right_type->integer.is_constant & !!right_type->integer.bit_count))
//                                 {
//                                     trap();
//                                 }
//                             }
//                     } break;
//                 case TYPE_BOTTOM:
//                     {
//                         assert(type_is_simple(left_type));
//                         if ((left_type->id == TYPE_BOTTOM) | (right_type->id == TYPE_TOP))
//                         {
//                             result = left;
//                         }
//                         else if ((left_type->id == TYPE_TOP) | (right_type->id == TYPE_BOTTOM))
//                         {
//                             result = right;
//                         }
//                         else if (!type_is_simple(right_type))
//                         {
//                             result = thread->types.bottom;
//                         }
//                         else if (left_type->id == TYPE_LIVE_CONTROL)
//                         {
//                             result = thread->types.live_control;
//                         }
//                         else
//                         {
//                             result = thread->types.dead_control;
//                         }
//                     } break;
//                 default:
//                     trap();
//             }
//         }
//     }
//
//     assert(validi(result));
//
//     return result;
// }

// fn u8 type_is_a(Thread* thread, TypeIndex a, TypeIndex b)
// {
//     auto m = type_meet(thread, a, b);
//     return index_equal(m, b);
// }

// fn TypeIndex compute_type_integer_binary(Thread* thread, NodeIndex node_index)
// {
//     auto* node = thread_node_get(thread, node_index);
//     auto inputs = node_get_inputs(thread, node);
//     auto* left = thread_node_get(thread, inputs.pointer[1]);
//     auto* right = thread_node_get(thread, inputs.pointer[2]);
//     assert(!node_is_dead(left));
//     assert(!node_is_dead(right));
//     auto* left_type = thread_type_get(thread, left->type);
//     auto* right_type = thread_type_get(thread, right->type);
//
//     if (((left_type->id == TYPE_INTEGER) & (right_type->id == TYPE_INTEGER)) & (type_is_constant(left_type) & type_is_constant(right_type)))
//     {
//         auto left_value = left_type->integer.constant;
//         auto right_value = right_type->integer.constant;
//         assert(left_type->integer.bit_count == 0);
//         assert(right_type->integer.bit_count == 0);
//         assert(!left_type->integer.is_signed);
//         assert(!right_type->integer.is_signed);
//
//         u64 result;
//         TypeInteger type_integer = left_type->integer;
//
//         switch (node->id)
//         {
//             case IR_INTEGER_ADD:
//                 result = left_value + right_value;
//                 break;
//             case IR_INTEGER_SUBSTRACT:
//                 result = left_value - right_value;
//                 break;
//             case IR_INTEGER_MULTIPLY:
//                 result = left_value * right_value;
//                 break;
//             case IR_INTEGER_SIGNED_DIVIDE:
//                 result = left_value * right_value;
//                 break;
//             case IR_INTEGER_AND:
//                 result = left_value & right_value;
//                 break;
//             case IR_INTEGER_OR:
//                 result = left_value | right_value;
//                 break;
//             case IR_INTEGER_XOR:
//                 result = left_value ^ right_value;
//                 break;
//             case IR_INTEGER_SIGNED_SHIFT_LEFT:
//                 result = left_value << right_value;
//                 break;
//             case IR_INTEGER_SIGNED_SHIFT_RIGHT:
//                 result = left_value >> right_value;
//                 break;
//             default:
//                 trap();
//         }
//
//         type_integer.constant = result;
//
//         auto new_type = thread_get_integer_type(thread, type_integer);
//         return new_type;
//     }
//     else
//     {
//         auto result = type_meet(thread, left->type, right->type);
//         return result;
//     }
// }

global const TypeVirtualTable type_functions[TYPE_COUNT] = {
    [TYPE_BOTTOM] = { .get_hash = &type_get_hash_default },
    [TYPE_TOP] = { .get_hash = &type_get_hash_default },
    [TYPE_LIVE_CONTROL] = { .get_hash = &type_get_hash_default },
    [TYPE_DEAD_CONTROL] = { .get_hash = &type_get_hash_default },
    [TYPE_INTEGER] = { .get_hash = &type_get_hash_default },
    [TYPE_TUPLE] = { .get_hash = &type_get_hash_tuple },
};

global const NodeVirtualTable node_functions[NODE_COUNT] = {
    // [NODE_START] = {
    //     .compute_type = &compute_type_start,
    //     .idealize = &idealize_null,
    //     .get_hash = &node_get_hash_default,
    // },
    // [NODE_STOP] = {
    //     .compute_type = &compute_type_bottom,
    //     .idealize = &idealize_stop,
    //     .get_hash = &node_get_hash_default,
    // },
    // [NODE_CONTROL_PROJECTION] = {
    //     .compute_type = &compute_type_projection,
    //     .idealize = &idealize_control_projection,
    //     .get_hash = &node_get_hash_control_projection,
    // },
    // [NODE_DEAD_CONTROL] = {
    //     .compute_type = &compute_type_dead_control,
    //     .idealize = &idealize_null,
    //     .get_hash = &node_get_hash_default,
    // },
    [IR_RETURN] = {
        .compute_type = &compute_type_return,
        .idealize = &idealize_return,
        .get_hash = &node_get_hash_default,
    },
    // [NODE_PROJECTION] = {
    //     .compute_type = &compute_type_projection,
    //     .idealize = &idealize_null,
    //     .get_hash = &node_get_hash_projection,
    // },
    // [NODE_SCOPE] = {
    //     .compute_type = &compute_type_bottom,
    //     .idealize = &idealize_null,
    //     .get_hash = &node_get_hash_scope,
    // },

    // Integer operations
    // [NODE_INTEGER_ADD] = {
    //     .compute_type = &compute_type_integer_binary,
    // },
    // [NODE_INTEGER_SUBSTRACT] = {
    //     .compute_type = &compute_type_integer_binary,
    //     .idealize = &node_idealize_substract,
    //     .get_hash = &node_get_hash_default,
    // },
    // [NODE_INTEGER_SIGNED_DIVIDE] = {
    //     .compute_type = &compute_type_integer_binary,
    // },
    // [NODE_INTEGER_MULTIPLY] = {
    //     .compute_type = &compute_type_integer_binary,
    // },
    // [NODE_INTEGER_AND] = {
    //     .compute_type = &compute_type_integer_binary,
    // },
    // [NODE_INTEGER_OR] = {
    //     .compute_type = &compute_type_integer_binary,
    // },
    // [NODE_INTEGER_XOR] = {
    //     .compute_type = &compute_type_integer_binary,
    // },
    // [NODE_INTEGER_SIGNED_SHIFT_LEFT] = {
    //     .compute_type = &compute_type_integer_binary,
    // },
    // [NODE_INTEGER_SIGNED_SHIFT_RIGHT] = {
    //     .compute_type = &compute_type_integer_binary,
    // },
    //
    // [NODE_INTEGER_COMPARE_EQUAL] = {
    //     .compute_type = &compute_type_integer_binary,
    //     .idealize = &node_idealize_compare,
    //     .get_hash = &node_get_hash_default,
    // },
    // [NODE_INTEGER_COMPARE_NOT_EQUAL] = {
    //     .compute_type = &compute_type_integer_binary,
    //     .idealize = &node_idealize_compare,
    //     .get_hash = &node_get_hash_default,
    // },
    //
    // // Constant
    // [NODE_CONSTANT] = {
    //     .compute_type = &compute_type_constant,
    //     .idealize = &idealize_null,
    //     .get_hash = &node_get_hash_constant,
    // },
};

may_be_unused fn String type_id_to_string(Type* type)
{
    switch (type->id)
    {
        case_to_name(TYPE_, BOTTOM);
        case_to_name(TYPE_, TOP);
        case_to_name(TYPE_, LIVE_CONTROL);
        case_to_name(TYPE_, DEAD_CONTROL);
        case_to_name(TYPE_, INTEGER);
        case_to_name(TYPE_, TUPLE);
        case_to_name(TYPE_, COUNT);
    }
}


fn Hash64 hash_type(Thread* thread, Type* type)
{
    Hash64 hash = type->hash;

    if (!hash)
    {
        hash = type_functions[type->id].get_hash(thread, type);
        // print("Hashing type id {s}: {u64:x}\n", type_id_to_string(type), hash);
    }

    assert(hash != 0);
    type->hash = hash;

    return hash;
}

// fn NodeIndex intern_pool_put_node_assume_not_existent(Thread* thread, Hash64 hash, NodeIndex node)
// {
//     intern_pool_ensure_capacity(&thread->interned.nodes, thread, 1, INTERN_POOL_KIND_NODE);
//     return intern_pool_put_node_assume_not_existent_assume_capacity(thread, hash, node);
// }

// fn Hash64 hash_node(Thread* thread, Node* node, NodeIndex node_index)
// {
//     auto hash = node->hash;
//     if (!hash)
//     {
//         hash = fnv_offset;
//         hash = node_functions[node->id].get_hash(thread, node, node_index, hash);
//         // print("[HASH #{u32}] Received hash from callback: {u64:x}\n", node_index.index, hash);
//
//         hash = hash_byte(hash, node->id);
//
//         auto inputs = node_get_inputs(thread, node);
//         for (u32 i = 0; i < inputs.length; i += 1)
//         {
//             auto input_index = inputs.pointer[i];
//             if (validi(input_index))
//             {
//                 for (u8* it = (u8*)&input_index; it < (u8*)(&input_index + 1); it += 1)
//                 {
//                     hash = hash_byte(hash, *it);
//                 }
//             }
//         }
//
//         // print("[HASH] Node #{u32}, {s}: {u64:x}\n", node_index.index, node_id_to_string(node), hash);
//
//         node->hash = hash;
//     }
//
//     assert(hash);
//
//     return hash;
// }

// fn NodeGetOrPut intern_pool_get_or_put_node(Thread* thread, NodeIndex node_index)
// {
//     assert(thread->interned.nodes.length <= thread->interned.nodes.capacity);
//     auto existing_capacity = thread->interned.nodes.capacity;
//     auto* node = &thread->buffer.nodes.pointer[geti(node_index)];
//     auto hash = hash_node(thread, node, node_index);
//     auto original_index = hash & (existing_capacity - 1);
//     
//     auto slot = intern_pool_find_node_slot(thread, original_index, node_index);
//     if (slot != -1)
//     {
//         u32 index = slot;
//         auto* existing_ptr = &thread->interned.nodes.pointer[index];
//         NodeIndex existing_value = *(NodeIndex*)existing_ptr;
//         u8 existing = validi(existing_value);
//         NodeIndex new_value = existing_value;
//         if (!existing)
//         {
//             assert(thread->interned.nodes.length < thread->interned.nodes.capacity);
//             new_value = intern_pool_put_node_at_assume_not_existent_assume_capacity(thread, node_index, index);
//             assert(!index_equal(new_value, existing_value));
//             assert(index_equal(new_value, node_index));
//         }
//         return (NodeGetOrPut) {
//             .index = new_value,
//             .existing = existing,
//         };
//     }
//     else
//     {
//         if (thread->interned.nodes.length < existing_capacity)
//         {
//             trap();
//         }
//         else if (thread->interned.nodes.length == existing_capacity)
//         {
//             auto result = intern_pool_put_node_assume_not_existent(thread, hash, node_index);
//             return (NodeGetOrPut) {
//                 .index = result,
//                 .existing = 0,
//             };
//         }
//         else
//         {
//             trap();
//         }
//     }
// }

// fn NodeIndex intern_pool_remove_node(Thread* thread, NodeIndex node_index)
// {
//     auto existing_capacity = thread->interned.nodes.capacity;
//     auto* node = thread_node_get(thread, node_index);
//     auto hash = hash_node(thread, node, node_index);
//     
//     auto original_index = hash & (existing_capacity - 1);
//     auto slot = intern_pool_find_node_slot(thread, cast(u32, u64, original_index), node_index);
//
//     if (slot != -1)
//     {
//         auto i = (u32)slot;
//         auto* slot_pointer = &thread->interned.nodes.pointer[i];
//         auto old_node_index = *(NodeIndex*)slot_pointer;
//         assert(validi(old_node_index));
//         thread->interned.nodes.length -= 1;
//         *slot_pointer = 0;
//
//         auto j = i;
//
//         while (1)
//         {
//             j = (j + 1) & (existing_capacity - 1);
//
//             auto existing = thread->interned.nodes.pointer[j];
//             if (existing == 0)
//             {
//                 break;
//             }
//
//             auto existing_node_index = *(NodeIndex*)&existing;
//             auto* existing_node = thread_node_get(thread, existing_node_index);
//             auto existing_node_hash = hash_node(thread, existing_node, existing_node_index);
//             auto k = existing_node_hash & (existing_capacity - 1);
//
//             if (i <= j)
//             {
//                 if ((i < k) & (k <= j))
//                 {
//                     continue;
//                 }
//             }
//             else
//             {
//                 if ((k <= j) | (i < k))
//                 {
//                     continue;
//                 }
//             }
//
//             thread->interned.nodes.pointer[i] = thread->interned.nodes.pointer[j];
//             thread->interned.nodes.pointer[j] = 0;
//
//             i = j;
//         }
//
//         return old_node_index;
//     }
//     else
//     {
//         trap();
//     }
// }

STRUCT(Parser)
{
    u64 i;
    u32 line;
    u32 column;
};

[[gnu::hot]] fn void skip_space(Parser* parser, String src)
{
    u64 original_i = parser->i;

    if (original_i != src.length)
    {
        if (is_space(src.pointer[original_i], get_next_ch_safe(src, original_i)))
        {
            while (parser->i < src.length)
            {
                u64 index = parser->i;
                u8 ch = src.pointer[index];
                u64 new_line = ch == '\n';
                parser->line += new_line;

                if (new_line)
                {
                    // TODO: is this a bug?
                    parser->column = cast(u32, u64, index + 1);
                }

                if (!is_space(ch, get_next_ch_safe(src, parser->i)))
                {
                    break;
                }

                u32 is_comment = src.pointer[index] == '/';
                parser->i += is_comment + is_comment;
                if (is_comment)
                {
                    while (parser->i < src.length)
                    {
                        if (src.pointer[parser->i] == '\n')
                        {
                            break;
                        }

                        parser->i += 1;
                    }

                    continue;
                }

                parser->i += 1;
            }
        }
    }
}

[[gnu::hot]] fn void expect_character(Parser* parser, String src, u8 expected_ch)
{
    u64 index = parser->i;
    if (likely(index < src.length))
    {
        u8 ch = src.pointer[index];
        auto matches = cast(u64, s64, likely(ch == expected_ch));
        parser->i += matches;
        if (!matches)
        {
            print_string(strlit("expected character '"));
            print_string(ch_to_str(expected_ch));
            print_string(strlit("', but found '"));
            print_string(ch_to_str(ch));
            print_string(strlit("'\n"));
            fail();
        }
    }
    else
    {
        print_string(strlit("expected character '"));
        print_string(ch_to_str(expected_ch));
        print_string(strlit("', but found end of file\n"));
        fail();
    }
}

[[gnu::hot]] fn String parse_identifier(Parser* parser, String src)
{
    u64 identifier_start_index = parser->i;
    u64 is_string_literal = src.pointer[identifier_start_index] == '"';
    parser->i += is_string_literal;
    u8 identifier_start_ch = src.pointer[parser->i];
    u64 is_valid_identifier_start = is_identifier_start(identifier_start_ch);
    parser->i += is_valid_identifier_start;

    if (likely(is_valid_identifier_start))
    {
        while (parser->i < src.length)
        {
            u8 ch = src.pointer[parser->i];
            auto is_identifier = cast(u64, s64, likely(is_identifier_ch(ch)));
            parser->i += is_identifier;

            if (!is_identifier)
            {
                if (unlikely(is_string_literal))
                {
                    expect_character(parser, src, '"');
                }

                String result = s_get_slice(u8, src, identifier_start_index, parser->i - is_string_literal);
                return result;
            }
        }

        fail();
    }
    else
    {
        fail();
    }
}

#define array_start '['
#define array_end ']'

#define argument_start '('
#define argument_end ')'

#define block_start '{'
#define block_end '}'

#define pointer_sign '*'

// fn void thread_add_job(Thread* thread, NodeIndex node_index)
// {
//     unused(thread);
//     unused(node_index);
//     trap();
// }

// fn void thread_add_jobs(Thread* thread, Slice(NodeIndex) nodes)
// {
//     for (u32 i = 0; i < nodes.length; i += 1)
//     {
//         NodeIndex node_index = nodes.pointer[i];
//         thread_add_job(thread, node_index);
//     }
// }

union NodePair
{
    struct
    {
        NodeIndex old;
        NodeIndex new;
    };
    NodeIndex nodes[2];
};
typedef union NodePair NodePair;

// fn NodeIndex node_keep(Thread* thread, NodeIndex node_index)
// {
//     return node_add_output(thread, node_index, invalidi(Node));
// }

// fn NodeIndex node_unkeep(Thread* thread, NodeIndex node_index)
// {
//     node_remove_output(thread, node_index, invalidi(Node));
//     return node_index;
// }

fn NodeIndex dead_code_elimination(Thread* thread, NodePair nodes)
{
    NodeIndex old = nodes.old;
    NodeIndex new = nodes.new;

    if (!index_equal(old, new))
    {
        // print("[DCE] old: #{u32} != new: #{u32}. Proceeding to eliminate\n", old.index, new.index);
        auto* old_node = thread_node_get(thread, old);
        unused(old_node);
        todo();
        // if (node_is_unused(old_node) & !node_is_dead(old_node))
        // {
        //     node_keep(thread, new);
        //     todo();
        //     // node_kill(thread, old);
        //     // node_unkeep(thread, new);
        // }
    }

    return new;
}

// fn u8 type_is_high_or_const(Thread* thread, TypeIndex type_index)
// {
//     u8 result = index_equal(type_index, thread->types.top) | index_equal(type_index, thread->types.dead_control);
//     if (!result)
//     {
//         Type* type = thread_type_get(thread, type_index);
//         switch (type->id)
//         {
//             case TYPE_INTEGER:
//                 result = type->integer.is_constant | ((type->integer.constant == 0) & (type->integer.bit_count == 0));
//                 break;
//             default:
//                 break;
//         }
//     }
//
//     return result;
// }

// fn TypeIndex type_join(Thread* thread, TypeIndex a, TypeIndex b)
// {
//     TypeIndex result;
//     if (index_equal(a, b))
//     {
//         result = a;
//     }
//     else
//     {
//         unused(thread);
//         trap();
//     }
//
//     return result;
// }

// fn void node_set_type(Thread* thread, Node* node, TypeIndex new_type)
// {
//     todo();
//     // auto old_type = node->type;
//     // assert(!validi(old_type) || type_is_a(thread, new_type, old_type));
//     // if (!index_equal(old_type, new_type))
//     // {
//     //     node->type = new_type;
//     //     auto outputs = node_get_outputs(thread, node);
//     //     thread_add_jobs(thread, outputs);
//     //     // move_dependencies_to_worklist(thread, node);
//     // }
// }

global auto enable_peephole = 1;

fn NodeIndex peephole_optimize(Thread* thread, Function* function, NodeIndex node_index)
{
    assert(enable_peephole);
    auto result = node_index;
    auto* node = thread_node_get(thread, node_index);
    // print("Peepholing node #{u32} ({s})\n", node_index.index, node_id_to_string(node));
    auto old_type = node->type;
    auto new_type = node_functions[node->id].compute_type(thread, node_index);
    unused(new_type);
    unused(old_type);

    if (enable_peephole)
    {
        unused(function);
        // thread->iteration.total += 1;
        // node_set_type(thread, node, new_type);
        //
        // if (node->id != NODE_CONSTANT && node->id != NODE_DEAD_CONTROL && type_is_high_or_const(thread, node->type))
        // {
        //     if (index_equal(node->type, thread->types.dead_control))
        //     {
        //         trap();
        //     }
        //     else
        //     {
        //         auto constant_node = constant_int_create_with_type(thread, function, node->type);
        //         return constant_node;
        //     }
        // }
        //
        // auto idealize = 1;
        // if (!node->hash)
        // {
        //     auto gop = intern_pool_get_or_put_node(thread, node_index);
        //     idealize = !gop.existing;
        //
        //     if (gop.existing) 
        //     {
        //         auto interned_node_index = gop.index;
        //         auto* interned_node = thread_node_get(thread, interned_node_index);
        //         auto new_type = type_join(thread, interned_node->type, node->type);
        //         node_set_type(thread, interned_node, new_type);
        //         node->hash = 0;
        //         // print("[peephole_optimize] Eliminating #{u32} because an existing node was found: #{u32}\n", node_index.index, interned_node_index.index);
        //         auto dce_node = dead_code_elimination(thread, (NodePair) {
        //             .old = node_index,
        //             .new = interned_node_index,
        //         });
        //
        //         result = dce_node;
        //     }
        // }
        //
        // if (idealize)
        // {
        //     auto idealized_node = node_functions[node->id].idealize(thread, node_index);
        //     if (validi(idealized_node))
        //     {
        //         result = idealized_node;
        //     }
        //     else
        //     {
        //         u64 are_types_equal = index_equal(new_type, old_type);
        //         thread->iteration.nop += are_types_equal;
        //         
        //         result = are_types_equal ? invalidi(Node) : node_index;
        //     }
        // }
        todo();
    }
    else
    {
        todo();
        // node->type = new_type;
    }

    return result;
}

fn NodeIndex peephole(Thread* thread, Function* function, NodeIndex node_index)
{
    NodeIndex result;
    if (enable_peephole)
    {
        NodeIndex new_node = peephole_optimize(thread, function, node_index);
        if (validi(new_node))
        {
            NodeIndex peephole_new_node = peephole(thread, function, new_node);
            // print("[peephole] Eliminating #{u32} because a better node was found: #{u32}\n", node_index.index, new_node.index);
            auto dce_node = dead_code_elimination(thread, (NodePair)
            {
                .old = node_index,
                .new = peephole_new_node,
            });

            result = dce_node;
        }
        else
        {
            result = node_index;
        }
    }
    else
    {
        auto* node = thread_node_get(thread, node_index);
        auto new_type = node_functions[node->id].compute_type(thread, node_index);
        unused(new_type);
        todo();
        // node->type = new_type;
        // result = node_index;
    }

    return result;
}

fn NodeIndex node_project(Thread* thread, NodeIndex node_index, TypePair type, u32 index)
{
    auto* node = thread_node_get(thread, node_index);
    assert(type_pair_get_backend(node->type) == BACKEND_TYPE_TUPLE);
    auto projection_node_index = thread_node_add(thread, (NodeCreate)
    {
        .id = IR_PROJECTION,
        .inputs = array_to_slice(((NodeIndex[]) { node_index })),
        .type_pair = type,
    });
    auto* projection = thread_node_get(thread, projection_node_index);
    projection->projection = (NodeProjection)
    {
        .index = index,
    };
    return projection_node_index;
}

fn TypePair analyze_type(Thread* thread, Parser* parser, String src)
{
    u64 start_index = parser->i;
    u8 start_ch = src.pointer[start_index];
    u32 is_array_start = start_ch == array_start;
    u32 u_start = start_ch == 'u';
    u32 s_start = start_ch == 's';
    u32 float_start = start_ch == 'f';
    u32 void_start = start_ch == 'v';
    u32 pointer_start = start_ch == pointer_sign;
    u32 integer_start = u_start | s_start;
    u32 number_start = integer_start | float_start;

    if (void_start)
    {
        trap();
    }
    else if (is_array_start)
    {
        trap();
    }
    else if (pointer_start)
    {
        trap();
    }
    else if (number_start)
    {
        u64 expected_digit_start = start_index + 1;
        u64 i = expected_digit_start;
        u32 decimal_digit_count = 0;
        u64 top = i + 5;

        while (i < top)
        {
            u8 ch = src.pointer[i];
            auto is_digit = is_decimal_digit(ch);
            decimal_digit_count += is_digit;
            if (!is_digit)
            {
                auto is_alpha = is_alphabetic(ch);
                if (is_alpha)
                {
                    decimal_digit_count = 0;
                }
                break;
            }

            i += 1;
        }


        if (decimal_digit_count)
        {
            parser->i += 1;

            if (integer_start)
            {
                auto signedness = cast(u8, u64, s_start);
                u64 bit_size;
                u64 current_i = parser->i;
                assert(src.pointer[current_i] >= '0' & src.pointer[current_i] <= '9');
                switch (decimal_digit_count) {
                    case 0:
                        fail();
                    case 1:
                        bit_size = src.pointer[current_i] - '0';
                        break;
                    case 2:
                        bit_size = (src.pointer[current_i] - '0') * 10 + (src.pointer[current_i + 1] - '0');
                        break;
                    default:
                        fail();
                }
                parser->i += decimal_digit_count;

                assert(!is_decimal_digit(src.pointer[parser->i]));

                if (bit_size)
                {
                    auto bit_count = cast(u8, u64, bit_size);
                    auto valid = MIN(MAX(8, round_up_to_next_power_of_2(MAX(bit_count, 1))), 64);
                    if (bit_count != valid)
                    {
                        fail();
                    }
                    auto bit_index = cast(u32, s32, __builtin_ctz(bit_count >> 3));
                    static_assert(array_length(thread->types.debug.integer.array) == 8);
                    auto index = signedness * 4 + bit_index;
                    auto debug_type_index = thread->types.debug.integer.array[index];
                    BackendTypeId backend_type = bit_index + 1;
                    auto type_pair = type_pair_make(debug_type_index, backend_type);
                    return type_pair;
                }
                else
                {
                    fail();
                }
            }
            else if (float_start)
            {
                trap();
            }
            else
            {
                trap();
            }
        }
        else
        {
            fail();
        }
    }

    trap();
}

declare_ip_functions(Node, node)

// TODO:
fn NodeIndex node_gvn_intern(Thread* thread, NodeIndex node_index)
{
    auto result = ip_Node_get_or_put(&thread->interned.nodes, thread, node_index);
    if (result.existing)
    {
        assert(thread_node_get(thread, result.index)->interned);
    }
    else
    {
        thread_node_get(thread, node_index)->interned = 1;
    }
    return result.index;
}

fn void node_gvn_remove(Thread* thread, NodeIndex node_index)
{
    auto result = ip_Node_remove(&thread->interned.nodes, thread, node_index);
    assert(index_equal(result, node_index));
    thread_node_get(thread, node_index)->interned = 0;
}

fn NodeIndex analyze_primary_expression(Thread* thread, Parser* parser, FunctionBuilder* builder, String src)
{
    u8 starting_ch = src.pointer[parser->i];
    u64 is_digit = is_decimal_digit(starting_ch);
    u64 is_identifier = is_identifier_start(starting_ch);

    if (is_identifier)
    {
        String identifier = parse_identifier(parser, src);
        unused(identifier);
        todo();
        // auto node_index = scope_lookup(thread, builder, identifier);
        // if (validi(node_index))
        // {
        //     return node_index;
        // }
        // else
        // {
        //     fail();
        // }
    }
    else if (is_digit)
    {
        typedef enum IntegerPrefix {
            INTEGER_PREFIX_HEXADECIMAL,
            INTEGER_PREFIX_DECIMAL,
            INTEGER_PREFIX_OCTAL,
            INTEGER_PREFIX_BINARY,
        } IntegerPrefix;
        IntegerPrefix prefix = INTEGER_PREFIX_DECIMAL;
        u64 value = 0;

        if (starting_ch == '0')
        {
            auto follow_up_character = src.pointer[parser->i + 1];
            auto is_hex_start = follow_up_character == 'x';
            auto is_octal_start = follow_up_character == 'o';
            auto is_bin_start = follow_up_character == 'b';
            auto is_prefixed_start = is_hex_start | is_octal_start | is_bin_start;
            auto follow_up_alpha = is_alphabetic(follow_up_character);
            auto follow_up_digit = is_decimal_digit(follow_up_character);
            auto is_valid_after_zero = is_space(follow_up_character, get_next_ch_safe(src, follow_up_character)) | (!follow_up_digit & !follow_up_alpha);

            if (is_prefixed_start) {
                switch (follow_up_character) {
                    case 'x': prefix = INTEGER_PREFIX_HEXADECIMAL; break;
                    case 'o': prefix = INTEGER_PREFIX_OCTAL; break;
                    case 'd': prefix = INTEGER_PREFIX_DECIMAL; break;
                    case 'b': prefix = INTEGER_PREFIX_BINARY; break;
                    default: fail();
                };

                parser->i += 2;

            } else if (!is_valid_after_zero) {
                fail();
            }
        }

        auto start = parser->i;

        switch (prefix)
        {
            case INTEGER_PREFIX_HEXADECIMAL:
                {
                    // while (is_hex_digit(src[parser->i])) {
                    //     parser->i += 1;
                    // }

                    trap();
                    // auto slice = src.slice(start, parser->i);
                    // value = parse_hex(slice);
                } break;
            case INTEGER_PREFIX_DECIMAL:
                {
                    while (is_decimal_digit(src.pointer[parser->i]))
                    {
                        parser->i += 1;
                    }

                    value = parse_decimal(s_get_slice(u8, src, start, parser->i));
                } break;
            case INTEGER_PREFIX_OCTAL:
                {
                    trap();
                } break;
            case INTEGER_PREFIX_BINARY:
                {
                    trap();
                } break;
        }

        auto node_index = thread_node_add(thread, (NodeCreate){
            .inputs = array_to_slice(((NodeIndex []) {
                builder->function->root,
            })),
            .type_pair = type_pair_make(thread->types.debug.integer.u64, BACKEND_TYPE_INTEGER_64),
            .id = IR_INTEGER_CONSTANT,
        });

        auto* node = thread_node_get(thread, node_index);
        node->integer_constant = (NodeIntegerConstant) {
            .unsigned_value = value,
        };

        auto new_node_index = node_gvn_intern(thread, node_index);
       
        return new_node_index;
    }
    else
    {
        trap();
    }
}

fn NodeIndex analyze_unary(Thread* thread, Parser* parser, FunctionBuilder* builder, String src)
{
    typedef enum PrefixOperator
    {
        PREFIX_OPERATOR_NONE = 0,
        PREFIX_OPERATOR_NEGATION,
        PREFIX_OPERATOR_LOGICAL_NOT,
        PREFIX_OPERATOR_BITWISE_NOT,
        PREFIX_OPERATOR_ADDRESS_OF,
    } PrefixOperator;

    PrefixOperator prefix_operator;
    NodeIndex node_index;

    switch (src.pointer[parser->i])
    {
        case '-':
            todo();
        case '!':
            todo();
        case '~':
            todo();
        case '&':
            todo();
        default:
            {
                node_index = analyze_primary_expression(thread, parser, builder, src);
                prefix_operator = PREFIX_OPERATOR_NONE;
            } break;
    }

    // typedef enum SuffixOperator
    // {
    //     SUFFIX_OPERATOR_NONE = 0,
    //     SUFFIX_OPERATOR_CALL,
    //     SUFFIX_OPERATOR_ARRAY,
    //     SUFFIX_OPERATOR_FIELD,
    //     SUFFIX_OPERATOR_POINTER_DEREFERENCE,
    // } SuffixOperator;
    //
    // SuffixOperator suffix_operator;

    skip_space(parser, src);

    switch (src.pointer[parser->i])
    {
        case argument_start:
            todo();
        case array_start:
            todo();
        case '.':
            todo();
        default:
            break;
    }

    if (prefix_operator != PREFIX_OPERATOR_NONE)
    {
        todo();
    }

    return node_index;
}

fn NodeIndex analyze_multiplication(Thread* thread, Parser* parser, FunctionBuilder* builder, String src)
{
    auto left = analyze_unary(thread, parser, builder, src);

    while (1)
    {
        skip_space(parser, src);

        NodeId node_id;
        u64 skip_count = 1;

        switch (src.pointer[parser->i])
        {
            case '*':
                node_id = IR_INTEGER_MULTIPLY;
                break;
            case '/':
                node_id = IR_INTEGER_DIVIDE;
                break;
            case '%':
                todo();
            default:
                node_id = NODE_COUNT;
                break;
        }

        if (node_id == NODE_COUNT) 
        {
            break;
        }

        parser->i += skip_count;
        skip_space(parser, src);

        auto new_node_index = thread_node_add(thread, (NodeCreate) {
            .id = node_id,
            .inputs = array_to_slice(((NodeIndex[]) {
                invalidi(Node),
                left,
                invalidi(Node),
            })),
        });

        // print("Before right: LEFT is #{u32}\n", left.index);
        // print("Left code:\n```\n{s}\n```\n", s_get_slice(u8, src, parser->i, src.length));
        auto right = analyze_multiplication(thread, parser, builder, src);
        unused(right);
        // print("Addition: left: #{u32}, right: #{u32}\n", left.index, right.index);
        // print("Left code:\n```\n{s}\n```\n", s_get_slice(u8, src, parser->i, src.length));

        todo();
        // node_set_input(thread, new_node_index, 2, right);

        // print("Addition new node #{u32}\n", new_node_index.index);
        // print("Left code:\n```\n{s}\n```\n", s_get_slice(u8, src, parser->i, src.length));

        left = peephole(thread, builder->function, new_node_index);
    }

    // print("Analyze addition returned node #{u32}\n", left.index);

    return left;
}

fn NodeIndex analyze_addition(Thread* thread, Parser* parser, FunctionBuilder* builder, String src)
{
    auto left = analyze_multiplication(thread, parser, builder, src);

    while (1)
    {
        skip_space(parser, src);

        NodeId node_id;

        switch (src.pointer[parser->i])
        {
            case '+':
                node_id = IR_INTEGER_ADD;
                break;
            case '-':
                node_id = IR_INTEGER_SUBSTRACT;
                break;
            default:
                node_id = NODE_COUNT;
                break;
        }

        if (node_id == NODE_COUNT) 
        {
            break;
        }

        parser->i += 1;
        skip_space(parser, src);

        auto new_node_index = thread_node_add(thread, (NodeCreate) {
            .id = node_id,
            .inputs = array_to_slice(((NodeIndex[]) {
                invalidi(Node),
                left,
                invalidi(Node),
            })),
        });

        // print("Before right: LEFT is #{u32}\n", left.index);
        // print("Left code:\n```\n{s}\n```\n", s_get_slice(u8, src, parser->i, src.length));
        auto right = analyze_multiplication(thread, parser, builder, src);
        unused(right);
        // print("Addition: left: #{u32}, right: #{u32}\n", left.index, right.index);
        // print("Left code:\n```\n{s}\n```\n", s_get_slice(u8, src, parser->i, src.length));

        // node_set_input(thread, new_node_index, 2, right);
        todo();


        // print("Addition new node #{u32}\n", new_node_index.index);
        // print("Left code:\n```\n{s}\n```\n", s_get_slice(u8, src, parser->i, src.length));

        left = peephole(thread, builder->function, new_node_index);
    }

    // print("Analyze addition returned node #{u32}\n", left.index);

    return left;
}

fn NodeIndex analyze_shift(Thread* thread, Parser* parser, FunctionBuilder* builder, String src)
{
    auto left = analyze_addition(thread, parser, builder, src);

    while (1)
    {
        skip_space(parser, src);

        NodeId node_id;

        if ((src.pointer[parser->i] == '<') & (src.pointer[parser->i + 1] == '<'))
        {
            node_id = IR_INTEGER_SHIFT_LEFT;
        }
        else if ((src.pointer[parser->i] == '>') & (src.pointer[parser->i + 1] == '>'))
        {
            node_id = IR_INTEGER_SHIFT_RIGHT;
        }
        else
        {
            break;
        }

        parser->i += 2;
        skip_space(parser, src);

        auto new_node_index = thread_node_add(thread, (NodeCreate) {
            .id = node_id,
            .inputs = array_to_slice(((NodeIndex[]) {
                invalidi(Node),
                left,
                invalidi(Node),
            })),
        });

        // print("Before right: LEFT is #{u32}\n", left.index);
        // print("Left code:\n```\n{s}\n```\n", s_get_slice(u8, src, parser->i, src.length));
        auto right = analyze_addition(thread, parser, builder, src);
        unused(right);
        // print("Addition: left: #{u32}, right: #{u32}\n", left.index, right.index);
        // print("Left code:\n```\n{s}\n```\n", s_get_slice(u8, src, parser->i, src.length));

        // node_set_input(thread, new_node_index, 2, right);
        todo();

        // print("Addition new node #{u32}\n", new_node_index.index);
        // print("Left code:\n```\n{s}\n```\n", s_get_slice(u8, src, parser->i, src.length));

        left = peephole(thread, builder->function, new_node_index);
    }

    return left;
}

fn NodeIndex analyze_bitwise_binary(Thread* thread, Parser* parser, FunctionBuilder* builder, String src)
{
    auto left = analyze_shift(thread, parser, builder, src);

    while (1)
    {
        skip_space(parser, src);

        NodeId node_id;
        u64 skip_count = 1;

        switch (src.pointer[parser->i])
        {
            case '&':
                node_id = IR_INTEGER_AND;
                break;
            case '|':
                node_id = IR_INTEGER_OR;
                break;
            case '^':
                node_id = IR_INTEGER_XOR;
                break;
            default:
                node_id = NODE_COUNT;
                break;
        }

        if (node_id == NODE_COUNT)
        {
            break;
        }

        parser->i += skip_count;
        skip_space(parser, src);

        auto new_node_index = thread_node_add(thread, (NodeCreate) {
            .id = node_id,
            .inputs = array_to_slice(((NodeIndex[]) {
                invalidi(Node),
                left,
                invalidi(Node),
            })),
        });

        // print("Before right: LEFT is #{u32}\n", left.index);
        // print("Left code:\n```\n{s}\n```\n", s_get_slice(u8, src, parser->i, src.length));
        auto right = analyze_shift(thread, parser, builder, src);
        unused(right);
        // print("Addition: left: #{u32}, right: #{u32}\n", left.index, right.index);
        // print("Left code:\n```\n{s}\n```\n", s_get_slice(u8, src, parser->i, src.length));

        // node_set_input(thread, new_node_index, 2, right);
        todo();

        // print("Addition new node #{u32}\n", new_node_index.index);
        // print("Left code:\n```\n{s}\n```\n", s_get_slice(u8, src, parser->i, src.length));

        left = peephole(thread, builder->function, new_node_index);
    }

    return left;
}

fn NodeIndex analyze_comparison(Thread* thread, Parser* parser, FunctionBuilder* builder, String src)
{
    auto left = analyze_bitwise_binary(thread, parser, builder, src);

    while (1)
    {
        skip_space(parser, src);

        NodeId node_id;
        u64 skip_count = 1;

        switch (src.pointer[parser->i])
        {
            case '=':
                todo();
            case '!':
                if (src.pointer[parser->i + 1] == '=')
                {
                    skip_count = 2;
                    node_id = IR_INTEGER_COMPARE_NOT_EQUAL;
                }
                else
                {
                    fail();
                }
                break;
            case '<':
                todo();
            case '>':
                todo();
            default:
                node_id = NODE_COUNT;
                break;
        }

        if (node_id == NODE_COUNT)
        {
            break;
        }

        parser->i += skip_count;
        skip_space(parser, src);

        auto new_node_index = thread_node_add(thread, (NodeCreate) {
            .id = node_id,
            .inputs = array_to_slice(((NodeIndex[]) {
                invalidi(Node),
                left,
                invalidi(Node),
            })),
        });

        // print("Before right: LEFT is #{u32}\n", left.index);
        // print("Left code:\n```\n{s}\n```\n", s_get_slice(u8, src, parser->i, src.length));
        auto right = analyze_bitwise_binary(thread, parser, builder, src);
        unused(right);
        // print("Addition: left: #{u32}, right: #{u32}\n", left.index, right.index);
        // print("Left code:\n```\n{s}\n```\n", s_get_slice(u8, src, parser->i, src.length));

        // node_set_input(thread, new_node_index, 2, right);
        todo();

        // print("Addition new node #{u32}\n", new_node_index.index);
        // print("Left code:\n```\n{s}\n```\n", s_get_slice(u8, src, parser->i, src.length));

        left = peephole(thread, builder->function, new_node_index);
    }

    return left;
}

fn NodeIndex analyze_expression(Thread* thread, Parser* parser, FunctionBuilder* builder, String src, TypePair result_type)
{
    NodeIndex result = analyze_comparison(thread, parser, builder, src);
    // TODO: typecheck
    unused(result_type);
    return result;
}

fn void analyze_block(Thread* thread, Parser* parser, FunctionBuilder* builder, String src)
{
    expect_character(parser, src, block_start);

    // TODO
    // scope_push(thread, builder);

    Function* function = builder->function;

    while (1)
    {
        skip_space(parser, src);

        if (s_get(src, parser->i) == block_end)
        {
            break;
        }

        u8 statement_start_ch = src.pointer[parser->i];

        if (is_identifier_start(statement_start_ch))
        {
            String statement_start_identifier = parse_identifier(parser, src); 
            if (s_equal(statement_start_identifier, (strlit("return"))))
            {
                skip_space(parser, src);
                NodeIndex return_value = analyze_expression(thread, parser, builder, src, function->return_type);
                skip_space(parser, src);
                expect_character(parser, src, ';');

                auto* current_node = thread_node_get(thread, builder->current);
                auto current_inputs = node_get_inputs(thread, current_node);
                auto mem_state = current_inputs.pointer[2];

                auto return_node_index = node_get_inputs(thread, thread_node_get(thread, builder->function->root)).pointer[1];
                auto* return_node = thread_node_get(thread, return_node_index);
                assert(return_node->input_count >= 4);
                auto return_inputs = node_get_inputs(thread, return_node);

                node_add_input(thread, return_inputs.pointer[1], mem_state);

                node_add_input(thread, return_inputs.pointer[3], return_value);

                auto control = return_inputs.pointer[0];
                assert(thread_node_get(thread, control)->id == IR_REGION);

                assert(validi(current_inputs.pointer[0]));
                node_add_input(thread, control, current_inputs.pointer[0]);

                builder->current = invalidi(Node);
                continue;
            }

            String left_name = statement_start_identifier;
            unused(left_name);

            skip_space(parser, src);

            typedef enum AssignmentOperator
            {
                ASSIGNMENT_OPERATOR_NONE,
            } AssignmentOperator;

            AssignmentOperator assignment_operator;
            switch (src.pointer[parser->i])
            {
                case '=':
                    assignment_operator = ASSIGNMENT_OPERATOR_NONE;
                    parser->i += 1;
                    break;
                default:
                    trap();
            }
            unused(assignment_operator);

            skip_space(parser, src);

            NodeIndex initial_right = analyze_expression(thread, parser, builder, src, type_pair_invalid);
            unused(initial_right);

            expect_character(parser, src, ';');

            todo();

            // auto left = scope_lookup(thread, builder, left_name);
            // if (!validi(left))
            // {
            //     fail();
            // }
            //
            // NodeIndex right;
            // switch (assignment_operator)
            // {
            //     case ASSIGNMENT_OPERATOR_NONE:
            //         right = initial_right;
            //         break;
            // }
            //
            // scope_update(thread, builder, left_name, right);
        }
        else
        {
            switch (statement_start_ch)
            {
                case '>':
                    {
                        parser->i += 1;
                        skip_space(parser, src);

                        String local_name = parse_identifier(parser, src); 
                        unused(local_name);

                        skip_space(parser, src);

                        auto type = type_pair_invalid;

                        u8 has_type_declaration = src.pointer[parser->i] == ':';
                        if (has_type_declaration)
                        {
                            parser->i += 1;

                            skip_space(parser, src);

                            type = analyze_type(thread, parser, src);

                            skip_space(parser, src);
                        }

                        expect_character(parser, src, '=');

                        skip_space(parser, src);

                        auto initial_value_node_index = analyze_expression(thread, parser, builder, src, type);
                        skip_space(parser, src);
                        expect_character(parser, src, ';');

                        auto* initial_value_node = thread_node_get(thread, initial_value_node_index);
                        unused(initial_value_node);
                        
                        // TODO: typecheck
                        todo();

                        // auto result = scope_define(thread, builder, local_name, initial_value_node->type, initial_value_node_index);
                        // if (!validi(result))
                        // {
                        //     fail();
                        // }
                    } break;
                case block_start:
                    analyze_block(thread, parser, builder, src);
                    break;
                default:
                    todo();
                    break;
            }
        }
    }

    expect_character(parser, src, block_end);

    // scope_pop(thread, builder);
}

fn void analyze_file(Thread* thread, File* file)
{
    Parser p = {};
    Parser* restrict parser = &p;
    String src = file->source;

    while (1)
    {
        skip_space(parser, src);

        if (parser->i == src.length)
        {
            break;
        }

        // Parse top level declaration
        u64 start_ch_index = parser->i;
        auto start_line = parser->line;
        auto start_column = parser->column;
        u8 start_ch = s_get(src, start_ch_index);

        u64 is_identifier = is_identifier_start(start_ch);

        if (is_identifier)
        {
            u8 next_ch = get_next_ch_safe(src, start_ch_index);
            u64 is_fn = (start_ch == 'f') & (next_ch == 'n');

            if (is_fn)
            {
                parser->i += 2;

                FunctionBuilder function_builder = {};
                FunctionBuilder* builder = &function_builder;
                builder->file = file;

                skip_space(parser, src);

                Function* restrict function = vb_add(&thread->buffer.functions, 1);
                auto function_index = cast(u32, s64, function - thread->buffer.functions.pointer);
                memset(function, 0, sizeof(Function));
                builder->function = function;
                function->line = start_line;
                function->column = start_column;
                function->name = parse_identifier(parser, src);
                if (s_equal(function->name, strlit("main")))
                {
                    thread->main_function = function_index;
                }

                skip_space(parser, src);
                
                // Parse arguments
                expect_character(parser, src, argument_start);

                u32 argument_i = 0;
                String argument_names[255];

                while (1)
                {
                    skip_space(parser, src);
                    if (src.pointer[parser->i] == argument_end)
                    {
                        break;
                    }

                    if (argument_i == 255)
                    {
                        // Maximum arguments reached
                        fail();
                    }

                    auto argument_name = parse_identifier(parser, src);
                    argument_names[argument_i] = argument_name;

                    skip_space(parser, src);
                    expect_character(parser, src, ':');
                    skip_space(parser, src);

                    auto type_index = analyze_type(thread, parser, src);
                    unused(type_index);
                    // start_argument_type_buffer[argument_i] = type_index;
                    argument_i += 1;

                    skip_space(parser, src);

                    switch (src.pointer[parser->i])
                    {
                        case argument_end:
                            break;
                        default:
                            trap();
                    }
                }

                expect_character(parser, src, argument_end);
                skip_space(parser, src);

                function->return_type = analyze_type(thread, parser, src);

                function->root = thread_node_add(thread, (NodeCreate)
                {
                    .type_pair = type_pair_make(invalidi(DebugType), BACKEND_TYPE_TUPLE),
                    .id = IR_ROOT,
                    .inputs = array_to_slice(((NodeIndex[]){
                        invalidi(Node), // TODO: add callgraph node
                        invalidi(Node), // return node
                    })),
                });

                auto* root_node = thread_node_get(thread, function->root);
                root_node->root = (NodeRoot)
                {
                    .function_index = function_index,
                };

                auto control = node_project(thread, function->root, type_pair_make(invalidi(DebugType), BACKEND_TYPE_CONTROL), 0);
                auto memory = node_project(thread, function->root, type_pair_make(invalidi(DebugType), BACKEND_TYPE_MEMORY), 1);
                auto pointer = node_project(thread, function->root, type_pair_make(invalidi(DebugType), BACKEND_TYPE_POINTER), 2);

                if (argument_i > 0)
                {
                    // TODO: project arguments
                    todo();
                }

                NodeIndex fake[256] = {};
                auto slice = (Slice(NodeIndex)) array_to_slice(fake);
                slice.length = 4;
                auto return_node_index = thread_node_add(thread, (NodeCreate)
                {
                    .id = IR_RETURN,
                    .inputs = slice,
                    .type_pair = type_pair_make(invalidi(DebugType), BACKEND_TYPE_CONTROL),
                });

                node_set_input(thread, function->root, 1, return_node_index);

                auto region = thread_node_add(thread, (NodeCreate)
                {
                    .id = IR_REGION,
                    .inputs = {},
                    .type_pair = type_pair_make(invalidi(DebugType), BACKEND_TYPE_CONTROL),
                });

                auto memory_phi = thread_node_add(thread, (NodeCreate)
                {
                    .id = IR_PHI,
                    .inputs = array_to_slice(((NodeIndex[]) {
                        region,
                    })),
                    .type_pair = type_pair_make(invalidi(DebugType), BACKEND_TYPE_MEMORY),
                });

                node_set_input(thread, return_node_index, 0, region);
                node_set_input(thread, return_node_index, 1, memory_phi);
                node_set_input(thread, return_node_index, 2, pointer);

                auto ret_phi = thread_node_add(thread, (NodeCreate)
                {
                    .id = IR_PHI,
                    .inputs = array_to_slice(((NodeIndex[]) {
                        region,
                    })),
                    .type_pair = function->return_type,
                });
                node_set_input(thread, ret_phi, 0, region);
                node_set_input(thread, return_node_index, 3, ret_phi);

                thread_node_get(thread, region)->region = (NodeRegion)
                {
                    .in_mem = memory_phi,
                };

                node_gvn_intern(thread, function->root);
                node_gvn_intern(thread, control);
                node_gvn_intern(thread, memory);
                node_gvn_intern(thread, pointer);

                skip_space(parser, src);

                auto symbol_table = thread_node_add(thread, (NodeCreate)
                {
                    .id = IR_SYMBOL_TABLE,
                    .inputs = array_to_slice(((NodeIndex[])
                    {
                        control,
                        control,
                        memory,
                        pointer,
                    })),
                });
                builder->current = symbol_table;

                analyze_block(thread, parser, builder, src);

                node_gvn_intern(thread, return_node_index);
                node_gvn_intern(thread, region);
                node_gvn_intern(thread, memory_phi);
                node_gvn_intern(thread, ret_phi);
                node_gvn_intern(thread, symbol_table);
            }
            else
            {
                trap();
            }
        }
        else
        {
            trap();
        }
    }
}

// typedef NodeIndex NodeCallback(Thread* thread, Function* function, NodeIndex node_index);
//
// fn NodeIndex node_walk_internal(Thread* thread, Function* function, NodeIndex node_index, NodeCallback* callback)
// {
//     if (bitset_get(&thread->worklist.visited, geti(node_index)))
//     {
//         return invalidi(Node);
//     }
//     else
//     {
//         bitset_set_value(&thread->worklist.visited, geti(node_index), 1);
//         auto callback_result = callback(thread, function, node_index);
//         if (validi(callback_result))
//         {
//             return callback_result;
//         }
//
//         auto* node = thread_node_get(thread, node_index);
//         auto inputs = node_get_inputs(thread, node);
//         auto outputs = node_get_outputs(thread, node);
//
//         for (u64 i = 0; i < inputs.length; i += 1)
//         {
//             auto n = inputs.pointer[i];
//             if (validi(n))
//             {
//                 auto n_result = node_walk_internal(thread, function, n, callback);
//                 if (validi(n_result))
//                 {
//                     return n_result;
//                 }
//             }
//         }
//
//         for (u64 i = 0; i < outputs.length; i += 1)
//         {
//             auto n = outputs.pointer[i];
//             if (validi(n))
//             {
//                 auto n_result = node_walk_internal(thread, function, n, callback);
//                 if (validi(n_result))
//                 {
//                     return n_result;
//                 }
//             }
//         }
//
//         return invalidi(Node);
//     }
// }
//
// fn NodeIndex node_walk(Thread* thread, Function* function, NodeIndex node_index, NodeCallback* callback)
// {
//     assert(thread->worklist.visited.length == 0);
//     NodeIndex result = node_walk_internal(thread, function, node_index, callback);
//     bitset_clear(&thread->worklist.visited);
//     return result;
// }
//
// fn NodeIndex progress_on_list_callback(Thread* thread, Function* function, NodeIndex node_index)
// {
//     if (bitset_get(&thread->worklist.bitset, geti(node_index)))
//     {
//         return invalidi(Node);
//     }
//     else
//     {
//         NodeIndex new_node = peephole_optimize(thread, function, node_index);
//         return new_node;
//     }
// }
//
// fn u8 progress_on_list(Thread* thread, Function* function, NodeIndex stop_node_index)
// {
//     thread->worklist.mid_assert = 1;
//
//     NodeIndex changed = node_walk(thread, function, stop_node_index, &progress_on_list_callback);
//
//     thread->worklist.mid_assert = 0;
//
//     return !validi(changed);
// }
//
// fn void iterate_peepholes(Thread* thread, Function* function, NodeIndex stop_node_index)
// {
//     assert(progress_on_list(thread, function, stop_node_index));
//     if (thread->worklist.nodes.length > 0)
//     {
//         while (1)
//         {
//             auto node_index = thread_worklist_pop(thread);
//             if (!validi(node_index))
//             {
//                 break;
//             }
//
//             auto* node = thread_node_get(thread, node_index);
//             todo();
//             // if (!node_is_dead(node))
//             // {
//             //     auto new_node_index = peephole_optimize(thread, function, node_index);
//             //     if (validi(new_node_index))
//             //     {
//             //         trap();
//             //     }
//             // }
//         }
//     }
//
//     thread_worklist_clear(thread);
// }

// fn u8 node_is_cfg(Node* node)
// {
//     switch (node->id)
//     {
//         case IR_START:
//         case IR_DEAD_CONTROL:
//         case IR_CONTROL_PROJECTION:
//         case IR_RETURN:
//         case IR_STOP:
//             return 1;
//         case IR_SCOPE:
//         case IR_CONSTANT:
//         case IR_PROJECTION:
//             return 0;
//         default:
//             trap();
//     }
// }

// fn void rpo_cfg(Thread* thread, NodeIndex node_index)
// {
//     auto* node = thread_node_get(thread, node_index);
//     if (node_is_cfg(node) && !bitset_get(&thread->worklist.visited, geti(node_index)))
//     {
//         bitset_set_value(&thread->worklist.visited, geti(node_index), 1);
//         auto outputs = node_get_outputs(thread, node);
//         for (u64 i = 0; i < outputs.length; i += 1)
//         {
//             auto output = outputs.pointer[i];
//             if (validi(output))
//             {
//                 rpo_cfg(thread, output);
//             }
//         }
//
//         *vb_add(&thread->worklist.nodes, 1) = node_index;
//     }
// }

// fn s32 node_loop_depth(Thread* thread, Node* node)
// {
//     assert(node_is_cfg(node));
//     s32 loop_depth;
//
//     switch (node->id)
//     {
//         case IR_START:
//             {
//                 loop_depth = node->start.cfg.loop_depth;
//                 if (!loop_depth)
//                 {
//                     loop_depth = node->start.cfg.loop_depth = 1;
//                 }
//             } break;
//         case IR_STOP:
//             {
//                 loop_depth = node->stop.cfg.loop_depth;
//                 if (!loop_depth)
//                 {
//                     loop_depth = node->stop.cfg.loop_depth = 1;
//                 }
//             } break;
//         case IR_RETURN:
//             {
//                 loop_depth = node->return_node.cfg.loop_depth;
//                 if (!loop_depth)
//                 {
//                     auto input_index = node_input_get(thread, node, 0);
//                     auto input = thread_node_get(thread, input_index);
//                     node->return_node.cfg.loop_depth = loop_depth = node_loop_depth(thread, input);
//                 }
//             } break;
//         case IR_CONTROL_PROJECTION:
//             {
//                 loop_depth = node->control_projection.cfg.loop_depth;
//                 if (!loop_depth)
//                 {
//                     auto input_index = node_input_get(thread, node, 0);
//                     auto input = thread_node_get(thread, input_index);
//                     node->control_projection.cfg.loop_depth = loop_depth = node_loop_depth(thread, input);
//                 }
//             } break;
//         case IR_DEAD_CONTROL:
//             {
//                 loop_depth = node->dead_control.cfg.loop_depth;
//                 if (!loop_depth)
//                 {
//                     auto input_index = node_input_get(thread, node, 0);
//                     auto input = thread_node_get(thread, input_index);
//                     node->dead_control.cfg.loop_depth = loop_depth = node_loop_depth(thread, input);
//                 }
//             } break;
//         default:
//             trap();
//     }
//
//     return loop_depth;
// }

// fn u8 node_is_region(Node* node)
// {
//     return (node->id == IR_REGION) | (node->id == IR_REGION_LOOP);
// }
//
// fn u8 node_is_pinned(Node* node)
// {
//     switch (node->id)
//     {
//         case IR_PROJECTION:
//         case IR_START:
//             return 1;
//         case IR_CONSTANT:
//         case IR_INTEGER_SUBSTRACT:
//         case IR_INTEGER_COMPARE_EQUAL:
//         case IR_INTEGER_COMPARE_NOT_EQUAL:
//             return 0;
//         default:
//             trap();
//     }
// }

// fn s32 node_cfg_get_immediate_dominator_tree_depth(Node* node)
// {
//     assert(node_is_cfg(node));
//     switch (node->id)
//     {
//         case IR_START:
//             return 0;
//         case IR_DEAD_CONTROL:
//             todo();
//         case IR_CONTROL_PROJECTION:
//             todo();
//         case IR_RETURN:
//             todo();
//         case IR_STOP:
//             todo();
//         default:
//             trap();
//     }
// }

// fn void schedule_early(Thread* thread, NodeIndex node_index, NodeIndex start_node)
// {
//     if (validi(node_index) && !bitset_get(&thread->worklist.visited, geti(node_index)))
//     {
//         bitset_set_value(&thread->worklist.visited, geti(node_index), 1);
//
//         auto* node = thread_node_get(thread, node_index);
//         auto inputs = node_get_inputs(thread, node);
//
//         for (u64 i = 0; i < inputs.length; i += 1)
//         {
//             auto input = inputs.pointer[i];
//
//             if (validi(input))
//             {
//                 auto* input_node = thread_node_get(thread, input);
//                 if (!node_is_pinned(input_node))
//                 {
//                     schedule_early(thread, node_index, start_node);
//                 }
//             }
//         }
//
//         if (!node_is_pinned(node))
//         {
//             auto early = start_node;
//
//             for (u64 i = 1; i < inputs.length; i += 1)
//             {
//                 auto input_index = inputs.pointer[i];
//                 auto input_node = thread_node_get(thread, input_index);
//                 auto control_input_index = node_input_get(thread, input_node, 0);
//                 auto* control_input_node = thread_node_get(thread, control_input_index);
//                 auto* early_node = thread_node_get(thread, early);
//                 auto input_depth = node_cfg_get_immediate_dominator_tree_depth(control_input_node);
//                 auto early_depth = node_cfg_get_immediate_dominator_tree_depth(early_node);
//                 if (input_depth > early_depth)
//                 {
//                     early = control_input_index;
//                     trap();
//                 }
//             }
//
//             node_set_input(thread, node_index, 0, early);
//         }
//     }
// }
//
// fn u8 node_cfg_block_head(Node* node)
// {
//     assert(node_is_cfg(node));
//     switch (node->id)
//     {
//         case IR_START:
//             return 1;
//         default:
//             trap();
//     }
// }
//
// fn u8 is_forwards_edge(Thread* thread, NodeIndex output_index, NodeIndex input_index)
// {
//     u8 result = validi(output_index) & validi(input_index);
//     if (result)
//     {
//         auto* output = thread_node_get(thread, output_index);
//         result = output->input_count > 2;
//         if (result)
//         {
//             auto input_index2 = node_input_get(thread, output, 2);
//
//             result = index_equal(input_index2, input_index);
//             
//             if (result)
//             {
//                 trap();
//             }
//         }
//     }
//
//     return result;
// }
//
// fn void schedule_late(Thread* thread, NodeIndex node_index, Slice(NodeIndex) nodes, Slice(NodeIndex) late)
// {
//     if (!validi(late.pointer[geti(node_index)]))
//     {
//         auto* node = thread_node_get(thread, node_index);
//
//         if (node_is_cfg(node))
//         {
//             late.pointer[geti(node_index)] = node_cfg_block_head(node) ? node_index : node_input_get(thread, node, 0);
//         }
//
//         if (node->id == IR_PHI)
//         {
//             trap();
//         }
//
//         auto outputs = node_get_outputs(thread, node);
//
//         for (u32 i = 0; i < outputs.length; i += 1)
//         {
//             NodeIndex output = outputs.pointer[i];
//             if (is_forwards_edge(thread, output, node_index))
//             {
//                 trap();
//             }
//         }
//
//         for (u32 i = 0; i < outputs.length; i += 1)
//         {
//             NodeIndex output = outputs.pointer[i];
//             if (is_forwards_edge(thread, output, node_index))
//             {
//                 trap();
//             }
//         }
//
//         if (!node_is_pinned(node))
//         {
//             unused(nodes);
//             trap();
//         }
//     }
// }

// fn void gcm_build_cfg(Thread* thread, NodeIndex start_node_index, NodeIndex stop_node_index)
// {
//     unused(stop_node_index);
//     // Fix loops
//     {
//         // TODO:
//     }
//
//     // Schedule early
//     rpo_cfg(thread, start_node_index);
//
//     u32 i = thread->worklist.nodes.length;
//     while (i > 0)
//     {
//         i -= 1;
//         auto node_index = thread->worklist.nodes.pointer[i];
//         auto* node = thread_node_get(thread, node_index);
//         node_loop_depth(thread, node);
//         auto inputs = node_get_inputs(thread, node);
//         for (u64 i = 0; i < inputs.length; i += 1)
//         {
//             auto input = inputs.pointer[i];
//             schedule_early(thread, input, start_node_index);
//         }
//
//         if (node_is_region(node))
//         {
//             trap();
//         }
//     }
//
//     // Schedule late
//
//     auto max_node_count = thread->buffer.nodes.length;
//     auto* alloc = arena_allocate(thread->arena, NodeIndex, max_node_count * 2);
//     auto late = (Slice(NodeIndex)) {
//         .pointer = alloc,
//         .length = max_node_count,
//     };
//     auto nodes = (Slice(NodeIndex)) {
//         .pointer = alloc + max_node_count,
//         .length = max_node_count,
//     };
//
//     schedule_late(thread, start_node_index, nodes, late);
//
//     for (u32 i = 0; i < late.length; i += 1)
//     {
//         auto node_index = nodes.pointer[i];
//         if (validi(node_index))
//         {
//             trap();
//             auto late_node_index = late.pointer[i];
//             node_set_input(thread, node_index, 0, late_node_index);
//         }
//     }
// }

// may_be_unused fn void print_function(Thread* thread, Function* function)
// {
//     print("fn {s}\n====\n", function->name);
//     VirtualBuffer(NodeIndex) nodes = {};
//     *vb_add(&nodes, 1) = function->stop;
//
//     while (1)
//     {
//         auto node_index = nodes.pointer[nodes.length - 1];
//         auto* node = thread_node_get(thread, node_index);
//
//         if (node->input_count)
//         {
//             for (u32 i = 1; i < node->input_count; i += 1)
//             {
//                 *vb_add(&nodes, 1) = node_input_get(thread, node, 1);
//             }
//             *vb_add(&nodes, 1) = node_input_get(thread, node, 0);
//         }
//         else
//         {
//             break;
//         }
//     }
//
//     u32 i = nodes.length;
//     while (i > 0)
//     {
//         i -= 1;
//
//         auto node_index = nodes.pointer[i];
//         auto* node = thread_node_get(thread, node_index);
//         unused(node);
//         todo();
//         // auto* type = thread_type_get(thread, node->type);
//         // print("%{u32} - {s} - {s} ", geti(node_index), type_id_to_string(type), node_id_to_string(node));
//         // auto inputs = node_get_inputs(thread, node);
//         // auto outputs = node_get_outputs(thread, node);
//         // 
//         // print("(INPUTS: { ");
//         // for (u32 i = 0; i < inputs.length; i += 1)
//         // {
//         //     auto input_index = inputs.pointer[i];
//         //     print("%{u32} ", geti(input_index));
//         // }
//         // print("} OUTPUTS: { ");
//         // for (u32 i = 0; i < outputs.length; i += 1)
//         // {
//         //     auto output_index = outputs.pointer[i];
//         //     print("%{u32} ", geti(output_index));
//         // }
//         // print_string(strlit("})\n"));
//     }
//
//
//     print("====\n", function->name);
// }

// struct CBackend
// {
//     VirtualBuffer(u8) buffer;
//     Function* function;
// };
//
// typedef struct CBackend CBackend;
//
// fn void c_lower_append_string(CBackend* backend, String string)
// {
//     vb_append_bytes(&backend->buffer, string);
// }
//
// fn void c_lower_append_ch(CBackend* backend, u8 ch)
// {
//     *vb_add(&backend->buffer, 1) = ch;
// }
//
// fn void c_lower_append_ch_repeated(CBackend* backend, u8 ch, u32 times)
// {
//     u8* pointer = vb_add(&backend->buffer, times);
//     memset(pointer, ch, times);
// }
//
// fn void c_lower_append_space(CBackend* backend)
// {
//     c_lower_append_ch(backend, ' ');
// }
//
// fn void c_lower_append_space_margin(CBackend* backend, u32 times)
// {
//     c_lower_append_ch_repeated(backend, ' ', times * 4);
// }

// fn void c_lower_type(CBackend* backend, Thread* thread, TypeIndex type_index)
// {
//     Type* type = thread_type_get(thread, type_index);
//     switch (type->id)
//     {
//         case TYPE_INTEGER:
//             {
//                 u8 ch[] = { 'u', 's' };
//                 auto integer = &type->integer;
//                 u8 signedness_ch = ch[type->integer.is_signed];
//                 c_lower_append_ch(backend, signedness_ch);
//                 u8 upper_digit = integer->bit_count / 10;
//                 u8 lower_digit = integer->bit_count % 10;
//                 if (upper_digit)
//                 {
//                     c_lower_append_ch(backend, upper_digit + '0');
//                 }
//                 c_lower_append_ch(backend, lower_digit + '0');
//             } break;
//         default:
//             trap();
//     }
// }

// fn void c_lower_node(CBackend* backend, Thread* thread, NodeIndex node_index)
// {
//     unused(backend);
//     auto* node = thread_node_get(thread, node_index);
//     unused(node);
//     // auto* type = thread_type_get(thread, node->type);
//     // auto inputs = node_get_inputs(thread, node);
//     //
//     // switch (node->id)
//     // {
//     //     case IR_CONSTANT:
//     //         {
//     //             switch (type->id)
//     //             {
//     //                 case TYPE_INTEGER:
//     //                     {
//     //                         assert(type->integer.bit_count == 0);
//     //                         assert(type->integer.is_constant);
//     //                         assert(!type->integer.is_signed);
//     //                         vb_generic_ensure_capacity(&backend->buffer, 1, 64);
//     //                         auto current_length = backend->buffer.length;
//     //                         auto buffer_slice = (String){ .pointer = backend->buffer.pointer + current_length, .length = backend->buffer.capacity - current_length, };
//     //                         auto written_characters = format_hexadecimal(buffer_slice, type->integer.constant);
//     //                         backend->buffer.length = current_length + written_characters;
//     //                     } break;
//     //                     trap();
//     //                 default:
//     //                     trap();
//     //             }
//     //         } break;
//     //     case IR_INTEGER_SUBSTRACT:
//     //         {
//     //             auto left = inputs.pointer[1];
//     //             auto right = inputs.pointer[2];
//     //             c_lower_node(backend, thread, left);
//     //             c_lower_append_string(backend, strlit(" - "));
//     //             c_lower_node(backend, thread, right);
//     //         } break;
//     //     case IR_INTEGER_COMPARE_EQUAL:
//     //         {
//     //             auto left = inputs.pointer[1];
//     //             auto right = inputs.pointer[2];
//     //             c_lower_node(backend, thread, left);
//     //             c_lower_append_string(backend, strlit(" == "));
//     //             c_lower_node(backend, thread, right);
//     //         } break;
//     //     case IR_INTEGER_COMPARE_NOT_EQUAL:
//     //         {
//     //             auto left = inputs.pointer[1];
//     //             auto right = inputs.pointer[2];
//     //             c_lower_node(backend, thread, left);
//     //             c_lower_append_string(backend, strlit(" != "));
//     //             c_lower_node(backend, thread, right);
//     //         } break;
//     //     // case IR_PROJECTION:
//     //     //     {
//     //     //         auto projected_node_index = inputs.pointer[0];
//     //     //         auto projection_index = node->projection.index;
//     //     //
//     //     //         if (index_equal(projected_node_index, backend->function->start))
//     //     //         {
//     //     //             if (projection_index == 0)
//     //     //             {
//     //     //                 fail();
//     //     //             }
//     //     //             // if (projection_index > interpreter->arguments.length + 1)
//     //     //             // {
//     //     //             //     fail();
//     //     //             // }
//     //     //
//     //     //             switch (projection_index)
//     //     //             {
//     //     //                 case 1:
//     //     //                     c_lower_append_string(backend, strlit("argc"));
//     //     //                     break;
//     //     //                     // return interpreter->arguments.length;
//     //     //                 case 2:
//     //     //                     trap();
//     //     //                 default:
//     //     //                     trap();
//     //     //             }
//     //     //         }
//     //     //         else
//     //     //         {
//     //     //         trap();
//     //     //         }
//     //     //     } break;
//     //     default:
//     //         trap();
//     // }
//     todo();
// }

// fn String c_lower(Thread* thread)
// {
//     CBackend backend_stack = {};
//     CBackend* backend = &backend_stack;
//     auto program_epilogue = strlit("#include <stdint.h>\n"
//             "typedef uint8_t u8;\n"
//             "typedef uint16_t u16;\n"
//             "typedef uint32_t u32;\n"
//             "typedef uint64_t u64;\n"
//             "typedef int8_t s8;\n"
//             "typedef int16_t s16;\n"
//             "typedef int32_t s32;\n"
//             "typedef int64_t s64;\n"
//             );
//     c_lower_append_string(backend, program_epilogue);
//
//     for (u32 function_i = 0; function_i < thread->buffer.functions.length; function_i += 1)
//     {
//         auto* function = &thread->buffer.functions.pointer[function_i];
//         backend->function = function;
//         c_lower_type(backend, thread, function->return_type);
//         c_lower_append_space(backend);
//
//         c_lower_append_string(backend, function->name);
//         c_lower_append_ch(backend, argument_start);
//         if (s_equal(function->name, strlit("main")))
//         {
//             c_lower_append_string(backend, strlit("int argc, char* argv[]"));
//         }
//                 
//         c_lower_append_ch(backend, argument_end);
//         c_lower_append_ch(backend, '\n');
//         c_lower_append_ch(backend, block_start);
//         c_lower_append_ch(backend, '\n');
//
//         auto start_node_index = function->start;
//         auto* start_node = thread_node_get(thread, start_node_index);
//         assert(start_node->output_count > 0);
//         auto stop_node_index = function->stop;
//
//         auto proj_node_index = node_output_get(thread, start_node, 1);
//         auto it_node_index = proj_node_index;
//         u32 current_statement_margin = 1;
//
//         while (!index_equal(it_node_index, stop_node_index))
//         {
//             auto* it_node = thread_node_get(thread, it_node_index);
//             auto outputs = node_get_outputs(thread, it_node);
//             auto inputs = node_get_inputs(thread, it_node);
//
//             switch (it_node->id)
//             {
//                 // case IR_CONTROL_PROJECTION:
//                 //     break;
//                 case IR_RETURN:
//                     {
//                         c_lower_append_space_margin(backend, current_statement_margin);
//                         c_lower_append_string(backend, strlit("return "));
//                         assert(inputs.length > 1);
//                         assert(inputs.length == 2);
//                         auto input = inputs.pointer[1];
//                         c_lower_node(backend, thread, input);
//                         c_lower_append_ch(backend, ';');
//                         c_lower_append_ch(backend, '\n');
//                     } break;
//                 // case IR_STOP:
//                 //     break;
//                 default:
//                     todo();
//             }
//
//             assert(outputs.length == 1);
//             it_node_index = outputs.pointer[0];
//         }
//
//         c_lower_append_ch(backend, block_end);
//     }
//
//     return (String) { .pointer = backend->buffer.pointer, .length = backend->buffer.length };
// }

declare_ip_functions(DebugType, debug_type)

fn void thread_init(Thread* thread)
{
    memset(thread, 0, sizeof(Thread));
    thread->arena = arena_init_default(KB(64));
    thread->main_function = -1;

    // This assertion is here to make the pertinent changes in the reserve syscall

    // UINT32_MAX so they can be indexed via an unsigned integer of 32 bits
    const u64 offsets[] = {
        align_forward(sizeof(Type) * UINT32_MAX, page_size),
        align_forward(sizeof(Node) * UINT32_MAX, page_size),
        align_forward(sizeof(DebugType) * UINT32_MAX, page_size),
        align_forward(sizeof(NodeIndex) * UINT32_MAX, page_size),
        align_forward(sizeof(ArrayReference) * UINT32_MAX, page_size),
        align_forward(sizeof(Function) * UINT32_MAX, page_size),
        align_forward(sizeof(u8) * UINT32_MAX, page_size),
        align_forward(sizeof(RegisterMask) * UINT32_MAX, page_size),
    };
    static_assert(sizeof(thread->buffer) / sizeof(VirtualBuffer(u8)) == array_length(offsets));

    // Compute the total size (this is optimized out into a constant
    u64 total_size = 0;
    for (u32 i = 0; i < array_length(offsets); i += 1)
    {
        total_size += offsets[i];
    }

    // Actually make the syscall
    auto* ptr = reserve(total_size);
    assert(ptr);

    auto* buffer_it = (VirtualBuffer(u8)*)&thread->buffer;
    for (u32 i = 0; i < array_length(offsets); i += 1)
    {
        buffer_it->pointer = ptr;
        ptr += offsets[i];
    }

    DebugType integer_type;
    memset(&integer_type, 0, sizeof(u8));
    auto* it = &thread->types.debug.integer.array[0];

    for (u8 signedness = 0; signedness <= 1; signedness += 1)
    {
        integer_type.integer.signedness = signedness;

        for (u8 bit_count = 8; bit_count <= 64; bit_count *= 2, it += 1)
        {
            integer_type.integer.bit_count = bit_count;
            auto put_result = ip_DebugType_get_or_put_new(&thread->interned.debug_types, thread, &integer_type);
            assert(!put_result.existing);
            assert(validi(put_result.index));
            *it = put_result.index;
        }
    }

    // Type top, bot, live_control, dead_control;
    // memset(&top, 0, sizeof(Type));
    // top.id = TYPE_TOP;
    // memset(&bot, 0, sizeof(Type));
    // bot.id = TYPE_BOTTOM;
    // memset(&live_control, 0, sizeof(Type));
    // live_control.id = TYPE_LIVE_CONTROL;
    // memset(&dead_control, 0, sizeof(Type));
    // dead_control.id = TYPE_DEAD_CONTROL;
    //
    // thread->types.top = intern_pool_get_or_put_new_type(thread, &top).index;
    // thread->types.bottom = intern_pool_get_or_put_new_type(thread, &bot).index;
    // thread->types.live_control = intern_pool_get_or_put_new_type(thread, &live_control).index;
    // thread->types.dead_control = intern_pool_get_or_put_new_type(thread, &dead_control).index;
    //
    // thread->types.integer.top = thread_get_integer_type(thread, (TypeInteger) {
    //     .constant = 0,
    //     .is_constant = 0,
    //     .is_signed = 0,
    //     .bit_count = 0,
    // });
    // thread->types.integer.bottom = thread_get_integer_type(thread, (TypeInteger) {
    //     .constant = 1,
    //     .is_constant = 0,
    //     .is_signed = 0,
    //     .bit_count = 0,
    // });
    // thread->types.integer.zero = thread_get_integer_type(thread, (TypeInteger) {
    //     .constant = 0,
    //     .is_constant = 1,
    //     .is_signed = 0,
    //     .bit_count = 0,
    // });
    // thread->types.integer.u8 = thread_get_integer_type(thread, (TypeInteger) {
    //     .constant = 0,
    //     .is_constant = 0,
    //     .is_signed = 0,
    //     .bit_count = 8,
    // });
    // thread->types.integer.u16 = thread_get_integer_type(thread, (TypeInteger) {
    //     .constant = 0,
    //     .is_constant = 0,
    //     .is_signed = 0,
    //     .bit_count = 16,
    // });
    // thread->types.integer.u32 = thread_get_integer_type(thread, (TypeInteger) {
    //     .constant = 0,
    //     .is_constant = 0,
    //     .is_signed = 0,
    //     .bit_count = 32,
    // });
    // thread->types.integer.u64 = thread_get_integer_type(thread, (TypeInteger) {
    //     .constant = 0,
    //     .is_constant = 0,
    //     .is_signed = 0,
    //     .bit_count = 64,
    // });
    // thread->types.integer.s8 = thread_get_integer_type(thread, (TypeInteger) {
    //     .constant = 0,
    //     .is_constant = 0,
    //     .is_signed = 1,
    //     .bit_count = 8,
    // });
    // thread->types.integer.s16 = thread_get_integer_type(thread, (TypeInteger) {
    //     .constant = 0,
    //     .is_constant = 0,
    //     .is_signed = 1,
    //     .bit_count = 16,
    // });
    // thread->types.integer.s32 = thread_get_integer_type(thread, (TypeInteger) {
    //     .constant = 0,
    //     .is_constant = 0,
    //     .is_signed = 1,
    //     .bit_count = 32,
    // });
    // thread->types.integer.s64 = thread_get_integer_type(thread, (TypeInteger) {
    //     .constant = 0,
    //     .is_constant = 0,
    //     .is_signed = 1,
    //     .bit_count = 64,
    // });

    *vb_add(&thread->buffer.register_masks, 1) = (RegisterMask) {
        .class = 1,
        .may_spill = 0,
        .mask = 0,
    };
    *vb_add(&thread->buffer.register_masks, 1) = (RegisterMask) {
        .class = REGISTER_CLASS_X86_64_GPR,
        .may_spill = 0,
        .mask = ((u16)0xffff & ~((u16)1 << RSP)), // & ~((u16)1 << RBP),
    };

// global RegisterMask register_masks[] = {
//     {
//     },
//     {
//     },
// };

}

fn void thread_clear(Thread* thread)
{
    arena_reset(thread->arena);
}

#define DO_UNIT_TESTS 1
#if DO_UNIT_TESTS
fn void unit_tests()
{
    for (u64 power = 1, log2_i = 0; log2_i < 64; power <<= 1, log2_i += 1)
    {
        assert(log2_alignment(power) == log2_i);
    }
}
#endif

Slice(String) arguments;

typedef enum CompilerBackend : u8
{
    COMPILER_BACKEND_C = 'c',
    COMPILER_BACKEND_INTERPRETER = 'i',
    COMPILER_BACKEND_MACHINE = 'm',
} CompilerBackend;

STRUCT(ObjectOptions)
{
    char* object_path;
    char* exe_path;
    Slice(u8) code;
    u64 dynamic:1;
    u64 reserved:63;
};

STRUCT(ELFBuilder)
{
    VirtualBuffer(u8) file;
    VirtualBuffer(ELFSectionHeader) section_headers;
    VirtualBuffer(u8) section_string_table;
    VirtualBuffer(ElfProgramHeader) program_headers;
    SymbolTable static_st;
    SymbolTable dynamic_st;
};

// fn void elf_ph_init(ELFBuilder* restrict builder)
// {
//     builder->program_headers.length = 6;
//     auto program_header_size = sizeof(ElfProgramHeader) * builder->program_headers.length;
//     builder->program_headers.pointer = (ElfProgramHeader*)vb_add(&builder->file, program_header_size);
//     builder->program_header_i = 0;
// }
//
// fn void elf_ph_end(ELFBuilder* restrict builder)
// {
//     assert(builder->program_header_i == builder->program_headers.length);
// }

STRUCT(ELFSectionCreate)
{
    String name;
    ELFSectionType type;
    ELFSectionHeaderFlags flags;
    u32 link;
    u32 info;
    u64 size;
    u64 alignment;
    u64 entry_size;
};

fn void vb_align(VirtualBuffer(u8)* buffer, u64 alignment)
{
    auto current_length = buffer->length;
    auto target_len = align_forward(current_length, alignment);
    auto count = cast(u32, u64, target_len - current_length);
    auto* pointer = vb_add(buffer, count);
    memset(pointer, 0, count);
}

// fn void elf_add_program_header_raw(ELFBuilder* restrict builder, ElfProgramHeader ph)
// {
//     assert(builder->program_header_i < builder->program_headers.length);
//     auto* program_header = &builder->program_headers.pointer[builder->program_header_i];
//     *program_header = ph;
//     builder->program_header_i += 1;
// }

fn u64 elf_add_section_no_copy_raw(ELFBuilder* restrict builder, ELFSectionCreate create)
{
    assert(create.size > 0);
    assert(create.name.pointer);
    assert(create.name.length);
    assert((builder->file.length & (create.alignment - 1)) == 0);

    auto name_offset = builder->section_string_table.length;
    {
        memcpy(vb_add(&builder->section_string_table, create.name.length), create.name.pointer, create.name.length);
        *vb_add(&builder->section_string_table, 1) = 0;
    }

    auto offset = builder->file.length;

    auto sh = (ELFSectionHeader) {
        .name_offset = name_offset,
        .type = create.type,
        .flags = create.flags,
        .address = offset,
        .offset = offset,
        .size = create.size,
        .link = create.link,
        .info = create.info,
        .alignment = create.alignment,
        .entry_size = create.entry_size,
    };

    *vb_add(&builder->section_headers, 1) = sh;

    return offset;
}

fn u64 elf_add_section_no_copy(ELFBuilder* restrict builder, ELFSectionCreate create)
{
    assert(create.size > 0);
    vb_align(&builder->file, create.alignment);
    return elf_add_section_no_copy_raw(builder, create);
}

fn u64 elf_add_section_copy(ELFBuilder* restrict builder, ELFSectionCreate create, String content)
{
    assert(create.size == 0);
    assert(content.length > 0);
    create.size = content.length;
    auto result = elf_add_section_no_copy(builder, create);

    memcpy(vb_add(&builder->file, content.length), content.pointer, content.length);

    return result;
}

STRUCT(ELFSegmentCreate)
{
    ElfProgramHeaderType type;
    ElfProgramHeaderFlags flags;
    u64 size;
    u64 alignment;
    u64 offset;
};

// fn void elf_add_program_segment_no_copy_raw(ELFBuilder* restrict builder, ELFSegmentCreate create)
// {
//     assert((create.offset & (create.alignment - 1)) == 0);
//
//     elf_add_program_header_raw(builder, (ElfProgramHeader){
//         .type = create.type,
//         .flags = create.flags,
//         .offset = create.offset,
//         .virtual_address = create.offset,
//         .physical_address = create.offset,
//         .file_size = create.size,
//         .memory_size = create.size,
//         .alignment = create.alignment,
//     });
// }

fn void elf_fill_program_header(ElfProgramHeader* restrict ph, ELFSegmentCreate create)
{
    assert((create.offset & (create.alignment - 1)) == 0);
    *ph = (ElfProgramHeader){
        .type = create.type,
        .flags = create.flags,
        .offset = create.offset,
        .virtual_address = create.offset,
        .physical_address = create.offset,
        .file_size = create.size,
        .memory_size = create.size,
        .alignment = create.alignment,
    };
}

STRUCT(ELFSegmentSectionCreate)
{
    ELFSectionCreate section;
    ElfProgramHeaderType ph_type;
    ElfProgramHeaderFlags ph_flags;
    String content;
};

// fn u64 elf_add_both_segment_and_section(ELFBuilder* restrict builder, ELFSegmentSectionCreate create)
// {
//     assert(create.section.size == 0);
//     assert(create.content.length > 0);
//     create.section.size = create.content.length;
//     vb_align(&builder->file, create.section.alignment);
//     auto offset = elf_add_section_no_copy_raw(builder, create.section);
//     ELFSegmentCreate segment = {
//         .type = create.ph_type,
//         .flags = create.ph_flags,
//         .size = create.section.size,
//         .alignment = create.section.alignment, 
//         .offset = offset,
//     };
//     elf_add_program_segment_no_copy_raw(builder, segment);
//     memcpy(vb_add(&builder->file, create.content.length), create.content.pointer, create.content.length);
//
//     return offset;
// }

typedef enum SymbolKind : u8
{
    SYMBOL_TABLE_STATIC,
    SYMBOL_TABLE_DYNAMIC,
} SymbolKind;

// fn void st_add_symbol_raw(SymbolTable* restrict st, ELFSymbol symbol)
// {
//     *vb_add(&st->symbol_table, 1) = symbol;
// }
//
// fn void st_add_symbol(SymbolTable* restrict st, ELFSymbol symbol, String name)
// {
//     symbol.name_offset = st_add_string(st, name);
//
//     st_add_symbol_raw(st, symbol);
// }

fn void st_init(SymbolTable* restrict section)
{
    *vb_add(&section->string_table, 1) = 0;
    *vb_add(&section->symbol_table, 1) = (ELFSymbol) {};
}

// fn u32 elf_builder_add_string(ELFBuilder* builder, SymbolType symbol_type, String string)
// {
//     u32 name_offset = 0;
//     if (string.length)
//     {
//         switch (symbol_type)
//         {
//             case SYMBOL_TABLE_STATIC:
//                 {
//                     name_offset = builder->string_table.length;
//                     vb_append_bytes(&builder->string_table, string);
//                     *vb_add(&builder->string_table, 1) = 0;
//                 } break;
//             case SYMBOL_TABLE_DYNAMIC:
//                 {
//                     name_offset = builder->dynamic_string_table.length;
//                     vb_append_bytes(&builder->dynamic_string_table, string);
//                     *vb_add(&builder->dynamic_string_table, 1) = 0;
//                 } break;
//         }
//     }
//
//     return name_offset;
// }

// fn void elf_builder_add_symbol(ELFBuilder* builder, ELFSymbol symbol, SymbolType symbol_type, String string)
// {
//     symbol.name_offset = elf_builder_add_string(builder, symbol_type, string);
//     switch (symbol_type)
//     {
//         case SYMBOL_TABLE_STATIC:
//             {
//                 *vb_add(&builder->symbol_table, 1) = symbol;
//             } break;
//         case SYMBOL_TABLE_DYNAMIC:
//             {
//                 *vb_add(&builder->dynamic_symbol_table, 1) = symbol;
//             } break;
//     }
// }


// fn ELFSectionHeader* elf_builder_add_section(ELFBuilder* builder, ELFSectionHeader section, String section_name, Slice(u8) content)
// {
// }

// TODO: delete this. This was used in order to learn about ELF
    // {
    //     auto main_c_content = strlit("int main()\n{\n    return 0;\n}");
    //     int fd = syscall_open("main.c", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    //     assert(fd != -1);
    //     auto result = syscall_write(fd, main_c_content.pointer, main_c_content.length);
    //     assert(result >= 0);
    //     assert((u64)result == main_c_content.length);
    //     syscall_close(fd);
    // }

    // {
    //     char* command[] = {
    //         clang_path,
    //         "-c",
    //         "main.c",
    //         "-o",
    //         "main.o",
    //         "-Oz",
    //         "-fno-exceptions",
    //         "-fno-asynchronous-unwind-tables",
    //         "-fno-addrsig",
    //         "-fno-stack-protector",
    //         "-fno-ident",
    //         0,
    //     };
    //     run_command((CStringSlice) array_to_slice(command), envp);
    // }
    //
    // {
    //     char* command[] = {
    //         "/usr/bin/objcopy",
    //         "--remove-section",
    //         ".note.GNU-stack",
    //         "main.o",
    //         "main2.o",
    //         0,
    //     };
    //     run_command((CStringSlice) array_to_slice(command), envp);
    // }
    //
    // {
    //
    //     main_o = file_read(thread->arena, strlit("main2.o"));
    //     auto r1 = syscall_unlink("main.o");
    //     assert(!r1);
    //     auto r2 = syscall_unlink("main2.o");
    //     assert(!r2);
    //     auto r3 = syscall_unlink("main.c");
    //     assert(!r3);
    // }

typedef enum SymbolTableKind
{
    SYMBOL_TABLE_KIND_STATIC,
    SYMBOL_TABLE_KIND_DYNAMIC,
} SymbolTableKind;

STRUCT(SymbolTableOutput)
{
    u64 symbol_table_offset;
    u64 string_table_offset;
};

fn SymbolTableOutput emit_symbol_table(ELFBuilder* restrict builder, SymbolTable* st, SymbolTableKind kind)
{
    SymbolTableOutput result = {};

    if (st->symbol_table.length > 0)
    {
        String symbol_table_name;
        String string_table_name;
        ELFSectionType type;

        switch (kind)
        {
            case SYMBOL_TABLE_KIND_STATIC:
                symbol_table_name = strlit(".symtab");
                string_table_name = strlit(".strtab");
                type = ELF_SECTION_SYMBOL_TABLE;
                break;
            case SYMBOL_TABLE_KIND_DYNAMIC:
                symbol_table_name = strlit(".dynsym");
                string_table_name = strlit(".dynstr");
                type = ELF_SECTION_DYNAMIC_SYMBOL_TABLE;
                break;
        }

        result.symbol_table_offset = elf_add_section_copy(builder, (ELFSectionCreate){
            .name = symbol_table_name,
            .type = type,
            .flags = {},
            .link = builder->section_headers.length + 1, // TODO: figure out
            .info = 4, // TODO: figure out
            .alignment = alignof(ELFSymbol),
            .entry_size = sizeof(ELFSymbol),
        }, (String) { .pointer = (u8*)st->symbol_table.pointer, .length = sizeof(*st->symbol_table.pointer) * st->symbol_table.length, });

        result.string_table_offset = elf_add_section_copy(builder, (ELFSectionCreate){
            .name = string_table_name,
            .type = ELF_SECTION_STRING_TABLE,
            .flags = {},
            .link = 0,
            .info = 0,
            .alignment = 1,
            .entry_size = 0,
        }, (String) { .pointer = st->string_table.pointer, .length = sizeof(*st->string_table.pointer) * st->string_table.length, });
    }

    return result;
}

fn u32 elf_get_string(VirtualBuffer(u8)* restrict buffer, String string)
{
    assert(buffer->length > 0);

    if (string.length == 0)
    {
        return 0;
    }

    u32 i = 0;

    while (i < buffer->length)
    {
        auto* ptr = &buffer->pointer[i];
        auto length = strlen((char*)ptr);
        auto existing = (String) {
            .pointer = ptr,
            .length = length,
        };

        while (existing.length)
        {
            if (s_equal(existing, string))
            {
                return cast(u32, s64, existing.pointer - buffer->pointer);
            }

            existing.pointer += 1;
            existing.length -= 1;
        }

        i += length + 1;
    }

    auto length = buffer->length;
    auto* ptr = vb_add(buffer, string.length + 1);
    memcpy(ptr, string.pointer, string.length);
    *(ptr + string.length) = 0;

    return length;
}

fn u32 st_get_string(SymbolTable* restrict section, String string)
{
    return elf_get_string(&section->string_table, string);
}

fn u32 elf_get_section_name(ELFBuilder* builder, String name)
{
    return elf_get_string(&builder->section_string_table, name);
}

typedef enum ELFNoteType : u32
{
    NT_GNU_ABI_TAG = 1,
    NT_GNU_HWCAP = 2,
    NT_GNU_BUILD_ID = 3,
    NT_GNU_GOLD_VERSION = 4,
    NT_GNU_PROPERTY_TYPE_0 = 5,
} ELFNoteType;

STRUCT(ELFNoteHeader)
{
    u32 name_size;
    u32 descriptor_size;
    ELFNoteType type;
};
static_assert(sizeof(ELFNoteHeader) == 12);

STRUCT(ELFVersionDefinition)
{
    u16 version;
    u16 flags;
    u16 index;
    u16 count;
    u32 hash;
    u32 aux;
    u32 next;
};
static_assert(sizeof(ELFVersionDefinition) == 20);

#define elf_eh_frame_absptr 0x00
#define elf_eh_frame_udata4 0x03
#define elf_eh_frame_sdata4 0x0b
#define elf_eh_frame_pcrel 0x10
#define elf_eh_frame_datarel 0x30

STRUCT(EhFrameHeader)
{
    u8 version;
    u8 pointer_encoding;
    u8 count_encoding;
    u8 table_encoding;
    u32 frame_start;
    u32 entry_count;
};

STRUCT(EhFrameHeaderEntry)
{
    u32 pc;
    u32 fde;
};


STRUCT(Uleb128)
{
    u64 number;
    u64 i;
};

fn Uleb128 uleb128_decode(String input)
{
    assert(input.length);
    Uleb128 result = {};
    u16 shift = 0;

    while (result.i < input.length)
    {
        auto byte = input.pointer[result.i];
        assert(shift < 64);
        result.number |= (u64)(byte & 0b01111111) << shift;
        shift += 7;
        result.i += 1;

        if ((byte & 0b10000000) == 0)
        {
            break;
        }
    }

    return result;
}


fn void sleb128_encode(VirtualBuffer(u8)* buffer, s32 value)
{
    auto extra_bits = (u32)(value ^ (value >> 31)) >> 6;
    u8 out = value & 0b01111111;
    while (extra_bits)
    {
        *vb_add(buffer, 1) = out | 0x80;
        value >>= 7;
        out = value & 0b01111111;
        extra_bits >>= 7;
    }
    *vb_add(buffer, 1) = out;
}

fn void uleb128_encode(VirtualBuffer(u8)* buffer, u32 value)
{
    u8 out = value & 0b01111111;
    value >>= 7;
    while (value)
    {
        *vb_add(buffer, 1) = out | 0x80;
        out = value & 0b01111111;
        value >>= 7;
    }
    *vb_add(buffer, 1) = out;
}


may_be_unused fn void write_elf(Thread* thread, const ObjectOptions* const restrict options, char** envp)
{
    unused(thread);

    ELFBuilder builder_stack = {};
    ELFBuilder* restrict builder = &builder_stack;
    // Initialization
    {
        if (options->dynamic)
        {
            st_init(&builder->dynamic_st);
        }

        st_init(&builder->static_st);
        // Init section table
        *vb_add(&builder->section_string_table, 1) = 0;
        *vb_add(&builder->section_headers, 1) = (ELFSectionHeader){};
    }

    auto symtab_section_name = elf_get_section_name(builder, strlit(".symtab"));
    auto strtab_section_name = elf_get_section_name(builder, strlit(".strtab"));
    auto shstrtab_section_name = elf_get_section_name(builder, strlit(".shstrtab"));

    auto* elf_header = vb_add_struct(&builder->file, ELFHeader);

    // TODO: precompute properly how many program segments we are going to need
    u16 program_header_count = 13;
    auto* program_headers = vb_add(&builder->file, sizeof(ElfProgramHeader) * program_header_count);

    {
        // Add program header segment
        auto offset = sizeof(ELFHeader);
        auto size = sizeof(ElfProgramHeader) * program_header_count;
        *vb_add(&builder->program_headers, 1) = (ElfProgramHeader) {
            .type = PT_PHDR,
            .flags = { .readable = 1 },
            .offset = offset,
            .virtual_address = offset,
            .physical_address = offset,
            .file_size = size,
            .memory_size = size,
            .alignment = alignof(ElfProgramHeader),
        };
    }

    {
        // .interp
        // Section #1
        auto* section_header = vb_add(&builder->section_headers, 1);
        auto interp_section_name = elf_get_section_name(builder, strlit(".interp"));

        u64 alignment = 1;
        vb_align(&builder->file, alignment);
        auto offset = builder->file.length;

        auto content = strlit("/lib64/ld-linux-x86-64.so.2");
        auto size = content.length + 1;
        memcpy(vb_add(&builder->file, size), content.pointer, size);


        *section_header = (ELFSectionHeader)
        {
            .name_offset = interp_section_name,
                .type = ELF_SECTION_PROGRAM,
                .flags = {
                    .alloc = 1,
                },
                .address = offset,
                .offset = offset,
                .size = size,
                .link = 0,
                .info = 0,
                .alignment = alignment,
                .entry_size = 0,
        };

        *vb_add(&builder->program_headers, 1) = (ElfProgramHeader) {
            .type = PT_INTERP,
            .flags = {.readable = 1,},
            .offset = offset,
            .virtual_address = offset,
            .physical_address = offset,
            .file_size = size,
            .memory_size = size,
            .alignment = alignment,
        };
    }

    u32 gnu_property_offset = 0;
    u32 gnu_property_size = 0;
    u32 gnu_property_alignment = 0;
    {
        // .note.gnu.property
        // Section #2
        auto* gnu_property_section_header = vb_add(&builder->section_headers, 1);
        u64 alignment = 8;
        gnu_property_alignment = alignment;
        vb_align(&builder->file, alignment);
        auto offset = builder->file.length;
        gnu_property_offset = offset;
        auto gnu_property_section_name = elf_get_section_name(builder, strlit(".note.gnu.property"));

        auto gnu_string = strlit("GNU");
        auto gnu_string_size = gnu_string.length + 1;
        auto* ptr = vb_add_struct(&builder->file, ELFNoteHeader);
        *ptr = (ELFNoteHeader)
        {
            .name_size = gnu_string_size,
            .descriptor_size = 16,
            .type = NT_GNU_PROPERTY_TYPE_0,
        };
        memcpy(vb_add(&builder->file, gnu_string_size), gnu_string.pointer, gnu_string_size);
        u8 gnu_property_blob[] = { 0x02, 0x80, 0x00, 0xC0, 0x04, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, };
        static_assert(array_length(gnu_property_blob) == 16);

        memcpy(vb_add(&builder->file, sizeof(gnu_property_blob)), gnu_property_blob, sizeof(gnu_property_blob));

        gnu_property_size = builder->file.length - offset;

        *gnu_property_section_header = (ELFSectionHeader)
        {
            .name_offset = gnu_property_section_name,
            .type = ELF_SECTION_NOTE,
            .flags = {
                .alloc = 1,
            },
            .address = offset,
            .offset = offset,
            .size = gnu_property_size,
            .link = 0,
            .info = 0,
            .alignment = alignment,
            .entry_size = 0,
        };
    }

    u32 gnu_build_id_offset = 0;
    u32 gnu_build_id_alignment = 0;
    {
        // .note.gnu.build-id
        // Section #3
        auto* section_header = vb_add(&builder->section_headers, 1);
        u64 alignment = 4;
        gnu_build_id_alignment = alignment;
        auto name = elf_get_section_name(builder, strlit(".note.gnu.build-id"));
        vb_align(&builder->file, alignment);
        auto offset = builder->file.length;
        gnu_build_id_offset = offset;

        auto gnu_string = strlit("GNU");
        auto gnu_string_size = gnu_string.length + 1;

        u8 blob[] = { 0xF0, 0xDE, 0x30, 0xDB, 0x5A, 0x9F, 0xE2, 0x4E, 0x0A, 0xCF, 0x93, 0xA4, 0x4C, 0x11, 0x60, 0xB5, 0x0C, 0xD9, 0xAF, 0x50, };
        static_assert(sizeof(blob) == 20);

        *(vb_add_struct(&builder->file, ELFNoteHeader)) = (ELFNoteHeader)
        {
            .name_size = gnu_string_size,
            .descriptor_size = sizeof(blob),
            .type = NT_GNU_BUILD_ID,
        };
        memcpy(vb_add(&builder->file, gnu_string_size), gnu_string.pointer, gnu_string_size);
        memcpy(vb_add(&builder->file, sizeof(blob)), blob, sizeof(blob));

        auto size = builder->file.length - offset;

        *section_header = (ELFSectionHeader)
        {
            .name_offset = name,
            .type = ELF_SECTION_NOTE,
            .flags = {
                .alloc = 1,
            },
            .address = offset,
            .offset = offset,
            .size = size,
            .link = 0,
            .info = 0,
            .alignment = alignment,
            .entry_size = 0,
        };
    }

    {
        // .note.ABI-tag
        // Section #4
        auto* section_header = vb_add(&builder->section_headers, 1);
        u64 alignment = 4;

        vb_align(&builder->file, alignment);
        auto offset = builder->file.length;

        auto name = elf_get_section_name(builder, strlit(".note.ABI-tag"));

        auto gnu_string = strlit("GNU");
        auto gnu_string_size = gnu_string.length + 1;
        *vb_add_struct(&builder->file, ELFNoteHeader) = (ELFNoteHeader) {
            .name_size = gnu_string_size,
            .descriptor_size = 16,
            .type = NT_GNU_ABI_TAG,
        };
        memcpy(vb_add(&builder->file, gnu_string_size), gnu_string.pointer, gnu_string_size);

        u32 abi = ELF_ABI_SYSTEM_V;
        u32 major = 4;
        u32 minor = 4;
        u32 patch = 0;
        u32 abi_content[] = { abi, major, minor, patch };

        memcpy(vb_add(&builder->file, sizeof(abi_content)), abi_content, sizeof(abi_content));

        auto size = builder->file.length - offset;

        *section_header = (ELFSectionHeader)
        {
            .name_offset = name,
            .type = ELF_SECTION_NOTE,
            .flags = {
                .alloc = 1,
            },
            .address = offset,
            .offset = offset,
            .size = size,
            .link = 0,
            .info = 0,
            .alignment = alignment,
            .entry_size = 0,
        };
    }

    auto gnu_build_id_abi_note_size = builder->file.length - gnu_build_id_offset;

    u16 preliminar_section_count = builder->section_headers.length + 1;
    auto dynamic_symbol_table_index = preliminar_section_count;
    auto dynamic_string_table_index = dynamic_symbol_table_index + 1;

    u32 gnu_hash_offset = 0;
    {
        // .gnu.hash
        // Section #5
        auto* section_header = vb_add(&builder->section_headers, 1);
        u64 alignment = 8;

        vb_align(&builder->file, alignment);
        auto offset = builder->file.length;
        gnu_hash_offset = offset;

        auto name = elf_get_section_name(builder, strlit(".gnu.hash"));

        *vb_add_struct(&builder->file, ElfGnuHashHeader) = (ElfGnuHashHeader) {
            .bucket_count = 1,
            .symbol_offset = 1,
            .bloom_size = 1,
            .bloom_shift = 0,
        };

        u64 bloom_filters[] = {0};
        memcpy(vb_add(&builder->file, sizeof(bloom_filters)), bloom_filters, sizeof(bloom_filters));

        u32 buckets[] = {0};
        memcpy(vb_add(&builder->file, sizeof(buckets)), buckets, sizeof(buckets));

        // u32 hash_chains[] = {0};
        // memcpy(vb_add(&builder->file, sizeof(hash_chains)), hash_chains, sizeof(hash_chains));

        auto size = builder->file.length - offset;


        *section_header = (ELFSectionHeader) {
            .name_offset = name,
            .type = ELF_SECTION_GNU_HASH,
            .flags = {
                .alloc = 1,
            },
            .address = offset,
            .offset = offset,
            .size = size,
            .link = dynamic_symbol_table_index,
            .info = 0,
            .alignment = alignment,
            .entry_size = 0,
        };
    }

    auto libc_start_main = st_get_string(&builder->dynamic_st, strlit("__libc_start_main"));
    auto cxa_finalize = st_get_string(&builder->dynamic_st, strlit("__cxa_finalize"));
    auto libcso6 = st_get_string(&builder->dynamic_st, strlit("libc.so.6"));
    auto glibc_225 = st_get_string(&builder->dynamic_st, strlit("GLIBC_2.2.5"));
    auto glibc_234 = st_get_string(&builder->dynamic_st, strlit("GLIBC_2.34"));
    auto itm_deregister = st_get_string(&builder->dynamic_st, strlit("_ITM_deregisterTMCloneTable"));
    auto gmon_start = st_get_string(&builder->dynamic_st, strlit("__gmon_start__"));
    auto itm_register = st_get_string(&builder->dynamic_st, strlit("_ITM_registerTMCloneTable"));

    u32 dynsym_offset = 0;
    {
        // .dynsym
        // Section #6
        auto* section_header = vb_add(&builder->section_headers, 1);
        u64 alignment = alignof(ELFSymbol);

        vb_align(&builder->file, alignment);
        auto offset = builder->file.length;
        dynsym_offset = offset;

        auto name = elf_get_section_name(builder, strlit(".dynsym"));

        ELFSymbol expected_symbols[] = {
            // 1
            {
                .name_offset = libc_start_main,
                .type = ELF_SYMBOL_TYPE_FUNCTION,
                .binding = ELF_SYMBOL_BINDING_GLOBAL,
                .visibility = ELF_SYMBOL_VISIBILITY_DEFAULT,
                .section_index = 0,
                .value = 0,
                .size = 0,
            },
            // 2
            {
                .name_offset = itm_deregister,
                .type = ELF_SYMBOL_TYPE_NONE,
                .binding = ELF_SYMBOL_BINDING_WEAK,
                .visibility = ELF_SYMBOL_VISIBILITY_DEFAULT,
                .section_index = 0,
                .value = 0,
                .size = 0,
            },
            // 3
            {
                .name_offset = gmon_start,
                .type = ELF_SYMBOL_TYPE_NONE,
                .binding = ELF_SYMBOL_BINDING_WEAK,
                .visibility = ELF_SYMBOL_VISIBILITY_DEFAULT,
                .section_index = 0,
                .value = 0,
                .size = 0,
            },
            // 4
            {
                .name_offset = itm_register,
                .type = ELF_SYMBOL_TYPE_NONE,
                .binding = ELF_SYMBOL_BINDING_WEAK,
                .visibility = ELF_SYMBOL_VISIBILITY_DEFAULT,
                .section_index = 0,
                .value = 0,
                .size = 0,
            },
            // 5
            {
                .name_offset = cxa_finalize,
                .type = ELF_SYMBOL_TYPE_FUNCTION,
                .binding = ELF_SYMBOL_BINDING_WEAK,
                .visibility = ELF_SYMBOL_VISIBILITY_DEFAULT,
                .section_index = 0,
                .value = 0,
                .size = 0,
            },
        };
        memcpy(vb_add(&builder->dynamic_st.symbol_table, array_length(expected_symbols)), expected_symbols, sizeof(expected_symbols));
        u64 size = builder->dynamic_st.symbol_table.length * sizeof(ELFSymbol);

        memcpy(vb_add(&builder->file, size), builder->dynamic_st.symbol_table.pointer, size);

        u64 entry_size = sizeof(ELFSymbol);

        *section_header = (ELFSectionHeader) {
            .name_offset = name,
            .type = ELF_SECTION_DYNAMIC_SYMBOL_TABLE,
            .flags = {
                .alloc = 1,
            },
            .address = offset,
            .offset = offset,
            .size = size,
            .link = dynamic_string_table_index,
            .info = 1, // TODO: figure out
            .alignment = alignment,
            .entry_size = entry_size,
        };
    }

    u32 dynstr_offset = 0;
    u32 dynstr_size = 0;
    {
        // .dynstr
        // Section #7
        auto* section_header = vb_add(&builder->section_headers, 1);
        u64 alignment = 1;

        vb_align(&builder->file, alignment);
        auto offset = builder->file.length;
        dynstr_offset = offset;

        auto name = elf_get_section_name(builder, strlit(".dynstr"));

        auto size = builder->dynamic_st.string_table.length;
        dynstr_size = size;
        memcpy(vb_add(&builder->file, size), builder->dynamic_st.string_table.pointer, size);

        *section_header = (ELFSectionHeader) {
            .name_offset = name,
            .type = ELF_SECTION_STRING_TABLE,
            .flags = {
                .alloc = 1,
            },
            .address = offset,
            .offset = offset,
            .size = size,
            .link = 0,
            .info = 0,
            .alignment = alignment,
            .entry_size = 0,
        };
    }

    u32 gnu_version_offset = 0;
    {
        // .gnu.version
        auto* section_header = vb_add(&builder->section_headers, 1);
        u64 alignment = alignof(u16);
        assert(alignment == 2);

        vb_align(&builder->file, alignment);
        auto offset = builder->file.length;
        gnu_version_offset = offset;

        auto name = elf_get_section_name(builder, strlit(".gnu.version"));

        // 0: means local symbol (not versioned)
        // 1: indicates base version (nothing specific)
        // >1: refers to an index into the .gnu.version_r
        u16 symbol_versions[] = {
            0,
            2, // .gnu.version_r
            1,
            1,
            1,
            3 // .gnu.version_r
        };

        auto size = sizeof(symbol_versions);

        memcpy(vb_add(&builder->file, size), symbol_versions, size);

        *section_header = (ELFSectionHeader) {
            .name_offset = name,
            .type = ELF_SECTION_GNU_VERSYM,
            .flags = {
                .alloc = 1,
            },
            .address = offset,
            .offset = offset,
            .size = size,
            .link = dynamic_symbol_table_index,
            .info = 0,
            .alignment = alignment,
            .entry_size = sizeof(u16),
        };
    }

    u32 gnu_version_r_offset = 0;
    {
        // .gnu.version_r
        auto* section_header = vb_add(&builder->section_headers, 1);
        u64 alignment = 8;

        vb_align(&builder->file, alignment);
        auto offset = builder->file.length;
        gnu_version_r_offset = offset;

        auto name = elf_get_section_name(builder, strlit(".gnu.version_r"));

        // TODO: figure out
        u8 blob[] = {
            0x01, 0x00, 0x02, 0x00, 0x22, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
            0x75, 0x1A, 0x69, 0x09, 0x00, 0x00, 0x03, 0x00, 0x2C, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 
            0xB4, 0x91, 0x96, 0x06, 0x00, 0x00, 0x02, 0x00, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        };

        auto size = sizeof(blob);

        memcpy(vb_add(&builder->file, size), blob, size);

        *section_header = (ELFSectionHeader) {
            .name_offset = name,
            .type = ELF_SECTION_GNU_VERNEED,
            .flags = {
                .alloc = 1,
            },
            .address = offset,
            .offset = offset,
            .size = size,
            .link = dynamic_string_table_index,
            .info = 1, // TODO: figure out
            .alignment = alignment,
            .entry_size = 0,
        };
    }

    ElfRelocationWithAddend* dynamic_relocations = {};
    // * The symbol indices make mention to the .dynsym table (where 0 means a special case: none)
    // * The addend is related to the (virtual) address
    // ElfRelocationWithAddend expected_relocations[] = {
    //     { .offset = 15888, .info = { .type = { .x86_64 = R_X86_64_RELATIVE }, .symbol = 0}, .addend = 4368 }, // .fini_array content in .text
    //     { .offset = 15896, .info = { .type = { .x86_64 = R_X86_64_RELATIVE }, .symbol = 0}, .addend = 4288 }, // .init_array content in .text
    //     { .offset = 16392, .info = { .type = { .x86_64 = R_X86_64_RELATIVE }, .symbol = 0}, .addend = 16392 }, // something in .data
    //
    //     { .offset = 16320, .info = { .type = { .x86_64 = R_X86_64_GLOB_DAT }, .symbol = 1}, .addend = 0 }, // libc_start_main
    //     { .offset = 16328, .info = { .type = { .x86_64 = R_X86_64_GLOB_DAT }, .symbol = 2}, .addend = 0 }, // itm_deregister
    //     { .offset = 16336, .info = { .type = { .x86_64 = R_X86_64_GLOB_DAT }, .symbol = 3}, .addend = 0 }, // gmon_start
    //     { .offset = 16344, .info = { .type = { .x86_64 = R_X86_64_GLOB_DAT }, .symbol = 4}, .addend = 0 }, // itm_register
    //     { .offset = 16352, .info = { .type = { .x86_64 = R_X86_64_GLOB_DAT }, .symbol = 5}, .addend = 0 }, // cxa_finalize
    // };

    auto expected_relocation_count = 8;
    auto relocation_index = 0;
    auto rela_count = 3;
    u32 rela_dyn_offset = 0;
    u32 rela_dyn_size = 0;
    {
        // .rela.dyn
        auto* section_header = vb_add(&builder->section_headers, 1);
        u64 alignment = alignof(ElfRelocationWithAddend);

        vb_align(&builder->file, alignment);
        auto offset = builder->file.length;
        rela_dyn_offset = offset;

        auto name = elf_get_section_name(builder, strlit(".rela.dyn"));

        auto size = sizeof(ElfRelocationWithAddend) * expected_relocation_count;
        rela_dyn_size = size;
        dynamic_relocations = (ElfRelocationWithAddend*)vb_add(&builder->file, size);

        *section_header = (ELFSectionHeader) {
            .name_offset = name,
            .type = ELF_SECTION_RELOCATION_WITH_ADDENDS,
            .flags = {
                .alloc = 1,
            },
            .address = offset,
            .offset = offset,
            .size = size,
            .link = dynamic_symbol_table_index,
            .info = 0,
            .alignment = alignment,
            .entry_size = sizeof(ElfRelocationWithAddend),
        };
    }

    // Add read-only program segment
    {
        auto offset = builder->file.length;
        assert(offset == 1496);

        *vb_add(&builder->program_headers, 1) = (ElfProgramHeader)
        {
            .type = PT_LOAD,
            .flags = {.readable = 1},
            .offset = 0,
            .virtual_address = 0,
            .physical_address = 0,
            .file_size = offset,
            .memory_size = offset,
            .alignment = 0x1000,
        };
    }

    vb_align(&builder->file, 0x1000);

    auto code_offset = builder->file.length;
    auto init_offset = code_offset;
    auto init_section_index = builder->section_headers.length;
    {
        // .init
        auto* section_header = vb_add(&builder->section_headers, 1);
        u64 alignment = 4;

        vb_align(&builder->file, alignment);
        auto offset = builder->file.length;

        auto name = elf_get_section_name(builder, strlit(".init"));

        u8 data[] = {
            0xF3, 0x0F, 0x1E, 0xFA, 0x48, 0x83, 0xEC, 0x08, 0x48, 0x8B, 0x05, 0xC1, 0x2F, 0x00, 0x00, 0x48, 
            0x85, 0xC0, 0x74, 0x02, 0xFF, 0xD0, 0x48, 0x83, 0xC4, 0x08, 0xC3, 
        };

        auto size = sizeof(data);

        memcpy(vb_add(&builder->file, size), data, size);

        *section_header = (ELFSectionHeader) {
            .name_offset = name,
            .type = ELF_SECTION_PROGRAM,
            .flags = {
                .alloc = 1,
                .executable = 1,
            },
            .address = offset,
            .offset = offset,
            .size = size,
            .link = 0,
            .info = 0,
            .alignment = alignment,
            .entry_size = 0,
        };
    }

    u32 text_init_array_offset = 0;
    u32 text_fini_array_offset = 0;
    u32 _start_offset = 0;
    u32 _start_size = 0;
    u32 main_offset = 0;
    u32 main_size = 0;
    auto text_section_index = builder->section_headers.length;
    {
        //.text
        auto* section_header = vb_add(&builder->section_headers, 1);
        u64 alignment = 16;

        vb_align(&builder->file, alignment);
        auto offset = builder->file.length;

        auto name = elf_get_section_name(builder, strlit(".text"));

        _start_offset = builder->file.length;
        {
            {
                {
                    u8 data[] = {
                        0xF3, 0x0F, 0x1E, 0xFA, 0x31, 0xED, 0x49, 0x89, 0xD1, 0x5E, 0x48, 0x89, 0xE2, 0x48, 0x83, 0xE4, 
                        0xF0, 0x50, 0x54, 0x45, 0x31, 0xC0, 0x31, 0xC9, 0x48, 0x8D, 0x3D, 0xDD, 0x00, 0x00, 0x00, 0xFF, 
                        0x15, 0x7B, 0x2F, 0x00, 0x00, 0xF4,
                    };
                    _start_size = sizeof(data);
                    memcpy(vb_add(&builder->file, sizeof(data)), data, sizeof(data));
                }
                // padding after _start (not counting for _start size)
                {
                    u8 data[] = {
                        0x66, 0x2E, 0x0F, 0x1F, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00, 
                    };
                    memcpy(vb_add(&builder->file, sizeof(data)), data, sizeof(data));
                }
            }
        }

        auto uk0_offset = builder->file.length;
        {
            u8 data[] = {
                0x48, 0x8D, 0x3D, 0xB9, 0x2F, 0x00, 0x00, 0x48, 0x8D, 0x05, 0xB2, 0x2F, 0x00, 0x00, 0x48, 0x39, 
                0xF8, 0x74, 0x15, 0x48, 0x8B, 0x05, 0x5E, 0x2F, 0x00, 0x00, 0x48, 0x85, 0xC0, 0x74, 0x09, 0xFF, 
                0xE0, 0x0F, 0x1F, 0x80, 0x00, 0x00, 0x00, 0x00, 0xC3, 0x0F, 0x1F, 0x80, 0x00, 0x00, 0x00, 0x00, 
            };
            memcpy(vb_add(&builder->file, sizeof(data)), data, sizeof(data));
        }

        auto uk1_offset = builder->file.length;
        {
            u8 data[] = {
                0x48, 0x8D, 0x3D, 0x89, 0x2F, 0x00, 0x00, 0x48, 0x8D, 0x35, 0x82, 0x2F, 0x00, 0x00, 0x48, 0x29, 
                0xFE, 0x48, 0x89, 0xF0, 0x48, 0xC1, 0xEE, 0x3F, 0x48, 0xC1, 0xF8, 0x03, 0x48, 0x01, 0xC6, 0x48, 
                0xD1, 0xFE, 0x74, 0x14, 0x48, 0x8B, 0x05, 0x2D, 0x2F, 0x00, 0x00, 0x48, 0x85, 0xC0, 0x74, 0x08, 
                0xFF, 0xE0, 0x66, 0x0F, 0x1F, 0x44, 0x00, 0x00, 0xC3, 0x0F, 0x1F, 0x80, 0x00, 0x00, 0x00, 0x00, 
            };
            memcpy(vb_add(&builder->file, sizeof(data)), data, sizeof(data));
        }

        text_fini_array_offset = builder->file.length;
        {
            u8 data[] = {
                0xF3, 0x0F, 0x1E, 0xFA, 0x80, 0x3D, 0x45, 0x2F, 0x00, 0x00, 0x00, 0x75, 0x33, 0x55, 0x48, 0x83, 
                0x3D, 0x0A, 0x2F, 0x00, 0x00, 0x00, 0x48, 0x89, 0xE5, 0x74, 0x0D, 0x48, 0x8B, 0x3D, 0x26, 0x2F, 
                0x00, 0x00, 0xFF, 0x15, 0xF8, 0x2E, 0x00, 0x00, 0xE8, 0x63, 0xFF, 0xFF, 0xFF, 0xC6, 0x05, 0x1C, 
                0x2F, 0x00, 0x00, 0x01, 0x5D, 0xC3, 0x66, 0x2E, 0x0F, 0x1F, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00, 
            };
            memcpy(vb_add(&builder->file, sizeof(data)), data, sizeof(data));
        }
        auto uk3_offset = builder->file.length;
        {
            u8 data[] = {
                0xC3, 0x66, 0x66, 0x2E, 0x0F, 0x1F, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x1F, 0x40, 0x00, 
            };
            memcpy(vb_add(&builder->file, sizeof(data)), data, sizeof(data));
        }
        text_init_array_offset = builder->file.length;
        {
            u8 data[] = {
                0xF3, 0x0F, 0x1E, 0xFA, 0xE9, 0x67, 0xFF, 0xFF, 0xFF, 0x0F, 0x1F, 0x00,
            };
            memcpy(vb_add(&builder->file, sizeof(data)), data, sizeof(data));
        }

        // TODO: fix this
        main_offset = builder->file.length;
        main_size = options->code.length;

        memcpy(vb_add(&builder->file, options->code.length), options->code.pointer, options->code.length);

        auto size = builder->file.length - offset;

        *section_header = (ELFSectionHeader) {
            .name_offset = name,
            .type = ELF_SECTION_PROGRAM,
            .flags = {
                .alloc = 1,
                .executable = 1,
            },
            .address = offset,
            .offset = offset,
            .size = size,
            .link = 0,
            .info = 0,
            .alignment = alignment,
            .entry_size = 0,
        };
    }

    u32 fini_offset = 0; 
    u32 fini_section_index = builder->section_headers.length;
    {
        // .fini
        auto* section_header = vb_add(&builder->section_headers, 1);
        u64 alignment = 4;

        vb_align(&builder->file, alignment);
        auto offset = builder->file.length;
        fini_offset = offset;

        auto name = elf_get_section_name(builder, strlit(".fini"));

        u8 data[] = { 0xF3, 0x0F, 0x1E, 0xFA, 0x48, 0x83, 0xEC, 0x08, 0x48, 0x83, 0xC4, 0x08, 0xC3, };

        auto size = sizeof(data);

        memcpy(vb_add(&builder->file, size), data, size);

        *section_header = (ELFSectionHeader) {
            .name_offset = name,
            .type = ELF_SECTION_PROGRAM,
            .flags = {
                .alloc = 1,
                .executable = 1,
            },
            .address = offset,
            .offset = offset,
            .size = size,
            .link = 0,
            .info = 0,
            .alignment = alignment,
            .entry_size = 0,
        };
    }

    // Add code segment (read, execute)
    {
        auto current_offset = builder->file.length;
        auto length = current_offset - code_offset;
        *vb_add(&builder->program_headers, 1) = (ElfProgramHeader)
        {
            .type = PT_LOAD,
            .flags = { .executable = 1, .readable = 1},
            .offset = code_offset,
            .virtual_address = code_offset,
            .physical_address = code_offset,
            .file_size = length,
            .memory_size = length,
            .alignment = 0x1000,
        };
    }

    vb_align(&builder->file, 0x1000);

    auto read_only_offset = builder->file.length;

    auto rodata_section_index = builder->section_headers.length;
    u32 _IO_stdin_used_size = 0;
    u32 rodata_va = 0;
    {
        // .rodata
        auto* section_header = vb_add(&builder->section_headers, 1);
        u64 alignment = 4;

        vb_align(&builder->file, alignment);
        auto offset = builder->file.length;
        rodata_va = offset;

        auto name = elf_get_section_name(builder, strlit(".rodata"));

        u32 _IO_stdin_used = 0x20001;
        u32 data[] = {0x20001};
        _IO_stdin_used_size = sizeof(_IO_stdin_used);
        auto size = sizeof(data);

        memcpy(vb_add(&builder->file, size), data, size);

        *section_header = (ELFSectionHeader) {
            .name_offset = name,
            .type = ELF_SECTION_PROGRAM,
            .flags = {
                .alloc = 1,
                .merge = 1,
            },
            .address = offset,
            .offset = offset,
            .size = size,
            .link = 0,
            .info = 0,
            .alignment = alignment,
            .entry_size = sizeof(data[0]),
        };
    }

    u32 eh_frame_offset = 0;
    u32 eh_frame_size = 0;
    u32 eh_frame_alignment = 0;
    u32 eh_frame_hdr_section_index = builder->section_headers.length;
    {
        // .eh_frame_hdr
        auto* section_header = vb_add(&builder->section_headers, 1);
        u64 alignment = 4;
        eh_frame_alignment = alignment;

        vb_align(&builder->file, alignment);
        auto offset = builder->file.length;
        eh_frame_offset = offset;

        auto name = elf_get_section_name(builder, strlit(".eh_frame_hdr"));

        // TODO: figure out a link between this and the code
        EhFrameHeaderEntry entries[] = {
            { .pc = 0xfffff01c, .fde = 0x34 },
            { .pc = 0xfffff118, .fde = 0x4c },
        };

        EhFrameHeader eh_frame_hdr = {
            .version = 1,
            .pointer_encoding = elf_eh_frame_sdata4 | elf_eh_frame_pcrel,
            .count_encoding = elf_eh_frame_udata4 | elf_eh_frame_absptr,
            .table_encoding = elf_eh_frame_sdata4 | elf_eh_frame_datarel,
            .frame_start = 0x18,
            .entry_count = array_length(entries),
        };

        auto size = sizeof(eh_frame_hdr) + sizeof(entries);
        eh_frame_size = size;
        auto* dst = vb_add(&builder->file, size);

        memcpy(dst, &eh_frame_hdr, sizeof(eh_frame_hdr));
        memcpy(dst + sizeof(eh_frame_hdr), entries, sizeof(entries));

        *section_header = (ELFSectionHeader) {
            .name_offset = name,
            .type = ELF_SECTION_PROGRAM,
            .flags = {
                .alloc = 1,
            },
            .address = offset,
            .offset = offset,
            .size = size,
            .link = 0,
            .info = 0,
            .alignment = alignment,
            .entry_size = 0,
        };
    }

    {
        // .eh_frame
        auto* section_header = vb_add(&builder->section_headers, 1);
        u64 alignment = 8;

        vb_align(&builder->file, alignment);
        auto offset = builder->file.length;

        auto name = elf_get_section_name(builder, strlit(".eh_frame"));

        u32 length = 0x14;
        *(u32*)vb_add(&builder->file, sizeof(u32)) = length;
        u32 id = 0;
        *(u32*)vb_add(&builder->file, sizeof(u32)) = id;
        u8 version = 1;
        *vb_add(&builder->file, 1) = version;
        
        auto augmentation = strlit("zR");
        memcpy(vb_add(&builder->file, augmentation.length + 1), augmentation.pointer, augmentation.length + 1);
        u32 code_alignment_factor = 1;
        s32 data_alignment_factor = -8;
        u32 return_address_coumn = 0x10;
        
        uleb128_encode(&builder->file, code_alignment_factor);
        sleb128_encode(&builder->file, data_alignment_factor);
        uleb128_encode(&builder->file, return_address_coumn);

        // TODO: figure out what this is
        *vb_add(&builder->file, 1) = 0x01;
        *vb_add(&builder->file, 1) = 0x1b;

        // TODO: figure out this bunch of dwarf stuff
        u8 data[] = {
            0x0C, 0x07, 0x08, 0x90, 0x01, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x1C, 0x00, 0x00, 0x00, 
            0xE0, 0xEF, 0xFF, 0xFF, 0x26, 0x00, 0x00, 0x00, 0x00, 0x44, 0x07, 0x10, 0x00, 0x00, 0x00, 0x00, 
            0x10, 0x00, 0x00, 0x00, 0x34, 0x00, 0x00, 0x00, 0xC4, 0xF0, 0xFF, 0xFF, 0x03, 0x00, 0x00, 0x00, 
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        };

        memcpy(vb_add(&builder->file, sizeof(data)), data, sizeof(data));

        auto size = builder->file.length - offset;

        *section_header = (ELFSectionHeader) {
            .name_offset = name,
            .type = ELF_SECTION_PROGRAM,
            .flags = {
                .alloc = 1,
            },
            .address = offset,
            .offset = offset,
            .size = size,
            .link = 0,
            .info = 0,
            .alignment = alignment,
            .entry_size = 0,
        };
    }

    {
        auto current_offset = builder->file.length;
        auto size = current_offset - read_only_offset;
        // Add ro program header
        *vb_add(&builder->program_headers, 1) = (ElfProgramHeader) {
            .type = PT_LOAD,
            .flags = {.readable = 1},
            .offset = read_only_offset,
            .virtual_address = read_only_offset,
            .physical_address = read_only_offset,
            .file_size = size,
            .memory_size = size,
            .alignment = 0x1000,
        };
    }

    // TODO: do a more thorough precomputation
    u64 init_array_size = 8;
    u64 fini_array_size = 8;
    u64 dynamic_size = 416;
    u64 got_size = 40;
    u64 got_plt_size = 24;

    auto total_size = init_array_size + fini_array_size + dynamic_size + got_size + got_plt_size;

    auto old_length = builder->file.length;
    vb_align(&builder->file, 0x1000);
    builder->file.length -= total_size;
    assert(old_length < builder->file.length);

    // TODO: figure out why a virtual address offset is needed here
    auto virtual_address_offset = 0x1000;

    auto* data_program_header = vb_add(&builder->program_headers, 1);
    auto data_offset = builder->file.length;

    u32 init_array_va = 0;
    {
        // .init_array
        auto* section_header = vb_add(&builder->section_headers, 1);
        u64 alignment = alignof(u64);

        vb_align(&builder->file, alignment);
        auto offset = builder->file.length;

        auto virtual_address = offset + virtual_address_offset;
        init_array_va = virtual_address;

        assert((virtual_address - offset) % 0x1000 == 0);

        auto name = elf_get_section_name(builder, strlit(".init_array"));

        u64 content[] = { text_init_array_offset };

        auto size = sizeof(content);
        assert(init_array_size == size);

        memcpy(vb_add(&builder->file, size), content, size);

        *section_header = (ELFSectionHeader) {
            .name_offset = name,
            .type = ELF_SECTION_INIT_ARRAY,
            .flags = {
                .alloc = 1,
                .write = 1,
            },
            .address = virtual_address,
            .offset = offset,
            .size = size,
            .link = 0,
            .info = 0,
            .alignment = alignment,
            .entry_size = sizeof(content[0]),
        };

        dynamic_relocations[relocation_index] = (ElfRelocationWithAddend) {
            .offset = virtual_address,
            .info = { .type = { .x86_64 = R_X86_64_RELATIVE }, .symbol = 0},
            .addend = text_init_array_offset,
        };
        relocation_index += 1;
    }

    u32 fini_array_va = 0;
    {
        // .fini_array
        auto* section_header = vb_add(&builder->section_headers, 1);
        u64 alignment = alignof(u64);

        vb_align(&builder->file, alignment);
        auto offset = builder->file.length;
        auto virtual_address = offset + virtual_address_offset;
        fini_array_va = virtual_address;

        assert((virtual_address - offset) % 0x1000 == 0);

        auto name = elf_get_section_name(builder, strlit(".fini_array"));

        u64 content[] = { text_fini_array_offset };
        auto size = sizeof(content);
        assert(size == fini_array_size);
        memcpy(vb_add(&builder->file, size), content, size);

        *section_header = (ELFSectionHeader) {
            .name_offset = name,
            .type = ELF_SECTION_FINI_ARRAY,
            .flags = {
                .alloc = 1,
                .write = 1,
            },
            .address = virtual_address,
            .offset = offset,
            .size = size,
            .link = 0,
            .info = 0,
            .alignment = alignment,
            .entry_size = sizeof(content[0]),
        };

        dynamic_relocations[relocation_index] = (ElfRelocationWithAddend) {
            .offset = virtual_address,
            .info = { .type = { .x86_64 = R_X86_64_RELATIVE }, .symbol = 0},
            .addend = text_fini_array_offset,
        };
        relocation_index += 1;
    }

    auto* __dso_handle_relocation = &dynamic_relocations[relocation_index];
    relocation_index += 1;

    auto dynamic_section_index = builder->section_headers.length;
    u32 dynamic_va = 0;
    {
        // .dynamic
        auto* section_header = vb_add(&builder->section_headers, 1);
        u64 alignment = alignof(ElfDynamicEntry);

        vb_align(&builder->file, alignment);
        auto offset = builder->file.length;
        auto virtual_address = offset + virtual_address_offset;
        dynamic_va = virtual_address;

        auto name = elf_get_section_name(builder, strlit(".dynamic"));

        ElfDynamicEntry dynamic_entries[] = {
            { .tag = DT_NEEDED, { .address = libcso6 }},
            { .tag = DT_INIT, { .address = init_offset }},
            { .tag = DT_FINI, { .address = fini_offset }},
            { .tag = DT_INIT_ARRAY, { .address = init_array_va }},
            { .tag = DT_INIT_ARRAYSZ, { .address = init_array_size }},
            { .tag = DT_FINI_ARRAY, { .address = fini_array_va }},
            { .tag = DT_FINI_ARRAYSZ, { .address = fini_array_size }},
            { .tag = DT_GNU_HASH, { .address = gnu_hash_offset }},
            { .tag = DT_STRTAB, { .address = dynstr_offset }},
            { .tag = DT_SYMTAB, { .address = dynsym_offset }},
            { .tag = DT_STRSZ, { .address = dynstr_size }},
            { .tag = DT_SYMENT, { .address = sizeof(ELFSymbol) }},
            // TODO
            { .tag = DT_DEBUG, { .address = 0}},
            { .tag = DT_RELA, { .address = rela_dyn_offset }},
            { .tag = DT_RELASZ, { .address = rela_dyn_size }},
            { .tag = DT_RELAENT, { .address = sizeof(ElfRelocationWithAddend) }},
            // TODO
            { .tag = DT_FLAGS_1, { .address = 134217728}},
            { .tag = DT_VERNEED, { .address = gnu_version_r_offset }},
            // TODO:
            { .tag = DT_VERNEEDNUM, { .address = 1 }},
            { .tag = DT_VERSYM, { .address = gnu_version_offset }},
            { .tag = DT_RELACOUNT, { .address = rela_count }},
            // TODO: figure out if these are needed
            { .tag = DT_NULL, { .address = 0}},
            { .tag = DT_NULL, { .address = 0}},
            { .tag = DT_NULL, { .address = 0}},
            { .tag = DT_NULL, { .address = 0}},
            { .tag = DT_NULL, { .address = 0}},
        };

        auto size = sizeof(dynamic_entries);

        memcpy(vb_add(&builder->file, size), dynamic_entries, size);

        *section_header = (ELFSectionHeader) {
            .name_offset = name,
            .type = ELF_SECTION_DYNAMIC,
            .flags = {
                .alloc = 1,
                .write = 1,
            },
            .address = virtual_address,
            .offset = offset,
            .size = size,
            .link = dynamic_string_table_index,
            .info = 0,
            .alignment = alignment,
            .entry_size = sizeof(ElfDynamicEntry),
        };

        // Add dynamic program header
        {
            *vb_add(&builder->program_headers, 1) = (ElfProgramHeader) {
                .type = PT_DYNAMIC,
                .flags = {.writeable = 1, .readable = 1},
                .offset = offset,
                .virtual_address = virtual_address,
                .physical_address = virtual_address,
                .file_size = size,
                .memory_size = size,
                .alignment = alignof(ElfDynamicEntry),
            };
        }
    }

    {
        // .got
        auto* section_header = vb_add(&builder->section_headers, 1);
        u64 alignment = 8;

        vb_align(&builder->file, alignment);
        auto offset = builder->file.length;
        auto virtual_address = offset + virtual_address_offset;

        auto name = elf_get_section_name(builder, strlit(".got"));
        // { .offset = 16320, .info = { .type = { .x86_64 = R_X86_64_GLOB_DAT }, .symbol = 1}, .addend = 0 }, // libc_start_main
        // { .offset = 16328, .info = { .type = { .x86_64 = R_X86_64_GLOB_DAT }, .symbol = 2}, .addend = 0 }, // itm_deregister
        // { .offset = 16336, .info = { .type = { .x86_64 = R_X86_64_GLOB_DAT }, .symbol = 3}, .addend = 0 }, // gmon_start
        // { .offset = 16344, .info = { .type = { .x86_64 = R_X86_64_GLOB_DAT }, .symbol = 4}, .addend = 0 }, // itm_register
        // { .offset = 16352, .info = { .type = { .x86_64 = R_X86_64_GLOB_DAT }, .symbol = 5}, .addend = 0 }, // cxa_finalize

        // TODO: unify these entries with code before
        u64 entries[] = { 0, 0, 0, 0, 0 };
        for (u32 i = 0; i < array_length(entries); i += 1)
        {
            auto* relocation = &dynamic_relocations[relocation_index];
            relocation_index += 1;
            *relocation = (ElfRelocationWithAddend) {
                .offset = virtual_address + (i * sizeof(u64)),
                .info = {
                    .type = { .x86_64 = R_X86_64_GLOB_DAT },
                    .symbol = i + 1,
                },
                .addend = 0,
            };
        }

        auto size = sizeof(entries);

        memcpy(vb_add(&builder->file, size), entries, size);

        *section_header = (ELFSectionHeader) {
            .name_offset = name,
            .type = ELF_SECTION_PROGRAM,
            .flags = {
                .alloc = 1,
                .write = 1,
            },
            .address = virtual_address,
            .offset = offset,
            .size = size,
            .link = 0,
            .info = 0,
            .alignment = alignment,
            .entry_size = alignof(u64),
        };
    }

    auto got_plt_section_index = builder->section_headers.length;
    u32 got_plt_va = 0;
    {
        // .got.plt
        auto* section_header = vb_add(&builder->section_headers, 1);
        u64 alignment = alignof(u64);

        vb_align(&builder->file, alignment);
        auto offset = builder->file.length;
        auto virtual_address = offset + virtual_address_offset;
        got_plt_va = virtual_address;

        auto name = elf_get_section_name(builder, strlit(".got.plt"));

        // TODO: figure out why there are three entries here
        u64 entries[] = { dynamic_va, 0, 0 };

        auto size = sizeof(entries);

        memcpy(vb_add(&builder->file, size), entries, size);

        *section_header = (ELFSectionHeader) {
            .name_offset = name,
            .type = ELF_SECTION_PROGRAM,
            .flags = {
                .alloc = 1,
                .write = 1,
            },
            .address = virtual_address,
            .offset = offset,
            .size = size,
            .link = 0,
            .info = 0,
            .alignment = alignment,
            .entry_size = 8,
        };
    }

    // Add several program headers
    {
        *vb_add(&builder->program_headers, 1) = (ElfProgramHeader) {
            .type = PT_NOTE,
            .flags = {.readable = 1},
            .offset = gnu_property_offset,
            .virtual_address = gnu_property_offset,
            .physical_address = gnu_property_offset,
            .file_size = gnu_property_size,
            .memory_size = gnu_property_size,
            .alignment = gnu_property_alignment,
        };

        *vb_add(&builder->program_headers, 1) = (ElfProgramHeader) {
            .type = PT_NOTE,
            .flags = {.readable = 1},
            .offset = gnu_build_id_offset,
            .virtual_address = gnu_build_id_offset,
            .physical_address = gnu_build_id_offset,
            .file_size = gnu_build_id_abi_note_size,
            .memory_size = gnu_build_id_abi_note_size,
            .alignment = gnu_build_id_alignment,
        };

        *vb_add(&builder->program_headers, 1) = (ElfProgramHeader) {
            .type = PT_GNU_PROPERTY,
            .flags = {.readable = 1},
            .offset = gnu_property_offset,
            .virtual_address = gnu_property_offset,
            .physical_address = gnu_property_offset,
            .file_size = gnu_property_size,
            .memory_size = gnu_property_size,
            .alignment = gnu_property_alignment,
        };

        *vb_add(&builder->program_headers, 1) = (ElfProgramHeader) {
            .type = PT_GNU_EH_FRAME,
            .flags = {.readable = 1},
            .offset = eh_frame_offset,
            .virtual_address = eh_frame_offset,
            .physical_address = eh_frame_offset,
            .file_size = eh_frame_size,
            .memory_size = eh_frame_size,
            .alignment = eh_frame_alignment,
        };
        *vb_add(&builder->program_headers, 1) = (ElfProgramHeader) {
            .type = PT_GNU_STACK,
            .flags = { .writeable = 1, .readable = 1},
            .offset = 0,
            .virtual_address = 0,
            .physical_address = 0,
            .file_size = 0,
            .memory_size = 0,
            .alignment = 16,
        };

        auto relro_end = builder->file.length;
        auto size = relro_end - data_offset;
        auto virtual_address = data_offset + virtual_address_offset;
        *vb_add(&builder->program_headers, 1) = (ElfProgramHeader) {
            .type = PT_GNU_RELRO,
            .flags = { .readable = 1},
            .offset = data_offset,
            .virtual_address = virtual_address,
            .physical_address = virtual_address,
            .file_size = size,
            .memory_size = size,
            .alignment = 1
        };
    }

    vb_align(&builder->file, 0x1000);

    u32 data_va_start = 0;
    u32 data_va_end = 0;
    u32 data_section_index = builder->section_headers.length;
    u32 __dso_handle_va;
    {
        // .data
        auto* section_header = vb_add(&builder->section_headers, 1);
        u64 alignment = alignof(u64);

        vb_align(&builder->file, alignment);
        auto offset = builder->file.length;
        auto virtual_address = offset + virtual_address_offset;
        data_va_start = virtual_address;

        auto name = elf_get_section_name(builder, strlit(".data"));

        // TODO: figure out what's this
        __dso_handle_va = virtual_address + sizeof(u64);
        u64 entries[] = { 0, __dso_handle_va };
        auto size = sizeof(entries);
        memcpy(vb_add(&builder->file, size), entries, size);
        data_va_end = data_va_start + size;

        *section_header = (ELFSectionHeader) {
            .name_offset = name,
            .type = ELF_SECTION_PROGRAM,
            .flags = {
                .alloc = 1,
                .write = 1,
            },
            .address = virtual_address,
            .offset = offset,
            .size = size,
            .link = 0,
            .info = 0,
            .alignment = alignment,
            .entry_size = 0,
        };

        *__dso_handle_relocation = (ElfRelocationWithAddend) {
            .offset = __dso_handle_va,
            .info = { .type = { .x86_64 = R_X86_64_RELATIVE }, .symbol = 0 },
            .addend = __dso_handle_va,
        };
    }

    auto bss_size = 0;
    auto bss_section_index = builder->section_headers.length;
    auto bss_end = 0;
    auto bss_start = 0;
    {
        // .bss
        auto* section_header = vb_add(&builder->section_headers, 1);
        u64 alignment = 1;

        vb_align(&builder->file, alignment);
        auto offset = builder->file.length;
        auto virtual_address = offset + virtual_address_offset;
        bss_start = virtual_address;

        auto name = elf_get_section_name(builder, strlit(".bss"));

        bss_size = 8;
        bss_end = virtual_address + bss_size;

        *section_header = (ELFSectionHeader) {
            .name_offset = name,
            .type = ELF_SECTION_BSS,
            .flags = {
                .alloc = 1,
                .write = 1,
            },
            .address = virtual_address,
            .offset = offset,
            .size = bss_size,
            .link = 0,
            .info = 0,
            .alignment = alignment,
            .entry_size = 0,
        };
    }

    // Fill data program header (writeable, readable)
    {
        auto current_offset = builder->file.length;
        auto file_size = current_offset - data_offset;
        auto virtual_address = data_offset + virtual_address_offset;
        *data_program_header = (ElfProgramHeader) {
            .type = PT_LOAD,
            .flags = {.writeable = 1, .readable = 1},
            .offset = data_offset,
            .virtual_address = virtual_address,
            .physical_address = virtual_address,
            .file_size = file_size,
            .memory_size = file_size + bss_size,
            .alignment = 0x1000,
        };
    }

    u64 virtual_address = 0;

    {
        // .comment
        auto* section_header = vb_add(&builder->section_headers, 1);
        u64 alignment = 1;

        vb_align(&builder->file, alignment);
        auto offset = builder->file.length;

        auto name = elf_get_section_name(builder, strlit(".comment"));

        String strings[] = {
            strlit("GCC: (GNU) 14.2.1 20240805"),
            strlit("GCC: (GNU) 14.2.1 20240910"),
            strlit("clang version 18.1.8"),
        };

        for (u32 i = 0; i < array_length(strings); i += 1)
        {
            String string = strings[i];
            // Copy also null-terminated byte
            auto string_size = string.length + 1;
            memcpy(vb_add(&builder->file, string_size), string.pointer, string_size);
        }

        auto size = builder->file.length - offset;

        *section_header = (ELFSectionHeader) {
            .name_offset = name,
            .type = ELF_SECTION_PROGRAM,
            .flags = {
                .merge = 1,
                .strings = 1,
            },
            .address = virtual_address,
            .offset = offset,
            .size = size,
            .link = 0,
            .info = 0,
            .alignment = alignment,
            .entry_size = 1,
        };
    }

    {
        // .debug_info
        auto* section_header = vb_add(&builder->section_headers, 1);
        u64 alignment = 1;

        vb_align(&builder->file, alignment);
        auto offset = builder->file.length;

        auto name = elf_get_section_name(builder, strlit(".debug_info"));

        u8 data[] = {
            0x33, 0x00, 0x00, 0x00, 0x05, 0x00, 0x01, 0x08, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x1D, 0x00, 
            0x01, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x03, 0x00, 0x00, 0x00, 0x08, 
            0x00, 0x00, 0x00, 0x02, 0x00, 0x03, 0x00, 0x00, 0x00, 0x01, 0x57, 0x03, 0x00, 0x01, 0x32, 0x00, 
            0x00, 0x00, 0x03, 0x04, 0x05, 0x04, 0x00, 
        };

        auto size = sizeof(data);

        memcpy(vb_add(&builder->file, size), data, size);

        *section_header = (ELFSectionHeader) {
            .name_offset = name,
            .type = ELF_SECTION_PROGRAM,
            .flags = {},
            .address = virtual_address,
            .offset = offset,
            .size = size,
            .link = 0,
            .info = 0,
            .alignment = alignment,
            .entry_size = 0,
        };
    }

    {
        // .debug_abbrev
        auto* section_header = vb_add(&builder->section_headers, 1);
        u64 alignment = 1;

        vb_align(&builder->file, alignment);
        auto offset = builder->file.length;

        auto name = elf_get_section_name(builder, strlit(".debug_abbrev"));

        u8 data[] = {
            0x01, 0x11, 0x01, 0x25, 0x25, 0x13, 0x05, 0x03, 0x25, 0x72, 0x17, 0x10, 0x17, 0x1B, 0x25, 0x11, 
            0x1B, 0x12, 0x06, 0x73, 0x17, 0x00, 0x00, 0x02, 0x2E, 0x00, 0x11, 0x1B, 0x12, 0x06, 0x40, 0x18, 
            0x7A, 0x19, 0x03, 0x25, 0x3A, 0x0B, 0x3B, 0x0B, 0x49, 0x13, 0x3F, 0x19, 0x00, 0x00, 0x03, 0x24, 
            0x00, 0x03, 0x25, 0x3E, 0x0B, 0x0B, 0x0B, 0x00, 0x00, 0x00, 
        };

        auto size = sizeof(data);

        memcpy(vb_add(&builder->file, size), data, size);

        *section_header = (ELFSectionHeader) {
            .name_offset = name,
            .type = ELF_SECTION_PROGRAM,
            .flags = {},
            .address = virtual_address,
            .offset = offset,
            .size = size,
            .link = 0,
            .info = 0,
            .alignment = alignment,
            .entry_size = 0,
        };
    }

    {
        // .debug_line
        auto* section_header = vb_add(&builder->section_headers, 1);
        u64 alignment = 1;

        vb_align(&builder->file, alignment);
        auto offset = builder->file.length;

        auto name = elf_get_section_name(builder, strlit(".debug_line"));

        u8 data[] = {
            0x55, 0x00, 0x00, 0x00, 0x05, 0x00, 0x08, 0x00, 0x37, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0xFB, 
            0x0E, 0x0D, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01, 
            0x1F, 0x01, 0x00, 0x00, 0x00, 0x00, 0x03, 0x01, 0x1F, 0x02, 0x0F, 0x05, 0x1E, 0x01, 0x14, 0x00, 
            0x00, 0x00, 0x00, 0x05, 0xAB, 0x89, 0xF5, 0x48, 0x1B, 0xC9, 0xF2, 0xD0, 0x37, 0xE7, 0x88, 0x66, 
            0x41, 0xE9, 0x19, 0x04, 0x00, 0x05, 0x05, 0x0A, 0x00, 0x09, 0x02, 0x1C, 0x11, 0x00, 0x00, 0x00, 
            0x00, 0x00, 0x00, 0x14, 0x02, 0x03, 0x00, 0x01, 0x01, 
        };

        auto size = sizeof(data);

        memcpy(vb_add(&builder->file, size), data, size);

        *section_header = (ELFSectionHeader) {
            .name_offset = name,
            .type = ELF_SECTION_PROGRAM,
            .flags = {},
            .address = virtual_address,
            .offset = offset,
            .size = size,
            .link = 0,
            .info = 0,
            .alignment = alignment,
            .entry_size = 0,
        };
    }

    {
        // .debug_str
        auto* section_header = vb_add(&builder->section_headers, 1);
        u64 alignment = 1;

        vb_align(&builder->file, alignment);
        auto offset = builder->file.length;

        auto name = elf_get_section_name(builder, strlit(".debug_str"));

        u8 data[] = {
            0x63, 0x6C, 0x61, 0x6E, 0x67, 0x20, 0x76, 0x65, 0x72, 0x73, 0x69, 0x6F, 0x6E, 0x20, 0x31, 0x38, 
            0x2E, 0x31, 0x2E, 0x38, 0x00, 0x6D, 0x61, 0x69, 0x6E, 0x2E, 0x63, 0x00, 0x2F, 0x68, 0x6F, 0x6D, 
            0x65, 0x2F, 0x64, 0x61, 0x76, 0x69, 0x64, 0x2F, 0x6D, 0x69, 0x6E, 0x69, 0x6D, 0x61, 0x6C, 0x00, 
            0x6D, 0x61, 0x69, 0x6E, 0x00, 0x69, 0x6E, 0x74, 0x00, 
        };

        auto size = sizeof(data);

        memcpy(vb_add(&builder->file, size), data, size);

        *section_header = (ELFSectionHeader) {
            .name_offset = name,
            .type = ELF_SECTION_PROGRAM,
            .flags = {
                .merge = 1,
                .strings = 1,
            },
            .address = virtual_address,
            .offset = offset,
            .size = size,
            .link = 0,
            .info = 0,
            .alignment = alignment,
            .entry_size = 1,
        };
    }

    {
        // .debug_addr
        auto* section_header = vb_add(&builder->section_headers, 1);
        u64 alignment = 1;

        vb_align(&builder->file, alignment);
        auto offset = builder->file.length;

        auto name = elf_get_section_name(builder, strlit(".debug_addr"));

        u8 data[] = { 0x0C, 0x00, 0x00, 0x00, 0x05, 0x00, 0x08, 0x00, 0x1C, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, };

        auto size = sizeof(data);

        memcpy(vb_add(&builder->file, size), data, size);

        *section_header = (ELFSectionHeader) {
            .name_offset = name,
            .type = ELF_SECTION_PROGRAM,
            .flags = {},
            .address = virtual_address,
            .offset = offset,
            .size = size,
            .link = 0,
            .info = 0,
            .alignment = alignment,
            .entry_size = 0,
        };
    }

    {
        // .debug_line_str
        auto* section_header = vb_add(&builder->section_headers, 1);
        u64 alignment = 1;

        vb_align(&builder->file, alignment);
        auto offset = builder->file.length;

        auto name = elf_get_section_name(builder, strlit(".debug_line_str"));

        u8 data[] = {
            0x2F, 0x68, 0x6F, 0x6D, 0x65, 0x2F, 0x64, 0x61, 0x76, 0x69, 0x64, 0x2F, 0x6D, 0x69, 0x6E, 0x69, 
            0x6D, 0x61, 0x6C, 0x00, 0x6D, 0x61, 0x69, 0x6E, 0x2E, 0x63, 0x00, 
        };

        auto size = sizeof(data);

        memcpy(vb_add(&builder->file, size), data, size);

        *section_header = (ELFSectionHeader) {
            .name_offset = name,
            .type = ELF_SECTION_PROGRAM,
            .flags = {
                .merge = 1,
                .strings = 1,
            },
            .address = virtual_address,
            .offset = offset,
            .size = size,
            .link = 0,
            .info = 0,
            .alignment = alignment,
            .entry_size = 1,
        };
    }

    {
        // .debug_str_offsets
        auto* section_header = vb_add(&builder->section_headers, 1);
        u64 alignment = 1;

        vb_align(&builder->file, alignment);
        auto offset = builder->file.length;

        auto name = elf_get_section_name(builder, strlit(".debug_str_offsets"));

        u8 data[] = {
            0x18, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x15, 0x00, 0x00, 0x00, 
            0x1C, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x35, 0x00, 0x00, 0x00, 
        };

        auto size = sizeof(data);

        memcpy(vb_add(&builder->file, size), data, size);

        *section_header = (ELFSectionHeader) {
            .name_offset = name,
            .type = ELF_SECTION_PROGRAM,
            .flags = {},
            .address = virtual_address,
            .offset = offset,
            .size = size,
            .link = 0,
            .info = 0,
            .alignment = alignment,
            .entry_size = 0,
        };
    }

    {
        // .symtab
        auto* section_header = vb_add(&builder->section_headers, 1);
        u64 alignment = alignof(ELFSymbol);

        vb_align(&builder->file, alignment);
        auto offset = builder->file.length;

        auto name = elf_get_section_name(builder, strlit(".symtab"));

        u16 section_index_undefined = 0;
        u16 section_index_absolute = 0xfff1;

        auto main_c_string = st_get_string(&builder->static_st, strlit("main.c"));
        auto _dynamic_string = st_get_string(&builder->static_st, strlit("_DYNAMIC"));
        auto eh_frame_hdr_string = st_get_string(&builder->static_st, strlit("__GNU_EH_FRAME_HDR"));
        auto got_string = st_get_string(&builder->static_st, strlit("_GLOBAL_OFFSET_TABLE_"));
        auto libc_start_main_string = st_get_string(&builder->static_st, strlit("__libc_start_main@GLIBC_2.34"));
        auto deregister_string = st_get_string(&builder->static_st, strlit("_ITM_deregisterTMCloneTable"));
        auto edata_string = st_get_string(&builder->static_st, strlit("_edata"));
        auto fini_string = st_get_string(&builder->static_st, strlit("_fini"));
        auto __data_start_string = st_get_string(&builder->static_st, strlit("__data_start"));
        auto data_start_string = st_get_string(&builder->static_st, strlit("data_start"));

        ELFSymbol symbols[] = {
            {
                .name_offset = main_c_string,
                .type = ELF_SYMBOL_TYPE_FILE,
                .binding = ELF_SYMBOL_BINDING_LOCAL,
                .visibility = ELF_SYMBOL_VISIBILITY_DEFAULT,
                .section_index = section_index_absolute,
                .value = 0,
                .size = 0,
            },
            {
                .name_offset = 0,
                .type = ELF_SYMBOL_TYPE_FILE,
                .binding = ELF_SYMBOL_BINDING_LOCAL,
                .visibility = ELF_SYMBOL_VISIBILITY_DEFAULT,
                .section_index = section_index_absolute,
                .value = 0,
                .size = 0
            },
            {
                .name_offset = _dynamic_string,
                .type = ELF_SYMBOL_TYPE_OBJECT,
                .binding = ELF_SYMBOL_BINDING_LOCAL,
                .visibility = ELF_SYMBOL_VISIBILITY_DEFAULT,
                .section_index = dynamic_section_index,
                .value = dynamic_va,
                .size = 0,
            },
            {
                .name_offset = eh_frame_hdr_string,
                .type = ELF_SYMBOL_TYPE_NONE,
                .binding = ELF_SYMBOL_BINDING_LOCAL,
                .visibility = ELF_SYMBOL_VISIBILITY_DEFAULT,
                .section_index = eh_frame_hdr_section_index,
                .value = eh_frame_offset,
                .size = 0,
            },
            {
                .name_offset = got_string,
                .type = ELF_SYMBOL_TYPE_OBJECT,
                .binding = ELF_SYMBOL_BINDING_LOCAL,
                .visibility = ELF_SYMBOL_VISIBILITY_DEFAULT,
                .section_index = got_plt_section_index,
                .value = got_plt_va,
                .size = 0,
            },
            {
                .name_offset = libc_start_main_string,
                .type = ELF_SYMBOL_TYPE_FUNCTION,
                .binding = ELF_SYMBOL_BINDING_GLOBAL,
                .visibility = ELF_SYMBOL_VISIBILITY_DEFAULT,
                .section_index = section_index_undefined,
                .value = 0,
                .size = 0,
            },
            {
                .name_offset = deregister_string,
                .type = ELF_SYMBOL_TYPE_NONE,
                .binding = ELF_SYMBOL_BINDING_WEAK,
                .visibility = ELF_SYMBOL_VISIBILITY_DEFAULT,
                .section_index = section_index_undefined,
                .value = 0,
                .size = 0,
            },
            {
                .name_offset = data_start_string,
                .type = ELF_SYMBOL_TYPE_NONE,
                .binding = ELF_SYMBOL_BINDING_WEAK,
                .visibility = ELF_SYMBOL_VISIBILITY_DEFAULT,
                .section_index = data_section_index,
                .value = data_va_start,
                .size = 0,
            },
            {
                .name_offset = edata_string,
                .type = ELF_SYMBOL_TYPE_NONE,
                .binding = ELF_SYMBOL_BINDING_GLOBAL,
                .visibility = ELF_SYMBOL_VISIBILITY_DEFAULT,
                .section_index = data_section_index,
                .value = data_va_end,
                .size = 0,
            },
            {
                .name_offset = fini_string,
                .type = ELF_SYMBOL_TYPE_FUNCTION,
                .binding = ELF_SYMBOL_BINDING_GLOBAL,
                .visibility = ELF_SYMBOL_VISIBILITY_HIDDEN, // TODO: investigate this other field
                .section_index = fini_section_index,
                .value = fini_offset,
                .size = 0,
            },
            {
                .name_offset = __data_start_string,
                .type = ELF_SYMBOL_TYPE_NONE,
                .binding = ELF_SYMBOL_BINDING_GLOBAL,
                .visibility = ELF_SYMBOL_VISIBILITY_DEFAULT,
                .section_index = data_section_index,
                .value = data_va_start,
                .size = 0,
            },
            {
                .name_offset = st_get_string(&builder->static_st, strlit("__gmon_start__")),
                .type = ELF_SYMBOL_TYPE_NONE,
                .binding = ELF_SYMBOL_BINDING_WEAK,
                .visibility = ELF_SYMBOL_VISIBILITY_DEFAULT,
                .section_index = section_index_undefined,
                .value = 0,
                .size = 0,
            },
            {
                .name_offset = st_get_string(&builder->static_st, strlit("__dso_handle")),
                .type = ELF_SYMBOL_TYPE_OBJECT,
                .binding = ELF_SYMBOL_BINDING_GLOBAL,
                .visibility = ELF_SYMBOL_VISIBILITY_HIDDEN,
                .section_index = data_section_index,
                .value = __dso_handle_va,
                .size = 0,
            },
            {
                .name_offset = st_get_string(&builder->static_st, strlit("_IO_stdin_used")),
                .type = ELF_SYMBOL_TYPE_OBJECT,
                .binding = ELF_SYMBOL_BINDING_GLOBAL,
                .visibility = ELF_SYMBOL_VISIBILITY_DEFAULT,
                .section_index = rodata_section_index,
                .value = rodata_va,
                .size = _IO_stdin_used_size,
            },
            {
                .name_offset = st_get_string(&builder->static_st, strlit("_end")),
                .type = ELF_SYMBOL_TYPE_NONE,
                .binding = ELF_SYMBOL_BINDING_GLOBAL,
                .visibility = ELF_SYMBOL_VISIBILITY_DEFAULT,
                .section_index = bss_section_index,
                .value = bss_end,
                .size = 0,
            },
            {
                .name_offset = st_get_string(&builder->static_st, strlit("_start")),
                .type = ELF_SYMBOL_TYPE_FUNCTION,
                .binding = ELF_SYMBOL_BINDING_GLOBAL,
                .visibility = ELF_SYMBOL_VISIBILITY_DEFAULT,
                .section_index = text_section_index,
                .value = _start_offset,
                .size = _start_size,
            },
            {
                .name_offset = st_get_string(&builder->static_st, strlit("__bss_start")),
                .type = ELF_SYMBOL_TYPE_NONE,
                .binding = ELF_SYMBOL_BINDING_GLOBAL,
                .visibility = ELF_SYMBOL_VISIBILITY_DEFAULT,
                .section_index = bss_section_index,
                .value = bss_start,
                .size = 0,
            },
            {
                .name_offset = st_get_string(&builder->static_st, strlit("main")),
                .type = ELF_SYMBOL_TYPE_FUNCTION,
                .binding = ELF_SYMBOL_BINDING_GLOBAL,
                .visibility = ELF_SYMBOL_VISIBILITY_DEFAULT,
                .section_index = text_section_index,
                .value = main_offset,
                .size = main_size,
            },
            {
                .name_offset = st_get_string(&builder->static_st, strlit("__TMC_END__")),
                .type = ELF_SYMBOL_TYPE_OBJECT,
                .binding = ELF_SYMBOL_BINDING_GLOBAL,
                .visibility = ELF_SYMBOL_VISIBILITY_HIDDEN,
                .section_index = data_section_index,
                .value = data_va_end,
                .size = 0,
            },
            {
                .name_offset = st_get_string(&builder->static_st, strlit("_ITM_registerTMCloneTable")),
                .type = ELF_SYMBOL_TYPE_NONE,
                .binding = ELF_SYMBOL_BINDING_WEAK,
                .visibility = ELF_SYMBOL_VISIBILITY_DEFAULT,
                .section_index = section_index_undefined,
                .value = 0,
                .size = 0,
            },
            {
                .name_offset = st_get_string(&builder->static_st, strlit("__cxa_finalize@GLIBC_2.2.5")),
                .type = ELF_SYMBOL_TYPE_FUNCTION,
                .binding = ELF_SYMBOL_BINDING_WEAK,
                .visibility = ELF_SYMBOL_VISIBILITY_DEFAULT,
                .section_index = section_index_undefined,
                .value = 0,
                .size = 0,
            },
            {
                .name_offset = st_get_string(&builder->static_st, strlit("_init")),
                .type = ELF_SYMBOL_TYPE_FUNCTION,
                .binding = ELF_SYMBOL_BINDING_GLOBAL,
                .visibility = ELF_SYMBOL_VISIBILITY_HIDDEN,
                .section_index = init_section_index,
                .value = init_offset,
                .size = 0,
            }
        };
        memcpy(vb_add(&builder->static_st.symbol_table, array_length(symbols)), symbols, sizeof(symbols));

        memcpy(vb_add(&builder->file, builder->static_st.symbol_table.length * sizeof(ELFSymbol)), builder->static_st.symbol_table.pointer, builder->static_st.symbol_table.length * sizeof(ELFSymbol));

        auto size = builder->file.length - offset;

        *section_header = (ELFSectionHeader) {
            .name_offset = name,
            .type = ELF_SECTION_SYMBOL_TABLE,
            .flags = {},
            .address = virtual_address,
            .offset = offset,
            .size = size,
            .link = builder->section_headers.length, // Symtab is already added, so the length is actually the index of the .strtab section
            .info = 6, // TODO: figure out
            .alignment = alignment,
            .entry_size = sizeof(ELFSymbol),
        };
    }

    {
        // .strtab
        auto* section_header = vb_add(&builder->section_headers, 1);
        u64 alignment = 1;

        vb_align(&builder->file, alignment);
        auto offset = builder->file.length;

        auto name = elf_get_section_name(builder, strlit(".strtab"));

        memcpy(vb_add(&builder->file, builder->static_st.string_table.length), builder->static_st.string_table.pointer, builder->static_st.string_table.length);

        auto size = builder->file.length - offset;

        *section_header = (ELFSectionHeader) {
            .name_offset = name,
            .type = ELF_SECTION_STRING_TABLE,
            .flags = {},
            .address = virtual_address,
            .offset = offset,
            .size = size,
            .link = 0,
            .info = 0,
            .alignment = alignment,
            .entry_size = 0,
        };
    }

    {
        // .shstrtab
        auto* section_header = vb_add(&builder->section_headers, 1);
        u64 alignment = 1;

        vb_align(&builder->file, alignment);
        auto offset = builder->file.length;

        auto name = elf_get_section_name(builder, strlit(".shstrtab"));

        memcpy(vb_add(&builder->file, builder->section_string_table.length), builder->section_string_table.pointer, builder->section_string_table.length);

        auto size = builder->file.length - offset;

        *section_header = (ELFSectionHeader) {
            .name_offset = name,
            .type = ELF_SECTION_STRING_TABLE,
            .flags = {},
            .address = virtual_address,
            .offset = offset,
            .size = size,
            .link = 0,
            .info = 0,
            .alignment = alignment,
            .entry_size = 0,
        };
    }

    vb_align(&builder->file, alignof(ELFSectionHeader));
    auto section_header_offset = builder->file.length;
    auto section_header_count = builder->section_headers.length;
    memcpy(vb_add(&builder->file, sizeof(ELFSectionHeader) * section_header_count), builder->section_headers.pointer, builder->section_headers.length * sizeof(ELFSectionHeader));

    *elf_header = (ELFHeader)
    {
        .identifier = { 0x7f, 'E', 'L', 'F' },
        .bit_count = bits64,
        .endianness = little,
        .format_version = 1,
        .abi = ELF_ABI_SYSTEM_V,
        .abi_version = 0,
        .padding = {},
        .type = shared,
        .machine = x86_64,
        .version = 1,
        .entry_point = _start_offset,
        .program_header_offset = sizeof(ELFHeader),
        .section_header_offset = section_header_offset,
        .flags = 0,
        .elf_header_size = sizeof(ELFHeader),
        .program_header_size = sizeof(ElfProgramHeader),
        .program_header_count = program_header_count,
        .section_header_size = sizeof(ELFSectionHeader),
        .section_header_count = section_header_count,
        .section_header_string_table_index = section_header_count - 1,
    };

    assert(builder->program_headers.length == program_header_count);
    memcpy(program_headers, builder->program_headers.pointer, sizeof(ElfProgramHeader) * builder->program_headers.length);

    assert(relocation_index == expected_relocation_count);

    // Check if the file matches
    
    // {
    //     auto s = file_read(thread->arena, strlit("/home/david/minimal/main"));
    //
    //     if (s.length != builder->file.length)
    //     {
    //         fail();
    //     }
    //
    //     for (u32 i = 0; i < s.length; i += 1)
    //     {
    //         auto mine = builder->file.pointer[i];
    //         auto correct = s.pointer[i];
    //         if (mine != correct)
    //         {
    //             fail();
    //         }
    //     }
    // }

    auto exe_path_z = options->exe_path;
    {
        int fd = syscall_open(exe_path_z, O_WRONLY | O_CREAT | O_TRUNC, 0755);
        assert(fd != -1);
        syscall_write(fd, builder->file.pointer, builder->file.length);

        syscall_close(fd);
    }
}

void subsume_node_without_killing(Thread* thread, NodeIndex old_node_index, NodeIndex new_node_index)
{
    assert(!index_equal(old_node_index, new_node_index));
    auto* old = thread_node_get(thread, old_node_index);
    auto* new = thread_node_get(thread, new_node_index);
    auto old_node_outputs = node_get_outputs(thread, old);

    u8 allow_cycle = old->id == IR_PHI || old->id == IR_REGION || new->id == IR_REGION;

    for (auto i = old->output_count; i > 0; i -= 1)
    {
        auto output = old_node_outputs.pointer[i - 1];
        old->output_count -= 1;
        if (!allow_cycle && index_equal(output, new_node_index))
        {
            continue;
        }

        auto* output_node = thread_node_get(thread, output);
        auto output_inputs = node_get_inputs(thread, output_node);

        u16 output_input_index;
        for (output_input_index = 0; output_input_index < output_inputs.length; output_input_index += 1)
        {
            auto output_input = output_inputs.pointer[output_input_index];
            if (index_equal(output_input, old_node_index))
            {
                output_inputs.pointer[output_input_index] = new_node_index;
                node_add_output(thread, new_node_index, output);
                break;
            }
        }

        assert(output_input_index < output_inputs.length);
    }
}

fn NodeIndex function_get_control_start(Thread* thread, Function* function)
{
    auto* root = thread_node_get(thread, function->root);
    auto outputs = node_get_outputs(thread, root);
    auto result = outputs.pointer[0];
    return result;
}

fn u8 cfg_is_control(Thread* thread, NodeIndex node_index)
{
    auto* node = thread_node_get(thread, node_index);
    auto backend_type = type_pair_get_backend(node->type);
    if (backend_type == BACKEND_TYPE_CONTROL)
    {
        return 1;
    }
    else if (backend_type == BACKEND_TYPE_TUPLE)
    {
        if (node->id == IR_ROOT)
        {
            return 1;
        }
        else
        {
            todo();
        }
    }

    return 0;
}

fn u8 cfg_node_terminator(Node* node)
{
    u8 is_terminator;
    switch (node->id)
    {
        case IR_PROJECTION:
        case IR_REGION:
            is_terminator = 0;
            break;
        case IR_RETURN:
        case IR_ROOT:
            is_terminator = 1;
            break;
        default:
            todo();
    }

    return is_terminator;
}

fn NodeIndex basic_block_end(Thread* thread, NodeIndex start_index)
{
    auto node_index = start_index;
    while (1)
    {
        auto* node = thread_node_get(thread, node_index);
        u8 is_terminator = cfg_node_terminator(node);

        if (is_terminator)
        {
            break;
        }

        auto outputs = node_get_outputs(thread, node);
        auto new_node_index = node_index;

        for (u16 i = 0; i < outputs.length; i += 1)
        {
            auto output_index = outputs.pointer[i];
            auto* output = thread_node_get(thread, output_index);
            auto output_inputs = node_get_inputs(thread, output);
            if (index_equal(output_inputs.pointer[0], node_index) && cfg_is_control(thread, output_index))
            {
                if (output->id == IR_REGION)
                {
                    return node_index;
                }

                new_node_index = output_index;
                break;
            }
        }

        if (index_equal(node_index, new_node_index))
        {
            break;
        }

        node_index = new_node_index;
    }

    return node_index;
}

STRUCT(Block)
{
    NodeIndex start;
    NodeIndex end;
    NodeIndex successors[2];
    u32 successor_count;
    struct Block* parent;
};

fn NodeIndex cfg_next_control(Thread* thread, NodeIndex node_index)
{
    auto* node = thread_node_get(thread, node_index);
    auto outputs = node_get_outputs(thread, node);

    for (u16 i = 0; i < outputs.length; i += 1)
    {
        auto output_index = outputs.pointer[i];
        if (cfg_is_control(thread, output_index))
        {
            return output_index;
        }
    }

    return invalidi(Node);
}

fn NodeIndex cfg_next_user(Thread* thread, NodeIndex node_index)
{
    auto* node = thread_node_get(thread, node_index);
    auto outputs = node_get_outputs(thread, node);

    for (u16 i = 0; i < outputs.length; i += 1)
    {
        auto output_index = outputs.pointer[i];
        if (cfg_is_control(thread, output_index))
        {
            return output_index;
        }
    }

    return invalidi(Node);
}

fn u8 cfg_is_endpoint(Thread* thread, Node* node)
{
    unused(thread);
    switch (node->id)
    {
        case IR_ROOT:
        case IR_RETURN:
            return 1;
        default:
            return 0;
    }
}

fn Block* create_block(Thread* thread, NodeIndex node_index)
{
    auto end_of_basic_block_index = basic_block_end(thread, node_index);
    auto* end_node = thread_node_get(thread, end_of_basic_block_index);

    u32 successor_count = 0;
    // Branch
    auto is_endpoint = cfg_is_endpoint(thread, end_node);
    auto is_branch = 0;
    if (is_branch)
    {
        todo();
    }
    else if (type_pair_get_backend(end_node->type) == BACKEND_TYPE_TUPLE)
    {
        todo();
    }
    else if (!is_endpoint)
    {
        successor_count = 1;
    }

    auto* block = arena_allocate(thread->arena, Block, 1);
    *block = (Block)
    {
        .start = node_index,
        .end = end_of_basic_block_index,
        .successor_count = successor_count,
    };

    if (node_is_cfg_fork(end_node))
    {
        todo();
    }
    else if (!is_endpoint)
    {
        block->successors[0] = cfg_next_user(thread, end_of_basic_block_index);
    }

    return block;
}

fn NodeIndex node_select_instruction(Thread* thread, NodeIndex node_index)
{
    auto* node = thread_node_get(thread, node_index);
    switch (node->id)
    {
        case IR_PROJECTION:
            return node_index;
        case IR_ROOT:
            return node_index;
        case IR_PHI:
            {
                auto backend_type = type_pair_get_backend(node->type);
                if (backend_type <= BACKEND_TYPE_SCALAR_LAST)
                {
                    {
                        auto copy_index = thread_node_add(thread, (NodeCreate)
                                {
                                .id = MACHINE_COPY,
                                .type_pair = node->type,
                                .inputs = array_to_slice(((NodeIndex[]) {
                                    invalidi(Node),
                                    invalidi(Node),
                                })),
                                });
                        thread_node_get(thread, copy_index)->machine_copy = (NodeMachineCopy) {
                            .use_mask = Index(RegisterMask, REGISTER_MASK_GPR),
                            .def_mask = Index(RegisterMask, REGISTER_MASK_GPR),
                        };
                        subsume_node_without_killing(thread, node_index, copy_index);
                        node_set_input(thread, copy_index, 1, node_index);
                        node_gvn_intern(thread, copy_index);
                    }

                    {
                        auto inputs = node_get_inputs(thread, node);

                        for (u16 i = 1; i < inputs.length; i += 1)
                        {
                            auto input_index = inputs.pointer[i];
                            auto input = thread_node_get(thread, input_index);
                            assert(input->id != MACHINE_MOVE);
                            auto move_index = thread_node_add(thread, (NodeCreate)
                                    {
                                    .id = MACHINE_MOVE,
                                    .type_pair = input->type,
                                    .inputs = array_to_slice(((NodeIndex[]) {
                                                invalidi(Node),
                                                invalidi(Node),
                                                })),
                                    });
                            node_set_input(thread, move_index, 1, input_index);
                            node_set_input(thread, node_index, i, move_index);
                            node_gvn_intern(thread, move_index);
                        }
                    }
                }

                return node_index;
            } break;
        case IR_RETURN:
        case IR_REGION:
        case IR_INTEGER_CONSTANT:

        case MACHINE_MOVE:
        case MACHINE_COPY:
            return invalidi(Node);
        default:
            todo();
    }
}

STRUCT(BasicBlockIndex)
{
    u32 index;
};
decl_vb(BasicBlockIndex);

STRUCT(BasicBlock)
{
    VirtualBuffer(NodeIndex) items;
    Bitset gen;
    Bitset kill;
    Bitset live_in;
    Bitset live_out;
    NodeIndex start;
    NodeIndex end;
    s32 dominator_depth;
    BasicBlockIndex dominator;
    s32 forward;
};
decl_vb(BasicBlock);
decl_vbp(BasicBlock);

fn u8 node_is_pinned(Node* node)
{
    switch (node->id)
    {
    case IR_ROOT:
    case IR_PROJECTION:
    case IR_RETURN:
    case IR_REGION:
    case IR_PHI:
        return 1;
    case IR_SYMBOL_TABLE:
    case IR_INTEGER_ADD:
    case IR_INTEGER_SUBSTRACT:
    case IR_INTEGER_MULTIPLY:
    case IR_INTEGER_DIVIDE:
    case IR_INTEGER_REMAINDER:
    case IR_INTEGER_SHIFT_LEFT:
    case IR_INTEGER_SHIFT_RIGHT:
    case IR_INTEGER_AND:
    case IR_INTEGER_OR:
    case IR_INTEGER_XOR:
    case IR_INTEGER_COMPARE_EQUAL:
    case IR_INTEGER_COMPARE_NOT_EQUAL:
    case IR_INTEGER_NEGATION:
    case IR_INTEGER_CONSTANT:
    case MACHINE_COPY:
    case MACHINE_MOVE:
    case NODE_COUNT:
        return 0;
    case MACHINE_JUMP:
        todo();
    }
}

fn u8 node_has_memory_out(NodeId id)
{
    switch (id)
    {
    case IR_ROOT:
    case IR_PROJECTION:
    case IR_RETURN:
    case IR_REGION:
    case IR_PHI:
    case IR_SYMBOL_TABLE:
    case IR_INTEGER_ADD:
    case IR_INTEGER_SUBSTRACT:
    case IR_INTEGER_MULTIPLY:
    case IR_INTEGER_DIVIDE:
    case IR_INTEGER_REMAINDER:
    case IR_INTEGER_SHIFT_LEFT:
    case IR_INTEGER_SHIFT_RIGHT:
    case IR_INTEGER_AND:
    case IR_INTEGER_OR:
    case IR_INTEGER_XOR:
    case IR_INTEGER_COMPARE_EQUAL:
    case IR_INTEGER_COMPARE_NOT_EQUAL:
    case IR_INTEGER_NEGATION:
    case IR_INTEGER_CONSTANT:
    case MACHINE_COPY:
    case MACHINE_MOVE:
    case NODE_COUNT:
        return 0;
    case MACHINE_JUMP:
        todo();
    }
}

fn u8 node_has_memory_in(NodeId id)
{
    switch (id)
    {
    case IR_ROOT:
    case IR_RETURN:
        return 1;
    case IR_PROJECTION:
    case IR_REGION:
    case IR_PHI:
    case IR_SYMBOL_TABLE:
    case IR_INTEGER_ADD:
    case IR_INTEGER_SUBSTRACT:
    case IR_INTEGER_MULTIPLY:
    case IR_INTEGER_DIVIDE:
    case IR_INTEGER_REMAINDER:
    case IR_INTEGER_SHIFT_LEFT:
    case IR_INTEGER_SHIFT_RIGHT:
    case IR_INTEGER_AND:
    case IR_INTEGER_OR:
    case IR_INTEGER_XOR:
    case IR_INTEGER_COMPARE_EQUAL:
    case IR_INTEGER_COMPARE_NOT_EQUAL:
    case IR_INTEGER_NEGATION:
    case IR_INTEGER_CONSTANT:
    case MACHINE_COPY:
    case MACHINE_MOVE:
    case NODE_COUNT:
        return 0;
    case MACHINE_JUMP:
        todo();
    }
}

fn NodeIndex node_memory_in(Thread* thread, Node* node)
{
    auto result = invalidi(Node);
    if (node_has_memory_in(node->id))
    {
        result = node_get_inputs(thread, node).pointer[1];
    }

    return result;
}

fn s32 node_last_use_in_block(Thread* thread, VirtualBuffer(BasicBlockIndex) scheduled, Slice(s32) order, Node* node, BasicBlockIndex basic_block_index)
{
    auto outputs = node_get_outputs(thread, node);
    s32 result = 0;
    for (u16 i = 0; i < node->output_count; i += 1)
    {
        auto output_index = outputs.pointer[i];
        if (index_equal(basic_block_index, scheduled.pointer[geti(output_index)]) && result < order.pointer[geti(output_index)])
        {
            result = order.pointer[geti(output_index)];
        }
    }

    return result;
}

fn BasicBlockIndex find_use_block(Thread* thread, VirtualBuffer(BasicBlockIndex) scheduled, NodeIndex node_index, NodeIndex actual_node_index, NodeIndex use_index)
{
    auto use_block_index = scheduled.pointer[geti(use_index)];
    if (!validi(use_block_index))
    {
        return use_block_index;
    }

    Node* use = thread_node_get(thread, use_index);
    auto use_inputs = node_get_inputs(thread, use);

    if (use->id == IR_PHI)
    {
        auto use_first_input_index = use_inputs.pointer[0];
        auto use_first_input = thread_node_get(thread, use_first_input_index);
        assert(use_first_input->id == IR_REGION);
        assert(use->input_count == use_first_input->input_count + 1);

        auto use_first_input_inputs = node_get_inputs(thread, use_first_input);

        u16 i;
        for (i = 0; i < use_inputs.length; i += 1)
        {
            auto use_input_index = use_inputs.pointer[i];
            if (index_equal(use_input_index, actual_node_index))
            {
                // TODO: this assertion is mine for debugging when this function is only called from a single code path, 
                // it's not absolutely valid in other contexts
                assert(index_equal(actual_node_index, node_index));
                auto input_index = use_first_input_inputs.pointer[i - 1];
                auto bb_index = scheduled.pointer[geti(input_index)];
                if (validi(bb_index))
                {
                    use_block_index = bb_index;
                }
                break;
            }
        }

        assert(i < use_inputs.length);
    }

    return use_block_index;
}

fn BasicBlockIndex find_lca(BasicBlockIndex a, BasicBlockIndex b)
{
    unused(a);
    unused(b);
    // TODO: dominators
    return invalidi(BasicBlock);
}

fn u8 node_is_ready(Thread* thread, VirtualBuffer(BasicBlockIndex) scheduled, WorkListHandle handle, Node* node, BasicBlockIndex basic_block_index)
{
    // TODO: this is my assert and might not be true after all
    assert(node->input_capacity == node->input_count);
    auto inputs = node_get_inputs(thread, node);
    for (u16 i = 0; i < node->input_capacity; i += 1)
    {
        auto input = inputs.pointer[i];
        if (validi(input) && index_equal(scheduled.pointer[geti(input)], basic_block_index) && !thread_worklist_test(thread, handle, input))
        {
            return 0;
        }
    }

    return 1;
}

fn u64 node_get_latency(Thread* thread, Node* node, Node* end)
{
    unused(end);
    unused(thread);
    switch (node->id)
    {
        case IR_INTEGER_CONSTANT:
        case IR_RETURN:
        case MACHINE_COPY:
            return 1;
        case MACHINE_MOVE:
            return 0;
        default:
            todo();
    }
}

fn u64 node_get_unit_mask(Thread* thread, Node* node)
{
    unused(thread);
    unused(node);
    return 1;
}

STRUCT(ReadyNode)
{
    u64 unit_mask;
    NodeIndex node_index;
    s32 priority;
};
decl_vb(ReadyNode);

STRUCT(InFlightNode)
{
    NodeIndex node_index;
    u32 end;
    s32 unit_i;
};
decl_vb(InFlightNode);

STRUCT(Scheduler)
{
    Bitset ready_set;
    VirtualBuffer(ReadyNode) ready;
    NodeIndex cmp;
};

fn s32 node_best_ready(Scheduler* restrict scheduler, u64 in_use_mask)
{
    auto length = scheduler->ready.length;
    if (length == 1)
    {
        u64 available = scheduler->ready.pointer[0].unit_mask & ~in_use_mask;
        return available ? 0 : -1;
    }

    while (length--)
    {
        auto node_index = scheduler->ready.pointer[length].node_index;
        if (index_equal(node_index, scheduler->cmp))
        {
            continue;
        }

        auto available = scheduler->ready.pointer[length].unit_mask & ~in_use_mask;
        if (available == 0)
        {
            continue;
        }

        return cast(s32, u32, length);
    }

    return -1;
}

declare_ip_functions(RegisterMask, register_mask)

fn RegisterMaskIndex register_mask_intern(Thread* thread, RegisterMask register_mask)
{
    auto* new_rm = vb_add(&thread->buffer.register_masks, 1);
    *new_rm = register_mask;
    auto candidate_index = Index(RegisterMask, cast(u32, s64, new_rm - thread->buffer.register_masks.pointer));
    auto result = ip_RegisterMask_get_or_put(&thread->interned.register_masks, thread, candidate_index);
    auto final_index = result.index;
    assert((!index_equal(candidate_index, final_index)) == result.existing);
    thread->buffer.register_masks.length -= result.existing;

    return final_index;
}

fn RegisterMaskIndex node_constraint(Thread* thread, Node* node, Slice(RegisterMaskIndex) ins)
{
    switch (node->id)
    {
        case IR_PROJECTION:
            {
                auto backend_type = type_pair_get_backend(node->type);
                if (backend_type == BACKEND_TYPE_MEMORY || backend_type == BACKEND_TYPE_CONTROL)
                {
                    return empty_register_mask;
                }

                auto index = node->projection.index;
                auto inputs = node_get_inputs(thread, node);

                auto* first_input = thread_node_get(thread, inputs.pointer[0]);
                if (first_input->id == IR_ROOT)
                {
                    assert(index >= 2);
                    if (index == 2)
                    {
                        return empty_register_mask;
                    }
                    else
                    {
                        todo();
                    }
                    todo();
                }
                else
                {
                    todo();
                }
            } break;
        case IR_INTEGER_CONSTANT:
            return Index(RegisterMask, REGISTER_MASK_GPR);
        case MACHINE_MOVE:
            {
                // TODO: float
                auto mask =  Index(RegisterMask, REGISTER_MASK_GPR);
                if (ins.length)
                {
                    ins.pointer[1] = mask;
                }
                return mask;
            } break;
        case MACHINE_COPY:
            {
                if (ins.length)
                {
                    ins.pointer[1] = node->machine_copy.use_mask;
                }

                return node->machine_copy.def_mask;
            } break;
        case IR_REGION:
            {
                if (ins.length)
                {
                    for (u16 i = 1; i < node->input_count; i += 1)
                    {
                        ins.pointer[i] = empty_register_mask;
                    }
                }

                return empty_register_mask;
            } break;
        case IR_PHI:
            {
                if (ins.length)
                {
                    for (u16 i = 1; i < node->input_count; i += 1)
                    {
                        ins.pointer[i] = empty_register_mask;
                    }
                }

                auto backend_type = type_pair_get_backend(node->type);
                RegisterMaskIndex mask;
                if (backend_type == BACKEND_TYPE_MEMORY)
                {
                    mask = empty_register_mask;
                }
                // TODO: float
                else
                {
                    mask = Index(RegisterMask, REGISTER_MASK_GPR);
                }

                return mask;
            } break;
        case IR_RETURN:
            {
                if (ins.length)
                {
                    const global s32 ret_gprs[] = { RAX, RDX };

                    ins.pointer[1] = empty_register_mask;
                    ins.pointer[2] = empty_register_mask;

                    // TODO: returns
                    auto index = 3;
                    ins.pointer[index] = register_mask_intern(thread, (RegisterMask) {
                        .class = REGISTER_CLASS_X86_64_GPR,
                        .may_spill = 0,
                        .mask = ((u32)1 << ret_gprs[index - 3]),
                    });

                    auto gpr_caller_saved = ((1u << RAX) | (1u << RDI) | (1u << RSI) | (1u << RCX) | (1u << RDX) | (1u << R8) | (1u << R9) | (1u << R10) | (1u << R11));
                    auto gpr_callee_saved = ~gpr_caller_saved;
                    gpr_callee_saved &= ~(1u << RSP);
                    gpr_callee_saved &= ~(1u << RBP);

                    auto j = 3 + 1;

                    for (u32 i = 0; i < register_count_per_class[REGISTER_CLASS_X86_64_GPR]; i += 1)
                    {
                        if ((gpr_callee_saved >> i) & 1)
                        {
                            ins.pointer[j++] = register_mask_intern(thread, (RegisterMask) {
                                .class = REGISTER_CLASS_X86_64_GPR,
                                .mask = (u32)1 << i,
                                .may_spill = 0,
                            });
                        }
                    }

                    // TODO: float
                }

                return empty_register_mask;
            } break;
        default:
            todo();
    }
}

fn u32 node_tmp_count(Node* node)
{
    switch (node->id)
    {
    case IR_ROOT:
    case IR_PROJECTION:
    case IR_RETURN:
    case IR_REGION:
    case IR_PHI:
    case IR_SYMBOL_TABLE:
    case IR_INTEGER_ADD:
    case IR_INTEGER_SUBSTRACT:
    case IR_INTEGER_MULTIPLY:
    case IR_INTEGER_DIVIDE:
    case IR_INTEGER_REMAINDER:
    case IR_INTEGER_SHIFT_LEFT:
    case IR_INTEGER_SHIFT_RIGHT:
    case IR_INTEGER_AND:
    case IR_INTEGER_OR:
    case IR_INTEGER_XOR:
    case IR_INTEGER_COMPARE_EQUAL:
    case IR_INTEGER_COMPARE_NOT_EQUAL:
    case IR_INTEGER_NEGATION:
    case IR_INTEGER_CONSTANT:
    case MACHINE_COPY:
    case MACHINE_MOVE:
    case NODE_COUNT:
        return 0;
    case MACHINE_JUMP:
        todo();
    }
}

STRUCT(VirtualRegister)
{
    RegisterMaskIndex mask;
    NodeIndex node_index;
    f32 spill_cost;
    f32 spill_bias;
    s16 class;
    s16 assigned;
    s32 hint_vreg;
};
decl_vb(VirtualRegister);


fn s32 fixed_register_mask(RegisterMask mask)
{
    if (mask.class == REGISTER_CLASS_STACK)
    {
        todo();
    }
    else
    {
        s32 set = -1;
        // TODO: count?
        for (s32 i = 0; i < 1; i += 1)
        {
            u32 m = mask.mask;
            s32 found = 32 - __builtin_clz(m);
            if (m == ((u32)1 << found))
            {
                if (set >= 0)
                {
                    return -1;
                }

                set = i * 64 + found;
            }
        }

        return set;
    }
}

fn RegisterMaskIndex register_mask_meet(Thread* thread, RegisterMaskIndex a_index, RegisterMaskIndex b_index)
{
    if (index_equal(a_index, b_index))
    {
        return a_index;
    }

    if (!validi(a_index))
    {
        return b_index;
    }
    if (!validi(b_index))
    {
        return a_index;
    }

    auto* a = thread_register_mask_get(thread, a_index);
    auto* b = thread_register_mask_get(thread, b_index);

    u32 may_spill = a->may_spill && b->may_spill;
    if (!may_spill && a->class != b->class)
    {
        return empty_register_mask;
    }

    auto a_mask = a->mask;
    auto b_mask = b->mask;
    auto mask = a_mask & b_mask;

    auto result = register_mask_intern(thread, (RegisterMask) {
        .class = mask == 0 ? 1 : a->class,
        .may_spill = may_spill,
        .mask = mask,
    });

    return result;
}

fn s32 node_to_address(Thread* thread, NodeIndex node_index)
{
    auto* node = thread_node_get(thread, node_index);
    switch (node->id)
    {
        case IR_PHI:
        case IR_INTEGER_CONSTANT:
            return -1;
        case MACHINE_COPY:
            return 1;
        default:
            todo();
    }
}

fn u8 interfere_in_block(Thread* thread, VirtualBuffer(BasicBlockIndex) scheduled, VirtualBuffer(BasicBlock) bb, Slice(s32) order, NodeIndex left, NodeIndex right, BasicBlockIndex block_index)
{
    assert(!index_equal(left, right));
    auto* block = &bb.pointer[geti(block_index)];
    auto left_live_out = bitset_get(&block->live_out, geti(left));
    auto right_live_out = bitset_get(&block->live_out, geti(right));
    auto* left_node = thread_node_get(thread, left);
    auto* right_node = thread_node_get(thread, right);

    if (left_node->id == IR_PHI || right_node->id == IR_PHI)
    {
        auto phi = right;
        auto other = left;

        if (left_node->id == IR_PHI && right_node->id != IR_PHI)
        {
            phi = left;
            other = right;
        }

        unused(other);

        block_index = scheduled.pointer[geti(phi)];
        block = &bb.pointer[geti(block_index)];
        if (bitset_get(&block->live_out, geti(phi)))
        {
            todo();
        }
    }

    if (left_live_out && right_live_out)
    {
        todo();
    }
    else if (!left_live_out && !right_live_out)
    {
        auto first = left;
        auto last = right;

        if (order.pointer[geti(left)] > order.pointer[geti(right)])
        {
            first = right;
            last = left;
        }

        block_index = scheduled.pointer[geti(last)];
        block = &bb.pointer[geti(block_index)];

        auto* first_node = thread_node_get(thread, first);
        auto outputs = node_get_outputs(thread, first_node);
        
        for (u16 i = 0; i < first_node->output_count; i += 1)
        {
            auto output_index = outputs.pointer[i];
            assert(validi(output_index));
            auto* output_node = thread_node_get(thread, output_index);
            auto output_inputs = node_get_inputs(thread, output_node);

            u16 i;
            for (i = 0; i < output_node->input_count; i += 1)
            {
                auto input_index = output_inputs.pointer[i];
                if (index_equal(input_index, first))
                {
                    if (index_equal(block_index, scheduled.pointer[geti( output_index)]))
                    {
                        if (order.pointer[geti(output_index)] > order.pointer[geti(last)])
                        {
                            return 1;
                        }
                    }
                    break;
                }
            }
            assert(i < output_node->input_count);
        }
    }
    else
    {
        todo();
    }

    return 0;
}

fn u8 interfere(Thread* thread, VirtualBuffer(BasicBlockIndex) scheduled, VirtualBuffer(BasicBlock) bb, Slice(s32) order, NodeIndex left, NodeIndex right)
{
    auto left_block = scheduled.pointer[geti(left)];
    auto right_block = scheduled.pointer[geti(right)];
    // These asserts are mine, they might not be valid
    assert(validi(left_block));
    assert(validi(right_block));

    auto result = interfere_in_block(thread, scheduled, bb, order, left, right, left_block);

    if (!index_equal(left_block, right_block))
    {
        result = result || interfere_in_block(thread, scheduled, bb, order, right, left, right_block);
    }

    return result;
}

fn Slice(s32) compute_ordinals(Thread* thread, VirtualBuffer(BasicBlock) bb, u32 node_count)
{
    auto order_cap = round_up_to_next_power_of_2(node_count);
    auto order = arena_allocate(thread->arena, s32, order_cap);

    for (u32 i = 0; i < bb.length; i += 1)
    {
        auto* basic_block = & bb.pointer[i];
        s32 timeline = 1;
        for (u32 i = 0; i < basic_block->items.length; i += 1)
        {
            auto node_index = basic_block->items.pointer[i];
            order[geti(node_index)] = timeline;
            timeline += 1;
        }
    }

    return (Slice(s32)) { .pointer = order, .length = order_cap, };
}

fn u8 can_remat(Thread* thread, NodeIndex node_index)
{
    auto* node = thread_node_get(thread, node_index);
    switch (node->id)
    {
        case MACHINE_COPY:
            return 1;
        default:
            todo();
    }
}

may_be_unused fn f32 get_spill_cost(Thread* thread, VirtualRegister* virtual_register)
{
    auto spill_cost = virtual_register->spill_cost;
    if (__builtin_isnan(spill_cost))
    {
        if (can_remat(thread, virtual_register->node_index))
        {
            spill_cost = virtual_register->spill_bias - 1.0f;
        }
        else
        {
            todo();
        }

        virtual_register->spill_cost = spill_cost;
    }

    return spill_cost;
}

fn u8 register_mask_not_empty(RegisterMask mask)
{
    return mask.mask != 0;
}

fn u8 register_mask_spill(RegisterMask mask)
{
    return mask.class != REGISTER_CLASS_STACK && (!register_mask_not_empty(mask) && mask.may_spill);
}

fn void dataflow(Thread* thread, WorkListHandle worker, VirtualBuffer(BasicBlock) bb, VirtualBuffer(BasicBlockIndex) scheduled, u32 node_count)
{
    // Dataflow analysis
    thread_worklist_clear(thread, worker);

    // TODO: separate per function
    for (u32 i = 0; i < bb.length; i += 1)
    {
        BasicBlock* basic_block = &bb.pointer[i];

        bitset_clear(&basic_block->gen);
        bitset_clear(&basic_block->kill);

        bitset_ensure_length(&basic_block->gen, node_count);
        bitset_ensure_length(&basic_block->kill, node_count);
    }

    for (u32 i = 0; i < bb.length; i += 1)
    {
        BasicBlock* basic_block = &bb.pointer[i];

        for (u32 i = 0; i < basic_block->items.length; i += 1)
        {
            NodeIndex node_index = basic_block->items.pointer[i];
            Node* node = thread_node_get(thread, node_index);

            if (node->id == IR_PHI)
            {
                auto phi_inputs = node_get_inputs(thread, node);
                for (u16 i = 1; i < phi_inputs.length; i += 1)
                {
                    auto input = phi_inputs.pointer[i];
                    if (validi(input))
                    {
                        auto input_bb_index = scheduled.pointer[geti(input)];
                        bitset_set_value(&bb.pointer[geti(input_bb_index)].kill, geti(node_index), 1);
                    }
                }
            }
            else
            {
                bitset_set_value(&basic_block->kill, geti(node_index), 1);
            }
        }
    }

    for (u32 i = 0; i < bb.length; i += 1)
    {
        BasicBlock* basic_block = &bb.pointer[i];

        for (u32 i = 0; i < basic_block->items.length; i += 1)
        {
            NodeIndex node_index = basic_block->items.pointer[i];
            Node* node = thread_node_get(thread, node_index);

            if (node->id != IR_PHI)
            {
                auto inputs = node_get_inputs(thread, node);
                for (u16 i = 1; i < inputs.length; i += 1)
                {
                    auto input_index = inputs.pointer[i];
                    if (validi(input_index))
                    {
                        auto* input = thread_node_get(thread, input_index);
                        if (input->id == IR_PHI || !bitset_get(&basic_block->kill, geti(input_index)))
                        {
                            bitset_set_value(&basic_block->gen, geti(input_index), 1);
                        }
                    }
                }
            }
        }
    }

    thread_worklist_clear(thread, worker);

    for (u32 i = 0; i < bb.length; i += 1)
    {
        BasicBlock* basic_block = &bb.pointer[i];
        assert(basic_block->gen.arr.length == basic_block->live_in.arr.length);
        assert(basic_block->gen.arr.capacity == basic_block->live_in.arr.capacity);
        memcpy(basic_block->live_in.arr.pointer, basic_block->gen.arr.pointer, sizeof(basic_block->gen.arr.pointer[0]) * basic_block->gen.arr.length);
        basic_block->live_in.length = basic_block->gen.length;
        thread_worklist_push(thread, worker, basic_block->start);
    }

    while (thread_worklist_length(thread, worker) > 0)
    {
        auto bb_node_index = thread_worklist_pop(thread, worker);
        auto basic_block_index = scheduled.pointer[geti(bb_node_index)];
        BasicBlock* basic_block = &bb.pointer[geti(basic_block_index)];

        auto* live_out = &basic_block->live_out;
        auto* live_in = &basic_block->live_in;
        bitset_clear(live_out);

        auto end_index = basic_block->end;
        auto* end = thread_node_get(thread, end_index);
        auto cfg_is_fork = 0;
        if (cfg_is_fork)
        {
            todo();
        }
        else if (!cfg_is_endpoint(thread, end))
        {
            auto succ_index = cfg_next_control(thread, end_index);
            auto succ_bb_index = scheduled.pointer[geti(succ_index)];
            auto succ_bb = &bb.pointer[geti(succ_bb_index)];
            assert(live_out->arr.capacity == live_in->arr.capacity);
            u64 changes = 0;
            for (u32 i = 0; i < succ_bb->live_in.arr.capacity; i += 1)
            {
                auto old = live_out->arr.pointer[i];
                auto new = old | succ_bb->live_in.arr.pointer[i];

                live_out->arr.pointer[i] = new;
                changes |= (old ^ new);
            }
            unused(changes);
        }

        auto* gen = &basic_block->gen;
        auto* kill = &basic_block->kill;

        auto changes = 0;

        for (u32 i = 0; i < kill->arr.length; i += 1)
        {
            u64 new_in = (live_out->arr.pointer[i] & ~kill->arr.pointer[i]) | gen->arr.pointer[i];
            changes |= live_in->arr.pointer[i] != new_in;
            live_in->arr.pointer[i] = new_in;
        }

        if (changes)
        {
            todo();
        }
    }
}

fn void redo_dataflow(Thread* thread, WorkListHandle worker, VirtualBuffer(BasicBlock) bb, VirtualBuffer(BasicBlockIndex) scheduled, u32 node_count)
{
    for (u32 i = 0; i < bb.length; i += 1)
    {
        BasicBlock* basic_block = &bb.pointer[i];

        bitset_clear(&basic_block->gen);
        bitset_clear(&basic_block->kill);

        bitset_ensure_length(&basic_block->gen, node_count);
        bitset_ensure_length(&basic_block->kill, node_count);
    }

    dataflow(thread, worker, bb, scheduled, node_count);
}

fn String gpr_to_string(GPR gpr)
{
    switch (gpr)
    {
        case_to_name(, RAX);
        case_to_name(, RCX);
        case_to_name(, RDX);
        case_to_name(, RBX);
        case_to_name(, RSP);
        case_to_name(, RBP);
        case_to_name(, RSI);
        case_to_name(, RDI);
        case_to_name(, R8);
        case_to_name(, R9);
        case_to_name(, R10);
        case_to_name(, R11);
        case_to_name(, R12);
        case_to_name(, R13);
        case_to_name(, R14);
        case_to_name(, R15);
        case_to_name(, GPR_NONE);
    }
}

fn u8 register_allocate(Thread* thread, VirtualBuffer(VirtualRegister) virtual_registers, VirtualBuffer(u32)* spills, Bitset* active, Bitset* future_active, VirtualBuffer(BasicBlockIndex) scheduled, VirtualBuffer(BasicBlock) bb, Slice(s32) order, u32 virtual_register_id, u32 in_use)
{
    if (bitset_get(future_active, virtual_register_id))
    {
        todo();
    }

    auto* virtual_register = &virtual_registers.pointer[virtual_register_id];
    auto mask = thread_register_mask_get(thread, virtual_register->mask);

    if (virtual_register->assigned >= 0)
    {
        bitset_set_value(active, virtual_register_id, 1);
        return 1;
    }
    else if (register_mask_spill(*mask))
    {
        todo();
    }
    else if (mask->class == REGISTER_CLASS_STACK)
    {
        todo();
    }

    auto mask_value = mask->mask;
    auto old_in_use = in_use;
    in_use |= ~mask_value;
    print("Vreg mask: {u32:x}. Complement: {u32:x}. In use before: {u32:x}. In use after: {u32:x}\n", mask_value, ~mask_value, old_in_use, in_use);

    spills->length = 0;

    *vb_add(spills, 1) = virtual_register_id;

    FOREACH_SET(i, active)
    {
        print("Active[{u64}] set\n", i);
        VirtualRegister* other = &virtual_registers.pointer[i];
        if (other->class == mask->class)
        {
            print("Interfere with active: {u32}\n", (s32)other->assigned);
            in_use |= ((u32)1 << other->assigned);
            *vb_add(spills, 1) = cast(u32, u64, i);
        }
    }

    FOREACH_SET(i, future_active)
    {
        print("Future active[{u64}] set\n", i);
        VirtualRegister* other = &virtual_registers.pointer[i];
        if (other->class == mask->class && (in_use & ((u32)1 << other->assigned)) == 0)
        {
            if (interfere(thread, scheduled, bb, order, virtual_register->node_index, other->node_index))
            {
                todo();
            }
        }
    }

    NodeIndex node_index = virtual_register->node_index;
    auto hint_vreg = virtual_register->hint_vreg;
    auto shared_edge = node_to_address(thread, node_index);

    if (shared_edge >= 0)
    {
        todo();
    }

    if (in_use == UINT32_MAX)
    {
        return 0;
    }

    virtual_register->class = mask->class;

    auto hint_virtual_register = virtual_registers.pointer[hint_vreg];

    s32 hint_reg = hint_vreg > 0 && hint_virtual_register.class == mask->class ?
        hint_virtual_register.assigned : -1;

    print("IN USE: {u32:x}: ~ -> {u32:x}\n", in_use, ~in_use);

    if (hint_reg >= 0 && (in_use & ((u64)1 << hint_reg)) == 0)
    {
        todo();
    }
    else
    {
        virtual_register->assigned = cast(s16, s32, __builtin_ffsll(~in_use) - 1);
        print("Register assigned: {s}\n", gpr_to_string((GPR)virtual_register->assigned));
    }

    bitset_set_value(active, virtual_register_id, 1);

    return 1;
}

fn s32 machine_operand_at(u32* virtual_register_map, VirtualBuffer(VirtualRegister) virtual_registers, NodeIndex node_index, s32 class)
{
    assert(validi(node_index));
    auto virtual_register_id = virtual_register_map[geti(node_index)];
    assert(virtual_register_id > 0);
    assert(virtual_register_id < virtual_registers.length);
    auto* virtual_register = &virtual_registers.pointer[virtual_register_id];
    assert(virtual_register->assigned >= 0);
    assert(virtual_register->class == class);

    return virtual_register->assigned;
}

typedef enum MachineOperandId : u8
{
    MACHINE_OPERAND_MEMORY,
    MACHINE_OPERAND_GPR,
    MACHINE_OPERAND_XMM,
} MachineOperandId;

STRUCT(MachineOperand)
{
    MachineOperandId id;
    s16 register_value;
};

fn MachineOperand operand_from_node(VirtualBuffer(VirtualRegister) virtual_registers, u32* virtual_register_map, NodeIndex node_index)
{
    assert(validi(node_index));
    auto virtual_register_id = virtual_register_map[geti(node_index)];
    assert(virtual_register_id > 0);
    auto* virtual_register = &virtual_registers.pointer[virtual_register_id];

    if (virtual_register->class == REGISTER_CLASS_STACK)
    {
        todo();
    }
    else
    {
        assert(virtual_register->assigned >= 0);
        MachineOperandId id;
        switch (virtual_register->class)
        {
            case REGISTER_CLASS_X86_64_GPR:
                id = MACHINE_OPERAND_GPR;
                break;
            default:
                todo();
        }

        return (MachineOperand) {
            .id = id,
            .register_value = virtual_register->assigned,
        };
    }

    todo();
}

fn void node_ready_up(Thread* thread, Scheduler* scheduler, NodeIndex node_index, Node* end)
{
    auto* node = thread_node_get(thread, node_index);
    auto priority = node_get_latency(thread, node, end);
    auto unit_mask = node_get_unit_mask(thread, node);

    bitset_set_value(&scheduler->ready_set, geti(node_index), 1);

    if (type_pair_get_backend(node->type) == BACKEND_TYPE_TUPLE)
    {
        todo();
    }

    u32 i;
    auto count = scheduler->ready.length;

    for (i = 0; i < count; i += 1)
    {
        if (cast(s32, u64, priority) < scheduler->ready.pointer[i].priority)
        {
            break;
        }
    }

    *vb_add(&scheduler->ready, 1) = (ReadyNode){};

    memmove(&scheduler->ready.pointer[i + 1], &scheduler->ready.pointer[i], (count - i) * sizeof(ReadyNode));

    // print("Readying up node #{u32} ({s}) at index {u32}\n", geti(node_index), node_id_to_string(node->id), i);

    scheduler->ready.pointer[i] = (ReadyNode) {
        .node_index = node_index,
            .priority = cast(s32, u64, priority),
            .unit_mask = unit_mask,
    };
}

STRUCT(FixedBlockMap)
{
    NodeIndex* keys;
    u32 count;
};

fn FixedBlockMap fixed_block_map_create(Thread* restrict thread, u32 count)
{
    auto* pointer = arena_allocate_bytes(thread->arena, sizeof(NodeIndex) * count + sizeof(BasicBlockIndex) * count, MAX(alignof(BasicBlockIndex), alignof(NodeIndex)));
    return (FixedBlockMap) {
        .keys = (NodeIndex*)pointer,
        .count = count,
    };
}

fn BasicBlockIndex* fixed_block_map_values(FixedBlockMap* restrict map)
{
    return (BasicBlockIndex*)(map->keys + map->count);
}

fn void fixed_block_map_put(FixedBlockMap* restrict map, NodeIndex key, BasicBlockIndex value)
{
    auto count = map->count;
    for (u32 i = 0; i < count; i += 1)
    {
        if (index_equal(key, map->keys[i]))
        {
            fixed_block_map_values(map)[i] = value;
            break;
        }
        else if (!validi(map->keys[i]))
        {
            map->keys[i] = key;
            fixed_block_map_values(map)[i] = value;
            break;
        }
    }
}

fn BasicBlockIndex fixed_block_map_get(FixedBlockMap* restrict map, NodeIndex key)
{
    auto count = map->count;
    for (u32 i = 0; i < count; i += 1)
    {
        if (index_equal(key, map->keys[i]))
        {
            return fixed_block_map_values(map)[i];
        }
    }

    return invalidi(BasicBlock);
}

STRUCT(CFGBuilder)
{
    VirtualBuffer(NodeIndex) pinned;
    VirtualBuffer(BasicBlock) basic_blocks;
    VirtualBuffer(BasicBlockIndex) scheduled;
    FixedBlockMap block_map;
    WorkListHandle walker;
    WorkListHandle worker;
};

fn CFGBuilder cfg_builder_init(Thread* restrict thread)
{
    CFGBuilder cfg_builder = {};
    cfg_builder.walker = thread_worklist_acquire(thread);
    cfg_builder.worker = thread_worklist_acquire(thread);

    return cfg_builder;
}

fn void cfg_builder_clear(CFGBuilder* restrict builder, Thread* restrict thread)
{
    thread_worklist_clear(thread, builder->walker);
    thread_worklist_clear(thread, builder->worker);

    builder->pinned.length = 0;
    builder->basic_blocks.length = 0;
    builder->scheduled.length = 0;
}

STRUCT(CodegenOptions)
{
    String test_name;
    CompilerBackend backend;
};

fn BasicBlockIndex cfg_get_predicate_basic_block(Thread* restrict thread, FixedBlockMap* map, NodeIndex arg_node_index, u16 i)
{
    auto* arg_node = thread_node_get(thread, arg_node_index);
    auto arg_inputs = node_get_inputs(thread, arg_node);
    auto node_index = arg_inputs.pointer[i];

    while (1)
    {
        auto* node = thread_node_get(thread, node_index);

        auto search = fixed_block_map_get(map, node_index);

        if (validi(search))
        {
            return search;
        }
        else 
        {
            // TODO: or dead
            if (node->id == IR_REGION)
            {
                return invalidi(BasicBlock);
            }
        }

        auto inputs = node_get_inputs(thread, node);
        node_index = inputs.pointer[0];
    }

    unreachable();
}

fn void cfg_build(CFGBuilder* restrict builder, Thread* restrict thread, Function* restrict function)
{
    thread_worklist_push(thread, builder->worker, function->root);

    for (u32 i = 0; i < thread_worklist_length(thread, builder->worker); i += 1)
    {
        NodeIndex node_index = thread_worklist_get(thread, builder->worker, i);
        Node* node = thread_node_get(thread, node_index);
        auto pin = 0;
        switch (node->id)
        {
            case IR_ROOT:
            case IR_PHI:
            case IR_RETURN:
            case IR_REGION:
                pin = 1;
                break;
            case IR_PROJECTION:
            case IR_INTEGER_CONSTANT:
            case IR_SYMBOL_TABLE:
                break;
            default:
                todo();
        }

        if (pin)
        {
            *vb_add(&builder->pinned, 1) = node_index;
        }

        auto outputs = node_get_outputs(thread, node);
        for (u16 i = 0; i < outputs.length; i += 1)
        {
            auto output = outputs.pointer[i];
            assert(validi(output));
            thread_worklist_push(thread, builder->worker, output);
        }
    }

    thread_worklist_clear(thread, builder->worker);

    for (u64 pin_index = 0; pin_index < builder->pinned.length; pin_index += 1)
    {
        auto pinned_node_index = builder->pinned.pointer[pin_index];
        thread_worklist_push(thread, builder->walker, pinned_node_index);
        // auto* pinned_node = thread_node_get(thread, pinned_node_index);

        while (thread_worklist_length(thread, builder->walker) > 0)
        {
            auto node_index = thread_worklist_pop_array(thread, builder->walker);
            auto* node = thread_node_get(thread, node_index);
            assert(node->interned);
            auto inputs = node_get_inputs(thread, node);
            auto outputs = node_get_outputs(thread, node);

            if (node->id != IR_PROJECTION && node->output_count == 0)
            {
                todo();
            }

            if (type_pair_get_backend(node->type) == BACKEND_TYPE_MEMORY)
            {
                for (u16 i = 0; i < outputs.length; i += 1)
                {
                    auto output_index = outputs.pointer[i];
                    auto* output = thread_node_get(thread, output_index);
                    if (output->output_count == 0)
                    {
                        thread_worklist_push(thread, builder->worker, output_index);
                    }
                }
            }

            node_gvn_remove(thread, node_index);

            auto new_node_index = node_select_instruction(thread, node_index);

            if (validi(new_node_index) && !index_equal(node_index, new_node_index))
            {
                todo();
            }

            for (auto i = inputs.length; i > 0; i -= 1)
            {
                auto input = inputs.pointer[i - 1];
                if (validi(input))
                {
                    thread_worklist_push(thread, builder->walker, input);
                }
            }

            if (node->id == IR_REGION)
            {
                for (u16 i = 0; i < outputs.length; i += 1)
                {
                    auto output_index = outputs.pointer[i];
                    assert(validi(output_index));
                    auto output = thread_node_get(thread, output_index);
                    if (output->id)
                    {
                        thread_worklist_push(thread, builder->walker, output_index);
                    }
                }
            }
        }
    }

    auto control_start = function_get_control_start(thread, function);
    auto* top = create_block(thread, control_start);
    thread_worklist_clear(thread, builder->worker);
    thread_worklist_test_and_set(thread, builder->worker, control_start);

    while (top)
    {
        auto successor_count = top->successor_count;
        if (successor_count > 0)
        {
            auto index = successor_count - 1;
            auto node_index = top->successors[index];
            assert(validi(node_index));
            top->successor_count = index;

            // Returns valid when the node hasnt been pushed to the worklist yet
            if (!thread_worklist_test_and_set(thread, builder->worker, node_index))
            {
                auto* new_top = create_block(thread, node_index);
                new_top->parent = top;
                top = new_top;
            }
        }
        else
        {
            Block* parent = top->parent;
            *vb_add(&builder->basic_blocks, 1) = (BasicBlock) {
                .start = top->start,
                .end = top->end,
                .dominator_depth = -1,
            };

            top = parent;
        }
    }

    for (u32 i = 0; i < builder->basic_blocks.length / 2; i += 1)
    {
        SWAP(builder->basic_blocks.pointer[i], builder->basic_blocks.pointer[(builder->basic_blocks.length - 1) - i]);
    }

    auto* blocks = builder->basic_blocks.pointer;
    blocks[0].dominator_depth = 0;
    blocks[0].dominator = Index(BasicBlock, 0);
    auto block_count = builder->basic_blocks.length;

    builder->block_map = fixed_block_map_create(thread, block_count);
    for (u32 i = 0; i < block_count; i += 1)
    {
        auto* block = &blocks[i];
        auto block_index = Index(BasicBlock, i);
        fixed_block_map_put(&builder->block_map, block->start, block_index);
    }

    // Compute dominators
    u8 changed = 1;
    while (changed)
    {
        changed = 0;
        
        for (u32 i = 1; i < block_count; i += 1)
        {
            // auto basic_block_index = Index(BasicBlock, i);
            auto* basic_block = &blocks[i];

            auto new_immediate_dominator_index = invalidi(BasicBlock);

            auto start_index = basic_block->start;
            auto* start_node = thread_node_get(thread, start_index);

            // auto start_inputs = node_get_inputs(thread, start_node);

            for (u16 j = 0; j < start_node->input_count; j += 1)
            {
                auto predecessor_basic_block_index = cfg_get_predicate_basic_block(thread, &builder->block_map, start_index, j);
                if (validi(predecessor_basic_block_index))
                {
                    auto* predecessor_basic_block = &blocks[geti(predecessor_basic_block_index)];
                    auto immediate_dominator_predecessor_index = predecessor_basic_block->dominator;
                    if (validi(immediate_dominator_predecessor_index))
                    {
                        if (validi(new_immediate_dominator_index))
                        {
                            todo();
                        }
                        else
                        {
                            new_immediate_dominator_index = predecessor_basic_block_index;
                        }
                    }
                }
            }

            assert(validi(new_immediate_dominator_index));
            if (!index_equal(basic_block->dominator, new_immediate_dominator_index))
            {
                basic_block->dominator = new_immediate_dominator_index;
                changed = 1;
            }
        }
    }

    // Compute the depths
    for (u32 i = 0; i < block_count; i += 1)
    {
        auto basic_block_index = Index(BasicBlock, i);
        auto* basic_block = &blocks[geti(basic_block_index)];
        auto current_index = basic_block_index;
        s32 depth = 0;
        while (1)
        {
            auto* current = &blocks[geti(current_index)];
            if (current->dominator_depth >= 0)
            {
                break;
            }

            current_index = current->dominator;
            depth += 1;
        }

        auto* current = &blocks[geti(current_index)];
        basic_block->dominator_depth = depth + current->dominator_depth;
    }
}

STRUCT(GlobalScheduleOptions)
{
    u8 dataflow:1;
};

fn void basic_block_add_node(Thread* restrict thread, BasicBlock* restrict basic_block, NodeIndex node_index, u32 place)
{
    // if (geti(node_index) == 1)
    // {
    //     breakpoint();
    // }
    print("[PLACE #{u32}] Adding node #{u32} ({s}) to basic block 0x{u64:x} with index {u32}\n", place, geti(node_index), node_id_to_string(thread_node_get(thread, node_index)->id), basic_block, basic_block->items.length);
    *vb_add(&basic_block->items, 1) = node_index;
}

fn void cfg_global_schedule(CFGBuilder* restrict builder, Thread* restrict thread, Function* restrict function, GlobalScheduleOptions options)
{
    // Global code motion

    auto node_count = thread->buffer.nodes.length;
    vb_add(&builder->scheduled, thread->buffer.nodes.length);

    for (u32 i = 0; i < builder->basic_blocks.length; i += 1)
    {
        auto* basic_block = &builder->basic_blocks.pointer[i];
        bitset_ensure_length(&basic_block->live_in, node_count);
        bitset_ensure_length(&basic_block->live_out, node_count);
    }

    auto bb0 = Index(BasicBlock, cast(u32, s64, &builder->basic_blocks.pointer[0] - builder->basic_blocks.pointer));

    for (u32 i = 0; i < builder->basic_blocks.length; i += 1)
    {
        auto* basic_block = &builder->basic_blocks.pointer[i];
        auto bb_index = Index(BasicBlock, cast(u32, s64, basic_block - builder->basic_blocks.pointer));
        builder->scheduled.pointer[geti(basic_block->start)] = bb_index;

        if (i == 0)
        {
            auto* root_node = thread_node_get(thread, function->root);
            auto outputs = node_get_outputs(thread, root_node);

            for (u16 i = 0; i < outputs.length; i += 1)
            {
                auto output = outputs.pointer[i];
                builder->scheduled.pointer[geti(output)] = bb0;
                basic_block_add_node(thread, &builder->basic_blocks.pointer[0], output, 0);
            }
        }

        auto* start = thread_node_get(thread, basic_block->start);
        if (start->id == IR_REGION)
        {
            basic_block_add_node(thread, basic_block, basic_block->start, 1);

            auto outputs = node_get_outputs(thread, start);

            for (u16 i = 0; i < outputs.length; i += 1)
            {
                auto output = outputs.pointer[i];
                auto* output_node = thread_node_get(thread, output);
                if (output_node->id == IR_PHI)
                {
                    builder->scheduled.pointer[geti(output)] = bb_index;
                    basic_block_add_node(thread, basic_block, output, 2);
                }
            }
        }
    }

    thread_worklist_clear(thread, builder->worker);
    thread_worklist_push(thread, builder->worker, function->root);

    VirtualBuffer(NodeIndex) pins = {};

    for (u32 i = 0; i < thread_worklist_length(thread, builder->worker); i += 1)
    {
        auto node_index = thread_worklist_get(thread, builder->worker, i);
        auto* node = thread_node_get(thread, node_index);
        if (node->id != IR_ROOT && node_is_pinned(node))
        {
            auto bb_index = builder->scheduled.pointer[geti(node_index)];
            if (node->id == IR_PROJECTION && !node_is_pinned(thread_node_get(thread, node_get_inputs(thread, node).pointer[0])))
            {
            }
            else
            {
                auto current = node_index;
                while (!validi(bb_index))
                {
                    bb_index = builder->scheduled.pointer[geti(current)];
                    auto* current_node = thread_node_get(thread, current);
                    auto current_inputs = node_get_inputs(thread, current_node);
                    current = current_inputs.pointer[0];
                }

                auto* basic_block = &builder->basic_blocks.pointer[geti(bb_index)];
                builder->scheduled.pointer[geti(node_index)] = bb_index;
                *vb_add(&pins, 1) = node_index;
                basic_block_add_node(thread, basic_block, node_index, 3);
            }
        }

        auto outputs = node_get_outputs(thread, node);
        for (u16 i = 0; i < outputs.length; i += 1)
        {
            auto output = outputs.pointer[i];
            thread_worklist_push(thread, builder->worker, output);
        }
    }

    // Early schedule
    thread_worklist_clear(thread, builder->worker);

    for (u32 i = 0; i < pins.length; i += 1)
    {
        auto pin_node_index = pins.pointer[i];
        auto* pin = thread_node_get(thread, pin_node_index);

        STRUCT(Elem)
        {
            struct Elem* parent;
            NodeIndex node;
            u32 i;
        };

        auto* top = arena_allocate(thread->arena, Elem, 1);
        *top = (Elem)
        {
            .node = pin_node_index,
                .parent = 0,
                .i = pin->input_count,
        };

        while (top)
        {
            NodeIndex node_index = top->node;
            Node* node = thread_node_get(thread, node_index);
            auto node_inputs = node_get_inputs(thread, node);

            if (top->i > 0)
            {
                auto new_top_i = top->i - 1;
                top->i = new_top_i;
                NodeIndex input_index = node_inputs.pointer[new_top_i];

                if (validi(input_index))
                {
                    Node* input = thread_node_get(thread, input_index);
                    if (input->id == IR_PROJECTION)
                    {
                        auto input_inputs = node_get_inputs(thread, input);
                        input_index = input_inputs.pointer[0];
                        input = thread_node_get(thread, input_index);
                    }

                    if (!node_is_pinned(input) && !thread_worklist_test_and_set(thread, builder->worker, input_index))
                    {
                        auto* new_top = arena_allocate(thread->arena, Elem, 1);
                        *new_top = (Elem)
                        {
                            .parent = top,
                                .node = input_index,
                                .i = input->input_count,
                        };

                        top = new_top;
                    }
                }

                continue;
            }

            if (!index_equal(node_index, pin_node_index))
            {
                auto best = Index(BasicBlock, 0);
                s32 best_depth = 0;

                auto inputs = node_get_inputs(thread, node);

                for (u16 i = 0; i < node->input_count; i += 1)
                {
                    auto input_index = inputs.pointer[i];
                    if (validi(input_index))
                    {
                        auto basic_block_index = builder->scheduled.pointer[geti(input_index)];
                        if (validi(basic_block_index))
                        {
                            auto* basic_block = &builder->basic_blocks.pointer[geti(basic_block_index)];

                            if (best_depth < basic_block->dominator_depth)
                            {
                                best_depth = basic_block->dominator_depth;
                                best = basic_block_index;
                            }
                        }
                    }
                }

                builder->scheduled.pointer[geti(node_index)] = best;

                if (type_pair_get_backend(node->type) == BACKEND_TYPE_TUPLE)
                {
                    todo();
                }

                thread_worklist_push_array(thread, builder->worker, node_index);
            }

            top = top->parent;
        }
    }

    // Late schedule
    for (u32 i = thread_worklist_length(thread, builder->worker); i > 0; i -= 1)
    {
        auto node_index = thread_worklist_get(thread, builder->worker, i - 1);
        auto* node = thread_node_get(thread, node_index);
        assert(!node_is_pinned(node));
        auto current_basic_block_index = builder->scheduled.pointer[geti(node_index)];
        auto current_basic_block = &builder->basic_blocks.pointer[geti(current_basic_block_index)];

        auto lca = invalidi(BasicBlock);

        if (!node_has_memory_out(node->id))
        {
            auto memory_in = node_memory_in(thread, node);
            if (validi(memory_in))
            {
                todo();
            }
        }

        if (type_pair_get_backend(node->type) == BACKEND_TYPE_TUPLE)
        {
            todo();
        }
        else
        {
            auto outputs = node_get_outputs(thread, node);
            for (u16 i = 0; i < outputs.length; i += 1)
            {
                auto output = outputs.pointer[i];
                auto use_block_index = find_use_block(thread, builder->scheduled, node_index, node_index, output);
                if (validi(use_block_index))
                {
                    lca = find_lca(lca, use_block_index);
                }
            }
        }

        if (validi(lca))
        {
            todo();
        }

        if (type_pair_get_backend(node->type) == BACKEND_TYPE_TUPLE)
        {
            todo();
        }

        basic_block_add_node(thread, current_basic_block, node_index, 4);
    }

    if (options.dataflow)
    {
        dataflow(thread, builder->worker, builder->basic_blocks, builder->scheduled, node_count);
    }
}

fn void cfg_build_and_global_schedule(CFGBuilder* restrict builder, Thread* restrict thread, Function* restrict function, GlobalScheduleOptions options)
{
    cfg_build(builder, thread, function);

    cfg_global_schedule(builder, thread, function, options);
}

fn void cfg_list_schedule(Thread* restrict thread, CFGBuilder* restrict builder, Function* restrict function, BasicBlockIndex basic_block_index)
{
    // print("=================================\nLIST SCHEDULER START\n=================================\n");
    thread_worklist_clear(thread, builder->worker);

    auto* restrict basic_block = &builder->basic_blocks.pointer[geti(basic_block_index)];
    thread_worklist_push(thread, builder->worker, basic_block->start);

    if (geti(basic_block_index) == 0)
    {
        auto* root_node = thread_node_get(thread, function->root);
        auto root_outputs = node_get_outputs(thread, root_node);

        for (u32 i = 0; i < root_outputs.length; i += 1)
        {
            auto output = root_outputs.pointer[i];
            auto* output_node = thread_node_get(thread, output);
            if (output_node->id == IR_PROJECTION)
            {
                thread_worklist_push(thread, builder->worker, output);
            }
        }
    }
    else
    {
        auto* bb_start = thread_node_get(thread, basic_block->start);
        auto outputs = node_get_outputs(thread, bb_start);
        for (u32 i = 0; i < outputs.length; i += 1)
        {
            auto output_index = outputs.pointer[i];
            auto* output = thread_node_get(thread, output_index);
            if (output->id == IR_PHI)
            {
                thread_worklist_push(thread, builder->worker, output_index);
            }
        }
    }

    auto end_index = basic_block->end;
    auto* end = thread_node_get(thread, end_index);
    Scheduler scheduler = {};
    bitset_ensure_length(&scheduler.ready_set, thread->buffer.nodes.length);

    for (u32 i = 0; i < basic_block->items.length; i += 1)
    {
        auto node_index = basic_block->items.pointer[i];
        auto* node = thread_node_get(thread, node_index);

        if (!thread_worklist_test(thread, builder->worker, node_index) && index_equal(builder->scheduled.pointer[geti(node_index)], basic_block_index) && node_is_ready(thread, builder->scheduled, builder->worker, node, basic_block_index))
        {
            node_ready_up(thread, &scheduler, node_index, end);
        }
    }

    // TODO: IS BRANCH
    VirtualBuffer(InFlightNode) active = {};
    u64 in_use_mask = 0;
    u64 blocked_mask = UINT64_MAX >> (64 - 1);
    u32 cycle = 0;

    while (active.length > 0 || scheduler.ready.length > 0)
    {
        while (in_use_mask != blocked_mask && scheduler.ready.length > 0)
        {
            auto signed_index = node_best_ready(&scheduler, in_use_mask);
            if (signed_index < 0)
            {
                break;
            }
            auto index = cast(u32, s32, signed_index);
            auto available = scheduler.ready.pointer[index].unit_mask & ~in_use_mask;
            auto unit_i = __builtin_ffsll(cast(s64, u64, available)) - 1;

            auto node_index = scheduler.ready.pointer[index].node_index;
            auto* node = thread_node_get(thread, node_index);

            in_use_mask |= (u64)1 << unit_i;

            if (index + 1 < scheduler.ready.length)
            {
                todo();
            }

            scheduler.ready.length -= 1;
            assert(node->id != IR_PROJECTION);

            auto end_cycle = cycle + node_get_latency(thread, node, end);
            *vb_add(&active, 1) = (InFlightNode)
            {
                .node_index = node_index,
                    .end = cast(u32, u64, end_cycle),
                    .unit_i = unit_i,
            };

            if (node != end)
            {
                thread_worklist_push(thread, builder->worker, node_index);

                if (type_pair_get_backend(node->type) == BACKEND_TYPE_TUPLE)
                {
                    todo();
                }
            }
        }

        cycle += 1;

        for (u32 i = 0; i < active.length; i += 1)
        {
            auto active_i = active.pointer[i];
            auto node_index = active_i.node_index;
            auto* node = thread_node_get(thread, node_index);

            if (active_i.end > cycle)
            {
                i += 1;
                continue;
            }

            in_use_mask &= ~((u64)1 << active_i.unit_i);
            auto last = active.pointer[active.length - 1];
            active.pointer[i] = last;
            active.length -= 1;

            auto outputs = node_get_outputs(thread, node);
            for (u16 i = 0; i < outputs.length; i += 1)
            {
                auto output_index = outputs.pointer[i];
                auto* output = thread_node_get(thread, output_index);
                if (output->id == IR_PROJECTION)
                {
                    print("TODO: proj\n");
                    todo();
                }
                else if (!bitset_get(&scheduler.ready_set, geti(output_index)) && index_equal(builder->scheduled.pointer[geti(output_index)], basic_block_index) && !thread_worklist_test(thread, builder->worker, output_index) && node_is_ready(thread, builder->scheduled, builder->worker, output, basic_block_index))
                {
                    node_ready_up(thread, &scheduler, output_index, end);
                }
            }
        }
    }

    if (!index_equal(end_index, basic_block->start))
    {
        thread_worklist_push(thread, builder->worker, end_index);
    }
    // print("=================================\nLIST SCHEDULER END\n=================================\n");
}

fn u8 operand_equal(MachineOperand a, MachineOperand b)
{
    if (a.id != b.id)
    {
        return 0;
    }

    if (a.id == MACHINE_OPERAND_MEMORY)
    {
        todo();
    }

    return (a.id == MACHINE_OPERAND_GPR || a.id == MACHINE_OPERAND_XMM) ? a.register_value == b.register_value : 0;
}

fn void write_macho(Thread* restrict thread, const ObjectOptions * const restrict options, char** envp)
{
    todo();
}

fn void code_generation(Thread* restrict thread, CodegenOptions options, char** envp)
{
    auto cfg_builder = cfg_builder_init(thread);
    auto* restrict builder = &cfg_builder;
    VirtualBuffer(u8) code = {};

    for (u32 function_i = 0; function_i < thread->buffer.functions.length; function_i += 1)
    {
        Function* restrict function = &thread->buffer.functions.pointer[function_i];
        cfg_builder_clear(builder, thread);

        cfg_build_and_global_schedule(builder, thread, function, (GlobalScheduleOptions) {
            .dataflow = 1,
        });
        auto node_count = thread->buffer.nodes.length;

        u32 max_ins = 0;
        u32 virtual_register_count = 1;
        auto* virtual_register_map = arena_allocate(thread->arena, u32, round_up_to_next_power_of_2( node_count + 16));
        VirtualBuffer(u32) spills = {};

        for (u32 i = 0; i < builder->basic_blocks.length; i += 1)
        {
            auto basic_block_index = Index(BasicBlock, i);
            BasicBlock* basic_block = &builder->basic_blocks.pointer[i];

            cfg_list_schedule(thread, builder, function, basic_block_index);

            auto max_item_count = thread_worklist_length(thread, builder->worker);
            print("Item count: {u32}\n", max_item_count);

            basic_block->items.length = 0;

            for (u32 i = 0; i < max_item_count; i += 1)
            {
                auto node_index = thread_worklist_get(thread, builder->worker, i);
                basic_block_add_node(thread, basic_block, node_index, 5);
                auto* node = thread_node_get(thread, node_index);
                auto def_mask = node_constraint(thread, node, (Slice(RegisterMaskIndex)){});
                auto inputs = node->input_count + node_tmp_count(node);

                if (inputs > max_ins)
                {
                    max_ins = inputs;
                }

                u32 virtual_register_id = 0;
                if (!index_equal(def_mask, empty_register_mask))
                {
                    if (node->id == MACHINE_MOVE)
                    {
                        assert(node->output_count == 1);
                        auto outputs = node_get_outputs(thread, node);
                        auto phi_index = outputs.pointer[0];
                        auto* phi = thread_node_get(thread, phi_index);
                        assert(phi->id == IR_PHI);

                        if (virtual_register_map[geti(phi_index)] == 0)
                        {
                            virtual_register_id = virtual_register_count;
                            virtual_register_count += 1;
                            virtual_register_map[geti(phi_index)] = virtual_register_id;
                        }
                        else
                        {
                            todo();
                        }
                    }
                    else if (node->id == IR_PHI && virtual_register_map[geti(node_index)] > 0)
                    {
                        virtual_register_id = virtual_register_map[geti(node_index)];
                    }
                    else
                    {
                        virtual_register_id = virtual_register_count;
                        virtual_register_count += 1;
                    }
                }

                virtual_register_map[geti(node_index)] = virtual_register_id;
                print("Assigning VR{u32} to node #{u32} ({s})\n", virtual_register_id, geti(node_index), node_id_to_string(node->id));
            }
        }

        auto ins = (Slice(RegisterMaskIndex)) {
            .pointer = arena_allocate(thread->arena, RegisterMaskIndex, max_ins),
            .length = max_ins,
        };
        // TODO: remove?
        memset(ins.pointer, 0, sizeof(RegisterMaskIndex) * max_ins);

        VirtualBuffer(VirtualRegister) virtual_registers = {};
        vb_ensure_capacity(&virtual_registers, cast(u32, u64, round_up_to_next_power_of_2(virtual_register_count + 16)));
        virtual_registers.length = virtual_register_count;

        for (u32 i = 0; i < builder->basic_blocks.length; i += 1)
        {
            auto* basic_block = &builder->basic_blocks.pointer[i];

            print("BB items: {u32}\n", basic_block->items.length);
            for (u32 i = 0; i < basic_block->items.length; i += 1)
            {
                auto node_index = basic_block->items.pointer[i];
                auto* node = thread_node_get(thread, node_index);
                auto virtual_register_id = virtual_register_map[geti(node_index)];
                assert(virtual_register_id >= 0 && virtual_register_id < virtual_register_count);

                if (virtual_register_id > 0 && node->id != MACHINE_MOVE)
                {
                    auto mask_index = node_constraint(thread, node, (Slice(RegisterMaskIndex)){});
                    print("Node #{u32} ({s}), VR{u32}, mask: ", geti(node_index), node_id_to_string(node->id), virtual_register_id);
                    if (validi(mask_index))
                    {
                        print("0x{u32:x}", thread_register_mask_get(thread, mask_index)->mask);
                    }
                    else
                    {
                        print("invalid");
                    }
                    print("\n");
                    virtual_registers.pointer[virtual_register_id] = (VirtualRegister) {
                        .mask = mask_index,
                        .node_index = node_index,
                        .assigned = -1,
                        .spill_cost = NAN,
                    };
                }
            }
        }

        thread_worklist_clear(thread, builder->worker);

        u32 max_registers_in_class = 0;
        auto* fixed = arena_allocate(thread->arena, s32, REGISTER_CLASS_X86_64_COUNT);
        auto* in_use = arena_allocate(thread->arena, u32, REGISTER_CLASS_X86_64_COUNT);

        for (u32 class = 0; class < REGISTER_CLASS_X86_64_COUNT; class += 1)
        {
            auto count = register_count_per_class[class];
            max_registers_in_class = MAX(max_registers_in_class, count);
            auto base = virtual_registers.length;

            for (u32 i = 0; i < count; i += 1)
            {
                auto mask = register_mask_intern(thread, (RegisterMask) {
                    .class = class,
                    .may_spill = 0,
                    .mask = class == 0 ? i : ((u32)1 << i),
                });

                *vb_add(&virtual_registers, 1) = (VirtualRegister) {
                    .mask = mask,
                    .class = cast(s16, u32, class),
                    .assigned = cast(s16, u32, i),
                    .spill_cost = INFINITY,
                };
            }

            fixed[class] = cast(s32, u32, base);
        }

        // Insert legalizing moves
        for (u32 i = 0; i < builder->basic_blocks.length; i += 1)
        {
            auto* basic_block = &builder->basic_blocks.pointer[i];
            // auto basic_block_index = Index(BasicBlock, i);

            for (u32 i = 0; i < basic_block->items.length; i += 1)
            {
                auto node_index = basic_block->items.pointer[i];
                auto* node = thread_node_get(thread, node_index);
                auto tmp_count = node_tmp_count(node);
                node_constraint(thread, node, ins);

                auto inputs = node_get_inputs(thread, node);
                for (u16 i = 1; i < inputs.length; i += 1)
                {
                    auto input_index = inputs.pointer[i];
                    if (validi(input_index))
                    {
                        // auto* input = thread_node_get(thread, input_index);
                        auto in_mask_index = ins.pointer[i];

                        if (!index_equal(in_mask_index, empty_register_mask))
                        {
                            auto in_mask = thread_register_mask_get(thread, in_mask_index);
                            VirtualRegister* vreg = 0;
                            auto vreg_index = virtual_register_map[geti(input_index)];
                            if (vreg_index > 0)
                            {
                                vreg = &virtual_registers.pointer[vreg_index];
                            }
                            assert(vreg);
                            auto vreg_mask = thread_register_mask_get(thread, vreg->mask);

                            auto hint = fixed_register_mask(*in_mask);
                            if (hint >= 0 && vreg_mask->class == in_mask->class)
                            {
                                vreg->hint_vreg = fixed[in_mask->class] + hint;
                            }

                            auto new_mask_index = register_mask_meet(thread, in_mask_index, vreg->mask);
                            print("Input #{u32} ({s})\n", geti(input_index), node_id_to_string(thread_node_get(thread, input_index)->id));
                            print("IN mask index: {u32}. TODO: not equal: {u32}, {u32}, {u32}\n", i, in_mask_index, empty_register_mask, new_mask_index);
                            if (!index_equal(in_mask_index, empty_register_mask) && index_equal(new_mask_index, empty_register_mask))
                            {
                                // if (node->id == MACHINE_COPY)
                                {
                                    print("{s} input count: {u32}\n", node_id_to_string(node->id), (u32)node->input_count);
                                }
                                todo();
                            }

                            auto* new_mask = thread_register_mask_get(thread, new_mask_index);
                            auto fixed = fixed_register_mask(*new_mask);

                            if (fixed >= 0)
                            {
                                // auto fixed_mask = ((u32)1 << fixed);
                                auto shared_edge = node_to_address(thread, input_index);

                                if (shared_edge >= 0)
                                {
                                    auto* input_node = thread_node_get(thread, input_index);
                                    auto p_shared_edge = cast(u16, s32, shared_edge);
                                    assert(p_shared_edge < input_node->input_count);
                                    auto inputs = node_get_inputs(thread, input_node);
                                    for (u16 i = 1; i < input_node->input_count; i += 1)
                                    {
                                        if (i != shared_edge)
                                        {
                                            auto input_index = inputs.pointer[i];
                                            if (validi(input_index))
                                            {
                                                todo();
                                            }
                                        }
                                    }
                                }
                            }

                            vreg->mask = new_mask_index;
                        }
                    }
                }

                auto virtual_register_index = virtual_register_map[geti(node_index)];

                if (tmp_count > 0)
                {
                    todo();
                }

                if (virtual_register_index > 0)
                {
                    auto* virtual_register = &virtual_registers.pointer[virtual_register_index];
                    virtual_register->spill_cost = NAN;

                    if (node->id == MACHINE_COPY)
                    {
                        auto* in = thread_node_get(thread, inputs.pointer[1]);
                        if (in->id == IR_PHI)
                        {
                            thread_worklist_push(thread, builder->worker, node_index);
                        }
                    }
                }
            }
        }

        u8 changes = 0;

        if (thread_worklist_length(thread, builder->worker) > 0)
        {
            // Compute ordinals
            auto order = compute_ordinals(thread, builder->basic_blocks, node_count);
            while (thread_worklist_length(thread, builder->worker) > 0)
            {
                auto node_index = thread_worklist_pop(thread, builder->worker);
                auto* node = thread_node_get(thread, node_index);
                assert(node->id == MACHINE_COPY);
                auto id = virtual_register_map[geti(node_index)];
                assert(id > 0);
                // auto mask_index = virtual_registers.pointer[id].mask;
                auto inputs = node_get_inputs(thread, node);

                if (!interfere(thread, builder->scheduled, builder->basic_blocks, order, node_index, inputs.pointer[1]))
                {
                    auto basic_block_index = builder->scheduled.pointer[geti(node_index)];
                    auto* basic_block = &builder->basic_blocks.pointer[geti(basic_block_index)];
                    u64 i = 0;
                    auto count = basic_block->items.length;
                    while (i < count && !index_equal(basic_block->items.pointer[i], node_index))
                    {
                        i += 1;
                    }

                    assert(index_equal(basic_block->items.pointer[i], node_index));
                    memmove(&basic_block->items.pointer[i], &basic_block->items.pointer[i + 1], (count - (i + 1)) * sizeof(NodeIndex));
                    basic_block->items.length -= 1;
                    builder->scheduled.pointer[geti(node_index)] = invalidi(BasicBlock);
                    subsume_node_without_killing(thread, node_index, inputs.pointer[1]);
                    changes = 1;
                }
            }
        }

        // TODO: spills
        if (spills.length)
        {
            todo();
            changes = 1;
        }

        if (changes)
        {
            redo_dataflow(thread, builder->worker, builder->basic_blocks, builder->scheduled, node_count);
        }

        auto al_index = 0;
        // Allocate loop
        while (1)
        {
            print("==============================\n#{u32} Allocate loop\n==============================\n", al_index++);

            auto order = compute_ordinals(thread, builder->basic_blocks, node_count);

            Bitset active = {};
            bitset_ensure_length(&active, virtual_registers.length);
            Bitset future_active = {};
            bitset_ensure_length(&future_active, virtual_registers.length);
            Bitset live_out = {};
            bitset_ensure_length(&live_out, node_count);

            for (u32 block_i = 0; block_i < builder->basic_blocks.length; block_i += 1)
            {
                auto* basic_block = &builder->basic_blocks.pointer[block_i];

                for (u32 node_i = 0; node_i < basic_block->items.length; node_i += 1)
                {
                    auto node_index = basic_block->items.pointer[node_i];
                    auto virtual_register_id = virtual_register_map[geti(node_index)];

                    if (virtual_register_id > 0)
                    {
                        // auto* node = thread_node_get(thread, node_index);

                        auto mask_index = virtual_registers.pointer[virtual_register_id].mask;
                        auto mask_pointer = thread_register_mask_get(thread, mask_index);
                        auto mask_value = *mask_pointer;
                        auto reg = fixed_register_mask(mask_value);

                        // print("Block #{u32}, Node index #{u32}, Node GVN #{u32}, Node id: {s}, VR{u32}. Mask: {u32:x}. Reg: {u32:x}\n", block_i, node_i, geti(node_index), node_id_to_string(node->id), virtual_register_id, mask_value.mask, reg);

                        if (reg >= 0)
                        {
                            todo();
                        }
                    }
                }
            }

            if (spills.length)
            {
                todo();
            }

            for (u32 i = 0; i < builder->basic_blocks.length; i += 1)
            {
                auto* basic_block = &builder->basic_blocks.pointer[i];
                print("============\nBlock #{u32}\n============\n", i);

                auto basic_block_index = Index(BasicBlock, i);

                auto bb_live_in = &basic_block->live_in;
                auto bb_live_out = &basic_block->live_out;

                FOREACH_SET(j, &live_out) if (!bitset_get(bb_live_in, j))
                {
                    auto virtual_register_id = virtual_register_map[j];
                    print("General live out not present in basic block live in: N{u64}, VR{u32}\n", j, virtual_register_id);
                    if (virtual_register_id != 0)
                    {
                        u8 pause = 0;
                        for (u32 k = i; k < builder->basic_blocks.length; k += 1)
                        {
                            auto* other_basic_block = &builder->basic_blocks.pointer[k];
                            if (bitset_get(&other_basic_block->live_in, j))
                            {
                                unused(pause);
                                todo();
                            }
                        }

                        bitset_set_value(&active, virtual_register_id, 0);
                        bitset_set_value(&live_out, j, 0);
                    }
                }

                FOREACH_SET(j, bb_live_in) if (!bitset_get(&live_out, j))
                {
                    auto virtual_register_id = virtual_register_map[j];
                    print("Basic block live in not present in general live out: N{u64}, VR{u32}\n", j, virtual_register_id);

                    if (virtual_register_id > 0)
                    {
                        {
                            auto* virtual_register = &virtual_registers.pointer[virtual_register_id];
                            auto node_index = virtual_register->node_index;
                            auto* node = thread_node_get(thread, node_index);
                            print("[BB LIVE IN   ] Allocating register for node #{u32} ({s})\n", geti(node_index), node_id_to_string(node->id));
                        }

                        if (!register_allocate(thread, virtual_registers, &spills, &active, &future_active, builder->scheduled, builder->basic_blocks, order, virtual_register_id, 0))
                        {
                            todo();
                        }
                    }
                }

                for (u32 i = 0; i < basic_block->items.length; i += 1)
                {
                    NodeIndex node_index = basic_block->items.pointer[i];
                    auto* node = thread_node_get(thread, node_index);
                    auto def = order.pointer[geti(node_index)];

                    auto inputs = node_get_inputs(thread, node);

                    print("Node #{u32} ({s}). Def: {u32}\n", geti(node_index), node_id_to_string(node->id), def);

                    if (node->id == IR_PROJECTION && !index_equal(inputs.pointer[0], function->root))
                    {
                        print("Skipping...\n");
                        continue;
                    }

                    if (node->id != IR_PHI)
                    {
                        print("Node is not PHI. Examining inputs ({u32})...\n", (u32)node->input_count);

                        for (u16 i = 1; i < node->input_count; i += 1)
                        {
                            auto input_index = inputs.pointer[i];
                            if (validi(input_index))
                            {
                                auto virtual_register_id = virtual_register_map[geti(input_index)];
                                print("Input {u32}: node #{u32} ({s}). VR{u32}\n", i, geti(input_index), node_id_to_string(thread_node_get(thread, input_index)->id), virtual_register_id);
                                if (virtual_register_id == 0)
                                {
                                    print("Invalid vreg id. Removing from general live out and skipping...\n");
                                    bitset_set_value(&live_out, geti(input_index), 0);
                                    continue;
                                }

                                if (!bitset_get(&live_out, geti(input_index)))
                                {
                                    print("Duplicate input. Skipping...\n");
                                    continue;
                                }

                                auto* input = thread_node_get(thread, input_index);
                                auto last_use = node_last_use_in_block(thread, builder->scheduled, order, input, basic_block_index);
                                print("Last use: {u32}\n", last_use);

                                if (bitset_get(bb_live_out, geti(input_index)))
                                {
                                    todo();
                                }

                                print("Removing node #{u32} from general liveout\n", geti(input_index));
                                bitset_set_value(&live_out, geti(input_index), 0);

                                auto pause = last_use > def;
                                if (!pause)
                                {
                                    for (u32 i = geti(basic_block_index); i < builder->basic_blocks.length; i += 1)
                                    {
                                        auto* other = &builder->basic_blocks.pointer[i];
                                        if (bitset_get(&other->live_in, geti(input_index)))
                                        {
                                            pause = 1;
                                            break;
                                        }
                                    }
                                }

                                if (pause)
                                {
                                    bitset_set_value(&future_active, virtual_register_id, 1);
                                }

                                print("Removing VR{u32} from general active\n", virtual_register_id);
                                bitset_set_value(&active, virtual_register_id, 0);
                            }
                        }
                    }

                    for (u32 i = 0; i < REGISTER_CLASS_X86_64_COUNT; i += 1)
                    {
                        in_use[i] = 0;
                    }

                    // TODO: tmps
                    
                    auto virtual_register_id = virtual_register_map[geti(node_index)];

                    if (virtual_register_id > 0)
                    {
                        auto* virtual_register = &virtual_registers.pointer[virtual_register_id];
                        auto class = virtual_register->class;
                        auto in_use_local = in_use[class];
                        print("[ALLOCATE LOOP] Allocating register for node #{u32} ({s}), VR{u32}\n", geti(node_index), node_id_to_string(node->id), virtual_register_id);
                        if (!register_allocate(thread, virtual_registers, &spills, &active, &future_active, builder->scheduled, builder->basic_blocks, order, virtual_register_id, in_use_local))
                        {
                            todo();
                        }

                        print("[END ALLOCATE LOOP]\n");
                        assert(virtual_register_map[geti(node_index)] == virtual_register_id);

                        auto def = virtual_register->node_index;
                        bitset_set_value(&live_out, geti(def), 1);
                        print("Setting as general live out node #{u32} ({s})\n", geti(def), node_id_to_string(thread_node_get(thread, def)->id));
                    }
                    else if (type_pair_get_backend(node->type) == BACKEND_TYPE_TUPLE)
                    {
                        todo();
                    }
                }
            }

            break;
        }

        // Basic block scheduling

        for (u32 i = 0; i < builder->basic_blocks.length; i += 1)
        {
            auto* basic_block = &builder->basic_blocks.pointer[i];
            auto basic_block_index = Index(BasicBlock, cast(u32, s64, basic_block - builder->basic_blocks.pointer));
            auto first_node = thread_node_get(thread, basic_block->items.pointer[0]);
            auto item_count = basic_block->items.length;
            u8 empty = 1;

            if (first_node->id == IR_REGION)
            {
                for (u32 i = 1; i < item_count; i += 1)
                {
                    auto node_index = basic_block->items.pointer[i];
                    auto* node = thread_node_get(thread, node_index);
                    if (node->id != IR_PHI)
                    {
                        empty = 0;
                        break;
                    }
                }
            }
            else if (item_count > 1 || node_is_control_projection(first_node))
            {
                empty = 0;
            }

            if (empty)
            {
                todo();
            }
            else
            {
                basic_block->forward = cast(s32, u32, i);

                auto* bb_end = thread_node_get(thread, basic_block->end);
                if (!cfg_node_terminator(bb_end))
                {
                    auto jump_node_index = thread_node_add(thread, (NodeCreate)
                    {
                        .id = MACHINE_JUMP,
                        .inputs = array_to_slice(((NodeIndex[]) {
                            invalidi(Node),
                        })),
                        .type_pair = type_pair_make(invalidi(DebugType), BACKEND_TYPE_CONTROL),
                    });
                    auto successor_node_index = cfg_next_user(thread, basic_block->end);
                    auto* successor_node = thread_node_get(thread, successor_node_index);
                    auto successor_inputs = node_get_inputs(thread, successor_node);
                    u16 i;
                    for (i = 0; i < successor_node->input_count; i += 1)
                    {
                        auto input_index = successor_inputs.pointer[i];
                        if (index_equal(input_index, basic_block->end))
                        {
                            break;
                        }
                    }
                    assert(i < successor_node->input_count);
                    node_set_input(thread, successor_node_index, i, jump_node_index);
                    node_set_input(thread, jump_node_index, 0, basic_block->end);
                    basic_block->end = jump_node_index;

                    basic_block_add_node(thread, basic_block, jump_node_index, 6);

                    assert(builder->scheduled.length == geti(jump_node_index));
                    *vb_add(&builder->scheduled, 1) = basic_block_index;
                }
            }
        }

        for (u32 i = 0; i < builder->basic_blocks.length; i += 1)
        {
            auto* basic_block = &builder->basic_blocks.pointer[i];

            auto forward = basic_block->forward;
            while (forward != builder->basic_blocks.pointer[forward].forward)
            {
                forward = builder->basic_blocks.pointer[forward].forward;
            }

            basic_block->forward = forward;
        }

        auto* order = arena_allocate(thread->arena, s32, builder->basic_blocks.length);

        u32 order_index = 0;
        for (s32 i = 0; i < cast(s32, u32, builder->basic_blocks.length); i += 1)
        {
            auto* basic_block = &builder->basic_blocks.pointer[i];
            if (basic_block->forward == i)
            {
                auto* end_node = thread_node_get(thread, basic_block->end);
                if (end_node->id != IR_RETURN)
                {
                    order[order_index] = i;
                    order_index += 1;
                }
            }
        }

        for (s32 i = 0; i < cast(s32, u32, builder->basic_blocks.length); i += 1)
        {
            auto* basic_block = &builder->basic_blocks.pointer[i];
            if (basic_block->forward == i)
            {
                auto* end_node = thread_node_get(thread, basic_block->end);
                if (end_node->id == IR_RETURN)
                {
                    order[order_index] = i;
                    order_index += 1;
                }
            }
        }

        // Emit
        auto final_order_count = order_index;

        for (u32 order_index = 0; order_index < final_order_count; order_index += 1)
        {
            auto i = order[order_index];
            auto* basic_block = &builder->basic_blocks.pointer[i];

            for (u32 i = 0; i < basic_block->items.length; i += 1)
            {
                auto node_index = basic_block->items.pointer[i];
                auto* node = thread_node_get(thread, node_index);
                // auto virtual_register_id = virtual_register_map[geti(node_index)];
                // auto* virtual_register = &virtual_registers.pointer[virtual_register_id];
                auto inputs = node_get_inputs(thread, node);

                auto fallthrough = INT32_MAX;
                if (order_index + 1 < final_order_count)
                {
                    fallthrough = order[order_index + 1];
                }

                switch (node->id)
                {
                    case IR_PROJECTION:
                    case IR_REGION:
                    case IR_PHI:
                        break;
                    case IR_INTEGER_CONSTANT:
                        {
                            auto value = node->integer_constant.unsigned_value;
                            auto gpr = (GPR)machine_operand_at(virtual_register_map, virtual_registers, node_index, REGISTER_CLASS_X86_64_GPR);
                            auto backend_type = type_pair_get_backend(node->type);

                            if (backend_type == BACKEND_TYPE_INTEGER_32)
                            {
                                if (value == 0)
                                {
                                    if (gpr == RAX)
                                    {
                                        *vb_add(&code, 1) = 0x31;
                                        *vb_add(&code, 1) = 0xc0;
                                    }
                                    else
                                    {
                                        todo();
                                    }
                                }
                                else
                                {
                                    todo();
                                }
                            }
                        } break;
                    case MACHINE_MOVE:
                        {
                            auto destination = operand_from_node(virtual_registers, virtual_register_map, node_index);
                            auto source = operand_from_node(virtual_registers, virtual_register_map, inputs.pointer[1]);
                            if (!operand_equal(destination, source))
                            {
                                todo();
                            }
                        } break;
                    case MACHINE_JUMP:
                        {
                            auto successor_node_index = cfg_next_control(thread, node_index);
                            assert(validi(successor_node_index));
                            auto successor_basic_block_index = fixed_block_map_get(&builder->block_map, successor_node_index);
                            assert(validi(successor_basic_block_index));
                            auto* successor_basic_block = &builder->basic_blocks.pointer[geti(successor_basic_block_index)];
                            if (fallthrough != successor_basic_block->forward)
                            {
                                todo();
                            }
                        } break;
                    case IR_RETURN:
                        {
                            *vb_add(&code, 1) = 0xc3;
                        } break;
                    default:
                        todo();
                }
            }
        }
    }

    auto object_path = arena_join_string(thread->arena, (Slice(String)) array_to_slice(((String[]) {
        strlit("nest/"),
        options.test_name,
        options.backend == COMPILER_BACKEND_C ? strlit(".c") : strlit(".o"),
    })));

    auto exe_path_view = s_get_slice(u8, object_path, 0, object_path.length - 2);
    auto exe_path = (char*)arena_allocate_bytes(thread->arena, exe_path_view.length + 1, 1);
    memcpy(exe_path, exe_path_view.pointer, exe_path_view.length);
    exe_path[exe_path_view.length] = 0;

    switch (options.backend)
    {
    case COMPILER_BACKEND_C:
        {
            // auto lowered_source = c_lower(thread);
            // // print("Transpiled to C:\n```\n{s}\n```\n", lowered_source);
            //
            // file_write(object_path, lowered_source);
            //
            // char* command[] = {
            //     clang_path, "-g",
            //     "-o", exe_path,
            //     string_to_c(object_path),
            //     0,
            // };
            //
            // run_command((CStringSlice) array_to_slice(command), envp);
            todo();
        } break;
    case COMPILER_BACKEND_INTERPRETER:
        {
            // auto* main_function = &thread->buffer.functions.pointer[thread->main_function];
            // auto* interpreter = interpreter_create(thread);
            // interpreter->function = main_function;
            // interpreter->arguments = (Slice(String)) array_to_slice(((String[]) {
            //     test_name,
            // }));
            // auto exit_code = interpreter_run(interpreter, thread);
            // print("Interpreter exited with exit code: {u32}\n", exit_code);
            // syscall_exit(exit_code);
            todo();
        } break;
    case COMPILER_BACKEND_MACHINE:
        {
            auto code_slice = (Slice(u8)) { .pointer = code.pointer, .length = code.length, };
            auto object_options = (ObjectOptions) {
                .object_path = string_to_c(object_path),
                .exe_path = exe_path,
                .code = code_slice,
                .dynamic = 1,
            };
#if defined(__APPLE__)
            write_macho(thread, &object_options, envp);
#elif defined(__linux__)
            write_elf(thread, &object_options, envp);
#else
            todo();
#endif
        } break;
    }
}

fn u8 node_is_empty_control_projection(Thread* restrict thread, CFGBuilder* restrict builder, NodeIndex node_index)
{
    auto* restrict node = thread_node_get(thread, node_index);
    u8 result = 0;

    if (node_is_control_projection(node))
    {
        auto basic_block_index = builder->scheduled.pointer[geti(node_index)];
        auto* basic_block = &builder->basic_blocks.pointer[geti(basic_block_index)];
        result = basic_block->items.length == 1;
    }

    return result;
}

STRUCT(SchedPhi)
{
    NodeIndex phi;
    NodeIndex node;
};
decl_vb(SchedPhi);

fn void fill_phis(Thread* restrict thread, VirtualBuffer(SchedPhi)* sched_phis, Node* restrict successor_node, NodeIndex original_index)
{
    auto succesor_inputs = node_get_inputs(thread, successor_node);
    u16 i;
    for (i = 0; i < successor_node->input_count; i += 1)
    {
        auto input_index = succesor_inputs.pointer[i];
        if (index_equal(input_index, original_index))
        {
            break;
        }
    }
    assert(i < successor_node->input_count);
    auto phi_index = i;

    auto successor_outputs = node_get_outputs(thread, successor_node);
    for (u16 i = 0; i < successor_node->output_count; i += 1)
    {
        auto output_index = successor_outputs.pointer[i];
        auto* output_node = thread_node_get(thread, output_index);
        if (output_node->id == IR_PHI)
        {
            auto output_inputs = node_get_inputs(thread, output_node);
            assert(phi_index + 1 < output_node->input_count);
            *vb_add(sched_phis, 1) = (SchedPhi) {
                .phi = output_index,
                .node = output_inputs.pointer[phi_index + 1],
            };
        }
    }
}

STRUCT(SchedNode)
{
    SchedNode* parent;
    NodeIndex node_index;
    s32 index;
};

fn u8 sched_in_basic_block(Thread* restrict thread, CFGBuilder* restrict builder, BasicBlockIndex basic_block_index, NodeIndex node_index)
{
    return index_equal(builder->scheduled.pointer[geti(node_index)], basic_block_index) && !thread_worklist_test_and_set(thread, builder->worker, node_index);
}

fn void greedy_scheduler(Thread* restrict thread, CFGBuilder* restrict builder, Function* restrict function, BasicBlockIndex basic_block_index)
{
    thread_worklist_clear(thread, builder->worker);

    auto* restrict basic_block = &builder->basic_blocks.pointer[geti(basic_block_index)];
    auto end_index = basic_block->end;
    auto* end_node = thread_node_get(thread, end_index);

    VirtualBuffer(SchedPhi) phis = {};

    if (node_is_cfg_fork(end_node))
    {
        todo();
    }
    else if (!cfg_is_endpoint(thread, end_node))
    {
        auto successor_index = cfg_next_user(thread, end_index);
        auto* successor_node = thread_node_get(thread, successor_index);
        if (successor_node->id == IR_REGION)
        {
            fill_phis(thread, &phis, successor_node, end_index);
        }
    }

    auto* top = arena_allocate(thread->arena, SchedNode, 1);
    *top = (SchedNode)
    {
        .node_index = end_index,
    };
    thread_worklist_test_and_set(thread, builder->worker, end_index);

    if (geti(basic_block_index) == 0)
    {
        auto* root_node = thread_node_get(thread, function->root);
        auto outputs = node_get_outputs(thread, root_node);
        for (u16 i = 0; i < root_node->output_count; i += 1)
        {
            auto output_index = outputs.pointer[i];
            auto* output_node = thread_node_get(thread, output_index);

            if (output_node->id == IR_PROJECTION && !thread_worklist_test_and_set(thread, builder->worker, output_index))
            {
                thread_worklist_push_array(thread, builder->worker, output_index);
            }
        }
    }

    u64 phi_current = 0;
    u64 leftovers = 0;
    auto leftover_count = basic_block->items.length;

    while (top)
    {
        auto node_index = top->node_index;
        auto* node = thread_node_get(thread, node_index);

        if (node->id != IR_PHI && top->index < node->input_capacity)
        {
            auto inputs = node_get_inputs(thread, node);
            auto input_index = inputs.pointer[top->index];
            top->index += 1;

            if (validi(input_index))
            {
                auto* input_node = thread_node_get(thread, input_index);
                if (input_node->id == IR_PROJECTION)
                {
                    auto projection_inputs = node_get_inputs(thread, input_node);
                    input_index = projection_inputs.pointer[0];
                    input_node = thread_node_get(thread, input_index);
                }

                if (sched_in_basic_block(thread, builder, basic_block_index, input_index))
                {
                    auto* new_top = arena_allocate(thread->arena, SchedNode, 1);
                    *new_top = (SchedNode)
                    {
                        .node_index = input_index,
                        .parent = top,
                    };
                    top = new_top;
                }
            }

            continue;
        }

        if (index_equal(end_index, node_index))
        {
            if (phi_current < phis.length)
            {
                auto* restrict phi = &phis.pointer[phi_current];
                phi_current += 1;

                auto value = phi->node;
                if (sched_in_basic_block(thread, builder, basic_block_index, value))
                {
                    auto* new_top = arena_allocate(thread->arena, SchedNode, 1);
                    *new_top = (SchedNode)
                    {
                        .node_index = value,
                        .parent = top,
                    };
                    top = new_top;
                }

                continue;
            }

            auto try_again = 0;
            while (leftovers < leftover_count)
            {
                auto index = leftovers;
                leftovers += 1;

                auto bb_node_index = basic_block->items.pointer[index];

                if (!thread_worklist_test_and_set(thread, builder->worker, bb_node_index))
                {
                    auto* new_top = arena_allocate(thread->arena, SchedNode, 1);
                    *new_top = (SchedNode)
                    {
                        .node_index = bb_node_index,
                        .parent = top,
                    };
                    top = new_top;
                    try_again = 1;
                    break;
                }
            }

            if (try_again)
            {
                continue;
            }
        }

        thread_worklist_push_array(thread, builder->worker, node_index);
        auto* parent = top->parent;
        top = parent;

        if (type_pair_get_backend(node->type) == BACKEND_TYPE_TUPLE)
        {
            todo();
        }
    }
}

fn void print_reference_to_node(Thread* restrict thread, NodeIndex node_index, u8 def)
{
    unused(def);
    auto* restrict node = thread_node_get(thread, node_index);
    print("[#{u32} ({s})", geti(node_index), node_id_to_string(node->id));

    switch (node->id)
    {
        case IR_PROJECTION:
            {
                if (node_is_control_projection(node))
                {
                    todo();
                }
                else
                {
                }
            } break;
        case IR_INTEGER_CONSTANT:
            {
                print(": 0x{u64:x}", node->integer_constant.unsigned_value);
            } break;
            // TODO:
        case IR_REGION:
            break;
        case IR_PHI:
            break;
        case MACHINE_COPY:
        case MACHINE_MOVE:
            break;
        default:
            todo();
    }

    print("]");
}

fn void print_basic_block(Thread* restrict thread, CFGBuilder* restrict builder, Function* restrict function, BasicBlockIndex basic_block_index)
{
    auto* restrict basic_block = &builder->basic_blocks.pointer[geti(basic_block_index)];
    print_reference_to_node(thread, basic_block->start, 1);
    print("\n");
    greedy_scheduler(thread, builder, function, basic_block_index);

    for (u32 i = 0; i < thread_worklist_length(thread, builder->worker); i += 1)
    {
        auto node_index = thread_worklist_get(thread, builder->worker, i);
        auto* node = thread_node_get(thread, node_index);

        switch (node->id)
        {
            case IR_PROJECTION:
            case IR_INTEGER_CONSTANT:
            case IR_REGION:
            case IR_PHI:
                continue;
            case MACHINE_MOVE:
            case MACHINE_COPY:
            case IR_RETURN:
                {
                    auto is_branch = 0;

                    if (is_branch)
                    {
                        todo();
                    }
                    else if (type_pair_get_backend(node->type) == BACKEND_TYPE_TUPLE)
                    {
                        todo();
                    }
                    else
                    {
                        print("  ");
                        print("#{u32}", geti(node_index));
                        print(" = {s}.", node_id_to_string(node->id));
                        // TODO: print type
                    }

                    print(" I({u32})", (u32)node->input_count);

                    u64 first = node->id != IR_PROJECTION;
                    auto inputs = node_get_inputs(thread, node);
                    if (node->input_count - first)
                    {
                        print(": ");
                        for (auto i = first; i < node->input_count; i += 1)
                        {
                            if (i != first)
                            {
                                print(", ");
                            }
                            print_reference_to_node(thread, inputs.pointer[i], 0);
                        }
                    }
                    else
                    {
                        print(" ");
                    }

                    switch (node->id)
                    {
                        case MACHINE_MOVE:
                        case MACHINE_COPY:
                        case IR_REGION:
                        case IR_PHI:
                        case IR_RETURN:
                            break;
                        default:
                            todo();
                    }
                } break;
            default:
                todo();
        }

        print("\n");
    }

    thread_worklist_clear(thread, builder->worker);

    auto* end_node = thread_node_get(thread, basic_block->end);
    if (cfg_node_terminator(end_node))
    {
        // todo();
    }
}

fn void print_ir(Thread* restrict thread)
{
    auto cfg_builder = cfg_builder_init(thread);
    auto* restrict builder = &cfg_builder;

    for (u32 i = 0; i < thread->buffer.functions.length; i += 1)
    {
        Function* restrict function = &thread->buffer.functions.pointer[i];
        cfg_builder_clear(builder, thread);

        cfg_build_and_global_schedule(builder, thread, function, (GlobalScheduleOptions) {
            .dataflow = 0,
        });

        auto end_basic_block_index = invalidi(BasicBlock);
        for (u32 i = 0; i < builder->basic_blocks.length; i += 1)
        {
            auto* restrict basic_block = &builder->basic_blocks.pointer[i];
            auto end_node_index = basic_block->end;
            auto* end_node = thread_node_get(thread, end_node_index);
            if (end_node->id == IR_RETURN)
            {
                end_basic_block_index = Index(BasicBlock, i);
                continue;
            }
            else if (node_is_empty_control_projection(thread, builder, end_node_index))
            {
                continue;
            }

            print_basic_block(thread, builder, function, Index(BasicBlock, i));
        }

        if (validi(end_basic_block_index))
        {
            print_basic_block(thread, builder, function, end_basic_block_index);
        }
    }
}

fn void dwarf_playground(Thread* thread)
{
    auto file = file_read(thread->arena,
#ifdef __APPLE__
            strlit("/Users/david/dwarf/main.o")
#else
            strlit("/home/david/dwarf/main.o")
#endif
    );
    auto* elf_header = (ELFHeader*)file.pointer;
    auto section_count = elf_header->section_header_count;
    auto section_header_offset = elf_header->section_header_offset;
    auto string_table_section_index = elf_header->section_header_string_table_index;

    auto debug_abbrev_section_index = -1;
    auto debug_info_section_index = -1;
    auto debug_addr_section_index = -1;
    auto debug_str_section_index = -1;
    auto debug_str_offsets_section_index = -1;
    auto rela_debug_str_offsets_section_index = -1;
    auto rela_debug_addr_section_index = -1;

    auto section_headers = (ELFSectionHeader*)(file.pointer + section_header_offset);
    auto* string_table_section_header = (ELFSectionHeader*)(file.pointer + section_header_offset) + string_table_section_index;
    auto string_table_offset = string_table_section_header->offset;

    for (u16 i = 0; i < section_count; i += 1)
    {
        auto* section_header = section_headers + i;
        auto name_offset = string_table_offset + section_header->name_offset;
        auto* name = file.pointer + name_offset;
        print("Section #{u32}\t\"{cstr}\". Size: {u64}\n", i, name, section_header->size);

        if (strcmp(".debug_abbrev", (char*)name) == 0)
        {
            debug_abbrev_section_index = i;
        }
        else if (strcmp(".debug_info", (char*)name) == 0)
        {
            debug_info_section_index = i;
        }
        else if (strcmp(".debug_addr", (char*)name) == 0)
        {
            debug_addr_section_index = i;
        }
        else if (strcmp(".debug_str", (char*)name) == 0)
        {
            debug_str_section_index = i;
        }
        else if (strcmp(".debug_str_offsets", (char*)name) == 0)
        {
            debug_str_offsets_section_index = i;
        }
        else if (strcmp(".rela.debug_addr", (char*)name) == 0)
        {
            rela_debug_addr_section_index = i;
        }
        else if (strcmp(".rela.debug_str_offsets", (char*)name) == 0)
        {
            rela_debug_str_offsets_section_index = i;
        }
    }

    assert(debug_info_section_index != -1);
    assert(debug_abbrev_section_index != -1);
    assert(debug_addr_section_index != -1);
    assert(debug_str_section_index != -1);
    assert(debug_str_offsets_section_index != -1);
    assert(rela_debug_addr_section_index != -1);
    assert(rela_debug_str_offsets_section_index != -1);

    auto* debug_abbrev_section_header = section_headers + debug_abbrev_section_index;
    auto* debug_info_section_header = section_headers + debug_info_section_index;
    auto* debug_addr_section_header = section_headers + debug_addr_section_index;
    auto* debug_str_section_header = section_headers + debug_str_section_index;
    auto* debug_str_offsets_section_header = section_headers + debug_str_offsets_section_index;
    auto* rela_debug_str_offsets_section_header = section_headers + rela_debug_str_offsets_section_index;
    auto* rela_debug_addr_section_header = section_headers + rela_debug_addr_section_index;

    auto* rela_debug_str_offsets = (ElfRelocation*)(file.pointer + rela_debug_str_offsets_section_header->offset);
    auto* rela_debug_addresses = (ElfRelocation*)(file.pointer + rela_debug_addr_section_header->offset);

    auto debug_info_bytes = (String) {
        .pointer = file.pointer + debug_info_section_header->offset,
        .length = debug_info_section_header->size,
    };

    auto* compile_unit_header = (DwarfCompilationUnit*)debug_info_bytes.pointer;
    debug_info_bytes.pointer += sizeof(DwarfCompilationUnit);
    debug_info_bytes.length -= sizeof(DwarfCompilationUnit);

    auto debug_abbrev_bytes = (String) {
        .pointer = file.pointer + debug_abbrev_section_header->offset,
        .length = debug_abbrev_section_header->size,
    };
    auto* debug_addr_header = (DwarfAddressTableHeader*)(file.pointer + debug_addr_section_header->offset);
    assert(debug_addr_header->unit_length == debug_addr_section_header->size - sizeof(debug_addr_header->unit_length));
    assert(debug_addr_header->version == 5);
    assert(debug_addr_header->address_size == 8);
    auto* debug_addresses = (u64*)debug_addr_header + 1;
    auto* debug_str_offsets_header = (DwarfStringOffsetsTableHeader*)(file.pointer + debug_str_offsets_section_header->offset);
    assert(debug_str_offsets_header->unit_length == debug_str_offsets_section_header->size - sizeof(debug_str_offsets_header->unit_length));
    auto string_count = (debug_str_offsets_section_header->size - sizeof(DwarfStringOffsetsTableHeader)) / sizeof(u32);
    auto* string_index_offset_map = (u32*)(debug_str_offsets_header + 1);
    auto* string_table = file.pointer + debug_str_section_header->offset;

    auto debug_str_offset_base_guess = 8;

    auto top = 0;
    while (debug_abbrev_bytes.length > 0)
    {
        auto first = uleb128_decode(debug_abbrev_bytes);
        debug_abbrev_bytes.pointer += first.i;
        debug_abbrev_bytes.length -= first.i;

        if (first.number != 0)
        {
            auto second = uleb128_decode(debug_abbrev_bytes);
            debug_abbrev_bytes.pointer += second.i;
            debug_abbrev_bytes.length -= second.i;
            auto children = debug_abbrev_bytes.pointer[0];
            debug_abbrev_bytes.pointer += 1;
            debug_abbrev_bytes.length -= 1;

            auto di_abbrev_code = uleb128_decode(debug_info_bytes);
            debug_info_bytes.pointer += di_abbrev_code.i;
            debug_info_bytes.length -= di_abbrev_code.i;
            assert(di_abbrev_code.number == first.number);

            print("Abbreviation entry #{u64}: \"{s}\", (0x{u64:x})\n", first.number, dwarf_tag_to_string((DwarfTag)second.number), second.number);

            while (1)
            {
                auto first = uleb128_decode(debug_abbrev_bytes);
                debug_abbrev_bytes.pointer += first.i;
                debug_abbrev_bytes.length -= first.i;
                auto second = uleb128_decode(debug_abbrev_bytes);
                debug_abbrev_bytes.pointer += second.i;
                debug_abbrev_bytes.length -= second.i;

                if (first.number == 0 && second.number == 0)
                {
                    break;
                }

                auto attribute = (DwarfAttribute)first.number;
                auto form = (DwarfForm)second.number;
                print("Attribute: \"{s}\" (0x{u64:x}). Form: \"{s}\" (0x{u64:x})\n", dwarf_attribute_to_string(attribute), (u64)attribute, dwarf_form_to_string(form), (u64)form);

                switch (form)
                {
                    // .debug_str_offsets
                    case DW_FORM_strx1:
                        {
                            auto index = debug_info_bytes.pointer[0];
                            debug_info_bytes.pointer += 1;
                            debug_info_bytes.length -= 1;
                            auto* relocation = &rela_debug_str_offsets[index];
                            auto offset = relocation->addend;
                            auto* c_string = &string_table[offset];
                            print("Index: {u32}. Offset: {u32}. String: \"{cstr}\"\n", (u32)index, offset, c_string);
                        } break;
                    case DW_FORM_data1:
                        {
                            auto data = *debug_info_bytes.pointer;
                            debug_info_bytes.pointer += 1;
                            debug_info_bytes.length -= 1;
                            print("Data1: 0x{u32:x}\n", (u32)data);
                        } break;
                    case DW_FORM_data2:
                        {
                            auto data = *(u16*)debug_info_bytes.pointer;
                            debug_info_bytes.pointer += sizeof(u16);
                            debug_info_bytes.length -= sizeof(u16);
                            print("Data2: 0x{u32:x}\n", (u32)data);
                        } break;
                    case DW_FORM_data4:
                        {
                            auto data = *(u32*)debug_info_bytes.pointer;
                            debug_info_bytes.pointer += sizeof(u32);
                            debug_info_bytes.length -= sizeof(u32);
                            print("Data4: 0x{u32:x}\n", data);
                        } break;
                    case DW_FORM_sec_offset:
                        {
                            auto sec_offset = *(u32*)debug_info_bytes.pointer;
                            debug_info_bytes.pointer += sizeof(u32);
                            debug_info_bytes.length -= sizeof(u32);
                            print("Sec offset: 0x{u32:x}\n", sec_offset);
                        } break;
                    case DW_FORM_addrx:
                        {
                            auto addrx = uleb128_decode(debug_info_bytes);
                            debug_info_bytes.pointer += addrx.i;
                            debug_info_bytes.length -= addrx.i;
                            auto relocation_index = addrx.number;

                            auto* relocation = &rela_debug_addresses[relocation_index];
                            auto index = relocation->addend;

                            switch (attribute)
                            {
                                case DW_AT_low_pc:
                                    {
                                        auto address = debug_addresses[index];
                                        print("Address: 0x{u64:x}\n", address);
                                    } break;
                                default:
                                    todo();
                            }
                        } break;
                    case DW_FORM_exprloc:
                        {
                            auto length = uleb128_decode(debug_info_bytes);
                            debug_info_bytes.pointer += length.i;
                            debug_info_bytes.length -= length.i;

                            switch (length.number)
                            {
                                case 1:
                                    {
                                        switch (attribute)
                                        {
                                            case DW_AT_frame_base:
                                                {
                                                    auto b = *debug_info_bytes.pointer;
                                                    debug_info_bytes.pointer += 1;
                                                    debug_info_bytes.length -= 1;
                                                    auto operation = (DwarfOperation)b;

                                                    print("Operation: {s}\n", dwarf_operation_to_string(operation));
                                                } break;
                                            default:
                                                todo();
                                        }
                                    } break;
                                default:
                                    todo();
                            }
                        } break;
                    case DW_FORM_flag_present:
                        {
                            print("Flag present\n");
                        } break;
                    case DW_FORM_ref4:
                        {
                            auto ref4 = *(u32*)debug_info_bytes.pointer;
                            debug_info_bytes.pointer += sizeof(u32);
                            debug_info_bytes.length -= sizeof(u32);
                            print("Ref4: {u32:x}\n", ref4);
                        } break;
                    default:
                        todo();
                }
            }
        }
    }

    assert(debug_abbrev_bytes.length == 0);
    assert(debug_info_bytes.length == 1);
    assert(*debug_info_bytes.pointer == 0);
}

fn void entry_point(int argc, char* argv[], char* envp[])
{
#if DO_UNIT_TESTS
    unit_tests();
#endif

    Arena* global_arena = arena_init(MB(2), KB(64), KB(64));

    {
        arguments.length = cast(u64, s32, argc);
        arguments.pointer = arena_allocate(global_arena, String, arguments.length);

        for (int i = 0; i < argc; i += 1)
        {
            u64 len = strlen(argv[i]);
            arguments.pointer[i] = (String) {
                .pointer = (u8*)argv[i],
                .length = len,
            };
        }
    }

    Thread* thread = arena_allocate(global_arena, Thread, 1);
    thread_init(thread);

    // clang -c main.c -o main.o -g -Oz -fno-exceptions -fno-asynchronous-unwind-tables -fno-addrsig -fno-stack-protector -fno-ident
    // dwarf_playground(thread);

    if (argc < 3)
    {
        fail();
    }

    String source_file_path = arguments.pointer[1];
    CompilerBackend compiler_backend = arguments.pointer[2].pointer[0];
    u8 emit_ir = arguments.length >= 4 && arguments.pointer[3].pointer[0] == 'y';

    dir_make("nest");

    File file = {
        .path = source_file_path,
        .source = file_read(thread->arena, source_file_path),
    };
    analyze_file(thread, &file);

    if (thread->main_function == -1)
    {
        fail();
    }

    print("File path: {s}\n", source_file_path);
    auto test_dir = string_no_extension(file.path);
    print("Test dir path: {s}\n", test_dir);
    auto test_name = string_base(test_dir);
    print("Test name: {s}\n", test_name);

    if (emit_ir)
    {
        print_ir(thread);
    }
    else
    {
        code_generation(thread, (CodegenOptions) {
                .test_name = test_name,
                .backend = compiler_backend,
                }, envp);
    }

    thread_clear(thread);
}
