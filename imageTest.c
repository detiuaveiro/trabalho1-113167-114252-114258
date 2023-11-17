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
  //for(int i = 0;i<ImageHeight(img1);i++){
    //for(int j = 0;j<ImageHeight(img1);j++){
      //uint8 p = ImageGetPixel(img1,j,i);
      //printf("%d",p);
    //}
  //}

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
  //ImageBrighten(img2, 1.3);

  Image img2 = ImageLoad("art4_300x300.pgm");
  Image img3 = ImageLoad(argv[1]);

  //ImagePaste(img3,54,45,img2);

  //printf("%d",ImageLocateSubImage(img3,0,0,img2));
  //ImageBlend(img3,0,0,img2,0.5);
  ImageBlur(img3,5,5);

  if (ImageSave(img3, argv[2]) == 0) {
    error(2, errno, "%s: %s", argv[2], ImageErrMsg());
  }

  ImageDestroy(&img1);
  ImageDestroy(&img2);
  return 0;
}

