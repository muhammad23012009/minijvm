/* 
 * This file is part of MiniJVM (https://github.com/muhammad23012009/minijvm)
 * Copyright (c) 2025 Muhammad  <thevancedgamer@mentallysanemainliners.org>
 * 
 * This program is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CONSTANTPOOL_H
#define CONSTANTPOOL_H

#include <stdint.h>
#include "method.h"
#include "reader.h"

/* Constant pool tags */
#define CONSTANT_UTF8       0x01
#define CONSTANT_INT        0x03
#define CONSTANT_FLOAT      0x04
#define CONSTANT_LONG       0x05
#define CONSTANT_DOUBLE     0x06
#define CONSTANT_CLASS      0x07
#define CONSTANT_STRING     0x08
#define CONSTANT_FIELDREF   0x09
#define CONSTANT_METHODREF  0x0A
#define CONSTANT_NAMEANDTYPE    0x0C

typedef struct Classes Classes;

typedef struct ConstantPoolInfo {
    uint8_t tag;
    union {
        uint32_t int_val;
        uint32_t float_val;
        uint16_t string_index;
        uint16_t name_index;

        uint16_t class_index;

        struct {
            uint32_t high_bytes;
            uint32_t low_bytes;
        } long_val;

        struct {
            uint16_t class_index;
            uint16_t name_and_type_index;
        } method_ref;

        struct {
            uint16_t length;
            uint8_t *bytes;
        } byte_ref;

        struct {
            uint16_t name_index;
            uint16_t descriptor_index;
        } name_and_type_info;
    };
} ConstantPoolInfo;

typedef struct ConstantPool {
    uint16_t count;
    /* Array of pool items */
    ConstantPoolInfo *pool;

    /* All of the parsed classes */
    Classes *classes;
} ConstantPool;

typedef struct Interface {
    uint16_t index;
    char *interface;
} Interface;

/* Helper functions */
extern char *constant_pool_resolve_string(ConstantPool *pool, uint16_t index);
extern ConstantPool *constant_pool_new(Reader *reader);
extern void constant_pool_free(ConstantPool *pool);

#endif