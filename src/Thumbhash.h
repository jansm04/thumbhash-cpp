#ifndef _THUMBHASH_H_
#define _THUMBHASH_H_

using namespace std;

class ThumbHash {
    public:

        /**
         * Encodes an Image to a ThumbHash.
         * 
         * @param image - the image to be converted to a ThumbHash
         * @returns the encoded unsigned 8-bit integer array
        */
        uint8_t* RGBAToThumbHash(Image *image);

        /**
         * Decodes a ThumbHash to an Image.
         * 
         * @param hash - the unsigned 8-bit integer array
         * @returns the decoded image
        */
        Image* ThumbHashToRGBA(uint8_t *hash);

        /**
         * Computes the average colour from a given thumbhash.
         * 
         * @param hash - the unsigned 8-bit integer array
         * @returns the average rgba values
        */
        RGBAPixel ThumbHashToAverageRGBA(uint8_t *hash);

        /**
         * Computes the approximate aspect ratio (width / height) from a given thumbhash.
         * 
         * @param hash - the unsigned 8-bit integer array
         * @returns the approximate aspect ratio
        */
        double ThumbHashToApproximateAspectRatio(uint8_t *hash);
};

class Image {
    public:
        unsigned int width_; /* the width of the image */
        unsigned int height_; /* the height of the image */
        RGBAPixel *image_data_; /* the RGBA pixels in the image */

        /**
         * Constructs an Image using the given width, height, and RGBA pixels.
         * 
         * @param width - the width of the image
         * @param height - the height of the image
         * @param pixels - the RGBA pixels in the image, row by row
        */
        Image(unsigned int width, unsigned int height, RGBAPixel *image_data);
};

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
         * @param alpha - the alpha value for the new pixel, in [0, 1].
        */
       RGBAPixel(unsigned char red, unsigned char green, unsigned char blue, double alpha);
};

class Channel {
    public:
        int nx_;
        int ny_;
        float dc_;
        float *ac_;
        float scale_;
        int size_;

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
        Channel* Encode(int width, int height, float *channel);

        /**
         * Decodes the varying terms and returns the index
         * 
         * @param hash - the unsigned 8-bit integer array
         * @param start - the start index for the decoder
         * @param index - the current index in the decoder
         * @param scale - the scale for the decoded values
         * @returns the current index in the decoder
        */
        int Decode(uint8_t *hash, int start, int index, float scale);

        /**
         * Writes the encoded varying terms into the array and returns the index
         * 
         * @param hash - the unsigned 8-bit integer array
         * @param start - the start index for the decoder
         * @param index - the current index in the decoder
         * @returns the current index in the decoder
        */
        int Write(uint8_t *hash, int start, int index);
};

#endif