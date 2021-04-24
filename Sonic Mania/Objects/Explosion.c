#include "../SonicMania.h"

ObjectExplosion *Explosion;

void Explosion_Update(void)
{
    EntityExplosion *entity = (EntityExplosion *)RSDK_sceneInfo->entity;
    RSDK.ProcessAnimation(&entity->data);
    entity->position.x += entity->velocity.x;
    entity->position.y += entity->velocity.y;
    if (entity->data.frameID == entity->data.frameCount - 1)
        RSDK.ResetEntityPtr(entity, 0, 0);
}

void Explosion_LateUpdate(void)
{

}

void Explosion_StaticUpdate(void)
{

}

void Explosion_Draw(void)
{
    EntityExplosion *entity = (EntityExplosion *)RSDK_sceneInfo->entity;
    RSDK.DrawSprite(&entity->data, NULL, false);
}

void Explosion_Create(void* data)
{
    EntityExplosion *entity = (EntityExplosion *)RSDK_sceneInfo->entity;
    entity->active  = ACTIVE_NORMAL;
    entity->visible = true;
    if (entity->planeFilter > 0 && ((byte)entity->planeFilter - 1) & 2)
        entity->drawOrder = Zone->drawOrderHigh;
    else
        entity->drawOrder = Zone->drawOrderLow;
    RSDK.SetSpriteAnimation(Explosion->spriteIndex, (int)(size_t)data, &entity->data, true, 0);
}

void Explosion_StageLoad(void)
{
    Explosion->spriteIndex = RSDK.LoadSpriteAnimation("Global/Explosions.bin", SCOPE_STAGE);
    Explosion->sfx_Destroy = RSDK.GetSFX("Global/Destroy.wav");
}

void Explosion_EditorDraw(void)
{

}

void Explosion_EditorLoad(void)
{

}

void Explosion_Serialize(void)
{

}
