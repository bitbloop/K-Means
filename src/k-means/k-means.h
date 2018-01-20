#pragma once

#include <vector>

#include<assert.h>
#include<iostream>
#include<unordered_set>

#include "util\random.h"

//#define PROFILE_KMEANS
#define DEBUG_KMEANS

#ifdef PROFILE_KMEANS
#include "util\timer.h"
#endif


/* 

//	Algorithm:
//	m - Number of input vectors
//	K - Number of cluster centroids
//	Randomly initalize cluster centroids
//	Repeat
//		for i = 1 .. m
//			c_i = index of min ( for k = 1 .. K { dist(x_i, u_k) } )
//		for k = 1 .. K
//			u_k = median of for i = 1 .. m { x_i } that belong to u_k

	Use: const auto cluster_assignments{ kmeans::assign_clusters(inputs) };
	where std::vector< kmeans::InputData<dim_num> > inputs(data_num);
	Instead of InputData any structure can be used with operators -, *, +=, /=, length_squared
*/

namespace kmeans
{

	// The input data structure which can be used to wrap the input data, and also a reference to what operators T has to have for assign_clusters() to work properly
	template<int DIM_NUM>
	struct InputData
	{
		float values[DIM_NUM];

		inline float length_squared() const { float sum{ 0 };  for (int i = 0; i < DIM_NUM; ++i) sum += this->values[i] * this->values[i]; return sum; };
		//inline InputData& operator=(const InputData& other) { memcpy(this->values, other.values, sizeof(float) * DIM_NUM); return *this; };
		inline InputData operator-(const InputData& other) const { InputData out; for (int i = 0; i < DIM_NUM; ++i) out.values[i] = this->values[i] - other.values[i];  return out; };
		inline InputData operator*(const InputData& other) const { InputData out; for (int i = 0; i < DIM_NUM; ++i) out.values[i] = this->values[i] * other.values[i];  return out; };
		inline InputData& operator+=(const InputData& other) { for (int i = 0; i < DIM_NUM; ++i) this->values[i] += other.values[i];  return *this; };
		//inline InputData& operator/=(const float& other) { for (int i = 0; i < DIM_NUM; ++i) this->values[i] /= other;  return *this; };
		inline InputData& operator/=(const int& other) { for (int i = 0; i < DIM_NUM; ++i) this->values[i] /= other;  return *this; };
	};
	


	// Assign clusters for each input.
	template<typename T>
	std::vector<int> assign_clusters(const std::vector<T>& inputs, const unsigned int cluster_num = 2)
	{
		//	Algorithm:
		//	m - Number of input vectors
		//	K - Number of cluster centroids
		//	Randomly initalize cluster centroids 
		//	Repeat
		//		for i = 1 .. m
		//			c_i = index of min ( for k = 1 .. K { dist(x_i, u_k) } )
		//		for k = 1 .. K
		//			u_k = median of for i = 1 .. m { x_i } that belong to u_k 

		// The closest cluster index to the input
		std::vector<int> cluster_assignments(inputs.size());
		// Initialize the clusters. A cluster vector is in the same space / of the same dimensionality as an input vector.
		auto cluster_centroids{ intenrnal::make_centroids(cluster_num, inputs) };
		// for computing the median
		std::vector<int> cluser_residents(cluster_centroids.size());

		// Loop until the cost of the iteration is zero
		// The cost metric is the sum of the number of inputs which changed cluster ownership.

		int number_of_changes{ 1 };
		while (number_of_changes > 0)
		{
			number_of_changes = 0;

			// ________________________________________________________________
			// Assign cluster indices to inputs
			{
#ifdef PROFILE_KMEANS
				math::time::Timer tmr;
				tmr.reset();
				const auto start{ tmr.elapsed() };
#endif

				for (unsigned int i = 0; i < inputs.size(); ++i)
				{
					auto& assigned_cluster_index{ cluster_assignments[i] };
					const auto closest_cluster_index{ intenrnal::get_closest_cluster_to_input(inputs[i], cluster_centroids) };
					if (assigned_cluster_index != closest_cluster_index)
					{
						number_of_changes += 1;
						assigned_cluster_index = closest_cluster_index;
					}
				}

#ifdef PROFILE_KMEANS
				const auto end{ tmr.elapsed() };
				std::cout << "Assign cluster indices to inputs done in \t" << end - start << " s." << std::endl;
#endif
			}

			// ________________________________________________________________
			// Reset the memory we use for accumulation of values
			{
#ifdef PROFILE_KMEANS
				math::time::Timer tmr;
				tmr.reset();
				const auto start{ tmr.elapsed() };
#endif

				memset(cluster_centroids.data(), 0, sizeof(T)*cluster_centroids.size());
				//std::fill(cluser_residents.begin(), cluser_residents.end(), 0);
				memset(cluser_residents.data(), 0, sizeof(int)*cluser_residents.size());

#ifdef PROFILE_KMEANS
				const auto end{ tmr.elapsed() };
				std::cout << "Reset the memory done in \t\t\t" << end - start << " s." << std::endl;
#endif
			}

			// ________________________________________________________________
			// Center cluster centroids to be a median of the sum of inputs that belong to that cluster.
			{
#ifdef PROFILE_KMEANS
				math::time::Timer tmr;
				tmr.reset();
				const auto start{ tmr.elapsed() };
#endif

				for (unsigned int i = 0; i < cluster_assignments.size(); ++i) {
					const auto cluster_index_for_input_i{ cluster_assignments[i] };
					cluster_centroids[cluster_index_for_input_i] += inputs[i];
					cluser_residents[cluster_index_for_input_i] += 1;
				}
				for (unsigned int i = 0; i < cluster_centroids.size(); ++i)
					cluster_centroids[i] /= cluser_residents[i];

#ifdef PROFILE_KMEANS
				const auto end{ tmr.elapsed() };
				std::cout << "Center cluster centroids done in \t\t" << end - start << " s." << std::endl;
#endif
			}
#ifdef DEBUG_KMEANS
			std::cout << "Clusters reassigned: " << number_of_changes << std::endl;
#endif
		}

		return cluster_assignments;
	}


	namespace intenrnal
	{

		// Get the specified number of centroids from the data
		template<typename V>
		std::vector<V> make_centroids(const unsigned int centroid_num, const std::vector<V>& inputs)
		{
			assert(inputs.size() >= centroid_num);
			std::uniform_int_distribution<> dist(0, static_cast<int>(inputs.size())-1);
			std::unordered_set<int> chosen_indices;

			std::vector<V> out_centroids(centroid_num);
			for (auto&centroid : out_centroids) {
				auto random_index{ dist(util::random::get_mt19937()) };
				while (chosen_indices.find(random_index) != chosen_indices.end()) random_index = dist(util::random::get_mt19937());
				chosen_indices.insert(random_index);

				centroid = inputs[random_index];
			}

			return out_centroids;
		}

		// For all clusters, get the cluster closest to the input (minimizes the squared length of the distance between the input and a centroid)
		template<typename V>
		int get_closest_cluster_to_input(const V& input, const std::vector<V>& centroids)
		{
			int closest_intex{ 0 };
			float closest_dist{ ((input - centroids[0])*(input - centroids[0])).length_squared() };
			for (unsigned int i = 1; i < centroids.size(); ++i) {
				const auto dist{ ((input - centroids[i])*(input - centroids[i])).length_squared() };
				if (dist < closest_dist) {
					closest_dist = dist;
					closest_intex = i;
				}
			}
			return closest_intex;
		}
	};
};
