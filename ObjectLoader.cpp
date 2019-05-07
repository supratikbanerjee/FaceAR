#include "ObjectLoader.h"



ObjectLoader::ObjectLoader()
{
}


ObjectLoader::~ObjectLoader()
{
}

bool ObjectLoader::getSimilarVertexIndex_fast(PackedVertex & packed, std::map<PackedVertex, unsigned int> & VertexToOutIndex, unsigned int & result)
{
	std::map<PackedVertex, unsigned int>::iterator it = VertexToOutIndex.find(packed);
	if (it == VertexToOutIndex.end()) {
		return false;
	}
	else {
		result = it->second;
		return true;
	}
}

void ObjectLoader::indexVBO(
	std::vector<glm::vec3> & in_vertices,
	std::vector<glm::vec2> & in_uvs,
	std::vector<glm::vec3> & in_normals,
	std::vector<unsigned int> & out_indices,
	std::vector<glm::vec3> & out_vertices,
	std::vector<glm::vec2> & out_uvs,
	std::vector<glm::vec3> & out_normals)
{
	std::map<PackedVertex, unsigned int> VertexToOutIndex;

	// For each input vertex

	for (unsigned int i = 0; i < in_vertices.size(); i++) {
		PackedVertex packed = { in_vertices[i], in_uvs[i], in_normals[i] };
		// Try to find a similar vertex in out_XXXX
		unsigned int index;
		bool found = getSimilarVertexIndex_fast(packed, VertexToOutIndex, index);
		if (found) { // A similar vertex is already in the VBO, use it instead !
			out_indices.push_back(index);
		}
		else { // If not, it needs to be added in the output data.
			out_vertices.push_back(in_vertices[i]);
			out_uvs.push_back(in_uvs[i]);
			out_normals.push_back(in_normals[i]);
			unsigned int newindex = (unsigned int)out_vertices.size() - 1;
			out_indices.push_back(newindex);
			VertexToOutIndex[packed] = newindex;
		}
	}
}

bool ObjectLoader::loadOBJ(const char * path, std::vector < glm::vec3 > & out_vertices, std::vector < glm::vec2 > & out_uvs, std::vector < glm::vec3 > & out_normals)
{
	std::string buf("blendshapes/");
	buf.append(path);
	const char * dir = buf.c_str();
	unsigned int cc = 0;
	std::vector< unsigned int > vertexIndices, uvIndices, normalIndices;
	std::vector< glm::vec3 > temp_vertices;
	std::vector< glm::vec2 > temp_uvs;
	std::vector< glm::vec3 > temp_normals;

	FILE * file = fopen(dir, "r");
	if (file == NULL) {
		printf("Impossible to open the file !\n");
		return false;
	}

	while (1) {

		char lineHeader[128];
		// read the first word of the line
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.

		// else : parse lineHeader
		if (strcmp(lineHeader, "v") == 0) {
			glm::vec3 vertex;
			fscanf(file, " %f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			cc++;
			//printf(" %f %f %f\n", vertex.x, vertex.y, vertex.z);
			temp_vertices.push_back(vertex);
		}
		else if (strcmp(lineHeader, "vt") == 0) {
			glm::vec2 uv;
			fscanf(file, "%f %f\n", &uv.x, &uv.y);
			temp_uvs.push_back(uv);
		}
		else if (strcmp(lineHeader, "vn") == 0) {
			glm::vec3 normal;
			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			temp_normals.push_back(normal);
		}
		else if (strcmp(lineHeader, "f") == 0) {
			std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
			//printf("%d/%d/%d %d/%d/%d %d/%d/%d\n", vertexIndex[0], uvIndex[0], normalIndex[0], vertexIndex[1], uvIndex[1], normalIndex[1], vertexIndex[2], uvIndex[2], normalIndex[2]);
			//int matches = fscanf(file, "%d\//%d %d\//%d %d\//%d\n", &vertexIndex[0], &normalIndex[0], &vertexIndex[1], &normalIndex[1], &vertexIndex[2], &normalIndex[2]);
			//printf("%d//%d %d//%d %d//%d\n", vertexIndex[0], normalIndex[0], vertexIndex[1], normalIndex[1], vertexIndex[2], normalIndex[2]);
			if (matches != 9) {
				printf("File can't be read by our simple parser : ( Try exporting with other options\n");
				return false;
			}
			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);
			uvIndices.push_back(uvIndex[0]);
			uvIndices.push_back(uvIndex[1]);
			uvIndices.push_back(uvIndex[2]);
			normalIndices.push_back(normalIndex[0]);
			normalIndices.push_back(normalIndex[1]);
			normalIndices.push_back(normalIndex[2]);
		}
	}
	printf("%d %d\n", vertexIndices.size(), normalIndices.size());
	for (unsigned int i = 0; i < vertexIndices.size(); i++) {
		unsigned int vertexIndex = vertexIndices[i];
		glm::vec3 vertex = temp_vertices[vertexIndex - 1];
		//printf(" %f %f %f\n", vertex.x, vertex.y, vertex.z);
		out_vertices.push_back(vertex);
	}

	for (unsigned int i = 0; i < uvIndices.size(); i++) {
		unsigned int uvIndex = uvIndices[i];
		glm::vec2 uv = temp_uvs[uvIndex - 1];
		out_uvs.push_back(uv);
	}

	for (unsigned int i = 0; i < normalIndices.size(); i++) {
		unsigned int normalIndex = normalIndices[i];
		glm::vec3 normal = temp_normals[normalIndex - 1];
		out_normals.push_back(normal);
	}

	printf("%d %d\n", out_vertices.size(), out_normals.size());
	printf("load %d\n", cc);
}

