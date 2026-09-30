#include <nn/font/font_WordWrapping.h>

#include <cstdlib>
#include <cstring>
#include <nn/util.h>

namespace nn {
namespace font {

namespace {

enum LineBreakClass {
    LineBreakClass_OP,
    LineBreakClass_CL,
    LineBreakClass_CP,
    LineBreakClass_QU,
    LineBreakClass_GL,
    LineBreakClass_NS,
    LineBreakClass_EX,
    LineBreakClass_SY,
    LineBreakClass_IS,
    LineBreakClass_PR,
    LineBreakClass_PO,
    LineBreakClass_NU,
    LineBreakClass_AL,
    LineBreakClass_HL,
    LineBreakClass_ID,
    LineBreakClass_IN,
    LineBreakClass_HY,
    LineBreakClass_BA,
    LineBreakClass_BB,
    LineBreakClass_B2,
    LineBreakClass_ZW,
    LineBreakClass_CM,
    LineBreakClass_WJ,
    LineBreakClass_H2,
    LineBreakClass_H3,
    LineBreakClass_JL,
    LineBreakClass_JV,
    LineBreakClass_JT,
    LineBreakClass_RI,
    LineBreakClass_CY,
    LineBreakClass_PairCount,
    LineBreakClass_AI = LineBreakClass_PairCount,
    LineBreakClass_BK,
    LineBreakClass_CB,
    LineBreakClass_CJ,
    LineBreakClass_CR,
    LineBreakClass_LF,
    LineBreakClass_NL,
    LineBreakClass_SA,
    LineBreakClass_SG,
    LineBreakClass_SP,
};

enum BreakAction {
    DIR,
    IND,
    CIB,
    CPB,
    PRO,
};

struct LineBreakRange {
    uint16_t first;
    uint8_t length;
    uint8_t lineBreakClass;
};

const LineBreakRange LineBreakRanges[] = {
    {0x0000, 8, LineBreakClass_CM},   {0x0009, 0, LineBreakClass_BA},
    {0x000a, 0, LineBreakClass_LF},   {0x000b, 1, LineBreakClass_BK},
    {0x000d, 0, LineBreakClass_CR},   {0x000e, 17, LineBreakClass_CM},
    {0x0020, 0, LineBreakClass_SP},   {0x0021, 0, LineBreakClass_EX},
    {0x0022, 0, LineBreakClass_QU},   {0x0023, 0, LineBreakClass_AL},
    {0x0024, 0, LineBreakClass_PR},   {0x0025, 0, LineBreakClass_PO},
    {0x0026, 0, LineBreakClass_AL},   {0x0027, 0, LineBreakClass_QU},
    {0x0028, 0, LineBreakClass_OP},   {0x0029, 0, LineBreakClass_CP},
    {0x002a, 0, LineBreakClass_AL},   {0x002b, 0, LineBreakClass_PR},
    {0x002c, 0, LineBreakClass_IS},   {0x002d, 0, LineBreakClass_HY},
    {0x002e, 0, LineBreakClass_IS},   {0x002f, 0, LineBreakClass_SY},
    {0x0030, 9, LineBreakClass_NU},   {0x003a, 1, LineBreakClass_IS},
    {0x003c, 2, LineBreakClass_AL},   {0x003f, 0, LineBreakClass_EX},
    {0x0040, 26, LineBreakClass_AL},  {0x005b, 0, LineBreakClass_OP},
    {0x005c, 0, LineBreakClass_PR},   {0x005d, 0, LineBreakClass_CP},
    {0x005e, 28, LineBreakClass_AL},  {0x007b, 0, LineBreakClass_OP},
    {0x007c, 0, LineBreakClass_BA},   {0x007d, 0, LineBreakClass_CL},
    {0x007e, 0, LineBreakClass_AL},   {0x007f, 5, LineBreakClass_CM},
    {0x0085, 0, LineBreakClass_NL},   {0x0086, 25, LineBreakClass_CM},
    {0x00a0, 0, LineBreakClass_GL},   {0x00a1, 0, LineBreakClass_OP},
    {0x00a2, 0, LineBreakClass_PO},   {0x00a3, 2, LineBreakClass_PR},
    {0x00a6, 0, LineBreakClass_AL},   {0x00a7, 1, LineBreakClass_AI},
    {0x00a9, 0, LineBreakClass_AL},   {0x00aa, 0, LineBreakClass_AI},
    {0x00ab, 0, LineBreakClass_QU},   {0x00ac, 0, LineBreakClass_AL},
    {0x00ad, 0, LineBreakClass_BA},   {0x00ae, 1, LineBreakClass_AL},
    {0x00b0, 0, LineBreakClass_PO},   {0x00b1, 0, LineBreakClass_PR},
    {0x00b2, 1, LineBreakClass_AI},   {0x00b4, 0, LineBreakClass_BB},
    {0x00b5, 0, LineBreakClass_AL},   {0x00b6, 4, LineBreakClass_AI},
    {0x00bb, 0, LineBreakClass_QU},   {0x00bc, 2, LineBreakClass_AI},
    {0x00bf, 0, LineBreakClass_OP},   {0x00c0, 22, LineBreakClass_AL},
    {0x00d7, 0, LineBreakClass_AI},   {0x00d8, 30, LineBreakClass_AL},
    {0x00f7, 0, LineBreakClass_AI},   {0x00f8, 255, LineBreakClass_AL},
    {0x01f8, 206, LineBreakClass_AL}, {0x02c7, 0, LineBreakClass_AI},
    {0x02c8, 0, LineBreakClass_BB},   {0x02c9, 2, LineBreakClass_AI},
    {0x02cc, 0, LineBreakClass_BB},   {0x02cd, 0, LineBreakClass_AI},
    {0x02ce, 1, LineBreakClass_AL},   {0x02d0, 0, LineBreakClass_AI},
    {0x02d1, 6, LineBreakClass_AL},   {0x02d8, 3, LineBreakClass_AI},
    {0x02dc, 0, LineBreakClass_AL},   {0x02dd, 0, LineBreakClass_AI},
    {0x02de, 0, LineBreakClass_AL},   {0x02df, 0, LineBreakClass_BB},
    {0x02e0, 31, LineBreakClass_AL},  {0x0300, 78, LineBreakClass_CM},
    {0x034f, 0, LineBreakClass_GL},   {0x0350, 11, LineBreakClass_CM},
    {0x035c, 6, LineBreakClass_GL},   {0x0363, 12, LineBreakClass_CM},
    {0x0370, 7, LineBreakClass_AL},   {0x037a, 3, LineBreakClass_AL},
    {0x037e, 0, LineBreakClass_IS},   {0x0384, 6, LineBreakClass_AL},
    {0x038c, 0, LineBreakClass_AL},   {0x038e, 19, LineBreakClass_AL},
    {0x03a3, 92, LineBreakClass_AL},  {0x0400, 255, LineBreakClass_CY},
    {0x0500, 47, LineBreakClass_CY},  {0x0531, 37, LineBreakClass_AL},
    {0x0559, 6, LineBreakClass_AL},   {0x0561, 38, LineBreakClass_AL},
    {0x0589, 0, LineBreakClass_IS},   {0x058a, 0, LineBreakClass_BA},
    {0x058f, 0, LineBreakClass_PR},   {0x0591, 44, LineBreakClass_CM},
    {0x05be, 0, LineBreakClass_BA},   {0x05bf, 0, LineBreakClass_CM},
    {0x05c0, 0, LineBreakClass_AL},   {0x05c1, 1, LineBreakClass_CM},
    {0x05c3, 0, LineBreakClass_AL},   {0x05c4, 1, LineBreakClass_CM},
    {0x05c6, 0, LineBreakClass_EX},   {0x05c7, 0, LineBreakClass_CM},
    {0x05d0, 26, LineBreakClass_HL},  {0x05f0, 2, LineBreakClass_HL},
    {0x05f3, 1, LineBreakClass_AL},   {0x0600, 4, LineBreakClass_AL},
    {0x0606, 2, LineBreakClass_AL},   {0x0609, 2, LineBreakClass_PO},
    {0x060c, 1, LineBreakClass_IS},   {0x060e, 1, LineBreakClass_AL},
    {0x0610, 10, LineBreakClass_CM},  {0x061b, 0, LineBreakClass_EX},
    {0x061e, 1, LineBreakClass_EX},   {0x0620, 42, LineBreakClass_AL},
    {0x064b, 20, LineBreakClass_CM},  {0x0660, 9, LineBreakClass_NU},
    {0x066a, 0, LineBreakClass_PO},   {0x066b, 1, LineBreakClass_NU},
    {0x066d, 2, LineBreakClass_AL},   {0x0670, 0, LineBreakClass_CM},
    {0x0671, 98, LineBreakClass_AL},  {0x06d4, 0, LineBreakClass_EX},
    {0x06d5, 0, LineBreakClass_AL},   {0x06d6, 6, LineBreakClass_CM},
    {0x06dd, 1, LineBreakClass_AL},   {0x06df, 5, LineBreakClass_CM},
    {0x06e5, 1, LineBreakClass_AL},   {0x06e7, 1, LineBreakClass_CM},
    {0x06e9, 0, LineBreakClass_AL},   {0x06ea, 3, LineBreakClass_CM},
    {0x06ee, 1, LineBreakClass_AL},   {0x06f0, 9, LineBreakClass_NU},
    {0x06fa, 19, LineBreakClass_AL},  {0x070f, 1, LineBreakClass_AL},
    {0x0711, 0, LineBreakClass_CM},   {0x0712, 29, LineBreakClass_AL},
    {0x0730, 26, LineBreakClass_CM},  {0x074d, 88, LineBreakClass_AL},
    {0x07a6, 10, LineBreakClass_CM},  {0x07b1, 0, LineBreakClass_AL},
    {0x07c0, 9, LineBreakClass_NU},   {0x07ca, 32, LineBreakClass_AL},
    {0x07eb, 8, LineBreakClass_CM},   {0x07f4, 3, LineBreakClass_AL},
    {0x07f8, 0, LineBreakClass_IS},   {0x07f9, 0, LineBreakClass_EX},
    {0x07fa, 0, LineBreakClass_AL},   {0x0800, 21, LineBreakClass_AL},
    {0x0816, 3, LineBreakClass_CM},   {0x081a, 0, LineBreakClass_AL},
    {0x081b, 8, LineBreakClass_CM},   {0x0824, 0, LineBreakClass_AL},
    {0x0825, 2, LineBreakClass_CM},   {0x0828, 0, LineBreakClass_AL},
    {0x0829, 4, LineBreakClass_CM},   {0x0830, 14, LineBreakClass_AL},
    {0x0840, 24, LineBreakClass_AL},  {0x0859, 2, LineBreakClass_CM},
    {0x085e, 0, LineBreakClass_AL},   {0x08a0, 0, LineBreakClass_AL},
    {0x08a2, 10, LineBreakClass_AL},  {0x08e4, 26, LineBreakClass_CM},
    {0x0900, 3, LineBreakClass_CM},   {0x0904, 53, LineBreakClass_AL},
    {0x093a, 2, LineBreakClass_CM},   {0x093d, 0, LineBreakClass_AL},
    {0x093e, 17, LineBreakClass_CM},  {0x0950, 0, LineBreakClass_AL},
    {0x0951, 6, LineBreakClass_CM},   {0x0958, 9, LineBreakClass_AL},
    {0x0962, 1, LineBreakClass_CM},   {0x0964, 1, LineBreakClass_BA},
    {0x0966, 9, LineBreakClass_NU},   {0x0970, 7, LineBreakClass_AL},
    {0x0979, 6, LineBreakClass_AL},   {0x0981, 2, LineBreakClass_CM},
    {0x0985, 7, LineBreakClass_AL},   {0x098f, 1, LineBreakClass_AL},
    {0x0993, 21, LineBreakClass_AL},  {0x09aa, 6, LineBreakClass_AL},
    {0x09b2, 0, LineBreakClass_AL},   {0x09b6, 3, LineBreakClass_AL},
    {0x09bc, 0, LineBreakClass_CM},   {0x09bd, 0, LineBreakClass_AL},
    {0x09be, 6, LineBreakClass_CM},   {0x09c7, 1, LineBreakClass_CM},
    {0x09cb, 2, LineBreakClass_CM},   {0x09ce, 0, LineBreakClass_AL},
    {0x09d7, 0, LineBreakClass_CM},   {0x09dc, 1, LineBreakClass_AL},
    {0x09df, 2, LineBreakClass_AL},   {0x09e2, 1, LineBreakClass_CM},
    {0x09e6, 9, LineBreakClass_NU},   {0x09f0, 1, LineBreakClass_AL},
    {0x09f2, 1, LineBreakClass_PO},   {0x09f4, 4, LineBreakClass_AL},
    {0x09f9, 0, LineBreakClass_PO},   {0x09fa, 0, LineBreakClass_AL},
    {0x09fb, 0, LineBreakClass_PR},   {0x0a01, 2, LineBreakClass_CM},
    {0x0a05, 5, LineBreakClass_AL},   {0x0a0f, 1, LineBreakClass_AL},
    {0x0a13, 21, LineBreakClass_AL},  {0x0a2a, 6, LineBreakClass_AL},
    {0x0a32, 1, LineBreakClass_AL},   {0x0a35, 1, LineBreakClass_AL},
    {0x0a38, 1, LineBreakClass_AL},   {0x0a3c, 0, LineBreakClass_CM},
    {0x0a3e, 4, LineBreakClass_CM},   {0x0a47, 1, LineBreakClass_CM},
    {0x0a4b, 2, LineBreakClass_CM},   {0x0a51, 0, LineBreakClass_CM},
    {0x0a59, 3, LineBreakClass_AL},   {0x0a5e, 0, LineBreakClass_AL},
    {0x0a66, 9, LineBreakClass_NU},   {0x0a70, 1, LineBreakClass_CM},
    {0x0a72, 2, LineBreakClass_AL},   {0x0a75, 0, LineBreakClass_CM},
    {0x0a81, 2, LineBreakClass_CM},   {0x0a85, 8, LineBreakClass_AL},
    {0x0a8f, 2, LineBreakClass_AL},   {0x0a93, 21, LineBreakClass_AL},
    {0x0aaa, 6, LineBreakClass_AL},   {0x0ab2, 1, LineBreakClass_AL},
    {0x0ab5, 4, LineBreakClass_AL},   {0x0abc, 0, LineBreakClass_CM},
    {0x0abd, 0, LineBreakClass_AL},   {0x0abe, 7, LineBreakClass_CM},
    {0x0ac7, 2, LineBreakClass_CM},   {0x0acb, 2, LineBreakClass_CM},
    {0x0ad0, 0, LineBreakClass_AL},   {0x0ae0, 1, LineBreakClass_AL},
    {0x0ae2, 1, LineBreakClass_CM},   {0x0ae6, 9, LineBreakClass_NU},
    {0x0af0, 0, LineBreakClass_AL},   {0x0af1, 0, LineBreakClass_PR},
    {0x0b01, 2, LineBreakClass_CM},   {0x0b05, 7, LineBreakClass_AL},
    {0x0b0f, 1, LineBreakClass_AL},   {0x0b13, 21, LineBreakClass_AL},
    {0x0b2a, 6, LineBreakClass_AL},   {0x0b32, 1, LineBreakClass_AL},
    {0x0b35, 4, LineBreakClass_AL},   {0x0b3c, 0, LineBreakClass_CM},
    {0x0b3d, 0, LineBreakClass_AL},   {0x0b3e, 6, LineBreakClass_CM},
    {0x0b47, 1, LineBreakClass_CM},   {0x0b4b, 2, LineBreakClass_CM},
    {0x0b56, 1, LineBreakClass_CM},   {0x0b5c, 1, LineBreakClass_AL},
    {0x0b5f, 2, LineBreakClass_AL},   {0x0b62, 1, LineBreakClass_CM},
    {0x0b66, 9, LineBreakClass_NU},   {0x0b70, 7, LineBreakClass_AL},
    {0x0b82, 0, LineBreakClass_CM},   {0x0b83, 0, LineBreakClass_AL},
    {0x0b85, 5, LineBreakClass_AL},   {0x0b8e, 2, LineBreakClass_AL},
    {0x0b92, 3, LineBreakClass_AL},   {0x0b99, 1, LineBreakClass_AL},
    {0x0b9c, 0, LineBreakClass_AL},   {0x0b9e, 1, LineBreakClass_AL},
    {0x0ba3, 1, LineBreakClass_AL},   {0x0ba8, 2, LineBreakClass_AL},
    {0x0bae, 11, LineBreakClass_AL},  {0x0bbe, 4, LineBreakClass_CM},
    {0x0bc6, 2, LineBreakClass_CM},   {0x0bca, 3, LineBreakClass_CM},
    {0x0bd0, 0, LineBreakClass_AL},   {0x0bd7, 0, LineBreakClass_CM},
    {0x0be6, 9, LineBreakClass_NU},   {0x0bf0, 8, LineBreakClass_AL},
    {0x0bf9, 0, LineBreakClass_PR},   {0x0bfa, 0, LineBreakClass_AL},
    {0x0c01, 2, LineBreakClass_CM},   {0x0c05, 7, LineBreakClass_AL},
    {0x0c0e, 2, LineBreakClass_AL},   {0x0c12, 22, LineBreakClass_AL},
    {0x0c2a, 9, LineBreakClass_AL},   {0x0c35, 4, LineBreakClass_AL},
    {0x0c3d, 0, LineBreakClass_AL},   {0x0c3e, 6, LineBreakClass_CM},
    {0x0c46, 2, LineBreakClass_CM},   {0x0c4a, 3, LineBreakClass_CM},
    {0x0c55, 1, LineBreakClass_CM},   {0x0c58, 1, LineBreakClass_AL},
    {0x0c60, 1, LineBreakClass_AL},   {0x0c62, 1, LineBreakClass_CM},
    {0x0c66, 9, LineBreakClass_NU},   {0x0c78, 7, LineBreakClass_AL},
    {0x0c82, 1, LineBreakClass_CM},   {0x0c85, 7, LineBreakClass_AL},
    {0x0c8e, 2, LineBreakClass_AL},   {0x0c92, 22, LineBreakClass_AL},
    {0x0caa, 9, LineBreakClass_AL},   {0x0cb5, 4, LineBreakClass_AL},
    {0x0cbc, 0, LineBreakClass_CM},   {0x0cbd, 0, LineBreakClass_AL},
    {0x0cbe, 6, LineBreakClass_CM},   {0x0cc6, 2, LineBreakClass_CM},
    {0x0cca, 3, LineBreakClass_CM},   {0x0cd5, 1, LineBreakClass_CM},
    {0x0cde, 0, LineBreakClass_AL},   {0x0ce0, 1, LineBreakClass_AL},
    {0x0ce2, 1, LineBreakClass_CM},   {0x0ce6, 9, LineBreakClass_NU},
    {0x0cf1, 1, LineBreakClass_AL},   {0x0d02, 1, LineBreakClass_CM},
    {0x0d05, 7, LineBreakClass_AL},   {0x0d0e, 2, LineBreakClass_AL},
    {0x0d12, 40, LineBreakClass_AL},  {0x0d3d, 0, LineBreakClass_AL},
    {0x0d3e, 6, LineBreakClass_CM},   {0x0d46, 2, LineBreakClass_CM},
    {0x0d4a, 3, LineBreakClass_CM},   {0x0d4e, 0, LineBreakClass_AL},
    {0x0d57, 0, LineBreakClass_CM},   {0x0d60, 1, LineBreakClass_AL},
    {0x0d62, 1, LineBreakClass_CM},   {0x0d66, 9, LineBreakClass_NU},
    {0x0d70, 5, LineBreakClass_AL},   {0x0d79, 0, LineBreakClass_PO},
    {0x0d7a, 5, LineBreakClass_AL},   {0x0d82, 1, LineBreakClass_CM},
    {0x0d85, 17, LineBreakClass_AL},  {0x0d9a, 23, LineBreakClass_AL},
    {0x0db3, 8, LineBreakClass_AL},   {0x0dbd, 0, LineBreakClass_AL},
    {0x0dc0, 6, LineBreakClass_AL},   {0x0dca, 0, LineBreakClass_CM},
    {0x0dcf, 5, LineBreakClass_CM},   {0x0dd6, 0, LineBreakClass_CM},
    {0x0dd8, 7, LineBreakClass_CM},   {0x0df2, 1, LineBreakClass_CM},
    {0x0df4, 0, LineBreakClass_AL},   {0x0e01, 57, LineBreakClass_SA},
    {0x0e3f, 0, LineBreakClass_PR},   {0x0e40, 14, LineBreakClass_SA},
    {0x0e4f, 0, LineBreakClass_AL},   {0x0e50, 9, LineBreakClass_NU},
    {0x0e5a, 1, LineBreakClass_BA},   {0x0e81, 1, LineBreakClass_SA},
    {0x0e84, 0, LineBreakClass_SA},   {0x0e87, 1, LineBreakClass_SA},
    {0x0e8a, 0, LineBreakClass_SA},   {0x0e8d, 0, LineBreakClass_SA},
    {0x0e94, 3, LineBreakClass_SA},   {0x0e99, 6, LineBreakClass_SA},
    {0x0ea1, 2, LineBreakClass_SA},   {0x0ea5, 0, LineBreakClass_SA},
    {0x0ea7, 0, LineBreakClass_SA},   {0x0eaa, 1, LineBreakClass_SA},
    {0x0ead, 12, LineBreakClass_SA},  {0x0ebb, 2, LineBreakClass_SA},
    {0x0ec0, 4, LineBreakClass_SA},   {0x0ec6, 0, LineBreakClass_SA},
    {0x0ec8, 5, LineBreakClass_SA},   {0x0ed0, 9, LineBreakClass_NU},
    {0x0edc, 3, LineBreakClass_SA},   {0x0f00, 0, LineBreakClass_AL},
    {0x0f01, 3, LineBreakClass_BB},   {0x0f05, 0, LineBreakClass_AL},
    {0x0f06, 1, LineBreakClass_BB},   {0x0f08, 0, LineBreakClass_GL},
    {0x0f09, 1, LineBreakClass_BB},   {0x0f0b, 0, LineBreakClass_BA},
    {0x0f0c, 0, LineBreakClass_GL},   {0x0f0d, 4, LineBreakClass_EX},
    {0x0f12, 0, LineBreakClass_GL},   {0x0f13, 0, LineBreakClass_AL},
    {0x0f14, 0, LineBreakClass_EX},   {0x0f15, 2, LineBreakClass_AL},
    {0x0f18, 1, LineBreakClass_CM},   {0x0f1a, 5, LineBreakClass_AL},
    {0x0f20, 9, LineBreakClass_NU},   {0x0f2a, 9, LineBreakClass_AL},
    {0x0f34, 0, LineBreakClass_BA},   {0x0f35, 0, LineBreakClass_CM},
    {0x0f36, 0, LineBreakClass_AL},   {0x0f37, 0, LineBreakClass_CM},
    {0x0f38, 0, LineBreakClass_AL},   {0x0f39, 0, LineBreakClass_CM},
    {0x0f3a, 0, LineBreakClass_OP},   {0x0f3b, 0, LineBreakClass_CL},
    {0x0f3c, 0, LineBreakClass_OP},   {0x0f3d, 0, LineBreakClass_CL},
    {0x0f3e, 1, LineBreakClass_CM},   {0x0f40, 7, LineBreakClass_AL},
    {0x0f49, 35, LineBreakClass_AL},  {0x0f71, 13, LineBreakClass_CM},
    {0x0f7f, 0, LineBreakClass_BA},   {0x0f80, 4, LineBreakClass_CM},
    {0x0f85, 0, LineBreakClass_BA},   {0x0f86, 1, LineBreakClass_CM},
    {0x0f88, 4, LineBreakClass_AL},   {0x0f8d, 10, LineBreakClass_CM},
    {0x0f99, 35, LineBreakClass_CM},  {0x0fbe, 1, LineBreakClass_BA},
    {0x0fc0, 5, LineBreakClass_AL},   {0x0fc6, 0, LineBreakClass_CM},
    {0x0fc7, 5, LineBreakClass_AL},   {0x0fce, 1, LineBreakClass_AL},
    {0x0fd0, 1, LineBreakClass_BB},   {0x0fd2, 0, LineBreakClass_BA},
    {0x0fd3, 0, LineBreakClass_BB},   {0x0fd4, 4, LineBreakClass_AL},
    {0x0fd9, 1, LineBreakClass_GL},   {0x1000, 63, LineBreakClass_SA},
    {0x1040, 9, LineBreakClass_NU},   {0x104a, 1, LineBreakClass_BA},
    {0x104c, 3, LineBreakClass_AL},   {0x1050, 63, LineBreakClass_SA},
    {0x1090, 9, LineBreakClass_NU},   {0x109a, 5, LineBreakClass_SA},
    {0x10a0, 37, LineBreakClass_AL},  {0x10c7, 0, LineBreakClass_AL},
    {0x10cd, 0, LineBreakClass_AL},   {0x10d0, 47, LineBreakClass_AL},
    {0x1100, 95, LineBreakClass_JL},  {0x1160, 71, LineBreakClass_JV},
    {0x11a8, 87, LineBreakClass_JT},  {0x1200, 72, LineBreakClass_AL},
    {0x124a, 3, LineBreakClass_AL},   {0x1250, 6, LineBreakClass_AL},
    {0x1258, 0, LineBreakClass_AL},   {0x125a, 3, LineBreakClass_AL},
    {0x1260, 40, LineBreakClass_AL},  {0x128a, 3, LineBreakClass_AL},
    {0x1290, 32, LineBreakClass_AL},  {0x12b2, 3, LineBreakClass_AL},
    {0x12b8, 6, LineBreakClass_AL},   {0x12c0, 0, LineBreakClass_AL},
    {0x12c2, 3, LineBreakClass_AL},   {0x12c8, 14, LineBreakClass_AL},
    {0x12d8, 56, LineBreakClass_AL},  {0x1312, 3, LineBreakClass_AL},
    {0x1318, 66, LineBreakClass_AL},  {0x135d, 2, LineBreakClass_CM},
    {0x1360, 0, LineBreakClass_AL},   {0x1361, 0, LineBreakClass_BA},
    {0x1362, 26, LineBreakClass_AL},  {0x1380, 25, LineBreakClass_AL},
    {0x13a0, 84, LineBreakClass_AL},  {0x1400, 0, LineBreakClass_BA},
    {0x1401, 255, LineBreakClass_AL}, {0x1501, 255, LineBreakClass_AL},
    {0x1601, 126, LineBreakClass_AL}, {0x1680, 0, LineBreakClass_BA},
    {0x1681, 25, LineBreakClass_AL},  {0x169b, 0, LineBreakClass_OP},
    {0x169c, 0, LineBreakClass_CL},   {0x16a0, 74, LineBreakClass_AL},
    {0x16eb, 2, LineBreakClass_BA},   {0x16ee, 2, LineBreakClass_AL},
    {0x1700, 12, LineBreakClass_AL},  {0x170e, 3, LineBreakClass_AL},
    {0x1712, 2, LineBreakClass_CM},   {0x1720, 17, LineBreakClass_AL},
    {0x1732, 2, LineBreakClass_CM},   {0x1735, 1, LineBreakClass_BA},
    {0x1740, 17, LineBreakClass_AL},  {0x1752, 1, LineBreakClass_CM},
    {0x1760, 12, LineBreakClass_AL},  {0x176e, 2, LineBreakClass_AL},
    {0x1772, 1, LineBreakClass_CM},   {0x1780, 83, LineBreakClass_SA},
    {0x17d4, 1, LineBreakClass_BA},   {0x17d6, 0, LineBreakClass_NS},
    {0x17d7, 0, LineBreakClass_SA},   {0x17d8, 0, LineBreakClass_BA},
    {0x17d9, 0, LineBreakClass_AL},   {0x17da, 0, LineBreakClass_BA},
    {0x17db, 0, LineBreakClass_PR},   {0x17dc, 1, LineBreakClass_SA},
    {0x17e0, 9, LineBreakClass_NU},   {0x17f0, 9, LineBreakClass_AL},
    {0x1800, 1, LineBreakClass_AL},   {0x1802, 1, LineBreakClass_EX},
    {0x1804, 1, LineBreakClass_BA},   {0x1806, 0, LineBreakClass_BB},
    {0x1807, 0, LineBreakClass_AL},   {0x1808, 1, LineBreakClass_EX},
    {0x180a, 0, LineBreakClass_AL},   {0x180b, 2, LineBreakClass_CM},
    {0x180e, 0, LineBreakClass_GL},   {0x1810, 9, LineBreakClass_NU},
    {0x1820, 87, LineBreakClass_AL},  {0x1880, 40, LineBreakClass_AL},
    {0x18a9, 0, LineBreakClass_CM},   {0x18aa, 0, LineBreakClass_AL},
    {0x18b0, 69, LineBreakClass_AL},  {0x1900, 28, LineBreakClass_AL},
    {0x1920, 11, LineBreakClass_CM},  {0x1930, 11, LineBreakClass_CM},
    {0x1940, 0, LineBreakClass_AL},   {0x1944, 1, LineBreakClass_EX},
    {0x1946, 9, LineBreakClass_NU},   {0x1950, 29, LineBreakClass_SA},
    {0x1970, 4, LineBreakClass_SA},   {0x1980, 43, LineBreakClass_SA},
    {0x19b0, 25, LineBreakClass_SA},  {0x19d0, 9, LineBreakClass_NU},
    {0x19da, 0, LineBreakClass_SA},   {0x19de, 1, LineBreakClass_SA},
    {0x19e0, 54, LineBreakClass_AL},  {0x1a17, 4, LineBreakClass_CM},
    {0x1a1e, 1, LineBreakClass_AL},   {0x1a20, 62, LineBreakClass_SA},
    {0x1a60, 28, LineBreakClass_SA},  {0x1a7f, 0, LineBreakClass_CM},
    {0x1a80, 9, LineBreakClass_NU},   {0x1a90, 9, LineBreakClass_NU},
    {0x1aa0, 13, LineBreakClass_SA},  {0x1b00, 4, LineBreakClass_CM},
    {0x1b05, 46, LineBreakClass_AL},  {0x1b34, 16, LineBreakClass_CM},
    {0x1b45, 6, LineBreakClass_AL},   {0x1b50, 9, LineBreakClass_NU},
    {0x1b5a, 1, LineBreakClass_BA},   {0x1b5c, 0, LineBreakClass_AL},
    {0x1b5d, 3, LineBreakClass_BA},   {0x1b61, 9, LineBreakClass_AL},
    {0x1b6b, 8, LineBreakClass_CM},   {0x1b74, 8, LineBreakClass_AL},
    {0x1b80, 2, LineBreakClass_CM},   {0x1b83, 29, LineBreakClass_AL},
    {0x1ba1, 12, LineBreakClass_CM},  {0x1bae, 1, LineBreakClass_AL},
    {0x1bb0, 9, LineBreakClass_NU},   {0x1bba, 43, LineBreakClass_AL},
    {0x1be6, 13, LineBreakClass_CM},  {0x1bfc, 39, LineBreakClass_AL},
    {0x1c24, 19, LineBreakClass_CM},  {0x1c3b, 4, LineBreakClass_BA},
    {0x1c40, 9, LineBreakClass_NU},   {0x1c4d, 2, LineBreakClass_AL},
    {0x1c50, 9, LineBreakClass_NU},   {0x1c5a, 35, LineBreakClass_AL},
    {0x1c7e, 1, LineBreakClass_BA},   {0x1cc0, 7, LineBreakClass_AL},
    {0x1cd0, 2, LineBreakClass_CM},   {0x1cd3, 0, LineBreakClass_AL},
    {0x1cd4, 20, LineBreakClass_CM},  {0x1ce9, 3, LineBreakClass_AL},
    {0x1ced, 0, LineBreakClass_CM},   {0x1cee, 3, LineBreakClass_AL},
    {0x1cf2, 2, LineBreakClass_CM},   {0x1cf5, 1, LineBreakClass_AL},
    {0x1d00, 191, LineBreakClass_AL}, {0x1dc0, 38, LineBreakClass_CM},
    {0x1dfc, 3, LineBreakClass_CM},   {0x1e00, 255, LineBreakClass_AL},
    {0x1f00, 21, LineBreakClass_AL},  {0x1f18, 5, LineBreakClass_AL},
    {0x1f20, 37, LineBreakClass_AL},  {0x1f48, 5, LineBreakClass_AL},
    {0x1f50, 7, LineBreakClass_AL},   {0x1f59, 0, LineBreakClass_AL},
    {0x1f5b, 0, LineBreakClass_AL},   {0x1f5d, 0, LineBreakClass_AL},
    {0x1f5f, 30, LineBreakClass_AL},  {0x1f80, 52, LineBreakClass_AL},
    {0x1fb6, 14, LineBreakClass_AL},  {0x1fc6, 13, LineBreakClass_AL},
    {0x1fd6, 5, LineBreakClass_AL},   {0x1fdd, 18, LineBreakClass_AL},
    {0x1ff2, 2, LineBreakClass_AL},   {0x1ff6, 6, LineBreakClass_AL},
    {0x1ffd, 0, LineBreakClass_BB},   {0x1ffe, 0, LineBreakClass_AL},
    {0x2000, 6, LineBreakClass_BA},   {0x2007, 0, LineBreakClass_GL},
    {0x2008, 2, LineBreakClass_BA},   {0x200b, 0, LineBreakClass_ZW},
    {0x200c, 3, LineBreakClass_CM},   {0x2010, 0, LineBreakClass_BA},
    {0x2011, 0, LineBreakClass_GL},   {0x2012, 1, LineBreakClass_BA},
    {0x2014, 0, LineBreakClass_B2},   {0x2015, 1, LineBreakClass_AI},
    {0x2017, 0, LineBreakClass_AL},   {0x2018, 1, LineBreakClass_QU},
    {0x201a, 0, LineBreakClass_OP},   {0x201b, 2, LineBreakClass_QU},
    {0x201e, 0, LineBreakClass_OP},   {0x201f, 0, LineBreakClass_QU},
    {0x2020, 1, LineBreakClass_AI},   {0x2022, 1, LineBreakClass_AL},
    {0x2024, 2, LineBreakClass_IN},   {0x2027, 0, LineBreakClass_BA},
    {0x2028, 1, LineBreakClass_BK},   {0x202a, 4, LineBreakClass_CM},
    {0x202f, 0, LineBreakClass_GL},   {0x2030, 7, LineBreakClass_PO},
    {0x2038, 0, LineBreakClass_AL},   {0x2039, 1, LineBreakClass_QU},
    {0x203b, 0, LineBreakClass_AI},   {0x203c, 1, LineBreakClass_NS},
    {0x203e, 5, LineBreakClass_AL},   {0x2044, 0, LineBreakClass_IS},
    {0x2045, 0, LineBreakClass_OP},   {0x2046, 0, LineBreakClass_CL},
    {0x2047, 2, LineBreakClass_NS},   {0x204a, 11, LineBreakClass_AL},
    {0x2056, 0, LineBreakClass_BA},   {0x2057, 0, LineBreakClass_AL},
    {0x2058, 3, LineBreakClass_BA},   {0x205c, 0, LineBreakClass_AL},
    {0x205d, 2, LineBreakClass_BA},   {0x2060, 0, LineBreakClass_WJ},
    {0x2061, 3, LineBreakClass_AL},   {0x206a, 5, LineBreakClass_CM},
    {0x2070, 1, LineBreakClass_AL},   {0x2074, 0, LineBreakClass_AI},
    {0x2075, 7, LineBreakClass_AL},   {0x207d, 0, LineBreakClass_OP},
    {0x207e, 0, LineBreakClass_CL},   {0x207f, 0, LineBreakClass_AI},
    {0x2080, 0, LineBreakClass_AL},   {0x2081, 3, LineBreakClass_AI},
    {0x2085, 7, LineBreakClass_AL},   {0x208d, 0, LineBreakClass_OP},
    {0x208e, 0, LineBreakClass_CL},   {0x2090, 12, LineBreakClass_AL},
    {0x20a0, 6, LineBreakClass_PR},   {0x20a7, 0, LineBreakClass_PO},
    {0x20a8, 13, LineBreakClass_PR},  {0x20b6, 0, LineBreakClass_PO},
    {0x20b7, 3, LineBreakClass_PR},   {0x20d0, 32, LineBreakClass_CM},
    {0x2100, 2, LineBreakClass_AL},   {0x2103, 0, LineBreakClass_PO},
    {0x2104, 0, LineBreakClass_AL},   {0x2105, 0, LineBreakClass_AI},
    {0x2106, 2, LineBreakClass_AL},   {0x2109, 0, LineBreakClass_PO},
    {0x210a, 8, LineBreakClass_AL},   {0x2113, 0, LineBreakClass_AI},
    {0x2114, 1, LineBreakClass_AL},   {0x2116, 0, LineBreakClass_PR},
    {0x2117, 9, LineBreakClass_AL},   {0x2121, 1, LineBreakClass_AI},
    {0x2123, 7, LineBreakClass_AL},   {0x212b, 0, LineBreakClass_AI},
    {0x212c, 39, LineBreakClass_AL},  {0x2154, 1, LineBreakClass_AI},
    {0x2156, 4, LineBreakClass_AL},   {0x215b, 0, LineBreakClass_AI},
    {0x215c, 1, LineBreakClass_AL},   {0x215e, 0, LineBreakClass_AI},
    {0x215f, 0, LineBreakClass_AL},   {0x2160, 11, LineBreakClass_AI},
    {0x216c, 3, LineBreakClass_AL},   {0x2170, 9, LineBreakClass_AI},
    {0x217a, 14, LineBreakClass_AL},  {0x2189, 0, LineBreakClass_AI},
    {0x2190, 9, LineBreakClass_AI},   {0x219a, 55, LineBreakClass_AL},
    {0x21d2, 0, LineBreakClass_AI},   {0x21d3, 0, LineBreakClass_AL},
    {0x21d4, 0, LineBreakClass_AI},   {0x21d5, 42, LineBreakClass_AL},
    {0x2200, 0, LineBreakClass_AI},   {0x2201, 0, LineBreakClass_AL},
    {0x2202, 1, LineBreakClass_AI},   {0x2204, 2, LineBreakClass_AL},
    {0x2207, 1, LineBreakClass_AI},   {0x2209, 1, LineBreakClass_AL},
    {0x220b, 0, LineBreakClass_AI},   {0x220c, 2, LineBreakClass_AL},
    {0x220f, 0, LineBreakClass_AI},   {0x2210, 0, LineBreakClass_AL},
    {0x2211, 0, LineBreakClass_AI},   {0x2212, 1, LineBreakClass_PR},
    {0x2214, 0, LineBreakClass_AL},   {0x2215, 0, LineBreakClass_AI},
    {0x2216, 3, LineBreakClass_AL},   {0x221a, 0, LineBreakClass_AI},
    {0x221b, 1, LineBreakClass_AL},   {0x221d, 3, LineBreakClass_AI},
    {0x2221, 1, LineBreakClass_AL},   {0x2223, 0, LineBreakClass_AI},
    {0x2224, 0, LineBreakClass_AL},   {0x2225, 0, LineBreakClass_AI},
    {0x2226, 0, LineBreakClass_AL},   {0x2227, 5, LineBreakClass_AI},
    {0x222d, 0, LineBreakClass_AL},   {0x222e, 0, LineBreakClass_AI},
    {0x222f, 4, LineBreakClass_AL},   {0x2234, 3, LineBreakClass_AI},
    {0x2238, 3, LineBreakClass_AL},   {0x223c, 1, LineBreakClass_AI},
    {0x223e, 9, LineBreakClass_AL},   {0x2248, 0, LineBreakClass_AI},
    {0x2249, 2, LineBreakClass_AL},   {0x224c, 0, LineBreakClass_AI},
    {0x224d, 4, LineBreakClass_AL},   {0x2252, 0, LineBreakClass_AI},
    {0x2253, 12, LineBreakClass_AL},  {0x2260, 1, LineBreakClass_AI},
    {0x2262, 1, LineBreakClass_AL},   {0x2264, 3, LineBreakClass_AI},
    {0x2268, 1, LineBreakClass_AL},   {0x226a, 1, LineBreakClass_AI},
    {0x226c, 1, LineBreakClass_AL},   {0x226e, 1, LineBreakClass_AI},
    {0x2270, 17, LineBreakClass_AL},  {0x2282, 1, LineBreakClass_AI},
    {0x2284, 1, LineBreakClass_AL},   {0x2286, 1, LineBreakClass_AI},
    {0x2288, 12, LineBreakClass_AL},  {0x2295, 0, LineBreakClass_AI},
    {0x2296, 2, LineBreakClass_AL},   {0x2299, 0, LineBreakClass_AI},
    {0x229a, 10, LineBreakClass_AL},  {0x22a5, 0, LineBreakClass_AI},
    {0x22a6, 24, LineBreakClass_AL},  {0x22bf, 0, LineBreakClass_AI},
    {0x22c0, 81, LineBreakClass_AL},  {0x2312, 0, LineBreakClass_AI},
    {0x2313, 6, LineBreakClass_AL},   {0x231c, 12, LineBreakClass_AL},
    {0x2329, 0, LineBreakClass_OP},   {0x232a, 0, LineBreakClass_CL},
    {0x232b, 196, LineBreakClass_AL}, {0x2400, 38, LineBreakClass_AL},
    {0x2440, 10, LineBreakClass_AL},  {0x2460, 158, LineBreakClass_AI},
    {0x24ff, 0, LineBreakClass_AL},   {0x2500, 75, LineBreakClass_AI},
    {0x254c, 3, LineBreakClass_AL},   {0x2550, 36, LineBreakClass_AI},
    {0x2575, 10, LineBreakClass_AL},  {0x2580, 15, LineBreakClass_AI},
    {0x2590, 1, LineBreakClass_AL},   {0x2592, 3, LineBreakClass_AI},
    {0x2596, 9, LineBreakClass_AL},   {0x25a0, 1, LineBreakClass_AI},
    {0x25a2, 0, LineBreakClass_AL},   {0x25a3, 6, LineBreakClass_AI},
    {0x25aa, 7, LineBreakClass_AL},   {0x25b2, 1, LineBreakClass_AI},
    {0x25b4, 1, LineBreakClass_AL},   {0x25b6, 1, LineBreakClass_AI},
    {0x25b8, 3, LineBreakClass_AL},   {0x25bc, 1, LineBreakClass_AI},
    {0x25be, 1, LineBreakClass_AL},   {0x25c0, 1, LineBreakClass_AI},
    {0x25c2, 3, LineBreakClass_AL},   {0x25c6, 2, LineBreakClass_AI},
    {0x25c9, 1, LineBreakClass_AL},   {0x25cb, 0, LineBreakClass_AI},
    {0x25cc, 1, LineBreakClass_AL},   {0x25ce, 3, LineBreakClass_AI},
    {0x25d2, 15, LineBreakClass_AL},  {0x25e2, 3, LineBreakClass_AI},
    {0x25e6, 8, LineBreakClass_AL},   {0x25ef, 0, LineBreakClass_AI},
    {0x25f0, 15, LineBreakClass_AL},  {0x2604, 0, LineBreakClass_AL},
    {0x2605, 1, LineBreakClass_AI},   {0x2607, 1, LineBreakClass_AL},
    {0x2609, 0, LineBreakClass_AI},   {0x260a, 3, LineBreakClass_AL},
    {0x260e, 1, LineBreakClass_AI},   {0x2610, 3, LineBreakClass_AL},
    {0x2616, 1, LineBreakClass_AI},   {0x2619, 0, LineBreakClass_AL},
    {0x2620, 24, LineBreakClass_AL},  {0x263c, 3, LineBreakClass_AL},
    {0x2640, 0, LineBreakClass_AI},   {0x2641, 0, LineBreakClass_AL},
    {0x2642, 0, LineBreakClass_AI},   {0x2643, 28, LineBreakClass_AL},
    {0x2660, 1, LineBreakClass_AI},   {0x2662, 0, LineBreakClass_AL},
    {0x2663, 2, LineBreakClass_AI},   {0x2666, 0, LineBreakClass_AL},
    {0x2667, 0, LineBreakClass_AI},   {0x2669, 1, LineBreakClass_AI},
    {0x266b, 0, LineBreakClass_AL},   {0x266c, 1, LineBreakClass_AI},
    {0x266e, 0, LineBreakClass_AL},   {0x266f, 0, LineBreakClass_AI},
    {0x2670, 14, LineBreakClass_AL},  {0x2680, 29, LineBreakClass_AL},
    {0x269e, 1, LineBreakClass_AI},   {0x26a0, 28, LineBreakClass_AL},
    {0x26c9, 3, LineBreakClass_AI},   {0x26ce, 0, LineBreakClass_AL},
    {0x26d2, 0, LineBreakClass_AI},   {0x26d5, 2, LineBreakClass_AI},
    {0x26da, 1, LineBreakClass_AI},   {0x26dd, 1, LineBreakClass_AI},
    {0x26e2, 0, LineBreakClass_AL},   {0x26e3, 0, LineBreakClass_AI},
    {0x26e4, 3, LineBreakClass_AL},   {0x26e8, 1, LineBreakClass_AI},
    {0x26eb, 5, LineBreakClass_AI},   {0x26f6, 0, LineBreakClass_AI},
    {0x26fb, 1, LineBreakClass_AI},   {0x2705, 2, LineBreakClass_AL},
    {0x270e, 72, LineBreakClass_AL},  {0x2757, 0, LineBreakClass_AI},
    {0x2758, 2, LineBreakClass_AL},   {0x275b, 3, LineBreakClass_QU},
    {0x275f, 2, LineBreakClass_AL},   {0x2762, 1, LineBreakClass_EX},
    {0x2764, 3, LineBreakClass_AL},   {0x2768, 0, LineBreakClass_OP},
    {0x2769, 0, LineBreakClass_CL},   {0x276a, 0, LineBreakClass_OP},
    {0x276b, 0, LineBreakClass_CL},   {0x276c, 0, LineBreakClass_OP},
    {0x276d, 0, LineBreakClass_CL},   {0x276e, 0, LineBreakClass_OP},
    {0x276f, 0, LineBreakClass_CL},   {0x2770, 0, LineBreakClass_OP},
    {0x2771, 0, LineBreakClass_CL},   {0x2772, 0, LineBreakClass_OP},
    {0x2773, 0, LineBreakClass_CL},   {0x2774, 0, LineBreakClass_OP},
    {0x2775, 0, LineBreakClass_CL},   {0x2776, 29, LineBreakClass_AI},
    {0x2794, 48, LineBreakClass_AL},  {0x27c5, 0, LineBreakClass_OP},
    {0x27c6, 0, LineBreakClass_CL},   {0x27c7, 30, LineBreakClass_AL},
    {0x27e6, 0, LineBreakClass_OP},   {0x27e7, 0, LineBreakClass_CL},
    {0x27e8, 0, LineBreakClass_OP},   {0x27e9, 0, LineBreakClass_CL},
    {0x27ea, 0, LineBreakClass_OP},   {0x27eb, 0, LineBreakClass_CL},
    {0x27ec, 0, LineBreakClass_OP},   {0x27ed, 0, LineBreakClass_CL},
    {0x27ee, 0, LineBreakClass_OP},   {0x27ef, 0, LineBreakClass_CL},
    {0x27f0, 255, LineBreakClass_AL}, {0x28f0, 146, LineBreakClass_AL},
    {0x2983, 0, LineBreakClass_OP},   {0x2984, 0, LineBreakClass_CL},
    {0x2985, 0, LineBreakClass_OP},   {0x2986, 0, LineBreakClass_CL},
    {0x2987, 0, LineBreakClass_OP},   {0x2988, 0, LineBreakClass_CL},
    {0x2989, 0, LineBreakClass_OP},   {0x298a, 0, LineBreakClass_CL},
    {0x298b, 0, LineBreakClass_OP},   {0x298c, 0, LineBreakClass_CL},
    {0x298d, 0, LineBreakClass_OP},   {0x298e, 0, LineBreakClass_CL},
    {0x298f, 0, LineBreakClass_OP},   {0x2990, 0, LineBreakClass_CL},
    {0x2991, 0, LineBreakClass_OP},   {0x2992, 0, LineBreakClass_CL},
    {0x2993, 0, LineBreakClass_OP},   {0x2994, 0, LineBreakClass_CL},
    {0x2995, 0, LineBreakClass_OP},   {0x2996, 0, LineBreakClass_CL},
    {0x2997, 0, LineBreakClass_OP},   {0x2998, 0, LineBreakClass_CL},
    {0x2999, 62, LineBreakClass_AL},  {0x29d8, 0, LineBreakClass_OP},
    {0x29d9, 0, LineBreakClass_CL},   {0x29da, 0, LineBreakClass_OP},
    {0x29db, 0, LineBreakClass_CL},   {0x29dc, 31, LineBreakClass_AL},
    {0x29fc, 0, LineBreakClass_OP},   {0x29fd, 0, LineBreakClass_CL},
    {0x29fe, 255, LineBreakClass_AL}, {0x2afe, 78, LineBreakClass_AL},
    {0x2b50, 4, LineBreakClass_AL},   {0x2b55, 4, LineBreakClass_AI},
    {0x2c00, 46, LineBreakClass_AL},  {0x2c30, 46, LineBreakClass_AL},
    {0x2c60, 142, LineBreakClass_AL}, {0x2cef, 2, LineBreakClass_CM},
    {0x2cf2, 1, LineBreakClass_AL},   {0x2cf9, 0, LineBreakClass_EX},
    {0x2cfa, 2, LineBreakClass_BA},   {0x2cfd, 0, LineBreakClass_AL},
    {0x2cfe, 0, LineBreakClass_EX},   {0x2cff, 0, LineBreakClass_BA},
    {0x2d00, 37, LineBreakClass_AL},  {0x2d27, 0, LineBreakClass_AL},
    {0x2d2d, 0, LineBreakClass_AL},   {0x2d30, 55, LineBreakClass_AL},
    {0x2d6f, 0, LineBreakClass_AL},   {0x2d70, 0, LineBreakClass_BA},
    {0x2d7f, 0, LineBreakClass_CM},   {0x2d80, 22, LineBreakClass_AL},
    {0x2da0, 6, LineBreakClass_AL},   {0x2da8, 6, LineBreakClass_AL},
    {0x2db0, 6, LineBreakClass_AL},   {0x2db8, 6, LineBreakClass_AL},
    {0x2dc0, 6, LineBreakClass_AL},   {0x2dc8, 6, LineBreakClass_AL},
    {0x2dd0, 6, LineBreakClass_AL},   {0x2dd8, 6, LineBreakClass_AL},
    {0x2de0, 31, LineBreakClass_CM},  {0x2e00, 13, LineBreakClass_QU},
    {0x2e0e, 7, LineBreakClass_BA},   {0x2e16, 0, LineBreakClass_AL},
    {0x2e17, 0, LineBreakClass_BA},   {0x2e18, 0, LineBreakClass_OP},
    {0x2e19, 0, LineBreakClass_BA},   {0x2e1a, 1, LineBreakClass_AL},
    {0x2e1c, 1, LineBreakClass_QU},   {0x2e1e, 1, LineBreakClass_AL},
    {0x2e20, 1, LineBreakClass_QU},   {0x2e22, 0, LineBreakClass_OP},
    {0x2e23, 0, LineBreakClass_CL},   {0x2e24, 0, LineBreakClass_OP},
    {0x2e25, 0, LineBreakClass_CL},   {0x2e26, 0, LineBreakClass_OP},
    {0x2e27, 0, LineBreakClass_CL},   {0x2e28, 0, LineBreakClass_OP},
    {0x2e29, 0, LineBreakClass_CL},   {0x2e2a, 3, LineBreakClass_BA},
    {0x2e2e, 0, LineBreakClass_EX},   {0x2e2f, 0, LineBreakClass_AL},
    {0x2e30, 1, LineBreakClass_BA},   {0x2e32, 0, LineBreakClass_AL},
    {0x2e33, 1, LineBreakClass_BA},   {0x2e35, 4, LineBreakClass_AL},
    {0x2e3a, 1, LineBreakClass_B2},   {0x3001, 1, LineBreakClass_CL},
    {0x3005, 0, LineBreakClass_NS},   {0x3008, 0, LineBreakClass_OP},
    {0x3009, 0, LineBreakClass_CL},   {0x300a, 0, LineBreakClass_OP},
    {0x300b, 0, LineBreakClass_CL},   {0x300c, 0, LineBreakClass_OP},
    {0x300d, 0, LineBreakClass_CL},   {0x300e, 0, LineBreakClass_OP},
    {0x300f, 0, LineBreakClass_CL},   {0x3010, 0, LineBreakClass_OP},
    {0x3011, 0, LineBreakClass_CL},   {0x3014, 0, LineBreakClass_OP},
    {0x3015, 0, LineBreakClass_CL},   {0x3016, 0, LineBreakClass_OP},
    {0x3017, 0, LineBreakClass_CL},   {0x3018, 0, LineBreakClass_OP},
    {0x3019, 0, LineBreakClass_CL},   {0x301a, 0, LineBreakClass_OP},
    {0x301b, 0, LineBreakClass_CL},   {0x301c, 0, LineBreakClass_NS},
    {0x301d, 0, LineBreakClass_OP},   {0x301e, 1, LineBreakClass_CL},
    {0x302a, 5, LineBreakClass_CM},   {0x303b, 1, LineBreakClass_NS},
    {0x3041, 0, LineBreakClass_CJ},   {0x3043, 0, LineBreakClass_CJ},
    {0x3045, 0, LineBreakClass_CJ},   {0x3047, 0, LineBreakClass_CJ},
    {0x3049, 0, LineBreakClass_CJ},   {0x3063, 0, LineBreakClass_CJ},
    {0x3083, 0, LineBreakClass_CJ},   {0x3085, 0, LineBreakClass_CJ},
    {0x3087, 0, LineBreakClass_CJ},   {0x308e, 0, LineBreakClass_CJ},
    {0x3095, 1, LineBreakClass_CJ},   {0x3099, 1, LineBreakClass_CM},
    {0x309b, 3, LineBreakClass_NS},   {0x30a0, 0, LineBreakClass_NS},
    {0x30a1, 0, LineBreakClass_CJ},   {0x30a3, 0, LineBreakClass_CJ},
    {0x30a5, 0, LineBreakClass_CJ},   {0x30a7, 0, LineBreakClass_CJ},
    {0x30a9, 0, LineBreakClass_CJ},   {0x30c3, 0, LineBreakClass_CJ},
    {0x30e3, 0, LineBreakClass_CJ},   {0x30e5, 0, LineBreakClass_CJ},
    {0x30e7, 0, LineBreakClass_CJ},   {0x30ee, 0, LineBreakClass_CJ},
    {0x30f5, 1, LineBreakClass_CJ},   {0x30fb, 0, LineBreakClass_NS},
    {0x30fc, 0, LineBreakClass_CJ},   {0x30fd, 1, LineBreakClass_NS},
    {0x31f0, 15, LineBreakClass_CJ},  {0x3248, 7, LineBreakClass_AI},
    {0x4dc0, 63, LineBreakClass_AL},  {0xa015, 0, LineBreakClass_NS},
    {0xa4d0, 45, LineBreakClass_AL},  {0xa4fe, 1, LineBreakClass_BA},
    {0xa500, 255, LineBreakClass_AL}, {0xa600, 12, LineBreakClass_AL},
    {0xa60d, 0, LineBreakClass_BA},   {0xa60e, 0, LineBreakClass_EX},
    {0xa60f, 0, LineBreakClass_BA},   {0xa610, 15, LineBreakClass_AL},
    {0xa620, 9, LineBreakClass_NU},   {0xa62a, 1, LineBreakClass_AL},
    {0xa640, 46, LineBreakClass_AL},  {0xa66f, 3, LineBreakClass_CM},
    {0xa673, 0, LineBreakClass_AL},   {0xa674, 9, LineBreakClass_CM},
    {0xa67e, 25, LineBreakClass_AL},  {0xa69f, 0, LineBreakClass_CM},
    {0xa6a0, 79, LineBreakClass_AL},  {0xa6f0, 1, LineBreakClass_CM},
    {0xa6f2, 0, LineBreakClass_AL},   {0xa6f3, 4, LineBreakClass_BA},
    {0xa700, 142, LineBreakClass_AL}, {0xa790, 3, LineBreakClass_AL},
    {0xa7a0, 10, LineBreakClass_AL},  {0xa7f8, 9, LineBreakClass_AL},
    {0xa802, 0, LineBreakClass_CM},   {0xa803, 2, LineBreakClass_AL},
    {0xa806, 0, LineBreakClass_CM},   {0xa807, 3, LineBreakClass_AL},
    {0xa80b, 0, LineBreakClass_CM},   {0xa80c, 22, LineBreakClass_AL},
    {0xa823, 4, LineBreakClass_CM},   {0xa828, 3, LineBreakClass_AL},
    {0xa830, 7, LineBreakClass_AL},   {0xa838, 0, LineBreakClass_PO},
    {0xa839, 0, LineBreakClass_AL},   {0xa840, 51, LineBreakClass_AL},
    {0xa874, 1, LineBreakClass_BB},   {0xa876, 1, LineBreakClass_EX},
    {0xa880, 1, LineBreakClass_CM},   {0xa882, 49, LineBreakClass_AL},
    {0xa8b4, 16, LineBreakClass_CM},  {0xa8ce, 1, LineBreakClass_BA},
    {0xa8d0, 9, LineBreakClass_NU},   {0xa8e0, 17, LineBreakClass_CM},
    {0xa8f2, 9, LineBreakClass_AL},   {0xa900, 9, LineBreakClass_NU},
    {0xa90a, 27, LineBreakClass_AL},  {0xa926, 7, LineBreakClass_CM},
    {0xa92e, 1, LineBreakClass_BA},   {0xa930, 22, LineBreakClass_AL},
    {0xa947, 12, LineBreakClass_CM},  {0xa95f, 0, LineBreakClass_AL},
    {0xa960, 28, LineBreakClass_JL},  {0xa980, 3, LineBreakClass_CM},
    {0xa984, 46, LineBreakClass_AL},  {0xa9b3, 13, LineBreakClass_CM},
    {0xa9c1, 5, LineBreakClass_AL},   {0xa9c7, 2, LineBreakClass_BA},
    {0xa9ca, 3, LineBreakClass_AL},   {0xa9cf, 0, LineBreakClass_AL},
    {0xa9d0, 9, LineBreakClass_NU},   {0xa9de, 1, LineBreakClass_AL},
    {0xaa00, 40, LineBreakClass_AL},  {0xaa29, 13, LineBreakClass_CM},
    {0xaa40, 2, LineBreakClass_AL},   {0xaa43, 0, LineBreakClass_CM},
    {0xaa44, 7, LineBreakClass_AL},   {0xaa4c, 1, LineBreakClass_CM},
    {0xaa50, 9, LineBreakClass_NU},   {0xaa5c, 0, LineBreakClass_AL},
    {0xaa5d, 2, LineBreakClass_BA},   {0xaa60, 27, LineBreakClass_SA},
    {0xaa80, 66, LineBreakClass_SA},  {0xaadb, 4, LineBreakClass_SA},
    {0xaae0, 10, LineBreakClass_AL},  {0xaaeb, 4, LineBreakClass_CM},
    {0xaaf0, 1, LineBreakClass_BA},   {0xaaf2, 2, LineBreakClass_AL},
    {0xaaf5, 1, LineBreakClass_CM},   {0xab01, 5, LineBreakClass_AL},
    {0xab09, 5, LineBreakClass_AL},   {0xab11, 5, LineBreakClass_AL},
    {0xab20, 6, LineBreakClass_AL},   {0xab28, 6, LineBreakClass_AL},
    {0xabc0, 34, LineBreakClass_AL},  {0xabe3, 7, LineBreakClass_CM},
    {0xabeb, 0, LineBreakClass_BA},   {0xabec, 1, LineBreakClass_CM},
    {0xabf0, 9, LineBreakClass_NU},   {0xac00, 0, LineBreakClass_H2},
    {0xac01, 25, LineBreakClass_H3},  {0xac1c, 0, LineBreakClass_H2},
    {0xac1d, 25, LineBreakClass_H3},  {0xac38, 0, LineBreakClass_H2},
    {0xac39, 25, LineBreakClass_H3},  {0xac54, 0, LineBreakClass_H2},
    {0xac55, 25, LineBreakClass_H3},  {0xac70, 0, LineBreakClass_H2},
    {0xac71, 25, LineBreakClass_H3},  {0xac8c, 0, LineBreakClass_H2},
    {0xac8d, 25, LineBreakClass_H3},  {0xaca8, 0, LineBreakClass_H2},
    {0xaca9, 25, LineBreakClass_H3},  {0xacc4, 0, LineBreakClass_H2},
    {0xacc5, 25, LineBreakClass_H3},  {0xace0, 0, LineBreakClass_H2},
    {0xace1, 25, LineBreakClass_H3},  {0xacfc, 0, LineBreakClass_H2},
    {0xacfd, 25, LineBreakClass_H3},  {0xad18, 0, LineBreakClass_H2},
    {0xad19, 25, LineBreakClass_H3},  {0xad34, 0, LineBreakClass_H2},
    {0xad35, 25, LineBreakClass_H3},  {0xad50, 0, LineBreakClass_H2},
    {0xad51, 25, LineBreakClass_H3},  {0xad6c, 0, LineBreakClass_H2},
    {0xad6d, 25, LineBreakClass_H3},  {0xad88, 0, LineBreakClass_H2},
    {0xad89, 25, LineBreakClass_H3},  {0xada4, 0, LineBreakClass_H2},
    {0xada5, 25, LineBreakClass_H3},  {0xadc0, 0, LineBreakClass_H2},
    {0xadc1, 25, LineBreakClass_H3},  {0xaddc, 0, LineBreakClass_H2},
    {0xaddd, 25, LineBreakClass_H3},  {0xadf8, 0, LineBreakClass_H2},
    {0xadf9, 25, LineBreakClass_H3},  {0xae14, 0, LineBreakClass_H2},
    {0xae15, 25, LineBreakClass_H3},  {0xae30, 0, LineBreakClass_H2},
    {0xae31, 25, LineBreakClass_H3},  {0xae4c, 0, LineBreakClass_H2},
    {0xae4d, 25, LineBreakClass_H3},  {0xae68, 0, LineBreakClass_H2},
    {0xae69, 25, LineBreakClass_H3},  {0xae84, 0, LineBreakClass_H2},
    {0xae85, 25, LineBreakClass_H3},  {0xaea0, 0, LineBreakClass_H2},
    {0xaea1, 25, LineBreakClass_H3},  {0xaebc, 0, LineBreakClass_H2},
    {0xaebd, 25, LineBreakClass_H3},  {0xaed8, 0, LineBreakClass_H2},
    {0xaed9, 25, LineBreakClass_H3},  {0xaef4, 0, LineBreakClass_H2},
    {0xaef5, 25, LineBreakClass_H3},  {0xaf10, 0, LineBreakClass_H2},
    {0xaf11, 25, LineBreakClass_H3},  {0xaf2c, 0, LineBreakClass_H2},
    {0xaf2d, 25, LineBreakClass_H3},  {0xaf48, 0, LineBreakClass_H2},
    {0xaf49, 25, LineBreakClass_H3},  {0xaf64, 0, LineBreakClass_H2},
    {0xaf65, 25, LineBreakClass_H3},  {0xaf80, 0, LineBreakClass_H2},
    {0xaf81, 25, LineBreakClass_H3},  {0xaf9c, 0, LineBreakClass_H2},
    {0xaf9d, 25, LineBreakClass_H3},  {0xafb8, 0, LineBreakClass_H2},
    {0xafb9, 25, LineBreakClass_H3},  {0xafd4, 0, LineBreakClass_H2},
    {0xafd5, 25, LineBreakClass_H3},  {0xaff0, 0, LineBreakClass_H2},
    {0xaff1, 25, LineBreakClass_H3},  {0xb00c, 0, LineBreakClass_H2},
    {0xb00d, 25, LineBreakClass_H3},  {0xb028, 0, LineBreakClass_H2},
    {0xb029, 25, LineBreakClass_H3},  {0xb044, 0, LineBreakClass_H2},
    {0xb045, 25, LineBreakClass_H3},  {0xb060, 0, LineBreakClass_H2},
    {0xb061, 25, LineBreakClass_H3},  {0xb07c, 0, LineBreakClass_H2},
    {0xb07d, 25, LineBreakClass_H3},  {0xb098, 0, LineBreakClass_H2},
    {0xb099, 25, LineBreakClass_H3},  {0xb0b4, 0, LineBreakClass_H2},
    {0xb0b5, 25, LineBreakClass_H3},  {0xb0d0, 0, LineBreakClass_H2},
    {0xb0d1, 25, LineBreakClass_H3},  {0xb0ec, 0, LineBreakClass_H2},
    {0xb0ed, 25, LineBreakClass_H3},  {0xb108, 0, LineBreakClass_H2},
    {0xb109, 25, LineBreakClass_H3},  {0xb124, 0, LineBreakClass_H2},
    {0xb125, 25, LineBreakClass_H3},  {0xb140, 0, LineBreakClass_H2},
    {0xb141, 25, LineBreakClass_H3},  {0xb15c, 0, LineBreakClass_H2},
    {0xb15d, 25, LineBreakClass_H3},  {0xb178, 0, LineBreakClass_H2},
    {0xb179, 25, LineBreakClass_H3},  {0xb194, 0, LineBreakClass_H2},
    {0xb195, 25, LineBreakClass_H3},  {0xb1b0, 0, LineBreakClass_H2},
    {0xb1b1, 25, LineBreakClass_H3},  {0xb1cc, 0, LineBreakClass_H2},
    {0xb1cd, 25, LineBreakClass_H3},  {0xb1e8, 0, LineBreakClass_H2},
    {0xb1e9, 25, LineBreakClass_H3},  {0xb204, 0, LineBreakClass_H2},
    {0xb205, 25, LineBreakClass_H3},  {0xb220, 0, LineBreakClass_H2},
    {0xb221, 25, LineBreakClass_H3},  {0xb23c, 0, LineBreakClass_H2},
    {0xb23d, 25, LineBreakClass_H3},  {0xb258, 0, LineBreakClass_H2},
    {0xb259, 25, LineBreakClass_H3},  {0xb274, 0, LineBreakClass_H2},
    {0xb275, 25, LineBreakClass_H3},  {0xb290, 0, LineBreakClass_H2},
    {0xb291, 25, LineBreakClass_H3},  {0xb2ac, 0, LineBreakClass_H2},
    {0xb2ad, 25, LineBreakClass_H3},  {0xb2c8, 0, LineBreakClass_H2},
    {0xb2c9, 25, LineBreakClass_H3},  {0xb2e4, 0, LineBreakClass_H2},
    {0xb2e5, 25, LineBreakClass_H3},  {0xb300, 0, LineBreakClass_H2},
    {0xb301, 25, LineBreakClass_H3},  {0xb31c, 0, LineBreakClass_H2},
    {0xb31d, 25, LineBreakClass_H3},  {0xb338, 0, LineBreakClass_H2},
    {0xb339, 25, LineBreakClass_H3},  {0xb354, 0, LineBreakClass_H2},
    {0xb355, 25, LineBreakClass_H3},  {0xb370, 0, LineBreakClass_H2},
    {0xb371, 25, LineBreakClass_H3},  {0xb38c, 0, LineBreakClass_H2},
    {0xb38d, 25, LineBreakClass_H3},  {0xb3a8, 0, LineBreakClass_H2},
    {0xb3a9, 25, LineBreakClass_H3},  {0xb3c4, 0, LineBreakClass_H2},
    {0xb3c5, 25, LineBreakClass_H3},  {0xb3e0, 0, LineBreakClass_H2},
    {0xb3e1, 25, LineBreakClass_H3},  {0xb3fc, 0, LineBreakClass_H2},
    {0xb3fd, 25, LineBreakClass_H3},  {0xb418, 0, LineBreakClass_H2},
    {0xb419, 25, LineBreakClass_H3},  {0xb434, 0, LineBreakClass_H2},
    {0xb435, 25, LineBreakClass_H3},  {0xb450, 0, LineBreakClass_H2},
    {0xb451, 25, LineBreakClass_H3},  {0xb46c, 0, LineBreakClass_H2},
    {0xb46d, 25, LineBreakClass_H3},  {0xb488, 0, LineBreakClass_H2},
    {0xb489, 25, LineBreakClass_H3},  {0xb4a4, 0, LineBreakClass_H2},
    {0xb4a5, 25, LineBreakClass_H3},  {0xb4c0, 0, LineBreakClass_H2},
    {0xb4c1, 25, LineBreakClass_H3},  {0xb4dc, 0, LineBreakClass_H2},
    {0xb4dd, 25, LineBreakClass_H3},  {0xb4f8, 0, LineBreakClass_H2},
    {0xb4f9, 25, LineBreakClass_H3},  {0xb514, 0, LineBreakClass_H2},
    {0xb515, 25, LineBreakClass_H3},  {0xb530, 0, LineBreakClass_H2},
    {0xb531, 25, LineBreakClass_H3},  {0xb54c, 0, LineBreakClass_H2},
    {0xb54d, 25, LineBreakClass_H3},  {0xb568, 0, LineBreakClass_H2},
    {0xb569, 25, LineBreakClass_H3},  {0xb584, 0, LineBreakClass_H2},
    {0xb585, 25, LineBreakClass_H3},  {0xb5a0, 0, LineBreakClass_H2},
    {0xb5a1, 25, LineBreakClass_H3},  {0xb5bc, 0, LineBreakClass_H2},
    {0xb5bd, 25, LineBreakClass_H3},  {0xb5d8, 0, LineBreakClass_H2},
    {0xb5d9, 25, LineBreakClass_H3},  {0xb5f4, 0, LineBreakClass_H2},
    {0xb5f5, 25, LineBreakClass_H3},  {0xb610, 0, LineBreakClass_H2},
    {0xb611, 25, LineBreakClass_H3},  {0xb62c, 0, LineBreakClass_H2},
    {0xb62d, 25, LineBreakClass_H3},  {0xb648, 0, LineBreakClass_H2},
    {0xb649, 25, LineBreakClass_H3},  {0xb664, 0, LineBreakClass_H2},
    {0xb665, 25, LineBreakClass_H3},  {0xb680, 0, LineBreakClass_H2},
    {0xb681, 25, LineBreakClass_H3},  {0xb69c, 0, LineBreakClass_H2},
    {0xb69d, 25, LineBreakClass_H3},  {0xb6b8, 0, LineBreakClass_H2},
    {0xb6b9, 25, LineBreakClass_H3},  {0xb6d4, 0, LineBreakClass_H2},
    {0xb6d5, 25, LineBreakClass_H3},  {0xb6f0, 0, LineBreakClass_H2},
    {0xb6f1, 25, LineBreakClass_H3},  {0xb70c, 0, LineBreakClass_H2},
    {0xb70d, 25, LineBreakClass_H3},  {0xb728, 0, LineBreakClass_H2},
    {0xb729, 25, LineBreakClass_H3},  {0xb744, 0, LineBreakClass_H2},
    {0xb745, 25, LineBreakClass_H3},  {0xb760, 0, LineBreakClass_H2},
    {0xb761, 25, LineBreakClass_H3},  {0xb77c, 0, LineBreakClass_H2},
    {0xb77d, 25, LineBreakClass_H3},  {0xb798, 0, LineBreakClass_H2},
    {0xb799, 25, LineBreakClass_H3},  {0xb7b4, 0, LineBreakClass_H2},
    {0xb7b5, 25, LineBreakClass_H3},  {0xb7d0, 0, LineBreakClass_H2},
    {0xb7d1, 25, LineBreakClass_H3},  {0xb7ec, 0, LineBreakClass_H2},
    {0xb7ed, 25, LineBreakClass_H3},  {0xb808, 0, LineBreakClass_H2},
    {0xb809, 25, LineBreakClass_H3},  {0xb824, 0, LineBreakClass_H2},
    {0xb825, 25, LineBreakClass_H3},  {0xb840, 0, LineBreakClass_H2},
    {0xb841, 25, LineBreakClass_H3},  {0xb85c, 0, LineBreakClass_H2},
    {0xb85d, 25, LineBreakClass_H3},  {0xb878, 0, LineBreakClass_H2},
    {0xb879, 25, LineBreakClass_H3},  {0xb894, 0, LineBreakClass_H2},
    {0xb895, 25, LineBreakClass_H3},  {0xb8b0, 0, LineBreakClass_H2},
    {0xb8b1, 25, LineBreakClass_H3},  {0xb8cc, 0, LineBreakClass_H2},
    {0xb8cd, 25, LineBreakClass_H3},  {0xb8e8, 0, LineBreakClass_H2},
    {0xb8e9, 25, LineBreakClass_H3},  {0xb904, 0, LineBreakClass_H2},
    {0xb905, 25, LineBreakClass_H3},  {0xb920, 0, LineBreakClass_H2},
    {0xb921, 25, LineBreakClass_H3},  {0xb93c, 0, LineBreakClass_H2},
    {0xb93d, 25, LineBreakClass_H3},  {0xb958, 0, LineBreakClass_H2},
    {0xb959, 25, LineBreakClass_H3},  {0xb974, 0, LineBreakClass_H2},
    {0xb975, 25, LineBreakClass_H3},  {0xb990, 0, LineBreakClass_H2},
    {0xb991, 25, LineBreakClass_H3},  {0xb9ac, 0, LineBreakClass_H2},
    {0xb9ad, 25, LineBreakClass_H3},  {0xb9c8, 0, LineBreakClass_H2},
    {0xb9c9, 25, LineBreakClass_H3},  {0xb9e4, 0, LineBreakClass_H2},
    {0xb9e5, 25, LineBreakClass_H3},  {0xba00, 0, LineBreakClass_H2},
    {0xba01, 25, LineBreakClass_H3},  {0xba1c, 0, LineBreakClass_H2},
    {0xba1d, 25, LineBreakClass_H3},  {0xba38, 0, LineBreakClass_H2},
    {0xba39, 25, LineBreakClass_H3},  {0xba54, 0, LineBreakClass_H2},
    {0xba55, 25, LineBreakClass_H3},  {0xba70, 0, LineBreakClass_H2},
    {0xba71, 25, LineBreakClass_H3},  {0xba8c, 0, LineBreakClass_H2},
    {0xba8d, 25, LineBreakClass_H3},  {0xbaa8, 0, LineBreakClass_H2},
    {0xbaa9, 25, LineBreakClass_H3},  {0xbac4, 0, LineBreakClass_H2},
    {0xbac5, 25, LineBreakClass_H3},  {0xbae0, 0, LineBreakClass_H2},
    {0xbae1, 25, LineBreakClass_H3},  {0xbafc, 0, LineBreakClass_H2},
    {0xbafd, 25, LineBreakClass_H3},  {0xbb18, 0, LineBreakClass_H2},
    {0xbb19, 25, LineBreakClass_H3},  {0xbb34, 0, LineBreakClass_H2},
    {0xbb35, 25, LineBreakClass_H3},  {0xbb50, 0, LineBreakClass_H2},
    {0xbb51, 25, LineBreakClass_H3},  {0xbb6c, 0, LineBreakClass_H2},
    {0xbb6d, 25, LineBreakClass_H3},  {0xbb88, 0, LineBreakClass_H2},
    {0xbb89, 25, LineBreakClass_H3},  {0xbba4, 0, LineBreakClass_H2},
    {0xbba5, 25, LineBreakClass_H3},  {0xbbc0, 0, LineBreakClass_H2},
    {0xbbc1, 25, LineBreakClass_H3},  {0xbbdc, 0, LineBreakClass_H2},
    {0xbbdd, 25, LineBreakClass_H3},  {0xbbf8, 0, LineBreakClass_H2},
    {0xbbf9, 25, LineBreakClass_H3},  {0xbc14, 0, LineBreakClass_H2},
    {0xbc15, 25, LineBreakClass_H3},  {0xbc30, 0, LineBreakClass_H2},
    {0xbc31, 25, LineBreakClass_H3},  {0xbc4c, 0, LineBreakClass_H2},
    {0xbc4d, 25, LineBreakClass_H3},  {0xbc68, 0, LineBreakClass_H2},
    {0xbc69, 25, LineBreakClass_H3},  {0xbc84, 0, LineBreakClass_H2},
    {0xbc85, 25, LineBreakClass_H3},  {0xbca0, 0, LineBreakClass_H2},
    {0xbca1, 25, LineBreakClass_H3},  {0xbcbc, 0, LineBreakClass_H2},
    {0xbcbd, 25, LineBreakClass_H3},  {0xbcd8, 0, LineBreakClass_H2},
    {0xbcd9, 25, LineBreakClass_H3},  {0xbcf4, 0, LineBreakClass_H2},
    {0xbcf5, 25, LineBreakClass_H3},  {0xbd10, 0, LineBreakClass_H2},
    {0xbd11, 25, LineBreakClass_H3},  {0xbd2c, 0, LineBreakClass_H2},
    {0xbd2d, 25, LineBreakClass_H3},  {0xbd48, 0, LineBreakClass_H2},
    {0xbd49, 25, LineBreakClass_H3},  {0xbd64, 0, LineBreakClass_H2},
    {0xbd65, 25, LineBreakClass_H3},  {0xbd80, 0, LineBreakClass_H2},
    {0xbd81, 25, LineBreakClass_H3},  {0xbd9c, 0, LineBreakClass_H2},
    {0xbd9d, 25, LineBreakClass_H3},  {0xbdb8, 0, LineBreakClass_H2},
    {0xbdb9, 25, LineBreakClass_H3},  {0xbdd4, 0, LineBreakClass_H2},
    {0xbdd5, 25, LineBreakClass_H3},  {0xbdf0, 0, LineBreakClass_H2},
    {0xbdf1, 25, LineBreakClass_H3},  {0xbe0c, 0, LineBreakClass_H2},
    {0xbe0d, 25, LineBreakClass_H3},  {0xbe28, 0, LineBreakClass_H2},
    {0xbe29, 25, LineBreakClass_H3},  {0xbe44, 0, LineBreakClass_H2},
    {0xbe45, 25, LineBreakClass_H3},  {0xbe60, 0, LineBreakClass_H2},
    {0xbe61, 25, LineBreakClass_H3},  {0xbe7c, 0, LineBreakClass_H2},
    {0xbe7d, 25, LineBreakClass_H3},  {0xbe98, 0, LineBreakClass_H2},
    {0xbe99, 25, LineBreakClass_H3},  {0xbeb4, 0, LineBreakClass_H2},
    {0xbeb5, 25, LineBreakClass_H3},  {0xbed0, 0, LineBreakClass_H2},
    {0xbed1, 25, LineBreakClass_H3},  {0xbeec, 0, LineBreakClass_H2},
    {0xbeed, 25, LineBreakClass_H3},  {0xbf08, 0, LineBreakClass_H2},
    {0xbf09, 25, LineBreakClass_H3},  {0xbf24, 0, LineBreakClass_H2},
    {0xbf25, 25, LineBreakClass_H3},  {0xbf40, 0, LineBreakClass_H2},
    {0xbf41, 25, LineBreakClass_H3},  {0xbf5c, 0, LineBreakClass_H2},
    {0xbf5d, 25, LineBreakClass_H3},  {0xbf78, 0, LineBreakClass_H2},
    {0xbf79, 25, LineBreakClass_H3},  {0xbf94, 0, LineBreakClass_H2},
    {0xbf95, 25, LineBreakClass_H3},  {0xbfb0, 0, LineBreakClass_H2},
    {0xbfb1, 25, LineBreakClass_H3},  {0xbfcc, 0, LineBreakClass_H2},
    {0xbfcd, 25, LineBreakClass_H3},  {0xbfe8, 0, LineBreakClass_H2},
    {0xbfe9, 25, LineBreakClass_H3},  {0xc004, 0, LineBreakClass_H2},
    {0xc005, 25, LineBreakClass_H3},  {0xc020, 0, LineBreakClass_H2},
    {0xc021, 25, LineBreakClass_H3},  {0xc03c, 0, LineBreakClass_H2},
    {0xc03d, 25, LineBreakClass_H3},  {0xc058, 0, LineBreakClass_H2},
    {0xc059, 25, LineBreakClass_H3},  {0xc074, 0, LineBreakClass_H2},
    {0xc075, 25, LineBreakClass_H3},  {0xc090, 0, LineBreakClass_H2},
    {0xc091, 25, LineBreakClass_H3},  {0xc0ac, 0, LineBreakClass_H2},
    {0xc0ad, 25, LineBreakClass_H3},  {0xc0c8, 0, LineBreakClass_H2},
    {0xc0c9, 25, LineBreakClass_H3},  {0xc0e4, 0, LineBreakClass_H2},
    {0xc0e5, 25, LineBreakClass_H3},  {0xc100, 0, LineBreakClass_H2},
    {0xc101, 25, LineBreakClass_H3},  {0xc11c, 0, LineBreakClass_H2},
    {0xc11d, 25, LineBreakClass_H3},  {0xc138, 0, LineBreakClass_H2},
    {0xc139, 25, LineBreakClass_H3},  {0xc154, 0, LineBreakClass_H2},
    {0xc155, 25, LineBreakClass_H3},  {0xc170, 0, LineBreakClass_H2},
    {0xc171, 25, LineBreakClass_H3},  {0xc18c, 0, LineBreakClass_H2},
    {0xc18d, 25, LineBreakClass_H3},  {0xc1a8, 0, LineBreakClass_H2},
    {0xc1a9, 25, LineBreakClass_H3},  {0xc1c4, 0, LineBreakClass_H2},
    {0xc1c5, 25, LineBreakClass_H3},  {0xc1e0, 0, LineBreakClass_H2},
    {0xc1e1, 25, LineBreakClass_H3},  {0xc1fc, 0, LineBreakClass_H2},
    {0xc1fd, 25, LineBreakClass_H3},  {0xc218, 0, LineBreakClass_H2},
    {0xc219, 25, LineBreakClass_H3},  {0xc234, 0, LineBreakClass_H2},
    {0xc235, 25, LineBreakClass_H3},  {0xc250, 0, LineBreakClass_H2},
    {0xc251, 25, LineBreakClass_H3},  {0xc26c, 0, LineBreakClass_H2},
    {0xc26d, 25, LineBreakClass_H3},  {0xc288, 0, LineBreakClass_H2},
    {0xc289, 25, LineBreakClass_H3},  {0xc2a4, 0, LineBreakClass_H2},
    {0xc2a5, 25, LineBreakClass_H3},  {0xc2c0, 0, LineBreakClass_H2},
    {0xc2c1, 25, LineBreakClass_H3},  {0xc2dc, 0, LineBreakClass_H2},
    {0xc2dd, 25, LineBreakClass_H3},  {0xc2f8, 0, LineBreakClass_H2},
    {0xc2f9, 25, LineBreakClass_H3},  {0xc314, 0, LineBreakClass_H2},
    {0xc315, 25, LineBreakClass_H3},  {0xc330, 0, LineBreakClass_H2},
    {0xc331, 25, LineBreakClass_H3},  {0xc34c, 0, LineBreakClass_H2},
    {0xc34d, 25, LineBreakClass_H3},  {0xc368, 0, LineBreakClass_H2},
    {0xc369, 25, LineBreakClass_H3},  {0xc384, 0, LineBreakClass_H2},
    {0xc385, 25, LineBreakClass_H3},  {0xc3a0, 0, LineBreakClass_H2},
    {0xc3a1, 25, LineBreakClass_H3},  {0xc3bc, 0, LineBreakClass_H2},
    {0xc3bd, 25, LineBreakClass_H3},  {0xc3d8, 0, LineBreakClass_H2},
    {0xc3d9, 25, LineBreakClass_H3},  {0xc3f4, 0, LineBreakClass_H2},
    {0xc3f5, 25, LineBreakClass_H3},  {0xc410, 0, LineBreakClass_H2},
    {0xc411, 25, LineBreakClass_H3},  {0xc42c, 0, LineBreakClass_H2},
    {0xc42d, 25, LineBreakClass_H3},  {0xc448, 0, LineBreakClass_H2},
    {0xc449, 25, LineBreakClass_H3},  {0xc464, 0, LineBreakClass_H2},
    {0xc465, 25, LineBreakClass_H3},  {0xc480, 0, LineBreakClass_H2},
    {0xc481, 25, LineBreakClass_H3},  {0xc49c, 0, LineBreakClass_H2},
    {0xc49d, 25, LineBreakClass_H3},  {0xc4b8, 0, LineBreakClass_H2},
    {0xc4b9, 25, LineBreakClass_H3},  {0xc4d4, 0, LineBreakClass_H2},
    {0xc4d5, 25, LineBreakClass_H3},  {0xc4f0, 0, LineBreakClass_H2},
    {0xc4f1, 25, LineBreakClass_H3},  {0xc50c, 0, LineBreakClass_H2},
    {0xc50d, 25, LineBreakClass_H3},  {0xc528, 0, LineBreakClass_H2},
    {0xc529, 25, LineBreakClass_H3},  {0xc544, 0, LineBreakClass_H2},
    {0xc545, 25, LineBreakClass_H3},  {0xc560, 0, LineBreakClass_H2},
    {0xc561, 25, LineBreakClass_H3},  {0xc57c, 0, LineBreakClass_H2},
    {0xc57d, 25, LineBreakClass_H3},  {0xc598, 0, LineBreakClass_H2},
    {0xc599, 25, LineBreakClass_H3},  {0xc5b4, 0, LineBreakClass_H2},
    {0xc5b5, 25, LineBreakClass_H3},  {0xc5d0, 0, LineBreakClass_H2},
    {0xc5d1, 25, LineBreakClass_H3},  {0xc5ec, 0, LineBreakClass_H2},
    {0xc5ed, 25, LineBreakClass_H3},  {0xc608, 0, LineBreakClass_H2},
    {0xc609, 25, LineBreakClass_H3},  {0xc624, 0, LineBreakClass_H2},
    {0xc625, 25, LineBreakClass_H3},  {0xc640, 0, LineBreakClass_H2},
    {0xc641, 25, LineBreakClass_H3},  {0xc65c, 0, LineBreakClass_H2},
    {0xc65d, 25, LineBreakClass_H3},  {0xc678, 0, LineBreakClass_H2},
    {0xc679, 25, LineBreakClass_H3},  {0xc694, 0, LineBreakClass_H2},
    {0xc695, 25, LineBreakClass_H3},  {0xc6b0, 0, LineBreakClass_H2},
    {0xc6b1, 25, LineBreakClass_H3},  {0xc6cc, 0, LineBreakClass_H2},
    {0xc6cd, 25, LineBreakClass_H3},  {0xc6e8, 0, LineBreakClass_H2},
    {0xc6e9, 25, LineBreakClass_H3},  {0xc704, 0, LineBreakClass_H2},
    {0xc705, 25, LineBreakClass_H3},  {0xc720, 0, LineBreakClass_H2},
    {0xc721, 25, LineBreakClass_H3},  {0xc73c, 0, LineBreakClass_H2},
    {0xc73d, 25, LineBreakClass_H3},  {0xc758, 0, LineBreakClass_H2},
    {0xc759, 25, LineBreakClass_H3},  {0xc774, 0, LineBreakClass_H2},
    {0xc775, 25, LineBreakClass_H3},  {0xc790, 0, LineBreakClass_H2},
    {0xc791, 25, LineBreakClass_H3},  {0xc7ac, 0, LineBreakClass_H2},
    {0xc7ad, 25, LineBreakClass_H3},  {0xc7c8, 0, LineBreakClass_H2},
    {0xc7c9, 25, LineBreakClass_H3},  {0xc7e4, 0, LineBreakClass_H2},
    {0xc7e5, 25, LineBreakClass_H3},  {0xc800, 0, LineBreakClass_H2},
    {0xc801, 25, LineBreakClass_H3},  {0xc81c, 0, LineBreakClass_H2},
    {0xc81d, 25, LineBreakClass_H3},  {0xc838, 0, LineBreakClass_H2},
    {0xc839, 25, LineBreakClass_H3},  {0xc854, 0, LineBreakClass_H2},
    {0xc855, 25, LineBreakClass_H3},  {0xc870, 0, LineBreakClass_H2},
    {0xc871, 25, LineBreakClass_H3},  {0xc88c, 0, LineBreakClass_H2},
    {0xc88d, 25, LineBreakClass_H3},  {0xc8a8, 0, LineBreakClass_H2},
    {0xc8a9, 25, LineBreakClass_H3},  {0xc8c4, 0, LineBreakClass_H2},
    {0xc8c5, 25, LineBreakClass_H3},  {0xc8e0, 0, LineBreakClass_H2},
    {0xc8e1, 25, LineBreakClass_H3},  {0xc8fc, 0, LineBreakClass_H2},
    {0xc8fd, 25, LineBreakClass_H3},  {0xc918, 0, LineBreakClass_H2},
    {0xc919, 25, LineBreakClass_H3},  {0xc934, 0, LineBreakClass_H2},
    {0xc935, 25, LineBreakClass_H3},  {0xc950, 0, LineBreakClass_H2},
    {0xc951, 25, LineBreakClass_H3},  {0xc96c, 0, LineBreakClass_H2},
    {0xc96d, 25, LineBreakClass_H3},  {0xc988, 0, LineBreakClass_H2},
    {0xc989, 25, LineBreakClass_H3},  {0xc9a4, 0, LineBreakClass_H2},
    {0xc9a5, 25, LineBreakClass_H3},  {0xc9c0, 0, LineBreakClass_H2},
    {0xc9c1, 25, LineBreakClass_H3},  {0xc9dc, 0, LineBreakClass_H2},
    {0xc9dd, 25, LineBreakClass_H3},  {0xc9f8, 0, LineBreakClass_H2},
    {0xc9f9, 25, LineBreakClass_H3},  {0xca14, 0, LineBreakClass_H2},
    {0xca15, 25, LineBreakClass_H3},  {0xca30, 0, LineBreakClass_H2},
    {0xca31, 25, LineBreakClass_H3},  {0xca4c, 0, LineBreakClass_H2},
    {0xca4d, 25, LineBreakClass_H3},  {0xca68, 0, LineBreakClass_H2},
    {0xca69, 25, LineBreakClass_H3},  {0xca84, 0, LineBreakClass_H2},
    {0xca85, 25, LineBreakClass_H3},  {0xcaa0, 0, LineBreakClass_H2},
    {0xcaa1, 25, LineBreakClass_H3},  {0xcabc, 0, LineBreakClass_H2},
    {0xcabd, 25, LineBreakClass_H3},  {0xcad8, 0, LineBreakClass_H2},
    {0xcad9, 25, LineBreakClass_H3},  {0xcaf4, 0, LineBreakClass_H2},
    {0xcaf5, 25, LineBreakClass_H3},  {0xcb10, 0, LineBreakClass_H2},
    {0xcb11, 25, LineBreakClass_H3},  {0xcb2c, 0, LineBreakClass_H2},
    {0xcb2d, 25, LineBreakClass_H3},  {0xcb48, 0, LineBreakClass_H2},
    {0xcb49, 25, LineBreakClass_H3},  {0xcb64, 0, LineBreakClass_H2},
    {0xcb65, 25, LineBreakClass_H3},  {0xcb80, 0, LineBreakClass_H2},
    {0xcb81, 25, LineBreakClass_H3},  {0xcb9c, 0, LineBreakClass_H2},
    {0xcb9d, 25, LineBreakClass_H3},  {0xcbb8, 0, LineBreakClass_H2},
    {0xcbb9, 25, LineBreakClass_H3},  {0xcbd4, 0, LineBreakClass_H2},
    {0xcbd5, 25, LineBreakClass_H3},  {0xcbf0, 0, LineBreakClass_H2},
    {0xcbf1, 25, LineBreakClass_H3},  {0xcc0c, 0, LineBreakClass_H2},
    {0xcc0d, 25, LineBreakClass_H3},  {0xcc28, 0, LineBreakClass_H2},
    {0xcc29, 25, LineBreakClass_H3},  {0xcc44, 0, LineBreakClass_H2},
    {0xcc45, 25, LineBreakClass_H3},  {0xcc60, 0, LineBreakClass_H2},
    {0xcc61, 25, LineBreakClass_H3},  {0xcc7c, 0, LineBreakClass_H2},
    {0xcc7d, 25, LineBreakClass_H3},  {0xcc98, 0, LineBreakClass_H2},
    {0xcc99, 25, LineBreakClass_H3},  {0xccb4, 0, LineBreakClass_H2},
    {0xccb5, 25, LineBreakClass_H3},  {0xccd0, 0, LineBreakClass_H2},
    {0xccd1, 25, LineBreakClass_H3},  {0xccec, 0, LineBreakClass_H2},
    {0xcced, 25, LineBreakClass_H3},  {0xcd08, 0, LineBreakClass_H2},
    {0xcd09, 25, LineBreakClass_H3},  {0xcd24, 0, LineBreakClass_H2},
    {0xcd25, 25, LineBreakClass_H3},  {0xcd40, 0, LineBreakClass_H2},
    {0xcd41, 25, LineBreakClass_H3},  {0xcd5c, 0, LineBreakClass_H2},
    {0xcd5d, 25, LineBreakClass_H3},  {0xcd78, 0, LineBreakClass_H2},
    {0xcd79, 25, LineBreakClass_H3},  {0xcd94, 0, LineBreakClass_H2},
    {0xcd95, 25, LineBreakClass_H3},  {0xcdb0, 0, LineBreakClass_H2},
    {0xcdb1, 25, LineBreakClass_H3},  {0xcdcc, 0, LineBreakClass_H2},
    {0xcdcd, 25, LineBreakClass_H3},  {0xcde8, 0, LineBreakClass_H2},
    {0xcde9, 25, LineBreakClass_H3},  {0xce04, 0, LineBreakClass_H2},
    {0xce05, 25, LineBreakClass_H3},  {0xce20, 0, LineBreakClass_H2},
    {0xce21, 25, LineBreakClass_H3},  {0xce3c, 0, LineBreakClass_H2},
    {0xce3d, 25, LineBreakClass_H3},  {0xce58, 0, LineBreakClass_H2},
    {0xce59, 25, LineBreakClass_H3},  {0xce74, 0, LineBreakClass_H2},
    {0xce75, 25, LineBreakClass_H3},  {0xce90, 0, LineBreakClass_H2},
    {0xce91, 25, LineBreakClass_H3},  {0xceac, 0, LineBreakClass_H2},
    {0xcead, 25, LineBreakClass_H3},  {0xcec8, 0, LineBreakClass_H2},
    {0xcec9, 25, LineBreakClass_H3},  {0xcee4, 0, LineBreakClass_H2},
    {0xcee5, 25, LineBreakClass_H3},  {0xcf00, 0, LineBreakClass_H2},
    {0xcf01, 25, LineBreakClass_H3},  {0xcf1c, 0, LineBreakClass_H2},
    {0xcf1d, 25, LineBreakClass_H3},  {0xcf38, 0, LineBreakClass_H2},
    {0xcf39, 25, LineBreakClass_H3},  {0xcf54, 0, LineBreakClass_H2},
    {0xcf55, 25, LineBreakClass_H3},  {0xcf70, 0, LineBreakClass_H2},
    {0xcf71, 25, LineBreakClass_H3},  {0xcf8c, 0, LineBreakClass_H2},
    {0xcf8d, 25, LineBreakClass_H3},  {0xcfa8, 0, LineBreakClass_H2},
    {0xcfa9, 25, LineBreakClass_H3},  {0xcfc4, 0, LineBreakClass_H2},
    {0xcfc5, 25, LineBreakClass_H3},  {0xcfe0, 0, LineBreakClass_H2},
    {0xcfe1, 25, LineBreakClass_H3},  {0xcffc, 0, LineBreakClass_H2},
    {0xcffd, 25, LineBreakClass_H3},  {0xd018, 0, LineBreakClass_H2},
    {0xd019, 25, LineBreakClass_H3},  {0xd034, 0, LineBreakClass_H2},
    {0xd035, 25, LineBreakClass_H3},  {0xd050, 0, LineBreakClass_H2},
    {0xd051, 25, LineBreakClass_H3},  {0xd06c, 0, LineBreakClass_H2},
    {0xd06d, 25, LineBreakClass_H3},  {0xd088, 0, LineBreakClass_H2},
    {0xd089, 25, LineBreakClass_H3},  {0xd0a4, 0, LineBreakClass_H2},
    {0xd0a5, 25, LineBreakClass_H3},  {0xd0c0, 0, LineBreakClass_H2},
    {0xd0c1, 25, LineBreakClass_H3},  {0xd0dc, 0, LineBreakClass_H2},
    {0xd0dd, 25, LineBreakClass_H3},  {0xd0f8, 0, LineBreakClass_H2},
    {0xd0f9, 25, LineBreakClass_H3},  {0xd114, 0, LineBreakClass_H2},
    {0xd115, 25, LineBreakClass_H3},  {0xd130, 0, LineBreakClass_H2},
    {0xd131, 25, LineBreakClass_H3},  {0xd14c, 0, LineBreakClass_H2},
    {0xd14d, 25, LineBreakClass_H3},  {0xd168, 0, LineBreakClass_H2},
    {0xd169, 25, LineBreakClass_H3},  {0xd184, 0, LineBreakClass_H2},
    {0xd185, 25, LineBreakClass_H3},  {0xd1a0, 0, LineBreakClass_H2},
    {0xd1a1, 25, LineBreakClass_H3},  {0xd1bc, 0, LineBreakClass_H2},
    {0xd1bd, 25, LineBreakClass_H3},  {0xd1d8, 0, LineBreakClass_H2},
    {0xd1d9, 25, LineBreakClass_H3},  {0xd1f4, 0, LineBreakClass_H2},
    {0xd1f5, 25, LineBreakClass_H3},  {0xd210, 0, LineBreakClass_H2},
    {0xd211, 25, LineBreakClass_H3},  {0xd22c, 0, LineBreakClass_H2},
    {0xd22d, 25, LineBreakClass_H3},  {0xd248, 0, LineBreakClass_H2},
    {0xd249, 25, LineBreakClass_H3},  {0xd264, 0, LineBreakClass_H2},
    {0xd265, 25, LineBreakClass_H3},  {0xd280, 0, LineBreakClass_H2},
    {0xd281, 25, LineBreakClass_H3},  {0xd29c, 0, LineBreakClass_H2},
    {0xd29d, 25, LineBreakClass_H3},  {0xd2b8, 0, LineBreakClass_H2},
    {0xd2b9, 25, LineBreakClass_H3},  {0xd2d4, 0, LineBreakClass_H2},
    {0xd2d5, 25, LineBreakClass_H3},  {0xd2f0, 0, LineBreakClass_H2},
    {0xd2f1, 25, LineBreakClass_H3},  {0xd30c, 0, LineBreakClass_H2},
    {0xd30d, 25, LineBreakClass_H3},  {0xd328, 0, LineBreakClass_H2},
    {0xd329, 25, LineBreakClass_H3},  {0xd344, 0, LineBreakClass_H2},
    {0xd345, 25, LineBreakClass_H3},  {0xd360, 0, LineBreakClass_H2},
    {0xd361, 25, LineBreakClass_H3},  {0xd37c, 0, LineBreakClass_H2},
    {0xd37d, 25, LineBreakClass_H3},  {0xd398, 0, LineBreakClass_H2},
    {0xd399, 25, LineBreakClass_H3},  {0xd3b4, 0, LineBreakClass_H2},
    {0xd3b5, 25, LineBreakClass_H3},  {0xd3d0, 0, LineBreakClass_H2},
    {0xd3d1, 25, LineBreakClass_H3},  {0xd3ec, 0, LineBreakClass_H2},
    {0xd3ed, 25, LineBreakClass_H3},  {0xd408, 0, LineBreakClass_H2},
    {0xd409, 25, LineBreakClass_H3},  {0xd424, 0, LineBreakClass_H2},
    {0xd425, 25, LineBreakClass_H3},  {0xd440, 0, LineBreakClass_H2},
    {0xd441, 25, LineBreakClass_H3},  {0xd45c, 0, LineBreakClass_H2},
    {0xd45d, 25, LineBreakClass_H3},  {0xd478, 0, LineBreakClass_H2},
    {0xd479, 25, LineBreakClass_H3},  {0xd494, 0, LineBreakClass_H2},
    {0xd495, 25, LineBreakClass_H3},  {0xd4b0, 0, LineBreakClass_H2},
    {0xd4b1, 25, LineBreakClass_H3},  {0xd4cc, 0, LineBreakClass_H2},
    {0xd4cd, 25, LineBreakClass_H3},  {0xd4e8, 0, LineBreakClass_H2},
    {0xd4e9, 25, LineBreakClass_H3},  {0xd504, 0, LineBreakClass_H2},
    {0xd505, 25, LineBreakClass_H3},  {0xd520, 0, LineBreakClass_H2},
    {0xd521, 25, LineBreakClass_H3},  {0xd53c, 0, LineBreakClass_H2},
    {0xd53d, 25, LineBreakClass_H3},  {0xd558, 0, LineBreakClass_H2},
    {0xd559, 25, LineBreakClass_H3},  {0xd574, 0, LineBreakClass_H2},
    {0xd575, 25, LineBreakClass_H3},  {0xd590, 0, LineBreakClass_H2},
    {0xd591, 25, LineBreakClass_H3},  {0xd5ac, 0, LineBreakClass_H2},
    {0xd5ad, 25, LineBreakClass_H3},  {0xd5c8, 0, LineBreakClass_H2},
    {0xd5c9, 25, LineBreakClass_H3},  {0xd5e4, 0, LineBreakClass_H2},
    {0xd5e5, 25, LineBreakClass_H3},  {0xd600, 0, LineBreakClass_H2},
    {0xd601, 25, LineBreakClass_H3},  {0xd61c, 0, LineBreakClass_H2},
    {0xd61d, 25, LineBreakClass_H3},  {0xd638, 0, LineBreakClass_H2},
    {0xd639, 25, LineBreakClass_H3},  {0xd654, 0, LineBreakClass_H2},
    {0xd655, 25, LineBreakClass_H3},  {0xd670, 0, LineBreakClass_H2},
    {0xd671, 25, LineBreakClass_H3},  {0xd68c, 0, LineBreakClass_H2},
    {0xd68d, 25, LineBreakClass_H3},  {0xd6a8, 0, LineBreakClass_H2},
    {0xd6a9, 25, LineBreakClass_H3},  {0xd6c4, 0, LineBreakClass_H2},
    {0xd6c5, 25, LineBreakClass_H3},  {0xd6e0, 0, LineBreakClass_H2},
    {0xd6e1, 25, LineBreakClass_H3},  {0xd6fc, 0, LineBreakClass_H2},
    {0xd6fd, 25, LineBreakClass_H3},  {0xd718, 0, LineBreakClass_H2},
    {0xd719, 25, LineBreakClass_H3},  {0xd734, 0, LineBreakClass_H2},
    {0xd735, 25, LineBreakClass_H3},  {0xd750, 0, LineBreakClass_H2},
    {0xd751, 25, LineBreakClass_H3},  {0xd76c, 0, LineBreakClass_H2},
    {0xd76d, 25, LineBreakClass_H3},  {0xd788, 0, LineBreakClass_H2},
    {0xd789, 25, LineBreakClass_H3},  {0xd7b0, 22, LineBreakClass_JV},
    {0xd7cb, 48, LineBreakClass_JT},  {0xd800, 255, LineBreakClass_SG},
    {0xd900, 255, LineBreakClass_SG}, {0xda00, 255, LineBreakClass_SG},
    {0xdb00, 126, LineBreakClass_SG}, {0xdb80, 126, LineBreakClass_SG},
    {0xdc00, 255, LineBreakClass_SG}, {0xdd00, 255, LineBreakClass_SG},
    {0xde00, 255, LineBreakClass_SG}, {0xdf00, 254, LineBreakClass_SG},
    {0xfb00, 6, LineBreakClass_AL},   {0xfb13, 4, LineBreakClass_AL},
    {0xfb1d, 0, LineBreakClass_HL},   {0xfb1e, 0, LineBreakClass_CM},
    {0xfb1f, 9, LineBreakClass_HL},   {0xfb29, 0, LineBreakClass_AL},
    {0xfb2a, 12, LineBreakClass_HL},  {0xfb38, 4, LineBreakClass_HL},
    {0xfb3e, 0, LineBreakClass_HL},   {0xfb40, 1, LineBreakClass_HL},
    {0xfb43, 1, LineBreakClass_HL},   {0xfb46, 9, LineBreakClass_HL},
    {0xfb50, 113, LineBreakClass_AL}, {0xfbd3, 255, LineBreakClass_AL},
    {0xfcd3, 106, LineBreakClass_AL}, {0xfd3e, 0, LineBreakClass_OP},
    {0xfd3f, 0, LineBreakClass_CL},   {0xfd50, 63, LineBreakClass_AL},
    {0xfd92, 53, LineBreakClass_AL},  {0xfdf0, 11, LineBreakClass_AL},
    {0xfdfc, 0, LineBreakClass_PO},   {0xfdfd, 0, LineBreakClass_AL},
    {0xfe00, 15, LineBreakClass_CM},  {0xfe10, 0, LineBreakClass_IS},
    {0xfe11, 1, LineBreakClass_CL},   {0xfe13, 1, LineBreakClass_IS},
    {0xfe15, 1, LineBreakClass_EX},   {0xfe17, 0, LineBreakClass_OP},
    {0xfe18, 0, LineBreakClass_CL},   {0xfe19, 0, LineBreakClass_IN},
    {0xfe20, 6, LineBreakClass_CM},   {0xfe35, 0, LineBreakClass_OP},
    {0xfe36, 0, LineBreakClass_CL},   {0xfe37, 0, LineBreakClass_OP},
    {0xfe38, 0, LineBreakClass_CL},   {0xfe39, 0, LineBreakClass_OP},
    {0xfe3a, 0, LineBreakClass_CL},   {0xfe3b, 0, LineBreakClass_OP},
    {0xfe3c, 0, LineBreakClass_CL},   {0xfe3d, 0, LineBreakClass_OP},
    {0xfe3e, 0, LineBreakClass_CL},   {0xfe3f, 0, LineBreakClass_OP},
    {0xfe40, 0, LineBreakClass_CL},   {0xfe41, 0, LineBreakClass_OP},
    {0xfe42, 0, LineBreakClass_CL},   {0xfe43, 0, LineBreakClass_OP},
    {0xfe44, 0, LineBreakClass_CL},   {0xfe47, 0, LineBreakClass_OP},
    {0xfe48, 0, LineBreakClass_CL},   {0xfe50, 0, LineBreakClass_CL},
    {0xfe52, 0, LineBreakClass_CL},   {0xfe54, 1, LineBreakClass_NS},
    {0xfe56, 1, LineBreakClass_EX},   {0xfe59, 0, LineBreakClass_OP},
    {0xfe5a, 0, LineBreakClass_CL},   {0xfe5b, 0, LineBreakClass_OP},
    {0xfe5c, 0, LineBreakClass_CL},   {0xfe5d, 0, LineBreakClass_OP},
    {0xfe5e, 0, LineBreakClass_CL},   {0xfe69, 0, LineBreakClass_PR},
    {0xfe6a, 0, LineBreakClass_PO},   {0xfe70, 4, LineBreakClass_AL},
    {0xfe76, 134, LineBreakClass_AL}, {0xfeff, 0, LineBreakClass_WJ},
    {0xff01, 0, LineBreakClass_EX},   {0xff04, 0, LineBreakClass_PR},
    {0xff05, 0, LineBreakClass_PO},   {0xff08, 0, LineBreakClass_OP},
    {0xff09, 0, LineBreakClass_CL},   {0xff0c, 0, LineBreakClass_CL},
    {0xff0e, 0, LineBreakClass_CL},   {0xff1a, 1, LineBreakClass_NS},
    {0xff1f, 0, LineBreakClass_EX},   {0xff3b, 0, LineBreakClass_OP},
    {0xff3d, 0, LineBreakClass_CL},   {0xff5b, 0, LineBreakClass_OP},
    {0xff5d, 0, LineBreakClass_CL},   {0xff5f, 0, LineBreakClass_OP},
    {0xff60, 1, LineBreakClass_CL},   {0xff62, 0, LineBreakClass_OP},
    {0xff63, 1, LineBreakClass_CL},   {0xff65, 0, LineBreakClass_NS},
    {0xff66, 0, LineBreakClass_AL},   {0xff67, 9, LineBreakClass_CJ},
    {0xff71, 44, LineBreakClass_AL},  {0xff9e, 1, LineBreakClass_NS},
    {0xffa0, 30, LineBreakClass_AL},  {0xffc2, 5, LineBreakClass_AL},
    {0xffca, 5, LineBreakClass_AL},   {0xffd2, 5, LineBreakClass_AL},
    {0xffda, 2, LineBreakClass_AL},   {0xffe0, 0, LineBreakClass_PO},
    {0xffe1, 0, LineBreakClass_PR},   {0xffe5, 1, LineBreakClass_PR},
    {0xffe8, 6, LineBreakClass_AL},   {0xfff9, 2, LineBreakClass_CM},
    {0xfffc, 0, LineBreakClass_CB},   {0xfffd, 0, LineBreakClass_AI},
};

const BreakAction PairTableAlternative[LineBreakClass_PairCount][LineBreakClass_PairCount] = {
    {PRO, PRO, PRO, PRO, PRO, PRO, PRO, PRO, PRO, PRO, PRO, PRO, PRO, PRO, PRO,
     PRO, PRO, PRO, PRO, PRO, PRO, CPB, PRO, PRO, PRO, PRO, PRO, PRO, PRO, PRO},
    {DIR, PRO, PRO, IND, IND, PRO, PRO, PRO, PRO, IND, IND, DIR, DIR, DIR, DIR,
     DIR, IND, IND, DIR, DIR, PRO, CIB, PRO, DIR, DIR, DIR, DIR, DIR, DIR, DIR},
    {DIR, PRO, PRO, IND, IND, PRO, PRO, PRO, PRO, IND, PRO, IND, IND, IND, DIR,
     DIR, IND, IND, DIR, DIR, PRO, CIB, PRO, DIR, DIR, DIR, DIR, DIR, DIR, IND},
    {PRO, PRO, PRO, IND, IND, IND, PRO, PRO, PRO, IND, IND, IND, IND, IND, IND,
     IND, IND, IND, IND, IND, PRO, CIB, PRO, IND, IND, IND, IND, IND, IND, IND},
    {IND, PRO, PRO, IND, IND, IND, PRO, PRO, PRO, IND, IND, IND, IND, IND, IND,
     IND, IND, IND, IND, IND, PRO, CIB, PRO, IND, IND, IND, IND, IND, IND, IND},
    {DIR, PRO, PRO, IND, IND, IND, PRO, PRO, PRO, DIR, DIR, DIR, DIR, DIR, DIR,
     DIR, IND, IND, DIR, DIR, PRO, CIB, PRO, DIR, DIR, DIR, DIR, DIR, DIR, DIR},
    {DIR, PRO, PRO, IND, IND, IND, PRO, PRO, PRO, DIR, DIR, DIR, DIR, DIR, DIR,
     DIR, IND, IND, DIR, DIR, PRO, CIB, PRO, DIR, DIR, DIR, DIR, DIR, DIR, DIR},
    {DIR, PRO, PRO, IND, IND, IND, PRO, PRO, PRO, DIR, DIR, IND, DIR, DIR, DIR,
     DIR, IND, IND, DIR, DIR, PRO, CIB, PRO, DIR, DIR, DIR, DIR, DIR, DIR, DIR},
    {DIR, PRO, PRO, IND, IND, IND, PRO, PRO, PRO, DIR, DIR, IND, IND, IND, DIR,
     DIR, IND, IND, DIR, DIR, PRO, CIB, PRO, DIR, DIR, DIR, DIR, DIR, DIR, IND},
    {IND, PRO, PRO, IND, IND, IND, PRO, PRO, PRO, DIR, DIR, IND, IND, IND, IND,
     DIR, IND, IND, DIR, DIR, PRO, CIB, PRO, IND, IND, IND, IND, IND, DIR, IND},
    {IND, PRO, PRO, IND, IND, IND, PRO, PRO, PRO, DIR, DIR, IND, IND, IND, DIR,
     DIR, IND, IND, DIR, DIR, PRO, CIB, PRO, DIR, DIR, DIR, DIR, DIR, DIR, IND},
    {IND, PRO, PRO, IND, IND, IND, PRO, PRO, PRO, IND, PRO, IND, IND, IND, DIR,
     IND, IND, IND, DIR, DIR, PRO, CIB, PRO, DIR, DIR, DIR, DIR, DIR, DIR, IND},
    {IND, PRO, PRO, IND, IND, IND, PRO, PRO, PRO, DIR, DIR, IND, IND, IND, DIR,
     IND, IND, IND, DIR, DIR, PRO, CIB, PRO, DIR, DIR, DIR, DIR, DIR, DIR, IND},
    {IND, PRO, PRO, IND, IND, IND, PRO, PRO, PRO, DIR, DIR, IND, IND, IND, DIR,
     IND, IND, IND, DIR, DIR, PRO, CIB, PRO, DIR, DIR, DIR, DIR, DIR, DIR, IND},
    {DIR, PRO, PRO, IND, IND, IND, PRO, PRO, PRO, DIR, IND, DIR, DIR, DIR, DIR,
     IND, IND, IND, DIR, DIR, PRO, CIB, PRO, DIR, DIR, DIR, DIR, DIR, DIR, DIR},
    {DIR, PRO, PRO, IND, IND, IND, PRO, PRO, PRO, DIR, DIR, DIR, DIR, DIR, DIR,
     IND, IND, IND, DIR, DIR, PRO, CIB, PRO, DIR, DIR, DIR, DIR, DIR, DIR, DIR},
    {DIR, PRO, PRO, IND, DIR, IND, PRO, PRO, PRO, DIR, DIR, IND, DIR, DIR, DIR,
     DIR, IND, IND, DIR, DIR, PRO, CIB, PRO, DIR, DIR, DIR, DIR, DIR, DIR, DIR},
    {DIR, PRO, PRO, IND, DIR, IND, PRO, PRO, PRO, DIR, DIR, DIR, DIR, DIR, DIR,
     DIR, IND, IND, DIR, DIR, PRO, CIB, PRO, DIR, DIR, DIR, DIR, DIR, DIR, DIR},
    {IND, PRO, PRO, IND, IND, IND, PRO, PRO, PRO, IND, IND, IND, IND, IND, IND,
     IND, IND, IND, IND, IND, PRO, CIB, PRO, IND, IND, IND, IND, IND, IND, IND},
    {DIR, PRO, PRO, IND, IND, IND, PRO, PRO, PRO, DIR, DIR, DIR, DIR, DIR, DIR,
     DIR, IND, IND, DIR, PRO, PRO, CIB, PRO, DIR, DIR, DIR, DIR, DIR, DIR, DIR},
    {DIR, DIR, DIR, DIR, DIR, DIR, DIR, DIR, DIR, DIR, DIR, DIR, DIR, DIR, DIR,
     DIR, DIR, DIR, DIR, DIR, PRO, DIR, DIR, DIR, DIR, DIR, DIR, DIR, DIR, DIR},
    {IND, PRO, PRO, IND, IND, IND, PRO, PRO, PRO, DIR, DIR, IND, IND, IND, DIR,
     IND, IND, IND, DIR, DIR, PRO, CIB, PRO, DIR, DIR, DIR, DIR, DIR, DIR, IND},
    {IND, PRO, PRO, IND, IND, IND, PRO, PRO, PRO, IND, IND, IND, IND, IND, IND,
     IND, IND, IND, IND, IND, PRO, CIB, PRO, IND, IND, IND, IND, IND, IND, IND},
    {DIR, PRO, PRO, IND, IND, IND, PRO, PRO, PRO, DIR, IND, DIR, DIR, DIR, DIR,
     IND, IND, IND, DIR, DIR, PRO, CIB, PRO, DIR, DIR, DIR, IND, IND, DIR, DIR},
    {DIR, PRO, PRO, IND, IND, IND, PRO, PRO, PRO, DIR, IND, DIR, DIR, DIR, DIR,
     IND, IND, IND, DIR, DIR, PRO, CIB, PRO, DIR, DIR, DIR, DIR, IND, DIR, DIR},
    {DIR, PRO, PRO, IND, IND, IND, PRO, PRO, PRO, DIR, IND, DIR, DIR, DIR, DIR,
     IND, IND, IND, DIR, DIR, PRO, CIB, PRO, IND, IND, IND, IND, DIR, DIR, DIR},
    {DIR, PRO, PRO, IND, IND, IND, PRO, PRO, PRO, DIR, IND, DIR, DIR, DIR, DIR,
     IND, IND, IND, DIR, DIR, PRO, CIB, PRO, DIR, DIR, DIR, IND, IND, DIR, DIR},
    {DIR, PRO, PRO, IND, IND, IND, PRO, PRO, PRO, DIR, IND, DIR, DIR, DIR, DIR,
     IND, IND, IND, DIR, DIR, PRO, CIB, PRO, DIR, DIR, DIR, DIR, IND, DIR, DIR},
    {DIR, PRO, PRO, IND, IND, IND, PRO, PRO, PRO, DIR, DIR, DIR, DIR, DIR, DIR,
     DIR, IND, IND, DIR, DIR, PRO, CIB, PRO, DIR, DIR, DIR, DIR, DIR, IND, DIR},
    {IND, PRO, PRO, PRO, IND, IND, PRO, PRO, PRO, DIR, DIR, IND, IND, IND, DIR,
     IND, IND, IND, DIR, DIR, PRO, CIB, PRO, DIR, DIR, DIR, DIR, DIR, DIR, PRO},
};

const BreakAction PairTableDefault[LineBreakClass_PairCount][LineBreakClass_PairCount] = {
    {PRO, PRO, PRO, PRO, PRO, PRO, PRO, PRO, PRO, PRO, PRO, PRO, PRO, PRO, PRO,
     PRO, PRO, PRO, PRO, PRO, PRO, CPB, PRO, PRO, PRO, PRO, PRO, PRO, PRO, PRO},
    {DIR, PRO, PRO, IND, IND, PRO, PRO, PRO, PRO, IND, IND, DIR, DIR, DIR, DIR,
     DIR, IND, IND, DIR, DIR, PRO, CIB, PRO, DIR, DIR, DIR, DIR, DIR, DIR, DIR},
    {DIR, PRO, PRO, IND, IND, PRO, PRO, PRO, PRO, IND, IND, IND, IND, IND, DIR,
     DIR, IND, IND, DIR, DIR, PRO, CIB, PRO, DIR, DIR, DIR, DIR, DIR, DIR, IND},
    {PRO, PRO, PRO, IND, IND, IND, PRO, PRO, PRO, IND, IND, IND, IND, IND, IND,
     IND, IND, IND, IND, IND, PRO, CIB, PRO, IND, IND, IND, IND, IND, IND, IND},
    {IND, PRO, PRO, IND, IND, IND, PRO, PRO, PRO, IND, IND, IND, IND, IND, IND,
     IND, IND, IND, IND, IND, PRO, CIB, PRO, IND, IND, IND, IND, IND, IND, IND},
    {DIR, PRO, PRO, IND, IND, IND, PRO, PRO, PRO, DIR, DIR, DIR, DIR, DIR, DIR,
     DIR, IND, IND, DIR, DIR, PRO, CIB, PRO, DIR, DIR, DIR, DIR, DIR, DIR, DIR},
    {DIR, PRO, PRO, IND, IND, IND, PRO, PRO, PRO, DIR, DIR, DIR, DIR, DIR, DIR,
     DIR, IND, IND, DIR, DIR, PRO, CIB, PRO, DIR, DIR, DIR, DIR, DIR, DIR, DIR},
    {DIR, PRO, PRO, IND, IND, IND, PRO, PRO, PRO, DIR, DIR, IND, DIR, DIR, DIR,
     DIR, IND, IND, DIR, DIR, PRO, CIB, PRO, DIR, DIR, DIR, DIR, DIR, DIR, DIR},
    {DIR, PRO, PRO, IND, IND, IND, PRO, PRO, PRO, DIR, DIR, IND, IND, IND, DIR,
     DIR, IND, IND, DIR, DIR, PRO, CIB, PRO, DIR, DIR, DIR, DIR, DIR, DIR, IND},
    {IND, PRO, PRO, IND, IND, IND, PRO, PRO, PRO, DIR, DIR, IND, IND, IND, IND,
     DIR, IND, IND, DIR, DIR, PRO, CIB, PRO, IND, IND, IND, IND, IND, DIR, IND},
    {IND, PRO, PRO, IND, IND, IND, PRO, PRO, PRO, DIR, DIR, IND, IND, IND, DIR,
     DIR, IND, IND, DIR, DIR, PRO, CIB, PRO, DIR, DIR, DIR, DIR, DIR, DIR, IND},
    {IND, PRO, PRO, IND, IND, IND, PRO, PRO, PRO, IND, IND, IND, IND, IND, DIR,
     IND, IND, IND, DIR, DIR, PRO, CIB, PRO, DIR, DIR, DIR, DIR, DIR, DIR, IND},
    {IND, PRO, PRO, IND, IND, IND, PRO, PRO, PRO, DIR, DIR, IND, IND, IND, DIR,
     IND, IND, IND, DIR, DIR, PRO, CIB, PRO, DIR, DIR, DIR, DIR, DIR, DIR, IND},
    {IND, PRO, PRO, IND, IND, IND, PRO, PRO, PRO, DIR, DIR, IND, IND, IND, DIR,
     IND, IND, IND, DIR, DIR, PRO, CIB, PRO, DIR, DIR, DIR, DIR, DIR, DIR, IND},
    {DIR, PRO, PRO, IND, IND, IND, PRO, PRO, PRO, DIR, IND, DIR, DIR, DIR, DIR,
     IND, IND, IND, DIR, DIR, PRO, CIB, PRO, DIR, DIR, DIR, DIR, DIR, DIR, DIR},
    {DIR, PRO, PRO, IND, IND, IND, PRO, PRO, PRO, DIR, DIR, DIR, DIR, DIR, DIR,
     IND, IND, IND, DIR, DIR, PRO, CIB, PRO, DIR, DIR, DIR, DIR, DIR, DIR, DIR},
    {DIR, PRO, PRO, IND, DIR, IND, PRO, PRO, PRO, DIR, DIR, IND, DIR, DIR, DIR,
     DIR, IND, IND, DIR, DIR, PRO, CIB, PRO, DIR, DIR, DIR, DIR, DIR, DIR, DIR},
    {DIR, PRO, PRO, IND, DIR, IND, PRO, PRO, PRO, DIR, DIR, DIR, DIR, DIR, DIR,
     DIR, IND, IND, DIR, DIR, PRO, CIB, PRO, DIR, DIR, DIR, DIR, DIR, DIR, DIR},
    {IND, PRO, PRO, IND, IND, IND, PRO, PRO, PRO, IND, IND, IND, IND, IND, IND,
     IND, IND, IND, IND, IND, PRO, CIB, PRO, IND, IND, IND, IND, IND, IND, IND},
    {DIR, PRO, PRO, IND, IND, IND, PRO, PRO, PRO, DIR, DIR, DIR, DIR, DIR, DIR,
     DIR, IND, IND, DIR, PRO, PRO, CIB, PRO, DIR, DIR, DIR, DIR, DIR, DIR, DIR},
    {DIR, DIR, DIR, DIR, DIR, DIR, DIR, DIR, DIR, DIR, DIR, DIR, DIR, DIR, DIR,
     DIR, DIR, DIR, DIR, DIR, PRO, DIR, DIR, DIR, DIR, DIR, DIR, DIR, DIR, DIR},
    {IND, PRO, PRO, IND, IND, IND, PRO, PRO, PRO, DIR, DIR, IND, IND, IND, DIR,
     IND, IND, IND, DIR, DIR, PRO, CIB, PRO, DIR, DIR, DIR, DIR, DIR, DIR, IND},
    {IND, PRO, PRO, IND, IND, IND, PRO, PRO, PRO, IND, IND, IND, IND, IND, IND,
     IND, IND, IND, IND, IND, PRO, CIB, PRO, IND, IND, IND, IND, IND, IND, IND},
    {DIR, PRO, PRO, IND, IND, IND, PRO, PRO, PRO, DIR, IND, DIR, DIR, DIR, DIR,
     IND, IND, IND, DIR, DIR, PRO, CIB, PRO, DIR, DIR, DIR, IND, IND, DIR, DIR},
    {DIR, PRO, PRO, IND, IND, IND, PRO, PRO, PRO, DIR, IND, DIR, DIR, DIR, DIR,
     IND, IND, IND, DIR, DIR, PRO, CIB, PRO, DIR, DIR, DIR, DIR, IND, DIR, DIR},
    {DIR, PRO, PRO, IND, IND, IND, PRO, PRO, PRO, DIR, IND, DIR, DIR, DIR, DIR,
     IND, IND, IND, DIR, DIR, PRO, CIB, PRO, IND, IND, IND, IND, DIR, DIR, DIR},
    {DIR, PRO, PRO, IND, IND, IND, PRO, PRO, PRO, DIR, IND, DIR, DIR, DIR, DIR,
     IND, IND, IND, DIR, DIR, PRO, CIB, PRO, DIR, DIR, DIR, IND, IND, DIR, DIR},
    {DIR, PRO, PRO, IND, IND, IND, PRO, PRO, PRO, DIR, IND, DIR, DIR, DIR, DIR,
     IND, IND, IND, DIR, DIR, PRO, CIB, PRO, DIR, DIR, DIR, DIR, IND, DIR, DIR},
    {DIR, PRO, PRO, IND, IND, IND, PRO, PRO, PRO, DIR, DIR, DIR, DIR, DIR, DIR,
     DIR, IND, IND, DIR, DIR, PRO, CIB, PRO, DIR, DIR, DIR, DIR, DIR, IND, DIR},
    {IND, PRO, PRO, PRO, IND, IND, PRO, PRO, PRO, DIR, DIR, IND, IND, IND, DIR,
     IND, IND, IND, DIR, DIR, PRO, CIB, PRO, DIR, DIR, DIR, DIR, DIR, DIR, PRO},
};

const int LineBreakRangeCount = sizeof(LineBreakRanges) / sizeof(LineBreakRanges[0]);

LineBreakClass GetLineBreakClass(uint32_t code) {
    int low = 0;
    int high = LineBreakRangeCount;
    do {
        const int diff = high - low;
        const int mid = low + diff / 2;
        const LineBreakRange& rRange = LineBreakRanges[mid];
        if (static_cast<uint32_t>(diff + 1) <= 2) {
            if (code <= rRange.first + rRange.length) {
                return static_cast<LineBreakClass>(rRange.lineBreakClass);
            }

            break;
        }

        if (code < rRange.first) {
            high = mid;
        } else {
            low = mid;
        }
    } while (high > low);
    return LineBreakClass_ID;
}

LineBreakClass ResolveLineBreakClass(LineBreakClass lineBreakClass) {
    if (lineBreakClass >= LineBreakClass_CR && lineBreakClass <= LineBreakClass_NL) {
        lineBreakClass = LineBreakClass_BK;
    }

    if (lineBreakClass == LineBreakClass_CJ) {
        lineBreakClass = LineBreakClass_NS;
    }

    if (lineBreakClass == LineBreakClass_AI) {
        lineBreakClass = LineBreakClass_AL;
    }

    if (lineBreakClass == LineBreakClass_SA) {
        lineBreakClass = LineBreakClass_AL;
    }

    if (lineBreakClass == LineBreakClass_SG) {
        lineBreakClass = LineBreakClass_AL;
    }

    if (lineBreakClass == LineBreakClass_CB) {
        lineBreakClass = LineBreakClass_ID;
    }

    return lineBreakClass;
}

bool IsSpace(uint32_t code) {
    return GetLineBreakClass(code) == LineBreakClass_SP;
}

uint32_t PeekChar(const uint16_t* pPos) {
    return *pPos;
}

uint32_t PeekChar(const char* pPos) {
    uint32_t code;
    char buffer[4];
    nn::util::PickOutCharacterFromUtf8String(buffer, &pPos);
    code = 0;
    nn::util::ConvertCharacterUtf8ToUtf32(&code, buffer);
    return code;
}

void StepChar(const uint16_t** ppPos) {
    (*ppPos)++;
}

void StepChar(const char** ppPos) {
    char buffer[4];
    nn::util::PickOutCharacterFromUtf8String(buffer, ppPos);
}

uint32_t ToCode(uint16_t c) {
    return c;
}

uint32_t ToCode(char c) {
    return static_cast<uint8_t>(c);
}

template <typename CharType>
__attribute__((always_inline)) LineBreakClass
ReadLineBreakClass(const CharType** ppCharPos, const CharType** ppPos, const CharType* pEnd,
                   WordWrapCallbackBase<CharType>& rCallback, size_t* pSkipCount) {
    uint32_t code;
    for (;;) {
        code = PeekChar(*ppPos);
        if (code >= 0x20) {
            break;
        }

        const size_t tagSize = rCallback.GetTagSize(pSkipCount, *ppPos, pEnd);
        if (tagSize == 0) {
            break;
        }

        *ppPos =
            reinterpret_cast<const CharType*>(reinterpret_cast<const uint8_t*>(*ppPos) + tagSize);
        if (*ppPos == pEnd) {
            *ppCharPos = *ppPos - 1;
            return ResolveLineBreakClass(GetLineBreakClass(0));
        }
    }

    *ppCharPos = *ppPos;
    StepChar(ppPos);
    if (*pSkipCount != 0) {
        (*pSkipCount)--;
        return LineBreakClass_GL;
    }

    return ResolveLineBreakClass(GetLineBreakClass(code));
}

template <typename CharType>
const CharType* FindLineBreakImpl(const CharType* pStart, const CharType* pEnd,
                                  WordWrapCallbackBase<CharType>& rCallback,
                                  const WordWrapConfig& rConfig) {
    if (pStart == pEnd) {
        return pStart;
    }

    size_t skipCount = 0;
    const CharType* pPos = pStart;
    const CharType* pCharPos;
    LineBreakClass prevClass = ReadLineBreakClass(&pCharPos, &pPos, pEnd, rCallback, &skipCount);
    if (prevClass == LineBreakClass_BK) {
        return pCharPos;
    }

    if (prevClass == LineBreakClass_SP) {
        prevClass = LineBreakClass_WJ;
    }

    const CharType* pLimit = rCallback.GetLineBreakLimit(pStart, pEnd) - 1;
    const CharType* pResult = pLimit > pStart ? pLimit : pStart;
    const CharType* pLimitPos = pResult + 1;
    const CharType* pBreak = pEnd;
    const CharType* pPrevPos = pCharPos;
    bool isSpace = false;
    while (pPos != pEnd) {
        LineBreakClass lineBreakClass =
            ReadLineBreakClass(&pCharPos, &pPos, pEnd, rCallback, &skipCount);
        if (lineBreakClass == LineBreakClass_SP) {
            isSpace = true;
            pPrevPos = pCharPos;
            continue;
        }

        bool isMandatory = false;
        bool isDirect = false;
        bool canBreak = true;
        if (lineBreakClass == LineBreakClass_BK) {
            isMandatory = true;
        } else {
            if (lineBreakClass == LineBreakClass_CY) {
                if (rConfig.isCyrillicBreakEnabled &&
                    (isSpace || prevClass == LineBreakClass_OP || prevClass == LineBreakClass_QU ||
                     prevClass == LineBreakClass_SY || prevClass == LineBreakClass_SP)) {
                    lineBreakClass = LineBreakClass_CY;
                } else {
                    lineBreakClass = LineBreakClass_AL;
                }
            }

            const BreakAction action = rConfig.isAlternativePairTableUsed ?
                                           PairTableAlternative[prevClass][lineBreakClass] :
                                           PairTableDefault[prevClass][lineBreakClass];
            if (action == IND || action == CIB) {
                canBreak = isSpace;
            } else if (action == DIR) {
                isDirect = true;
            } else {
                canBreak = false;
            }
        }

        if (canBreak) {
            if (pCharPos >= pLimitPos) {
                if (isSpace && isDirect) {
                    return pResult;
                }

                return pPrevPos == pEnd ? pResult : pPrevPos;
            }

            if (isMandatory) {
                return pCharPos;
            }

            pBreak = pPrevPos;
        } else if (pCharPos >= pLimitPos) {
            return pBreak == pEnd ? pResult : pBreak;
        }

        isSpace = false;
        prevClass = lineBreakClass;
        pPrevPos = pCharPos;
    }

    if (pLimitPos > pEnd) {
        return pCharPos;
    }

    if (isSpace) {
        return pResult;
    }

    return pBreak == pEnd ? pResult : pBreak;
}

template <typename CharType>
bool CalculateWordWrappingImpl(uint32_t* pOutLength, CharType* pDst, uint32_t dstSize,
                               const CharType* pSrc, uint32_t srcLength,
                               WordWrapCallbackBase<CharType>& rCallback,
                               const WordWrapConfig& rConfig) {
    if (pOutLength != nullptr) {
        *pOutLength = 0;
    }

    if (pSrc == nullptr || pDst == nullptr || dstSize == 0) {
        return false;
    }

    pDst[0] = 0;
    if (srcLength == 0 || pSrc[0] == 0) {
        return true;
    }

    const int maxLineCount = rConfig.maxLineCount;
    const CharType* pEnd = pSrc + srcLength;
    int dstPos = 0;
    int lineCount = 0;
    for (;;) {
        const CharType* pBreak = FindLineBreakImpl(pSrc, pEnd, rCallback, rConfig);
        const int lineLength = pBreak - pSrc;
        int breakLength = lineLength + 1;
        const int remain = dstSize - dstPos - 1;
        int copyLength = remain < breakLength ? remain : lineLength + 1;
        const bool isSpace = IsSpace(ToCode((pSrc + copyLength)[-1]));
        copyLength -= isSpace;
        if (isSpace) {
            breakLength = lineLength;
        }

        std::memcpy(pDst + dstPos, pSrc, copyLength * sizeof(CharType));
        dstPos += copyLength;
        pDst[dstPos] = 0;
        if (breakLength != copyLength) {
            if (pOutLength != nullptr) {
                *pOutLength = dstPos;
            }

            return false;
        }

        pSrc = pBreak;
        StepChar(&pSrc);
        if (pSrc == pEnd) {
            if (pOutLength != nullptr) {
                *pOutLength = dstPos;
            }

            return true;
        }

        lineCount++;
        if (maxLineCount > 0 && lineCount >= rConfig.maxLineCount) {
            if (pOutLength != nullptr) {
                *pOutLength = dstPos;
            }

            return false;
        }

        if (*pBreak != '\n') {
            if (static_cast<uint32_t>(dstPos + 1) >= dstSize) {
                if (pOutLength != nullptr) {
                    *pOutLength = dstPos;
                }

                return false;
            }

            pDst[dstPos] = '\n';
            dstPos++;
            pDst[dstPos] = 0;
        }

        if (rConfig.isLeadingSpaceRemoved) {
            while (IsSpace(PeekChar(pSrc))) {
                StepChar(&pSrc);
                if (pSrc == pEnd) {
                    dstPos--;
                    pDst[dstPos] = 0;
                    if (pOutLength != nullptr) {
                        *pOutLength = dstPos;
                    }

                    return true;
                }
            }
        }
    }
}

}  // namespace

/**
 * Constructs a callback measuring lines with a copy of a text writer.
 * @param pWriter text writer to copy
 */
template <typename CharType>
DefaultWordWrapCallbackBase<CharType>::DefaultWordWrapCallbackBase(
    const TextWriterBase<CharType>* pWriter)
    : m_Writer(*pWriter), m_pUpdatedPos(nullptr) {}

/**
 * Destroys the callback.
 */
template <typename CharType>
DefaultWordWrapCallbackBase<CharType>::~DefaultWordWrapCallbackBase() = default;

/**
 * Gets the position where a line starting at a position exceeds the width limit.
 * @param pStart start of the line
 * @param pEnd end of the text
 * @return the first position that does not fit in the line
 */
template <typename CharType>
const CharType* DefaultWordWrapCallbackBase<CharType>::GetLineBreakLimit(const CharType* pStart,
                                                                         const CharType* pEnd) {
    if (m_pUpdatedPos == nullptr) {
        m_pUpdatedPos = pStart;
    } else if (m_pUpdatedPos != pStart) {
        const CharType* pUpdatedPos = m_pUpdatedPos;
        const int length = pStart - pUpdatedPos;
        m_pUpdatedPos = pStart;
        m_Writer.UpdateTextWriterWithTags(pUpdatedPos, length);
    }

    return m_Writer.FindPosOfWidthLimit(pStart, pEnd - pStart);
}

/**
 * Finds the position where a UTF-16 text should be broken.
 * @param pStart start of the text
 * @param pEnd end of the text
 * @param rCallback callback giving the width limit and tag sizes
 * @param rConfig word wrapping configuration
 * @return the position of the last character on the line
 */
const uint16_t* WordWrapping::FindLineBreak(const uint16_t* pStart, const uint16_t* pEnd,
                                            WordWrapCallbackBase<uint16_t>& rCallback,
                                            const WordWrapConfig& rConfig) {
    return FindLineBreakImpl(pStart, pEnd, rCallback, rConfig);
}

/**
 * Finds the position where a UTF-8 text should be broken.
 * @param pStart start of the text
 * @param pEnd end of the text
 * @param rCallback callback giving the width limit and tag sizes
 * @param rConfig word wrapping configuration
 * @return the position of the last character on the line
 */
const char* WordWrapping::FindLineBreakUtf8(const char* pStart, const char* pEnd,
                                            WordWrapCallbackBase<char>& rCallback,
                                            const WordWrapConfig& rConfig) {
    return FindLineBreakImpl(pStart, pEnd, rCallback, rConfig);
}

/**
 * Copies a UTF-16 text inserting line breaks where it has to be wrapped.
 * @param pOutLength set to the length of the output
 * @param pDst output buffer
 * @param dstSize size of the output buffer in characters
 * @param pSrc input text
 * @param srcLength length of the input text
 * @param rCallback callback giving the width limit and tag sizes
 * @param rConfig word wrapping configuration
 * @return whether the whole text fit in the buffer and line count
 */
bool WordWrapping::CalculateWordWrapping(uint32_t* pOutLength, uint16_t* pDst, uint32_t dstSize,
                                         const uint16_t* pSrc, uint32_t srcLength,
                                         WordWrapCallbackBase<uint16_t>& rCallback,
                                         const WordWrapConfig& rConfig) {
    return CalculateWordWrappingImpl(pOutLength, pDst, dstSize, pSrc, srcLength, rCallback,
                                     rConfig);
}

/**
 * Copies a UTF-8 text inserting line breaks where it has to be wrapped.
 * @param pOutLength set to the length of the output
 * @param pDst output buffer
 * @param dstSize size of the output buffer in bytes
 * @param pSrc input text
 * @param srcLength length of the input text in bytes
 * @param rCallback callback giving the width limit and tag sizes
 * @param rConfig word wrapping configuration
 * @return whether the whole text fit in the buffer and line count
 */
bool WordWrapping::CalculateWordWrappingUtf8(uint32_t* pOutLength, char* pDst, uint32_t dstSize,
                                             const char* pSrc, uint32_t srcLength,
                                             WordWrapCallbackBase<char>& rCallback,
                                             const WordWrapConfig& rConfig) {
    return CalculateWordWrappingImpl(pOutLength, pDst, dstSize, pSrc, srcLength, rCallback,
                                     rConfig);
}

template class DefaultWordWrapCallbackBase<uint16_t>;
template class DefaultWordWrapCallbackBase<char>;

}  // namespace font
}  // namespace nn
