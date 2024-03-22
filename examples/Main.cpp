#include <iostream>

#include "../src/Thumbhash.h"
#include "../src/lodepng/Lodepng.h"

using namespace std;

int main(void) {
    Image input;
	bool is_loaded = input.ReadFromFile("examples/images-original/flower.png");
	if (!is_loaded) return 0;
	ThumbHash th;
	uint8_t *hash = th.RGBAToThumbHash(input);
	if (hash) {
		Image output = th.ThumbHashToRGBA(hash);
		output.WriteToFile("examples/images-output/flower.png");    
	} else {
		cout << "Image size too big." << endl;
	}
	return 0;
}