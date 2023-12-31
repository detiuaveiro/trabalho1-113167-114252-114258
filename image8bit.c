/// image8bit - A simple image processing module.
///
/// This module is part of a programming project
/// for the course AED, DETI / UA.PT
///
/// You may freely use and modify this code, at your own risk,
/// as long as you give proper credit to the original and subsequent authors.
///
/// João Manuel Rodrigues <jmr@ua.pt>
/// 2013, 2023

// Student authors (fill in below):
// NMec:  Name:
// 113167 Gabriel Oliveira
// 114258 Rafael Dias
// 
// Date:
//

#include "image8bit.h"

#include <math.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include "instrumentation.h"

#define MAX(a,b) ((a) > (b)? (a):(b))
#define MIN(a,b) ((a) < (b)? (a):(b))
// The data structure
//
// An image is stored in a structure containing 3 fields:
// Two integers store the image width and height.
// The other field is a pointer to an array that stores the 8-bit gray
// level of each pixel in the image.  The pixel array is one-dimensional
// and corresponds to a "raster scan" of the image from left to right,
// top to bottom.
// For example, in a 100-pixel wide image (img->width == 100),
//   pixel position (x,y) = (33,0) is stored in img->pixel[33];
//   pixel position (x,y) = (22,1) is stored in img->pixel[122].
// 
// Clients should use images only through variables of type Image,
// which are pointers to the image structure, and should not access the
// structure fields directly.

// Maximum value you can store in a pixel (maximum maxval accepted)
const uint8 PixMax = 255;

// Internal structure for storing 8-bit graymap images
struct image {
  int width;
  int height;
  int maxval;   // maximum gray value (pixels with maxval are pure WHITE)
  uint8* pixel; // pixel data (a raster scan)
};


// This module follows "design-by-contract" principles.
// Read `Design-by-Contract.md` for more details.

/// Error handling functions

// In this module, only functions dealing with memory allocation or file
// (I/O) operations use defensive techniques.
// 
// When one of these functions fails, it signals this by returning an error
// value such as NULL or 0 (see function documentation), and sets an internal
// variable (errCause) to a string indicating the failure cause.
// The errno global variable thoroughly used in the standard library is
// carefully preserved and propagated, and clients can use it together with
// the ImageErrMsg() function to produce informative error messages.
// The use of the GNU standard library error() function is recommended for
// this purpose.
//
// Additional information:  man 3 errno;  man 3 error;

// Variable to preserve errno temporarily
static int errsave = 0;

// Error cause
static char* errCause;

/// Error cause.
/// After some other module function fails (and returns an error code),
/// calling this function retrieves an appropriate message describing the
/// failure cause.  This may be used together with global variable errno
/// to produce informative error messages (using error(), for instance).
///
/// After a successful operation, the result is not garanteed (it might be
/// the previous error cause).  It is not meant to be used in that situation!
char* ImageErrMsg() { ///
  return errCause;
}


// Defensive programming aids
//
// Proper defensive programming in C, which lacks an exception mechanism,
// generally leads to possibly long chains of function calls, error checking,
// cleanup code, and return statements:
//   if ( funA(x) == errorA ) { return errorX; }
//   if ( funB(x) == errorB ) { cleanupForA(); return errorY; }
//   if ( funC(x) == errorC ) { cleanupForB(); cleanupForA(); return errorZ; }
//
// Understanding such chains is difficult, and writing them is boring, messy
// and error-prone.  Programmers tend to overlook the intricate details,
// and end up producing unsafe and sometimes incorrect programs.
//
// In this module, we try to deal with these chains using a somewhat
// unorthodox technique.  It resorts to a very simple internal function
// (check) that is used to wrap the function calls and error tests, and chain
// them into a long Boolean expression that reflects the success of the entire
// operation:
//   success = 
//   check( funA(x) != error , "MsgFailA" ) &&
//   check( funB(x) != error , "MsgFailB" ) &&
//   check( funC(x) != error , "MsgFailC" ) ;
//   if (!success) {
//     conditionalCleanupCode();
//   }
//   return success;
// 
// When a function fails, the chain is interrupted, thanks to the
// short-circuit && operator, and execution jumps to the cleanup code.
// Meanwhile, check() set errCause to an appropriate message.
// 
// This technique has some legibility issues and is not always applicable,
// but it is quite concise, and concentrates cleanup code in a single place.
// 
// See example utilization in ImageLoad and ImageSave.
//
// (You are not required to use this in your code!)


// Check a condition and set errCause to failmsg in case of failure.
// This may be used to chain a sequence of operations and verify its success.
// Propagates the condition.
// Preserves global errno!
static int check(int condition, const char* failmsg) {
  errCause = (char*)(condition ? "" : failmsg);
  return condition;
}


/// Init Image library.  (Call once!)
/// Currently, simply calibrate instrumentation and set names of counters.
void ImageInit(void) { ///
  InstrCalibrate();
  InstrName[0] = "pixmem";  // InstrCount[0] will count pixel array acesses
  // Name other counters here...
  
}

// Macros to simplify accessing instrumentation counters:
#define PIXMEM InstrCount[0]
// Add more macros here...

// TIP: Search for PIXMEM or InstrCount to see where it is incremented!


/// Image management functions

/// Create a new black image.
///   width, height : the dimensions of the new image.
///   maxval: the maximum gray level (corresponding to white).
/// Requires: width and height must be non-negative, maxval > 0.
/// 
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
/// On failure, returns NULL and errno/errCause are set accordingly.
Image ImageCreate(int width, int height, uint8 maxval) { ///
  assert (width >= 0);
  assert (height >= 0);
  assert (0 < maxval && maxval <= PixMax);
  // Insert your code here!
  Image img = (Image)malloc(sizeof(Image));    // Allocates space for an image.
  if(!check(img != NULL,"Image allocation failed")){free(img); return NULL;}  // Returns NULL and frees memory alocated img if memory allocation fails.
  img->width = width;
  img->height = height;
  img->maxval = maxval;
  img->pixel = (uint8*)malloc((sizeof(uint8))*(img->width * img->height)); // Allocates space for the array where the pixel levels will be stored.
  if(!check(img->pixel!=NULL,"Array alocation failed")){free(img->pixel);return NULL;}; // Returns NULL and frees the array if memory allocation fails.
  return img;
}

/// Destroy the image pointed to by (*imgp).
///   imgp : address of an Image variable.
/// If (*imgp)==NULL, no operation is performed.
/// Ensures: (*imgp)==NULL.
/// Should never fail, and should preserve global errno/errCause.
void ImageDestroy(Image* imgp) { ///
  assert (imgp != NULL);
  // Insert your code here!
  if(*imgp != NULL){
    free((*imgp)->pixel); // Frees memory allocated for the image pixel array.
    free(*imgp);  //  Frees memory allocated for the image.
    imgp = NULL;  // Sets pointer for the image to NULL.
  }
}


/// PGM file operations

// See also:
// PGM format specification: http://netpbm.sourceforge.net/doc/pgm.html

// Match and skip 0 or more comment lines in file f.
// Comments start with a # and continue until the end-of-line, inclusive.
// Returns the number of comments skipped.
static int skipComments(FILE* f) {
  char c;
  int i = 0;
  while (fscanf(f, "#%*[^\n]%c", &c) == 1 && c == '\n') {
    i++;
  }
  return i;
}

/// Load a raw PGM file.
/// Only 8 bit PGM files are accepted.
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
/// On failure, returns NULL and errno/errCause are set accordingly.
Image ImageLoad(const char* filename) { ///
  int w, h;
  int maxval;
  char c;
  FILE* f = NULL;
  Image img = NULL;

  int success = 
  check( (f = fopen(filename, "rb")) != NULL, "Open failed" ) &&
  // Parse PGM header
  check( fscanf(f, "P%c ", &c) == 1 && c == '5' , "Invalid file format" )&&
  skipComments(f) >= 0 &&
  check( fscanf(f, "%d ", &w) == 1 && w >= 0 , "Invalid width" ) &&
  skipComments(f) >= 0 &&
  check( fscanf(f, "%d ", &h) == 1 && h >= 0 , "Invalid height" ) &&
  skipComments(f) >= 0 &&
  check( fscanf(f, "%d", &maxval) == 1 && 0 < maxval && maxval <= (int)PixMax , "Invalid maxval" ) &&
  check( fscanf(f, "%c", &c) == 1 && isspace(c) , "Whitespace expected" ) &&
  // Allocate image
  (img = ImageCreate(w, h, (uint8)maxval)) != NULL &&
  // Read pixels
  check( fread(img->pixel, sizeof(uint8), w*h, f) == w*h , "Reading pixels" );
  PIXMEM += (unsigned long)(w*h);  // count pixel memory accesses

  // Cleanup
  if (!success) {
    errsave = errno;
    ImageDestroy(&img);
    errno = errsave;
  }
  if (f != NULL) fclose(f);
  return img;
}

/// Save image to PGM file.
/// On success, returns nonzero.
/// On failure, returns 0, errno/errCause are set appropriately, and
/// a partial and invalid file may be left in the system.
int ImageSave(Image img, const char* filename) { ///
  assert (img != NULL);
  int w = img->width;
  int h = img->height;
  uint8 maxval = img->maxval;
  FILE* f = NULL;

  int success =
  check( (f = fopen(filename, "wb")) != NULL, "Open failed" ) &&
  check( fprintf(f, "P5\n%d %d\n%u\n", w, h, maxval) > 0, "Writing header failed" ) &&
  check( fwrite(img->pixel, sizeof(uint8), w*h, f) == w*h, "Writing pixels failed" ); 
  PIXMEM += (unsigned long)(w*h);  // count pixel memory accesses

  // Cleanup
  if (f != NULL) fclose(f);
  return success;
}


/// Information queries

/// These functions do not modify the image and never fail.

/// Get image width
int ImageWidth(Image img) { ///
  assert (img != NULL);
  return img->width;
}

/// Get image height
int ImageHeight(Image img) { ///
  assert (img != NULL);
  return img->height;
}

/// Get image maximum gray level
int ImageMaxval(Image img) { ///
  assert (img != NULL);
  return img->maxval;
}

/// Pixel stats
/// Find the minimum and maximum gray levels in image.
/// On return,
/// *min is set to the minimum gray level in the image,
/// *max is set to the maximum.
void ImageStats(Image img, uint8* min, uint8* max) { ///
  assert (img != NULL);
  // Insert your code here!
  uint8 minimum = 0;
  uint8 maximum = 0;
  for(int i = 0;i<(ImageHeight(img)*ImageWidth(img));i++){
    if(img->pixel[i] < minimum){
      minimum = img->pixel[i];
    }
    if(img->pixel[i]>maximum){
      maximum = img->pixel[i];
    }
    *min = minimum;
    *max = maximum;
  }
}

/// Check if pixel position (x,y) is inside img.
int ImageValidPos(Image img, int x, int y) { ///
  assert (img != NULL);
  //printf("%dx\n%dy",x,y);
  return (0 <= x && x < ImageWidth(img)) && (0 <= y && y < ImageHeight(img));
}

/// Check if rectangular area (x,y,w,h) is completely inside img.
int ImageValidRect(Image img, int x, int y, int w, int h) { ///
  assert (img != NULL);
  // Insert your code here!
  return(ImageValidPos(img,x+w,y+h));
}

/// Pixel get & set operations

/// These are the primitive operations to access and modify a single pixel
/// in the image.
/// These are very simple, but fundamental operations, which may be used to 
/// implement more complex operations.

// Transform (x, y) coords into linear pixel index.
// This internal function is used in ImageGetPixel / ImageSetPixel. 
// The returned index must satisfy (0 <= index < img->width*img->height)
static inline int G(Image img, int x, int y) {
  int index;
  // Insert your code here!
  index = (y * ImageWidth(img))+x;  //  To get to the values that have height y we need to pass through the previous rows 
                                    //  (y*image width) then sum the position of the pixel in that row x
  assert (0 <= index && index < ImageWidth(img)*ImageHeight(img));
  return index;
}

/// Get the pixel (level) at position (x,y).
uint8 ImageGetPixel(Image img, int x, int y) { ///
  assert (img != NULL);
  assert (ImageValidPos(img, x, y));
  PIXMEM += 1;  // count one pixel access (read)
  return img->pixel[G(img, x, y)];
} 

/// Set the pixel at position (x,y) to new level.
void ImageSetPixel(Image img, int x, int y, uint8 level) { ///
  assert (img != NULL);
  assert (ImageValidPos(img, x, y));
  PIXMEM += 1;  // count one pixel access (store)
  img->pixel[G(img, x, y)] = level;
} 


/// Pixel transformations

/// These functions modify the pixel levels in an image, but do not change
/// pixel positions or image geometry in any way.
/// All of these functions modify the image in-place: no allocation involved.
/// They never fail.


/// Transform image to negative image.
/// This transforms dark pixels to light pixels and vice-versa,
/// resulting in a "photographic negative" effect.
void ImageNegative(Image img) { ///
  assert (img != NULL);
  // Insert your code here!
  for(int i = 0; i<ImageHeight(img)*ImageWidth(img);i++){
    if((img->pixel[i]<(img->maxval)/2) || (img->pixel[i]>(img->maxval)/2)){  //  Checks if the pixel is darker or lighter than the mid pixel level in the range(0,maxval) and subtracts the pixel value to the maxvalue.
      img->pixel[i]=(img->maxval) - (img->pixel[i]);
    }
  }
}

/// Apply threshold to image.
/// Transform all pixels with level<thr to black (0) and
/// all pixels with level>=thr to white (maxval).
void ImageThreshold(Image img, uint8 thr) { ///
  assert (img != NULL);
  // Insert your code here!
  for(int i = 0;i<ImageHeight(img)*ImageWidth(img);i++){ 
    if(img->pixel[i]<thr){
      img->pixel[i] = 0;
    }
    if(img->pixel[i]>=thr){
      img->pixel[i] = img->maxval;
    }
  }
}

/// Brighten image by a factor.
/// Multiply each pixel level by a factor, but saturate at maxval.
/// This will brighten the image if factor>1.0 and
/// darken the image if factor<1.0.
void ImageBrighten(Image img, double factor) { ///
  assert (img != NULL);
  assert (factor >= 0.0);
  // Insert your code here!
  for(int i = 0;i<ImageHeight(img)*ImageWidth(img);i++){  // Multiplies each pixel value by the factor, if the pixel values is bigger than the maxvalue set the pixel level to the maxvalue
    uint8 new_pixel = round((img->pixel[i])*(factor));    //Rouds the because the resulting value is a double and the pixel level is a uint8
    if(new_pixel<= ImageMaxval(img)){
      img->pixel[i] = (new_pixel);
    }else{
      img->pixel[i] = ImageMaxval(img);
    }
  }
}


/// Geometric transformations

/// These functions apply geometric transformations to an image,
/// returning a new image as a result.
/// 
/// Success and failure are treated as in ImageCreate:
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
/// On failure, returns NULL and errno/errCause are set accordingly.

// Implementation hint: 
// Call ImageCreate whenever you need a new image!

/// Rotate an image.
/// Returns a rotated version of the image.
/// The rotation is 90 degrees anti-clockwise.
/// Ensures: The original img is not modified.
/// 
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
/// On failure, returns NULL and errno/errCause are set accordingly.
Image ImageRotate(Image img) { ///
  assert (img != NULL);
  // Insert your code here!
  Image newimg = ImageCreate(ImageHeight(img),ImageHeight(img),ImageMaxval(img));
  int x,y;
  for(y = 0;y<ImageHeight(newimg);y++){
    for(x = 0;x<ImageWidth(newimg);x++){
      ImageSetPixel(newimg,y,((ImageHeight(newimg)-1)-x),ImageGetPixel(img,x,y)); //funciona
    }
  }
  return newimg;
}

/// Mirror an image = flip left-right.
/// Returns a mirrored version of the image.
/// Ensures: The original img is not modified.
/// 
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
/// On failure, returns NULL and errno/errCause are set accordingly.
Image ImageMirror(Image img) { ///
  assert (img != NULL);
  // Insert your code here!
  Image newimg = ImageCreate(ImageHeight(img),ImageHeight(img),ImageMaxval(img)); // Allocates space for a new image to store the mirrored image
  int x,y;
  for(y = 0;y<ImageHeight(newimg);y++){       // Inverts each row of pixels switching the pixel values from the left most part of the row with the ones in the right most part
    for(x = 0;x<ImageWidth(newimg);x++){
      ImageSetPixel(newimg,((ImageWidth(newimg)-1)-x),y,ImageGetPixel(img,x,y));
    }
  }
  return newimg;
}

/// Crop a rectangular subimage from img.
/// The rectangle is specified by the top left corner coords (x, y) and
/// width w and height h.
/// Requires:
///   The rectangle must be inside the original image.
/// Ensures:
///   The original img is not modified.
///   The returned image has width w and height h.
/// 
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
/// On failure, returns NULL and errno/errCause are set accordingly.
Image ImageCrop(Image img, int x, int y, int w, int h) { ///
  assert (img != NULL);
  assert (ImageValidRect(img, x, y, w, h));
  // Insert your code here!
  Image newimg = ImageCreate(w,h,ImageMaxval(img)); //  Allocates memory a new image with dimensions wxh to store the cropped image
  int i,j;
  for(i = 0;i<ImageHeight(newimg);i++){
    for(j = 0;j<ImageWidth(newimg);j++){
      ImageSetPixel(newimg,j,i,ImageGetPixel(img,x+j,y+i)); // Finds the pixel in the coordinates (x,y) and then copies the levels of those pixels to the new image until it reaches pixel (x+w,y+h)
    }
  }
  return newimg;
}


/// Operations on two images

/// Paste an image into a larger image.
/// Paste img2 into position (x, y) of img1.
/// This modifies img1 in-place: no allocation involved.
/// Requires: img2 must fit inside img1 at position (x, y).
void ImagePaste(Image img1, int x, int y, Image img2) { ///
  assert (img1 != NULL);
  assert (img2 != NULL);
  assert (ImageValidRect(img1, x, y, img2->width, img2->height));
  // Insert your code here!
  int i,j;
  for(j=0;j<ImageHeight(img2);j++){                        //Replaces pixel levels of img1 with pixel levels of img2 starting from position(x,y)
    for(i = 0;i<ImageWidth(img2);i++){                    //until position(x+img2 width-1,j+img2 height-1).
      ImageSetPixel(img1,x+i,y+j,ImageGetPixel(img2,i,j));    
    }
  }
}

/// Blend an image into a larger image.
/// Blend img2 into position (x, y) of img1.
/// This modifies img1 in-place: no allocation involved.
/// Requires: img2 must fit inside img1 at position (x, y).
/// alpha usually is in [0.0, 1.0], but values outside that interval
/// may provide interesting effects.  Over/underflows should saturate.
void ImageBlend(Image img1, int x, int y, Image img2, double alpha) { ///
  assert (img1 != NULL);
  assert (img2 != NULL);
  assert (ImageValidRect(img1, x, y, img2->width, img2->height));   
  // Insert your code here!
  int i,j;
  for(j=0;j<ImageHeight(img2);j++){
    for(i = 0;i<ImageWidth(img2);i++){
      double pixel1 = ImageGetPixel(img2,i,j)*(alpha);        //Multiplies the pixel level of img2 by alpha
      double pixel2 = ImageGetPixel(img1,x+i,j+y)*(1-alpha); //Multiplies the pixel level of img1 by 1-alpha
      uint8 new_pixel = round(pixel1+pixel2);                 //Sums both and rounds because the new caculated pixels are doubles and the pixel level is a uint8 
      if(new_pixel<=ImageMaxval(img1)){
        ImageSetPixel(img1,x+i,y+j,new_pixel);              //Sets the pixel level to the new calculated value
      }else{                                               //By doing this the functions blends img2 into position(x,y) of img1
        ImageSetPixel(img1,x+i,y+j,ImageMaxval(img1));
      }    
    }
  }
}

/// Compare an image to a subimage of a larger image.
/// Returns 1 (true) if img2 matches subimage of img1 at pos (x, y).
/// Returns 0, otherwise.
int ImageMatchSubImage(Image img1, int x, int y, Image img2) { ///
  assert (img1 != NULL);
  assert (img2 != NULL);
  assert (ImageValidPos(img1, x, y));
  // Insert your code here!
  int i,j;
  for(i = 0;i<ImageHeight(img2);i++){
    for(j = 0;j<ImageWidth(img2);j++){
       if (!ImageValidPos(img1, x+j, y+i) || ImageGetPixel(img1, x+j, y+i) != ImageGetPixel(img2, j, i)) {
        return 0;
      }                                                 //If the comapared pixel levels are the same throughout the loop
    }                                                               //returns 1.
  }
  return 1;
}

/// Locate a subimage inside another image.
/// Searches for img2 inside img1.
/// If a match is found, returns 1 and matching position is set in vars (*px, *py).
/// If no match is found, returns 0 and (*px, *py) are left untouched.
int ImageLocateSubImage(Image img1, int* px, int* py, Image img2) { ///
  assert (img1 != NULL);
  assert (img2 != NULL);
  InstrReset(); // to reset instrumentation
  // Insert your code here!
  int i,j;
  for(i = 0;i<ImageHeight(img1);i++){
    for(j = 0;j<ImageWidth(img1);j++){
      if(ImageGetPixel(img1,j,i)==ImageGetPixel(img2,0,0)){     //Finds a pixel in img1 that has the same level of the first pixel of img2
        if(ImageMatchSubImage(img1,j,i,img2)){                  //Checks if the pixel levels from there are equal to the pixel levels of img2
          *px = j;                                              //If so set px and py to the coordinates where the pixel level was found x and y respectively and returns 1
          *py = i;                                              //If the pixel levels starting from that position do not equal the pixel levels of img2
          return 1;                                             //Finds the next pixel level equal to the first pixel level of img2 and does the above again
        }else{                                                  //If the image is not found returns 0
          continue;
        }
      }
    }
  }
  return 0;
}

/// Filtering

/// Blur an image by a applying a (2dx+1)x(2dy+1) mean filter.
/// Each pixel is substituted by the mean of the pixels in the rectangle
/// [x-dx, x+dx]x[y-dy, y+dy].
/// The image is changed in-place.
void ImageBlur(Image img, int dx, int dy) { ///
  // Insert your code here!
  assert(img!=NULL);
  assert(dx>=0 && dy>=0);
  int i,j;


  // Improved blur function
  int pixels_sum[ImageWidth(img) * ImageHeight(img)];
  int pixels_count[ImageWidth(img) * ImageHeight(img)];

  // Compute cumulative sum of the image pixels and counts
  for (i = 0; i < ImageHeight(img); i++) {
      for (j = 0; j < ImageWidth(img); j++) {
          pixels_sum[i * ImageWidth(img) + j] = ImageGetPixel(img, j, i)
              + (i > 0 ? pixels_sum[(i - 1) * ImageWidth(img) + j] : 0)                  //Calculates the mean of each pixel by using previous calculated values
              + (j > 0 ? pixels_sum[i * ImageWidth(img) + j - 1] : 0)                    //this algorithm makes an sort of intersection between previous calculated "matrices"
              - (i > 0 && j > 0 ? pixels_sum[(i - 1) * ImageWidth(img) + j - 1] : 0);    //and uses those values to calculate the new mean
          pixels_count[i * ImageWidth(img) + j] = 1                                     //Stores the number of pixels used to calculate every mean of each pixel
              + (i > 0 ? pixels_count[(i - 1) * ImageWidth(img) + j] : 0)
              + (j > 0 ? pixels_count[i * ImageWidth(img) + j - 1] : 0)
              - (i > 0 && j > 0 ? pixels_count[(i - 1) * ImageWidth(img) + j - 1] : 0);
      }
  }

  // Compute box blur using the cumulative sum and counts
  for (i = 0; i < ImageHeight(img); i++) {
      for (j = 0; j < ImageWidth(img); j++) {
          int x1 = MAX(0, j - dx);                              //These operations do are used to find the first valid position
          int x2 = MIN(ImageWidth(img) - 1, j + dx);            //in case of some indexes being out of bounds
          int y1 = MAX(0, i - dy);                              //for example corner cases
          int y2 = MIN(ImageHeight(img) - 1, i + dy);         

          int sum = pixels_sum[y2 * ImageWidth(img) + x2]
              - (x1 > 0 ? pixels_sum[y2 * ImageWidth(img) + x1 - 1] : 0)                     //Calculates each pixel mean by using the previous calculated values instead of calculating them for every iteration
              - (y1 > 0 ? pixels_sum[(y1 - 1) * ImageWidth(img) + x2] : 0)                        
              + (x1 > 0 && y1 > 0 ? pixels_sum[(y1 - 1) * ImageWidth(img) + x1 - 1] : 0);
          float count = pixels_count[y2 * ImageWidth(img) + x2]                              //Count is an float for the same reasons pointed below in the normal blur algorithm comments
              - (x1 > 0 ? pixels_count[y2 * ImageWidth(img) + x1 - 1] : 0)
              - (y1 > 0 ? pixels_count[(y1 - 1) * ImageWidth(img) + x2] : 0)
              + (x1 > 0 && y1 > 0 ? pixels_count[(y1 - 1) * ImageWidth(img) + x1 - 1] : 0);
          ImageSetPixel(img, j, i, MIN(round(sum / count), ImageMaxval(img)));
      }
}

// versão anterior com n^2 de complexidade
//Remove comments to test and comment the above loops. 
 /*uint8 pixels_mean[ImageWidth(img)][ImageHeight(img)];
  for(i = 0;i<ImageHeight(img);i++){
    for(j = 0;j<ImageWidth(img);j++){                       //Creates an array with the pixel levels of img so the original image 
      pixels_mean[j][i] = ImageGetPixel(img,j,i);           //does not get altered during the process and creates an undesireble result
    }
  }

  for(i = 0;i<ImageHeight(img);i++){
    for(j = 0;j<ImageWidth(img);j++){
      int sum = 0;
      float count = 0;                                      //Altough we will only sum 1 to count each iteration we chose to make it a float
      for(int k = -dy;k<=dy;k++){                           //because of rounding if sum and count were both integers the result of(sum/count) would have to be an integers abd if the result was not an integer it would be converted automatically to one by making count a float the end result of the division is going to be a float too thus being able to be rounded.                                                    //
        for(int l = -dx;l<=dx;l++){                         //For each pixel in the image calculates the mean of the pixels surrounding
          if(ImageValidPos(img,j+l,i+k)){                   //it on an area (2*dx+1)*(2*dy+1)
            sum += ImageGetPixel(img,j+l,i+k);              //if the positon is out of the image it is ignored and no value is added to the mean
            count++;                                       
          }
        }
      }
      if((round(sum/count))>ImageMaxval(img)){           //If the mean is bigger than the max pixel level the pixel level is set to the maximum value
        pixels_mean[j][i]=ImageMaxval(img);
      }else{
        pixels_mean[j][i]=(round(sum/count));            //Sets the pixel level to the value of the mean
      } 
    }
  }

  for(i = 0;i<ImageHeight(img);i++){                      //Sets the pixel levels of the image to the pixel levels store in the array
    for(j = 0;j<ImageWidth(img);j++){                     //The array was created because during the blur process the pixel levels are altered
      ImageSetPixel(img,j,i,pixels_mean[j][i]);           //Because of that instead of adding the value of the pixel level we are adding the value of the mean
    }                                                     //as the pixel level was already replaced by its mean in the previous iteration
  }
*/
}

