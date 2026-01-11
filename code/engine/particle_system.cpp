#include "particle_system.h"

#include "mesh.h"
#include "shader.h"

int CompareParticles(const void *a, const void *b)
{
    ParticleData *p1 = (ParticleData *)a;
    ParticleData *p2 = (ParticleData *)b;

    if (p1->cameraDist > p2->cameraDist) return -1;
    if (p1->cameraDist < p2->cameraDist) return 1;

    return 0;
}
