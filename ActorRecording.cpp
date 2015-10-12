#include <vector>
#include <algorithm>
#include "ActorRecording.h"
#include "script.h"
#include "Actor.h"

ActorRecordingItem::ActorRecordingItem(DWORD ticksStart, Ped actor, Vector3 location)
{
	m_ticksAfterRecordStart = ticksStart;
	m_actor = actor;
	m_location = location;
	m_ticksDeltaCheckCompletion = 300;
}

DWORD ActorRecordingItem::getTicksAfterRecordStart()
{
	return m_ticksAfterRecordStart;
}


Vector3 ActorRecordingItem::getLocation()
{
	return m_location;
}

void ActorRecordingItem::executeNativesForRecording(Actor actor)
{
	playback_recording_to_waypoint(actor.getActorPed(), m_location);
}

bool ActorRecordingItem::isRecordingItemCompleted(Actor actor, Vector3 location)
{
	Ped actorPed = actor.getActorPed();
	bool isInVehicle = PED::IS_PED_IN_ANY_VEHICLE(actorPed, 0);
	bool isPedInHeli = PED::IS_PED_IN_ANY_HELI(actorPed);
	bool isPedInPlane = PED::IS_PED_IN_ANY_PLANE(actorPed);
	bool isPedInBoat = PED::IS_PED_IN_ANY_BOAT(actorPed);

	float minDistance = 4.0;

	if (isPedInHeli) {
		minDistance = 50.0;
	}
	else if (isPedInPlane) {
		minDistance = 100.0;
	}
	else if (isPedInBoat) {
		minDistance = 60.0;
	}
	else if (isInVehicle) {
		minDistance = 15.0;
	}

	float distanceToTarget = SYSTEM::VDIST(m_location.x, m_location.y, m_location.z, location.x, location.y, location.z);

	log_to_file("Distance to target: " + std::to_string(distanceToTarget));

	if (distanceToTarget < minDistance) {
		log_to_file("Close enough to target for ActorRecordingItem");
		return true;
	}
	else {
		return false;
	}
}

DWORD ActorRecordingItem::getTicksDeltaCheckCompletion()
{
	return m_ticksDeltaCheckCompletion;
}

std::string ActorRecordingItem::toString()
{
	return "Recording start for actor " + std::to_string(m_actor) + " at " + std::to_string(m_ticksAfterRecordStart);
}

ActorMovementRecordingItem::ActorMovementRecordingItem(DWORD ticksStart, Ped actor, Vector3 location, float heading):ActorRecordingItem(ticksStart, actor, location)
{
	m_heading = heading;
}

float ActorMovementRecordingItem::getHeading()
{
	return m_heading;
}

std::string ActorMovementRecordingItem::toString()
{
	return ActorRecordingItem::toString() + " ActorMovementRecordingItem Location (" + std::to_string(m_location.x) + "," + std::to_string(m_location.y) + "," + std::to_string(m_location.z) + ") Heading " + std::to_string(m_heading);
}


ActorVehicleRecordingItem::ActorVehicleRecordingItem(DWORD ticksStart, Ped actor, Vector3 location, Vehicle veh) :ActorRecordingItem(ticksStart, actor, location)
{
	m_vehicle = veh;
	m_vehicleType = _getVehicleTypeFromNatives();
}


Vehicle ActorVehicleRecordingItem::getVehicle()
{
	return m_vehicle;
}

VEHICLE_TYPE ActorVehicleRecordingItem::getVehicleType()
{
	return m_vehicleType;
}

std::string ActorVehicleRecordingItem::toString()
{
	return ActorRecordingItem::toString() + " ActorVehicleRecordingItem Vehicle " + std::to_string(m_vehicle) + " Vehicle type " + std::to_string(m_vehicleType) + " Location (" + std::to_string(m_location.x) + "," + std::to_string(m_location.y) + "," + std::to_string(m_location.z) + ")";
}

VEHICLE_TYPE ActorVehicleRecordingItem::_getVehicleTypeFromNatives()
{
	//natives for VEHICLE all require the Hash of the model, so checking actor instead
	if (PED::IS_PED_IN_ANY_HELI(m_actor)) {
		return VEHICLE_TYPE_HELI;
	}
	else if (PED::IS_PED_IN_ANY_PLANE(m_actor)) {
		return VEHICLE_TYPE_PLANE;
	}
	else if (PED::IS_PED_IN_ANY_BOAT(m_actor)) {
		return VEHICLE_TYPE_BOAT;
	}
	else if (PED::IS_PED_IN_ANY_SUB(m_actor)) {
		return VEHICLE_TYPE_SUB;
	}
	else if (PED::IS_PED_ON_ANY_BIKE(m_actor)){
		return VEHICLE_TYPE_BIKE;
	}
	else {
		return VEHICLE_TYPE_CAR;
	}
}

ActorRecordingPlayback::ActorRecordingPlayback()
{
	m_recordingItemIndex = 0;
	m_ticksStartCurrentItem = 0;
}

ActorRecordingPlayback::ActorRecordingPlayback(DWORD tickStart, int maxRecordingItemIndex)
{
	m_recordingItemIndex = 0;
	m_ticksStartCurrentItem = tickStart;
	m_ticksPlaybackStarted = tickStart;
	m_playbackCompleted = false;
	m_maxRecordingItemIndex = maxRecordingItemIndex;
}

DWORD ActorRecordingPlayback::getTicksAfterCurrentItemStarted()
{
	return m_ticksStartCurrentItem;
}

DWORD ActorRecordingPlayback::getTicksPlaybackStarted()
{
	return m_ticksPlaybackStarted;
}

void ActorRecordingPlayback::setRecordingItemIndex(int index)
{
	m_recordingItemIndex = index;
}

void ActorRecordingPlayback::nextRecordingItemIndex(DWORD ticksNow)
{
	m_recordingItemIndex++;
	m_ticksLastCheckOfCurrentItem = ticksNow;
}

bool ActorRecordingPlayback::isCurrentRecordingItemLast()
{
	if (m_recordingItemIndex + 1 >= m_maxRecordingItemIndex) {
		return true;
	}
	else {
		return false;
	}

}

int ActorRecordingPlayback::getRecordingItemIndex()
{
	return m_recordingItemIndex;
}

void ActorRecordingPlayback::setPlaybackCompleted()
{
	m_playbackCompleted = true;
}

bool ActorRecordingPlayback::hasPlaybackCompleted()
{
	return m_playbackCompleted;
}

void ActorRecordingPlayback::setTickLastCheckOfCurrentItem(DWORD ticks)
{
	m_ticksLastCheckOfCurrentItem = ticks;
}

DWORD ActorRecordingPlayback::getTickLastCheckOfCurrentItem()
{
	return m_ticksLastCheckOfCurrentItem;
}

void ActorRecordingPlayback::setHasFirstItemPlayback(bool hasPlaybacked)
{
	m_hasFirstItemPlayback = hasPlaybacked;
}

bool ActorRecordingPlayback::getHasFirstItemPlayback()
{
	return m_hasFirstItemPlayback;
}
