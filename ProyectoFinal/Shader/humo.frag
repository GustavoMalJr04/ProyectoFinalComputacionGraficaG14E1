#version 330 core
in vec2 vUV;
in float vAlfa;
in float vSemilla;
in vec3 vPosMundo;

out vec4 FragColor;

uniform vec3 colorHumo;
// Misma niebla que lighting.frag para que el humo se pierda igual que el resto de la escena
uniform vec3 colorNiebla;
uniform float densidadNiebla;
uniform vec3 viewPos;

// Ruido procedural para que cada bocanada tenga forma irregular (sin textura)
float hash(vec2 p) { return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453); }

float ruido(vec2 p)
{
    vec2 i = floor(p), f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    return mix(mix(hash(i),               hash(i + vec2(1.0, 0.0)), f.x),
               mix(hash(i + vec2(0.0, 1.0)), hash(i + vec2(1.0, 1.0)), f.x), f.y);
}

float fbm(vec2 p)
{
    float v = 0.0, a = 0.5;
    for (int i = 0; i < 4; i++) { v += a * ruido(p); p *= 2.0; a *= 0.5; }
    return v;
}

void main()
{
    float d = length(vUV - 0.5) * 2.0;                 // 0 en el centro, 1 en el borde
    float base = 1.0 - smoothstep(0.2, 1.0, d);        // bola suave
    float n = fbm(vUV * 3.0 + vSemilla * 17.0);        // jirones
    float a = base * smoothstep(0.25, 0.75, n + base * 0.4) * vAlfa;
    if (a < 0.01) discard;
    float d2 = length(viewPos - vPosMundo);
    float niebla = exp(-pow(densidadNiebla * d2, 2.0));   // 1 = sin niebla
    FragColor = vec4(mix(colorNiebla, colorHumo, niebla), a * niebla);
}
