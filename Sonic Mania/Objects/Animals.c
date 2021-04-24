#include "../SonicMania.h"

ObjectAnimals *Animals;

void Animals_Update(void)
{
    EntityAnimals *entity = (EntityAnimals *)RSDK_sceneInfo->entity;
    StateMachine_Run(entity->state);
    if (!entity->behaviour && !RSDK.CheckOnScreen(entity, NULL))
        RSDK.ResetEntityPtr(entity, TYPE_BLANK, NULL);
}

void Animals_LateUpdate(void) {}

void Animals_StaticUpdate(void) {}

void Animals_Draw(void)
{
    EntityAnimals *entity = (EntityAnimals *)RSDK_sceneInfo->entity;
    RSDK.DrawSprite(&entity->data, NULL, false);
}

void Animals_Create(void *data)
{
    EntityAnimals *entity = (EntityAnimals *)RSDK_sceneInfo->entity;
    if (entity->behaviour == 2)
        entity->active = ACTIVE_BOUNDS;
    else
        entity->active = ACTIVE_NORMAL;
    entity->drawFX |= FX_FLIP;
    int type              = 2;
    entity->visible       = 1;
    entity->updateRange.x = 0x400000;
    entity->updateRange.y = 0x400000;
    entity->drawOrder     = Zone->drawOrderLow;
#if RETRO_USE_PLUS
    if (!(globals->secrets & 1))
        type = voidToInt(data);
#endif
    if (!entity->type && RSDK.Random(0, 256, &Zone->randKey) == 21) {
        type                  = 2;
        entity->velocity.y    = -0x40000;
        entity->type          = type - 1;
        entity->state         = Animals_State_Freed;
        entity->hitbox.top    = -Animals->hitboxes[entity->type] >> 16;
        entity->hitbox.left   = -4;
        entity->hitbox.right  = 4;
        entity->hitbox.bottom = Animals->hitboxes[entity->type] >> 16;
        RSDK.SetSpriteAnimation(Animals->spriteIndex, 2 * entity->type, &entity->data, true, 0);
    }
    else if (type) {
        entity->velocity.y    = -0x40000;
        entity->type          = type - 1;
        entity->state         = Animals_State_Freed;
        entity->hitbox.top    = -Animals->hitboxes[entity->type] >> 16;
        entity->hitbox.left   = -4;
        entity->hitbox.right  = 4;
        entity->hitbox.bottom = Animals->hitboxes[entity->type] >> 16;
        RSDK.SetSpriteAnimation(Animals->spriteIndex, 2 * entity->type, &entity->data, true, 0);
    }
    else if (entity->behaviour == 1) {
        entity->active = ACTIVE_BOUNDS;
        switch (entity->type) {
            case 0:
            case 5:
            case 8: entity->state = Animals_State_FollowPlayer_Bird; break;
            case 1:
            case 2:
            case 3:
            case 4:
            case 6:
            case 7:
            case 9:
            case 10:
            case 11: entity->state = Animals_State_FollowPlayer_Normal; break;
            default: break;
        }
        Animals_CheckPlayerPos();
        entity->velocity.y = Animals->yVelocity[entity->type];
        if (entity->direction == FLIP_NONE)
            entity->velocity.x = -Animals->xVelocity[entity->type];
        else
            entity->velocity.x = Animals->xVelocity[entity->type];
        RSDK.SetSpriteAnimation(Animals->spriteIndex, 2 * entity->type + 1, &entity->data, true, 0);
    }
    else {
        entity->state = Animals_State_BounceAround;
    }
}

void Animals_StageLoad(void)
{
    Animals->spriteIndex = RSDK.LoadSpriteAnimation("Global/Animals.bin", SCOPE_STAGE);
    if (RSDK.GetObjectIDByName("Platform"))
        Animals->hasPlatform = true;
    if (RSDK.GetObjectIDByName("Bridge"))
        Animals->hasBridge = true;
}

void Animals_CheckPlayerPos(void)
{
    EntityAnimals *entity = (EntityAnimals *)RSDK_sceneInfo->entity;

    switch (entity->behaviour) {
        default:
        case 0: entity->direction = FLIP_X; break;
        case 1: {
            EntityPlayer *player = (EntityPlayer *)RSDK.GetEntityByID(SLOT_PLAYER1);
            if (!player) {
                entity->direction = FLIP_NONE;
            }
            else if (entity->position.x < player->position.x) {
                entity->direction = FLIP_NONE;
            }
            break;
        }
        case 2: entity->direction = RSDK.Random(0, 2, &Zone->randKey); break;
    }

    if (!entity->direction)
        entity->velocity.x = -Animals->xVelocity[entity->type];
    else
        entity->velocity.x = Animals->xVelocity[entity->type];
}

bool32 Animals_CheckPlatformCollision(void *plat)
{
    EntityPlatform *platform = (EntityPlatform *)plat;
    EntityAnimals *entity    = (EntityAnimals *)RSDK_sceneInfo->entity;
    bool32 flag              = false;
    if (platform->state != Platform_State_Falling && platform->state != Platform_State_OffScreenReset) {
        platform->position.x = platform->drawPos.x - platform->collisionOffset.x;
        platform->position.y = platform->drawPos.y - platform->collisionOffset.y;
        if (platform->collision <= 1) {
            flag = RSDK.CheckObjectCollisionPlatform(platform, RSDK.GetHitbox(&platform->animData, 0), entity, &entity->hitbox, true);
        }
        else if (platform->collision == 2 && RSDK.CheckObjectCollisionTouchBox(platform, &platform->hitbox, entity, &entity->hitbox)) {
            if (entity->collisionLayers & Zone->moveID) {
                TileLayer *move  = RSDK.GetSceneLayer(Zone->moveLayer);
                move->position.x = -(platform->drawPos.x + platform->tileOrigin.x) >> 16;
                move->position.y = -(platform->drawPos.y + platform->tileOrigin.y) >> 16;
            }
            if (entity->velocity.y >= 0x3800)
                flag = true;
        }
        platform->position.x = platform->centerPos.x;
        platform->position.y = platform->centerPos.y;
    }
    return flag;
}

bool32 Animals_CheckGroundCollision(void)
{
    EntityAnimals *entity = (EntityAnimals *)RSDK_sceneInfo->entity;
    if (entity->velocity.y <= 0)
        return false;
    if (RSDK.ObjectTileCollision(entity, Zone->fgLayers, CMODE_FLOOR, 0, 0, Animals->hitboxes[entity->type], false))
        return true;
    if (Animals->hasPlatform) {
        foreach_active(Platform, platform)
        {
            if (Animals_CheckPlatformCollision(platform))
                return true;
        }
    }

    if (Animals->hasBridge) {
        foreach_active(Bridge, bridge)
        {
            if (entity->position.x > bridge->startPos && entity->position.x < bridge->endPos && entity->velocity.y >= 0) {
                Hitbox bridgeHitbox;
                bridgeHitbox.left  = -0x400;
                bridgeHitbox.right = 0x400;

                int divisor = 0;
                int ang     = 0;
                if (entity->position.x - bridge->startPos <= divisor) {
                    divisor = bridge->stoodPos;
                    ang     = (entity->position.x - bridge->startPos) << 7;
                }
                else {
                    divisor = bridge->endPos - divisor - bridge->startPos;
                    ang     = (bridge->endPos - entity->position.x) << 7;
                }

                int hitY = (bridge->field_6C * RSDK.Sin512(ang / divisor) >> 9) - 0x80000;
                if (entity->velocity.y >= 0x8000) {
                    bridgeHitbox.bottom = (hitY >> 16);
                    bridgeHitbox.top    = (hitY >> 16) - 8;
                }
                else {
                    bridgeHitbox.top    = (hitY >> 16);
                    bridgeHitbox.bottom = (hitY >> 16) + 8;
                }
                if (RSDK.CheckObjectCollisionTouchBox(bridge, &bridgeHitbox, entity, &entity->hitbox)) {
                    entity->position.y = hitY + bridge->position.y - (entity->hitbox.bottom << 16);
                    return true;
                }
            }
        }
    }
    return false;
}

void Animals_State_Freed(void)
{
    EntityAnimals *entity = (EntityAnimals *)RSDK_sceneInfo->entity;
    entity->position.y += entity->velocity.y;
    entity->velocity.y += 0x3800;
    RSDK.ProcessAnimation(&entity->data);
    if (Animals_CheckGroundCollision()) {
        RSDK.SetSpriteAnimation(Animals->spriteIndex, 2 * entity->type + 1, &entity->data, true, 0);
        switch (entity->type) {
            case 0:
            case 5:
            case 8: entity->state = Animals_State_FollowPlayer_Bird; break;
            case 1:
            case 2:
            case 3:
            case 4:
            case 6:
            case 7:
            case 9:
            case 0xA:
            case 0xB: entity->state = Animals_State_FollowPlayer_Normal; break;
            default: break;
        }
        Animals_CheckPlayerPos();
        entity->velocity.y = Animals->yVelocity[entity->type];
    }
}

void Animals_State_FollowPlayer_Normal(void)
{
    EntityAnimals *entity = (EntityAnimals *)RSDK_sceneInfo->entity;
    entity->position.x += entity->velocity.x;
    entity->position.y += entity->velocity.y;
    entity->velocity.y += Animals->gravityStrength[entity->type];
    if (Animals_CheckGroundCollision()) {
        Animals_CheckPlayerPos();
        entity->velocity.y = Animals->yVelocity[entity->type];
    }
    entity->data.frameID = entity->velocity.y < 0;
}

void Animals_State_FollowPlayer_Bird(void)
{
    EntityAnimals *entity = (EntityAnimals *)RSDK_sceneInfo->entity;
    entity->position.x += entity->velocity.x;
    entity->position.y += entity->velocity.y;
    entity->velocity.y += Animals->gravityStrength[entity->type];
    if (Animals_CheckGroundCollision()) {
        Animals_CheckPlayerPos();
        entity->velocity.y = Animals->yVelocity[entity->type];
    }
    RSDK.ProcessAnimation(&entity->data);
}

void Animals_State_BounceAround(void)
{
    EntityAnimals *entity = (EntityAnimals *)RSDK_sceneInfo->entity;
    if (entity->delay <= 0)
        entity->state = Animals_State_Freed;
    else
        entity->delay--;
}

void Animals_EditorDraw(void) {}

void Animals_EditorLoad(void) {}

void Animals_Serialize(void)
{
    RSDK_EDITABLE_VAR(Animals, VAR_ENUM, type);
    RSDK_EDITABLE_VAR(Animals, VAR_ENUM, behaviour);
}