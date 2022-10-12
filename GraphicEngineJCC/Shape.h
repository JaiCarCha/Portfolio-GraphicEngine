#ifndef SHAPE_H
#define SHAPE_H

#include <vector>
#include "glm/glm.hpp"

class Shape {
public:
	static void generatePlane(float x, float y, std::vector<float>& vertices, std::vector<unsigned int>& indices)
	{
		x /= 2;
		y /= 2;
		// Without tangents
		//std::vector<float> vertOld = {
		//	// x, y, z, n1, n2, n3, u, v
		//	-x, -y, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
		//	x, -y, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
		//	x, y, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
		//	x, y, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
		//	-x, y, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		//	-x, -y, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f
		//};

		// With tangents
		std::vector<float> vert = {
			// x, y, z, n1, n2, n3, u, v, t1, t2, t3
			-x, -y, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f,
			x, -y, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
			x, y, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
			x, y, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
			-x, y, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
			-x, -y, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f
		};
		vertices = vert;

		std::vector<unsigned int> ind = { 0,1,2,3,4,5 };
		indices = ind;
	}

	static void generateCube(float x, float y, float z, std::vector<float>& vertices, std::vector<unsigned int>& indices, float u = 1.f, float v = 1.f) {
		x /= 2;
		y /= 2;
		z /= 2;

		//std::vector<float> vert = {
		//	//x, y, z, n1, n2, n3, u, v
		//	-x, -y, -z, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
		//	x, -y, -z, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,
		//	x, y, -z, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
		//	x, y, -z, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
		//	-x, y, -z, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,
		//	-x, -y, -z, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
		//	x, y, z, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
		//	x, -y, z, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
		//	-x, -y, z, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
		//	-x, -y, z, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
		//	-x, y, z, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		//	x, y, z, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
		//	-x, -y, -z, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
		//	-x, y, -z, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
		//	-x, y, z, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
		//	-x, y, z, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
		//	-x, -y, z, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
		//	-x, -y, -z, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
		//	x, y, z, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
		//	x, y, -z, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
		//	x, -y, -z, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
		//	x, -y, -z, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
		//	x, -y, z, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
		//	x, y, z, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
		//	x, -y, z, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		//	x, -y, -z, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f,
		//	-x, -y, -z, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,
		//	-x, -y, -z, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,
		//	-x, -y, z, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		//	x, -y, z, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		//	-x, y, -z, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
		//	x, y, -z, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
		//	x, y, z, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
		//	x, y, z, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
		//	-x, y, z, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		//	-x, y, -z, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f
		//};

		std::vector<float> vert = {
			//x, y, z, n1, n2, n3, u, v, t1, t2, t3
			-x, -y, -z, 0.0f, 0.0f, -1.0f, u, v, -1.0f, 0.0f, 0.0f,
			x, -y, -z, 0.0f, 0.0f, -1.0f, 0.0f, v, -1.0f, 0.0f, 0.0f,
			x, y, -z, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,

			x, y, -z, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
			-x, y, -z, 0.0f, 0.0f, -1.0f, u, 0.0f, -1.0f, 0.0f, 0.0f,
			-x, -y, -z, 0.0f, 0.0f, -1.0f, u, v, -1.0f, 0.0f, 0.0f,

			x, y, z, 0.0f, 0.0f, 1.0f, u, 0.0f, 1.0f, 0.0f, 0.0f,
			x, -y, z, 0.0f, 0.0f, 1.0f, u, v, 1.0f, 0.0f, 0.0f,
			-x, -y, z, 0.0f, 0.0f, 1.0f, 0.0f, v, 1.0f, 0.0f, 0.0f,

			-x, -y, z, 0.0f, 0.0f, 1.0f, 0.0f, v, 1.0f, 0.0f, 0.0f,
			-x, y, z, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
			x, y, z, 0.0f, 0.0f, 1.0f, u, 0.0f, 1.0f, 0.0f, 0.0f,
			//
			-x, -y, -z, -1.0f, 0.0f, 0.0f, 0.0f, v, 0.0f, 0.0f, 1.0f,
			-x, y, -z, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
			-x, y, z, -1.0f, 0.0f, 0.0f, u, 0.0f, 0.0f, 0.0f, 1.0f,

			-x, y, z, -1.0f, 0.0f, 0.0f, u, 0.0f, 0.0f, 0.0f, 1.0f,
			-x, -y, z, -1.0f, 0.0f, 0.0f, u, v, 0.0f, 0.0f, 1.0f,
			-x, -y, -z, -1.0f, 0.0f, 0.0f, 0.0f, v, 0.0f, 0.0f, 1.0f,
			
			x, y, z, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f,
			x, y, -z, 1.0f, 0.0f, 0.0f, u, 0.0f, 0.0f, 0.0f, -1.0f,
			x, -y, -z, 1.0f, 0.0f, 0.0f, u, v, 0.0f, 0.0f, -1.0f,

			x, -y, -z, 1.0f, 0.0f, 0.0f, u, v, 0.0f, 0.0f, -1.0f,
			x, -y, z, 1.0f, 0.0f, 0.0f, 0.0f, v, 0.0f, 0.0f, -1.0f,
			x, y, z, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f,
			//
			x, -y, z, 0.0f, -1.0f, 0.0f, u, 0.0f, 1.0f, 0.0f, 0.0f,
			x, -y, -z, 0.0f, -1.0f, 0.0f, u, v, 1.0f, 0.0f, 0.0f,
			-x, -y, -z, 0.0f, -1.0f, 0.0f, 0.0f, v, 1.0f, 0.0f, 0.0f,

			-x, -y, -z, 0.0f, -1.0f, 0.0f, 0.0f, v, 1.0f, 0.0f, 0.0f,
			-x, -y, z, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
			x, -y, z, 0.0f, -1.0f, 0.0f, u, 0.0f, 1.0f, 0.0f, 0.0f,

			-x, y, -z, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
			x, y, -z, 0.0f, 1.0f, 0.0f, u, 0.0f, 1.0f, 0.0f, 0.0f,
			x, y, z, 0.0f, 1.0f, 0.0f, u, v, 1.0f, 0.0f, 0.0f,

			x, y, z, 0.0f, 1.0f, 0.0f, u, v, 1.0f, 0.0f, 0.0f,
			-x, y, z, 0.0f, 1.0f, 0.0f, 0.0f, v, 1.0f, 0.0f, 0.0f,
			-x, y, -z, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f
		};

		vertices = vert;
		std::vector<unsigned int> ind = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35 };
		indices = ind;
	}

	static void generateSphere(float radius, unsigned int rowCount, unsigned int columnCount, 
		std::vector<float>& vertices, std::vector<unsigned int>& indices) {

		float x, y, z, xz;                              // vertex position
		float nx, ny, nz, lengthInv = 1.0f / radius;    // vertex normal
		float u, v;                                     // vertex texCoord
		constexpr float PI = glm::pi<float>();

		float rowStep = 2 * PI / rowCount;
		float columnStep = PI / columnCount;
		float rowAngle, columnAngle;

		for (unsigned int i = 0; i <= columnCount; ++i)
		{
			columnAngle = PI / 2 - i * columnStep;        // starting from pi/2 to -pi/2
			xz = radius * cos(columnAngle);             // r * cos(u)
			y = radius * sin(columnAngle);              // r * sin(u)

			// add (rowCount+1) vertices per column
			// the first and last vertices have same position and normal, but different tex coords
			for (unsigned int j = 0; j <= rowCount; ++j)
			{
				rowAngle = j * rowStep;           // starting from 0 to 2pi

				// vertex position (x, y, z)
				x = xz * cos(rowAngle);             // r * cos(u) * cos(v)
				z = xz * sin(rowAngle);             // r * cos(u) * sin(v)
				vertices.push_back(x);
				vertices.push_back(y);
				vertices.push_back(z);

				// normalized vertex normal (nx, ny, nz), asuming a center (0,0,0)
				nx = x * lengthInv;					// remove radius -> r * 1/r * cos(u) * cos(v) = cos(u) * cos(v)
				ny = y * lengthInv;
				nz = z * lengthInv;
				/*glm::vec3 n(nx, ny, nz);
				n = glm::normalize(n);
				float l = glm::length(n);*/
				vertices.push_back(nx);
				vertices.push_back(ny);
				vertices.push_back(nz);

				// vertex tex coord (u, v) range between [0, 1]
				//u = (float)j / rowCount;
				u = (float)(rowCount - j) / rowCount; // Inverse u
				v = (float)i / columnCount;
				vertices.push_back(u);
				vertices.push_back(v);

				// tangent vertex
				float xt = sin(rowAngle);
				float zt = -cos(rowAngle);

				float tx = xt;
				float ty = 0;
				float tz = zt;

				/*glm::vec3 t(tx, ty, tz);
				t = glm::normalize(t - n * glm::dot(t, n));*/
				float d = glm::dot(glm::vec3(nx, ny, nz), glm::vec3(tx, ty, tz));

				vertices.push_back(tx);
				vertices.push_back(ty);
				vertices.push_back(tz);

			}
		}

		int k1, k2;
		for (unsigned int i = 0; i < columnCount; ++i)
		{
			k1 = i * (rowCount + 1);     // beginning of current column
			k2 = k1 + rowCount + 1;      // beginning of next column

			for (unsigned int j = 0; j < rowCount; ++j, ++k1, ++k2)
			{
				// 2 triangles per sector excluding first and last columns
				// k1 => k2 => k1+1
				if (i != 0)
				{
					indices.push_back(k1);
					indices.push_back(k2);
					indices.push_back(k1 + 1);
				}

				// k1+1 => k2 => k2+1
				if (i != (columnCount - 1))
				{
					indices.push_back(k1 + 1);
					indices.push_back(k2);
					indices.push_back(k2 + 1);
				}
			}
		} 
	}

private:
	Shape() {}; // You cannot create an instance of this object

};

#endif SHAPE_H