#ifndef _PNG_H
#define _PNG_H

#include <vector>
#include <string>
#include <exception>
#include <fstream>
#include <iostream>
#include <boost/cstdint.hpp>
#include <cmath>

typedef std::size_t size_t;

/* Wrapper class for picoPNG. This class loads a PNG and stores 
 * the pixel values and metadata in object form. - ELD
 */
class PNG{
 private:
  std::vector<unsigned char> pixels;
  unsigned long width;
  unsigned long height;

 public:
  PNG(std::string filename);
  unsigned long getWidth();
  unsigned long getHeight();
  uint16_t operator[](int index);
  std::pair<float, float> getCentroid(bool weighted = true, int threshold = 1000);
  float distanceToCentroid(int x, int y, bool weighted = true, int threshold = 1000);
};

/* Exception to throw if PNG constructor encounters error opening file*/
class PNGDecodeException: public std::exception{
  virtual const char* what() const throw()
  {
    return "PNG decode failed";
  }
};

/* Helper functions for PNG constructor */
int decodePNG(std::vector<unsigned char>& out_image, unsigned long& image_width, unsigned long& image_height, const unsigned char* in_png, size_t in_size, bool convert_to_rgba32 = true);
void loadFile(std::vector<unsigned char>& buffer, const std::string& filename);

#endif
