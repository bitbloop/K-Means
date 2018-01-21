#include"k-means\test_k-means_visual.h"

#include <iostream>

int main()
{
	kmeans::run_visual_spheres();
	kmeans::run_visual_mesh("k-means/bunny.obj");


	std::cout << std::endl << "waiting on a keypress to terminate.";
	getchar();

	return 0;
}

