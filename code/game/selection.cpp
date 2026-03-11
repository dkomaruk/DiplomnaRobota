#include "selection.h"

#include "game.h"
#include "intersections.h"

void SelectSingleObject(Game *game, Ray *pickingRay)
{
    int closestId = -1;
    float closestDistance = FLT_MAX;

    for(int entityIndex = 0; entityIndex < game->sceneEntities.size(); entityIndex++)
    {
        Entity *entity = game->sceneEntities[entityIndex];

        AABB transformedAABB = {glm::vec3(entity->modelMatPosScale * glm::vec4(entity->aabb.min, 1.0f)),
                                glm::vec3(entity->modelMatPosScale * glm::vec4(entity->aabb.max, 1.0f))};

        float intersectionDist;
        bool result = RayBoxIntersection(&transformedAABB, pickingRay, &intersectionDist);
        if(result && (intersectionDist < closestDistance))
        {
            closestDistance = intersectionDist;
            closestId = entity->id;
        }
    }

    if(!game->input.keys[SDL_SCANCODE_LSHIFT])
    {
        game->selectedIDs.clear();
    }

    if(closestId >= 0)
    {
        uint16 pickedId = (uint16)closestId;
        bool isAlreadyPicked = game->selectedIDs.count(pickedId);
        if(isAlreadyPicked && game->input.keys[SDL_SCANCODE_LSHIFT])
        {
            game->selectedIDs.erase(pickedId);
        }
        else
        {
            game->selectedIDs.insert(pickedId);
        }
    }
    game->lastSelectedId = closestId;
}

void SelectMultipleObjects(Game *game)
{
    glm::vec2 mouse = game->input.mousePos;
    mouse.y = game->windowSize.y - mouse.y;

    game->selectionBox.start.y = game->windowSize.y - game->selectionBox.start.y;

    glm::vec2 min = glm::min(game->selectionBox.start, mouse);
    glm::vec2 max = glm::max(game->selectionBox.start, mouse);

    glm::vec4 viewport = glm::vec4(0.0f, 0.0f, game->windowSize);

    glm::vec2 positions[4];
    positions[Frustum_BL] = glm::vec2(min.x, min.y);
    positions[Frustum_BR] = glm::vec2(max.x, min.y);
    positions[Frustum_UL] = glm::vec2(min.x, max.y);
    positions[Frustum_UR] = glm::vec2(max.x, max.y);

    float rayLength = 2000.0f;
    glm::vec3 nearPoints[4], farPoints[4];
    for(int i = 0; i < ArrayCount(positions); i++)
    {
        glm::vec3 rayNear = glm::unProject(glm::vec3(positions[i], 0.0f), game->view, game->perspectiveProjection, viewport);
        glm::vec3 rayFar = glm::unProject(glm::vec3(positions[i], 1.0f), game->view, game->perspectiveProjection, viewport);

        glm::vec3 rayDirection = glm::normalize(rayFar - rayNear);
        glm::vec3 rayOrigin = game->camera.position + rayDirection * 0.5f;

        nearPoints[i] = rayOrigin;
        farPoints[i] = rayOrigin + rayDirection * rayLength;

        UpdateLine(&game->frustumLines[i], nearPoints[i], farPoints[i]);
    }

    float planeNormalLength = 0.1f;

    Frustum frustum = CreateFrustum(nearPoints, farPoints);

//#ifdef DEBUG
    UpdateLine(&game->frustumNormals[Frustum_R], nearPoints[Frustum_UR],
                nearPoints[Frustum_UR] + frustum.planes[Frustum_R].normal * planeNormalLength);
    UpdateLine(&game->frustumNormals[Frustum_U], nearPoints[Frustum_UR],
                nearPoints[Frustum_UR] + frustum.planes[Frustum_U].normal * planeNormalLength);
    UpdateLine(&game->frustumNormals[Frustum_F], nearPoints[Frustum_BR],
                nearPoints[Frustum_BR] + frustum.planes[Frustum_F].normal * planeNormalLength);
    UpdateLine(&game->frustumNormals[Frustum_L], nearPoints[Frustum_UL],
                nearPoints[Frustum_UL] + frustum.planes[Frustum_L].normal * planeNormalLength);
    UpdateLine(&game->frustumNormals[Frustum_B], nearPoints[Frustum_BL],
                nearPoints[Frustum_BL] + frustum.planes[Frustum_B].normal * planeNormalLength);
    UpdateLine(&game->frustumNormals[Frustum_N], nearPoints[Frustum_BL],
                nearPoints[Frustum_BL] + frustum.planes[Frustum_N].normal * planeNormalLength);
//#endif

    std::vector<uint16> pickedIDs;
    for(int entityIndex = 0; entityIndex < game->sceneEntities.size(); entityIndex++)
    {
        Entity *entity = game->sceneEntities[entityIndex];

        AABB transformedAABB = {glm::vec3(entity->modelMatPosScale * glm::vec4(entity->aabb.min, 1.0f)),
                                glm::vec3(entity->modelMatPosScale * glm::vec4(entity->aabb.max, 1.0f))};

        if(FrustumAABBIntersectionTest(&frustum, &transformedAABB))
        {
            pickedIDs.push_back(entity->id);
        }
    }

    if(!game->input.keys[SDL_SCANCODE_LSHIFT])
    {
        game->selectedIDs.clear();
        game->selectedIDs.insert(pickedIDs.begin(), pickedIDs.end());
    }
    else
    {
        bool hasNewEntities = false;
        for(int pickedIndex = 0; pickedIndex < pickedIDs.size(); pickedIndex++)
        {
            if(!game->selectedIDs.count(pickedIDs[pickedIndex]))
            {
                hasNewEntities = true;
                break;
            }
        }

        if(hasNewEntities) //Append
        {
            game->selectedIDs.insert(pickedIDs.begin(), pickedIDs.end());
        }
        else //Deselect if no new entities
        {
            for(int pickedIndex = 0; pickedIndex < pickedIDs.size(); pickedIndex++)
            {
                game->selectedIDs.erase(pickedIDs[pickedIndex]);
            }
        }
    }
}

void RenderSelectionBox(Game *game, SelectionBox *box)
{
    RenderRectUI(game, box->start, box->size, game->selectionBoxShader);
}
