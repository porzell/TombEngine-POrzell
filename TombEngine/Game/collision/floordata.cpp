#include "framework.h"
#include "Game/collision/floordata.h"

#include "Game/collision/collide_room.h"
#include "Game/collision/PointCollision.h"
#include "Game/items.h"
#include "Game/room.h"
#include "Game/Setup.h"
#include "Math/Math.h"
#include "Objects/Generic/Object/BridgeObject.h"
#include "Renderer/Renderer.h"
#include "Specific/level.h"
#include "Specific/trutils.h"

using namespace TEN::Collision::Floordata;
using namespace TEN::Collision::PointCollision;
using namespace TEN::Entities::Generic;
using namespace TEN::Math;
using namespace TEN::Utils;
using namespace TEN::Renderer;

int FloorInfo::GetSurfaceTriangleID(int x, int z, bool isFloor) const
{
	constexpr auto TRI_ID_0 = 0;
	constexpr auto TRI_ID_1 = 1;
	
	// Calculate bias.
	auto sectorPoint = GetSectorPoint(x, z).ToVector2();
	auto rotMatrix = Matrix::CreateRotationZ(TO_RAD(isFloor ? FloorSurface.SplitAngle : CeilingSurface.SplitAngle));
	float bias = Vector2::Transform(sectorPoint, rotMatrix).x;

	// Return triangle ID according to bias.
	return ((bias < 0.0f) ? TRI_ID_0 : TRI_ID_1);
}

const SectorSurfaceTriangleData& FloorInfo::GetSurfaceTriangle(int x, int z, bool isFloor) const
{
	// Get triangles.
	int triID = GetSurfaceTriangleID(x, z, isFloor);
	const auto& tris = isFloor ? FloorSurface.Triangles : CeilingSurface.Triangles;

	// Return triangle.
	return tris[triID];
}

Vector3 FloorInfo::GetSurfaceNormal(int triID, bool isFloor) const
{
	// Get triangle.
	const auto& tris = isFloor ? FloorSurface.Triangles : CeilingSurface.Triangles;
	const auto& tri = tris[triID];

	// Return plane normal.
	return tri.Plane.Normal();
}

Vector3 FloorInfo::GetSurfaceNormal(int x, int z, bool isFloor) const
{
	int triID = GetSurfaceTriangleID(x, z, isFloor);
	return GetSurfaceNormal(triID, isFloor);
}

short FloorInfo::GetSurfaceIllegalSlopeAngle(int x, int z, bool isFloor) const
{
	const auto& tri = GetSurfaceTriangle(x, z, isFloor);
	return tri.IllegalSlopeAngle;
}

MaterialType FloorInfo::GetSurfaceMaterial(int x, int z, bool isFloor) const
{
	const auto& tri = GetSurfaceTriangle(x, z, isFloor);
	return tri.Material;
}

bool FloorInfo::IsSurfaceSplit(bool isFloor) const
{
	const auto& tris = isFloor ? FloorSurface.Triangles : CeilingSurface.Triangles;

	// Check if surface planes aren't equal or portal is split.
	bool arePlanesEqual = (tris[0].Plane == tris[1].Plane);
	return (!arePlanesEqual || IsSurfaceSplitPortal(isFloor));
}

bool FloorInfo::IsSurfaceDiagonalStep(bool isFloor) const
{
	// 1) Test if surface is split.
	if (!IsSurfaceSplit(isFloor))
		return false;

	const auto& surface = isFloor ? FloorSurface : CeilingSurface;
	
	// 2) Test if plane distances are equal.
	// TODO: This check will fail if distances are equal but planes criss-cross. Update this for improved TE geometry building in future.
	float dist0 = surface.Triangles[0].Plane.D();
	float dist1 = surface.Triangles[1].Plane.D();
	if (dist0 == dist1)
		return false;

	// 3) Test if split angle is aligned diagonal. NOTE: Non-split surfaces default to 0 degrees.
	if (surface.SplitAngle != SectorSurfaceData::SPLIT_ANGLE_0 &&
		surface.SplitAngle != SectorSurfaceData::SPLIT_ANGLE_1)
	{
		return false;
	}

	return true;
}

bool FloorInfo::IsSurfaceSplitPortal(bool isFloor) const
{
	// Test if surface triangle portals are not equal.
	const auto& tris = isFloor ? FloorSurface.Triangles : CeilingSurface.Triangles;
	return (tris[0].PortalRoomNumber != tris[1].PortalRoomNumber);
}

std::optional<int> FloorInfo::GetNextRoomNumber(int x, int z, bool isBelow) const
{
	// Get triangle.
	int triID = GetSurfaceTriangleID(x, z, isBelow);
	const auto& surface = isBelow ? FloorSurface : CeilingSurface;
	const auto& tri = surface.Triangles[triID];

	// Return portal room number below or above if it exists.
	if (tri.PortalRoomNumber != NO_ROOM)
		return tri.PortalRoomNumber;

	return std::nullopt;
}

std::optional<int> FloorInfo::GetNextRoomNumber(const Vector3i& pos, bool isBelow) const
{
	// 1) Get sector floor and ceiling heights.
	int floorHeight = GetSurfaceHeight(pos.x, pos.z, true);
	int ceilingHeight = GetSurfaceHeight(pos.x, pos.z, false);

	// 2) Run through bridges in sector to test access to room below or above.
	for (int itemNumber : BridgeItemNumbers)
	{
		const auto& bridgeItem = g_Level.Items[itemNumber];
		const auto& bridge = GetBridgeObject(bridgeItem);

		// 2.1) Get bridge floor or ceiling height.
		auto bridgeSurfaceHeight = isBelow ? bridge.GetFloorHeight(bridgeItem, pos) : bridge.GetCeilingHeight(bridgeItem, pos);
		if (!bridgeSurfaceHeight.has_value())
			continue;

		// 2.2) Test if bridge blocks access to room below or above.
		// TODO: Check for potential edge case inaccuracies.
		if (isBelow ?
			*bridgeSurfaceHeight >= pos.y : // Bridge floor height is below current position.
			*bridgeSurfaceHeight <= pos.y)	// Bridge ceiling height is above current position.
		{
			// Test if bridge surface is inside sector.
			if (*bridgeSurfaceHeight <= floorHeight && // Bridge floor height is above sector floor height.
				*bridgeSurfaceHeight >= ceilingHeight) // Bridge floor height is below sector ceiling height.
			{
				return std::nullopt;
			}
		}
	}

	// 3) Get and return room number below or above.
	return GetNextRoomNumber(pos.x, pos.z, isBelow);
}

std::optional<int> FloorInfo::GetSideRoomNumber() const
{
	// Return side portal room number if it exists.
	// TODO: Check how side portals work when a sector connects to multiple side rooms.
	if (SidePortalRoomNumber != NO_ROOM)
		return SidePortalRoomNumber;

	return std::nullopt;
}

int FloorInfo::GetSurfaceHeight(int x, int z, bool isFloor) const
{
	// Get triangle.
	const auto& tri = GetSurfaceTriangle(x, z, isFloor);

	// Calculate relative plane height at intersection using plane equation.
	auto sectorPoint = GetSectorPoint(x, z);
	auto normal = tri.Plane.Normal();
	float relPlaneHeight = -((normal.x * sectorPoint.x) + (normal.z * sectorPoint.y)) / normal.y;

	// Return sector floor or ceiling height. NOTE: Bridges ignored.
	return (tri.Plane.D() + relPlaneHeight);
}

int FloorInfo::GetSurfaceHeight(const Vector3i& pos, bool isFloor) const
{
	// 1) Get sector floor and ceiling heights.
	int floorHeight = GetSurfaceHeight(pos.x, pos.z, true);
	int ceilingHeight = GetSurfaceHeight(pos.x, pos.z, false);

	// 2) Run through bridges in sector to find potential closer surface height.
	for (int itemNumber : BridgeItemNumbers)
	{
		const auto& bridgeItem = g_Level.Items[itemNumber];
		const auto& bridge = GetBridgeObject(bridgeItem);

		// 2.1) Get bridge surface height.
		auto bridgeSurfaceHeight = isFloor ? bridge.GetFloorHeight(bridgeItem, pos) : bridge.GetCeilingHeight(bridgeItem, pos);
		if (!bridgeSurfaceHeight.has_value())
			continue;

		// 2.2) Track closest floor or ceiling height.
		if (isFloor)
		{
			// Test if bridge floor height is closer.
			if (*bridgeSurfaceHeight >= pos.y &&	   // Bridge floor height is below position.
				*bridgeSurfaceHeight < floorHeight &&  // Bridge floor height is above current closest floor height.
				*bridgeSurfaceHeight >= ceilingHeight) // Bridge ceiling height is below sector ceiling height.
			{
				floorHeight = *bridgeSurfaceHeight;
			}
		}
		else
		{
			// Test if bridge ceiling height is closer.
			if (*bridgeSurfaceHeight <= pos.y &&		// Bridge ceiling height is above position.
				*bridgeSurfaceHeight > ceilingHeight && // Bridge ceiling height is below current closest ceiling height.
				*bridgeSurfaceHeight <= floorHeight)	// Bridge floor height is above sector floor height.
			{
				ceilingHeight = *bridgeSurfaceHeight;
			}
		}
	}

	// 3) Return floor or ceiling height. NOTE: Bridges considered.
	return (isFloor ? floorHeight : ceilingHeight);
}

int FloorInfo::GetBridgeSurfaceHeight(const Vector3i& pos, bool isFloor) const
{
	// 1) Find and return intersected bridge floor or ceiling height (if applicable).
	for (int itemNumber : BridgeItemNumbers)
	{
		const auto& bridgeItem = g_Level.Items[itemNumber];
		const auto& bridge = GetBridgeObject(bridgeItem);

		// 1.1) Get bridge floor and ceiling heights.
		auto floorHeight = bridge.GetFloorHeight(bridgeItem, pos);
		auto ceilingHeight = bridge.GetCeilingHeight(bridgeItem, pos);
		if (!floorHeight.has_value() || !ceilingHeight.has_value())
			continue;

		// 1.2) If position is inside bridge, return bridge floor or ceiling height.
		if (isFloor)
		{
			// Test for bridge intersection.
			if (pos.y > *floorHeight &&	 // Position is below bridge floor height.
				pos.y <= *ceilingHeight) // Position is above bridge ceiling height.
			{
				return *floorHeight;
			}
		}
		else
		{
			// Test for bridge intersection.
			if (pos.y >= *floorHeight && // Position is below bridge floor height.
				pos.y < *ceilingHeight)	 // Position is above bridge ceiling height.
			{
				return *ceilingHeight;
			}
		}
	}
	
	// 2) Get and return closest floor or ceiling height.
	return GetSurfaceHeight(pos, isFloor);
}

bool FloorInfo::IsWall(int triID) const
{
	const auto& floorTri = FloorSurface.Triangles[triID];
	const auto& ceilTri = CeilingSurface.Triangles[triID];

	bool areSplitAnglesEqual = (FloorSurface.SplitAngle == CeilingSurface.SplitAngle);
	bool areNormalsParallel = (floorTri.Plane.Normal() == -ceilTri.Plane.Normal());
	bool areDistsEqual = (floorTri.Plane.D() == ceilTri.Plane.D());

	return (areSplitAnglesEqual && areNormalsParallel && areDistsEqual);
}

bool FloorInfo::IsWall(int x, int z) const
{
	int triID = GetSurfaceTriangleID(x, z, true);
	return IsWall(triID);
}

int FloorInfo::GetInsideBridgeItemNumber(const Vector3i& pos, bool testFloorBorder, bool testCeilingBorder) const
{
	// 1) Find and return intersected bridge item number (if applicable).
	for (int itemNumber : BridgeItemNumbers)
	{
		const auto& bridgeItem = g_Level.Items[itemNumber];
		const auto& bridge = GetBridgeObject(bridgeItem);

		// 1.1) Get bridge floor and ceiling heights.
		auto floorHeight = bridge.GetFloorHeight(bridgeItem, pos);
		auto ceilingHeight = bridge.GetCeilingHeight(bridgeItem, pos);
		if (!floorHeight.has_value() || !ceilingHeight.has_value())
			continue;

		// 1.2) Test for bridge intersection.
		if (pos.y > *floorHeight && // Position is below bridge floor height.
			pos.y < *ceilingHeight) // Position is above bridge ceiling height.
		{
			return itemNumber;
		}

		// TODO: Check what this does.
		// 1.3) Test bridge floor and ceiling borders (if applicable).
		if ((testFloorBorder && pos.y == *floorHeight) ||	// Position matches floor height.
			(testCeilingBorder && pos.y == *ceilingHeight)) // Position matches ceiling height.
		{
			return itemNumber;
		}
	}

	// 2) No bridge intersection; return invalid item number.
	return NO_ITEM;
}

void FloorInfo::AddBridge(int itemNumber)
{
	BridgeItemNumbers.insert(itemNumber);
}

void FloorInfo::RemoveBridge(int itemNumber)
{
	BridgeItemNumbers.erase(itemNumber);
}

namespace TEN::Collision::Floordata
{
	struct FarthestHeightData
	{
		FloorInfo& Sector;
		int		   Height = 0;
	};

	// NOTE: Tilts are deprecated, but until all conversions are complete this function will remain useful.
	Vector2i GetSurfaceTilt(const Vector3& normal, bool isFloor)
	{
		// Scale normal to original fake plane length.
		float scaleFactor = 1.0f / normal.y;
		auto scaledNormal = normal * scaleFactor;

		// Calculate and return tilt.
		auto sign = isFloor ? 1 : -1;
		return Vector2i(
			round(scaledNormal.x * 4),
			round(scaledNormal.z * 4)) * sign;
	}

	Vector2i GetSectorPoint(int x, int z)
	{
		constexpr auto HALF_BLOCK = (int)BLOCK(0.5f);

		// Return relative 2D point in range [0, BLOCK(1)).
		return Vector2i(
			(x % BLOCK(1)) - HALF_BLOCK,
			(z % BLOCK(1)) - HALF_BLOCK);
	}

	Vector2i GetRoomGridCoord(int roomNumber, int x, int z, bool clampToBounds)
	{
		const auto& room = g_Level.Rooms[roomNumber];

		// Calculate room grid coord.
		auto roomGridCoord = Vector2i((x - room.x) / BLOCK(1), (z - room.z) / BLOCK(1));
		if (x < room.x)
			roomGridCoord.x -= 1;
		if (z < room.z)
			roomGridCoord.y -= 1;

		// Clamp room grid coord to room bounds (if applicable).
		if (clampToBounds)
		{
			roomGridCoord.x = std::clamp(roomGridCoord.x, 0, room.xSize - 1);
			roomGridCoord.y = std::clamp(roomGridCoord.y, 0, room.zSize - 1);
		}

		return roomGridCoord;
	}

	std::vector<Vector2i> GetNeighborRoomGridCoords(const Vector3i& pos, int roomNumber, unsigned int searchDepth)
	{
		auto originRoomGridCoord = GetRoomGridCoord(roomNumber, pos.x, pos.z, false);

		// Determine search area bounds.
		int xMax = originRoomGridCoord.x + searchDepth;
		int xMin = originRoomGridCoord.x - searchDepth;
		int zMax = originRoomGridCoord.y + searchDepth;
		int zMin = originRoomGridCoord.y - searchDepth;

		const auto& room = g_Level.Rooms[roomNumber];

		// Search area out of range; return empty vector.
		if (xMax <= 0 || xMin >= (room.xSize - 1) ||
			xMax <= 0 || xMin >= (room.xSize - 1))
		{
			return {};
		}

		// Collect room grid coords.
		auto roomGridCoords = std::vector<Vector2i>{};
		for (int x = xMin; x <= xMax; x++)
		{
			// Test if out of room X range.
			if (x <= 0 || x >= (room.xSize - 1))
				continue;

			for (int z = zMin; z <= zMax; z++)
			{
				// Test if out of room Z range.
				if (z <= 0 || z >= (room.zSize - 1))
					continue;

				roomGridCoords.push_back(Vector2i(x, z));
			}
		}

		return roomGridCoords;
	}

	std::vector<FloorInfo*> GetNeighborSectorPtrs(const Vector3i& pos, int roomNumber, unsigned int searchDepth)
	{
		auto sectorPtrs = std::vector<FloorInfo*>{};

		// Run through neighbor rooms.
		auto& room = g_Level.Rooms[roomNumber];
		for (int neighborRoomNumber : room.neighbors)
		{
			// Collect neighbor sector pointers.
			auto roomGridCoords = GetNeighborRoomGridCoords(pos, neighborRoomNumber, searchDepth);
			for (const auto& roomGridCoord : roomGridCoords)
				sectorPtrs.push_back(&GetFloor(neighborRoomNumber, roomGridCoord));
		}

		// Return neighbor sector pointers.
		return sectorPtrs;
	}

	FloorInfo& GetFloor(int roomNumber, const Vector2i& roomGridCoord)
	{
		auto& room = g_Level.Rooms[roomNumber];

		int sectorID = (room.zSize * roomGridCoord.x) + roomGridCoord.y;
		return room.floor[sectorID];
	}

	FloorInfo& GetFloor(int roomNumber, int x, int z)
	{
		auto roomGridCoord = GetRoomGridCoord(roomNumber, x, z);
		return GetFloor(roomNumber, roomGridCoord);
	}

	FloorInfo& GetFarthestSector(int roomNumber, int x, int z, bool isBottom)
	{
		auto* sectorPtr = &GetSideSector(roomNumber, x, z);

		// Find bottom or top sector.
		bool isWall = sectorPtr->IsWall(x, z);
		while (isWall)
		{
			auto nextRoomNumber = sectorPtr->GetNextRoomNumber(x, z, isBottom);
			if (!nextRoomNumber.has_value())
				break;

			// TODO: Check.
			sectorPtr = &GetSideSector(*nextRoomNumber, x, z);
			isWall = sectorPtr->IsWall(x, z);
		}

		return *sectorPtr;
	}

	FloorInfo& GetSideSector(int roomNumber, int x, int z)
	{
		auto* sectorPtr = &GetFloor(roomNumber, x, z);

		// Find side sector.
		auto sideRoomNumber = sectorPtr->GetSideRoomNumber();
		while (sideRoomNumber.has_value())
		{
			sectorPtr = &GetFloor(*sideRoomNumber, x, z);
			sideRoomNumber = sectorPtr->GetSideRoomNumber();
		}

		return *sectorPtr;
	}

	static std::optional<FarthestHeightData> GetFarthestHeightData(FloorInfo& currentSector, Vector3i pos, bool isBottom)
	{
		// Find bottom or top height while bridge exists(?).
		auto* sectorPtr = &currentSector;
		do
		{
			// Set vertical position to lowest bridge ceiling height or highest bridge floor height.
			pos.y = sectorPtr->GetBridgeSurfaceHeight(pos, !isBottom);

			// Find sector at lowest bridge floor height or highest bridge ceiling height.
			while (isBottom ?
				(pos.y >= sectorPtr->GetSurfaceHeight(pos.x, pos.z, true)) :
				(pos.y <= sectorPtr->GetSurfaceHeight(pos.x, pos.z, false)))
			{
				auto nextRoomNumber = sectorPtr->GetNextRoomNumber(pos.x, pos.z, isBottom);
				if (!nextRoomNumber.has_value())
					return std::nullopt;

				sectorPtr = &GetSideSector(*nextRoomNumber, pos.x, pos.z);
			}
		}
		while (sectorPtr->GetInsideBridgeItemNumber(pos, isBottom, !isBottom) != NO_ITEM);

		return FarthestHeightData{ *sectorPtr, pos.y };
	}

	std::optional<int> GetSurfaceHeight(const RoomVector& location, int x, int z, bool isFloor)
	{
		auto* sectorPtr = &GetSideSector(location.RoomNumber, x, z);

		auto pos = Vector3i(x, location.Height, z);
		int polarity = 0;

		if (sectorPtr->IsWall(x, z))
		{
			sectorPtr = &GetFarthestSector(location.RoomNumber, x, z, !isFloor);

			if (!sectorPtr->IsWall(x, z))
			{
				pos.y = sectorPtr->GetSurfaceHeight(x, z, isFloor);
				polarity = isFloor ? -1 : 1;
			}
			else
			{
				sectorPtr = &GetFarthestSector(location.RoomNumber, x, z, isFloor);

				if (!sectorPtr->IsWall(x, z))
				{
					pos.y = sectorPtr->GetSurfaceHeight(x, z, !isFloor);
					polarity = isFloor ? 1 : -1;
				}
				else
				{
					return std::nullopt;
				}
			}
		}

		int floorHeight = sectorPtr->GetSurfaceHeight(pos, true);
		int ceilingHeight = sectorPtr->GetSurfaceHeight(pos, false);

		pos.y = std::clamp(pos.y, std::min(floorHeight, ceilingHeight), std::max(floorHeight, ceilingHeight));

		bool testFloorBorder = (pos.y == ceilingHeight);
		bool testCeilBorder = (pos.y == floorHeight);
		int insideBridgeItemNumber = sectorPtr->GetInsideBridgeItemNumber(pos, testFloorBorder, testCeilBorder);

		if (insideBridgeItemNumber != NO_ITEM)
		{
			if (isFloor ? (polarity <= 0) : (polarity >= 0))
			{
				auto heightData = GetFarthestHeightData(*sectorPtr, pos, !isFloor);
				if (heightData.has_value())
					return heightData->Height;
			}

			if (isFloor ? (polarity >= 0) : (polarity <= 0))
			{
				auto heightData = GetFarthestHeightData(*sectorPtr, pos, isFloor);
				if (!heightData.has_value())
					return std::nullopt;

				sectorPtr = &heightData->Sector;
				pos.y = heightData->Height;
			}
		}

		if (isFloor ? (polarity >= 0) : (polarity <= 0))
		{
			auto nextRoomNumber = sectorPtr->GetNextRoomNumber(pos, isFloor);
			while (nextRoomNumber.has_value())
			{
				sectorPtr = &GetSideSector(*nextRoomNumber, x, z);
				nextRoomNumber = sectorPtr->GetNextRoomNumber(pos, isFloor);
			}
		}

		return sectorPtr->GetSurfaceHeight(pos, isFloor);
	}

	static std::optional<RoomVector> GetFarthestRoomVector(RoomVector location, const Vector3i& pos, bool isBottom)
	{
		auto* sectorPtr = &GetSideSector(location.RoomNumber, pos.x, pos.z);
		location.RoomNumber = sectorPtr->RoomNumber;

		if (sectorPtr->IsWall(pos.x, pos.z))
		{
			sectorPtr = &GetFarthestSector(location.RoomNumber, pos.x, pos.z, isBottom);
			location.RoomNumber = sectorPtr->RoomNumber;

			if (sectorPtr->IsWall(pos.x, pos.z))
				return std::nullopt;

			location.Height = sectorPtr->GetSurfaceHeight(pos.x, pos.z, !isBottom);
		}

		int floorHeight = sectorPtr->GetSurfaceHeight(Vector3i(pos.x, location.Height, pos.z), true);
		int ceilingHeight = sectorPtr->GetSurfaceHeight(Vector3i(pos.x, location.Height, pos.z), false);

		location.Height = std::clamp(location.Height, std::min(ceilingHeight, floorHeight), std::max(ceilingHeight, floorHeight));

		bool testFloorBorder = (location.Height == ceilingHeight);
		bool testCeilBorder = (location.Height == floorHeight);
		int insideBridgeItemNumber = sectorPtr->GetInsideBridgeItemNumber(Vector3i(pos.x, location.Height, pos.z), testFloorBorder, testCeilBorder);

		if (insideBridgeItemNumber != NO_ITEM)
		{
			auto heightData = GetFarthestHeightData(*sectorPtr, Vector3i(pos.x, location.Height, pos.z), isBottom);
			if (!heightData.has_value())
				return std::nullopt;

			sectorPtr = &heightData->Sector;
			location.RoomNumber = sectorPtr->RoomNumber;
			location.Height = heightData->Height;
		}

		bool isFirstSector = true;
		auto nextRoomNumber = std::optional<int>(location.RoomNumber);
		while (nextRoomNumber.has_value())
		{
			if (!isFirstSector)
			{
				sectorPtr = &GetSideSector(*nextRoomNumber, pos.x, pos.z);
				location.RoomNumber = sectorPtr->RoomNumber;
				location.Height = sectorPtr->GetSurfaceHeight(pos.x, pos.z, !isBottom);
			}
			isFirstSector = false;

			if (isBottom)
			{
				ceilingHeight = sectorPtr->GetSurfaceHeight(Vector3i(pos.x, location.Height, pos.z), false);
				if (pos.y < ceilingHeight && sectorPtr->GetNextRoomNumber(Vector3i(pos.x, location.Height, pos.z), false))
					return std::nullopt;

				floorHeight = sectorPtr->GetSurfaceHeight(Vector3i(pos.x, location.Height, pos.z), true);
				if (pos.y <= floorHeight)
				{
					location.Height = std::max(pos.y, ceilingHeight);
					return location;
				}
			}
			else
			{
				floorHeight = sectorPtr->GetSurfaceHeight(Vector3i(pos.x, location.Height, pos.z), true);
				if (pos.y > floorHeight && sectorPtr->GetNextRoomNumber(Vector3i(pos.x, location.Height, pos.z), true))
					return std::nullopt;

				ceilingHeight = sectorPtr->GetSurfaceHeight(Vector3i(pos.x, location.Height, pos.z), false);
				if (pos.y >= ceilingHeight)
				{
					location.Height = std::min(pos.y, floorHeight);
					return location;
				}
			}

			nextRoomNumber = sectorPtr->GetNextRoomNumber(Vector3i(pos.x, location.Height, pos.z), isBottom);
		}

		return std::nullopt;
	}

	RoomVector GetRoomVector(RoomVector location, const Vector3i& pos)
	{
		auto locationBelow = GetFarthestRoomVector(location, pos, true);
		if (locationBelow.has_value())
			return *locationBelow;

		auto locationAbove = GetFarthestRoomVector(location, pos, false);
		if (locationAbove.has_value())
			return *locationAbove;

		return location;
	}

	void AddBridge(int itemNumber, int x, int z)
	{
		const auto& bridgeItem = g_Level.Items[itemNumber];
		const auto& bridge = GetBridgeObject(bridgeItem);

		if (!Objects.CheckID(bridgeItem.ObjectNumber))
			return;

		x += bridgeItem.Pose.Position.x;
		z += bridgeItem.Pose.Position.z;

		auto* sectorPtr = &GetSideSector(bridgeItem.RoomNumber, x, z);
		sectorPtr->AddBridge(itemNumber);

		if (bridge.GetFloorBorder != nullptr)
		{
			int floorBorder = bridge.GetFloorBorder(bridgeItem);
			while (floorBorder <= sectorPtr->GetSurfaceHeight(x, z, false))
			{
				auto roomNumberAbove = sectorPtr->GetNextRoomNumber(x, z, false);
				if (!roomNumberAbove.has_value())
					break;

				sectorPtr = &GetSideSector(*roomNumberAbove, x, z);
				sectorPtr->AddBridge(itemNumber);
			}
		}
		
		if (bridge.GetCeilingBorder != nullptr)
		{
			int ceilingBorder = bridge.GetCeilingBorder(bridgeItem);
			while (ceilingBorder >= sectorPtr->GetSurfaceHeight(x, z, true))
			{
				auto roomNumberBelow = sectorPtr->GetNextRoomNumber(x, z, true);
				if (!roomNumberBelow.has_value())
					break;

				sectorPtr = &GetSideSector(*roomNumberBelow, x, z);
				sectorPtr->AddBridge(itemNumber);
			}
		}
	}

	void RemoveBridge(int itemNumber, int x, int z)
	{
		const auto& bridgeItem = g_Level.Items[itemNumber];
		const auto& bridge = GetBridgeObject(bridgeItem);

		if (!Objects.CheckID(bridgeItem.ObjectNumber))
			return;

		x += bridgeItem.Pose.Position.x;
		z += bridgeItem.Pose.Position.z;

		auto* sectorPtr = &GetSideSector(bridgeItem.RoomNumber, x, z);
		sectorPtr->RemoveBridge(itemNumber);

		if (bridge.GetFloorBorder != nullptr)
		{
			int floorBorder = bridge.GetFloorBorder(bridgeItem);
			while (floorBorder <= sectorPtr->GetSurfaceHeight(x, z, false))
			{
				auto roomNumberAbove = sectorPtr->GetNextRoomNumber(x, z, false);
				if (!roomNumberAbove.has_value())
					break;

				sectorPtr = &GetSideSector(*roomNumberAbove, x, z);
				sectorPtr->RemoveBridge(itemNumber);
			}
		}

		if (bridge.GetCeilingBorder != nullptr)
		{
			int ceilingBorder = bridge.GetCeilingBorder(bridgeItem);
			while (ceilingBorder >= sectorPtr->GetSurfaceHeight(x, z, true))
			{
				auto roomNumberBelow = sectorPtr->GetNextRoomNumber(x, z, true);
				if (!roomNumberBelow.has_value())
					break;

				sectorPtr = &GetSideSector(*roomNumberBelow, x, z);
				sectorPtr->RemoveBridge(itemNumber);
			}
		}
	}

	// Get precise floor/ceiling height from object's bounding box.
	// Animated objects are also supported, although horizontal collision shifting is unstable.
	// Method: get accurate bounds in world transform by converting to OBB, then do a ray test
	// on top or bottom (depending on test side) to determine if box is present at a particular point.
	std::optional<int> GetBridgeItemIntersect(const ItemInfo& item, const Vector3i& pos, bool useBottomHeight)
	{
		constexpr auto VERTICAL_MARGIN = 4;

		auto box = GameBoundingBox(&item).ToBoundingOrientedBox(item.Pose);
		
		auto origin = Vector3(pos.x, pos.y + (useBottomHeight ? VERTICAL_MARGIN : -VERTICAL_MARGIN), pos.z);
		auto dir = useBottomHeight ? -Vector3::UnitY : Vector3::UnitY;

		// Ray intersects box; return bridge box height.
		float dist = 0.0f;
		if (box.Intersects(origin, dir, dist))
			return Geometry::TranslatePoint(origin, dir, dist).y;

		return std::nullopt;
	}

	// Gets bridge min or max height regardless of actual X/Z world position.
	int GetBridgeBorder(const ItemInfo& item, bool isBottom)
	{
		auto bounds = GameBoundingBox(&item);
		return (item.Pose.Position.y + (isBottom ? bounds.Y2 : bounds.Y1));
	}

	// Updates BridgeItem for all blocks which are enclosed by bridge bounds.
	void UpdateBridgeItem(const ItemInfo& item, bool forceRemoval)
	{
		constexpr auto SECTOR_EXTENTS = Vector3(BLOCK(0.5f));

		if (!Objects.CheckID(item.ObjectNumber))
			return;

		if (!Objects[item.ObjectNumber].loaded)
			return;

		// Force removal if item was killed.
		if (item.Flags & IFLAG_KILLED)
			forceRemoval = true;

		// Get bridge OBB.
		auto bridgeBox = GameBoundingBox(&item).ToBoundingOrientedBox(item.Pose);

		// Get bridge OBB corners. NOTE: only 0, 1, 4, 5 are relevant.
		auto corners = std::array<Vector3, 8>{};
		bridgeBox.GetCorners(corners.data());

		const auto& room = g_Level.Rooms[item.RoomNumber];

		// Get projected AABB min and max of bridge OBB.
		float xMin = floor((std::min(std::min(std::min(corners[0].x, corners[1].x), corners[4].x), corners[5].x) - room.x) / BLOCK(1));
		float zMin = floor((std::min(std::min(std::min(corners[0].z, corners[1].z), corners[4].z), corners[5].z) - room.z) / BLOCK(1));
		float xMax =  ceil((std::max(std::max(std::max(corners[0].x, corners[1].x), corners[4].x), corners[5].x) - room.x) / BLOCK(1));
		float zMax =  ceil((std::max(std::max(std::max(corners[0].z, corners[1].z), corners[4].z), corners[5].z) - room.z) / BLOCK(1));

		// Run through sectors enclosed in projected bridge AABB.
		for (int x = 0; x < room.xSize; x++)
		{
			for (int z = 0; z < room.zSize; z++)
			{
				float pX = (room.x + BLOCK(x)) + BLOCK(0.5f);
				float pZ = (room.z + BLOCK(z)) + BLOCK(0.5f);
				float offX = pX - item.Pose.Position.x;
				float offZ = pZ - item.Pose.Position.z;

				// Clean previous bridge state.
				RemoveBridge(item.Index, offX, offZ);

				// In sweep mode; don't try readding to sector.
				if (forceRemoval)
					continue;

				// Sector is outside enclosed AABB space; ignore precise check.
				if (x < xMin || z < zMin ||
					x > xMax || z > zMax)
				{
					continue;
				}

				// Sector is in enclosed bridge AABB space; do more precise test.
				// Construct OBB within same plane as bridge OBB and test intersection.
				auto sectorBox = BoundingOrientedBox(Vector3(pX, bridgeBox.Center.y, pZ), SECTOR_EXTENTS, Vector4::UnitY);

				// Add bridge to current sector if intersection is valid.
				if (bridgeBox.Intersects(sectorBox))
					AddBridge(item.Index, offX, offZ);
			}
		}
	}

	bool TestMaterial(MaterialType refMaterial, const std::vector<MaterialType>& materials)
	{
		return Contains(materials, refMaterial);
	}

	static void DrawSectorFlagLabel(const Vector3& pos, const std::string& string, const Vector4& color, float verticalOffset)
	{
		constexpr auto LABEL_SCALE = 0.8f;
		constexpr auto HALF_BLOCK  = BLOCK(0.5f);

		// Get 2D label position.
		auto labelPos = pos + Vector3(HALF_BLOCK, 0.0f, HALF_BLOCK);
		auto labelPos2D = g_Renderer.Get2DPosition(labelPos);

		// Draw label.
		if (labelPos2D.has_value())
		{
			*labelPos2D += Vector2(0.0f, verticalOffset);
			g_Renderer.AddDebugString(string, *labelPos2D, color, LABEL_SCALE, 0, RendererDebugPage::CollisionStats);
		}
	}

	void DrawNearbySectorFlags(const ItemInfo& item)
	{
		constexpr auto SECTOR_SEARCH_DEPTH = 2;
		constexpr auto STRING_SPACING	   = -20.0f;

		constexpr auto STOPPER_COLOR				 = Vector4(1.0f, 0.4f, 0.4f, 1.0f);
		constexpr auto DEATH_COLOR					 = Vector4(0.4f, 1.0f, 0.4f, 1.0f);
		constexpr auto MONKEY_SWING_COLOR			 = Vector4(1.0f, 0.4f, 0.4f, 1.0f);
		constexpr auto BEETLE_MINECART_RIGHT_COLOR	 = Vector4(0.4f, 0.4f, 1.0f, 1.0f);
		constexpr auto ACTIVATOR_MINECART_LEFT_COLOR = Vector4(1.0f, 0.4f, 1.0f, 1.0f);
		constexpr auto MINECART_STOP_COLOR			 = Vector4(0.4f, 1.0f, 1.0f, 1.0f);

		// Get point collision.
		auto pointColl = GetPointCollision(item);
		auto pos = item.Pose.Position.ToVector3();

		// Run through neighboring rooms.
		const auto& room = g_Level.Rooms[item.RoomNumber];
		for (int neighborRoomNumber : room.neighbors)
		{
			const auto& neighborRoom = g_Level.Rooms[neighborRoomNumber];

			// Run through neighbor sectors.
			auto roomGridCoords = GetNeighborRoomGridCoords(item.Pose.Position, neighborRoomNumber, SECTOR_SEARCH_DEPTH);
			for (const auto& roomGridCoord : roomGridCoords)
			{
				pos.x = BLOCK(roomGridCoord.x) + neighborRoom.x;
				pos.z = BLOCK(roomGridCoord.y) + neighborRoom.z;

				pointColl = GetPointCollision(pos, neighborRoomNumber);
				pos.y = pointColl.GetFloorHeight();

				float verticalOffset = STRING_SPACING;

				// Stopper
				if (pointColl.GetSector().Stopper)
				{
					DrawSectorFlagLabel(pos, "Stopper", STOPPER_COLOR, verticalOffset);
					verticalOffset += STRING_SPACING;
				}

				// Death
				if (pointColl.GetSector().Flags.Death)
				{
					DrawSectorFlagLabel(pos, "Death", DEATH_COLOR, verticalOffset);
					verticalOffset += STRING_SPACING;
				}

				// Monkey Swing
				if (pointColl.GetSector().Flags.Monkeyswing)
				{
					DrawSectorFlagLabel(pos, "Monkey Swing", MONKEY_SWING_COLOR, verticalOffset);
					verticalOffset += STRING_SPACING;
				}

				// Beetle / Minecart Right
				if (pointColl.GetSector().Flags.MarkBeetle)
				{
					auto labelString = std::string("Beetle") + (!pointColl.GetSector().Flags.MinecartStop() ? " / Minecart Right" : "");
					DrawSectorFlagLabel(pos, labelString, BEETLE_MINECART_RIGHT_COLOR, verticalOffset);
					verticalOffset += STRING_SPACING;
				}

				// Activator / Minecart Left
				if (pointColl.GetSector().Flags.MarkTriggerer)
				{
					auto labelString = std::string("Activator") + (!pointColl.GetSector().Flags.MinecartStop() ? " / Minecart Left" : "");
					DrawSectorFlagLabel(pos, labelString, ACTIVATOR_MINECART_LEFT_COLOR, verticalOffset);
					verticalOffset += STRING_SPACING;
				}

				// Minecart Stop
				if (pointColl.GetSector().Flags.MinecartStop())
				{
					DrawSectorFlagLabel(pos, "Minecart Stop", MINECART_STOP_COLOR, verticalOffset);
					verticalOffset += STRING_SPACING;
				}
			}
		}
	}
}
