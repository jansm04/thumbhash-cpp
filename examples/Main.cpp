#include <iostream>
#include "../src/Thumbhash.h"

using namespace std;

int main(void) {
    Image input;
	bool is_loaded = input.ReadFromFile("examples/images-original/bart.png");
	if (!is_loaded) return 0;
	ThumbHash th;
	uint8_t *hash = th.RGBAToThumbHash(input);
	if (hash) {
		Image output = th.ThumbHashToRGBA(hash);
		output.WriteToFile("examples/images-output/bart.png");    
	} else {
		cout << "Image size too big." << endl;
	}
	return 0;
}