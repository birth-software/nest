#include "lib.h"
#define clang_path "/usr/bin/clang"

#define RawIndex(T, i) (T ## Index) { .index = (i) }
#define Index(T, i) RawIndex(T, (i) + 1)
#define geti(i) ((i).index - 1)
#define validi(i) ((i).index != 0)
#define invalidi(T) RawIndex(T, 0)

#define InternPool(T) InternPool_ ## T
#define GetOrPut(T) T ## GetOrPut
#define declare_ip(T) \
struct InternPool(T) \
{ \
    T ## Index * pointer; \
    u32 length;\
    u32 capacity;\
}; \
typedef struct InternPool(T) InternPool(T);\
struct GetOrPut(T) \
{\
    T ## Index index; \
    u8 existing;\
};\
typedef struct GetOrPut(T) GetOrPut(T)

struct StringMapValue
{
    String string;
    u32 value;
};
typedef struct StringMapValue StringMapValue;

struct StringMap
{
    u32* pointer;
    u32 length;
    u32 capacity;
};
typedef struct StringMap StringMap;

// fn StringMapValue* string_map_values(StringMap* map)
// {
//     assert(map->pointer);
//     return (StringMapValue*)(map->pointer + map->capacity);
// }

// fn s32 string_map_find_slot(StringMap* map, u32 original_index, String key)
// {
//     s32 result = -1;
//
//     if (map->pointer)
//     {
//         auto it_index = original_index;
//         auto existing_capacity = map->capacity;
//         auto* values = string_map_values(map);
//
//         for (u32 i = 0; i < existing_capacity; i += 1)
//         {
//             auto index = it_index & (existing_capacity - 1);
//             u32 existing_key = map->pointer[index];
//
//             // Not set
//             if (existing_key == 0)
//             {
//                 result = cast(s32, u32, index);
//                 break;
//             }
//             else 
//             {
//                 auto pair = &values[index];
//                 if (s_equal(pair->string, key))
//                 {
//                     result = cast(s32, u32, index);
//                     break;
//                 }
//                 else
//                 {
//                     trap();
//                 }
//             }
//
//             it_index += 1;
//         }
//     }
//
//     return result;
// }

struct StringMapPut
{
    u32 value;
    u8 existing;
};
typedef struct StringMapPut StringMapPut;

// fn void string_map_ensure_capacity(StringMap* map, Arena* arena, u32 additional)
// {
//     auto current_capacity = map->capacity;
//     auto half_capacity = current_capacity >> 1;
//     auto destination_length = map->length + additional;
//
//     if (destination_length > half_capacity)
//     {
//         auto new_capacity = cast(u32, u64, MAX(round_up_to_next_power_of_2(destination_length), 32));
//         auto new_capacity_bytes = sizeof(u32) * new_capacity + new_capacity * sizeof(StringMapValue);
//
//         void* ptr = arena_allocate_bytes(arena, new_capacity_bytes, MAX(alignof(u32), alignof(StringMapValue)));
//         memset(ptr, 0, new_capacity_bytes);
//
//         auto* keys = (u32*)ptr;
//         auto* values = (StringMapValue*)(keys + new_capacity);
//
//         auto* old_keys = map->pointer;
//         auto old_capacity = map->capacity;
//         auto* old_values = (StringMapValue*)(map->pointer + current_capacity);
//
//         map->length = 0;
//         map->pointer = keys;
//         map->capacity = new_capacity;
//
//         for (u32 i = 0; i < old_capacity; i += 1)
//         {
//             auto key = old_keys[i];
//             if (key)
//             {
//                 unused(values);
//                 unused(old_values);
//                 trap();
//             }
//         }
//
//         for (u32 i = 0; i < old_capacity; i += 1)
//         {
//             trap();
//         }
//     }
// }

// fn StringMapPut string_map_put_at_assume_not_existent_assume_capacity(StringMap* map, u32 hash, String key, u32 value, u32 index)
// {
//     u32 existing_hash = map->pointer[index];
//     map->pointer[index] = hash;
//     auto* values = string_map_values(map);
//     auto existing_value = values[index];
//     values[index] = (StringMapValue) {
//         .value = value,
//         .string = key,
//     };
//     map->length += 1;
//     assert(existing_hash ? s_equal(existing_value.string, key) : 1);
//
//     return (StringMapPut)
//     {
//         .value = existing_value.value,
//         .existing = existing_hash != 0,
//     };
// }

// fn StringMapPut string_map_put_assume_not_existent_assume_capacity(StringMap* map, u32 hash, String key, u32 value)
// {
//     assert(map->length < map->capacity);
//     auto index = hash & (map->capacity - 1);
//     
//     return string_map_put_at_assume_not_existent_assume_capacity(map, hash, key, value, index);
// }

// fn StringMapPut string_map_put_assume_not_existent(StringMap* map, Arena* arena, u32 hash, String key, u32 value)
// {
//     string_map_ensure_capacity(map, arena, 1);
//     return string_map_put_assume_not_existent_assume_capacity(map, hash, key, value);
// }

// fn StringMapPut string_map_get(StringMap* map, String key)
// {
//     u32 value = 0;
//     auto long_hash = hash_bytes(key);
//     static_assert(sizeof(long_hash) == sizeof(u64));
//     auto hash = hash64_to_hash32(long_hash);
//     static_assert(sizeof(hash) == sizeof(u32));
//     assert(hash);
//     auto index = hash & (map->capacity - 1);
//     auto slot = string_map_find_slot(map, index, key);
//     u8 existing = slot != -1;
//     if (existing)
//     {
//         existing = map->pointer[slot] != 0;
//         auto* value_pair = &string_map_values(map)[slot];
//         value = value_pair->value;
//     }
//
//     return (StringMapPut) {
//         .value = value,
//         .existing = existing,
//     };
// }

// fn StringMapPut string_map_put(StringMap* map, Arena* arena, String key, u32 value)
// {
//     auto long_hash = hash_bytes(key);
//     static_assert(sizeof(long_hash) == sizeof(u64));
//     auto hash = hash64_to_hash32(long_hash);
//     static_assert(sizeof(hash) == sizeof(u32));
//     assert(hash);
//     auto index = hash & (map->capacity - 1);
//     auto slot = string_map_find_slot(map, index, key);
//     if (slot != -1)
//     {
//         auto* values = string_map_values(map);
//         auto* key_pointer = &map->pointer[slot];
//         auto old_key_pointer = *key_pointer;
//         *key_pointer = hash;
//         values[slot].string = key;
//         values[slot].value = value;
//         return (StringMapPut) {
//             .value = value,
//             .existing = old_key_pointer != 0,
//         };
//     }
//     else
//     {
//         if (map->length < map->capacity)
//         {
//             trap();
//         }
//         else if (map->length == map->capacity)
//         {
//             auto result = string_map_put_assume_not_existent(map, arena, hash, key, value);
//             assert(!result.existing);
//             return result;
//         }
//         else
//         {
//             trap();
//         }
//     }
// }

// fn int file_write(String file_path, String file_data)
// {
//     int file_descriptor = syscall_open(string_to_c(file_path), O_WRONLY | O_CREAT | O_TRUNC, 0644);
//     assert(file_descriptor != -1);
//
//     auto bytes = syscall_write(file_descriptor, file_data.pointer, file_data.length);
//     assert(bytes >= 0);
//     assert((u64)bytes == file_data.length);
//
//     int close_result = syscall_close(file_descriptor);
//     assert(close_result == 0);
//     return 0;
// }

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
#if SILENT == 0
        ssize_t result = syscall_write(1, message.pointer, message.length);
        assert(result >= 0);
        assert((u64)result == message.length);
#else
        unused(message);
#endif
}

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
} ELFSectionType;

struct ELFSectionHeaderFlags
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
typedef struct ELFSectionHeaderFlags ELFSectionHeaderFlags;
static_assert(sizeof(ELFSectionHeaderFlags) == sizeof(u64));

struct ELFSectionHeader
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
typedef struct ELFSectionHeader ELFSectionHeader;
static_assert(sizeof(ELFSectionHeader) == 64);
decl_vb(ELFSectionHeader);

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
    system_v_abi = 0,
    linux_abi = 3,
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

struct ELFHeader
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
typedef struct ELFHeader ELFHeader;
static_assert(sizeof(ELFHeader) == 0x40);

typedef enum ELFSymbolBinding : u8
{
    LOCAL = 0,
    GLOBAL = 1,
    WEAK = 2,
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
struct ELFSymbol
{
    u32 name_offset;
    ELFSymbolType type:4;
    ELFSymbolBinding binding:4;
    u8 other;
    u16 section_index; // In the section header table
    u64 value;
    u64 size;
};
typedef struct ELFSymbol ELFSymbol;
decl_vb(ELFSymbol);
static_assert(sizeof(ELFSymbol) == 24);

// DWARF
struct DwarfCompilationUnit
{
    u32 length;
    u16 version;
    u8 type;
    u8 address_size;
    u32 debug_abbreviation_offset;
};
typedef struct DwarfCompilationUnit DwarfCompilationUnit;

struct StringReference
{
    u32 offset;
    u32 length;
};

typedef struct NameReference NameReference;

typedef struct Thread Thread;

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

struct TypeIndex
{
    u32 index;
};

typedef struct TypeIndex TypeIndex;
#define index_equal(a, b) (a.index == b.index)
static_assert(sizeof(TypeIndex) == sizeof(u32));
declare_slice(TypeIndex);

struct TypeInteger
{
    u64 constant;
    u8 bit_count;
    u8 is_constant;
    u8 is_signed;
    u8 padding1;
    u32 padding;
};
typedef struct TypeInteger TypeInteger;
static_assert(sizeof(TypeInteger) == 16);

struct TypeTuple
{
    Slice(TypeIndex) types;
};
typedef struct TypeTuple TypeTuple;

struct Type
{
    Hash64 hash;
    union
    {
        TypeInteger integer;
        TypeTuple tuple;
    };
    TypeId id;
};
typedef struct Type Type;
static_assert(offsetof(Type, hash) == 0);
decl_vb(Type);

struct DebugTypeIndex
{
    u32 index;
};
typedef struct DebugTypeIndex DebugTypeIndex;

struct DebugTypeInteger
{
    u8 bit_count:7;
    u8 signedness:1;
};

typedef struct DebugTypeInteger DebugTypeInteger;

typedef enum DebugTypeId : u8
{
    DEBUG_TYPE_VOID = 0,
    DEBUG_TYPE_INTEGER,
} DebugTypeId;

struct DebugType
{
    union
    {
        DebugTypeInteger integer;
    };
    DebugTypeId id;
};
typedef struct DebugType DebugType;
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

struct TypePair
{
    u32 raw;
};
typedef struct TypePair TypePair;
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

struct NodeIndex
{
    u32 index;
};
typedef struct NodeIndex NodeIndex;
declare_slice(NodeIndex);
decl_vb(NodeIndex);

struct Function
{
    String name;
    NodeIndex root;
    TypePair return_type;
};
typedef struct Function Function;
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
struct NodeProjection
{
    u32 index;
};
typedef struct NodeProjection NodeProjection;

struct NodeRoot
{
    u32 function_index;
};

typedef struct NodeRoot NodeRoot;

struct NodeRegion
{
    NodeIndex in_mem;
};

typedef struct NodeRegion NodeRegion;

union NodeIntegerConstant
{
    s64 signed_value;
    u64 unsigned_value;
};

typedef union NodeIntegerConstant NodeIntegerConstant;

struct RegisterMaskIndex
{
    u32 index;
};
typedef struct RegisterMaskIndex RegisterMaskIndex;
declare_slice(RegisterMaskIndex);

struct NodeMachineCopy
{
    RegisterMaskIndex use_mask;
    RegisterMaskIndex def_mask;
};
typedef struct NodeMachineCopy NodeMachineCopy;

struct Node
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
typedef struct Node Node;
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

struct ArrayReference
{
    u32 offset;
    u32 length;
};
typedef struct ArrayReference ArrayReference;
decl_vb(ArrayReference);

struct File
{
    String path;
    String source;
    StringMap values;
    StringMap types;
};
typedef struct File File;

struct FunctionBuilder
{
    Function* function;
    File* file;
    NodeIndex current;
};
typedef struct FunctionBuilder FunctionBuilder;

// struct InternPool
// {
//     u32* pointer;
//     u32 length;
//     u32 capacity;
// };
// typedef struct InternPool InternPool;

typedef u64 BitsetElement;
decl_vb(BitsetElement);
declare_slice(BitsetElement);
struct Bitset
{
    VirtualBuffer(BitsetElement) arr;
    u32 length;
};
typedef struct Bitset Bitset;
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

struct WorkList
{
    VirtualBuffer(NodeIndex) nodes;
    Bitset visited;
    Bitset bitset;
    u32 mid_assert:1;
};
typedef struct WorkList WorkList;

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

struct RegisterMask
{
    u32 mask;
    u32 class:3;
    u32 may_spill:1;
    u32 reserved:28;
};
typedef struct RegisterMask RegisterMask;
decl_vb(RegisterMask);
declare_ip(RegisterMask);

struct Thread
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
typedef struct Thread Thread;

struct WorkListHandle
{
    u8 index:3;
    u8 is_valid:1;
    u8 reserved:4;
};

typedef struct WorkListHandle WorkListHandle;

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

typedef struct NodeDualReference NodeDualReference;

struct UseReference
{
    NodeIndex* pointer;
    u32 index;
};
typedef struct UseReference UseReference;

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

struct NodeCreate
{
    Slice(NodeIndex) inputs;
    TypePair type_pair;
    NodeId id;
};
typedef struct NodeCreate NodeCreate;

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
struct GenericInternPool
{
    u32* pointer;
    u32 length;
    u32 capacity;
};
typedef struct GenericInternPool GenericInternPool;

struct GenericInternPoolBufferResult
{
    void* pointer;
    u32 index;
};
typedef struct GenericInternPoolBufferResult GenericInternPoolBufferResult;

struct GenericGetOrPut
{
    u32 index;
    u8 existing;
};
typedef struct GenericGetOrPut GenericGetOrPut;

typedef s64 FindSlotCallback(GenericInternPool* pool, Thread* thread, Hash32 hash, u32 raw_item_index, u32 saved_index, u32 slots_ahead);
typedef GenericInternPoolBufferResult AddToBufferCallback(Thread* thread);
// typedef s64 Find

struct InternPoolInterface
{
    FindSlotCallback * const find_slot; 
    AddToBufferCallback* const add_to_buffer;
};
typedef struct InternPoolInterface InternPoolInterface;
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

struct TypeGetOrPut
{
    TypeIndex index;
    u8 existing;
};

typedef struct TypeGetOrPut TypeGetOrPut;

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

struct NodeVirtualTable
{
    NodeComputeType* const compute_type;
    NodeIdealize* const idealize;
    NodeGetHash* const get_hash;
};
typedef struct NodeVirtualTable NodeVirtualTable;

struct TypeVirtualTable
{
    TypeGetHash* const get_hash;
};
typedef struct TypeVirtualTable TypeVirtualTable;
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

struct Parser
{
    u64 i;
    u32 line;
    u32 column;
};
typedef struct Parser Parser;

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

typedef struct Parser Parser;

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
    Parser* parser = &p;
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

                function->name = parse_identifier(parser, src);
                if (s_equal(function->name, strlit("main")))
                {
                    thread->main_function = thread->buffer.functions.length - 1;
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

struct Interpreter
{
    Function* function;
    Slice(String) arguments;
};
typedef struct Interpreter Interpreter;

struct ELFOptions
{
    char* object_path;
    char* exe_path;
    Slice(u8) code;
};
typedef struct ELFOptions ELFOptions;

struct ELFBuilder
{
    VirtualBuffer(u8) file;
    VirtualBuffer(u8) string_table;
    VirtualBuffer(ELFSymbol) symbol_table;
    VirtualBuffer(ELFSectionHeader) section_table;
};
typedef struct ELFBuilder ELFBuilder;

fn u32 elf_builder_add_string(ELFBuilder* builder, String string)
{
    u32 name_offset = 0;
    if (string.length)
    {
        name_offset = builder->string_table.length;
        vb_append_bytes(&builder->string_table, string);
        *vb_add(&builder->string_table, 1) = 0;
    }

    return name_offset;
}

fn void elf_builder_add_symbol(ELFBuilder* builder, ELFSymbol symbol, String string)
{
    symbol.name_offset = elf_builder_add_string(builder, string);
    *vb_add(&builder->symbol_table, 1) = symbol;
}

fn void vb_align(VirtualBuffer(u8)* buffer, u64 alignment)
{
    auto current_length = buffer->length;
    auto target_len = align_forward(current_length, alignment);
    auto count = cast(u32, u64, target_len - current_length);
    auto* pointer = vb_add(buffer, count);
    memset(pointer, 0, count);
}

fn ELFSectionHeader* elf_builder_add_section(ELFBuilder* builder, ELFSectionHeader section, String section_name, Slice(u8) content)
{
    section.name_offset = elf_builder_add_string(builder, section_name);
    section.offset = builder->file.length;
    section.size = content.length;
    if (content.length)
    {
        vb_align(&builder->file, section.alignment);
        section.offset = builder->file.length;
        vb_append_bytes(&builder->file, content);
    }
    auto* section_header = vb_add(&builder->section_table, 1);
    *section_header = section;
    return section_header;
}

may_be_unused fn void write_elf(Thread* thread, char** envp, const ELFOptions* const options)
{
    unused(thread);
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

    ELFBuilder builder_stack = {};
    ELFBuilder* builder = &builder_stack;
    auto* elf_header = (ELFHeader*)(vb_add(&builder->file, sizeof(ELFHeader)));
    // vb_append_bytes(&file, struct_to_bytes(elf_header));
    
    // .symtab
    // Null symbol
    *vb_add(&builder->string_table, 1) = 0;
    elf_builder_add_symbol(builder, (ELFSymbol){}, (String){});
    elf_builder_add_section(builder, (ELFSectionHeader) {}, (String){}, (Slice(u8)){});

    assert(builder->string_table.length == 1);
    elf_builder_add_symbol(builder, (ELFSymbol){
        .type = ELF_SYMBOL_TYPE_FILE,
        .binding = LOCAL,
        .section_index = (u16)ABSOLUTE,
        .value = 0,
        .size = 0,
    }, strlit("main.c"));

    assert(builder->string_table.length == 8);
    elf_builder_add_symbol(builder, (ELFSymbol) {
        .type = ELF_SYMBOL_TYPE_FUNCTION,
        .binding = GLOBAL,
        .section_index = 1,
        .value = 0,
        .size = 3,
    }, strlit("main"));

    elf_builder_add_section(builder, (ELFSectionHeader) {
        .type = ELF_SECTION_PROGRAM,
        .flags = {
            .alloc = 1,
            .executable = 1,
        },
        .address = 0,
        .size = options->code.length,
        .link = 0,
        .info = 0,
        .alignment = 4,
        .entry_size = 0,
    }, strlit(".text"), options->code);

    elf_builder_add_section(builder, (ELFSectionHeader) {
        .type = ELF_SECTION_SYMBOL_TABLE,
        .link = builder->section_table.length + 1,
        // TODO: One greater than the symbol table index of the last local symbol (binding STB_LOCAL).
        .info = builder->symbol_table.length - 1,
        .alignment = alignof(ELFSymbol),
        .entry_size = sizeof(ELFSymbol),
    }, strlit(".symtab"), vb_to_bytes(builder->symbol_table));

    auto strtab_name_offset = elf_builder_add_string(builder, strlit(".strtab"));
    auto strtab_bytes = vb_to_bytes(builder->string_table);
    auto strtab_offset = builder->file.length;
    vb_append_bytes(&builder->file, strtab_bytes);

    auto* strtab_section_header = vb_add(&builder->section_table, 1);
    *strtab_section_header = (ELFSectionHeader) {
        .name_offset = strtab_name_offset,
        .type = ELF_SECTION_STRING_TABLE,
        .offset = strtab_offset,
        .size = strtab_bytes.length,
        .alignment = 1,
    };

    vb_align(&builder->file, alignof(ELFSectionHeader));
    auto section_header_offset = builder->file.length;
    vb_append_bytes(&builder->file, vb_to_bytes(builder->section_table));

    *elf_header = (ELFHeader)
    {
        .identifier = { 0x7f, 'E', 'L', 'F' },
        .bit_count = bits64,
        .endianness = little,
        .format_version = 1,
        .abi = system_v_abi,
        .abi_version = 0,
        .padding = {},
        .type = relocatable,
        .machine = x86_64,
        .version = 1,
        .entry_point = 0,
        .program_header_offset = 0,
        .section_header_offset = section_header_offset,
        .flags = 0,
        .elf_header_size = sizeof(ELFHeader),
        .program_header_size = 0,
        .program_header_count = 0,
        .section_header_size = sizeof(ELFSectionHeader),
        .section_header_count = cast(u16, u64, builder->section_table.length),
        .section_header_string_table_index = cast(u16, u64, builder->section_table.length - 1),
    };

    auto object_path_z = options->object_path;
    {
        int fd = syscall_open(object_path_z, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        assert(fd != -1);
        syscall_write(fd, builder->file.pointer, builder->file.length);

        syscall_close(fd);
    }

    char* command[] = {
        clang_path,
        object_path_z,
        "-o",
        options->exe_path,
        0,
    };
    run_command((CStringSlice) array_to_slice(command), envp);
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

struct Block
{
    NodeIndex start;
    NodeIndex end;
    NodeIndex successors[2];
    u32 successor_count;
    struct Block* parent;
};
typedef struct Block Block;

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

struct BasicBlockIndex
{
    u32 index;
};
typedef struct BasicBlockIndex BasicBlockIndex;
decl_vb(BasicBlockIndex);

struct BasicBlock
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
typedef struct BasicBlock BasicBlock;
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

struct ReadyNode
{
    u64 unit_mask;
    NodeIndex node_index;
    s32 priority;
};
typedef struct ReadyNode ReadyNode;
decl_vb(ReadyNode);

struct InFlightNode
{
    NodeIndex node_index;
    u32 end;
    s32 unit_i;
};
typedef struct InFlightNode InFlightNode;
decl_vb(InFlightNode);

struct Scheduler
{
    Bitset ready_set;
    VirtualBuffer(ReadyNode) ready;
    NodeIndex cmp;
};
typedef struct Scheduler Scheduler;

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

struct VirtualRegister
{
    RegisterMaskIndex mask;
    NodeIndex node_index;
    f32 spill_cost;
    f32 spill_bias;
    s16 class;
    s16 assigned;
    s32 hint_vreg;
};
typedef struct VirtualRegister VirtualRegister;
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

    u64 may_spill = a->may_spill && b->may_spill;
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

fn f32 get_spill_cost(Thread* thread, VirtualRegister* virtual_register)
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

fn u8 register_allocate(Thread* thread, VirtualBuffer(VirtualRegister) virtual_registers, VirtualBuffer(s32)* spills, Bitset* active, Bitset* future_active, VirtualBuffer(BasicBlockIndex) scheduled, VirtualBuffer(BasicBlock) bb, Slice(s32) order, u32 virtual_register_id, u32 in_use)
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
            *vb_add(spills, 1) = i;
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
        virtual_register->assigned = __builtin_ffsll(~in_use) - 1;
        print("Register assigned: {s}\n", gpr_to_string(virtual_register->assigned));
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

struct MachineOperand
{
    MachineOperandId id;
    s16 register_value;
};
typedef struct MachineOperand MachineOperand;

fn MachineOperand operand_from_node(Thread* thread, VirtualBuffer(VirtualRegister) virtual_registers, u32* virtual_register_map, NodeIndex node_index)
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

struct FixedBlockMap
{
    NodeIndex* keys;
    u32 count;
};
typedef struct FixedBlockMap FixedBlockMap;

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

struct CFGBuilder
{
    VirtualBuffer(NodeIndex) pinned;
    VirtualBuffer(BasicBlock) basic_blocks;
    VirtualBuffer(BasicBlockIndex) scheduled;
    FixedBlockMap block_map;
    WorkListHandle walker;
    WorkListHandle worker;
};

typedef struct CFGBuilder CFGBuilder;

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

struct CodegenOptions
{
    String test_name;
    CompilerBackend backend;
};
typedef struct CodegenOptions CodegenOptions;

fn BasicBlockIndex cfg_get_predicate_basic_block(Thread* restrict thread, CFGBuilder* restrict builder, FixedBlockMap* map, NodeIndex arg_node_index, u16 i)
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

    for (u64 i = 0; i < thread_worklist_length(thread, builder->worker); i += 1)
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
            auto basic_block_index = Index(BasicBlock, i);
            auto* basic_block = &blocks[i];

            auto new_immediate_dominator_index = invalidi(BasicBlock);

            auto start_index = basic_block->start;
            auto* start_node = thread_node_get(thread, start_index);

            auto start_inputs = node_get_inputs(thread, start_node);

            for (u16 j = 0; j < start_node->input_count; j += 1)
            {
                auto predecessor_basic_block_index = cfg_get_predicate_basic_block(thread, builder, &builder->block_map, start_index, j);
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

struct GlobalScheduleOptions
{
    u8 dataflow:1;
};

typedef struct GlobalScheduleOptions GlobalScheduleOptions;

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

        struct Elem
        {
            struct Elem* parent;
            NodeIndex node;
            u32 i;
        };
        typedef struct Elem Elem;

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
    for (u64 i = thread_worklist_length(thread, builder->worker); i > 0; i -= 1)
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
        VirtualBuffer(s32) spills = {};

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
                    .mask = class == 0 ? i : ((u64)1 << i),
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
                                auto fixed_mask = ((u32)1 << fixed);
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
                auto mask_index = virtual_registers.pointer[id].mask;
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
                        auto* node = thread_node_get(thread, node_index);

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
            auto basic_block_index = Index(BasicBlock, basic_block - builder->basic_blocks.pointer);
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
        for (u32 i = 0; i < builder->basic_blocks.length; i += 1)
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

        for (u32 i = 0; i < builder->basic_blocks.length; i += 1)
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
                auto virtual_register_id = virtual_register_map[geti(node_index)];
                auto* virtual_register = &virtual_registers.pointer[virtual_register_id];
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
                            GPR gpr = machine_operand_at(virtual_register_map, virtual_registers, node_index, REGISTER_CLASS_X86_64_GPR);
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
                            auto destination = operand_from_node(thread, virtual_registers, virtual_register_map, node_index);
                            auto source = operand_from_node(thread, virtual_registers, virtual_register_map, inputs.pointer[1]);
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
            write_elf(thread, envp, &(ELFOptions) {
                .object_path = string_to_c(object_path),
                .exe_path = exe_path,
                .code = code_slice,
            });
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

struct SchedPhi
{
    NodeIndex phi;
    NodeIndex node;
};
typedef struct SchedPhi SchedPhi;
decl_vb(SchedPhi);

fn void fill_phis(Thread* restrict thread, CFGBuilder* restrict builder, Function* restrict function, VirtualBuffer(SchedPhi)* sched_phis, Node* restrict successor_node, NodeIndex original_index)
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

typedef struct SchedNode SchedNode;
struct SchedNode
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
            fill_phis(thread, builder, function, &phis, successor_node, end_index);
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

fn void print_reference_to_node(Thread* restrict thread, CFGBuilder* restrict builder, Function* restrict function, NodeIndex node_index, u8 def)
{
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
    print_reference_to_node(thread, builder, function, basic_block->start, 1);
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
                            print_reference_to_node(thread, builder, function, inputs.pointer[i], 0);
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

#if LINK_LIBC
int main(int argc, const char* argv[], char* envp[])
{
#else
void entry_point(int argc, const char* argv[])
{
    char** envp = (char**)&argv[argc + 1];
    unused(envp);
#endif
#if DO_UNIT_TESTS
    unit_tests();
#endif

    // calibrate_cpu_timer();

    if (argc < 3)
    {
        fail();
    }

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

    String source_file_path = arguments.pointer[1];
    CompilerBackend compiler_backend = arguments.pointer[2].pointer[0];
    u8 emit_ir = arguments.length >= 4 && arguments.pointer[3].pointer[0] == 'y';

    Thread* thread = arena_allocate(global_arena, Thread, 1);
    thread_init(thread);

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
#if LINK_LIBC == 0
    syscall_exit(0);
#endif
}

#if LINK_LIBC == 0
[[gnu::naked]] [[noreturn]] void _start()
{
    __asm__ __volatile__(
            "\nxor %ebp, %ebp"
            "\npopq %rdi"
            "\nmov %rsp, %rsi"
            "\nand $~0xf, %rsp"
            "\npushq %rsp"
            "\npushq $0"
            "\ncallq entry_point"
            "\nud2\n"
       );
}
#endif
