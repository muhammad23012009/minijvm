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

#include "constantpool.h"
#include "reader.h"

char *constant_pool_resolve_string(ConstantPool *pool, uint16_t index)
{
    ConstantPoolInfo info = pool->pool[index - 1];
    if (info.tag == CONSTANT_CLASS) {
        // Resolve to the actual tag
        info = pool->pool[info.class_index - 1];
    }

    if (info.tag == CONSTANT_STRING) {
        info = pool->pool[info.string_index - 1];
    }

    if (info.tag == CONSTANT_UTF8) {
        return (char*) info.byte_ref.bytes;
    }

    return "";
}

/* Creates the constant pool array from a data reader */
ConstantPool *constant_pool_new(Reader *reader)
{
    ConstantPool *cpool = malloc(sizeof(ConstantPool));
    cpool->count = reader_read_uint16_be(reader);
    cpool->pool = malloc(sizeof(ConstantPoolInfo) * cpool->count);
    cpool->classes = malloc(sizeof(Classes));

    for (int i = 1; i < cpool->count; i++) {
        ConstantPoolInfo *cp_info = &cpool->pool[i - 1];
        cp_info->tag = reader_read_uint8(reader);
        switch (cp_info->tag) {
            case CONSTANT_UTF8:
                cp_info->byte_ref.length = reader_read_uint16_be(reader);
                cp_info->byte_ref.bytes = malloc(cp_info->byte_ref.length);
                reader_read_bytes(reader, cp_info->byte_ref.bytes, cp_info->byte_ref.length);
                break;
            case CONSTANT_INT:
                cp_info->int_val = reader_read_uint32_be(reader);
                break;
            case CONSTANT_CLASS:
                cp_info->class_index = reader_read_uint16_be(reader);
                break;
            case CONSTANT_STRING:
                cp_info->string_index = reader_read_uint16_be(reader);
                break;
            case CONSTANT_FIELDREF:
            case CONSTANT_METHODREF:
                cp_info->method_ref.class_index = reader_read_uint16_be(reader);
                cp_info->method_ref.name_and_type_index = reader_read_uint16_be(reader);
                break;
            case CONSTANT_NAMEANDTYPE:
                cp_info->name_and_type_info.name_index = reader_read_uint16_be(reader);
                cp_info->name_and_type_info.descriptor_index = reader_read_uint16_be(reader);
                break;
            default:
                printf("Unknown constant pool type 0x%x!\n", cp_info->tag);
        }
    }

    return cpool;
}

void constant_pool_free(ConstantPool *pool)
{
    for (int i = 0; i < pool->count; i++) {
        ConstantPoolInfo info = pool->pool[i];
        switch (info.tag) {
            case CONSTANT_UTF8:
                free(info.byte_ref.bytes);
            default:
                break;
        }
    }

    free(pool->pool);
    free(pool);
}