#include "OS_FONT.h" 
#include "POSAPI.h"

UINT8  Alphanumeric2BIG5[127][2] = {

    /*
     * code=0, hex=0x00, ascii="^@"
     */
	{0xa1,0x40},

    /*
     * code=1, hex=0x01, ascii="^A"
     */
    {0xa1,0x40},

    /*
     * code=2, hex=0x02, ascii="^B"
     */
    {0xa1,0x40},

    /*
     * code=3, hex=0x03, ascii="^C"
     */
    {0xa1,0x40},

    /*
     * code=4, hex=0x04, ascii="^D"
     */
    {0xa1,0x40},

    /*
     * code=5, hex=0x05, ascii="^E"
     */
    {0xa1,0x40},

    /*
     * code=6, hex=0x06, ascii="^F"
     */
    {0xa1,0x40},

    /*
     * code=7, hex=0x07, ascii="^G"
     */
    {0xa1,0x40},

    /*
     * code=8, hex=0x08, ascii="^H"
     */
    {0xa1,0x40},

    /*
     * code=9, hex=0x09, ascii="^I"
     */
    {0xa1,0x40},

    /*
     * code=10, hex=0x0A, ascii="^J"
     */
    {0xa1,0x40},

    /*
     * code=11, hex=0x0B, ascii="^K"
     */
    {0xa1,0x40},

    /*
     * code=12, hex=0x0C, ascii="^L"
     */
    {0xa1,0x40},

    /*
     * code=13, hex=0x0D, ascii="^M"
     */
    {0xa1,0x40},

    /*
     * code=14, hex=0x0E, ascii="^N"
     */
    {0xa1,0x40},

    /*
     * code=15, hex=0x0F, ascii="^O"
     */
    {0xa1,0x40},

    /*
     * code=16, hex=0x10, ascii="^P"
     */
    {0xa1,0x40},

    /*
     * code=17, hex=0x11, ascii="^Q"
     */
    {0xa1,0x40},

    /*
     * code=18, hex=0x12, ascii="^R"
     */
    {0xa1,0x40},

    /*
     * code=19, hex=0x13, ascii="^S"
     */
    {0xa1,0x40},

    /*
     * code=20, hex=0x14, ascii="^T"
     */
    {0xa1,0x40},

    /*
     * code=21, hex=0x15, ascii="^U"
     */
    {0xa1,0x40},

    /*
     * code=22, hex=0x16, ascii="^V"
     */
    {0xa1,0x40},

    /*
     * code=23, hex=0x17, ascii="^W"
     */
    {0xa1,0x40},

    /*
     * code=24, hex=0x18, ascii="^X"
     */
    {0xa1,0x40},

    /*
     * code=25, hex=0x19, ascii="^Y"
     */
    {0xa1,0x40},

    /*
     * code=26, hex=0x1A, ascii="^Z"
     */
    {0xa1,0x40},

    /*
     * code=27, hex=0x1B, ascii="^["
     */
    {0xa1,0x40},

    /*
     * code=28, hex=0x1C, ascii="^\"
     */
    {0xa1,0x40},

    /*
     * code=29, hex=0x1D, ascii="^]"
     */
    {0xa1,0x40},

    /*
     * code=30, hex=0x1E, ascii="^^"
     */
    {0xa1,0x40},

    /*
     * code=31, hex=0x1F, ascii="^_"
     */
    {0xa1,0x40},

    /*
     * code=32, hex=0x20, ascii=" "
     */
    {0xa1,0x40},

    /*
     * code=33, hex=0x21, ascii="!"
     */
    {0xa1,0x49},

    /*
     * code=34, hex=0x22, ascii="""
     */
    {0xa1,0xb2},

    /*
     * code=35, hex=0x23, ascii="#"
     */
    {0xa1,0xcc},

    /*
     * code=36, hex=0x24, ascii="$"
     */
    {0xa2,0x43},

    /*
     * code=37, hex=0x25, ascii="%"
     */
    {0xa2,0x48},

    /*
     * code=38, hex=0x26, ascii="&"
     */
    {0xa1,0xcd},

    /*
     * code=39, hex=0x27, ascii="'"
     */
    {0xa1,0xac},

    /*
     * code=40, hex=0x28, ascii="("
     */
    {0xa1,0x7d},

    /*
     * code=41, hex=0x29, ascii=")"
     */
    {0xa1,0x5e},

    /*
     * code=42, hex=0x2A, ascii="*"
     */
    {0xa1,0xce},

    /*
     * code=43, hex=0x2B, ascii="+"
     */
    {0xa1,0xcf},

    /*
     * code=44, hex=0x2C, ascii=","
     */
    {0xa1,0x4d},

    /*
     * code=45, hex=0x2D, ascii="-"
     */
    {0xa1,0xd0},

    /*
     * code=46, hex=0x2E, ascii="."
     */
    {0xa1,0x4f},

    /*
     * code=47, hex=0x2F, ascii="/"
     */
    {0xa2,0xaa},

    /*
     * code=48, hex=0x30, ascii="0"
     */
    {0xa2,0xaf},

    /*
     * code=49, hex=0x31, ascii="1"
     */
    {0xa2,0xb0},

    /*
     * code=50, hex=0x32, ascii="2"
     */
    {0xa2,0xb1},

    /*
     * code=51, hex=0x33, ascii="3"
     */
    {0xa2,0xb2},

    /*
     * code=52, hex=0x34, ascii="4"
     */
    {0xa2,0xb3},

    /*
     * code=53, hex=0x35, ascii="5"
     */
    {0xa2,0xb4},

    /*
     * code=54, hex=0x36, ascii="6"
     */
    {0xa2,0xb5},

    /*
     * code=55, hex=0x37, ascii="7"
     */
    {0xa2,0xb6},

    /*
     * code=56, hex=0x38, ascii="8"
     */
    {0xa2,0xb7},

    /*
     * code=57, hex=0x39, ascii="9"
     */
    {0xa2,0xb8},

    /*
     * code=58, hex=0x3A, ascii=":"
     */
    {0xa1,0x4a},

    /*
     * code=59, hex=0x3B, ascii=";"
     */
    {0xa1,0x51},

    /*
     * code=60, hex=0x3C, ascii="<"
     */
    {0xa1,0xe0},

    /*
     * code=61, hex=0x3D, ascii="="
     */
    {0xa1,0xe2},

    /*
     * code=62, hex=0x3E, ascii=">"
     */
    {0xa1,0xe1},

    /*
     * code=63, hex=0x3F, ascii="?"
     */
    {0xa1,0x48},

    /*
     * code=64, hex=0x40, ascii="@"
     */
    {0xa2,0x49},

    /*
     * code=65, hex=0x41, ascii="A"
     */
    {0xa2,0xcf},

    /*
     * code=66, hex=0x42, ascii="B"
     */
    {0xa2,0xd0},

    /*
     * code=67, hex=0x43, ascii="C"
     */
    {0xa2,0xd1},

    /*
     * code=68, hex=0x44, ascii="D"
     */
    {0xa2,0xd2},

    /*
     * code=69, hex=0x45, ascii="E"
     */
    {0xa2,0xd3},

    /*
     * code=70, hex=0x46, ascii="F"
     */
    {0xa2,0xd4},

    /*
     * code=71, hex=0x47, ascii="G"
     */
    {0xa2,0xd5},

    /*
     * code=72, hex=0x48, ascii="H"
     */
    {0xa2,0xd6},

    /*
     * code=73, hex=0x49, ascii="I"
     */
    {0xa2,0xd7},

    /*
     * code=74, hex=0x4A, ascii="J"
     */
    {0xa2,0xd8},

    /*
     * code=75, hex=0x4B, ascii="K"
     */
    {0xa2,0xd9},

    /*
     * code=76, hex=0x4C, ascii="L"
     */
    {0xa2,0xda},

    /*
     * code=77, hex=0x4D, ascii="M"
     */
    {0xa2,0xdb},

    /*
     * code=78, hex=0x4E, ascii="N"
     */
    {0xa2,0xdc},

    /*
     * code=79, hex=0x4F, ascii="O"
     */
    {0xa2,0xdd},

    /*
     * code=80, hex=0x50, ascii="P"
     */
    {0xa2,0xde},

    /*
     * code=81, hex=0x51, ascii="Q"
     */
    {0xa2,0xdf},

    /*
     * code=82, hex=0x52, ascii="R"
     */
    {0xa2,0xe0},

    /*
     * code=83, hex=0x53, ascii="S"
     */
    {0xa2,0xe1},

    /*
     * code=84, hex=0x54, ascii="T"
     */
    {0xa2,0xe2},

    /*
     * code=85, hex=0x55, ascii="U"
     */
    {0xa2,0xe3},

    /*
     * code=86, hex=0x56, ascii="V"
     */
    {0xa2,0xe4},

    /*
     * code=87, hex=0x57, ascii="W"
     */
    {0xa2,0xe5},

    /*
     * code=88, hex=0x58, ascii="X"
     */
    {0xa2,0xe6},

    /*
     * code=89, hex=0x59, ascii="Y"
     */
    {0xa2,0xe7},

    /*
     * code=90, hex=0x5A, ascii="Z"
     */
    {0xa2,0xe8},

    /*
     * code=91, hex=0x5B, ascii="["
     */
    {0xa1,0x65},

    /*
     * code=92, hex=0x5C, ascii="\"
     */
    {0xa2,0xad},

    /*
     * code=93, hex=0x5D, ascii="]"
     */
    {0xa1,0x66},

    /*
     * code=94, hex=0x5E, ascii="^"
     */
    {0xa1,0x73},

    /*
     * code=95, hex=0x5F, ascii="_"
     */
    {0xa2,0x62},

    /*
     * code=96, hex=0x60, ascii="`"
     */
    {0xa3,0xbf},

    /*
     * code=97, hex=0x61, ascii="a"
     */
    {0xa2,0xe9},

    /*
     * code=98, hex=0x62, ascii="b"
     */
    {0xa2,0xea},

    /*
     * code=99, hex=0x63, ascii="c"
     */
    {0xa2,0xeb},

    /*
     * code=100, hex=0x64, ascii="d"
     */
    {0xa2,0xec},

    /*
     * code=101, hex=0x65, ascii="e"
     */
    {0xa2,0xed},

    /*
     * code=102, hex=0x66, ascii="f"
     */
    {0xa2,0xee},

    /*
     * code=103, hex=0x67, ascii="g"
     */
    {0xa2,0xef},

    /*
     * code=104, hex=0x68, ascii="h"
     */
    {0xa2,0xf0},

    /*
     * code=105, hex=0x69, ascii="i"
     */
    {0xa2,0xf1},

    /*
     * code=106, hex=0x6A, ascii="j"
     */
    {0xa2,0xf2},

    /*
     * code=107, hex=0x6B, ascii="k"
     */
    {0xa2,0xf3},

    /*
     * code=108, hex=0x6C, ascii="l"
     */
    {0xa2,0xf4},

    /*
     * code=109, hex=0x6D, ascii="m"
     */
    {0xa2,0xf5},

    /*
     * code=110, hex=0x6E, ascii="n"
     */
    {0xa2,0xf6},

    /*
     * code=111, hex=0x6F, ascii="o"
     */
    {0xa2,0xf7},

    /*
     * code=112, hex=0x70, ascii="p"
     */
    {0xa2,0xf8},

    /*
     * code=113, hex=0x71, ascii="q"
     */
    {0xa2,0xf9},

    /*
     * code=114, hex=0x72, ascii="r"
     */
    {0xa2,0xfa},

    /*
     * code=115, hex=0x73, ascii="s"
     */
    {0xa2,0xfb},

    /*
     * code=116, hex=0x74, ascii="t"
     */
    {0xa2,0xfc},

    /*
     * code=117, hex=0x75, ascii="u"
     */
    {0xa2,0xfd},

    /*
     * code=118, hex=0x76, ascii="v"
     */
    {0xa2,0xfe},

    /*
     * code=119, hex=0x77, ascii="w"
     */
    {0xa3,0x40},

    /*
     * code=120, hex=0x78, ascii="x"
     */
    {0xa3,0x41},

    /*
     * code=121, hex=0x79, ascii="y"
     */
    {0xa3,0x42},

    /*
     * code=122, hex=0x7A, ascii="z"
     */
    {0xa3,0x43},

    /*
     * code=123, hex=0x7B, ascii="{"
     */
    {0xa1,0xa1},

    /*
     * code=124, hex=0x7C, ascii="|"
     */
    {0xa1,0x55},

    /*
     * code=125, hex=0x7D, ascii="}"
     */
    {0xa1,0xa2},

    /*
     * code=126, hex=0x7E, ascii="~"
     */
    {0xa1,0xe3}
};

