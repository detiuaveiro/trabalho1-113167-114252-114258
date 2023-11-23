// imageTest - A program that performs some image processing.
//
// This program is an example use of the image8bit module,
// a programming project for the course AED, DETI / UA.PT
//
// You may freely use and modify this code, NO WARRANTY, blah blah,
// as long as you give proper credit to the original and subsequent authors.
//
// João Manuel Rodrigues <jmr@ua.pt>
// 2023

#include <assert.h>
#include <errno.h>
#include "error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "image8bit.h"
#include "instrumentation.h"

int main(int argc, char* argv[]) {
  program_name = argv[0];
  if (argc != 3) {
    error(1, 0, "Usage: imageTest input.pgm output.pgm");
  }

  ImageInit();
  
  printf("# LOAD image");
  InstrReset(); // to reset instrumentation
  Image img1 = ImageLoad(argv[1]);
  if (img1 == NULL) {
    error(2, errno, "Loading %s: %s", argv[1], ImageErrMsg());
  }
  InstrPrint(); // to print instrumentation
  /*for(int i = 0;i<ImageHeight(img1);i++){
    for(int j = 0;j<ImageHeight(img1);j++){
      uint8 p = ImageGetPixel(img1,j,i);
      printf("%d",p);
    }
  }*/
  //Image img2 = ImageLoad(argv[1]);
  // Try changing the behaviour of the program by commenting/uncommenting
  // the appropriate lines.

  //Image img2 = ImageCrop(img1, ImageWidth(img1)/4, ImageHeight(img1)/4, ImageWidth(img1)/2, ImageHeight(img1)/2);
  //Image img2 = ImageRotate(img1);
  //Image img2 = ImageMirror(img1);
  //if (img2 == NULL) {
    //error(2, errno, "Rotating img2: %s", ImageErrMsg());
  //}
  //ImageNegative(img2);
  //ImageThreshold(img2, 100);
  //ImageBrighten(img2, 1.5);

  Image img2 = ImageLoad("test/blend.pgm");
  Image img3 = ImageLoad(argv[1]);

  //ImagePaste(img3,54,45,img2);

  //printf("%d",ImageLocateSubImage(img3,0,0,img2));
  //ImageBlend(img2,0,0,img3,0.33);
  ImageBlur(img3,1,1);

  /*for(int i = 0;i<ImageHeight(img2);i++){
    for(int j = 0;j<ImageWidth(img2);j++){
      if(ImageGetPixel(img2,j,i) != ImageGetPixel(img3,j,i)){
        printf("%d img2\n",ImageGetPixel(img2,j,i));
        printf("%d img3\n",ImageGetPixel(img3,j,i));
        i=ImageHeight(img2);
        break;
      }
    }
  }*/

  if (ImageSave(img3, argv[2]) == 0) {
    error(2, errno, "%s: %s", argv[2], ImageErrMsg());
  }

  ImageDestroy(&img1);
  ImageDestroy(&img2);
  return 0;
}

