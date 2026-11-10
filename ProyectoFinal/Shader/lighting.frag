#version 330 core

#define NUMBER_OF_POINT_LIGHTS 1

struct Material
{
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct DirLight
{
    vec3 direction;
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight
{
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight
{
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
    
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

uniform vec3 viewPos;
uniform DirLight dirLight;
uniform PointLight pointLights[NUMBER_OF_POINT_LIGHTS];
uniform SpotLight spotLight;
uniform Material material;
uniform int transparency;

// =============================================================
//  NOCHE Y LUZ DE VELAS  (Equipo 1 - Dia de Muertos)
// =============================================================
// 1) Mapa de luz de velas: textura vista desde arriba que guarda cuanta luz
//    de TODAS las velas llega a cada punto del piso (se calcula una vez en main.cpp).
uniform sampler2D mapaVelas;     // unidad de textura 5
uniform vec2  mapaMin;           // esquina (x, z) del mapa en el mundo
uniform float mapaTam;           // metros que cubre el mapa por lado
uniform float alturaLuzVelas;    // a partir de esta altura ya no llega la luz del mapa

// 2) Las velas mas cercanas a la camara como luces puntuales de verdad
#define MAX_LUCES_VELA 16
uniform int   numLucesVela;
uniform vec3  luzVelaPos[MAX_LUCES_VELA];
uniform float luzVelaInt[MAX_LUCES_VELA];

uniform vec3  colorVela;         // naranja calido
uniform float brilloVelas;       // intensidad general (con un parpadeo suave)

// 3) Objetos que brillan solos (las llamas): no los afecta la noche
uniform int emisivo;

// 4) Niebla nocturna exponencial
uniform vec3  colorNiebla;
uniform float densidadNiebla;

// Function prototypes
vec3 CalcDirLight( DirLight light, vec3 normal, vec3 viewDir );
vec3 CalcPointLight( PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir );
vec3 CalcSpotLight( SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir );

float factorNiebla( )
{
    float d = length( viewPos - FragPos );
    return exp( -pow( densidadNiebla * d, 2.0 ) );   // 1 = sin niebla, 0 = solo niebla
}

void main( )
{
    vec4 tex = texture( material.diffuse, TexCoords );

    // ---- Llamas: color de la textura, brillante, sin iluminacion ----
    if ( emisivo == 1 )
    {
        if ( tex.a < 0.1 ) discard;
        color = vec4( tex.rgb * 1.8 * factorNiebla( ), tex.a );
        return;
    }

    // Properties
    vec3 norm = normalize( Normal );
    if ( !gl_FrontFacing ) norm = -norm;   // papel picado: se ilumina igual por los dos lados
    vec3 viewDir = normalize( viewPos - FragPos );
    vec3 albedo = tex.rgb;

    // Luz de luna (direccional, azulada y tenue: la pone main.cpp)
    vec3 result = CalcDirLight( dirLight, norm, viewDir );

    // Point lights
    for ( int i = 0; i < NUMBER_OF_POINT_LIGHTS; i++ )
    {
        result += CalcPointLight( pointLights[i], norm, FragPos, viewDir );
    }

    // Spot light (linterna, se prende con F)
    result += CalcSpotLight( spotLight, norm, FragPos, viewDir );

    // ---- Resplandor de todas las velas (mapa) ----
    vec2  uv = ( FragPos.xz - mapaMin ) / mapaTam;
    float resplandor = texture( mapaVelas, uv ).r;
    float caidaAltura = clamp( 1.0 - FragPos.y / alturaLuzVelas, 0.0, 1.0 );
    // las caras que miran hacia arriba reciben un poco mas que las verticales
    float orientacion = 0.6 + 0.4 * max( norm.y, 0.0 );
    result += colorVela * brilloVelas * resplandor * caidaAltura * orientacion * albedo;

    // ---- Velas cercanas: luz puntual con direccion ----
    for ( int i = 0; i < numLucesVela; i++ )
    {
        vec3  L   = luzVelaPos[i] - FragPos;
        float d   = length( L );
        L /= d;
        float att = luzVelaInt[i] / ( 1.0 + 0.9 * d + 1.6 * d * d );
        float dif = max( dot( norm, L ), 0.0 );
        result += colorVela * att * dif * albedo;
    }

    // ---- Niebla ----
    result = mix( colorNiebla, result, factorNiebla( ) );

    color = vec4( result, tex.a );
    if ( color.a < 0.1 && transparency == 1 )
        discard;
}

// Calculates the color when using a directional light.
vec3 CalcDirLight( DirLight light, vec3 normal, vec3 viewDir )
{
    vec3 lightDir = normalize( -light.direction );
    
    // Diffuse shading
    float diff = max( dot( normal, lightDir ), 0.0 );
    
    // Specular shading
    vec3 reflectDir = reflect( -lightDir, normal );
    float spec = pow( max( dot( viewDir, reflectDir ), 0.0 ), material.shininess );
    
    // Combine results
    vec3 ambient = light.ambient * vec3( texture( material.diffuse, TexCoords ) );
    vec3 diffuse = light.diffuse * diff * vec3( texture( material.diffuse, TexCoords ) );
    vec3 specular = light.specular * spec * vec3( texture( material.specular, TexCoords ) );
    
    return ( ambient + diffuse + specular );
}

// Calculates the color when using a point light.
vec3 CalcPointLight( PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir )
{
    vec3 lightDir = normalize( light.position - fragPos );
    
    // Diffuse shading
    float diff = max( dot( normal, lightDir ), 0.0 );
    
    // Specular shading
    vec3 reflectDir = reflect( -lightDir, normal );
    float spec = pow( max( dot( viewDir, reflectDir ), 0.0 ), material.shininess );
    
    // Attenuation
    float distance = length( light.position - fragPos );
    float attenuation = 1.0f / ( light.constant + light.linear * distance + light.quadratic * ( distance * distance ) );
    
    // Combine results
    vec3 ambient = light.ambient * vec3( texture( material.diffuse, TexCoords ) );
    vec3 diffuse = light.diffuse * diff * vec3( texture( material.diffuse, TexCoords ) );
    vec3 specular = light.specular * spec * vec3( texture( material.specular, TexCoords ) );
    
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    
    return ( ambient + diffuse + specular );
}

// Calculates the color when using a spot light.
vec3 CalcSpotLight( SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir )
{
    vec3 lightDir = normalize( light.position - fragPos );
    
    // Diffuse shading
    float diff = max( dot( normal, lightDir ), 0.0 );
    
    // Specular shading
    vec3 reflectDir = reflect( -lightDir, normal );
    float spec = pow( max( dot( viewDir, reflectDir ), 0.0 ), material.shininess );
    
    // Attenuation
    float distance = length( light.position - fragPos );
    float attenuation = 1.0f / ( light.constant + light.linear * distance + light.quadratic * ( distance * distance ) );
    
    // Spotlight intensity
    float theta = dot( lightDir, normalize( -light.direction ) );
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp( ( theta - light.outerCutOff ) / epsilon, 0.0, 1.0 );
    
    // Combine results
    vec3 ambient = light.ambient * vec3( texture( material.diffuse, TexCoords ) );
    vec3 diffuse = light.diffuse * diff * vec3( texture( material.diffuse, TexCoords ) );
    vec3 specular = light.specular * spec * vec3( texture( material.specular, TexCoords ) );
    
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    
    return ( ambient + diffuse + specular );
}