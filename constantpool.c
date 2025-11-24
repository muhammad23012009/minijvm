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

char *constant_pool_resolve_string(ConstantPool *pool, uint16_t index)
{
    ConstantPoolInfo info = pool->pool[index];
    if (info.tag == CONSTANT_CLASS) {
        // Resolve to the actual tag
        info = pool->pool[info.class_index];
    }

    if (info.tag == CONSTANT_STRING) {
        info = pool->pool[info.string_index];
    }

    if (info.tag == CONSTANT_UTF8) {
        return (char*) info.byte_ref.bytes;
    }

    return "";
}

/* Used by both methods and fields */
char *constant_pool_resolve_class_name(ConstantPool *pool, uint16_t index)
{
    ConstantPoolInfo info = pool->pool[index];
    if (info.tag == CONSTANT_METHODREF) {
        info = pool->pool[info.method_ref.class_index];
    }

    if (info.tag == CONSTANT_FIELDREF) {
        info = pool->pool[info.field_ref.class_index];
    }

    return constant_pool_resolve_string(pool, info.class_index);
}

char *constant_pool_resolve_field_name(ConstantPool *pool, uint16_t index)
{
    ConstantPoolInfo info = pool->pool[index];
    if (info.tag == CONSTANT_METHODREF) {
        info = pool->pool[info.method_ref.name_and_type_index];
    }

    if (info.tag == CONSTANT_FIELDREF) {
        info = pool->pool[info.field_ref.name_and_type_index];
    }

    return constant_pool_resolve_string(pool, info.name_and_type_info.name_index);
}

int constant_pool_resolve_int(ConstantPool *pool, uint16_t index)
{
    ConstantPoolInfo info = pool->pool[index];
    if (info.tag == CONSTANT_INT) {
        return info.int_val;
    }

    return -1;
}

/* I am not proud of this code... Need to find a better way to resolve classes */
bool constant_pool_resolve_unknowns(ConstantPool *pool, Classes *classes, Class *parent)
{
    for (int i = 1; i < pool->count; i++) {
        ConstantPoolInfo *info = &pool->pool[i];
        switch (info->tag) {
            case CONSTANT_CLASS: {
                int length;
                char class_path[2048];
                char *class_name = constant_pool_resolve_string(pool, info->class_index);
                if (!strcmp(class_name, parent->name))
                    continue;

                if (*class_name == '[') {
                    /* Java array. Skip this character */
                    class_name++;
                }

                if (*class_name == 'L' && *(class_name + strlen(class_name) - 1) == ';') {
                    /* Java object */
                    printf("Got a Java object!\n");
                    class_name++;
                    class_name[strlen(class_name) - 1] = '\0';
                }

                if (classes_get_class(classes, class_name))
                    continue;

                printf("Found unknown class %s, will try to resolve.\n", class_name);

                snprintf(class_path, 2048, "%s%s", class_name, ".class");

                if (!classes_add_class(classes, class_parse_file(classes, class_path))) {
                    fprintf(stderr, "Failed to resolve dependencies!\n");
                    return false;
                }
            }
        }
    }

    return true;
}

/* Creates the constant pool array from a data reader */
ConstantPool *constant_pool_new(Reader *reader)
{
    ConstantPool *cpool = malloc(sizeof(ConstantPool));
    cpool->count = reader_read_uint16_be(reader);
    cpool->pool = malloc(sizeof(ConstantPoolInfo) * (cpool->count + 1));

    for (int i = 1; i < cpool->count; i++) {
        ConstantPoolInfo *cp_info = &cpool->pool[i];
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
                cp_info->field_ref.class_index = reader_read_uint16_be(reader);
                cp_info->field_ref.name_and_type_index = reader_read_uint16_be(reader);
                break;
            case CONSTANT_METHODREF:
                cp_info->method_ref.class_index = reader_read_uint16_be(reader);
                cp_info->method_ref.name_and_type_index = reader_read_uint16_be(reader);
                break;
            case CONSTANT_NAMEANDTYPE:
                cp_info->name_and_type_info.name_index = reader_read_uint16_be(reader);
                cp_info->name_and_type_info.descriptor_index = reader_read_uint16_be(reader);
                break;
            case CONSTANT_DYNAMIC_INFO:
            case CONSTANT_INVOKEDYNAMIC:
                uint16_t bootstrap_index = reader_read_uint16_be(reader);
                uint16_t name_and_type_info = reader_read_uint16_be(reader);
                printf("testing!, bootstrap index: %d, name-and-type: %d\n", bootstrap_index, name_and_type_info);
                break;
            case 0xF:
                uint8_t ref_kind = reader_read_uint8(reader);
                uint16_t ref_index = reader_read_uint16_be(reader);
                printf("got method handle! kind is %d, index is %d\n", ref_kind, ref_index);
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
