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

#include <string.h>

#include "reader.h"

uint8_t reader_read_uint8(Reader *reader)
{
    void *ptr = &reader->data[reader->offset];
    reader->offset += 1;
    return *(uint8_t*)ptr;
}

uint16_t reader_read_uint16(Reader *reader)
{
    void *val = &reader->data[reader->offset];
    reader->offset += 2;
    return *(uint16_t*)val;
}

uint16_t reader_read_uint16_be(Reader *reader)
{
    void *ptr = &reader->data[reader->offset];
    reader->offset += 2;
    uint16_t val = htobe16(*(uint16_t*)ptr);
    return val;
}

uint32_t reader_read_uint32(Reader *reader)
{
    void *ptr = &reader->data[reader->offset];
    reader->offset += 4;
    uint32_t val = htole32(*(uint32_t*)ptr);
    return val;
}

uint32_t reader_read_uint32_be(Reader *reader)
{
    void *ptr = &reader->data[reader->offset];
    reader->offset += 4;
    uint32_t val = htobe32(*(uint32_t*)ptr);
    return val;
}

void reader_read_bytes(Reader *reader, char *dest, int len)
{
    void *ptr = &reader->data[reader->offset];
    memcpy(dest, ptr, len);
    reader->offset += len;
}

void reader_skip(Reader *reader, int length)
{
    reader->offset += length;
}

Reader *reader_new(void *data, int size)
{
    Reader *reader = malloc(sizeof(Reader));
    reader->data = data;
    reader->size = size;
    reader->offset = 0;
}

void reader_free(Reader *reader)
{
    free(reader);
}