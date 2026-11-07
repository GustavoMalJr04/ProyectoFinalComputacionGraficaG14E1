#version 330 core
// =============================================================
//  lighting.vs  (Equipo 1 - Día de Muertos)
//  Para todos los modelos funciona igual que antes (ondula = 0).
//  Con ondula = 1 deforma la malla del papel picado:
//  onda viajera + aleteo fino + balanceo tipo péndulo,
//  con el borde superior (el doblez del cordón) fijo.
// =============================================================
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;

out vec3 Normal;
out vec3 FragPos;
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// ---- Papel picado ----
uniform int   ondula;       // 1 = aplica la ondulación; 0 = modelo normal
uniform float tiempo;       // glfwGetTime()
uniform float fase;         // desfase por bandera para que no se muevan igual
uniform float altoBandera;  // alto de ESTA bandera (metros)
uniform float amplitud;     // cuánto se separa la punta del plano (~0.035)
uniform float frecuencia;   // ondas por metro a lo ancho (~12.0)
uniform float velocidad;    // rapidez del viento (~2.5)
uniform float balanceo;     // vaivén tipo péndulo (~0.12)

void main()
{
    vec3 pos = position;
    vec3 nor = normal;

    if (ondula == 1)
    {
        // La bandera cuelga de y = 0 (cordón) a y = -altoBandera (punta)
        float w    = clamp(-position.y / altoBandera, 0.0, 1.0);   // 0 arriba, 1 abajo
        float peso = w * w;                                        // el doblez no se mueve

        float arg1 = frecuencia * pos.x - velocidad * tiempo + fase;                    // onda principal
        float arg2 = 2.3 * frecuencia * pos.x + 1.7 * velocidad * tiempo + 0.5 * fase;  // aleteo fino
        float onda = sin(arg1) + 0.35 * sin(arg2);

        float pendulo = balanceo * sin(0.9 * velocidad * tiempo + fase);

        pos.z += amplitud * peso * onda + pendulo * w * altoBandera;

        // Normal analítica de z = f(x, y) para que la luz resalte los pliegues
        float dOnda_dx = frecuencia * cos(arg1) + 0.35 * 2.3 * frecuencia * cos(arg2);
        float dzdx = amplitud * peso * dOnda_dx;
        float dzdy = -(2.0 * amplitud * w * onda + pendulo * altoBandera) / altoBandera;   // dw/dy = -1/alto
        nor = normalize(vec3(-dzdx, -dzdy, 1.0));
    }

    gl_Position = projection * view * model * vec4(pos, 1.0);
    FragPos     = vec3(model * vec4(pos, 1.0));
    Normal      = mat3(transpose(inverse(model))) * nor;
    TexCoords   = texCoords;
}
