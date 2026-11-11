#version 330 core
layout (location = 0) in vec2 aEsquina;      // esquina del quad (-0.5..0.5)
layout (location = 1) in vec4 aPosTam;       // xyz = posición, w = tamaño
layout (location = 2) in vec2 aAlfaSemilla;  // x = alfa, y = semilla

uniform mat4 view;
uniform mat4 projection;
uniform float tiempo;

out vec2 vUV;
out float vAlfa;
out float vSemilla;
out vec3 vPosMundo;

void main()
{
    // Vectores derecha y arriba de la cámara -> el quad siempre mira a la cámara (billboard)
    vec3 derecha = vec3(view[0][0], view[1][0], view[2][0]);
    vec3 arriba  = vec3(view[0][1], view[1][1], view[2][1]);

    // Cada bocanada gira lentamente
    float ang = aAlfaSemilla.y * 6.2831 + tiempo * (0.3 + aAlfaSemilla.y * 0.4);
    mat2 R = mat2(cos(ang), sin(ang), -sin(ang), cos(ang));
    vec2 e = R * aEsquina;

    vec3 p = aPosTam.xyz + (derecha * e.x + arriba * e.y) * aPosTam.w;
    gl_Position = projection * view * vec4(p, 1.0);

    vUV = aEsquina + 0.5;
    vAlfa = aAlfaSemilla.x;
    vSemilla = aAlfaSemilla.y;
    vPosMundo = p;
}
