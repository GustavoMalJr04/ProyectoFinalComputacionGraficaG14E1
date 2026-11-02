#version 330 core
layout (location = 0) in vec3 position;

out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view;   // vista SIN traslación: mat4(mat3(camera.GetViewMatrix()))

void main()
{
    TexCoords = position;
    vec4 pos = projection * view * vec4(position, 1.0);
    gl_Position = pos.xyww;   // profundidad = 1.0, siempre al fondo
}
