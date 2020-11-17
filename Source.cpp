
#include<iostream>
#include "CImg.h"
#include "mjerenje.h";
#include<thread>
#include <math.h>


#define _CRT_SECURE_NO_WARNINGS
#define NUM_THREADS 8

using namespace cimg_library;
int row, col;

void sobel_filter(float** originalImage, float** newImage, int poc, int kraj)
{
	int x_poc, y_poc, x_zad, y_zad;
	float x_pixel, y_pixel;

	double Gx[3][3] = { { -1, 0, 1 },
							 { -2, 0, 2 },
							 { -1, 0, 1 }
	};

	double Gy[3][3] = { { -1, -2, -1 },
							 { 0,  0,  0 },
							 { 1,  2,  1 }
	};


	for (int i = poc; i < kraj; i++)
	{
		for (int j = 0; j < col; j++)
		{
			if ((i > 0) && (j > 0) && (i < row - 1) && (j < col - 1))
			{
				x_pixel = 0, y_pixel = 0;
				x_poc = 0, x_zad = 0, y_poc = 0, y_zad = 0;
				x_poc = i - 1;
				x_zad = i + 1;
				y_poc = j - 1;
				y_zad = j + 1;
				if (x_poc < 0) x_poc = 0;
				if (x_zad >= row) x_zad = row - 1;
				if (y_poc < 0) y_poc = 0;
				if (y_zad >= col) y_zad = col - 1;

				for (int m = x_poc, xx = 0; m <= x_zad; m++, xx++)
				{
					for (int n = y_poc, yy = 0; n <= y_zad; n++, yy++)
					{
						x_pixel += (Gx[xx][yy] * originalImage[m][n]);
						y_pixel += (Gy[xx][yy] * originalImage[m][n]);
					}
				}
				newImage[i][j] = sqrt(pow(x_pixel,2) + pow(y_pixel,2));
			}
		}
	}
}


int main()
{
	printf("Broj niti na kojima pokrecemo proces: %d\n", NUM_THREADS);

	/*
		Mozete probati i s drugim slikama, ali imao sam problema s ucitavanjem slika koje nisu BMP type.

	*/

	CImg<float> image("../image_data/marilyn.bmp");
	image.get_channel(0);

	row = image.height();
	col = image.width();

	printf("\nSlika ucitana s rezolucijom h:%d i w:%d\n", row, col);

	int size = image.size();

	float** originalImg, ** novaSeqImg, ** novaParImg;

	originalImg = new float* [row];
	novaSeqImg = new float* [row];
	novaParImg = new float* [row];
	for (int i = 0; i < row; i++) {
		originalImg[i] = new float[col];
		novaSeqImg[i] = new float[col];
		novaParImg[i] = new float[col];
	}


	for (int i = 0; i < row; i++)
		for (int j = 0; j < col; j++)
		{
			originalImg[i][j] = image(i * col + j);
		}

	double time1 = get_wall_time();
	std::thread t[NUM_THREADS];
	for (int i = 0; i < NUM_THREADS; i++)
	{
		int poc, kraj, step;
		if (i < NUM_THREADS - 1)
		{
			step = row / NUM_THREADS;
			poc = i * step;
			kraj = poc + step;
			t[i] = std::thread(sobel_filter, originalImg, novaParImg, poc, kraj);
		}
		else
		{
			step = row / NUM_THREADS;
			poc = i * step;
			kraj = poc + step + row % NUM_THREADS;
			t[i] = std::thread(sobel_filter, originalImg, novaParImg, poc, kraj);
		}

	}
	for (int i = 0; i < NUM_THREADS; i++)
	{
		t[i].join();
	}
	double time2 = get_wall_time();
	sobel_filter(originalImg, novaSeqImg, 0, row);
	double time3 = get_wall_time();



	CImg<float> final_seq(col, row);
	CImg<float> final_par(col, row);

	for (int i = 0; i < row; i++)
		for (int j = 0; j < col; j++)
		{
			final_seq(i * col + j) = novaSeqImg[i][j];
			final_par(i * col + j) = novaParImg[i][j];
		}

	final_seq.get_channel(0);
	final_par.get_channel(0);


	final_seq.save("../image_data/marilyn_seq.bmp");
	final_par.save("../image_data/marilyn_paralel.bmp");

	printf("\nSlike spremljene u folderu image_data!\n");


	printf("\nSekvencijalna izvedba %lf s\n", time3 - time2);
	printf("Paralelna izvedba %lf s\n", time2 - time1);

	return 0;

}