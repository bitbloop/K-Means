# K-Means Clustering

![Sphere clustering](http://radosjovanovic.com/projects/git/kmeans_sphere.jpg)
![Bunny clustering](http://radosjovanovic.com/projects/git/kmeans_bunny.jpg)

## Basic Use Pseudocode

```cpp
#include "k-means.h"
#include<vector>

using float3 = kmeans::InputData<3>; // InputData<T> memory layout is T consecutive floats.
void main()
{
	// Load the data
	const std::vector<float> data {<< load the data as a vector of xyz into the std::vector >>};
	// Change data dimensionality from (-1) to (-1,3)
	const std::vector<float3> inputs{<< cast the array of float to an array of float3 >>};
	// Assign clusters to data, will look for 2 clusters by default
	const std::vector<int> cluster_assignments{ kmeans::assign_clusters(inputs) };
}
```

## Summary

This implementation of the K-Means clustering algorithm written in C++ as a header only library. The critical parts of the algorithm are written in the k-means.h file, referencing several the standard library files and custom code for timing and random values. That additional custom code outside of k-means.h file is not needed for the functioning algorithm and can be replaced.

The two solution files in build/ folder, VisualScience and Science, correspond to the Visual Studio projects which were set up or not to link with the needed OpenGL libraries and run the data visualization part.

Rest of the repository are libraries needed for visualizations and code for an OpenGL renderer.

## Deployment

If distributing the built executable, put the data files in the folder <exe_folder>/kmeans

## Authors

* **Rados Jovanovic** - *Initial work* - [bitbloop](https://github.com/bitbloop)

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details

## Acknowledgments

* Hat tip to everyone contributing to science!


