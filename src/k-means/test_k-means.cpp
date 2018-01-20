#include "test_k-means.h"
#include "k-means.h"

#include<vector>
#include<assert.h>
#include<iostream>
#include "util\loader_obj_nogl.h"




void kmeans_test::run(const char* obj_mesh_filename)
{
	// ________________________________________________________________
	// load the data

	loader_obj_nogl::obj *mesh{ loader_obj_nogl::obj_create(obj_mesh_filename) };
	assert(obj_num_vert(mesh));

	const auto data_num{ obj_num_vert(mesh) };
	const int dim_num{ 3 };

	std::vector<kmeans::InputData<dim_num> > inputs(data_num);

	float* data_xyz{ (float*)(inputs.data()) };
	for (auto vi = 0; vi < obj_num_vert(mesh); ++vi)
		obj_get_vert_v(mesh, vi, &data_xyz[vi * dim_num]);

	// ________________________________________________________________
	// Print the number of vertices

	std::cout << "Loaded " << obj_mesh_filename << "." << std::endl << "Using " << data_num << " vertices input vectors." << std::endl;	

	// ________________________________________________________________
	// Assign clusters to data

	const auto cluster_assignments{ kmeans::assign_clusters(inputs) };

	// ________________________________________________________________

	std::cout << "done." << std::endl;
}