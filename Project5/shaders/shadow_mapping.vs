// преобразование в пространство источника света делаем в вершинном шейдере
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;

out VS_OUT {
    vec3 FragPos;//координаты вершины в пространстве мира
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;//координаты вершины в пространстве источника света
} vs_out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat4 lightSpaceMatrix;

void main()
{
    vs_out.FragPos = vec3(model * vec4(aPos, 1.0));
    vs_out.Normal = transpose(inverse(mat3(model))) * aNormal;
    vs_out.TexCoords = aTexCoords;
    vs_out.FragPosLightSpace = lightSpaceMatrix * vec4(vs_out.FragPos, 1.0);
    //Мы принимаем ту же самую lightSpaceMatrix, что использовалась в первом проходе для рисования глубины, и с её помощью переводим вектор в пространство источника света.
    
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
//Местоположение текстуры чаще называется !!!текстурным блоком!!!
