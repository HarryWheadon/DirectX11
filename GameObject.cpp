#include "GameObject.h"

GameObject::GameObject()
{
}

XMFLOAT4X4* GameObject::GetWorld()
{
    XMStoreFloat4x4(&world, XMMatrixRotationY(rotation) * XMMatrixTranslation(position.x, position.y, position.z) * XMMatrixScaling(scale.x, scale.y, scale.z));
    return &world;
}
