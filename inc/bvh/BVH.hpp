#pragma once
#include "common.h"
#include "Morton.h"

struct sortstruct {
	int order;
	MortonCode64 morton;
};

class BVH
{
public:
	void Init(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, const double tol = 1e-10)
	{
		if (!isInited) {
			isInited = true;
		}
		if (indices.size() % 3 != 0) {
			std::cout << "bvh indexs mod 3 is not 0\n";
			return;
		}
		int faceNum = indices.size() / 3;
		std::vector<std::array<glm::vec3, 2>> cornerlist(faceNum);
		for (int i = 0; i < faceNum; i++) {
			const glm::vec3 v0 = vertices[indices[i * 3]].position;
			const glm::vec3 v1 = vertices[indices[i * 3 + 1]].position;
			const glm::vec3 v2 = vertices[indices[i * 3 + 2]].position;

			const glm::vec3 min = glm::min(glm::min(v0, v1), v2) - glm::vec3(tol);
			const glm::vec3 max = glm::max(glm::max(v0, v1), v2) + glm::vec3(tol);
			cornerlist[i][0] = min;
			cornerlist[i][1] = max;
		}
		this->vertices = &vertices;
		this->indices = &indices;
		Init(cornerlist);
	}

	std::vector<int> GetNodesAtLevel(int level)
	{
		if (level <= 0) return {};

		int startIndex = 1 << (level - 1);  // 2^(level-1)
		int endIndex = (1 << level) - 1;    // 2^level - 1

		std::vector<int> nodes;
		for (int i = startIndex; i <= endIndex; ++i) {
			nodes.push_back(i);
		}
		return nodes;
	}

	bool IsInited() { return isInited; }
	int GetMaxLevel() { return maxLevel; }
	const std::vector<std::array<glm::vec3, 2>>& GetBoxList() { return boxList; }
private:
	const std::vector<Vertex>* vertices;
	const std::vector<uint32_t>* indices;
	int cornersNum;
	int maxLevel;
	std::vector<int> new2old;
	std::vector<std::array<glm::vec3, 2>> boxList;
	bool isInited = false;

	int MaxNodeIndex(int nodeIndex, int b, int e)
	{
		assert(e > b);
		if (b + 1 == e) {
			return nodeIndex;
		}
		int m = b + (e - b) / 2;
		int childl = 2 * nodeIndex;
		int childr = 2 * nodeIndex + 1;
		return std::max(MaxNodeIndex(childl, b, m), MaxNodeIndex(childr, m, e));
	}

	void Init(const std::vector<std::array<glm::vec3, 2>>& cornerlist)
	{
		cornersNum = cornerlist.size();
		std::vector<glm::vec3> boxCenters(cornersNum);
		glm::vec3 vmin = (cornerlist[0][0] + cornerlist[0][1]) * 0.5f;
		glm::vec3 vmax = (cornerlist[0][0] + cornerlist[0][1]) * 0.5f;
		for (int i = 0; i < cornersNum; i++) {
			boxCenters[i] = (cornerlist[i][0] + cornerlist[i][1]) * 0.5f;
			vmin = glm::min(vmin, boxCenters[i]);
			vmax = glm::max(vmax, boxCenters[i]);
		}
		glm::vec3 center = (vmax + vmin) * 0.5f;
		for (int i = 0; i < cornersNum; i++) {
			boxCenters[i] -= center;
		}
		glm::vec3 scalePoint = vmax - center;
		glm::vec3 absScalePoint = glm::abs(scalePoint);
		const double scale = glm::max(
			glm::max(absScalePoint.x, absScalePoint.y),
			absScalePoint.z
		);
		if (scale > 100) {
			for (int i = 0; i < cornersNum; i++) {
				boxCenters[i] /= scale;
			}
		}

		std::vector<sortstruct> list;
		const float multi = 1000.0f;
		list.resize(cornersNum);
		glm::vec3 tmp = glm::vec3(0);
		for (int i = 0; i < cornersNum; i++) {
			tmp = boxCenters[i] * multi;
			list[i].morton = MortonCode64(int(tmp.x), int(tmp.y), int(tmp.z));
			list[i].order = i;
		}
		const auto morton_compare = [](const sortstruct& a, const sortstruct& b) {
			return (a.morton < b.morton);
		};
		std::sort(list.begin(), list.end(), morton_compare);
		new2old.resize(cornersNum);
		for (int i = 0; i < cornersNum; i++) {
			new2old[i] = list[i].order;
		}
		std::vector<std::array<glm::vec3, 2>> sortedCornerlist(cornersNum);
		for (int i = 0; i < cornersNum; i++) {
			sortedCornerlist[i] = cornerlist[list[i].order];
		}
		int nodeNum = MaxNodeIndex(1, 0, cornersNum);
		maxLevel = CalculateLevels(nodeNum);
		boxList.resize(nodeNum + 1); // start with index 1
		std::cout << "bvh nodenum = " << nodeNum
			<< " indicesNum = " << indices->size()
			<< " maxLevel = " << maxLevel << "\n";
		InitBoxesRecursive(sortedCornerlist, 1, 0, cornersNum);
	}

	void InitBoxesRecursive(
		const std::vector<std::array<glm::vec3, 2>>& cornerlist,
		int nodeIndex, int begin, int end)
	{
		if (begin + 1 == end) {
			boxList[nodeIndex] = cornerlist[begin];
			return;
		}
		int midle = begin + (end - begin) / 2;
		int childl = 2 * nodeIndex;
		int childr = 2 * nodeIndex + 1;

		InitBoxesRecursive(cornerlist, childl, begin, midle);
		InitBoxesRecursive(cornerlist, childr, midle, end);

		boxList[nodeIndex][0] = glm::min(boxList[childl][0], boxList[childr][0]);
		boxList[nodeIndex][1] = glm::max(boxList[childl][1], boxList[childr][1]);
	}

	int CalculateLevels(int maxIndex)
	{
		if (maxIndex <= 0) return 0;
		int levels = 0;
		while (maxIndex > 0) {
			maxIndex >>= 1;
			levels++;
		}
		return levels;
	}
};