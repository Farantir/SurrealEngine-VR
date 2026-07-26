
#include "Precomp.h"
#include "RenderSubsystem.h"
#include "RenderDevice/RenderDevice.h"
#include "Engine.h"
#include "Math/hsb.h"
#include <vector>

FTextureInfo RenderSubsystem::GetBrushLightmap(UMover* mover, const Poly& poly, UZoneInfo* zoneActor, UModel* model)
{
	// Movers go through the same GetLightmap() as regular BSP surfaces, which already
	// relights against any nearby dynamic light every frame - bDynamicLightMover() needs
	// no separate handling.

	Coords localCoords;
	localCoords.Origin = -poly.Base;
	localCoords.XAxis = poly.TextureU;
	localCoords.YAxis = poly.TextureV;
	localCoords.ZAxis = poly.Normal;

	vec3 moverLocation = mover->BasePos() + mover->KeyPos()[mover->BrushRaytraceKey()];
	Rotator moverRotation = mover->BaseRot() + mover->KeyRot()[mover->BrushRaytraceKey()];
	mat4 objectToWorld = mat4::translate(moverLocation) * Coords::Rotation(moverRotation).ToMatrix() * mat4::scale(mover->MainScale().Scale) * mat4::translate(-mover->PrePivot()) * localCoords.ToMatrix();
	Coords worldCoords = Coords::FromMatrix(objectToWorld);

	return GetLightmap(model, poly.BrushPolyIndex, worldCoords, zoneActor);
}

FTextureInfo RenderSubsystem::GetSurfaceLightmap(BspSurface& surface, UZoneInfo* zoneActor, UModel* model)
{
	Coords mapCoords;
	mapCoords.Origin = model->Points[surface.pBase];
	mapCoords.XAxis = model->Vectors[surface.vTextureU];
	mapCoords.YAxis = model->Vectors[surface.vTextureV];
	mapCoords.ZAxis = model->Vectors[surface.vNormal];
	return GetLightmap(model, surface.LightMap, mapCoords, zoneActor);
}

FTextureInfo RenderSubsystem::GetLightmap(UModel* model, int lightmapIndex, const Coords& coords, UZoneInfo* zoneActor)
{
	if (lightmapIndex < 0)
		return {};

	uint32_t ambientID = (((uint32_t)zoneActor->AmbientHue()) << 16) | (((uint32_t)zoneActor->AmbientSaturation()) << 8) | (uint32_t)zoneActor->AmbientBrightness();
	uint64_t cacheID = (((uint64_t)model->LightMap[lightmapIndex].LMCacheID) << 32) | (((uint64_t)ambientID) << 8) | 1;

	auto& lmtexture = Light.lmtextures[cacheID];
	auto& lmbasecolors = Light.lmbasecolors[cacheID];
	auto& lmbounds = Light.lmbounds[cacheID];
	if (!lmtexture)
	{
		Light.Builder.Setup(model, coords, lightmapIndex, zoneActor);
		Light.Builder.AddStaticLights(model, lightmapIndex);

		size_t count = (size_t)Light.Builder.Width() * Light.Builder.Height();
		lmbasecolors.assign(Light.Builder.Pixels(), Light.Builder.Pixels() + count);
		Light.Builder.GetWorldBounds(lmbounds.Center, lmbounds.Radius);

		lmtexture = CreateLightmapTexture();
	}

	// Cheap broad-phase test: only pay for a per-texel relight of this lightmap when a
	// light that isn't part of its static bake is actually within reach.
	std::vector<UActor*> dynamicLights;
	engine->Level->Light.CollectNearbyDynamicLights(lmbounds.Center, lmbounds.Radius, dynamicLights);
	bool textureChanged = false;
	if (!dynamicLights.empty())
	{
		Light.Builder.CalcGeometry(model, coords, lightmapIndex);

		Array<vec3> colors(lmbasecolors.begin(), lmbasecolors.end());
		for (UActor* light : dynamicLights)
			Light.Builder.AddDynamicLight(light, colors.data());

		WriteLightmapPixels(lmtexture.get(), colors.data());
		lmbounds.Overlayed = true;
		textureChanged = true;
	}
	else if (lmbounds.Overlayed)
	{
		// The last dynamic light influencing this surface has moved on - restore the
		// cached texture back to its static-only colors instead of leaving it stuck lit.
		WriteLightmapPixels(lmtexture.get(), lmbasecolors.data());
		lmbounds.Overlayed = false;
		textureChanged = true;
	}

	const LightMapIndex& lmindex = model->LightMap[lightmapIndex];

	FTextureInfo texinfo;
	texinfo.CacheID = cacheID;
	texinfo.Format = lmtexture->Format;
	texinfo.Mips = &lmtexture->Mip;
	texinfo.NumMips = 1;
	texinfo.USize = texinfo.Mips[0].Width;
	texinfo.VSize = texinfo.Mips[0].Height;
	texinfo.Pan = { lmindex.PanX, lmindex.PanY };
	texinfo.UScale = lmindex.UScale;
	texinfo.VScale = lmindex.VScale;
	texinfo.bRealtimeChanged = textureChanged;
	return texinfo;
}

std::unique_ptr<LightmapTexture> RenderSubsystem::CreateLightmapTexture()
{
	auto lmtexture = std::make_unique<LightmapTexture>();
#if 1 // Float high quality lightmaps
	lmtexture->Format = TextureFormat::RGBA32_F;
	lmtexture->Mip.Data.resize((size_t)Light.Builder.Width() * Light.Builder.Height() * sizeof(vec4));
#else // Low quality lightmaps like UE1 got them
	lmtexture->Format = TextureFormat::BGRA8_LM;
	lmtexture->Mip.Data.resize((size_t)Light.Builder.Width() * Light.Builder.Height() * 4);
#endif
	lmtexture->Mip.Width = Light.Builder.Width();
	lmtexture->Mip.Height = Light.Builder.Height();
	WriteLightmapPixels(lmtexture.get(), Light.Builder.Pixels());
	return lmtexture;
}

void RenderSubsystem::WriteLightmapPixels(LightmapTexture* texture, const vec3* colors)
{
	int count = texture->Mip.Width * texture->Mip.Height;
	if (texture->Format == TextureFormat::RGBA32_F)
	{
		vec4* dest = (vec4*)texture->Mip.Data.data();
		for (int i = 0; i < count; i++)
			dest[i] = vec4(colors[i], 1.0f);
	}
	else // BGRA8_LM
	{
		uint32_t* dest = (uint32_t*)texture->Mip.Data.data();
		for (int i = 0; i < count; i++)
		{
			uint32_t red = (uint32_t)clamp(colors[i].r * 127.0f + 0.5f, 0.0f, 127.0f);
			uint32_t green = (uint32_t)clamp(colors[i].g * 127.0f + 0.5f, 0.0f, 127.0f);
			uint32_t blue = (uint32_t)clamp(colors[i].b * 127.0f + 0.5f, 0.0f, 127.0f);
			uint32_t alpha = 127;
			dest[i] = (alpha << 24) | (red << 16) | (green << 8) | blue;
		}
	}
}

void RenderSubsystem::UpdateActorLightList(UActor* actor)
{
	engine->Level->Light.UpdateLightList(actor);
}

vec3 RenderSubsystem::GetVertexLight(UActor* actor, const vec3& location, const vec3& normal, bool unlit, UZoneInfo* zoneActor)
{
	// AmbientGlow value 255 is a special pulsating effect used for powerups
	float ambientGlow = actor->AmbientGlow() == 255 ? AmbientGlowAmount : actor->AmbientGlow() * (1.0f / 255.0f);
	vec3 ambientColor = ambientGlow + hsbtorgb(zoneActor->AmbientHue(), zoneActor->AmbientSaturation(), zoneActor->AmbientBrightness());

	if (unlit)
	{
		return (ambientColor + actor->ScaleGlow() * 0.5f) * 2.0f;
	}
	else
	{
		vec3 color(0.0f);

		for (UActor* light : actor->LightInfo.LightList)
		{
			vec3 L = light->Location() - location;
			float attenuation = std::max(1.0f - length(L) / light->WorldLightRadius(), 0.0f);
			if (attenuation > 0.0f)
			{
				float angleAttenuation = std::abs(dot(normalize(L), normal));
				vec3 lightcolor = hsbtorgb(light->LightHue(), light->LightSaturation(), light->GetEffectiveLightBrightness());
				color += lightcolor * (attenuation * angleAttenuation);
			}
		}

		return (ambientColor + color * (actor->ScaleGlow() * 1.5f)) * 2.0f;
	}
}
