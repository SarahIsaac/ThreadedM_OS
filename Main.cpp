#include <iostream>
#include<ostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <cmath>
#include <omp.h>
#include "Timer.h"

struct Color {
	int red;
	int green;
	int blue;

	Color()
	{
		red = 0;
		green = 0;
		blue = 0;
	}

	Color(int r, int g, int b)
	{
		red = r;
		green = g;
		blue = b;
	}
};

typedef std::vector<std::vector<Color>> image;

void writeImage(image const &img, std::ofstream& o, float dimension, std::string filename = "mandelbrot.ppm")
{
	o.open(filename);
	o << "P3" << std::endl;
	o << dimension << " " << dimension << std::endl;
	o << 255 << std::endl;
	for (int i = 0; i<dimension; i++)
	{
		std::vector<Color> vec = img[i];
		for (int j = 0; j < dimension; j++)
		{
			o << vec[j].red % 2 << " " << vec[j].green << " " << vec[j].blue << " ";
		}
		o << std::endl;
	}
	o.close();
}

Color determineColor(int iterations)
{
	Color c(iterations, iterations, iterations);
	return c;
}

int doMandelbrot(float x_a, float y_a)
{
	float x = 0;
	float y = 0;
	int i = 0;
	while (i < 256 && (x*x) + (y*y) < (2 * 2))
	{
		float temp = x*x - y*y + x_a;
		y = (2 * x * y) + y_a;
		x = temp;
		i++;
	}
	return i;
}

image loopMandelBrotByRow()
{
	float dim = 512; //always going to be a square
	float minR = -2;
	float maxR = 1;
	float minI = -1;
	float maxI = 1;

	std::vector<std::vector<Color>> img(512, std::vector<Color>(512));
	int iteration;
	int a;
	for (a = 0; a < dim; a++)
	{
		float x = ((a / dim) * (maxR - minR)) + minR;
		int b;
#pragma omp parallel for schedule(static)
		for (b = 0; b < (int)dim; b++)
		{
			float y = ((b / dim) * (maxI - minI)) + minI;
			int iteration = doMandelbrot(x, y);
			Color color = determineColor(iteration);
			img[a][b] = color;
		}
	}
	return img;
}

std::vector<Color> MandelBrotRow(float dim, float x, float maxI, float minI)
{
	std::vector<Color> row(512);
	for (float b = 0; b < dim; b++)
	{
		float y = ((b / dim) * (maxI - minI)) + minI;
		int iteration = doMandelbrot(x, y);
		Color color = determineColor(iteration);
		row[b] = color;
	}
	return row;
}

image loopMandelBrotByPixel()
{
	float dim = 512; //always going to be a square
	float minR = -2;
	float maxR = 1;
	float minI = -1;
	float maxI = 1;

	std::vector<std::vector<Color>> img(512, std::vector<Color>(512));
	int iteration;
	int a;
#pragma omp parallel for schedule(static)
	for (a = 0; a < (int)dim; a++)
	{
		float x = ((a / dim) * (maxR - minR)) + minR;
		img[a] = MandelBrotRow(dim, x, maxI, minI);
	}
	return img;
}

double getAverage(std::vector<double> times)
{
	int size = times.size();
	double total = 0;
	for (int i = 0; i < times.size(); i++)
	{
		std::cout << times[i] << std::endl;
		total += times[i];
	}
	double average = total / size;
	return average;
}

double getStdDev(double average, std::vector<double> times)
{
	double size = times.size();
	double sum = 0;
	for (double i = 0; i < times.size(); i++)
	{
		sum += ((times[i] - average) * (times[i] - average));
	}
	sum = sqrt(sum / size);
	return sum;
}

int main()
{
	double dimension = 512; //always just make a square

	image img = loopMandelBrotByPixel();
	std::ofstream os;
	std::string filename = "mandelbrot_by_pixel.ppm";
	writeImage(*&img, os, dimension, filename);

	image img2 = loopMandelBrotByRow();
	std::ofstream os2;
	std::string filename2 = "mandelbrot_by_row.ppm";
	writeImage(*&img2, os2, dimension, filename2);

	std::vector<double> times;
	for (int i = 0; i < 30; i++)
	{
		double time = functionTimer([]() -> void {loopMandelBrotByRow(); });
		times.push_back(time);
	}

	double average = getAverage(times);
	double std_dev = getStdDev(average, times);
	std::cout << "Thread Strategy by ROW" << std::endl;
	std::cout << "Average time (in milliseconds): " << average << std::endl;
	std::cout << "Standard Deviation: " << std_dev << std::endl;

	std::vector<double> times_2;
	for (int i = 0; i < 30; i++)
	{
		double time = functionTimer([]() -> void {loopMandelBrotByPixel(); });
		times_2.push_back(time);
	}

	double average2 = getAverage(times_2);
	double std_dev2 = getStdDev(average, times_2);
	std::cout << "Thread Strategy by PIXEL" << std::endl;
	std::cout << "Average time (in milliseconds): " << average2 << std::endl;
	std::cout << "Standard Deviation: " << std_dev2 << std::endl;

	return 0;
}
