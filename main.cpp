#include "Halide.h"
#include "halide_image_io.h"

#include <iostream>
#include <filesystem> //C++ 17
#include <string>
#include <stdexcept>

using namespace std;

template <typename T>
void printShape(Halide::Buffer<T> buf) {
	cout << "(" << buf.width() << ", " << buf.height() << ", " << buf.channels() << ")\n";
}

/*
aval1-halide-luxai.exe ../../../inputs/mandrill.png 0.5748 ../../../inputs/peppers2.png 0.8947 0.2358 ../../../output.png
*/
int main(int argc, char* argv[]) {
	try {
		/*PARSE INPUT*/
		array<string, 6> cmds;
		for (int i = 1; i < argc; ++i) {
			cmds[i - 1] = argv[i];
		}
		const float ALPHA = stof(cmds[1]);
		const float BETA = stof(cmds[3]);
		const float GAMMA = stof(cmds[4]);
		cout << "\n\n##### P A R A M E T E R S #####\n\n";
		cout << "Path image 1: " << cmds[0] << "\n";
		cout << "Alpha:        " << ALPHA << "\n";
		cout << "Path image 2: " << cmds[2] << "\n";
		cout << "Beta:         " << BETA << "\n";
		cout << "Gamma:        " << GAMMA << "\n";
		cout << "Output Path:  " << cmds[5] << "\n";
		cout << "\n##### P A R A M E T E R S #####\n\n";

		/*LOAD IMAGES*/
		Halide::Buffer<uint8_t> inputImg_0 = Halide::Tools::load_image(cmds[0]);
		Halide::Buffer<uint8_t> inputImg_1 = Halide::Tools::load_image(cmds[2]);
		Halide::Buffer<uint8_t> outputImg = Halide::Buffer<uint8_t>(inputImg_0.width(), inputImg_0.height(), inputImg_0.channels());

		/*HALIDE PROCESSING*/
		Halide::Var x{ "x" }, y{ "y" }, c{ "c" };
		Halide::Func img_f32{ "img_f32" };
		img_f32(x, y, c) = Halide::ConciseCasts::f32(inputImg_0(x, y, c)) * ALPHA +
			Halide::ConciseCasts::f32(inputImg_1(x, y, c)) * BETA + GAMMA;
		Halide::Func out_img{ "output_image" };
		out_img(x, y, c) = Halide::ConciseCasts::u8(Halide::clamp(img_f32(x, y, c), 0.0f, 255.0f));
		outputImg = out_img.realize({ inputImg_0.width(), inputImg_0.height(), inputImg_0.channels() });

		/*SAVE OUTPUT IMAGE*/
		Halide::Tools::save_image(outputImg, cmds[5]);

		//################################################################
		Halide::Expr hImg0 = inputImg_0(x, y, c);
		hImg0 = Halide::cast<float>(hImg0);

		Halide::Expr hImg1 = inputImg_1(x, y, c);
		hImg1 = Halide::cast<float>(hImg1);

		Halide::Expr outImg = hImg0 * ALPHA + hImg1 * BETA + GAMMA;
		outImg = Halide::clamp(outImg, 0.0f, 255.0f);

		outImg = Halide::cast<uint8_t>(outImg);

		Halide::Func outFunc;
		outFunc(x, y, c) = outImg;

		Halide::Buffer<uint8_t> output1 = Halide::Buffer<uint8_t>(inputImg_0.width(), inputImg_0.height(), inputImg_0.channels());
		output1 = outFunc.realize({ inputImg_0.width(), inputImg_0.height(), inputImg_0.channels() });

		Halide::Tools::save_image(output1, "../../../output1.png");

		for (int j = 0; j < outputImg.height(); j++) {
			for (int i = 0; i < outputImg.width(); i++) {
				for (int c = 0; c < outputImg.channels(); c++) {
					if (outputImg(i, j, c) != output1(i, j, c)) {
						cout << "Pixel (" << i << ", " << j << ", " << c << ") " <<
							"was supposed to be " << unsigned(outputImg(i, j, c)) <<
							" but instead it's " << unsigned(output1(i, j, c)) << "\n";
					}
				}
			}
		}

	}
	catch (const exception& e) {
		cout << "\n\nException " << e.what() << endl << endl;
		cout << "\n####################\nFim" << endl;
		return -1;
	}

	cout << "\n####################\nFim" << endl;
	return 0;
}

