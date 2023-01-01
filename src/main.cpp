// Eternal Horror
// Copyright(C) 2020 John D. Corrado
// Planes based on Doom visplane code
// Copyright(C) 1993-1996 Id Software, Inc.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

#include <SDL.h>
#include <SDL_mixer.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "fixed.h"

typedef struct
{
	int32_t mapIndex;
	uint32_t state;
	fixed_t offset;
	uint32_t tics;
} door_t;

typedef struct
{
	int32_t mapIndex;
	uint32_t type;
	uint32_t state;
	int32_t health;
	int32_t gridX;
	int32_t gridY;
	uint32_t damageTics;
	uint32_t attackTics;
	uint32_t render;
	uint32_t damage;
} enemy_t;

typedef struct
{
	uint32_t type;
	int32_t gridX;
	int32_t gridY;
	uint32_t render;
} health_t;

typedef struct
{
	int32_t minX;
	int32_t maxX;
	uint32_t pad1;
	uint32_t top[64];
	uint32_t pad2;
} plane_t;

int32_t mapWidth;
int32_t mapHeight;

uint32_t *mapData;

uint32_t level = 1;
const uint32_t numLevels = 4;
const char *levels[] =
{
	"level1.map",
	"level2.map",
	"level3.map",
	"level4.map"
};

fixed_t cameraX;
fixed_t cameraY;
angle_t cameraAngle;
fixed_t oldCameraX;
fixed_t oldCameraY;
uint32_t state = 2;
int32_t health;
uint32_t attackTics = 0;

bool moveForwardPressed = false;
bool moveBackwardPressed = false;
bool strafeLeftPressed = false;
bool strafeRightPressed = false;
bool turnLeftPressed = false;
bool turnRightPressed = false;
bool fireWeaponPressed = false;
bool restartLevelPressed = false;

bool running = true;

door_t doors[64] =
{
	{ -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 },
	{ -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 },
	{ -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 },
	{ -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 },
	{ -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 },
	{ -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 },
	{ -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 },
	{ -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }
};

enemy_t enemies[64] =
{
	{ -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

health_t healths[64] =
{
	{ 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 },
	{ 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 },
	{ 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 },
	{ 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 },
	{ 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 },
	{ 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 },
	{ 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 },
	{ 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }
};

uint32_t *yTable[64];
uint32_t xTable[64];

plane_t plane;

uint32_t start[32];
uint32_t stop[32];
fixed_t currentX[32];
fixed_t currentY[32];
fixed_t stepX[32];
fixed_t stepY[32];

fixed_t zBuffer[64];

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Surface *surface;
SDL_Texture *texture;
SDL_Surface *textures;
SDL_Surface *sprites;
SDL_Surface *hand;
SDL_Surface *title;
SDL_Surface *dead;
SDL_Surface *end;
SDL_Surface *credits;
Mix_Music *music[2];

Uint32 lastTime;

uint32_t frames[6] = { 0, 4096, 8192, 12288, 16384, 20480 };
uint32_t frame = 0;
uint32_t frameTics = 0;

uint32_t bloodHeight[64];
uint32_t bloodSpeed[64];
uint32_t bloodTics = 0;

void DrawWallSlice(SDL_Surface *surface, const uint32_t *texture, uint32_t textureOffsetX, int32_t wallX, int32_t wallY, uint32_t wallHeight)
{
	uint32_t count;
	fixed_t textureOffsetY;
	fixed_t scalar = fixedDiv(64 << FRACBITS, wallHeight << FRACBITS);
	texture = &texture[textureOffsetX * 64];

	if (wallY < 0)
	{
		count = 63;
		textureOffsetY = -wallY * scalar;
		wallY = 0;
	}
	else
	{
		count = wallHeight - 1;
		textureOffsetY = 0;
	}

	uint32_t *p = yTable[wallY] + xTable[wallX];

	do
	{
		*p = texture[textureOffsetY >> FRACBITS];
		p += surface->w;
		textureOffsetY += scalar;
	} while (count--);
}

#define MIX_COLORS(src, dst, mask, shift) (((((src) & (mask)) + ((dst) & (mask))) >> 1) & (mask))
#define MIX_COLORS_R(src, dst) MIX_COLORS(src, dst, surface->format->Rmask, surface->format->Rshift)
#define MIX_COLORS_G(src, dst) MIX_COLORS(src, dst, surface->format->Gmask, surface->format->Gshift)
#define MIX_COLORS_B(src, dst) MIX_COLORS(src, dst, surface->format->Bmask, surface->format->Bshift)

void DrawSprite(SDL_Surface *surface, const uint32_t *sprite, int32_t spriteX, int32_t spriteY, uint32_t spriteSize, fixed_t spriteDistance, uint32_t spriteDamage)
{
	if (spriteX + (int32_t)spriteSize <= 0 || spriteX > 63)
		return;

	uint32_t countX;
	uint32_t countY;
	uint32_t yCount;
	fixed_t spriteOffsetX;
	fixed_t spriteOffsetY;
	fixed_t ySpriteOffset;
	fixed_t scalar = fixedDiv(64 << FRACBITS, spriteSize << FRACBITS);
	Uint32 colorKey = SDL_MapRGB(surface->format, 152, 0, 136);
	Uint32 damageColor = SDL_MapRGB(surface->format, 255, 0, 0);

	if (spriteX < 0)
	{
		countX = spriteSize + spriteX - 1;
		spriteOffsetX = -spriteX * scalar;
		spriteX = 0;
	}
	else
	{
		countX = (spriteX + spriteSize > 63 ? 64 - spriteX : spriteSize) - 1;
		spriteOffsetX = 0;
	}

	const uint32_t *spriteColumn = &sprite[(spriteOffsetX >> FRACBITS) * 64];

	if (spriteY < 0)
	{
		countY = yCount = 63;
		spriteOffsetY = ySpriteOffset = -spriteY * scalar;
		spriteY = 0;
	}
	else
	{
		countY = yCount = spriteSize - 1;
		spriteOffsetY = ySpriteOffset = 0;
	}

	uint32_t *p = yTable[spriteY] + xTable[spriteX];
	uint32_t *temp = p;

	do
	{
		if (spriteDistance < zBuffer[spriteX++])
		{
			do
			{
				Uint32 color = spriteColumn[spriteOffsetY >> FRACBITS];
				if (color != colorKey)
					*p = spriteDamage ? MIX_COLORS_R(damageColor, color) | MIX_COLORS_G(damageColor, color) | MIX_COLORS_B(damageColor, color) : color;
				p += surface->w;
				spriteOffsetY += scalar;
			} while (countY--);
		}

		spriteOffsetX += scalar;
		spriteColumn = &sprite[(spriteOffsetX >> FRACBITS) * 64];
		countY = yCount;
		spriteOffsetY = ySpriteOffset;
		p = ++temp;
	} while (countX--);
}

void PollInput()
{
	SDL_Event event;

	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_QUIT:
			running = false;
			break;
		case SDL_KEYDOWN:
			switch (event.key.keysym.sym)
			{
			case SDLK_ESCAPE:
				running = false;
				break;
			case SDLK_w:
				moveForwardPressed = true;
				break;
			case SDLK_s:
				moveBackwardPressed = true;
				break;
			case SDLK_a:
				strafeLeftPressed = true;
				break;
			case SDLK_d:
				strafeRightPressed = true;
				break;
			case SDLK_LEFT:
				turnLeftPressed = true;
				break;
			case SDLK_RIGHT:
				turnRightPressed = true;
				break;
			case SDLK_LCTRL:
				fireWeaponPressed = true;
				break;
			case SDLK_RETURN:
				restartLevelPressed = true;
				break;
			default:
				break;
			}
			break;
		case SDL_KEYUP:
			switch (event.key.keysym.sym)
			{
			case SDLK_w:
				moveForwardPressed = false;
				break;
			case SDLK_s:
				moveBackwardPressed = false;
				break;
			case SDLK_a:
				strafeLeftPressed = false;
				break;
			case SDLK_d:
				strafeRightPressed = false;
				break;
			case SDLK_LEFT:
				turnLeftPressed = false;
				break;
			case SDLK_RIGHT:
				turnRightPressed = false;
				break;
			case SDLK_LCTRL:
				fireWeaponPressed = false;
				break;
			default:
				break;
			}
			break;
		case SDL_MOUSEBUTTONDOWN:
			if (event.button.button == SDL_BUTTON_LEFT)
				fireWeaponPressed = true;
			break;
		case SDL_MOUSEBUTTONUP:
			if (event.button.button == SDL_BUTTON_LEFT)
				fireWeaponPressed = false;
			break;
		default:
			break;
		}
	}
}

void Update()
{
	Uint32 currentTime = SDL_GetTicks();
	Uint32 elapsedTime = currentTime - lastTime;

	while (elapsedTime < 16)
	{
		currentTime = SDL_GetTicks();
		elapsedTime = currentTime - lastTime;
	}

	lastTime = currentTime;

	if (state == 1)
	{
		oldCameraX = cameraX;
		oldCameraY = cameraY;

		if (moveForwardPressed)
		{
			cameraX += fixedMul(279620, fixedCos(cameraAngle));
			cameraY -= fixedMul(279620, fixedSin(cameraAngle));
		}

		if (moveBackwardPressed)
		{
			cameraX -= fixedMul(279620, fixedCos(cameraAngle));
			cameraY += fixedMul(279620, fixedSin(cameraAngle));
		}

		if (strafeLeftPressed)
		{
			cameraX += fixedMul(279620, fixedCos(cameraAngle + 64));
			cameraY -= fixedMul(279620, fixedSin(cameraAngle + 64));
		}

		if (strafeRightPressed)
		{
			cameraX -= fixedMul(279620, fixedCos(cameraAngle + 64));
			cameraY += fixedMul(279620, fixedSin(cameraAngle + 64));
		}

		bool fireWeapon = false;

		if (fireWeaponPressed)
		{
			attackTics++;

			if (attackTics > 30)
			{
				fireWeapon = true;
				attackTics = 0;
			}
		}

		int x;

		SDL_GetRelativeMouseState(&x, NULL);

		cameraAngle = (cameraAngle - (x >> 1)) & ANGLESMASK;

		if (turnLeftPressed)
			cameraAngle = (cameraAngle + 2) & ANGLESMASK;

		if (turnRightPressed)
			cameraAngle = (cameraAngle - 2) & ANGLESMASK;

		uint32_t tx = cameraX >> 22;
		uint32_t txm = (cameraX - (5 << FRACBITS)) >> 22;
		uint32_t txp = (cameraX + (5 << FRACBITS)) >> 22;
		uint32_t ty = cameraY >> 22;
		uint32_t tym = (cameraY - (5 << FRACBITS)) >> 22;
		uint32_t typ = (cameraY + (5 << FRACBITS)) >> 22;

		if (cameraX - (5 << FRACBITS) < 0 || (cameraX + (5 << FRACBITS)) >> 22 >= mapWidth)
			cameraX = oldCameraX;
		else if (mapData[ty * mapWidth + txp] == 1 || mapData[ty * mapWidth + txm] == 1 || mapData[ty * mapWidth + txp] == 3 || mapData[ty * mapWidth + txm] == 3 || mapData[ty * mapWidth + txp] == 4 || mapData[ty * mapWidth + txm] == 4)
			cameraX = oldCameraX;
		else
		{
			if (mapData[typ * mapWidth + tx] == 1 || mapData[typ * mapWidth + tx] == 3 || mapData[typ * mapWidth + tx] == 4)
				cameraY = (typ << 22) - (5 << FRACBITS);

			if (mapData[tym * mapWidth + tx] == 1 || mapData[tym * mapWidth + tx] == 3 || mapData[tym * mapWidth + tx] == 4)
				cameraY = (tym << 22) + (69 << FRACBITS);
		}

		if (cameraY - (5 << FRACBITS) < 0 || (cameraY + (5 << FRACBITS)) >> 22 >= mapHeight)
			cameraY = oldCameraY;
		else if (mapData[typ * mapWidth + tx] == 1 || mapData[tym * mapWidth + tx] == 1 || mapData[typ * mapWidth + tx] == 3 || mapData[tym * mapWidth + tx] == 3 || mapData[typ * mapWidth + tx] == 4 || mapData[tym * mapWidth + tx] == 4)
			cameraY = oldCameraY;
		else
		{
			if (mapData[ty * mapWidth + txp] == 1 || mapData[ty * mapWidth + txp] == 3 || mapData[ty * mapWidth + txp] == 4)
				cameraX = (txp << 22) - (5 << FRACBITS);

			if (mapData[ty * mapWidth + txm] == 1 || mapData[ty * mapWidth + txm] == 3 || mapData[ty * mapWidth + txm] == 4)
				cameraX = (txm << 22) + (69 << FRACBITS);
		}

		int32_t mapIndex = ty * mapWidth + tx;

		if (mapData[mapIndex] == 5)
		{
			if (health < 100)
			{
				mapData[mapIndex] = 0;

				health += 10;

				if (health > 100)
					health = 100;
			}
		}
		else if (mapData[mapIndex] == 6)
		{
			if (health < 100)
			{
				mapData[mapIndex] = 0;

				health += 25;

				if (health > 100)
					health = 100;
			}
		}
		else if (mapData[mapIndex] == 8)
		{
			level++;

			if (level <= numLevels)
			{
				FILE *fp = fopen(levels[level - 1], "rb");
				int32_t cameraGridX;
				int32_t cameraGridY;
				int32_t exitGridX;
				int32_t exitGridY;
				fread(&cameraGridX, 1, sizeof(int32_t), fp);
				fread(&cameraGridY, 1, sizeof(int32_t), fp);
				cameraX = (cameraGridX * 64 + 32) << FRACBITS;
				cameraY = (cameraGridY * 64 + 32) << FRACBITS;
				fread(&cameraAngle, 1, sizeof(angle_t), fp);
				fread(&exitGridX, 1, sizeof(int32_t), fp);
				fread(&exitGridY, 1, sizeof(int32_t), fp);
				fread(&mapWidth, 1, sizeof(int32_t), fp);
				fread(&mapHeight, 1, sizeof(int32_t), fp);
				free(mapData);
				mapData = (uint32_t *)malloc(mapWidth * mapHeight * sizeof(uint32_t));
				fread(mapData, mapWidth * mapHeight, sizeof(uint32_t), fp);
				fclose(fp);

				Mix_PlayMusic(music[(level - 1) % 2], -1);
			}
			else
			{
				level = 1;
				state = 4;
			}
		}

		mapIndex = (ty - 1) * mapWidth + tx;

		if (mapData[mapIndex] == 2)
		{
			door_t *door = &doors[(((ty - 1) & 7) << 3) + (tx & 7)];

			if (door->mapIndex != mapIndex)
			{
				door->mapIndex = mapIndex;
				door->state = 0;
				door->offset = 64 << FRACBITS;
				door->tics = 0;
			}

			if (door->state == 0 || door->state == 1)
			{
				if (door->mapIndex == (ty * mapWidth + txp) || door->mapIndex == (ty * mapWidth + txm))
					cameraX = oldCameraX;

				if (door->mapIndex == (typ * mapWidth + tx) || door->mapIndex == (tym * mapWidth + tx))
					cameraY = oldCameraY;
			}
		}
		else if (mapData[mapIndex] == 3 || mapData[mapIndex] == 4)
		{
			enemy_t *enemy1 = &enemies[(((ty - 1) & 7) << 3) + (tx & 7)];

			if (enemy1->mapIndex != mapIndex)
			{
				enemy1->mapIndex = mapIndex;
				enemy1->type = mapData[mapIndex];
				enemy1->state = 1;
				enemy1->health = 100;
				enemy1->damageTics = 0;
				enemy1->attackTics = 0;
				enemy1->damage = 0;
			}

			if (mapData[(ty + 1) * mapWidth + tx] == 0)
			{
				mapData[(ty + 1) * mapWidth + tx] = 4;

				enemy_t *enemy2 = &enemies[(((ty + 1) & 7) << 3) + (tx & 7)];
				enemy2->mapIndex = (ty + 1) * mapWidth + tx;
				enemy2->type = 1;
				enemy2->state = 1;
				enemy2->health = 100;
				enemy2->damageTics = 0;
				enemy2->attackTics = 0;
				enemy2->damage = 0;

				if (rand() % 2 == 0)
					enemy1->state = 2;
				else
					enemy2->state = 2;
			}

			if (fireWeapon && enemy1->state == 2 && cameraAngle >= 32 && cameraAngle < 96)
			{
				enemy1->health -= 25;
				enemy1->damage = 1;

				if (enemy1->health <= 0)
				{
					enemy1->state = 0;
					enemy1->health = 0;
				}
			}

			if (enemy1->state == 2)
			{
				enemy1->attackTics++;

				if (enemy1->attackTics > 60)
				{
					health -= 10;

					if (health <= 0)
					{
						state = 0;
						health = 0;
					}

					enemy1->attackTics = 0;
				}
			}

			if (enemy1->damage == 1)
			{
				enemy1->damageTics++;

				if (enemy1->damageTics > 15)
				{
					enemy1->damageTics = 0;
					enemy1->damage = 0;

					if (enemy1->state == 0)
					{
						mapData[mapIndex] = 7;
						enemy1->mapIndex = -1;
						enemy1->type = 0;
						enemy1->gridX = 0;
						enemy1->gridY = 0;
						enemy_t *enemy2 = &enemies[(((ty + 1) & 7) << 3) + (tx & 7)];
						enemy2->state = 2;
					}
				}
			}
		}

		mapIndex = (ty + 1) * mapWidth + tx;

		if (mapData[mapIndex] == 2)
		{
			door_t *door = &doors[(((ty + 1) & 7) << 3) + (tx & 7)];

			if (door->mapIndex != mapIndex)
			{
				door->mapIndex = mapIndex;
				door->state = 0;
				door->offset = 64 << FRACBITS;
				door->tics = 0;
			}

			if (door->state == 0 || door->state == 1)
			{
				if (door->mapIndex == (ty * mapWidth + txp) || door->mapIndex == (ty * mapWidth + txm))
					cameraX = oldCameraX;

				if (door->mapIndex == (typ * mapWidth + tx) || door->mapIndex == (tym * mapWidth + tx))
					cameraY = oldCameraY;
			}
		}
		else if (mapData[mapIndex] == 3 || mapData[mapIndex] == 4)
		{
			enemy_t *enemy1 = &enemies[(((ty + 1) & 7) << 3) + (tx & 7)];

			if (enemy1->mapIndex != mapIndex)
			{
				enemy1->mapIndex = mapIndex;
				enemy1->type = mapData[mapIndex];
				enemy1->state = 1;
				enemy1->health = 100;
				enemy1->damageTics = 0;
				enemy1->attackTics = 0;
				enemy1->damage = 0;
			}

			if (mapData[(ty - 1) * mapWidth + tx] == 0)
			{
				mapData[(ty - 1) * mapWidth + tx] = 4;

				enemy_t *enemy2 = &enemies[(((ty - 1) & 7) << 3) + (tx & 7)];
				enemy2->mapIndex = (ty - 1) * mapWidth + tx;
				enemy2->type = 1;
				enemy2->state = 1;
				enemy2->health = 100;
				enemy2->damageTics = 0;
				enemy2->attackTics = 0;
				enemy2->damage = 0;

				if (rand() % 2 == 0)
					enemy1->state = 2;
				else
					enemy2->state = 2;
			}

			if (fireWeapon && enemy1->state == 2 && cameraAngle >= 160 && cameraAngle < 224)
			{
				enemy1->health -= 25;
				enemy1->damage = 1;

				if (enemy1->health <= 0)
				{
					enemy1->state = 0;
					enemy1->health = 0;
				}
			}

			if (enemy1->state == 2)
			{
				enemy1->attackTics++;

				if (enemy1->attackTics > 60)
				{
					health -= 10;

					if (health <= 0)
					{
						state = 0;
						health = 0;
					}

					enemy1->attackTics = 0;
				}
			}

			if (enemy1->damage == 1)
			{
				enemy1->damageTics++;

				if (enemy1->damageTics > 15)
				{
					enemy1->damageTics = 0;
					enemy1->damage = 0;

					if (enemy1->state == 0)
					{
						mapData[mapIndex] = 7;
						enemy1->mapIndex = -1;
						enemy1->type = 0;
						enemy1->gridX = 0;
						enemy1->gridY = 0;
						enemy_t *enemy2 = &enemies[(((ty - 1) & 7) << 3) + (tx & 7)];
						enemy2->state = 2;
					}
				}
			}
		}

		mapIndex = ty * mapWidth + (tx - 1);

		if (mapData[mapIndex] == 2)
		{
			door_t *door = &doors[((ty & 7) << 3) + ((tx - 1) & 7)];

			if (door->mapIndex != mapIndex)
			{
				door->mapIndex = mapIndex;
				door->state = 0;
				door->offset = 64 << FRACBITS;
				door->tics = 0;
			}

			if (door->state == 0 || door->state == 1)
			{
				if (door->mapIndex == (ty * mapWidth + txp) || door->mapIndex == (ty * mapWidth + txm))
					cameraX = oldCameraX;

				if (door->mapIndex == (typ * mapWidth + tx) || door->mapIndex == (tym * mapWidth + tx))
					cameraY = oldCameraY;
			}
		}
		else if (mapData[mapIndex] == 3 || mapData[mapIndex] == 4)
		{
			enemy_t *enemy1 = &enemies[((ty & 7) << 3) + ((tx - 1) & 7)];

			if (enemy1->mapIndex != mapIndex)
			{
				enemy1->mapIndex = mapIndex;
				enemy1->type = mapData[mapIndex];
				enemy1->state = 1;
				enemy1->health = 100;
				enemy1->damageTics = 0;
				enemy1->attackTics = 0;
				enemy1->damage = 0;
			}

			if (mapData[ty * mapWidth + (tx + 1)] == 0)
			{
				mapData[ty * mapWidth + (tx + 1)] = 4;

				enemy_t *enemy2 = &enemies[((ty & 7) << 3) + ((tx + 1) & 7)];
				enemy2->mapIndex = ty * mapWidth + (tx + 1);
				enemy2->type = 1;
				enemy2->state = 1;
				enemy2->health = 100;
				enemy2->damageTics = 0;
				enemy2->attackTics = 0;
				enemy2->damage = 0;

				if (rand() % 2 == 0)
					enemy1->state = 2;
				else
					enemy2->state = 2;
			}

			if (fireWeapon && enemy1->state == 2 && cameraAngle >= 96 && cameraAngle < 160)
			{
				enemy1->health -= 25;
				enemy1->damage = 1;

				if (enemy1->health <= 0)
				{
					enemy1->state = 0;
					enemy1->health = 0;
				}
			}

			if (enemy1->state == 2)
			{
				enemy1->attackTics++;

				if (enemy1->attackTics > 60)
				{
					health -= 10;

					if (health <= 0)
					{
						state = 0;
						health = 0;
					}

					enemy1->attackTics = 0;
				}
			}

			if (enemy1->damage == 1)
			{
				enemy1->damageTics++;

				if (enemy1->damageTics > 15)
				{
					enemy1->damageTics = 0;
					enemy1->damage = 0;

					if (enemy1->state == 0)
					{
						mapData[mapIndex] = 7;
						enemy1->mapIndex = -1;
						enemy1->type = 0;
						enemy1->gridX = 0;
						enemy1->gridY = 0;
						enemy_t *enemy2 = &enemies[((ty & 7) << 3) + ((tx + 1) & 7)];
						enemy2->state = 2;
					}
				}
			}
		}

		mapIndex = ty * mapWidth + (tx + 1);

		if (mapData[mapIndex] == 2)
		{
			door_t *door = &doors[((ty & 7) << 3) + ((tx + 1) & 7)];

			if (door->mapIndex != mapIndex)
			{
				door->mapIndex = mapIndex;
				door->state = 0;
				door->offset = 64 << FRACBITS;
				door->tics = 0;
			}

			if (door->state == 0 || door->state == 1)
			{
				if (door->mapIndex == (ty * mapWidth + txp) || door->mapIndex == (ty * mapWidth + txm))
					cameraX = oldCameraX;

				if (door->mapIndex == (typ * mapWidth + tx) || door->mapIndex == (tym * mapWidth + tx))
					cameraY = oldCameraY;
			}
		}
		else if (mapData[mapIndex] == 3 || mapData[mapIndex] == 4)
		{
			enemy_t *enemy1 = &enemies[((ty & 7) << 3) + ((tx + 1) & 7)];

			if (enemy1->mapIndex != mapIndex)
			{
				enemy1->mapIndex = mapIndex;
				enemy1->type = mapData[mapIndex];
				enemy1->state = 1;
				enemy1->health = 100;
				enemy1->damageTics = 0;
				enemy1->attackTics = 0;
				enemy1->damage = 0;
			}

			if (mapData[ty * mapWidth + (tx - 1)] == 0)
			{
				mapData[ty * mapWidth + (tx - 1)] = 4;

				enemy_t *enemy2 = &enemies[((ty & 7) << 3) + ((tx - 1) & 7)];
				enemy2->mapIndex = ty * mapWidth + (tx - 1);
				enemy2->type = 1;
				enemy2->state = 1;
				enemy2->health = 100;
				enemy2->damageTics = 0;
				enemy2->attackTics = 0;
				enemy2->damage = 0;

				if (rand() % 2 == 0)
					enemy1->state = 2;
				else
					enemy2->state = 2;
			}

			if (fireWeapon && enemy1->state == 2 && (cameraAngle >= 224 || cameraAngle < 32))
			{
				enemy1->health -= 25;
				enemy1->damage = 1;

				if (enemy1->health <= 0)
				{
					enemy1->state = 0;
					enemy1->health = 0;
				}
			}

			if (enemy1->state == 2)
			{
				enemy1->attackTics++;

				if (enemy1->attackTics > 60)
				{
					health -= 10;

					if (health <= 0)
					{
						state = 0;
						health = 0;
					}

					enemy1->attackTics = 0;
				}
			}

			if (enemy1->damage == 1)
			{
				enemy1->damageTics++;

				if (enemy1->damageTics > 15)
				{
					enemy1->damageTics = 0;
					enemy1->damage = 0;

					if (enemy1->state == 0)
					{
						mapData[mapIndex] = 7;
						enemy1->mapIndex = -1;
						enemy1->type = 0;
						enemy1->gridX = 0;
						enemy1->gridY = 0;
						enemy_t *enemy2 = &enemies[((ty & 7) << 3) + ((tx - 1) & 7)];
						enemy2->state = 2;
					}
				}
			}
		}

		for (int32_t i = 0; i < 64; i++)
		{
			door_t *door = &doors[i];

			if (door->mapIndex != -1)
			{
				if (door->state == 0)
				{
					door->tics++;

					if (door->tics == 60)
					{
						door->state = 1;
						door->tics = 0;
					}
				}
				else if (door->state == 1)
				{
					door->offset -= 139810;

					if (door->offset < 0)
					{
						door->state = 2;
						door->offset = 0;
					}
				}
				else if (door->state == 2)
				{
					if (door->mapIndex != (ty * mapWidth + tx))
					{
						door->tics++;

						if (door->tics == 60)
						{
							door->state = 3;
							door->tics = 0;
						}
					}
				}
				else if (door->state == 3)
				{
					door->offset += 139810;

					if (door->offset > (64 << FRACBITS))
					{
						door->mapIndex = -1;
						door->state = 0;
						door->offset = 64 << FRACBITS;
					}
				}
			}
		}

		frameTics++;

		if (frameTics > 15)
		{
			frame = !frame;
			frameTics = 0;
		}

		restartLevelPressed = false;
	}
	else if (state == 0)
	{
		bloodTics++;

		if (bloodTics > 7)
		{
			uint32_t area = 0;

			for (uint32_t i = 0; i < 64; i++)
			{
				bloodHeight[i] += bloodSpeed[i];

				if (bloodHeight[i] > 64)
					bloodHeight[i] = 64;

				area += bloodHeight[i];
			}

			if (area == 4096)
				state = 3;

			bloodTics = 0;
		}

		restartLevelPressed = false;
	}
	else if (state == 2 || state == 3 || state == 4 || state == 5)
	{
		if (state < 4 && restartLevelPressed)
		{
			FILE *fp = fopen(levels[level - 1], "rb");
			int32_t cameraGridX;
			int32_t cameraGridY;
			int32_t exitGridX;
			int32_t exitGridY;
			fread(&cameraGridX, 1, sizeof(int32_t), fp);
			fread(&cameraGridY, 1, sizeof(int32_t), fp);
			cameraX = (cameraGridX * 64 + 32) << FRACBITS;
			cameraY = (cameraGridY * 64 + 32) << FRACBITS;
			fread(&cameraAngle, 1, sizeof(angle_t), fp);
			fread(&exitGridX, 1, sizeof(int32_t), fp);
			fread(&exitGridY, 1, sizeof(int32_t), fp);
			fread(&mapWidth, 1, sizeof(int32_t), fp);
			fread(&mapHeight, 1, sizeof(int32_t), fp);
			free(mapData);
			mapData = (uint32_t *)malloc(mapWidth * mapHeight * sizeof(uint32_t));
			fread(mapData, mapWidth * mapHeight, sizeof(uint32_t), fp);
			fclose(fp);
			state = 1;
			health = 100;

			for (uint32_t i = 0; i < 64; i++)
				bloodHeight[i] = 0;

			restartLevelPressed = false;
		}
		else if (state == 4 && restartLevelPressed)
		{
			state = 5;
			restartLevelPressed = false;
		}
		else if (restartLevelPressed)
		{
			state = 2;
			Mix_PlayMusic(music[(level - 1) % 2], -1);
			restartLevelPressed = false;
		}
	}
}

void Render()
{
	if (state == 1 || state == 0)
	{
		plane.minX = 64;
		plane.maxX = -1;

		plane.pad1 = 64;

		for (int32_t i = 0; i < 64; i++)
			plane.top[i] = 64;

		plane.pad2 = 64;

		uint32_t rayAngle = (cameraAngle + 31) & ANGLESMASK;

		for (int32_t i = 0; i < 64; i++)
		{
			fixed_t horizontalIntersectionY;
			fixed_t stepY;

			if (rayAngle < 128)
			{
				horizontalIntersectionY = (cameraY >> 22) * (64 << FRACBITS);
				stepY = -64 << FRACBITS;
			}
			else
			{
				horizontalIntersectionY = (cameraY >> 22) * (64 << FRACBITS) + (64 << FRACBITS);
				stepY = 64 << FRACBITS;
			}

			fixed_t horizontalIntersectionX = cameraX - fixedMul(horizontalIntersectionY - cameraY, fixedCot(rayAngle));
			fixed_t stepX = -fixedMul(stepY, fixedCot(rayAngle));
			fixed_t horizontalIntersectionDistance;
			int32_t horizontalIntersectionType;
			int32_t horizontalDoorOffset;

			if (rayAngle == 0 || rayAngle == 128)
				horizontalIntersectionDistance = INT_MAX;
			else
			{
				while (1)
				{
					int32_t gridX = horizontalIntersectionX >> 22;
					int32_t gridY = (horizontalIntersectionY >> 22) - (stepY < 0 ? 1 : 0);

					if (gridX < 0 || gridY < 0 || gridX >= mapWidth || gridY >= mapHeight)
					{
						horizontalIntersectionDistance = INT_MAX;
						break;
					}

					horizontalIntersectionType = mapData[gridY * mapWidth + gridX];

					if (horizontalIntersectionType == 1)
					{
						horizontalIntersectionDistance = fixedMul(horizontalIntersectionX - cameraX, fixedCos(cameraAngle)) - fixedMul(horizontalIntersectionY - cameraY, fixedSin(cameraAngle));
						break;
					}
					else if (horizontalIntersectionType == 2 && (((horizontalIntersectionX + (stepX >> 1)) >> FRACBITS) & 63) < (horizontalDoorOffset = (doors[((gridY & 7) << 3) + (gridX & 7)].mapIndex == (gridY * mapWidth + gridX) ? doors[((gridY & 7) << 3) + (gridX & 7)].offset >> FRACBITS : 64)))
					{
						horizontalIntersectionX += stepX >> 1;
						horizontalIntersectionY += stepY >> 1;
						horizontalIntersectionDistance = fixedMul(horizontalIntersectionX - cameraX, fixedCos(cameraAngle)) - fixedMul(horizontalIntersectionY - cameraY, fixedSin(cameraAngle));
						break;
					}
					else if (horizontalIntersectionType == 3 || horizontalIntersectionType == 4)
					{
						enemy_t *enemy = &enemies[((gridY & 7) << 3) + (gridX & 7)];
						enemy->type = horizontalIntersectionType - 3;
						enemy->gridX = gridX;
						enemy->gridY = gridY;
						enemy->render = 1;
					}
					else if (horizontalIntersectionType == 5 || horizontalIntersectionType == 6)
					{
						health_t *health = &healths[((gridY & 7) << 3) + (gridX & 7)];
						health->type = horizontalIntersectionType - 5;
						health->gridX = gridX;
						health->gridY = gridY;
						health->render = 1;
					}

					horizontalIntersectionX += stepX;
					horizontalIntersectionY += stepY;
				}
			}

			fixed_t verticalIntersectionX;

			if (rayAngle >= 64 && rayAngle < 192)
			{
				verticalIntersectionX = (cameraX >> 22) * (64 << FRACBITS);
				stepX = -64 << FRACBITS;
			}
			else
			{
				verticalIntersectionX = (cameraX >> 22) * (64 << FRACBITS) + (64 << FRACBITS);
				stepX = 64 << FRACBITS;
			}

			fixed_t verticalIntersectionY = cameraY - fixedMul(verticalIntersectionX - cameraX, fixedTan(rayAngle));
			stepY = -fixedMul(stepX, fixedTan(rayAngle));
			fixed_t verticalIntersectionDistance;
			int32_t verticalIntersectionType;
			int32_t verticalDoorOffset;

			if (rayAngle == 64 || rayAngle == 192)
				verticalIntersectionDistance = INT_MAX;
			else
			{
				while (1)
				{
					int32_t gridX = (verticalIntersectionX >> 22) - (stepX < 0 ? 1 : 0);
					int32_t gridY = verticalIntersectionY >> 22;

					if (gridX < 0 || gridY < 0 || gridX >= mapWidth || gridY >= mapHeight)
					{
						verticalIntersectionDistance = INT_MAX;
						break;
					}

					verticalIntersectionType = mapData[gridY * mapWidth + gridX];

					if (verticalIntersectionType == 1)
					{
						verticalIntersectionDistance = fixedMul(verticalIntersectionX - cameraX, fixedCos(cameraAngle)) - fixedMul((verticalIntersectionY - cameraY), fixedSin(cameraAngle));
						break;
					}
					else if (verticalIntersectionType == 2 && (((verticalIntersectionY + (stepY >> 1)) >> FRACBITS) & 63) < (verticalDoorOffset = (doors[((gridY & 7) << 3) + (gridX & 7)].mapIndex == (gridY * mapWidth + gridX) ? doors[((gridY & 7) << 3) + (gridX & 7)].offset >> FRACBITS : 64)))
					{
						verticalIntersectionX += stepX >> 1;
						verticalIntersectionY += stepY >> 1;
						verticalIntersectionDistance = fixedMul(verticalIntersectionX - cameraX, fixedCos(cameraAngle)) - fixedMul((verticalIntersectionY - cameraY), fixedSin(cameraAngle));
						break;
					}
					else if (verticalIntersectionType == 3 || verticalIntersectionType == 4)
					{
						enemy_t *enemy = &enemies[((gridY & 7) << 3) + (gridX & 7)];
						enemy->type = verticalIntersectionType - 3;
						enemy->gridX = gridX;
						enemy->gridY = gridY;
						enemy->render = 1;
					}
					else if (verticalIntersectionType == 5 || verticalIntersectionType == 6)
					{
						health_t *health = &healths[((gridY & 7) << 3) + (gridX & 7)];
						health->type = verticalIntersectionType - 5;
						health->gridX = gridX;
						health->gridY = gridY;
						health->render = 1;
					}

					verticalIntersectionX += stepX;
					verticalIntersectionY += stepY;
				}
			}

			fixed_t distance;
			const uint32_t *texture;
			int32_t textureOffsetX;

			if (horizontalIntersectionDistance < verticalIntersectionDistance)
			{
				distance = horizontalIntersectionDistance;
				texture = &((uint32_t *)textures->pixels)[0];
				textureOffsetX = (horizontalIntersectionX >> FRACBITS) & 63;

				if (horizontalIntersectionType == 2)
				{
					texture = &((uint32_t *)textures->pixels)[8192];
					textureOffsetX += 64 - horizontalDoorOffset;
				}

				if (horizontalIntersectionType != 2 && rayAngle >= 128)
					textureOffsetX = 63 - textureOffsetX;
			}
			else
			{
				distance = verticalIntersectionDistance;
				texture = &((uint32_t *)textures->pixels)[4096];
				textureOffsetX = (verticalIntersectionY >> FRACBITS) & 63;

				if (verticalIntersectionType == 2)
				{
					texture = &((uint32_t *)textures->pixels)[12288];
					textureOffsetX += 64 - verticalDoorOffset;
				}

				if (verticalIntersectionType != 2 && rayAngle >= 64 && rayAngle < 192)
					textureOffsetX = 63 - textureOffsetX;
			}

			int32_t wallHeight = fixedDiv(2048 << FRACBITS, distance) >> FRACBITS;

			if ((wallHeight & 1) != 0)
				wallHeight++;

			int32_t wallStart = (64 - wallHeight) >> 1;

			DrawWallSlice(surface, texture, textureOffsetX, i, wallStart, wallHeight);

			if (wallHeight < 64)
			{
				if (i < plane.minX)
					plane.minX = i;

				if (i > plane.maxX)
					plane.maxX = i;

				plane.top[i] = wallStart + wallHeight;
			}

			zBuffer[i] = distance;

			rayAngle = (rayAngle - 1) & ANGLESMASK;
		}

		const uint32_t *floorTexture = &((uint32_t *)textures->pixels)[16384];
		const uint32_t *ceilingTexture = &((uint32_t *)textures->pixels)[20480];

		for (int32_t i = 0; i < 32; i++)
			stop[i] = 0;

		for (int32_t x = plane.minX; x <= plane.maxX + 1; x++)
		{
			uint32_t t1 = plane.top[x - 1];
			uint32_t t2 = plane.top[x];

			while (t1 < t2)
			{
				uint32_t index = t1 - 32;

				if (stop[index] == 0)
				{
					fixed_t distance = fixedDiv(fixedDiv(1024 << FRACBITS, (index << FRACBITS) + 32768), fixedCos(31));
					fixed_t x1 = fixedMul(distance, fixedCos((cameraAngle + 31) & ANGLESMASK));
					fixed_t y1 = -fixedMul(distance, fixedSin((cameraAngle + 31) & ANGLESMASK));
					fixed_t x2 = fixedMul(distance, fixedCos((cameraAngle - 32) & ANGLESMASK));
					fixed_t y2 = -fixedMul(distance, fixedSin((cameraAngle - 32) & ANGLESMASK));
					currentX[index] = cameraX + x1;
					currentY[index] = cameraY + y1;
					stepX[index] = fixedDiv(x2 - x1, 64 << FRACBITS);
					stepY[index] = fixedDiv(y2 - y1, 64 << FRACBITS);
					currentX[index] += start[index] * stepX[index];
					currentY[index] += start[index] * stepY[index];
				}
				else
				{
					currentX[index] += (start[index] - stop[index]) * stepX[index];
					currentY[index] += (start[index] - stop[index]) * stepY[index];
				}

				uint32_t count = (x - 1) - start[index];
				uint32_t *p1 = yTable[t1] + xTable[start[index]];
				uint32_t *p2 = yTable[63 - t1] + xTable[start[index]];

				do
				{
					int32_t tx = (currentX[index] >> FRACBITS) & 63;
					int32_t ty = (currentY[index] >> FRACBITS) & 63;
					int32_t textureIndex = ty * 64 + tx;
					*p1 = floorTexture[textureIndex];
					p1++;
					*p2 = ceilingTexture[textureIndex];
					p2++;
					currentX[index] += stepX[index];
					currentY[index] += stepY[index];
				} while (count--);

				stop[index] = x;

				t1++;
			}

			while (t2 < t1)
			{
				start[t2 - 32] = x;
				t2++;
			}
		}

		for (int32_t i = 0; i < 64; i++)
		{
			health_t *health = &healths[i];

			if (health->render)
			{
				fixed_t distance = fixedMul(((health->gridX << 22) + (32 << FRACBITS)) - cameraX, fixedCos(cameraAngle)) - fixedMul(((health->gridY << 22) + (32 << FRACBITS)) - cameraY, fixedSin(cameraAngle));
				fixed_t x = fixedMul(((health->gridX << 22) + (32 << FRACBITS)) - cameraX, fixedSin(cameraAngle)) + fixedMul(((health->gridY << 22) + (32 << FRACBITS)) - cameraY, fixedCos(cameraAngle));
				x = fixedDiv(x << 5, distance);
				int32_t spriteSize = fixedDiv(2048 << FRACBITS, distance) >> FRACBITS;
				if ((spriteSize & 1) != 0)
					spriteSize++;
				int32_t spriteX = 32 + (x >> FRACBITS) - (spriteSize >> 1);
				int32_t spriteY = (64 - spriteSize) >> 1;
				const uint32_t *sprite = &((uint32_t *)sprites->pixels)[frames[4 + health->type]];
				DrawSprite(surface, sprite, spriteX, spriteY, spriteSize, distance, 0);
				health->render = 0;
			}
		}

		for (int32_t i = 0; i < 64; i++)
		{
			enemy_t *enemy = &enemies[i];

			if (enemy->render)
			{
				fixed_t distance = fixedMul(((enemy->gridX << 22) + (32 << FRACBITS)) - cameraX, fixedCos(cameraAngle)) - fixedMul(((enemy->gridY << 22) + (32 << FRACBITS)) - cameraY, fixedSin(cameraAngle));
				fixed_t x = fixedMul(((enemy->gridX << 22) + (32 << FRACBITS)) - cameraX, fixedSin(cameraAngle)) + fixedMul(((enemy->gridY << 22) + (32 << FRACBITS)) - cameraY, fixedCos(cameraAngle));
				x = fixedDiv(x << 5, distance);
				int32_t spriteSize = fixedDiv(2048 << FRACBITS, distance) >> FRACBITS;
				if ((spriteSize & 1) != 0)
					spriteSize++;
				int32_t spriteX = 32 + (x >> FRACBITS) - (spriteSize >> 1);
				int32_t spriteY = (64 - spriteSize) >> 1;
				const uint32_t *sprite = &((uint32_t *)sprites->pixels)[frames[enemy->type * 2 + frame]];
				DrawSprite(surface, sprite, spriteX, spriteY, spriteSize, distance, enemy->damage);
				enemy->render = 0;
			}
		}

		SDL_Rect srcrect;

		if (fireWeaponPressed)
			srcrect = { 25, 0, 25, 26 };
		else
			srcrect = { 0, 0, 25, 26 };

		SDL_Rect dstrect = { 39, 38, 25, 26 };
		SDL_BlitSurface(hand, &srcrect, surface, &dstrect);

		SDL_Rect rect = { 16, 60, fixedDiv(health << 21, 100 << FRACBITS) >> FRACBITS, 2 };
		SDL_FillRect(surface, &rect, SDL_MapRGB(surface->format, 255, 0, 0));

		if (state == 0)
		{
			for (uint32_t i = 0; i < 64; i++)
			{
				SDL_Rect rect = { (int)i, 0, 1, (int)bloodHeight[i] };
				SDL_FillRect(surface, &rect, SDL_MapRGB(surface->format, 255, 0, 0));
			}
		}
	}
	else if (state == 2)
		SDL_BlitSurface(title, NULL, surface, NULL);
	else if (state == 3)
		SDL_BlitSurface(dead, NULL, surface, NULL);
	else if (state == 4)
		SDL_BlitSurface(end, NULL, surface, NULL);
	else if (state == 5)
		SDL_BlitSurface(credits, NULL, surface, NULL);

	SDL_UpdateTexture(texture, NULL, surface->pixels, surface->pitch);

	SDL_RenderCopy(renderer, texture, NULL, NULL);
}

int main(int argc, char *argv[])
{
	uint32_t scale = 1;

	if (argc > 1)
	{
		for (int i = 1; i < argc; i++)
		{
			if (!SDL_strcasecmp(argv[i], "-scale"))
			{
				scale = atoi(argv[i + 1]);

				if (scale < 1 || scale > 8 || scale & (scale - 1))
					scale = 1;
			}
		}
	}

	SDL_Init(SDL_INIT_VIDEO);

	Mix_Init(MIX_INIT_MP3);

	window = SDL_CreateWindow("Eternal Horror", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 64 * scale, 64 * scale, SDL_WINDOW_BORDERLESS);

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024);

	Uint32 pixelFormat = SDL_GetWindowPixelFormat(window);

	texture = SDL_CreateTexture(renderer, pixelFormat, SDL_TEXTUREACCESS_STREAMING, 64, 64);

	surface = SDL_CreateRGBSurfaceWithFormat(0, 64, 64, 32, pixelFormat);

	SDL_Surface *tempSurface = SDL_LoadBMP("textures.bmp");

	textures = SDL_ConvertSurfaceFormat(tempSurface, pixelFormat, 0);

	SDL_FreeSurface(tempSurface);

	tempSurface = SDL_LoadBMP("sprites.bmp");

	sprites = SDL_ConvertSurfaceFormat(tempSurface, pixelFormat, 0);

	SDL_FreeSurface(tempSurface);

	tempSurface = SDL_LoadBMP("hand.bmp");

	hand = SDL_ConvertSurfaceFormat(tempSurface, pixelFormat, 0);

	SDL_FreeSurface(tempSurface);

	SDL_SetColorKey(hand, SDL_TRUE, SDL_MapRGB(hand->format, 152, 0, 136));

	tempSurface = SDL_LoadBMP("title.bmp");

	title = SDL_ConvertSurfaceFormat(tempSurface, pixelFormat, 0);

	SDL_FreeSurface(tempSurface);

	tempSurface = SDL_LoadBMP("dead.bmp");

	dead = SDL_ConvertSurfaceFormat(tempSurface, pixelFormat, 0);

	SDL_FreeSurface(tempSurface);

	tempSurface = SDL_LoadBMP("end.bmp");

	end = SDL_ConvertSurfaceFormat(tempSurface, pixelFormat, 0);

	SDL_FreeSurface(tempSurface);

	tempSurface = SDL_LoadBMP("credits.bmp");

	credits = SDL_ConvertSurfaceFormat(tempSurface, pixelFormat, 0);

	SDL_FreeSurface(tempSurface);

	music[0] = Mix_LoadMUS("music1.mp3");
	music[1] = Mix_LoadMUS("music2.mp3");

	SDL_SetRelativeMouseMode(SDL_TRUE);

	for (uint32_t i = 0; i < 64; i++)
		yTable[i] = (uint32_t *) ((uint8_t *) surface->pixels + (((surface->h - 64) >> 1) + i) * surface->pitch);

	for (uint32_t i = 0; i < 64; i++)
		xTable[i] = ((surface->w - 64) >> 1) + i;

	Mix_PlayMusic(music[(level - 1) % 2], -1);

	srand((unsigned)time(NULL));

	for (uint32_t i = 0; i < 64; i++)
		bloodSpeed[i] = rand() % 4 + 2;

	lastTime = SDL_GetTicks();

	while (running)
	{
		PollInput();
		Update();
		Render();
		SDL_RenderPresent(renderer);
	}

	Mix_FreeMusic(music[0]);
	Mix_FreeMusic(music[1]);

	free(mapData);

	SDL_FreeSurface(credits);

	SDL_FreeSurface(end);

	SDL_FreeSurface(dead);

	SDL_FreeSurface(title);

	SDL_FreeSurface(hand);

	SDL_FreeSurface(sprites);

	SDL_FreeSurface(textures);

	SDL_FreeSurface(surface);

	SDL_DestroyTexture(texture);

	Mix_CloseAudio();

	SDL_DestroyRenderer(renderer);

	SDL_DestroyWindow(window);

	Mix_Quit();

	SDL_Quit();

	return 0;
}