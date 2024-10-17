#version 460 core

uniform vec3 pixel00_loc;
uniform vec3 pixel_delta_u;
uniform vec3 pixel_delta_v;
uniform float defocus_angle = 0;
// uniform float focus_dist = 10;
uniform vec2 resolution;
uniform float rand_seed;
uniform vec3 cam_center;
// uniform vec3 cam_lookat;
uniform vec3 defocus_disk_u;
uniform vec3 defocus_disk_v;
uniform vec3 viewport_upper_left;
uniform int num_spheres;
uniform int max_depth = 10;

out vec4 o_color;
in vec2 tex_coords;

struct Sphere {
    vec3 center;
    float radius;
    uint material_index;
};

struct MaterialDielectric {
    float refraction_index;
};

struct MaterialMetal {
    vec3 albedo;
    float fuzz;
};

struct MaterialLambertian {
    vec3 albedo;
};

layout(std430, binding = 0) readonly buffer sphere_buffer {
    Sphere spheres[];
};

layout(std430, binding = 1) readonly buffer dielectric_mat_buffer {
    MaterialDielectric dielectric_materials[];
};

layout(std430, binding = 2) readonly buffer metal_mat_buffer {
    MaterialMetal metal_materials[];
};

layout(std430, binding = 3) readonly buffer lambertian_mat_buffer {
    MaterialLambertian lambertian_materials[];
};

float rand(vec2 co) {
    return fract(sin(dot(co.xy * rand_seed, vec2(12.9898, 78.233))) * 43758.5453123);
}

float RandFloat(vec2 seed) {
    return rand(seed);
}

float RandFloat(float min, float max, vec2 seed) {
    return min + RandFloat(seed) * (max - min);
}

float randFromSeed(float seed) {
    return fract(sin(rand_seed * seed) * 43758.5453123);
}

const float epsilon = 1e-8;
bool NearZero(vec3 v) {
    return (abs(v.x) < epsilon) && (abs(v.y) < epsilon) && (abs(v.z) < epsilon);
}

vec3 randomSequence(vec2 uv, int index) {
    return vec3(
        randFromSeed(uv.x * 100.0 + float(index)),
        randFromSeed(uv.y * 200.0 + float(index + 1)),
        randFromSeed(uv.x * 300.0 + float(index + 2))
    );
}

vec3 RandVec3(float min, float max, vec2 seed) {
    return vec3(RandFloat(min, max, seed), RandFloat(min, max, seed + 1.0), RandFloat(min, max, seed + 2.0));
}

vec3 RandInUnitSphere(vec2 seed) {
    vec3 p;
    float length_sq;
    do {
        p = RandVec3(-1.0, 1.0, seed);
        length_sq = dot(p, p);
    } while (length_sq >= 1.0 || length_sq < 1e-6);

    return p;
}

vec3 RandUnitVec3(vec2 seed) {
    return normalize(RandInUnitSphere(seed));
}

vec3 RandInUnitDisk(vec2 seed) {
    vec3 p;
    do {
        p = vec3(RandFloat(-1, 1, seed), RandFloat(-1, 1, seed + 1), 0.0);
    } while (dot(p, p) >= 1.0);
    return p;
}

vec3 DefocusDiskSample(vec2 seed) {
    vec3 p = RandInUnitDisk(seed);
    return cam_center + (p.x * defocus_disk_u) + (p.y * defocus_disk_v);
}

struct Ray {
    vec3 origin;
    vec3 direction;
};
vec3 RayAt(Ray ray, float t) {
    return ray.origin + ray.direction * t;
}

struct HitRecord {
    vec3 point;
    vec3 normal;
    float t;
    uint material_index;
    bool front_face;
};

Ray GetRay(vec2 uv) {
    // pixel center in world space
    // how to get pixel00_loc
    vec3 pixel_center = pixel00_loc + gl_FragCoord.x + gl_FragCoord.y;
    float px = -0.5 + randFromSeed(1);
    float py = -0.5 + randFromSeed(2);
    vec3 pixel_sample_square = vec3(px * pixel_delta_u.x, py * pixel_delta_v.y, 0.0);
    Ray ray;
    pixel_center += pixel_sample_square;
    vec3 center = (defocus_angle <= 0) ? cam_center : DefocusDiskSample(uv);
    ray.origin = center;
    ray.direction = normalize(pixel_center - center);
    return ray;
}

struct Interval {
    float min;
    float max;
};

bool IntervalContains(Interval i, float x) {
    return i.min <= x && x <= i.max;
}

bool IntervalSurrounds(Interval i, float x) {
    return i.min < x && x < i.max;
}

void HitRecordSetFaceNormal(Ray r, vec3 outward_normal, inout bool front_face, inout vec3 normal) {
    front_face = dot(r.direction, outward_normal) < 0;
    normal = float((int(front_face) << 1) - 1.0) * outward_normal;
}

bool SphereHit(Sphere sphere, Ray r, Interval ray_t, inout HitRecord rec) {
    vec3 oc = sphere.center - r.origin;
    float a = dot(r.direction, r.direction);
    float h = dot(r.direction, oc);
    float c = dot(oc, oc) - sphere.radius * sphere.radius;
    float discriminant = h * h - a * c;
    // TODO: branchless
    if (discriminant < 0) return false;
    float sqrtd = sqrt(discriminant);
    float root = (h - sqrtd) / a;
    if (!IntervalSurrounds(ray_t, root)) {
        root = (h + sqrtd) / a;
        if (!IntervalSurrounds(ray_t, root)) {
            return false;
        }
    }
    rec.t = root;
    rec.point = RayAt(r, rec.t);
    rec.material_index = sphere.material_index;
    vec3 outward_normal = (rec.point - sphere.center) / sphere.radius;
    HitRecordSetFaceNormal(r, outward_normal, rec.front_face, rec.normal);
    return true;
}

bool HitAny(Ray r, Interval ray_t, inout HitRecord rec) {
    HitRecord temp_rec;
    bool hit_any = false;
    // hit spheres
    for (int i = 0; i < num_spheres; i++) {
        if (SphereHit(spheres[i], r, ray_t, temp_rec)) {
            hit_any = true;
            ray_t.max = temp_rec.t;
            rec = temp_rec;
        }
    }
    return hit_any;
}

float SchlickReflectance(float cosine, float refraction_index) {
    float r0 = (1 - refraction_index) / (1 + refraction_index);
    r0 = r0 * r0;
    return r0 + (1 - r0) * pow(1 - cosine, 5);
}

bool DielectricScatter(MaterialDielectric mat, Ray r, HitRecord rec, inout vec3 attenuation, inout Ray scattered, vec2 seed) {
    attenuation = vec3(1.0);
    float ri = rec.front_face ? (1.0 / mat.refraction_index) : mat.refraction_index;
    vec3 unit_dir = normalize(r.direction);
    float cos_theta = min(dot(-unit_dir, rec.normal), 1.0);
    float sin_theta = sqrt(1.0 - cos_theta * cos_theta);
    bool cannot_refract = ri * sin_theta > 1.0;
    vec3 direction;
    if (cannot_refract || SchlickReflectance(cos_theta, ri) > rand(seed)) {
        direction = reflect(unit_dir, rec.normal);
    } else {
        direction = refract(unit_dir, rec.normal, ri);
    }
    scattered.origin = rec.point;
    scattered.direction = direction;
    return true;
}

bool MetalScatter(MaterialMetal mat, Ray r, HitRecord rec, inout vec3 attenuation, inout Ray scattered, vec2 seed) {
    vec3 reflected = normalize(reflect(r.direction, rec.normal)) + (mat.fuzz * RandUnitVec3(seed));
    Ray s;
    scattered.origin = rec.point;
    scattered.direction = reflected;
    attenuation = mat.albedo;
    return true;
}

bool LambertianScatter(MaterialLambertian mat, Ray r, HitRecord rec, inout vec3 attenuation, inout Ray scattered, vec2 seed) {
    vec3 scattered_dir = rec.normal * RandUnitVec3(seed);
    if (NearZero(scattered_dir)) {
        scattered_dir = rec.normal;
    }
    scattered.origin = rec.point;
    scattered.direction = scattered_dir;
    attenuation = mat.albedo;
    return true;
}

bool Scatter(uint material_index, Ray r, HitRecord rec, inout vec3 attenuation, inout Ray scattered, vec2 seed) {
    uint true_idx = bitfieldExtract(material_index, 0, 16);
    // lambertian
    if (material_index >> 16 == 0) {
        LambertianScatter(lambertian_materials[true_idx], r, rec, attenuation, scattered, seed);

        // dielectric
    } else if (material_index >> 16 == 1) {
        return DielectricScatter(dielectric_materials[true_idx], r, rec, attenuation, scattered, seed);

        // metal
    } else {
        return MetalScatter(metal_materials[true_idx], r, rec, attenuation, scattered, seed);
    }
}

vec3 RayColor(Ray r, int depth, vec2 seed) {
    vec3 attenuation = vec3(1);
    Ray scattered = r;
    while (depth > 0) {
        HitRecord rec;
        Interval i;
        i.min = 0.0001;
        i.max = 1.0 / 0.0;
        bool hit_any = HitAny(scattered, i, rec);
        if (hit_any) {
            vec3 local_attenuation;
            if (Scatter(rec.material_index, scattered, rec, local_attenuation, scattered, seed)) {
                attenuation *= local_attenuation;
            } else {
                return vec3(0);
            }
        } else {
            vec3 unit_dir = normalize(scattered.direction);
            float a = 0.5 * (unit_dir.y + 1.0);
            attenuation *= mix(vec3(1.0), vec3(0.5, 0.7, 1.0), a);
            break;
        }

        depth--;
    }
    return attenuation;
}
void main() {
    // Get pixel coordinate
    vec2 pixel_coord = vec2(gl_FragCoord.x - 0.5, gl_FragCoord.y - 0.5);

    // Compute pixel center in world space
    vec3 pixel_center = viewport_upper_left
            + pixel_coord.x * pixel_delta_u
            + pixel_coord.y * pixel_delta_v;
    // pixel_center = pixel00_loc + gl_FragCoord.x * pixel_delta_u + gl_FragCoord.y * pixel_delta_v;

    // Now, pixel_center is the world-space position of the fragment
    // You can use this to calculate rays, lighting, etc.

    // Example output color based on pixel world position (for testing)
    o_color = vec4(pixel_center * 0.1, 1.0); // Scale it for visualization
}
// void main() {
//     // vec2 uv = gl_FragCoord.xy / resolution.xy;
//     Ray ray = GetRay(tex_coords);
//     vec3 ray_color = RayColor(ray, max_depth, tex_coords);
//     vec3 gamma_corrected = pow(ray_color, vec3(1.0 / 2.2));
//     o_color = vec4(gamma_corrected, 1.0);
// }
