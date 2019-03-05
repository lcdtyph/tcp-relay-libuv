#include <stdlib.h>
#include <memory.h>

#include "buffer.h"

static size_t min(size_t a, size_t b) {
    return (a < b) ? a : b;
}

static size_t max(size_t a, size_t b) {
    return (a > b) ? a : b;
}

buffer_t *buffer_new(size_t capacity) {
    buffer_t *result = (buffer_t *)malloc(sizeof(buffer_t));
    buffer_construct(result, capacity);
    return result;
}

void buffer_construct(buffer_t *buf, size_t capacity) {
    buf->data = (uint8_t *)malloc(sizeof(*(buf->data)) * capacity);
    buf->length = 0;
    buf->capacity = capacity;
    memset(buffer_data(buf), 0, buffer_capacity(buf));
}

void buffer_destroy(buffer_t *buf) {
    buffer_destruct(buf);
    free(buf);
}

void buffer_destruct(buffer_t *buf) {
    free(buf->data);
}

void buffer_clear(buffer_t *buf) {
    buf->length = 0;
}

void buffer_resize(buffer_t *buf, size_t new_len) {
    buffer_reserve(buf, new_len);
    buf->length = new_len;
}

void buffer_reserve(buffer_t *buf, size_t new_capacity) {
    if (new_capacity <= buffer_capacity(buf)) {
        return;
    }
    buf->data = (uint8_t *)realloc(buf->data, new_capacity);
    memset(buffer_data(buf) + buffer_capacity(buf), 0, new_capacity - buffer_capacity(buf));
    buf->capacity = new_capacity;
}

void buffer_prepend(buffer_t *buf, uint8_t *data, size_t len) {
    buffer_insert(buf, 0, data, len);
}

void buffer_append(buffer_t *buf, uint8_t *data, size_t len) {
    buffer_insert(buf, buffer_size(buf), data, len);
}

void buffer_popfront(buffer_t *buf, size_t len) {
    buffer_erase(buf, 0, len);
}

void buffer_popend(buffer_t *buf, size_t len) {
    len = min(buffer_size(buf), len);
    buffer_erase(buf, buffer_size(buf) - len, len);
}

uint8_t *buffer_data(buffer_t *buf) {
    return buf->data;
}

uint8_t *buffer_end(buffer_t *buf) {
    return buffer_data(buf) + buffer_size(buf);
}

size_t buffer_size(buffer_t *buf) {
    return buf->length;
}

size_t buffer_remain(buffer_t *buf) {
    return buffer_capacity(buf) - buffer_size(buf);
}

size_t buffer_capacity(buffer_t *buf) {
    return buf->capacity;
}

void buffer_insert(buffer_t *buf, size_t pos, uint8_t *data, size_t len) {
    size_t new_size = max(buffer_size(buf), pos);
    buffer_resize(buf, new_size + len);
    uint8_t *old_end = buffer_end(buf) - len;

    uint8_t *ins_pos = buffer_data(buf) + pos;
    if (ins_pos < old_end) {
        memmove(ins_pos + len, ins_pos, old_end - ins_pos);
    }
    memcpy(ins_pos, data, len);
}

void buffer_erase(buffer_t *buf, size_t pos, size_t len) {
    if (pos >= buffer_size(buf)) {
        return;
    }
    len = min(buffer_size(buf) - pos, len);
    uint8_t *range_beg = buffer_data(buf) + pos;
    uint8_t *range_end = range_beg + len;
    if (range_end < buffer_end(buf)) {
        memmove(range_beg, range_end, len);
    }
    buf->length = buffer_size(buf) - len;
}

