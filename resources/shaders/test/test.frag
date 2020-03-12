#version 450
#extension GL_ARB_separate_shader_objects : enable

#define CHUNK_SIZE 32
#define MAX_VOXEL_TRAVERSAL 256

struct Sphere {
    vec4 color;
    vec3 c;
    float r;
};

struct Ray {
    vec3 o;
    vec3 d;
};

struct RayHit {
    vec4 color;
    vec3 pos;
};

layout (set = 0, binding = 0) uniform Camera {
    vec3 position;
    mat4 rotation;
    float aspect;
} camera;

layout (location = 0) in vec3 vertColor;
layout (location = 1) in vec2 vertPosition;

layout (location = 0) out vec4 fragColor;

Sphere sphere = Sphere(
    vec4(1, 0, 1, 1),
    vec3(0, 0, -4),
    1
);

//	Simplex 3D Noise 
//	by Ian McEwan, Ashima Arts
vec4 permute(vec4 x){return mod(((x*34.0)+1.0)*x, 289.0);}
vec4 taylorInvSqrt(vec4 r){return 1.79284291400159 - 0.85373472095314 * r;}
float simplex(vec3 v){ 
  const vec2  C = vec2(1.0/6.0, 1.0/3.0) ;
  const vec4  D = vec4(0.0, 0.5, 1.0, 2.0);

    // First corner
    vec3 i  = floor(v + dot(v, C.yyy) );
    vec3 x0 =   v - i + dot(i, C.xxx) ;

    // Other corners
    vec3 g = step(x0.yzx, x0.xyz);
    vec3 l = 1.0 - g;
    vec3 i1 = min( g.xyz, l.zxy );
    vec3 i2 = max( g.xyz, l.zxy );

    //  x0 = x0 - 0. + 0.0 * C 
    vec3 x1 = x0 - i1 + 1.0 * C.xxx;
    vec3 x2 = x0 - i2 + 2.0 * C.xxx;
    vec3 x3 = x0 - 1. + 3.0 * C.xxx;

    // Permutations
    i = mod(i, 289.0 ); 
    vec4 p = permute( permute( permute( 
                i.z + vec4(0.0, i1.z, i2.z, 1.0 ))
            + i.y + vec4(0.0, i1.y, i2.y, 1.0 )) 
            + i.x + vec4(0.0, i1.x, i2.x, 1.0 ));

    // Gradients
    // ( N*N points uniformly over a square, mapped onto an octahedron.)
    float n_ = 1.0/7.0; // N=7
    vec3  ns = n_ * D.wyz - D.xzx;

    vec4 j = p - 49.0 * floor(p * ns.z *ns.z);  //  mod(p,N*N)

    vec4 x_ = floor(j * ns.z);
    vec4 y_ = floor(j - 7.0 * x_ );    // mod(j,N)

    vec4 x = x_ *ns.x + ns.yyyy;
    vec4 y = y_ *ns.x + ns.yyyy;
    vec4 h = 1.0 - abs(x) - abs(y);

    vec4 b0 = vec4( x.xy, y.xy );
    vec4 b1 = vec4( x.zw, y.zw );

    vec4 s0 = floor(b0)*2.0 + 1.0;
    vec4 s1 = floor(b1)*2.0 + 1.0;
    vec4 sh = -step(h, vec4(0.0));

    vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy ;
    vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww ;

    vec3 p0 = vec3(a0.xy,h.x);
    vec3 p1 = vec3(a0.zw,h.y);
    vec3 p2 = vec3(a1.xy,h.z);
    vec3 p3 = vec3(a1.zw,h.w);

    //Normalise gradients
    vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
    p0 *= norm.x;
    p1 *= norm.y;
    p2 *= norm.z;
    p3 *= norm.w;

    // Mix final noise value
    vec4 m = max(0.6 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
    m = m * m;
    return 42.0 * dot( m*m, vec4( dot(p0,x0), dot(p1,x1), 
                                    dot(p2,x2), dot(p3,x3) ) );
}

vec4 getVoxel(ivec3 pos) {
    if (pos.y - 32 > simplex(vec3(pos.xz / 64.f, 0)) * 16.f + 16.f) {
        return vec4((sin(vec3(pos.x, 0, pos.z) / 32) * 0.5 + 0.5) * pos.y / 64.f, 1);
    } else return vec4(0);
}

bool raySphere(Ray ray, Sphere sphere, out RayHit rayHit) {
    vec3 diff = sphere.c - ray.o;
    float tca = dot(diff, ray.d);
    float distSq = dot(diff, diff) - tca * tca;

    float rSq = sphere.r * sphere.r;
    
    if (distSq > rSq) return false;

    float thc = sqrt(rSq - distSq);

    float t0 = tca - thc;
    float t1 = tca + thc;

    if (t0 > t1) rayHit.pos = ray.o + ray.d * t0;
    else rayHit.pos = ray.o + ray.d * t1;

    rayHit.color = sphere.color;
    rayHit.color.rgb = clamp(rayHit.color.rgb * 0.5 * abs(t0 - t1), vec3(0), vec3(1));

    return true;
}

bool rayVoxel(Ray ray, out RayHit rayHit) {
    
    bool hit = false;
    
    vec3 inc = sign(ray.d);
    vec3 delta = 1.f / abs(ray.d);
    vec3 currDelta = delta;

    vec3 pos = vec3(ray.o);
    if (delta.x < 0) pos.x++;
    if (delta.y < 0) pos.y++;
    if (delta.z < 0) pos.z++;

    rayHit.color = vec4(0, 0, 0, 1);

    for (uint i = 0; i < MAX_VOXEL_TRAVERSAL; i++) {
        // X is next
        if (currDelta.x < currDelta.y && currDelta.x < currDelta.z) {
            pos.x += inc.x;
            currDelta.x += delta.x;
        }

        // Y is next
        else if (currDelta.y < currDelta.x && currDelta.y < currDelta.z) {
            pos.y += inc.y;
            currDelta.y += delta.y;
        }

        // Z is next
        else if (currDelta.z < currDelta.x && currDelta.z < currDelta.y) {
            pos.z += inc.z;
            currDelta.z += delta.z;
        }

        vec4 voxel = getVoxel(ivec3(pos));

        if (voxel.a > 0) {
            if (!hit) {
                rayHit.pos = pos;
                hit = true;
            }

            rayHit.color.rgb += rayHit.color.a * voxel.rgb * voxel.a;
            rayHit.color.a *= 1.f - voxel.a;
        }
    }

    return hit;
}

void main() {

    Ray ray = Ray(camera.position, normalize(vec4(vertPosition / vec2(1, camera.aspect), -1, 0) * camera.rotation).xyz);
    
    RayHit rayHit;
    if (rayVoxel(ray, rayHit)) {
        fragColor = vec4(min(vec3(1, 1, 1), rayHit.color.rgb), rayHit.color.a);
    } else {
        discard;
    }
}