#pragma once

#include "Math/vec.h"
#include "LightEffect.h"
#include "Shadowmap.h"

class BspSurface;
class LightMapIndex;
class UModel;
class UZoneInfo;
class UActor;
class Coords;
struct Poly;

class LightmapBuilder
{
public:
	void Setup(UModel* model, const Coords& mapCoords, int lightMap, UZoneInfo* zoneActor);
	void AddStaticLights(UModel* model, int lightMap);

	// Just the per-texel world positions/normal, without resetting the lit colors to
	// ambient. Used to relight an already-cached lightmap with a dynamic light overlay.
	void CalcGeometry(UModel* model, const Coords& mapCoords, int lightMap);
	void GetWorldBounds(vec3& outCenter, float& outRadius) const;
	void AddDynamicLight(UActor* light, vec3* colors);

	int Width() const { return width; }
	int Height() const { return height; }
	const vec3* Pixels() const { return lightcolors.data(); }

private:
	const vec3* WorldLocations() const { return points.data(); }
	const vec3& WorldNormal() const { return normal; }

	void CalcWorldLocations(Coords MapCoords, const LightMapIndex& lmindex);

	int width = 0;
	int height = 0;
	Array<vec3> lightcolors;

	Array<vec3> points;
	vec3 normal;
	vec3 base;

	Shadowmap Shadow;
	LightEffect Effect;
	Array<float> illuminationmap;
	Array<float> unshadowedmap;
};
