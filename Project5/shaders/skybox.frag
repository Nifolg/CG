
//ФРАГМЕНТНЫЙ ШЕЙДЕР - расчет для фрагментов объекта, а вершинный шейдер - расчет для вершин (вершин обычно гораздо меньше чем фпагментов)

#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;

void main()
{
    FragColor = texture(skybox, TexCoords);
}
