#include <cstdint>
#include <string>
#include <vector>
#ifndef _THUMBHASH_H_
#define _THUMBHASH_H_

using namespace std;

class RGBAPixel {
    public:
        unsigned char red_; 
        unsigned char green_;
        unsigned char blue_;
        double alpha_;


        /**
         * Constructs an RGBAPixel with default r, g, b, a, values
        */
        RGBAPixel();

        /**
         * Constructs an RGBAPixel using the given r, g, b, a, values;
         * 
         * @param red - the red value for the new pixel, in [0, 255].
         * @param green - the green value for the new pixel, in [0, 255].
         * @param blue - the blue value for the new pixel, in [0, 255].
         * @param alpha - the alpha value for the new pixel, in [0, 255].
        */
       RGBAPixel(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha);
};

class Image {
    public:
        unsigned int width_; /* the width of the image */
        unsigned int height_; /* the height of the image */
        vector<RGBAPixel> image_data_; /* the RGBA pixels in the image */

        /**
         * Constructs a default Image. 
         * The default width and height is 0, with no RGBA pixels.
        */
        Image();

        /**
         * Constructs an Image using the given width, height, and RGBA pixels.
         * 
         * @param width - the width of the image
         * @param height - the height of the image
         * @param pixels - the RGBA pixels in the image, row by row
        */
        Image(unsigned int width, unsigned int height, vector<RGBAPixel> image_data);

        /**
         * Reads in a PNG image from a file.
         * Overwrites any current image content in the PNG.
         * 
         * @param fileName - name of the file to be read from.
         * @return true, if the image was successfully read and loaded.
         */
        bool ReadFromFile(string const & fileName);

        /**
         * Writes a PNG image to a file.
         * 
         * @param fileName - name of the file to be written.
         * @return true, if the image was successfully written.
         */
        bool WriteToFile(string const & fileName);
};

class ThumbHash {
    public:
        /**
         * Encodes an Image to a ThumbHash.
         * 
         * @param image - the image to be converted to a ThumbHash
         * @returns the encoded unsigned 8-bit integer array
        */
        vector<uint8_t> RGBAToThumbHash(Image image);

        /**
         * Decodes a ThumbHash to an Image.
         * 
         * @param hash - the unsigned 8-bit integer array
         * @returns the decoded image
        */
        Image ThumbHashToRGBA(vector<uint8_t> hash);

        /**
         * Computes the average colour from a given thumbhash.
         * 
         * @param hash - the unsigned 8-bit integer array
         * @returns the average rgba values
        */
        RGBAPixel ThumbHashToAverageRGBA(vector<uint8_t> hash);

        /**
         * Computes the approximate aspect ratio (width / height) from a given thumbhash.
         * 
         * @param hash - the unsigned 8-bit integer array
         * @returns the approximate aspect ratio
        */
        double ThumbHashToApproximateAspectRatio(vector<uint8_t> hash);
};

class Channel {
    public:
        int nx_;
        int ny_;
        float dc_;
        vector<float> ac_;
        float scale_;

        /**
         * Constructs a colour channel using the given nx and ny values
         * 
         * @param nx - the x-component of the normalized AC (varying) terms
         * @param ny - the y-component of the normalized AC (varying) terms
        */
        Channel(int nx, int ny);

        /**
         * Encodes the colour channel using the DCT into DC (constant) and AC (varying) terms
         * 
         * @param width - the width of the image
         * @param height - the height of the image
         * @param channel - the channel values for each pixel in the image
         * @returns the Channel object
        */
        Channel* Encode(int width, int height, vector<float> channel);

        /**
         * Decodes the varying terms and returns the index
         * 
         * @param hash - the unsigned 8-bit integer array
         * @param start - the start index for the decoder
         * @param index - the current index in the decoder
         * @param scale - the scale for the decoded values
         * @returns the current index in the decoder
        */
        int Decode(vector<uint8_t> hash, int start, int index, float scale);

        /**
         * Writes the encoded varying terms into the array and returns the index
         * 
         * @param hash - the unsigned 8-bit integer array
         * @param start - the start index for the decoder
         * @param index - the current index in the decoder
         * @returns the current index in the decoder
        */
        int Write(vector<uint8_t>& hash, int start, int index);
};

#endif