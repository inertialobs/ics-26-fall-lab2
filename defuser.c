#include "defuser.h"

#include <inttypes.h>
#include <stdio.h>

#define MAX_FIELDS 8

/*
 * 下面是本关卡固定数据结构的转写（从个人 bomb 的 .rodata 解析而来）：
 * struct { uint32_t value; uint32_t tag; struct node *next; } 共 16 字节，
 * next 用 p5_nodes 的下标表示，-1 表示 NULL。
 */
struct p5_node {
    uint32_t value;
    uint32_t tag;
    int next;
};

const struct p5_node p5_nodes[12] = {
    {1053,  309,  9},
    {5493,  1806, 3},
    {63244, 2608, 7},
    {50565, 2171, 2},
    {12537, 2433, 5},
    {46798, 56,   6},
    {20059, 1400, 0},
    {56845, 1122, 8},
    {40359, 849,  4},
    {19189, 2955, 10},
    {15986, 527,  -1},
    {24438, 1625, 1},
};

const int p5_heads[8] = {11, 1, 3, 2, 7, 8, 4, 5};

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
    size_t count = 0;
    (void)challenge;
    uint32_t low  = (uint32_t)challenge;
    uint32_t high = (uint32_t)(challenge >> 32);
    uint32_t mid  = (uint32_t)(challenge >> 16);

    uint32_t s1 = (mid ^ 0x182c) + high * 4;
    uint32_t s0 = (low + high * 8) ^ 0x1e2;
    uint32_t ans = (3 * s0 + s1) ^ 0xc08; 

    values[0] = ans;
    count = 1;
    // TODO: 根据 phase_01 的汇编，用 challenge 算出口令中的各个整数，依次存入 values，并把 count 改为整数个数。
    return write_answer(out, cap, values, count);
}

int solve_phase_2(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 0;
    (void)challenge;
    // TODO: 根据 phase_02 的汇编，用 challenge 算出口令中的各个整数，依次存入 values，并把 count 改为整数个数。
    values[0] = 3 * ((challenge >> 15) & 0x1f);

    for (int i = 1; i < 6; i++) {
        int shift = 28 + 5 * (i - 1);      // 28, 33, 38, 43, 48
        int nibble = (challenge >> shift) & 0xf; // 取 4 位
        values[i] = 3 * ((nibble + i) ^ values[i - 1]);
    }
    count = 6;
    return write_answer(out, cap, values, count);
}

int solve_phase_3(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 0;
    (void)challenge;

    uint32_t idx = challenge & 7;
    uint32_t x = (uint32_t)(challenge >> 32) ^ (uint32_t)(challenge >> 3);
    uint32_t ans;
    
    switch (idx) {
        case 0: ans=(2 * x) ^ 0x4099; break;
        case 1: ans=(x ^ 0x84b0) + (x >> 2); break;
        case 2: ans=5 * x + 0x48a4e; break;
        case 3: ans=(4 * x) ^ 0x6c23; break;
        case 4: ans=5 * x + 0x8de1; break;
        case 5: ans=5 * x + 0x49368; break;
        case 6: ans=3 * x + 0x15c3; break;
        case 7: ans=(x ^ 0xa012) + (x >> 1); break;
        default: return 0;
    }
    values [0] = idx;
    1 [values] = ans;
    count = 2;
    // TODO: 根据 phase_03 的汇编，用 challenge 算出口令中的各个整数，依次存入 values，并把 count 改为整数个数。
    return write_answer(out, cap, values, count);
}

uint32_t p4_walk(uint32_t target, uint32_t low, uint32_t high, uint32_t salt){

    uint32_t mid = low + ((high - low) >> 1);
    if (target == mid) {
        return (mid + 0x42) ^ salt;
    }

    uint32_t ebx = mid + salt;

    if (target > mid) {
        uint32_t res = p4_walk(target, mid + 1, high, salt);
        return ((res * 5) ^ ebx) ^ 0x11;
    } else {
        uint32_t res = p4_walk(target, low, mid - 1, salt);
        return ((res * 5) ^ ebx) ^ 0xb;
    }
}

int solve_phase_4(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 0;
    (void)challenge;

    uint32_t target = (uint32_t)challenge & 0x7f; 
    uint32_t low=0, high=0x7f;
    uint32_t salt   = (uint32_t)((challenge >> 7) ^ (challenge >> 32));
    
    values[0] =p4_walk(target, low, high, salt);
    count = 1;
    // TODO: 根据 phase_04 的汇编，用 challenge 算出口令中的各个整数，依次存入 values，并把 count 改为整数个数。
    return write_answer(out, cap, values, count);
}

int solve_phase_5(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};

    int idx = (int)(challenge & 7);
    const struct p5_node *node = &p5_nodes[p5_heads[idx]];

    values[0] = (uint32_t)(challenge >> 6) ^ (uint32_t)(challenge >> 32);
    values[0] ^= 0x2e3d;
    values[0] = values[0] * 3;
    values[0] += node->value;

    values[1] = node->tag;
    values[0] ^= node->tag;

    uint32_t extra = (uint32_t)((challenge >> 3) & 3);
    for (uint32_t i = 0; i < extra; i++) {
        node = &p5_nodes[node->next];
        values[0] = values[0] * 3 + node->value;
        values[1] = node->tag;
        values[0] ^= node->tag;
    }
    return write_answer(out, cap, values, 2);
}

static uint32_t rol(uint32_t x, uint32_t n){
    return (x<<n)|(x>>(32-n));
}

static uint32_t ror(uint32_t x, uint32_t n){
    return (x>>n)|(x<<(32-n));
}

uint32_t ops(uint32_t sw, uint32_t x){
    uint32_t a;
    switch (sw)
    {
    case 0:
        a = (x + 0x28b0) ^ (x>>1);
        break;
    case 1:
        a = rol(x,1) ^ 0xb0a9;
        break;
    case 2:
        a = (x ^ 0xf4c6) + (x<<7);
        break;
    case 3:
        a = ((x>>7) ^ x) + 0x189c;
        break;
    case 4:
    case 5:
        a = (0x8c28 + 2*x) ^ (7*x);
        break;
    case 6:
    case 7:
        a = ror(x,5);
        break;
    default:
        a = 0;
        break;
    }
    return a;
}
int solve_phase_6(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 0;
    (void)challenge;
    uint32_t a = (uint32_t)((challenge >> 6) ^ (challenge >> 32));
    uint32_t r = ops(challenge & 7 , a);
    r = ops((challenge >> 3) & 7 , r ^ 0x82cc);
    
    values[0]= r + 0x2174;
    count = 1;
    // TODO: 根据 phase_06 的汇编，用 challenge 算出口令中的各个整数，依次存入 values，并把 count 改为整数个数。
    return write_answer(out, cap, values, count);
}

int solve_secret(uint64_t challenge, uint64_t context, char *out, size_t cap) {
    static const char prefix[] = "0123456789abcdef0123456789abcdef";

    uint32_t xc = (uint32_t)(challenge >> 32) ^ (uint32_t)challenge;
    uint32_t xk = (uint32_t)(context >> 32) ^ (uint32_t)context;
    uint32_t v0 = (uint32_t)context & 3;

    uint32_t mix = xc ^ xk;
    mix = (mix << 1) | (mix >> 31);      /* rol(x,1) */
    mix = (mix + 0xb3ac) ^ 0x6df5;       /* secret_mix */

    uint32_t r9 = (v0 * 0xb3ac) ^ (uint32_t)(context >> 16);
    uint32_t v1 = mix ^ r9;
    uint32_t v2 = v1 ^ (xk + v0 + 0x6df5 + (xc >> 1));

    int size = snprintf(out, cap, "%sOPEN %" PRIu32 " %" PRIu32 " %" PRIu32,
                        prefix, v0, v1, v2);
    return size < 0 || (size_t)size >= cap;
}
