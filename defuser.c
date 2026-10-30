#include "defuser.h"

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#define MAX_FIELDS 8

/*
 * 把 values 中的 count 个整数写成一行“空格分隔的无符号十进制整数”，存入 out。
 * 成功返回 0；count 为 0（还没有实现）、snprintf 出错或结果被截断时返回非零。
 */
static int write_answer(char *out, size_t cap, const uint32_t *values, size_t count) {
    size_t used = 0;
    for (size_t i = 0; i < count; ++i) {
        int size = snprintf(out + used, cap - used, "%s%" PRIu32, i ? " " : "", values[i]);
        if (size < 0 || (size_t)size >= cap - used) return 1;
        used += (size_t)size;
    }
    return count == 0;
}



int solve_phase_1(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 1;

    uint32_t a = (uint32_t)(challenge >> 32);
    uint32_t b = (uint32_t)(challenge >> 16);
    uint32_t d = (uint32_t)challenge;

    uint32_t r1 = (b ^ 0xa98d) + 4 * a;
    uint32_t r0 = (a + 0x1283) ^ d;

    values[0] = (5 * r0 + r1) ^ 0x622f;
    return write_answer(out, cap, values, count);
}

int solve_phase_2(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 6;

    values[0] = (uint32_t)((challenge >> 29) & 0xf);

    for (uint32_t i = 1; i < 6; i++) {
        values[i] = (uint32_t)(((challenge >> (2 * i)) & 0xf) + 14)
                    ^ (3 * values[i - 1] - i);
    }

    return write_answer(out, cap, values, count);
}


int solve_phase_3(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 2;

    uint32_t x = (uint32_t)(challenge >> 32)
               ^ (uint32_t)(challenge >> 3);

    values[0] = (uint32_t)challenge & 0x7;

    switch (values[0]) {
        case 0: values[1] = (x ^ 0x408d) + (x >> 1); break;
        case 1: values[1] = (2 * x) ^ 0x2c7f; break;
        case 2: values[1] = (x ^ 0xa7c2) + (x >> 1); break;
        case 3: values[1] = 5 * x + 0x14f91; break;
        case 4: values[1] = 5 * x + 0x735; break;
        case 5: values[1] = 3 * x + 0xb2b4; break;
        case 6: values[1] = 5 * x + 0x44ad9; break;
        case 7: values[1] = (4 * x) ^ 0x448c; break;
    }

    return write_answer(out, cap, values, count);
}

static uint32_t p4_walk(uint32_t target, uint32_t low,
                        uint32_t high, uint32_t salt) {
    uint32_t mid = low + (high - low) / 2;

    if (target == mid)
        return (mid + 250) ^ salt;

    if (target < mid) {
        uint32_t result = p4_walk(target, low, mid - 1, salt);
        return ((5 * result) ^ (mid + salt)) ^ 0x3;
    }

    uint32_t result = p4_walk(target, mid + 1, high, salt);
    return ((5 * result) ^ (mid + salt)) ^ 0xd;
}

int solve_phase_4(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 1;

    uint32_t target = (uint32_t)challenge & 0x7f;
    uint32_t salt = (uint32_t)(challenge >> 7)
                  ^ (uint32_t)(challenge >> 32);

    values[0] = p4_walk(target, 0, 127, salt);

    return write_answer(out, cap, values, count);
}


int solve_phase_5(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 2;

    const int heads[8] = {2, 6, 8, 3, 5, 11, 4, 1};

    const int next[12] = {
        -1, 9, 6, 5, 1, 11, 8, 10, 3, 7, 0, 4
    };

    const uint32_t data[12] = {
        0xb17, 0x634, 0x40d, 0x05e,
        0x557, 0x97c, 0x35d, 0x133,
        0x233, 0xa59, 0x71d, 0x83d
    };

    const uint32_t tag[12] = {
        0x10da, 0xbffe, 0xf17f, 0x6e5e,
        0xeb34, 0xd42a, 0x4ece, 0x63ae,
        0xf1a2, 0x81aa, 0x5077, 0xdd52
    };

    int node = heads[challenge & 7];
    uint32_t steps = (uint32_t)((challenge >> 3) & 3);

    uint32_t result = (uint32_t)(challenge >> 6)
                    ^ (uint32_t)(challenge >> 32)
                    ^ tag[node] ^ 0x102b;

    result += data[node];

    for (uint32_t i = 0; i < steps; i++) {
        node = next[node];
        result = (result ^ tag[node]) + data[node];
    }

    values[0] = result;
    values[1] = data[node];

    return write_answer(out, cap, values, count);
}


static uint32_t p6_op(uint32_t x, uint32_t index) {
    switch (index) {
        case 0:
            return (x ^ 0xf9b1) + (x << 5);
        case 1:
            return (4 * x + 0x98fc) ^ (7 * x);
        case 2:
            return ((x << 3) | (x >> 29)) ^ 0x8f36;
        case 3:
            return ((x >> 5) ^ x) + 0xac08;
        case 4:
            return (x >> 2) | (x << 30);
        case 5:
        case 6:
            return (x + 0xecdc) ^ (x >> 2);
        case 7:
            return (x >> 2) | (x << 30);
        default:
            return 0;
    }
}



int solve_phase_6(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 1;

    uint32_t x = (uint32_t)(challenge >> 6)
               ^ (uint32_t)(challenge >> 32);

    uint32_t i = (uint32_t)(challenge & 7);
    uint32_t j = (uint32_t)((challenge >> 3) & 7);

    uint32_t result = p6_op(x, i);
    result ^= 0x3926;
    result = p6_op(result, j);

    values[0] = result + 0x88ef;

    return write_answer(out, cap, values, count);
}



int solve_secret(uint64_t challenge, uint64_t context,
                 char *out, size_t cap) {
    uint32_t v0 = (uint32_t)context & 3;

    uint32_t a = (uint32_t)(challenge >> 32)
               ^ (uint32_t)challenge;

    uint32_t b = (uint32_t)(context >> 32)
               ^ (uint32_t)context;

    uint32_t x = a ^ b;
    uint32_t mix = (5 * x + 0xf87b) ^ 0x4c7e;

    uint32_t v1 = mix ^ (0xf87b * v0
                ^ (uint32_t)(context >> 16));

    uint32_t v2 = v1 ^ (0x4c7e + b + v0 + (a >> 3));

    char prefix[33];
    memset(prefix, 'A', 32);
    prefix[32] = '\0';

    int n = snprintf(out, cap,
                     "%sOPEN %" PRIu32 " %" PRIu32 " %" PRIu32,
                     prefix, v0, v1, v2);

    if (n < 0 || (size_t)n >= cap)
        return 1;

    return 0;
}
