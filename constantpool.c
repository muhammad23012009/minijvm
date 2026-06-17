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

uint8_t constant_pool_get_tag(ConstantPool *pool, uint16_t index)
{
    return pool->pool[index].tag;
}

const char *constant_pool_resolve_string(ConstantPool *pool, uint16_t index)
{
    ConstantPoolInfo *info = &pool->pool[index];
    if (info->tag == CONSTANT_CLASS) {
        // Resolve to the actual tag
        info = &pool->pool[info->class_index];
    }

    if (info->tag == CONSTANT_STRING) {
        info = &pool->pool[info->string_index];
    }

    if (info->tag == CONSTANT_UTF8) {
        return (char*) info->byte_ref.bytes;
    }

    return "";
}

/* Used by both methods and fields */
const char *constant_pool_resolve_class_name(ConstantPool *pool, uint16_t index)
{
    ConstantPoolInfo *info = &pool->pool[index];
    if (info->tag == CONSTANT_METHODREF) {
        info = &pool->pool[info->method_ref.class_index];
    }

    if (info->tag == CONSTANT_FIELDREF) {
        info = &pool->pool[info->field_ref.class_index];
    }

    return constant_pool_resolve_string(pool, info->class_index);
}

const char *constant_pool_resolve_field_name(ConstantPool *pool, uint16_t index)
{
    ConstantPoolInfo *info = &pool->pool[index];
    if (info->tag == CONSTANT_METHODREF) {
        info = &pool->pool[info->method_ref.name_and_type_index];
    }

    if (info->tag == CONSTANT_FIELDREF) {
        info = &pool->pool[info->field_ref.name_and_type_index];
    }

    return constant_pool_resolve_string(pool, info->name_and_type_info.name_index);
}

const char *constant_pool_resolve_field_descriptor(ConstantPool *pool, uint16_t index)
{
    ConstantPoolInfo *info = &pool->pool[index];
    if (info->tag == CONSTANT_METHODREF) {
        info = &pool->pool[info->method_ref.name_and_type_index];
    }

    if (info->tag == CONSTANT_FIELDREF) {
        info = &pool->pool[info->field_ref.name_and_type_index];
    }

    return constant_pool_resolve_string(pool, info->name_and_type_info.descriptor_index);
}

int constant_pool_resolve_int(ConstantPool *pool, uint16_t index)
{
    ConstantPoolInfo *info = &pool->pool[index];
    if (info->tag == CONSTANT_INT) {
        return info->int_val;
    }

    return -1;
}

uint32_t constant_pool_resolve_float(ConstantPool *pool, uint16_t index)
{
    ConstantPoolInfo *info = &pool->pool[index];
    if (info->tag == CONSTANT_FLOAT) {
        return info->float_val;
    }

    return -1;
}

/* Creates the constant pool array from a data reader */
ConstantPool *constant_pool_new(Reader *reader)
{
    ConstantPool *cpool = malloc(sizeof(ConstantPool));
    cpool->count = reader_read_uint16_be(reader);
    cpool->pool = calloc(cpool->count, sizeof(ConstantPoolInfo));

    for (int i = 1; i < cpool->count; i++) {
        ConstantPoolInfo *cp_info = &cpool->pool[i];
        cp_info->tag = reader_read_uint8(reader);
        switch (cp_info->tag) {
            case CONSTANT_UTF8:
                cp_info->byte_ref.length = reader_read_uint16_be(reader);
                cp_info->byte_ref.bytes = malloc(cp_info->byte_ref.length + 1);
                reader_read_bytes(reader, (char*)cp_info->byte_ref.bytes, cp_info->byte_ref.length);
                cp_info->byte_ref.bytes[cp_info->byte_ref.length] = '\0';
                break;
            case CONSTANT_INT:
                cp_info->int_val = reader_read_uint32_be(reader);
                break;
            case CONSTANT_FLOAT:
                cp_info->float_val = reader_read_uint32_be(reader);
                break;
            case CONSTANT_LONG:
                cp_info->long_val.high_bytes = reader_read_uint32_be(reader);
                cp_info->long_val.low_bytes = reader_read_uint32_be(reader);
                i += 1; /* Long and double take up two entries in the constant pool */
                break;
            case CONSTANT_DOUBLE:
                cp_info->double_val.high_bytes = reader_read_uint32_be(reader);
                cp_info->double_val.low_bytes = reader_read_uint32_be(reader);
                i += 1; /* Long and double take up two entries in the constant pool */
                break;
            case CONSTANT_CLASS:
                cp_info->class_index = reader_read_uint16_be(reader);
                break;
            case CONSTANT_STRING:
                cp_info->string_index = reader_read_uint16_be(reader);
                break;
            case CONSTANT_FIELDREF:
                cp_info->field_ref.class_index = reader_read_uint16_be(reader);
                cp_info->field_ref.name_and_type_index = reader_read_uint16_be(reader);
                break;
            case CONSTANT_METHODREF:
            case CONSTANT_INTERFACE_METHODREF:
                cp_info->method_ref.class_index = reader_read_uint16_be(reader);
                cp_info->method_ref.name_and_type_index = reader_read_uint16_be(reader);
                break;
            case CONSTANT_METHODTYPE:
                cp_info->name_and_type_info.descriptor_index = reader_read_uint16_be(reader);
                break;
            case CONSTANT_NAMEANDTYPE:
                cp_info->name_and_type_info.name_index = reader_read_uint16_be(reader);
                cp_info->name_and_type_info.descriptor_index = reader_read_uint16_be(reader);
                break;
            case CONSTANT_DYNAMIC_INFO:
            case CONSTANT_INVOKEDYNAMIC:
                cp_info->dynamic_info.bootstrap_index = reader_read_uint16_be(reader);
                cp_info->dynamic_info.name_and_type_info = reader_read_uint16_be(reader);
                break;
            case CONSTANT_METHODHANDLE:
                cp_info->method_handle_info.ref_kind = reader_read_uint8(reader);
                cp_info->method_handle_info.ref_index = reader_read_uint16_be(reader);
                break;
            default:
                fprintf(stderr, "Unknown constant pool type 0x%x!\n", cp_info->tag);
                exit(1);
        }
    }

    return cpool;
}

void constant_pool_free(ConstantPool *pool)
{
    for (int i = 1; i < pool->count; i++) {
        ConstantPoolInfo *info = &pool->pool[i];

        switch (info->tag) {
            case CONSTANT_UTF8:
                free(info->byte_ref.bytes);
            default:
                break;
        }
    }

    free(pool->pool);
    free(pool);
}
