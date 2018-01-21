#include "test_k-means_visual.h"
#include "k-means.h"
#include<vector>
#include<assert.h>
#include<iostream>
#include "util\loader_obj_nogl.h"
#include "util\timer.h"
#include "renderer\renderer.h"
#include <random>
#include <cmath>

namespace
{
	// ________________________________________________________________
	// for color coding the cluster id's
	float color_map_8[]{1.f,0.f,0.f ,
						0.f,1.f,0.f ,
						1.f,1.f,0.f ,
						0.f,0.f,1.f ,
						1.f,0.f,1.f ,
						0.f,1.f,1.f ,
						1.f,1.f,1.f	};

	inline void cluster_to_rgb(const int cluster_index, float* data)
	{
		data[0] = color_map_8[cluster_index * 3];
		data[1] = color_map_8[cluster_index * 3+1];
		data[2] = color_map_8[cluster_index * 3+2];
	}

	// ________________________________________________________________

	// After the vertex data has been loaded, create vertex clusters, assign colours to clusters, and create a point cloud object with it.
	template <int DIM_NUM>
	auto process_and_get_pointcloud(std::vector<kmeans::InputData<DIM_NUM> > inputs, int cluster_num, unsigned int shader_id)
	{
		const float* data_xyz{ (float*)(inputs.data()) };
		const int data_num{ static_cast<int>(inputs.size()) };
		const auto cluster_assignments{ kmeans::assign_clusters(inputs, cluster_num) };

		// assign colours to points
		std::vector<float> colours(data_num * DIM_NUM);
		float* data_rgb{ (float*)(colours.data()) };
		for (auto vi = 0; vi < data_num; ++vi)
			cluster_to_rgb(cluster_assignments[vi], &data_rgb[vi * DIM_NUM]);

		// create the visual representation of loaded data
		return ::renderer::pointcloud::create(shader_id, data_num, data_xyz, data_rgb);
	}

	/** */
	unsigned int create_sphere_clusters_pointcloud(const unsigned int shader_id)
	{
		// Create the sphere vertices, stacked in a line next to one another.
		const int vertices_per_sphere{ 5000 };
		const int sphere_num{ 6 };
		const float sphere_radius{ 1.f };

		std::uniform_real_distribution<> dist(-sphere_radius, sphere_radius);
		std::uniform_real_distribution<> dist_phi(0, 6.28318530718);
		std::uniform_real_distribution<> dist_theta(0, 3.14159265359);

		const int dim_num{ 3 };
		const int data_num{ sphere_num * vertices_per_sphere };
		
		std::vector<kmeans::InputData<dim_num> > inputs(data_num);
		float* data_xyz{ (float*)(inputs.data()) };

		for (auto si=0; si<sphere_num; ++si)
			for (auto vi = 0; vi < vertices_per_sphere; ++vi)
			{
				const auto phi{ dist_phi (util::random::get_mt19937()) };
				const auto theta{ dist_theta(util::random::get_mt19937()) };
				data_xyz[si * (vertices_per_sphere * dim_num) + vi * dim_num + 0] = sin(phi)*cos(theta) + si*sphere_radius*2.f;	
				data_xyz[si * (vertices_per_sphere * dim_num) + vi * dim_num + 1] = sin(phi)*sin(theta);
				data_xyz[si * (vertices_per_sphere * dim_num) + vi * dim_num + 2] = cos(phi);
				
			}

		// Print the number of vertices
		std::cout << "Created " << sphere_num << " spheres." << std::endl << "Using " << data_num << " " << dim_num << " dimensional vertices as input vectors." << std::endl;

		// ________________________________________________________________
		// 
		const auto cluster_num{ sphere_num };
		return process_and_get_pointcloud<dim_num>(inputs, cluster_num, shader_id);
	}



	/** */
	unsigned int create_mesh_clusters_pointcloud(const char* obj_mesh_filename, const unsigned int shader_id)
	{
		// load the data
		loader_obj_nogl::obj *mesh{ loader_obj_nogl::obj_create(obj_mesh_filename) };
		assert(obj_num_vert(mesh));

		const auto data_num{ obj_num_vert(mesh) };
		const int dim_num{ 3 };
		std::vector<kmeans::InputData<dim_num> > inputs(data_num);

		float* data_xyz{ (float*)(inputs.data()) };
		for (auto vi = 0; vi < data_num; ++vi)
			obj_get_vert_v(mesh, vi, &data_xyz[vi * dim_num]);

		// delete the no longer needed mesh
		loader_obj_nogl::obj_delete(mesh);

		// Print the number of vertices
		std::cout << "Loaded " << obj_mesh_filename << "." << std::endl << "Using " << data_num << " " << dim_num << " dimensional vertices as input vectors." << std::endl;

		// ________________________________________________________________
		const auto cluster_num{ 3 };
		return process_and_get_pointcloud<dim_num>(inputs, cluster_num, shader_id);
	}
	

	void update_view_matrix(const glm::vec3& camera_pos, const glm::vec3& camera_target, unsigned int shader_id)
	{
		// Change the view transformation matrix
		::renderer::space::update_view_matrix(camera_pos, camera_target);

		// get shader uniform locations
		const auto u_projection_matrix{ glGetUniformLocation(shader_id, "u_projection_matrix") };
		const auto u_view_matrix{ glGetUniformLocation(shader_id, "u_view_matrix") };

		// update the shader uniforms
		if (u_projection_matrix != -1) glUniformMatrix4fv(u_projection_matrix, 1, GL_FALSE, glm::value_ptr(::renderer::space::get_projection_matrix()));
		if (u_view_matrix != -1) glUniformMatrix4fv(u_view_matrix, 1, GL_FALSE, glm::value_ptr(::renderer::space::get_view_matrix()));
	}

	/** Creates the OpenGL window and context, and returns the active shader id */
	unsigned int create_gl()
	{
		// Create OpenGL window and context
		::renderer::window::create();

		// Load the shader to be used for rendering
		const auto shader_id{ ::renderer::shader::load_program({ "../src/k-means/basic.glsl" }) };
		glUseProgram(shader_id);

		// return the used shader id
		return shader_id;
	}


};

/** */
void kmeans::run_visual_mesh(const char* obj_mesh_filename)
{
	// Create OpenGL window and context and get the shader id in use
	const auto shader_id{ create_gl() };

	// move the camera
	update_view_matrix({ 0, 3, 8 }, { 0, 1.5, 0 }, shader_id);

	// create the visual representation of loaded data
	const auto pointcloud_id_mesh{ create_mesh_clusters_pointcloud(obj_mesh_filename, shader_id) };

	// loop until we close the window
	while (!::renderer::window::should_window_close()) {
		glClear(GL_DEPTH_BUFFER_BIT);
		::renderer::pointcloud::render(pointcloud_id_mesh);
		::renderer::window::pool_events_and_swap_buffers();
	}

	// Uninitialize the data
	::renderer::pointcloud::kill(pointcloud_id_mesh);
	::renderer::window::kill();
}



/** */
void kmeans::run_visual_spheres()
{
	// Create OpenGL window and context and get the shader id in use
	const auto shader_id{ create_gl() };

	// Move the camera
	update_view_matrix({ 5, 0, 8 }, { 5, 0, 0 }, shader_id);

	// create the visual representation of loaded data
	const auto pointcloud_id_spheres{ create_sphere_clusters_pointcloud(shader_id) };

	// loop until we close the window
	while (!::renderer::window::should_window_close()) {
		glClear(GL_DEPTH_BUFFER_BIT);
		::renderer::pointcloud::render(pointcloud_id_spheres);
		::renderer::window::pool_events_and_swap_buffers();
	}

	// Uninitialize the data
	::renderer::pointcloud::kill(pointcloud_id_spheres);
	::renderer::window::kill();
}