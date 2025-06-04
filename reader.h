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

#ifndef READER_H
#define READER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <endian.h>

typedef struct Reader {
    void *data;
    int size;
    int offset;
} Reader;

extern Reader *reader_new(void *data, int size);

extern uint8_t reader_read_uint8(Reader *reader);
extern uint16_t reader_read_uint16(Reader *reader);
extern uint16_t reader_read_uint16_be(Reader *reader);
extern uint32_t reader_read_uint32(Reader *reader);
extern uint32_t reader_read_uint32_be(Reader *reader);
extern void reader_read_bytes(Reader *reader, char *dest, int len);
extern void reader_skip(Reader *reader, int length);

#endif