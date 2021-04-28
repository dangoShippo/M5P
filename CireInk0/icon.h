#include "utility/Ink_Sprite.h"

typedef struct {
    int width;
    int height;
    int bitCount;
    unsigned char *ptr;
}image_t;

// Num18x28
extern unsigned char image_num_28_01[63]; //"0"  W:18 H:29
extern unsigned char image_num_28_02[63]; //"1"  W:18 H:29
extern unsigned char image_num_28_03[63]; //"2"  W:18 H:29
extern unsigned char image_num_28_04[63]; //"3"  W:18 H:29
extern unsigned char image_num_28_05[63]; //"4"  W:18 H:29
extern unsigned char image_num_28_06[63]; //"5"  W:18 H:29
extern unsigned char image_num_28_07[63]; //"6"  W:18 H:29
extern unsigned char image_num_28_08[63]; //"7"  W:18 H:29
extern unsigned char image_num_28_09[63]; //"8"  W:18 H:29
extern unsigned char image_num_28_10[63]; //"9"  W:18 H:29
extern unsigned char image_num_28_11[63]; //"/"  W:18 H:29
extern unsigned char image_num_28_12[63]; //":"  W:18 H:29

extern image_t num18x28[12];
