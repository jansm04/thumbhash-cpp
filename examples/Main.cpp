#include <iostream>
#include <vector>
#include "../src/Thumbhash.h"

using namespace std;

int main(void) {
    Image input;
	bool is_loaded = input.ReadFromFile("examples/images-original/shark.png");
	if (!is_loaded) return 0;

	unsigned int sum = 0;
	for (unsigned int i = 0; i < input.image_data_.size(); i++) {
		sum += input.image_data_[i].red_ 	- '\0';
		sum += input.image_data_[i].green_	- '\0';
		sum += input.image_data_[i].blue_	- '\0';
		sum += input.image_data_[i].alpha_  - '\0';
	}
	cout << "Sum of image data: " << sum << endl;

	ThumbHash th;
	vector<uint8_t> hash = th.RGBAToThumbHash(input);
	if (!hash.empty()) {

		// print hash
		for (unsigned int i = 0; i < hash.size(); i++)
			cout << hash[i] - '\0' << " ";

		Image output = th.ThumbHashToRGBA(hash);
		output.WriteToFile("examples/images-output/shark.png");    
	} else {
		cout << "Image size too big." << endl;
	}
	return 0;
}