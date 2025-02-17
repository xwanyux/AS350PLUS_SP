#include "POSAPI.h"

//#pragma memory=constseg(P3_XCONST)

//----------------------------------------------------------------------------
const UCHAR CODE_TABLE_01[] = {
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 00 ' '
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 01 ' '
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 02 ' '
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 03 ' '
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 04 ' '
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 05 ' '
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 06 ' '
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 07 ' '
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 08 ' '
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 09 ' '
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 0A ' '
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 0B ' '
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 0C ' '
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 0D ' '
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 0E ' '
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 0F ' '
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 10 ' '
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 11 ' '
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 12 ' '
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 13 ' '
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 14 ' '
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 15 ' '
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 16 ' '
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 17 ' '
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 18 ' '
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 19 ' '
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 1A ' '
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 1B ' '
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 1C ' '
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 1D ' '
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 1E ' '
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 1F ' '

                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 20 ' '
                              0x00, 0x00, 0x4F, 0x00, 0x00, 0x00,  // 21 '!'
                              0x00, 0x07, 0x00, 0x07, 0x00, 0x00,  // 22 '"'
                              0x14, 0x7F, 0x14, 0x7F, 0x14, 0x00,  // 23 '#'
                              0x24, 0x2A, 0x7F, 0x2A, 0x12, 0x00,  // 24 '$'
                              0x23, 0x13, 0x08, 0x64, 0x62, 0x00,  // 25 '%'
                              0x36, 0x49, 0x55, 0x22, 0x40, 0x00,  // 26 '&'
                              0x00, 0x05, 0x03, 0x00, 0x00, 0x00,  // 27 '''
                              0x00, 0x1C, 0x22, 0x41, 0x00, 0x00,  // 28 '('
                              0x00, 0x41, 0x22, 0x1C, 0x00, 0x00,  // 29 ')'
                              0x14, 0x08, 0x3E, 0x08, 0x14, 0x00,  // 2A '*'
                              0x08, 0x08, 0x3E, 0x08, 0x08, 0x00,  // 2B '+'
                              0x00, 0x50, 0x30, 0x00, 0x00, 0x00,  // 2C ','
                              0x08, 0x08, 0x08, 0x08, 0x08, 0x00,  // 2D '-'
                              0x00, 0x60, 0x60, 0x00, 0x00, 0x00,  // 2E '.'
                              0x20, 0x10, 0x08, 0x04, 0x02, 0x00,  // 2F '/'
                              0x3E, 0x51, 0x49, 0x45, 0x3E, 0x00,  // 30 '0'
                              0x00, 0x42, 0x7F, 0x40, 0x00, 0x00,  // 31 '1'
                              0x42, 0x61, 0x51, 0x49, 0x46, 0x00,  // 32 '2'
                              0x21, 0x41, 0x45, 0x4B, 0x31, 0x00,  // 33 '3'
                              0x18, 0x14, 0x12, 0x7F, 0x10, 0x00,  // 34 '4'
                              0x27, 0x45, 0x45, 0x45, 0x39, 0x00,  // 35 '5'
                              0x3C, 0x4A, 0x49, 0x49, 0x30, 0x00,  // 36 '6'
                              0x01, 0x71, 0x09, 0x05, 0x03, 0x00,  // 37 '7'
                              0x36, 0x49, 0x49, 0x49, 0x36, 0x00,  // 38 '8'
                              0x06, 0x49, 0x49, 0x29, 0x1E, 0x00,  // 39 '9'
                              0x00, 0x36, 0x36, 0x00, 0x00, 0x00,  // 3A ':'
                              0x00, 0x56, 0x36, 0x00, 0x00, 0x00,  // 3B ';'
                              0x08, 0x14, 0x22, 0x41, 0x00, 0x00,  // 3C '<'
                              0x14, 0x14, 0x14, 0x14, 0x14, 0x00,  // 3D '='
                              0x41, 0x22, 0x14, 0x08, 0x00, 0x00,  // 3E '>'
                              0x02, 0x01, 0x51, 0x09, 0x06, 0x00,  // 3F '?'
                              0x32, 0x49, 0x79, 0x41, 0x3E, 0x00,  // 40 '@'
                              0x7E, 0x11, 0x11, 0x11, 0x7E, 0x00,  // 41 'A'
                              0x7F, 0x49, 0x49, 0x49, 0x36, 0x00,  // 42 'B'
                              0x3E, 0x41, 0x41, 0x41, 0x22, 0x00,  // 43 'C'
                              0x7F, 0x41, 0x41, 0x22, 0x1C, 0x00,  // 44 'D'
                              0x7F, 0x49, 0x49, 0x49, 0x41, 0x00,  // 45 'E'
                              0x7F, 0x09, 0x09, 0x09, 0x01, 0x00,  // 46 'F'
                              0x3E, 0x41, 0x49, 0x49, 0x7A, 0x00,  // 47 'G'
                              0x7F, 0x08, 0x08, 0x08, 0x7F, 0x00,  // 48 'H'
                              0x00, 0x41, 0x7F, 0x41, 0x00, 0x00,  // 49 'I'
                              0x20, 0x40, 0x41, 0x3F, 0x01, 0x00,  // 4A 'J'
                              0x7F, 0x08, 0x14, 0x22, 0x41, 0x00,  // 4B 'K'
                              0x7F, 0x40, 0x40, 0x40, 0x40, 0x00,  // 4C 'L'
                              0x7F, 0x02, 0x0C, 0x02, 0x7F, 0x00,  // 4D 'M'
                              0x7F, 0x04, 0x08, 0x10, 0x7F, 0x00,  // 4E 'N'
                              0x3E, 0x41, 0x41, 0x41, 0x3E, 0x00,  // 4F 'O'
                              0x7F, 0x11, 0x11, 0x11, 0x0E, 0x00,  // 50 'P'
                              0x3E, 0x41, 0x51, 0x21, 0x5E, 0x00,  // 51 'Q'
                              0x7F, 0x09, 0x19, 0x29, 0x46, 0x00,  // 52 'R'
                              0x46, 0x49, 0x49, 0x49, 0x31, 0x00,  // 53 'S'
                              0x01, 0x01, 0x7F, 0x01, 0x01, 0x00,  // 54 'T'
                              0x3F, 0x40, 0x40, 0x40, 0x3F, 0x00,  // 55 'U'
                              0x1F, 0x20, 0x40, 0x20, 0x1F, 0x00,  // 56 'V'
                              0x3F, 0x40, 0x38, 0x40, 0x3F, 0x00,  // 57 'W'
                              0x63, 0x14, 0x08, 0x14, 0x63, 0x00,  // 58 'X'
                              0x07, 0x08, 0x70, 0x08, 0x07, 0x00,  // 59 'Y'
                              0x61, 0x51, 0x49, 0x45, 0x43, 0x00,  // 5A 'Z'
                              0x00, 0x7F, 0x41, 0x41, 0x00, 0x00,  // 5B '['
                              0x02, 0x04, 0x08, 0x10, 0x20, 0x00,  // 5C '\'
                              0x00, 0x41, 0x41, 0x7F, 0x00, 0x00,  // 5D ']'
                              0x04, 0x02, 0x01, 0x02, 0x04, 0x00,  // 5E '^'
                              0x40, 0x40, 0x40, 0x40, 0x40, 0x00,  // 5F '_'
                              0x00, 0x01, 0x02, 0x04, 0x00, 0x00,  // 60 '`'
                              0x38, 0x44, 0x44, 0x44, 0x7C, 0x00,  // 61 'a'
                              0x7F, 0x44, 0x44, 0x44, 0x38, 0x00,  // 62 'b'
                              0x38, 0x44, 0x44, 0x44, 0x44, 0x00,  // 63 'c'
                              0x38, 0x44, 0x44, 0x44, 0x7F, 0x00,  // 64 'd'
                              0x38, 0x54, 0x54, 0x54, 0x58, 0x00,  // 65 'e'
                              0x04, 0x7E, 0x05, 0x05, 0x01, 0x00,  // 66 'f'
                              0x18, 0xA4, 0xA4, 0xA4, 0x7C, 0x00,  // 67 'g'
                              0x7F, 0x04, 0x04, 0x04, 0x78, 0x00,  // 68 'h'
                              0x00, 0x00, 0x7D, 0x00, 0x00, 0x00,  // 69 'i'
                              0x00, 0x80, 0x80, 0x7D, 0x00, 0x00,  // 6A 'j'
                              0x7F, 0x10, 0x28, 0x44, 0x00, 0x00,  // 6B 'k'
                              0x00, 0x00, 0x7F, 0x00, 0x00, 0x00,  // 6C 'l'
                              0x7C, 0x04, 0x7C, 0x04, 0x78, 0x00,  // 6D 'm'
                              0x7C, 0x04, 0x04, 0x04, 0x78, 0x00,  // 6E 'n'
                              0x38, 0x44, 0x44, 0x44, 0x38, 0x00,  // 6F 'o'
                              0xFC, 0x24, 0x24, 0x24, 0x18, 0x00,  // 70 'p'
                              0x18, 0x24, 0x24, 0x24, 0xFC, 0x00,  // 71 'q'
                              0x00, 0x7C, 0x08, 0x04, 0x00, 0x00,  // 72 'r'
                              0x48, 0x54, 0x54, 0x24, 0x00, 0x00,  // 73 's'
                              0x00, 0x04, 0x3E, 0x44, 0x00, 0x00,  // 74 't'
                              0x3C, 0x40, 0x40, 0x40, 0x7C, 0x00,  // 75 'u'
                              0x04, 0x18, 0x60, 0x18, 0x04, 0x00,  // 76 'v'
                              0x1C, 0x60, 0x1C, 0x60, 0x1C, 0x00,  // 77 'w'
                              0x44, 0x28, 0x10, 0x28, 0x44, 0x00,  // 78 'x'
                              0x1C, 0xA0, 0xA0, 0xA0, 0x7C, 0x00,  // 79 'y'
                              0x44, 0x64, 0x54, 0x4C, 0x44, 0x00,  // 7A 'z'
                              0x08, 0x36, 0x41, 0x41, 0x00, 0x00,  // 7B '{'
                              0x00, 0x00, 0x77, 0x00, 0x00, 0x00,  // 7C '|'
                              0x00, 0x41, 0x41, 0x36, 0x08, 0x00,  // 7D '}'
                              0x08, 0x04, 0x1C, 0x10, 0x08, 0x00,  // 7E '~'
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 7F (unused)

                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 80 (unused)
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 81 (unused)
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 82 (unused)
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 83 (unused)
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 84 (unused)
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 85 (unused)
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 86 (unused)
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 87 (unused)
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 88 (unused)
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 89 (unused)
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 8A (unused)
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 8B (unused)
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 8C (unused)
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 8D (unused)
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 8E (unused)
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 8F (unused)

//                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 90 (unused)
//                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 91 (unused)
//                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 92 (unused)
//                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 93 (unused)
//                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 94 (unused)
//                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 95 (unused)
//                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 96 (unused)
//                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 97 (unused)
//                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 98 (unused)
//                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 99 (unused)
//                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 9A (unused)
//                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 9B (unused)
//                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 9C (unused)
//                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 9D (unused)
//                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 9E (unused)
//                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 9F (unused)

                              0x00, 0x3E, 0x1C, 0x08, 0x00, 0x00,  // 90 Right arrow
                              0x00, 0x08, 0x1C, 0x3E, 0x00, 0x00,  // 91 Left arrow
                              0x14, 0x36, 0x77, 0x36, 0x14, 0x00,  // 92 Up/Down arrow
                              0x10, 0x18, 0x1C, 0x18, 0x10, 0x00,  // 93 Up arrow
                              0x04, 0x0C, 0x1C, 0x0C, 0x04, 0x00,  // 94 Down arrow
                              0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,  // 95 Black brick
                              0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA,  // 96 Gray brick
                              0x00, 0x24, 0x66, 0xFF, 0x66, 0x24,  // 97 On-line
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 98 Off-line
                              0x3E, 0x7F, 0x7F, 0x7F, 0x3E, 0x00,  // 99 Black 'O'
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 9A (RFU)
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 9B (RFU)
                              0x48, 0x7E, 0x49, 0x41, 0x42, 0x00,  // 9C Stering
                              0x15, 0x16, 0x7C, 0x16, 0x15, 0x00,  // 9D RMB
                              0x09, 0x0A, 0x7C, 0x0A, 0x09, 0x00,  // 9E Japanese Yen
                              0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF,  // 9F Vertical grid pattern

                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // A0 (160)
                              0x00, 0x00, 0xF2, 0x00, 0x00, 0x00,  // A1 (161)
                              0x38, 0xC4, 0x64, 0x5C, 0x46, 0x00,  // A2 (162)
                              0x48, 0x7E, 0x49, 0x41, 0x42, 0x00,  // A3 (163)
                              0x5A, 0x24, 0x24, 0x24, 0x5A, 0x00,  // A4 (164)
                              0x29, 0x2A, 0x7C, 0x2A, 0x29, 0x00,  // A5 (165)
                              0x00, 0x00, 0x77, 0x00, 0x00, 0x00,  // A6 (166)
                              0x90, 0xAA, 0xAD, 0x55, 0x15, 0x08,  // A7 (167)
                              0x03, 0x00, 0x03, 0x00, 0x00, 0x00,  // A8 (168)
                              0x7E, 0x99, 0xA5, 0xA5, 0x81, 0x7E,  // A9 (169)
                              0x18, 0x15, 0x15, 0x05, 0x1E, 0x00,  // AA (170)
                              0x08, 0x14, 0x2A, 0x14, 0x22, 0x00,  // AB (171)
                              0x02, 0x02, 0x02, 0x02, 0x06, 0x00,  // AC (172)
                              0x04, 0x04, 0x04, 0x00, 0x00, 0x00,  // AD (173)
                              0x7E, 0xFD, 0x95, 0xB5, 0xC9, 0x7E,  // AE (174)
                              0x01, 0x01, 0x01, 0x01, 0x01, 0x00,  // AF (175)
                              0x04, 0x0A, 0x0A, 0x04, 0x00, 0x00,  // B0 (176)
                              0x44, 0x44, 0x5F, 0x44, 0x44, 0x00,  // B1 (177)
                              0x19, 0x15, 0x15, 0x12, 0x00, 0x00,  // B2 (178)
                              0x11, 0x15, 0x15, 0x0A, 0x00, 0x00,  // B3 (179)
                              0x00, 0x00, 0x00, 0x02, 0x01, 0x00,  // B4 (180)
                              0xFC, 0x40, 0x40, 0x40, 0x7C, 0x00,  // B5 (181)
                              0x06, 0x09, 0xFF, 0x01, 0xFF, 0x01,  // B6 (182)
                              0x00, 0x00, 0x18, 0x18, 0x00, 0x00,  // B7 (183)
                              0x00, 0x80, 0xA0, 0xB0, 0x40, 0x00,  // B8 (184)
                              0x00, 0x08, 0x09, 0x0F, 0x08, 0x00,  // B9 (185)
                              0x00, 0x0A, 0x0D, 0x0D, 0x0A, 0x00,  // BA (186)
                              0x00, 0x22, 0x14, 0x2A, 0x14, 0x08,  // BB (187)
                              0x20, 0x17, 0x28, 0x34, 0x6A, 0x01,  // BC (188)
                              0x20, 0x17, 0x48, 0x6C, 0x6A, 0x51,  // BD (189)
                              0x25, 0x17, 0x2F, 0x34, 0x6A, 0x01,  // BE (190)
                              0x30, 0x48, 0x45, 0x40, 0x20, 0x00,  // BF (191)
                              0x78, 0x14, 0x15, 0x16, 0x78, 0x00,  // C0 (192)
                              0x78, 0x16, 0x15, 0x14, 0x78, 0x00,  // C1 (193)
                              0x78, 0x16, 0x15, 0x16, 0x78, 0x00,  // C2 (194)
                              0x7A, 0x15, 0x17, 0x16, 0x79, 0x00,  // C3 (195)
                              0x78, 0x15, 0x14, 0x15, 0x78, 0x00,  // C4 (196)
                              0x78, 0x15, 0x16, 0x15, 0x78, 0x00,  // C5 (197)
                              0x7C, 0x13, 0x11, 0x7F, 0x49, 0x41,  // C6 (198)
                              0x1E, 0xA1, 0xE1, 0xE1, 0x12, 0x00,  // C7 (199)
                              0x7C, 0x54, 0x55, 0x56, 0x44, 0x00,  // C8 (200)
                              0x7C, 0x56, 0x55, 0x54, 0x44, 0x00,  // C9 (201)
                              0x7C, 0x56, 0x55, 0x56, 0x44, 0x00,  // CA (202)
                              0x7C, 0x55, 0x54, 0x55, 0x44, 0x00,  // CB (203)
                              0x00, 0x44, 0x7D, 0x46, 0x00, 0x00,  // CC (204)
                              0x00, 0x46, 0x7D, 0x44, 0x00, 0x00,  // CD (205)
                              0x00, 0x46, 0x7D, 0x46, 0x00, 0x00,  // CE (206)
                              0x00, 0x45, 0x7C, 0x45, 0x00, 0x00,  // CF (207)
                              0x7F, 0x49, 0x41, 0x22, 0x1C, 0x00,  // D0 (208)
                              0x7E, 0x09, 0x13, 0x22, 0x7D, 0x00,  // D1 (209)
                              0x38, 0x44, 0x45, 0x46, 0x38, 0x00,  // D2 (210)
                              0x38, 0x46, 0x45, 0x44, 0x38, 0x00,  // D3 (211)
                              0x38, 0x46, 0x45, 0x46, 0x38, 0x00,  // D4 (212)
                              0x3A, 0x45, 0x47, 0x46, 0x39, 0x00,  // D5 (213)
                              0x38, 0x45, 0x44, 0x45, 0x38, 0x00,  // D6 (214)
                              0x22, 0x14, 0x08, 0x14, 0x22, 0x00,  // D7 (215)
                              0x3E, 0x51, 0x49, 0x45, 0x3E, 0x00,  // D8 (216)
                              0x3C, 0x40, 0x41, 0x42, 0x3C, 0x00,  // D9 (217)
                              0x3C, 0x42, 0x41, 0x40, 0x3C, 0x00,  // DA (218)
                              0x3C, 0x42, 0x41, 0x42, 0x3C, 0x00,  // DB (219)
                              0x3C, 0x41, 0x40, 0x41, 0x3C, 0x00,  // DC (220)
                              0x04, 0x0A, 0x71, 0x08, 0x04, 0x00,  // DD (221)
                              0x7F, 0x22, 0x22, 0x22, 0x1C, 0x00,  // DE (222)
                              0xFE, 0x49, 0x49, 0x4E, 0x30, 0x00,  // DF (223)
                              0xC0, 0xA8, 0xAA, 0x2C, 0xF0, 0x00,  // E0 (224)
                              0xC0, 0xAC, 0xAA, 0x28, 0xF0, 0x00,  // E1 (225)
                              0xC0, 0xAC, 0xAA, 0x2C, 0xF0, 0x00,  // E2 (226)
                              0xC4, 0xAA, 0xAE, 0x2C, 0xF2, 0x00,  // E3 (227)
                              0xC0, 0xAA, 0xA8, 0x2A, 0xF0, 0x00,  // E4 (228)
                              0xC0, 0xAA, 0xAD, 0x2D, 0xF2, 0x00,  // E5 (229)
                              0x39, 0x29, 0x7F, 0x4A, 0x4E, 0x00,  // E6 (230)
                              0x18, 0xA4, 0xE4, 0xE4, 0x24, 0x00,  // E7 (231)
                              0x70, 0xA8, 0xAA, 0xAC, 0xB0, 0x00,  // E8 (232)
                              0x70, 0xAC, 0xAA, 0xA8, 0xB0, 0x00,  // E9 (233)
                              0x70, 0xAC, 0xAA, 0xAC, 0xB0, 0x00,  // EA (234)
                              0x70, 0xAA, 0xA8, 0xAA, 0xB0, 0x00,  // EB (235)
                              0x00, 0x00, 0xFA, 0x04, 0x00, 0x00,  // EC (236)
                              0x00, 0x04, 0xFA, 0x00, 0x00, 0x00,  // ED (237)
                              0x00, 0x04, 0xFA, 0x04, 0x00, 0x00,  // EE (238)
                              0x00, 0x02, 0xF8, 0x02, 0x00, 0x00,  // EF (239)
                              0x30, 0x48, 0x4D, 0x4A, 0x3E, 0x01,  // F0 (240)
                              0xF4, 0x12, 0x16, 0x14, 0xE2, 0x00,  // F1 (241)
                              0x60, 0x90, 0x92, 0x94, 0x60, 0x00,  // F2 (242)
                              0x60, 0x94, 0x92, 0x90, 0x60, 0x00,  // F3 (243)
                              0x60, 0x94, 0x92, 0x94, 0x60, 0x00,  // F4 (244)
                              0x64, 0x92, 0x96, 0x94, 0x62, 0x00,  // F5 (245)
                              0x60, 0x92, 0x90, 0x92, 0x60, 0x00,  // F6 (246)
                              0x08, 0x08, 0x2A, 0x08, 0x08, 0x00,  // F7 (247)
                              0x30, 0xC8, 0x68, 0x58, 0x4C, 0x30,  // F8 (248)
                              0x70, 0x80, 0x82, 0x84, 0xF0, 0x00,  // F9 (249)
                              0x70, 0x84, 0x82, 0x80, 0xF0, 0x00,  // FA (250)
                              0x70, 0x84, 0x82, 0x84, 0xF0, 0x00,  // FB (251)
                              0x70, 0x82, 0x80, 0x82, 0xF0, 0x00,  // FC (252)
                              0x18, 0xA0, 0xA4, 0xA2, 0x78, 0x00,  // FD (253)
                              0xFF, 0x24, 0x24, 0x24, 0x18, 0x00,  // FE (254)
                              0x18, 0xA2, 0xA0, 0xA2, 0x78, 0x00   // FF (255)

                              };
//----------------------------------------------------------------------------

//#pragma memory=default
