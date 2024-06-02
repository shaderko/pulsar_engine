#version 450 core

in vec3 GridPos;
in vec3 FragPos;
in vec2 TexCoords;
flat in float textureIndex;
out vec4 FragColor;

uniform mat4 view;
uniform sampler2DArray heightMaps; // Array of height map textures each chunk is 64 in depth so 64 textures
uniform vec3 viewPos;

void setupDDA(in vec3 rayOrigin, in vec3 rayDir, in float size, out ivec3 rayStep, out vec3 tMax, out vec3 tDelta, out ivec3 mapCheck) {
    // Ray step (if ray dir is [-3, 2, 1] -> ray step is [-1, 1, 1])
    rayStep = ivec3(sign(rayDir));

    // Map check is the current ray position in the grid (ray origin [5, 0, 0] -> [0, 0, 0], [32, 0, 0] -> [1, 0, 0])
    mapCheck = ivec3(floor(rayOrigin / size));

    // Caclulate the next boundary we hit when we step
    vec3 nextBoundary = (vec3(mapCheck) + step(vec3(0.0), rayDir)) * size;

    // Get the distance to that boundary from where we are currently
    vec3 distanceToBoundary = nextBoundary - rayOrigin;

    // Calculate tMax for all axis to hit the boundary
    tMax = distanceToBoundary / rayDir;
    
    // Calculate tDelta (how much t increases when we step to the next cell) abs to account for negative rays
    tDelta = abs(size / rayDir);
}

// Function to intersect ray with axis-aligned bounding box
bool intersectRayWithBox(vec3 rayOrigin, vec3 rayDirection, vec3 boxMin, vec3 boxMax, out float tMin, out float tMax) {
    vec3 invDir = 1.0 / rayDirection;
    vec3 t0 = (boxMin - rayOrigin) * invDir;
    vec3 t1 = (boxMax - rayOrigin) * invDir;

    vec3 tMinVec = min(t0, t1);
    vec3 tMaxVec = max(t0, t1);

    tMin = max(max(tMinVec.x, tMinVec.y), tMinVec.z);
    tMax = min(min(tMaxVec.x, tMaxVec.y), tMaxVec.z);

    return tMax >= max(tMin, 0.0);
}

// Function to perform Parallax Occlusion Mapping
float parallaxOcclusionMapping(vec3 viewDir, float textureIndex) {
    // Parameters
    int numLayers = 64;

    // Calculate how far from the camera inside the grid, tEntry is how much to enter the grid, tExit how far until we exit the grid
    float tEntry, tExit;
    if (!intersectRayWithBox(viewPos, viewDir, vec3(GridPos), vec3(GridPos + vec3(64)), tEntry, tExit)) {
        // We couldn't find an intersection
        return 0.0;
    }

    if (tEntry < 0.0) {
        tEntry = 0.0;
    }

    float t = tEntry + 0.0001;
    vec3 cell = vec3(0);

    while (t < tExit) {
        vec3 samplePoint = viewPos + viewDir * t;
        cell = floor(mod(samplePoint, 64));

        // Normalize UV coordinates to range [0, 1]
        vec2 uv = vec2(cell.xy) / 63.0;
        // Compute the correct layer in the texture array
        int layer = int(textureIndex) + int(cell.z);
        float height = texture(heightMaps, vec3(uv, layer)).r;

        if (height > 0.0) {
            return height;
        }

        vec3 D1 = (mix(floor(samplePoint), ceil(samplePoint), step(0.0, viewDir)) - samplePoint) / viewDir;
        float d1 = min(D1.x, min(D1.y, D1.z));

        t += d1 + 0.0001;
    }

    return 0.0;
}

void main() {
    vec3 viewDir = normalize(FragPos - viewPos);

    float height = parallaxOcclusionMapping(viewDir, textureIndex * 64);
    if (height <= 0.0)
    {
        // FragColor = vec4(0.0, 0.0, 1.0, 1.0);
        // return;
        discard;
    }

    FragColor = vec4(vec3(height), 1.0);
}