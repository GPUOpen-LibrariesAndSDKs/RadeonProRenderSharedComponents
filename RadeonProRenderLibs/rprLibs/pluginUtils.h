
#pragma once

class VDBGridSize
{
public:
	VDBGridSize() : gridSizeX(0), gridSizeY(0), gridSizeZ(0), voxelSizeX(0.0), voxelSizeY(0.0), voxelSizeZ(0.0) {}

	size_t gridSizeX;
	size_t gridSizeY;
	size_t gridSizeZ;

	double voxelSizeX;
	double voxelSizeY;
	double voxelSizeZ;

	bool isValid() { return ((gridSizeX > 0) && (gridSizeY > 0) && (gridSizeZ > 0)); }

	size_t& operator[](size_t idx) {
		if (idx == 0) return gridSizeX;
		if (idx == 1) return gridSizeY;
		return gridSizeZ;
	}
};

// Wrapper around vdb grid that makes it convinient to pass data to RPR
template <typename GridT, typename LookupT, typename IndicesT>
class VDBGrid2RPR;

// for convinience
template <typename T>
using VDBGrid = VDBGrid2RPR<T, float, uint32_t>;

// Wrapper around vdb grid that makes it convinient to pass data to RPR
template <typename GridT, typename LookupT, typename IndicesT>
class VDBGrid2RPR
{
public:
	
	VDBGridSize size;

	std::vector<IndicesT> gridOnIndices;
	std::vector<GridT> gridOnValueIndices;
	std::vector<LookupT> valuesLookUpTable;

	GridT maxValue;
	GridT minValue;

	bool IsValid(void) { return (size.isValid() && gridOnIndices.size() > 0); }
};

using VDBGridParams = std::map<std::string, VDBGridSize>;

