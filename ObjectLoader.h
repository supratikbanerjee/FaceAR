#ifndef OBJECTLOADER_H
#define OBJECTLOADER_H
#include <glm\glm.hpp>
#include <map>
#include <vector>
class ObjectLoader
{
public:

	struct PackedVertex {
		glm::vec3 position;
		glm::vec2 uv;
		glm::vec3 normal;
		bool operator<(const PackedVertex that) const {
			return memcmp((void*)this, (void*)&that, sizeof(PackedVertex)) > 0;
		};
	};

	ObjectLoader();
	~ObjectLoader();

	void indexVBO(std::vector<glm::vec3>&, std::vector<glm::vec2>&, std::vector<glm::vec3>&, std::vector<unsigned int> &, std::vector<glm::vec3>&, std::vector<glm::vec2>&, std::vector<glm::vec3> &);
	bool loadOBJ(const char*, std::vector < glm::vec3 >&, std::vector < glm::vec2 >&, std::vector < glm::vec3 >&);

private:
	bool getSimilarVertexIndex_fast(PackedVertex&, std::map<PackedVertex, unsigned int>&, unsigned int&);
};
#endif

