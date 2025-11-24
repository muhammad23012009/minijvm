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

#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H

#include <string.h>
#include <stdint.h>
#include "constantpool.h"
#include "reader.h"

typedef struct ConstantPool ConstantPool;

typedef struct AttributeInfo {
    struct {
        uint16_t index;
        char *attribute;
    } attribute_info;
    uint32_t attribute_length;

    union {
        struct {
            uint16_t max_stack;
            uint16_t max_locals;
            uint32_t code_length;
            uint8_t *code;
            // TODO: Parse exceptions and optional attributes
            struct Attributes *attributes;
        } CodeAttribute;

        struct {
            uint8_t num_bootstrap_methods;
            struct {
                uint8_t bootstrap_method_ref;
                uint8_t num_bootstrap_arguments;
                uint8_t *bootstrap_arguments;
            } *bootstrap_methods;
        } BootstrapMethodsAttribute;

        uint16_t constantvalue_index;
    };
} AttributeInfo;

typedef struct Attributes {
    uint16_t count;
    AttributeInfo *attributes;
} Attributes;

/* Helper functions */
extern AttributeInfo attributes_get_attribute(Attributes *attrs, char *name);

extern Attributes *attributes_new(Reader *reader, ConstantPool *pool);
extern void attributes_free(Attributes *attributes);

#endif