#ifndef __TCP_REPLY_BUFFER_H__
#define __TCP_REPLY_BUFFER_H__

#include <stdint.h>

typedef struct buffer_s {
    uint8_t *data;
    size_t  length;
    size_t  capacity;
} buffer_t;

buffer_t *buffer_new(size_t capacity);
void buffer_construct(buffer_t *buf, size_t capacity);

void buffer_destroy(buffer_t *buf);
void buffer_destruct(buffer_t *buf);

void buffer_prepend(buffer_t *buf, uint8_t *data, size_t len);
void buffer_append(buffer_t *buf, uint8_t *data, size_t len);
void buffer_insert(buffer_t *buf, size_t pos, uint8_t *data, size_t len);

void buffer_popfront(buffer_t *buf, size_t len);
void buffer_popend(buffer_t *buf, size_t len);
void buffer_erase(buffer_t *buf, size_t pos, size_t len);

uint8_t *buffer_data(buffer_t *buf);
uint8_t *buffer_end(buffer_t *buf);
size_t buffer_size(buffer_t *buf);
size_t buffer_remain(buffer_t *buf);
size_t buffer_capacity(buffer_t *buf);

void buffer_reserve(buffer_t *buf, size_t new_capacity);

#endif // __TCP_REPLY_BUFFER_H__
