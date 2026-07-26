
#include "Precomp.h"
#include "LightSystem.h"
#include "UObject/UActor.h"
#include "UObject/ULevel.h"
#include "Math/floating.h"
#include "Math/coords.h"
#include "Engine.h"

void LightSystem::UpdateLightList(UActor* actor)
{
	vec3 location = actor->BspInfo.BoundingBox.center();
	float time = actor->Level() ? actor->Level()->TimeSeconds() : 0.0f;

	// Rescan periodically even for a stationary actor, since a light that is
	// itself moving or blinking can enter/leave range without this actor moving.
	constexpr float RescanInterval = 0.25f;
	if (!actor->LightInfo.NeedsUpdate && actor->LightInfo.Location == location && time < actor->LightInfo.NextRescanTime)
		return;

	actor->LightInfo.NeedsUpdate = false;
	actor->LightInfo.Location = location;
	actor->LightInfo.NextRescanTime = time + RescanInterval;
	actor->LightInfo.LightList.clear();

	if (actor->bUnlit())
		return;

	vec3 extents = actor->BspInfo.BoundingBox.extents();

	int checkCounter = NextCheckCounter();
	ivec3 start = GetStartExtents(location, extents);
	ivec3 end = GetEndExtents(location, extents);
	if (end.x - start.x < 100 && end.y - start.y < 100 && end.z - start.z < 100)
	{
		for (int z = start.z; z < end.z; z++)
		{
			for (int y = start.y; y < end.y; y++)
			{
				for (int x = start.x; x < end.x; x++)
				{
					for (UActor* light : GetActors(x, y, z))
					{
						if (light->Light.CheckCounter != checkCounter)
						{
							light->Light.CheckCounter = checkCounter;
							// bCorona only controls drawing a lens-flare sprite for the light actor
							// itself (see VisibleCorona.cpp) - it doesn't mean the actor isn't a real
							// light source, so it must not be excluded here (this is why Flares never
							// illuminated their surroundings). bSpecialLit is a genuine lighting-channel
							// flag and stays excluded.
							if (!light->bSpecialLit())
							{
								float radius = light->WorldLightRadius();
								vec3 L = light->Location() - location;
								if (light->LightEffect() == LE_Cylinder) // Cylinder lights have infinite Z axis range
								{
									L.z = 0.0f;
								}
								if (dot(L, L) < radius * radius && !engine->Level->Collision.TraceAnyHit(light->Location(), location, actor, false, true, true))
								{
									actor->LightInfo.LightList.push_back(light);
								}
							}
						}
					}
				}
			}
		}
	}
}

void LightSystem::SetLevel(ULevel* level)
{
	Level = level;
}

void LightSystem::AddLight(UActor* light)
{
	if (light->LightType() != LT_None && light->LightBrightness() > 0)
	{
		vec3 location = light->Location();
		float radius = light->WorldLightRadius();

		light->Light.Inserted = true;
		light->Light.Location = location;
		light->Light.Radius = radius;

		ivec3 start = GetStartExtents(location, radius);
		ivec3 end = GetEndExtents(location, radius);
		for (int z = start.z; z < end.z; z++)
		{
			for (int y = start.y; y < end.y; y++)
			{
				for (int x = start.x; x < end.x; x++)
				{
					LightActors[GetBucketId(x, y, z)].push_back(light);
				}
			}
		}

		if (light->Light.SpawnedAtRuntime || light->HasAnimatedLightBrightness())
		{
			light->Light.IsDynamicList = true;
			DynamicLights.push_back(light);
		}
	}
}

void LightSystem::RemoveLight(UActor* light)
{
	if (light->Light.Inserted)
	{
		vec3 location = light->Light.Location;
		float radius = light->Light.Radius;

		ivec3 start = GetStartExtents(location, radius);
		ivec3 end = GetEndExtents(location, radius);
		for (int z = start.z; z < end.z; z++)
		{
			for (int y = start.y; y < end.y; y++)
			{
				for (int x = start.x; x < end.x; x++)
				{
					auto it = LightActors.find(GetBucketId(x, y, z));
					if (it != LightActors.end())
					{
						it->second.remove(light);
						if (it->second.empty())
							LightActors.erase(it);
					}
				}
			}
		}

		if (light->Light.IsDynamicList)
		{
			DynamicLights.remove(light);
			light->Light.IsDynamicList = false;
		}

		light->Light.Inserted = false;
	}
}

void LightSystem::CollectNearbyDynamicLights(const vec3& center, float radius, std::vector<UActor*>& result) const
{
	for (UActor* light : DynamicLights)
	{
		float range = radius + light->WorldLightRadius();
		vec3 L = light->Location() - center;
		if (dot(L, L) < range * range)
			result.push_back(light);
	}
}
