#ifndef STRBUF_H
#define STRBUF_H

#include <inttypes.h>
#include <memory.h>
#include <stdlib.h>

typedef struct {
    size_t capacity;
    size_t size;
    char* data;
} strbuf;

#define STRBUF_DEFAULT_CAP 8

#define STRBUF_IS_FULL(v) (v->size == v->capacity)
#define STRBUF_INC_SZ(buf)                                    \
    char* nb = (char*)realloc(buf->data, buf->capacity << 1); \
    if (!nb) return 1;                                        \
    buf->data = nb;                                           \
    buf->capacity <<= 1;

static inline strbuf* STRBUF_new_with_capacity(size_t capacity) {
    strbuf* buf = (strbuf*)malloc(sizeof(strbuf));
    buf->capacity = capacity;
    buf->data = (char*)malloc(capacity);
    buf->size = 0;
    return buf;
}

static inline strbuf* STRBUF_new() {
    return STRBUF_new_with_capacity(STRBUF_DEFAULT_CAP);
}

static inline void STRBUF_free(strbuf* buf) {
    if (!buf) return;
    if (buf->data) {
        free(buf->data);
    }
    free(buf);
}

static inline int STRBUF_pushback(strbuf* buf, char c) {
    if (STRBUF_IS_FULL(buf)) {
        STRBUF_INC_SZ(buf);
    }
    buf->data[buf->size] = c;
    buf->size++;
    return 0;
}

static inline int STRBUF_append(strbuf* buf, char* str, size_t len) {
    while (buf->size + len >= buf->capacity) {
        STRBUF_INC_SZ(buf);
    }
    memcpy(buf->data + buf->size, str, len);
    buf->size += len;
    return 0;
}

static inline int STRBUF_reverse(strbuf* buf) {
    if (buf->size <= 1) return 0;
    char* end = buf->data + buf->size - 1;
    char* start = buf->data;
    while (start < end) {
        char tmp = *start;
        *start = *end;
        *end = tmp;
        start++;
        end--;
    }
    return 0;
}

static inline char* STRBUF_tocstr(const strbuf* buf) {
    char* res = (char*)malloc(buf->size + 1);
    memcpy(res, buf->data, buf->size);
    res[buf->size] = 0;
    return res;
}

#endif