#include <vector>
#include <array>
#include <map>
#include <tuple>
#include <string>
#include <memory>
#include <iostream>
#include <fstream>

#include <openvdb/openvdb.h>
#include "pluginUtils.h"

// These are the functions that are used in Maya plug-in to read vdb files thus far


// Finds grid with given name in vdb file and reads it into provided data container; returns error otherwise
// RET - bool - true if data was read succesfully
// RET - string - error description
// OUT - VDBGrid - vdb grid data ready to be passed to RPR
// IN  - openvdb::io::File - vdb file to be read (file must be opened)
// IN  - string - name of the grid to be read
template <typename GridValueT>
std::tuple<bool, std::string> ReadFileGridToVDBGrid(
	VDBGrid<GridValueT>& outGridData,
	openvdb::io::File& file, 
	const std::string& gridName);

	
// Reads vdb grid into provided data container and also adjust voxel indices values if necessary
// RET - bool - true if data was read succesfully
// OUT - VDBGrid - vdb grid data ready to be passed to RPR
// IN  - GridBase* - vdb pointer to vdb grid
// IN - Grid bbox - can be different from current grid bbox (this is to support case with different grids having different sizes)
template <typename GridValueT>
bool ProcessVDBGrid(
	VDBGrid<GridValueT>& outGrid,
	const openvdb::GridBase* baseGrid,
	const openvdb::CoordBBox& bbox);

	
// Reads only names of the grids
// The idea is that there could be large number of grids, and we want to proceed only up to 3 at a time so there is no need to read all of the grids
// However we need to grab the names of the grids into UI
// RET - bool - true if data was read succesfully
// RET - string - error description
// IN - string - name of the file
// IN - GridParams - data contaijner that holds grid name and grid dimensions
std::tuple<bool, std::string> ReadVolumeDataFromFile(
	const std::string& filename, 
	VDBGridParams& gridParams);	


// Reads vdb grid into provided data container and also adjust voxel indices values if necessary
// RET - bool - true if data was read succesfully
// OUT - VDBGrid - vdb grid data ready to be passed to RPR
// IN  - GridBase* - vdb pointer to vdb grid
// IN - Grid bbox - can be different from current grid bbox (this is to support case with different grids having different sizes)
template <typename GridValueT>
bool ProcessVDBGrid(
	VDBGrid<GridValueT>& outGrid,
	const openvdb::GridBase* baseGrid,
	const openvdb::CoordBBox& bbox)
{
	using TGrid = const openvdb::Grid<typename openvdb::tree::Tree4<GridValueT, 5, 4, 3>::Type>;
	using TGridPtr = TGrid*;

	TGridPtr grid = static_cast<TGridPtr>(baseGrid);
	if (!grid)
		return false;

	auto& gridOnIndices = outGrid.gridOnIndices;
	std::vector<GridValueT>& gridOnValueIndices = outGrid.gridOnValueIndices;

	// prepare data container
	size_t countVoxels = baseGrid->activeVoxelCount();
	gridOnValueIndices.reserve(countVoxels);
	gridOnIndices.reserve(countVoxels * 3);

	const openvdb::Coord& lowerBound = bbox.min();

	// background value is not added by vdb automatically
	float gridBackgroundVal = grid->background();

	using TGridValueCIter = typename TGrid::ValueOnCIter;
	for (TGridValueCIter iter = grid->cbeginValueOn(); iter; ++iter)
	{
		// for RPR negative voxel indices are invalid
		openvdb::Coord curCoord = iter.getCoord();
		openvdb::Int32 x = curCoord.x() - lowerBound.x();
		openvdb::Int32 y = curCoord.y() - lowerBound.y();
		openvdb::Int32 z = curCoord.z() - lowerBound.z();

		gridOnIndices.push_back(x);
		gridOnIndices.push_back(y);
		gridOnIndices.push_back(z);

		const GridValueT& value = *iter;
		gridOnValueIndices.push_back(value + gridBackgroundVal);
	}

	grid->evalMinMax(outGrid.minValue, outGrid.maxValue);

	return true;
}


// This is for performing type check in run-time
// will be expanded as we support more grid types
// RET - bool - true if types match
// IN  - GridBase::Ptr - vdb pointer to vdb grid
template <typename T>
bool DoGridTypeMatch(const openvdb::GridBase* baseGrid);

template <> bool DoGridTypeMatch<float>(const openvdb::GridBase* baseGrid)
{
	std::string gridValueType = baseGrid->valueType();

	if (gridValueType == "float")
		return true;

	return false;
}


// Read grid dimensions into provided data container
// OUT - VDBGrid - vdb grid data ready to be passed to RPR
// IN  - GridBase::Ptr - vdb pointer to vdb grid
template <typename T>
void GetVDBGridDimensions(VDBGrid<T>& data, const openvdb::GridBase* baseGrid)
{
	// read dimesions from grid
	openvdb::Coord gridDimensions = baseGrid->evalActiveVoxelDim();

	data.gridSizeX = gridDimensions.x();
	data.gridSizeY = gridDimensions.y();
	data.gridSizeZ = gridDimensions.z();
}

// Finds grid with given name in vdb file and reads it into provided data container; returns error otherwise
// RET - bool - true if data was read succesfully
// RET - string - error description
// OUT - VDBGrid - vdb grid data ready to be passed to RPR
// IN  - openvdb::io::File - vdb file to be read (file must be opened)
// IN  - string - name of the grid to be read
template <typename GridValueT>
std::tuple<bool, std::string> ReadFileGridToVDBGrid(
	VDBGrid<GridValueT>& outGridData,
	openvdb::io::File& file, 
	const std::string& gridName)
{
	try
	{
		// ensure grids have the same size
		// - grids of different size are valid case for openvdb, but not a valid case for RPR
		openvdb::CoordBBox maxBBox;

		// read grids from file
		openvdb::GridPtrVecPtr grids = file.getGrids();
		for(auto it = grids->begin(); it != grids->end(); ++it)
		{
			// get max bbox size
			maxBBox.expand(it->get()->evalActiveVoxelBoundingBox());
		}

		// loop over all grids in the file
		for (auto it = grids->begin(); it != grids->end(); ++it)
		{
			openvdb::GridBase* pBaseGrid = it->get();

			// find grid
			std::string tmpGridName = pBaseGrid->getName();
			if (tmpGridName != gridName)
				continue;

			// ensure correct grid type
			if (!DoGridTypeMatch<GridValueT>(pBaseGrid))
				return std::make_tuple(false, "wrong grid type!");

			// save grid dimensions
			const openvdb::Coord& lowerBound = maxBBox.min();
			outGridData.size.lowerBound[0] = lowerBound.x();
			outGridData.size.lowerBound[1] = lowerBound.y();
			outGridData.size.lowerBound[2] = lowerBound.z();
			const openvdb::Coord& upperBound = maxBBox.max();
			outGridData.size.upperBound[0] = upperBound.x();
			outGridData.size.upperBound[1] = upperBound.y();
			outGridData.size.upperBound[2] = upperBound.z();

			outGridData.size.gridSizeX = upperBound.x() - lowerBound.x();
			outGridData.size.gridSizeY = upperBound.y() - lowerBound.y();
			outGridData.size.gridSizeZ = upperBound.z() - lowerBound.z();

			// save voxel size
			openvdb::math::Vec3d voxelSize = pBaseGrid->transform().voxelSize();
			outGridData.size.voxelSizeX = voxelSize.x();
			outGridData.size.voxelSizeY = voxelSize.y();
			outGridData.size.voxelSizeZ = voxelSize.z();

			// process grid according to its type
			bool success = ProcessVDBGrid<GridValueT>(outGridData, pBaseGrid, maxBBox);

			if (!success)
				return std::make_tuple(false, "failed to read grid!");

			return std::make_tuple(true, "success");
		}
	}

	catch (openvdb::Exception& ex)
	{
		// display error message
		std::string err = ex.what();

		return std::make_tuple(false, err);
	}

	return std::make_tuple(false, "no requested grid in the file!");
}

// This is the function that only reads names of the grids
// The idea is that there could be large number of grids, and we want to proceed only up to 3 at a time so there is no need to read all of the grids
// However we need to grab the names of the grids into UI
// RET - bool - true if data was read succesfully
// RET - string - error description
// IN - string - name of the file
// IN - GridParams - data container that holds grid name and grid dimensions
std::tuple<bool, std::string> ReadVolumeDataFromFile(
	const std::string& filename, 
	VDBGridParams& gridParams)
{
	// back-off
	if (filename.empty())
		return std::make_tuple(false, "bad file name!");

	// process vdb file
	// initialize openvdb; it is necessary to call it before beginning working with vdb
	openvdb::initialize();

	// create a VDB file object.
	openvdb::io::File file(filename);

	try
	{
		// open the file; this reads the file header, but not any grids.
		file.open();
	}
	catch (openvdb::IoError ex)
	{
		// display error message
		std::string err = ex.what();

		return std::make_tuple(false, err);
	}

	try
	{
		gridParams.clear();

		// loop over all grids in the file
		for (openvdb::io::File::NameIterator nameIter = file.beginName();
			nameIter != file.endName(); ++nameIter)
		{
			// read grid name from file
			std::string gridName = nameIter.gridName();

			// get grid dimensions from file (we need this for UI)
			openvdb::GridBase::Ptr baseGrid = file.readGrid(nameIter.gridName());
			openvdb::Coord gridDimensions = baseGrid->evalActiveVoxelDim();
			openvdb::math::Vec3d voxelSize = baseGrid->transform().voxelSize();

			openvdb::CoordBBox gridBBox = baseGrid->evalActiveVoxelBoundingBox();
			const openvdb::Coord& lowerBound = gridBBox.min();
			gridParams[gridName].lowerBound[0] = lowerBound.x();
			gridParams[gridName].lowerBound[1] = lowerBound.y();
			gridParams[gridName].lowerBound[2] = lowerBound.z();
			const openvdb::Coord& upperBound = gridBBox.max();
			gridParams[gridName].upperBound[0] = upperBound.x();
			gridParams[gridName].upperBound[1] = upperBound.y();
			gridParams[gridName].upperBound[2] = upperBound.z();

			gridParams[gridName].gridSizeX = upperBound.x() - lowerBound.x();
			gridParams[gridName].gridSizeY = upperBound.y() - lowerBound.y();
			gridParams[gridName].gridSizeZ = upperBound.z() - lowerBound.z();

			gridParams[gridName].voxelSizeX = voxelSize.x();
			gridParams[gridName].voxelSizeY = voxelSize.y();
			gridParams[gridName].voxelSizeZ = voxelSize.z();
		}

		// close the file.
		file.close();
	}

	catch (openvdb::Exception& ex)
	{
		// display error message
		std::string err = ex.what();

		return std::make_tuple(false, err);
	}
	
	return std::make_tuple(true, "success");
}


