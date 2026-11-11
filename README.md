# Proyecto Final Computación Gráfica — G14 Equipo 1
Recorrido 3D de Día de Muertos (OpenGL + Visual Studio).

## Cómo correrlo en cualquier computadora
1. Instala **Git para Windows** (ya trae Git LFS). Los modelos (.obj) y texturas (.png/.jpg) están en **Git LFS**.
2. Clona (NO uses "Download ZIP", trae los modelos vacíos):
   ```
   git lfs install
   git clone https://github.com/GustavoMalJr04/ProyectoFinalComputacionGraficaG14E1.git
   ```
   Si ya lo habías clonado sin LFS: `git lfs pull`
3. Abre `ProyectoFinal.sln` en Visual Studio 2022.
4. Configuración **Debug | x86** (las librerías son de 32 bits) y presiona **F5**.

No hace falta configurar nada más: las librerías están en `External Libraries/` y se referencian con `$(SolutionDir)`, y todas las rutas de modelos, shaders e imágenes son relativas a la carpeta `ProyectoFinal/`.
