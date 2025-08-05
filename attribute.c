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

#include "attribute.h"

AttributeInfo attributes_get_attribute(Attributes *attrs, char *name)
{
    for (int i = 0; i < attrs->count; i++) {
        if (!strcmp(attrs->attributes[i].attribute_info.attribute, name)) {
            return attrs->attributes[i];
        }
    }
}

Attributes *attributes_new(Reader *reader, ConstantPool *pool)
{
    Attributes *attributes = malloc(sizeof(Attributes));
    attributes->count = reader_read_uint16_be(reader);
    if (attributes->count <= 0) {
        free(attributes);
        return NULL;
    }

    attributes->attributes = malloc(sizeof(AttributeInfo) * attributes->count);

    for (int i = 0; i < attributes->count; i++) {
        AttributeInfo *attr = &attributes->attributes[i];
        attr->attribute_info.index = reader_read_uint16_be(reader);
        attr->attribute_info.attribute = constant_pool_resolve_string(pool, attr->attribute_info.index);
        attr->attribute_length = reader_read_uint32_be(reader);

        if (!strcmp(attr->attribute_info.attribute, "ConstantValue")) {
            attr->constantvalue_index = reader_read_uint16_be(reader);
        } else if (!strcmp(attr->attribute_info.attribute, "Code")) {
            attr->CodeAttribute.max_stack = reader_read_uint16_be(reader);
            attr->CodeAttribute.max_locals = reader_read_uint16_be(reader);
            attr->CodeAttribute.code_length = reader_read_uint32_be(reader);
            attr->CodeAttribute.code = malloc(attr->CodeAttribute.code_length);
            reader_read_bytes(reader, attr->CodeAttribute.code, attr->CodeAttribute.code_length);

            uint16_t exception_table_length = reader_read_uint16_be(reader);
            /*
             *     {   u2 start_pc;
             *         u2 end_pc;
             *         u2 handler_pc;
             *         u2 catch_type;
             *     } exception_table[exception_table_length];
             * Skip these 8 bytes
             */
            reader_skip(reader, 8 * exception_table_length);

            // TODO: Parse these once we handle the basic attributes
            attr->CodeAttribute.attributes = attributes_new(reader, pool);
        } else if (!strcmp(attr->attribute_info.attribute, "InnerClasses")) {
        } else {
            reader_skip(reader, attr->attribute_length);
        }
    }

    return attributes;
}

void attributes_free(Attributes *attributes)
{
    if (!attributes)
        return;

    for (int i = 0; i < attributes->count; i++) {
        AttributeInfo info = attributes->attributes[i];
        if (!strcmp(info.attribute_info.attribute, "Code")) {
            attributes_free(info.CodeAttribute.attributes);
        }
    }
    free(attributes);
}