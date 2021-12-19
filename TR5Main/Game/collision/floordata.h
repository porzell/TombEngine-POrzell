#pragma once
#include <optional>
#include "Specific\trmath.h"
#include "roomvector.h"

constexpr auto WALL_PLANE = Vector3(0, 0, -CLICK(127));

enum class FLOOR_MATERIAL : unsigned char
{
	Mud = 0,
	Snow = 1,
	Sand = 2,
	Gravel = 3,
	Ice = 4,
	Water = 5,
	Stone = 6,
	Wood = 7,
	Metal = 8,
	Marble = 9,
	Grass = 10,
	Concrete = 11,
	OldWood = 12,
	OldMetal = 13,
	Unknown14 = 14,
	Unknown15 = 15
};


struct SECTOR_COLLISION_INFO
{
	float SplitAngle;
	int Portals[2];
	Vector3 Planes[2];
};

struct SECTOR_FLAGS
{
	bool Death;
	bool Monkeyswing;
	bool ClimbNorth;
	bool ClimbSouth;
	bool ClimbWest;
	bool ClimbEast;
	bool MarkBeetle;

	bool MarkTriggerer;
	bool MarkTriggererActive; // TODO: IT NEEDS TO BE WRITTEN/READ FROM SAVEGAMES!
};

class FLOOR_INFO
{
	public:
		int Room;
		SECTOR_COLLISION_INFO FloorCollision;
		SECTOR_COLLISION_INFO CeilingCollision;
		int WallPortal;
		std::set<short> BridgeItem;

		int Box;
		bool Stopper;
		int TriggerIndex;
		FLOOR_MATERIAL Material;
		SECTOR_FLAGS Flags;

		int SectorPlane(int x, int z) const;
		int SectorPlaneCeiling(int x, int z) const;
		std::pair<int, int> FLOOR_INFO::TiltXZ(int x, int z) const;
		bool FloorIsSplit() const;
		bool FloorIsDiagonalStep() const;
		bool CeilingIsDiagonalStep() const;
		bool CeilingIsSplit() const;
		bool FloorHasSplitPortal() const;
		bool CeilingHasSplitPortal() const;
		std::optional<int> RoomBelow(int plane) const;
		std::optional<int> RoomBelow(int x, int z) const;
		std::optional<int> RoomBelow(int x, int y, int z) const;
		std::optional<int> RoomAbove(int plane) const;
		std::optional<int> RoomAbove(int x, int z) const;
		std::optional<int> RoomAbove(int x, int y, int z) const;
		std::optional<int> RoomSide() const;
		int FloorHeight(int x, int z) const;
		int FloorHeight(int x, int y, int z) const;
		int BridgeFloorHeight(int x, int y, int z) const;
		int CeilingHeight(int x, int z) const;
		int CeilingHeight(int x, int y, int z) const;
		int BridgeCeilingHeight(int x, int y, int z) const;
		Vector2 FloorSlope(int plane) const;
		Vector2 FloorSlope(int x, int z) const;
		Vector2 CeilingSlope(int plane) const;
		Vector2 CeilingSlope(int x, int z) const;
		bool IsWall(int plane) const;
		bool IsWall(int x, int z) const;
		short InsideBridge(int x, int y, int z, bool floorBorder, bool ceilingBorder) const;
		void AddItem(short itemNumber);
		void RemoveItem(short itemNumber);
};

namespace TEN::Floordata
{
	VectorInt2 GetSectorPoint(int x, int z);
	VectorInt2 GetRoomPosition(int roomNumber, int x, int z);
	FLOOR_INFO& GetFloor(int roomNumber, const VectorInt2& pos);
	FLOOR_INFO& GetFloor(int roomNumber, int x, int z);
	FLOOR_INFO& GetFloorSide(int roomNumber, int x, int z, int* sideRoomNumber = nullptr);
	FLOOR_INFO& GetBottomFloor(int roomNumber, int x, int z, int* bottomRoomNumber = nullptr);
	FLOOR_INFO& GetTopFloor(int roomNumber, int x, int z, int* topRoomNumber = nullptr);
	std::optional<int> GetTopHeight(FLOOR_INFO& startFloor, int x, int y, int z, int* topRoomNumber = nullptr, FLOOR_INFO** topFloor = nullptr);
	std::optional<int> GetBottomHeight(FLOOR_INFO& startFloor, int x, int y, int z, int* bottomRoomNumber = nullptr, FLOOR_INFO** bottomFloor = nullptr);
	std::optional<int> GetFloorHeight(const ROOM_VECTOR& location, int x, int z);
	std::optional<int> GetCeilingHeight(const ROOM_VECTOR& location, int x, int z);
	std::optional<ROOM_VECTOR> GetBottomRoom(ROOM_VECTOR location, int x, int y, int z);
	std::optional<ROOM_VECTOR> GetTopRoom(ROOM_VECTOR location, int x, int y, int z);
	ROOM_VECTOR GetRoom(ROOM_VECTOR location, int x, int y, int z);

	void AddBridge(short itemNumber, int x = 0, int z = 0);
	void RemoveBridge(short itemNumber, int x = 0, int z = 0);

	std::optional<int> GetBridgeItemIntersect(int itemNumber, int x, int y, int z, bool bottom);
	int GetBridgeBorder(int itemNumber, bool bottom);
	void UpdateBridgeItem(int itemNumber, bool forceRemoval = false);
}