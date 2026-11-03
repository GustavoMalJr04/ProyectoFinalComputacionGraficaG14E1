#version 330 core
// =============================================================
//  Iluminación nocturna - Día de Muertos (Equipo 1)
//  Luz de luna (direccional) + hasta 16 veladoras/faroles (point lights)
//  + niebla exponencial + objetos emisivos (llamas) + transparencias con alfa
// =============================================================

#define MAX_POINT_LIGHTS 16

struct DirLight          // la luna
{
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight        // veladoras, cirios, faroles
{
    vec3 position;
    float constant;
    float linear;
    float quadratic;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

out vec4 color;

// Mesh.h liga la textura difusa del .mtl a "texture_diffuse1"
uniform sampler2D texture_diffuse1;

uniform vec3  viewPos;
uniform DirLight   moon;
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform int   numPointLights;       // cuántas luces del arreglo están activas

uniform float material_shininess;   // brillo especular (8-32)
uniform float specularStrength;     // 0 = mate (piedra, tierra), 0.3 = cera/barro barnizado

uniform int   transparency;         // 1 = descarta píxeles con alfa < 0.1 (papel picado, pétalos, alas)
uniform int   emissive;             // 1 = no se ilumina, brilla por sí mismo (llamas)
uniform float emissiveStrength;     // 1.0 - 3.0

uniform vec3  fogColor;             // mismo tono que el horizonte del skybox
uniform float fogDensity;           // 0.02 - 0.05; 0 desactiva la niebla

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 albedo)
{
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 halfway = normalize(lightDir + viewDir);              // Blinn-Phong
    float spec = pow(max(dot(normal, halfway), 0.0), material_shininess);

    vec3 ambient  = light.ambient  * albedo;
    vec3 diffuse  = light.diffuse  * diff * albedo;
    vec3 specular = light.specular * spec * specularStrength;
    return ambient + diffuse + specular;
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo)
{
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 halfway = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfway), 0.0), material_shininess);

    float dist = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * dist + light.quadratic * dist * dist);

    vec3 ambient  = light.ambient  * albedo;
    vec3 diffuse  = light.diffuse  * diff * albedo;
    vec3 specular = light.specular * spec * specularStrength;
    return (ambient + diffuse + specular) * attenuation;
}

void main()
{
    vec4 texColor = texture(texture_diffuse1, TexCoords);

    if (transparency == 1 && texColor.a < 0.1)
        discard;

    float dist = length(viewPos - FragPos);
    float fogFactor = exp(-pow(fogDensity * dist, 2.0));      // niebla exponencial cuadrada
    fogFactor = clamp(fogFactor, 0.0, 1.0);

    if (emissive == 1)
    {
        // Las llamas no reciben luz; la niebla las atenúa menos para que se vean a lo lejos
        vec3 glow = texColor.rgb * emissiveStrength;
        color = vec4(mix(fogColor, glow, max(fogFactor, 0.6)), texColor.a);
        return;
    }

    vec3 albedo  = texColor.rgb;
    vec3 norm    = normalize(Normal);
    if (!gl_FrontFacing) norm = -norm;                         // papel picado y pétalos se ven de ambos lados
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 result = CalcDirLight(moon, norm, viewDir, albedo);

    int n = min(numPointLights, MAX_POINT_LIGHTS);
    for (int i = 0; i < n; i++)
        result += CalcPointLight(pointLights[i], norm, FragPos, viewDir, albedo);

    result = mix(fogColor, result, fogFactor);
    color = vec4(result, texColor.a);
}
