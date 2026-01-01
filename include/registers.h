#ifndef REGISTERS_H
#define REGISTERS_H

#include <stdint.h>

// تعريف السجلات الرئيسية
#define NUM_REGISTERS 16

// أنواع السجلات
typedef enum {
    REG_TYPE_GENERAL,
    REG_TYPE_STACK,
    REG_TYPE_BASE,
    REG_TYPE_INDEX,
    REG_TYPE_POINTER
} RegisterType;

// بنية السجل
typedef struct {
    char name[8];
    int number;
    RegisterType type;
    int bits;  // 32 أو 64
    const char *description;
} RegisterInfo;

// جدول السجلات لمعمارية x64
static RegisterInfo registers[] = {
    // السجلات العامة
    {"RAX", 0, REG_TYPE_GENERAL, 64, "Accumulator"},
    {"RCX", 1, REG_TYPE_GENERAL, 64, "Counter"},
    {"RDX", 2, REG_TYPE_GENERAL, 64, "Data"},
    {"RBX", 3, REG_TYPE_GENERAL, 64, "Base"},
    {"RSP", 4, REG_TYPE_STACK, 64, "Stack Pointer"},
    {"RBP", 5, REG_TYPE_BASE, 64, "Base Pointer"},
    {"RSI", 6, REG_TYPE_INDEX, 64, "Source Index"},
    {"RDI", 7, REG_TYPE_INDEX, 64, "Destination Index"},
    
    // السجلات الممتدة
    {"R8", 8, REG_TYPE_GENERAL, 64, "Extended Register 8"},
    {"R9", 9, REG_TYPE_GENERAL, 64, "Extended Register 9"},
    {"R10", 10, REG_TYPE_GENERAL, 64, "Extended Register 10"},
    {"R11", 11, REG_TYPE_GENERAL, 64, "Extended Register 11"},
    {"R12", 12, REG_TYPE_GENERAL, 64, "Extended Register 12"},
    {"R13", 13, REG_TYPE_GENERAL, 64, "Extended Register 13"},
    {"R14", 14, REG_TYPE_GENERAL, 64, "Extended Register 14"},
    {"R15", 15, REG_TYPE_GENERAL, 64, "Extended Register 15"},
    
    // أجزاء السجلات 32-bit
    {"EAX", 0, REG_TYPE_GENERAL, 32, "32-bit EAX"},
    {"ECX", 1, REG_TYPE_GENERAL, 32, "32-bit ECX"},
    {"EDX", 2, REG_TYPE_GENERAL, 32, "32-bit EDX"},
    {"EBX", 3, REG_TYPE_GENERAL, 32, "32-bit EBX"},
    {"ESP", 4, REG_TYPE_STACK, 32, "32-bit Stack Pointer"},
    {"EBP", 5, REG_TYPE_BASE, 32, "32-bit Base Pointer"},
    {"ESI", 6, REG_TYPE_INDEX, 32, "32-bit Source Index"},
    {"EDI", 7, REG_TYPE_INDEX, 32, "32-bit Destination Index"},
    
    // أجزاء السجلات 16-bit
    {"AX", 0, REG_TYPE_GENERAL, 16, "16-bit AX"},
    {"CX", 1, REG_TYPE_GENERAL, 16, "16-bit CX"},
    {"DX", 2, REG_TYPE_GENERAL, 16, "16-bit DX"},
    {"BX", 3, REG_TYPE_GENERAL, 16, "16-bit BX"},
    {"SP", 4, REG_TYPE_STACK, 16, "16-bit Stack Pointer"},
    {"BP", 5, REG_TYPE_BASE, 16, "16-bit Base Pointer"},
    {"SI", 6, REG_TYPE_INDEX, 16, "16-bit Source Index"},
    {"DI", 7, REG_TYPE_INDEX, 16, "16-bit Destination Index"},
    
    // أجزاء السجلات 8-bit
    {"AL", 0, REG_TYPE_GENERAL, 8, "Low 8-bit of AX"},
    {"CL", 1, REG_TYPE_GENERAL, 8, "Low 8-bit of CX"},
    {"DL", 2, REG_TYPE_GENERAL, 8, "Low 8-bit of DX"},
    {"BL", 3, REG_TYPE_GENERAL, 8, "Low 8-bit of BX"},
    {"AH", 0, REG_TYPE_GENERAL, 8, "High 8-bit of AX"},
    {"CH", 1, REG_TYPE_GENERAL, 8, "High 8-bit of CX"},
    {"DH", 2, REG_TYPE_GENERAL, 8, "High 8-bit of DX"},
    {"BH", 3, REG_TYPE_GENERAL, 8, "High 8-bit of BX"},
    
    // سجلات خاصة
    {"RIP", 16, REG_TYPE_POINTER, 64, "Instruction Pointer"},
    {"EFLAGS", 17, REG_TYPE_GENERAL, 32, "Flags Register"},
    
    {0} // نهاية الجدول
};

// دوال للتعامل مع السجلات
int get_register_number(const char *name);
const char* get_register_name(int reg_num);
int get_register_size(const char *name);
RegisterType get_register_type(int reg_num);
int is_valid_register(const char *name);
int is_64bit_register(int reg_num);
int is_32bit_register(int reg_num);
int is_16bit_register(int reg_num);
int is_8bit_register(int reg_num);

// دوال لتسجيل المعاملات
typedef struct {
    uint8_t modrm;
    uint8_t sib;
    uint8_t rex;
    int has_displacement;
    int32_t displacement;
    int scale;
} ModRMInfo;

ModRMInfo encode_modrm(int reg_opcode, const char *base_reg, const char *index_reg, int scale, int32_t displacement);

#endif
