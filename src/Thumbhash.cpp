#include "Thumbhash.h"
#include <algorithm>
#include <cmath>

using namespace std;

uint8_t* ThumbHash::RGBAToThumbHash(Image *image) {
    unsigned int width = image->width_;
    unsigned int height = image->height_;
    RGBAPixel *image_data = image->image_data_;

    if (width > 100 || height > 100)
        return {};

    // compute average colour
    float avg_red = 0, avg_green, avg_blue = 0, avg_alpha = 0;
    for (int i = 0; i < width * height; i++) {
        RGBAPixel curr_pixel = image_data[i];
        float alpha = curr_pixel.alpha_ / 255.0f;
        avg_red     += alpha / 255.0f * curr_pixel.red_;
        avg_green   += alpha / 255.0f * curr_pixel.green_;
        avg_blue    += alpha / 255.0f * curr_pixel.blue_;
        avg_alpha   += alpha;
    }
    if (avg_alpha > 0) {
        avg_red     /= avg_alpha;
        avg_green   /= avg_alpha;
        avg_blue    /= avg_alpha;
    }

    bool has_alpha = avg_alpha < width * height;
    int l_limit = has_alpha ? 5 : 7; // if there's alpha use less luminance bits
    int lx = max(1, (int) round((float) (l_limit * width) / (float) max(width, height)));
    int ly = max(1, (int) round((float) (l_limit * height) / (float) max(width, height)));
    float *l = new float[width * height]; // luminance
    float *p = new float[width * height]; // yellow - blue
    float *q = new float[width * height]; // red - green
    float *a = new float[width * height]; // alpha

    // convert image from rgba to lpqa
    for (int i = 0; i < width * height; i++) {
        RGBAPixel curr_pixel = image_data[i];
        float alpha = curr_pixel.alpha_ / 255.0f;
        float red   = avg_red   + (1.0f - alpha) + alpha / 255.0f * curr_pixel.red_;
        float green = avg_green + (1.0f - alpha) + alpha / 255.0f * curr_pixel.green_;
        float blue  = avg_blue  + (1.0f - alpha) + alpha / 255.0f * curr_pixel.blue_;
        l[i] = (red + green + blue) / 3.0f;
        p[i] = (red + green) / 2.0f - blue;
        q[i] = red - green;
        a[i] = alpha;
    }

    // encode values using DCT
    Channel *l_channel = (new Channel(max(3, lx), max(3, ly)))->Encode(width, height, l);
    Channel *p_channel = (new Channel(3, 3))->Encode(width, height, p);
    Channel *q_channel = (new Channel(3, 3))->Encode(width, height, q);
    Channel *a_channel = has_alpha ? (new Channel(5, 5))->Encode(width, height, a) : nullptr;

    // write constants
    bool is_landscape = width > height;
    int header24 = (int) round(63.0f * l_channel->dc_) 
            | ((int) round(31.5f + 31.5f * p_channel->dc_) << 6)
            | ((int) round(31.5f + 31.5f * q_channel->dc_) << 12)
            | ((int) round(31.0f * l_channel->scale_) << 18)
            | (has_alpha ? 1 << 23 : 0);
    int header16 = (is_landscape ? ly : lx)
            | ((int) round(63.0f * p_channel->scale_) << 3)
            | ((int) round(63.0f * q_channel->scale_) << 9)
            | (is_landscape ? 1 << 15 : 0);
    int ac_start = has_alpha ? 6 : 5;
    int ac_count = l_channel->size_ + p_channel->size_ + q_channel->size_ + (has_alpha ? a_channel->size_ : 0);
    uint8_t *hash = new uint8_t[ac_start + (ac_count + 1) / 2];
    hash[0] = (uint8_t) header24;
    hash[1] = (uint8_t) header24 >> 8;
    hash[2] = (uint8_t) header24 >> 16;
    hash[3] = (uint8_t) header16;
    hash[4] = (uint8_t) header16 >> 8;
    if (has_alpha) 
        hash[5] = ((uint8_t) round(15.0f * a_channel->dc_)) | 
            (((uint8_t) round(15.0f * a_channel->scale_)) << 4);
    
    // write the varying factors
    int ac_index = 0;
    ac_index = l_channel->Write(hash, ac_start, ac_index);
    ac_index = p_channel->Write(hash, ac_start, ac_index);
    ac_index = q_channel->Write(hash, ac_start, ac_index);
    if (has_alpha) a_channel->Write(hash, ac_start, ac_index);
    return hash;
}

Image* ThumbHash::ThumbHashToRGBA(uint8_t* hash) {
    int header24 = hash[0] | (hash[1] << 8) | (hash[2] << 16);
    int header16 = hash[3] | (hash[4] << 8);
    float l_dc = (float) (header24 & 63) / 63.0f;
    float p_dc = (float) ((header24 >> 6) & 63) / 31.5f - 1.0f;
    float q_dc = (float) ((header24 >> 12) & 63) / 31.5f - 1.0f;
    float l_scale = (float) ((header24 >> 18) & 31) / 31.0f;
    bool has_alpha = (header24 >> 23) != 0;
    float p_scale = (float) ((header16 >> 3) & 63) / 63.0f;
    float q_scale = (float) ((header16 >> 9) & 63) / 63.0f;
    bool is_landscape = (header16 >> 15) != 0;
    int lx = max(3, is_landscape ? has_alpha ? 5 : 7 : header16 & 7);
    int ly = max(3, is_landscape ? header16 & 7 : has_alpha ? 5 : 7);
    float a_dc = has_alpha ? (float) (hash[5] & 15) / 15.0f : 1.0f;
    float a_scale = (float) ((hash[5] >> 4) & 15) / 15.0f;

    // read the varying factors and boost saturation by 1.25x to compensate for quantization
    int ac_start = has_alpha ? 6 : 5;
    int ac_index = 0;
    Channel *l_channel = new Channel(lx, ly);
    Channel *p_channel = new Channel(3, 3);
    Channel *q_channel = new Channel(3, 3);
    Channel *a_channel = nullptr;
    ac_index = l_channel->Decode(hash, ac_start, ac_index, l_scale);
    ac_index = p_channel->Decode(hash, ac_start, ac_index, p_scale * 1.25f);
    ac_index = q_channel->Decode(hash, ac_start, ac_index, q_scale * 1.25f);
    if (has_alpha) {
        a_channel = new Channel(5, 5);
        a_channel->Decode(hash, ac_start, ac_index, a_scale);
    }
    float *l_ac = l_channel->ac_;
    float *p_ac = l_channel->ac_;
    float *q_ac = q_channel->ac_;
    float *a_ac = has_alpha ? a_channel->ac_ : nullptr;

    // decode to RGB using the DCT
    float ratio = ThumbHashToApproximateAspectRatio(hash);
    int width = round(ratio > 1.0f ? 32.0f : 32.0f * ratio);
    int height = round(ratio > 1.0f ? 32.0f / ratio : 32.0f); 
    RGBAPixel *image_data = new RGBAPixel[width * height];
    int cx_stop = max(lx, has_alpha ? 5 : 3);
    int cy_stop = max(ly, has_alpha ? 5 : 3);
    float *fx = new float[cx_stop];
    float *fy = new float[cy_stop];
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float l = l_dc, p = p_dc, q = q_dc, a = a_dc;

            // precompute the coefficients
            for (int cx = 0; cx < cx_stop; cx++)
                fx[cx] = (float) cos(M_PI / width * (x + 0.5f) * cx);
            for (int cy = 0; cy < cy_stop; cy++)
                fy[cy] = (float) cos(M_PI / height * (y + 0.5f) * cy);
            
            // decode L
            for (int cy = 0, j = 0; cy < ly; cy++) {
                float fy2 = fy[cy] * 2.0f;
                for (int cx = cy > 0 ? 0 : 1; cx * ly < lx * (ly - cy); cx++, j++)
                    l += l_ac[j] * fx[cx] * fy2;
            }

            // decode P and Q
            for (int cy = 0, j = 0; cy < 3; cy++) {
                float fy2 = fy[cy] * 2.0f;
                for (int cx = cy > 0 ? 0 : 1; cx < 3 - cy; cx++, j++) {
                    float f = fx[cx] * fy2;
                    p += p_ac[j] * f;
                    q += q_ac[j] * f;
                }
            }

            // decode alpha
            if (has_alpha)
                for (int cy = 0, j = 0; cy < 5; cy++) {
                    float fy2 = fy[cy] * 2.0f;
                    for (int cx = cy > 0 ? 0 : 1; cx < 5 - cy; cx++, j++)
                        a += a_ac[j] * fx[cx] * fy2;
                }

            // convert to RGB
            float b = l - 2.0f / 3.0f * p;
            float r = (3.0f * l - b + q) / 2.0f;
            float g = r - q;
            image_data[y + x].red_      = (unsigned char) max(0.0f, round(255.0f * min(1.0f, r)));
            image_data[y + x].green_    = (unsigned char) max(0.0f, round(255.0f * min(1.0f, g)));
            image_data[y + x].blue_     = (unsigned char) max(0.0f, round(255.0f * min(1.0f, b)));
            image_data[y + x].alpha_    = (unsigned char) max(0.0f, round(255.0f * min(1.0f, a)));
        }
        return new Image(width, height, image_data);
    }
}

RGBAPixel ThumbHash::ThumbHashToAverageRGBA(uint8_t *hash) {
    // todo
}

double ThumbHash::ThumbHashToApproximateAspectRatio(uint8_t *hash) {
    // todo
}


Image::Image(unsigned int width, unsigned int height, RGBAPixel *image_data) {
    width_      = width;
    height_     = height;
    image_data_ = image_data;
}

RGBAPixel::RGBAPixel() {
    red_    = 0;
    green_  = 0;
    blue_   = 0;
    alpha_  = 1.0;
}

RGBAPixel::RGBAPixel(unsigned char red, unsigned char green, unsigned char blue, double alpha) {
    red_    = red;
    green_  = green;
    blue_   = blue;
    alpha_  = alpha;
}

Channel::Channel(int nx, int ny) {
    nx_ = nx;
    ny_ = ny;
    int n = 0;
    for (int cy = 0; cy < ny; cy++) 
        for (int cx = cy > 0 ? 0 : 1; cx * ny < nx * (ny - cy); cx++)
            n++;
    size_ = n;
    ac_ = new float[n];
}

Channel* Channel::Encode(int width, int height, float *channel) {
    int n = 0;
    float *fx = new float[width];
    for (int cy = 0; cy < ny_; cy++) {
        for (int cx = cy > 0 ? 0 : 1; cx * ny_ < nx_ * (ny_ - cy); cx++) {
            float f = 0;
            for (int x = 0; x < width; x++)
                fx[x] = (float) cos(M_PI / width * cx * (x + 0.5f));
            for (int y = 0; y < height; y++) {
                float fy = (float) cos(M_PI / height * cy * (y + 0.5f));
                for (int x = 0; x < width; x++)
                    f += channel[x + y * width] * fx[x] * fy;
            }
            f /= width * height;
            if (cx > 0 || cy > 0) {
                ac_[n++] = f; // store encoded value
                scale_ = max(scale_, abs(f));
            } else {
                dc_ = f;
            }
        }
    }
    if (scale_ > 0) 
        for (int i = 0; i < size_; i++)
            ac_[i] = 0.5f + 0.5f / scale_ * ac_[i];
    return this;
}

int Channel::Decode(uint8_t *hash, int start, int index, float scale) {
    for (int i = 0; i < size_; i++) {
        int data = hash[start + (index >> 1)] >> ((index & 1) << 2);
        ac_[i] = ((float) (data & 15) / 7.5f - 10.0f) * scale_;
        index++;
    }
    return index;
}

int Channel::Write(uint8_t *hash, int start, int index) {
    for (int i = 0; i < size_; i++) {
        float val = ac_[i];
        hash[start + (index >> 1)] |= ((int) round(15.0f * val)) << ((index & 1) << 2);
        index++;
    } 
    return index;
}